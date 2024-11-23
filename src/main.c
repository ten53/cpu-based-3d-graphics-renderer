#include <stdint.h>
#include <stdio.h>
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
#include "upng.h"
#include "camera.h"
#include "clipping.h"


// ----- GLOBAL VARIABLES FOR EXECUTION STATUS & GAME LOOP -----
bool is_running = false;
int previous_frame_time = 0;
float delta_time = 0.0;

// global transformation matrices
mat4_t world_matrix;
mat4_t proj_matrix;
mat4_t view_matrix;


// ----- ARRAY OF TRIANGLES TO RENDER FRAME BY FRAME -----
#define MAX_TRIANGLES_PER_MESH 100000
triangle_t triangles_to_render[MAX_TRIANGLES_PER_MESH];
int num_triangles_to_render = 0;


// ----- INIT VARIABLES & GAME FUNCTIONS -----
void setup(void) {
    // initialize render mode and triangle culling method
    set_render_method(RENDER_TEXTURED);
    set_cull_method(CULL_BACKFACE);

    // initialize scene light direction
    init_light(vec3_new(0, 0, 1));

    // initialize perspective projection matrix
    float aspect_x = (float) get_window_width() / (float) get_window_height();
    float aspect_y = (float) get_window_height() / (float) get_window_width();
    float fov_y = M_PI / 3.0; // radians, same as 180/3 or 60 deg
    float fov_x = atan(tan(fov_y / 2) * aspect_x) * 2.0;
    float z_near = 0.1;
    float z_far = 100.0;
    proj_matrix = mat4_make_perspective(fov_y, aspect_y, z_near, z_far);

    // initialize frustum planes with a point and normal
    init_frustum_planes(fov_x,fov_y, z_near, z_far);

    load_mesh("/Users/ten53/Developer/cpu-based-3d-graphics-renderer/assets/f22.obj", "/Users/ten53/Developer/cpu-based-3d-graphics-renderer/assets/f22.png", vec3_new(1 ,1 ,1), vec3_new(-3,0,8), vec3_new(0,0,0));
    load_mesh("/Users/ten53/Developer/cpu-based-3d-graphics-renderer/assets/efa.obj", "/Users/ten53/Developer/cpu-based-3d-graphics-renderer/assets/efa.png", vec3_new(1 ,1 ,1), vec3_new(3,0,8), vec3_new(0,0,0));
}


// ----- SYSTEM EVENTS & KEYBOARD INPUT -----
void process_input(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                is_running = false;
                break;
                    case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    is_running = false;
                    break;
                }
                if (event.key.keysym.sym == SDLK_1) {
                    set_render_method(RENDER_WIRE_VERTEX);
                    break;
                }
                if (event.key.keysym.sym == SDLK_2) {
                    set_render_method(RENDER_WIRE);
                    break;
                }
                if (event.key.keysym.sym == SDLK_3) {
                    set_render_method(RENDER_FILL_TRIANGLE);
                    break;
                }
                if (event.key.keysym.sym == SDLK_4) {
                    set_render_method(RENDER_FILL_TRIANGLE_WIRE);
                    break;
                }
                if (event.key.keysym.sym == SDLK_5) {
                    set_render_method(RENDER_TEXTURED);
                    break;
                }
                if (event.key.keysym.sym == SDLK_6) {
                    set_render_method(RENDER_TEXTURED_WIRE);
                    break;
                }
                if (event.key.keysym.sym == SDLK_c) {
                    set_cull_method(CULL_BACKFACE);
                    break;
                }
                if (event.key.keysym.sym == SDLK_x) {
                    set_cull_method(CULL_NONE);
                    break;
                }
                if (event.key.keysym.sym == SDLK_w) {
                    rotate_camera_pitch(+3.0 * delta_time);
                    break;
                }
                if (event.key.keysym.sym == SDLK_s) {
                    rotate_camera_pitch(-3.0 * delta_time);
                    break;
                }
                if (event.key.keysym.sym == SDLK_d) {
                    rotate_camera_yaw(+1.0 * delta_time);
                    break;
                }
                if (event.key.keysym.sym == SDLK_a) {
                    rotate_camera_yaw(-1.0 * delta_time);
                    break;
                }
                if (event.key.keysym.sym == SDLK_e) {
                    update_camera_forward_velocity(vec3_mul(get_camera_direction(), 5.0 * delta_time));
                    update_camera_position(vec3_add(get_camera_position(), get_camera_forward_velocity()));
                    break;
                }
                if (event.key.keysym.sym == SDLK_q) {
                    update_camera_forward_velocity(vec3_mul(get_camera_direction(), 5.0 * delta_time));
                    update_camera_position(vec3_sub(get_camera_position(), get_camera_forward_velocity()));
                    break;
                }
                break;
        }
    }
}


