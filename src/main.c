#include <stdint.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "array.h"
#include "matrix.h"
#include "light.h"
#include "texture.h"
#include "triangle.h"


// ----- ARRAY OF TRIANGLES TO RENDER FRAME BY FRAME -----
triangle_t* triangles_to_render = NULL;


// ----- GLOBAL VARIABLES FOR EXECUTION STATUS & GAME LOOP -----
bool is_running = false;
int previous_frame_time = 0;
vec3_t camera_position = { .x = 0, .y = 0, .z = 0 };   // origin
mat4_t proj_matrix;


// ----- INIT VARIABLES & GAME FUNCTIONS
void setup(void) {
    // initialize render mode and triangle culling method
    render_method = RENDER_WIRE;
    cull_method = CULL_BACKFACE;

    // allocate required memory in bytes to hold color buffer
    color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);

    // TODO: test for malloc fail, null pointer return

    // create SDL texture to display the color buffer
    color_buffer_texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            window_width,
            window_height);

    // initialize perspective projection matrix
    float fov = M_PI / 3.0; // radians, same as 180/3 or 60 deg
    float aspect = (float) window_width / (float) window_width;
    float znear = 0.1;
    float zfar = 100.0;
    proj_matrix = mat4_make_perspective(fov, aspect, znear, zfar);

    // manually load harcoded texture data from static array
    mesh_texture = (uint32_t*) REDBRICK_TEXTURE;
    texture_width = 64;
    texture_height = 64;

    // load vertex and face values for mesh data structure
    load_cube_mesh_data();   // hardcoded values
    // load_obj_file_data("./assets/f22.obj");
}


// ----- SYSTEM EVENTS & KEYBOARD INPUT -----
void process_input(void) {
    SDL_Event event;
    SDL_PollEvent(&event);
    switch (event.type) {
        case SDL_QUIT:
            is_running = false;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
                is_running = false;
            if (event.key.keysym.sym == SDLK_1)
                render_method = RENDER_WIRE_VERTEX;
            if (event.key.keysym.sym == SDLK_2)
                render_method = RENDER_WIRE;
            if (event.key.keysym.sym == SDLK_3)
                render_method = RENDER_FILL_TRIANGLE;
            if (event.key.keysym.sym == SDLK_4)
                render_method = RENDER_FILL_TRIANGLE_WIRE;
            if (event.key.keysym.sym == SDLK_5)
                render_method = RENDER_TEXTURED;
            if (event.key.keysym.sym == SDLK_6)
                render_method = RENDER_TEXTURED_WIRE;
            if (event.key.keysym.sym == SDLK_c)
                cull_method = CULL_BACKFACE;
            if (event.key.keysym.sym == SDLK_d)
                cull_method = CULL_NONE;
            break;
    }
}


