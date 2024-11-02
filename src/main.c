#include <stdint.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include "display.h"
#include "vector.h"


const int N_POINTS = 9 * 9 * 9;
vec3_t cube_points[N_POINTS];
vec2_t projected_points[N_POINTS];

vec3_t camera_position = { .x = 0, .y = 0, .z = -5 };

float fov_factor = 640;

bool is_running = false;


void setup(void) {
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
}


void process_input(void) {
    SDL_Event event;
    SDL_PollEvent(&event);

    switch(event.type) {
        case SDL_QUIT:
            is_running = false;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
                is_running = false;
            break;
    }

    int point_count = 0;

    for (float x = -1; x <= 1; x += 0.25) {
        for (float y = -1; y <= 1; y += 0.25) {
            for (float z = -1; z <= 1; z += 0.25) {
                vec3_t new_point = { .x = x, .y = y, .z = z };
                cube_points[point_count++] = new_point;
            }
        }
    }
}


vec2_t project(vec3_t point) {
    // converts a vect3_t point to a vec2_t point (strips out .z)
    vec2_t converted_point = {
        .x = (fov_factor * point.x) / point.z,
        .y = (fov_factor * point.y) / point.z
    };
    return converted_point;
}


void update(void) {
    for (int i = 0; i < N_POINTS; i++) {
        vec3_t point = cube_points[i];

        // Move points away from camera position
        point.z -= camera_position.z;

        // project the current point
        vec2_t current_point = project(point);

        // save current projected 2D vector in the array of projected points
        projected_points[i] = current_point;
    }
}


void render(void) {
    draw_grid();

    // loop all projected points and render them
    for (int i = 0; i < N_POINTS; i++) {
        vec2_t projected_point = projected_points[i];
        draw_rect(
                projected_point.x + (window_width / 2),
                projected_point.y + (window_height / 2),
                4,
                4,
                0xFFFFFF00);
    }

    render_color_buffer();

    clear_color_buffer(0xFF000000);     // ARGB pixel format

    SDL_RenderPresent(renderer);
}


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

    return 0;
}
