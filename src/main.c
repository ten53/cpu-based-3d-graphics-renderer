#include <stdint.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "array.h"


// array of triangles to render frame by frame
triangle_t* triangles_to_render = NULL;

// globals for execution status and game loop
vec3_t camera_position = { 0, 0, 0 };   // origin
float fov_factor = 640;
bool is_running = false;
int previous_frame_time = 0;


// initialize variables and game functions
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

    // initialize hardcoded cube mesh (if there is no obj file)
    load_cube_mesh_data();

    // load_obj_file_data("./assets/cube.obj");
}


// poll system events and handle keyboard input
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
            if (event.key.keysym.sym == SDLK_c)
                cull_method = CULL_BACKFACE;
            if (event.key.keysym.sym == SDLK_d)
                cull_method = CULL_NONE;
            break;
    }
}


// converts 3D vector into 2D point (strips out .z) for projection
vec2_t project(vec3_t point) {
    vec2_t converted_point = {
        .x = (fov_factor * point.x) / point.z,
        .y = (fov_factor * point.y) / point.z
    };
    return converted_point;
}


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

    mesh.rotation.x += 0.01;
    mesh.rotation.y += 0.01;
    mesh.rotation.z += 0.01;

    // loop all triangle faces of our mesh
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
        vec3_t transformed_vertices[3];

        // loop all 3 vertices of current face and apply transformations
        for (int j = 0; j < 3; j++) {
            vec3_t transformed_vertex = face_vertices[j];

            transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
            transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
            transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

            // translate vertex away from camera (left-handed coordinate system)
            transformed_vertex.z += 5;

            // save transformed vertex in array of transformed vertices
            transformed_vertices[j] = transformed_vertex;
        }

        // backface culling
        vec3_t vector_a = transformed_vertices[0];  /*    A     */
        vec3_t vector_b = transformed_vertices[1];  /*   / \    */
        vec3_t vector_c = transformed_vertices[2];  /*  C---B  */

        // get vector subtraction of B-A and C-A
        vec3_t vector_ab = vec3_sub(vector_b, vector_a);
        vec3_t vector_ac = vec3_sub(vector_c, vector_a);
        vec3_normalize(&vector_ab);
        vec3_normalize(&vector_ac);

        // calculate face normal using cross product to find perpendicular
        // in a left-handed coordinate system
        vec3_t normal = vec3_cross(vector_ab, vector_ac);  // opposite order 'vec3_cross(vector_ac, vector_ab)' for a right-handed coordinate system!
        // normalize face normal vector
        vec3_normalize(&normal);

        // find vector between point in triangle and camera origin
        vec3_t camera_ray = vec3_sub(camera_position, vector_a);

        // calculate alignement of camera ray with face normal using dot product
        float dot_normal_camera = vec3_dot(normal, camera_ray);

        // bypass projection of triangles that are looking away from camera
        if (dot_normal_camera < 0) {
            continue;
        }

        // projection
        vec2_t projected_points[3];

        // loop all 3 vertices and project them
        for (int j = 0; j < 3; j++) {

            // project current vertex
            projected_points[j] = project(transformed_vertices[j]);

            // scale and translate projected points to center of screen
            projected_points[j].x += (window_width / 2);
            projected_points[j].y += (window_height / 2);
        }


            triangle_t projected_triangle = {
                .points = {
                    { projected_points[0].x, projected_points[0].y },
                    { projected_points[1].x, projected_points[1].y },
                    { projected_points[2].x, projected_points[2].y },
                },
                .color = mesh_face.color

            };

        // save current projected triangle in the array of triangles to render
        array_push(triangles_to_render, projected_triangle);
        }
}


// draw obects on display
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

        // draw triangle wireframe
        if (render_method == RENDER_WIRE || render_method == RENDER_WIRE_VERTEX || render_method == RENDER_FILL_TRIANGLE_WIRE) {
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


// free all dynamically allocated memory
void free_resources(void) {
    free(color_buffer);
    array_free(mesh.faces);
    array_free(mesh.vertices);
}

// entry point
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