// ----- PROCESS GRAPHICS PIPELINE STAGES FOR ALL MESH TRIANGLES -----
//
// +-------------+
// | Model space |  <-- original mesh vertices
// +-------------+
// |   +-------------+
// `-> | World space |  <-- multiply by world matrix
//     +-------------+
//     |   +--------------+
//     `-> | Camera space |  <-- multiply by view matrix
//         +--------------+
//         |    +------------+
//         `--> |  Clipping  |  <-- clip against the six frustum planes
//              +------------+
//              |    +------------+
//              `--> | Projection |  <-- multiply by projection matrix
//                   +------------+
//                   |    +-------------+
//                   `--> | Image space |  <-- apply perspective divide
//                        +-------------+
//                        |    +--------------+
//                        `--> | Screen space |  <-- ready to render
//                             +--------------+
//
void process_graphics_pipeline_stages(mesh_t* mesh) {
    // create scale, rotation, and translation matrices used to multiply mesh vertices
    mat4_t scale_matrix = mat4_make_scale(mesh->scale.x, mesh->scale.y, mesh->scale.z);
    mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh->rotation.x);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh->rotation.y);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh->rotation.z);
    mat4_t translation_matrix = mat4_make_translation(mesh->translation.x, mesh->translation.y, mesh->translation.z);

    // update camera look at target to create view matrix
    vec3_t target = get_camera_lookat_target();
    vec3_t up_direction = vec3_new(0, 1, 0);
    view_matrix = mat4_look_at(get_camera_position(), target, up_direction);

    // loop all triangle faces of mesh
    int num_faces = array_length(mesh->faces);
    for (int i = 0; i < num_faces; i++) {
        face_t mesh_face = mesh->faces[i];

        // current face
        vec3_t face_vertices[3];
        face_vertices[0] = mesh->vertices[mesh_face.a];
        face_vertices[1] = mesh->vertices[mesh_face.b];
        face_vertices[2] = mesh->vertices[mesh_face.c];

        // transformation
        // init array to store transformed vertices(x,y,z)
        vec4_t transformed_vertices[3];

        // loop all 3 vertices of current face and apply transformations
        for (int j = 0; j < 3; j++) {
            vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

            // multiply view matrix by current vector to transform scene to camera space
            transformed_vertex = mat4_mul_vec4(view_matrix, transformed_vertex);

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

        // calculate triangle face normal
        vec3_t face_normal = get_triangle_normal(transformed_vertices);

        // backface culling test
        if (is_cull_backface()) {
            // find vector between point in triangle and camera origin
            vec3_t origin = { 0, 0, 0 };
            vec3_t camera_ray = vec3_sub(origin, vec3_from_vec4(transformed_vertices[0]));

            // calculate alignement of camera ray with face normal using dot product
            float dot_normal_camera = vec3_dot(face_normal, camera_ray);


            // bypass projection of triangles that are looking away from camera
            if (dot_normal_camera < 0) {
                continue;
            }
        }

        // create polygon from original transformed triangle to be clipped
        polygon_t polygon = polygon_from_triangle(
                vec3_from_vec4(transformed_vertices[0]),
                vec3_from_vec4(transformed_vertices[1]),
                vec3_from_vec4(transformed_vertices[2]),
                mesh_face.a_uv,
                mesh_face.b_uv,
                mesh_face.c_uv);

        // returns new polygon with potential new vertices
        clip_polygon(&polygon);

        // break polygon into triangles after clipping
        triangle_t triangles_after_clipping[MAX_NUM_POLY_TRIANGLES];
        int num_triangles_after_clipping = 0;
        triangles_from_polygon(&polygon, triangles_after_clipping, &num_triangles_after_clipping);

        // loop all assembled triangles after clipping
        for (int t = 0; t < num_triangles_after_clipping; t++) {
            triangle_t triangle_after_clipping = triangles_after_clipping[t];

            // projection
            vec4_t projected_points[3];

            // loop all 3 vertices and project and convert them
            for (int j = 0; j < 3; j++) {

                // project current vertex
                projected_points[j] = mat4_mul_vec4_project(proj_matrix, triangle_after_clipping.points[j]);

                // flip vertically since y values of 3D mesh grow bottom->up and in screen space y values grow top->down
                projected_points[j].y *= -1;

                // scale into the view
                projected_points[j].x *= (get_window_width() / 2.0);
                projected_points[j].y *= (get_window_height() / 2.0);

                // translate projected points to center of screen
                projected_points[j].x += (get_window_width() / 2.0);
                projected_points[j].y += (get_window_height() / 2.0);
            }

            // calculate shade intensity based on alignment of face normal and the inverse of the light ray
            float light_intensity_factor = -vec3_dot(face_normal, get_light_direction());

            // calculate triangle color based on angle of light
            uint32_t triangle_color = light_apply_intensity(mesh_face.color,light_intensity_factor );

            triangle_t triangle_to_render = {
                .points = {
                    { projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w },
                    { projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w },
                    { projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w },
                },
                .texcoords = {
                    { triangle_after_clipping.texcoords[0].u, triangle_after_clipping.texcoords[0].v },
                    { triangle_after_clipping.texcoords[1].u, triangle_after_clipping.texcoords[1].v },
                    { triangle_after_clipping.texcoords[2].u, triangle_after_clipping.texcoords[2].v },
                },
                .color = triangle_color,
                .texture = mesh->texture

            };

            // save current projected triangle in the array of triangles to render
            if (num_triangles_to_render < MAX_TRIANGLES_PER_MESH) {
                triangles_to_render[num_triangles_to_render++] = triangle_to_render;
            }
        }
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

    // delta time factor converted to seconds to be used to update game objects
    delta_time = (SDL_GetTicks() - previous_frame_time) / 1000.0;

    previous_frame_time = SDL_GetTicks();

    // init counter of triangles to render for current frame
    num_triangles_to_render = 0;

    // loop all meshes of our scene
    for (int mesh_index = 0; mesh_index < get_num_meshes(); mesh_index++) {
        mesh_t* mesh = get_mesh(mesh_index);

        // change mesh scale, rotation, and translation values per animation frame
        // mesh.rotation.x += 0.0 * delta_time;
        // mesh.rotation.y += 0.0 * delta_time;
        // mesh.rotation.z += 0.0 * delta_time;
        // mesh.translation.z = 5.0;

        // process graphics pipeline stages for every mesh of 3D scene
        process_graphics_pipeline_stages(mesh);
    }
}


// ----- DRAW OBJECTS ON DISPLAY -----
void render(void) {
    // clear all arrays to prepare for next frame
    clear_color_buffer(0xFF000000);
    clear_z_buffer();

    draw_grid();

    // loop all projected triangles and render them
    for (int i = 0; i < num_triangles_to_render; i++) {
        triangle_t triangle = triangles_to_render[i];

        // draw filled triangle
        if (should_render_filled_triangles()) {
            draw_filled_triangle(
                triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, // vertex A
                triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, // vertex A
                triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, // vertex A
                // vertex C
                triangle.color
            );
        }

        // draw textured triangle
        if (should_render_textured_triangles()) {
            draw_textured_triangle(
                triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, triangle.texcoords[0].u, triangle.texcoords[0].v, // vertex A
                triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, triangle.texcoords[1].u, triangle.texcoords[1].v, // vertex B
                triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, triangle.texcoords[2].u, triangle.texcoords[2].v, // vertex C
                triangle.texture
            );
        }

        // draw triangle wireframe
        if (should_render_wireframe()) {
            draw_triangle(
                triangle.points[0].x, triangle.points[0].y, // vertex A
                triangle.points[1].x, triangle.points[1].y, // vertex B
                triangle.points[2].x, triangle.points[2].y, // vertex C
                0xFFFFFFFF
            );
        }

        // draw triangle vertex points
        if (should_render_wire_vertex()) {
            draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFF0000); // vertex A
            draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFF0000); // vertex B
            draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFF0000); // vertex C
        }
    }

    // draw color buffer to SDL window
    render_color_buffer();
}


// ----- FREE ALL DYNAMICALLY ALLOCATED MEMORY -----
void free_resources(void) {
    free_meshes();
    destroy_window();
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

    free_resources();

    return 0;
}