// ----- UPDATE FRAME BY FRAME WITH FIXED TIME STEP -----
void update(void) {
    // sleep until target frame time (millisec) is reached
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

    // delay execution if running to fast
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }

    previous_frame_time = SDL_GetTicks();

    // init array of triangles to render
    triangles_to_render = NULL;

    // scale, rotate, and translate values per animation frame
    mesh.rotation.x += 0.005;
    // mesh.rotation.y += 0.0;
    // mesh.rotation.z += 0.0;

    mesh.scale.x += 0.0;
    mesh.scale.y += 0.0;

    mesh.translation.z = 5.0;   // move mesh back from camera

    // create scale, rotation, and translation matrices used to multiply mesh vertices
    mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
    mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);
    mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);

    // loop all triangle faces of mesh
    int num_faces = array_length(mesh.faces);
    for (int i = 0; i < num_faces; i++) {
        face_t mesh_face = mesh.faces[i];

        // current face
        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a - 1]; // -1 to compensate for index placement
        face_vertices[1] = mesh.vertices[mesh_face.b - 1];
        face_vertices[2] = mesh.vertices[mesh_face.c - 1];

        // transformation
        // init array to store transformed vertices(x,y,z)
        vec4_t transformed_vertices[3];

        // loop all 3 vertices of current face and apply transformations
        for (int j = 0; j < 3; j++) {
            vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

            // create world matrix - combine scale, rotation, and translation matrices
            mat4_t world_matrix = mat4_identity();

            // order matters - scale first, then rotate, then translate: [T]*[R]*[S]*v
            world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
            world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

            // multiply world matrix by original vector
            transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

            // save transformed vertex in array of transformed vertices
            transformed_vertices[j] = transformed_vertex;
        }

        // get vectors from A,B,C to calculate normal
        vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]);  /*    A     */
        vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]);  /*   / \    */
        vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]);  /*  C---B  */

        // get vector subtraction of B-A and C-A
        vec3_t vector_ab = vec3_sub(vector_b, vector_a);
        vec3_t vector_ac = vec3_sub(vector_c, vector_a);
        vec3_normalize(&vector_ab);
        vec3_normalize(&vector_ac);

        // calculate face normal using cross product to find perpendicular in a left-handed coordinate system
        vec3_t normal = vec3_cross(vector_ab, vector_ac);  // opposite order 'vec3_cross(vector_ac, vector_ab)' for a right-handed coordinate system!
        vec3_normalize(&normal);

        // find vector between point in triangle and camera origin
        vec3_t camera_ray = vec3_sub(camera_position, vector_a);

        // calculate alignement of camera ray with face normal using dot product
        float dot_normal_camera = vec3_dot(normal, camera_ray);

        // backface culling test
        if (cull_method == CULL_BACKFACE) {
            // bypass projection of triangles that are looking away from camera
            if (dot_normal_camera < 0) {
                continue;
            }
        }

        // projection
        vec4_t projected_points[3];

        // loop all 3 vertices and project them
        for (int j = 0; j < 3; j++) {

            // project current vertex
            projected_points[j] = mat4_mul_vec4_project(proj_matrix, transformed_vertices[j]);

            // flip vertically since y values of 3D mesh grow bottom->up and in screen space y values grow top->down
            projected_points[j].y *= -1;

            // scale into the view
            projected_points[j].x *= (window_width / 2.0);
            projected_points[j].y *= (window_height / 2.0);


            // translate projected points to center of screen
            projected_points[j].x += (window_width / 2.0);
            projected_points[j].y += (window_height / 2.0);
        }

        // calculate avg depth for each face based on vertices after transformation
        float avg_depth = (transformed_vertices[0].z + transformed_vertices
            [1].z + transformed_vertices[2].z) / 3.0;

        // calculate shade intensity based on alignment of face normal and the inverse of the light ray
        float light_intensity_factor = -vec3_dot(normal, light.direction);

        // calculate triangle color based on angle of light
        uint32_t triangle_color = light_apply_intensity(mesh_face.color,light_intensity_factor );

            triangle_t projected_triangle = {
               .points = {
                { projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w },
                { projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w },
                { projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w },
            },
                .texcoords = {
                    { mesh_face.a_uv.u, mesh_face.a_uv.v },
                    { mesh_face.b_uv.u, mesh_face.b_uv.v },
                    { mesh_face.c_uv.u, mesh_face.c_uv.v }
                },
                .color = triangle_color,
                .avg_depth = avg_depth
            };

        // save current projected triangle in the array of triangles to render
        array_push(triangles_to_render, projected_triangle);
        }

    // sort triangles to render by avg depth (painters algorithm)
    int num_triangles = array_length(triangles_to_render);
    for (int i = 0; i < num_triangles; i++) {
        for (int j = i; j < num_triangles; j++) {
            if (triangles_to_render[i].avg_depth < triangles_to_render[j].avg_depth) {
                // swap triangles positions in the array
                triangle_t temp = triangles_to_render[i];
                triangles_to_render[i] = triangles_to_render[j];
                triangles_to_render[j] = temp;
            }
        }
    }
}


// ----- DRAW OBJECTS ON DISPLAY -----
void render(void) {
    draw_grid();

   // loop all projected triangles and render them
    int num_triangles = array_length(triangles_to_render);
    for (int i = 0; i < num_triangles; i++) {
        triangle_t triangle = triangles_to_render[i];

        // draw filled triangle
        if (render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE) {
            draw_filled_triangle(
                triangle.points[0].x, triangle.points[0].y, // vertex A
                triangle.points[1].x, triangle.points[1].y, // vertex B
                triangle.points[2].x, triangle.points[2].y, // vertex C
                triangle.color
            );
        }

        // draw textured triangle
           if (render_method == RENDER_TEXTURED || render_method == RENDER_TEXTURED_WIRE) {
           draw_textured_triangle(
                triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, triangle.texcoords[0].u, triangle.texcoords[0].v, // vertex A
                triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, triangle.texcoords[1].u, triangle.texcoords[1].v, // vertex B
                triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, triangle.texcoords[2].u, triangle.texcoords[2].v, // vertex C
                mesh_texture
            );        }

        // draw triangle wireframe
        if (render_method == RENDER_WIRE || render_method == RENDER_WIRE_VERTEX || render_method == RENDER_FILL_TRIANGLE_WIRE || render_method == RENDER_TEXTURED_WIRE) {
            draw_triangle(
                triangle.points[0].x, triangle.points[0].y, // vertex A
                triangle.points[1].x, triangle.points[1].y, // vertex B
                triangle.points[2].x, triangle.points[2].y, // vertex C
                0xFFFFFFFF
            );
        }

        // draw triangle vertex points
        if (render_method == RENDER_WIRE_VERTEX) {
            draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFF0000); // vertex A
            draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFF0000); // vertex B
            draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFF0000); // vertex C
        }
    }

    // clear array of triangles to render every frame loop
    array_free(triangles_to_render);

    render_color_buffer();

    clear_color_buffer(0xFF000000);     // ARGB pixel format

    SDL_RenderPresent(renderer);
}


// ----- FREE ALL DYNAMICALLY ALLOCATED MEMORY -----
void free_resources(void) {
    free(color_buffer);
    array_free(mesh.faces);
    array_free(mesh.vertices);
}


// ----- ENTRY POINT -----
int main(void) {
    is_running = initialize_window();

    setup();

    // game loop
    while(is_running) {
        process_input();
        update();
        render();
    }

    destroy_window();
    free_resources();

    return 0;
}
