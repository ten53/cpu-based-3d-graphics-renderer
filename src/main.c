#include <stdint.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include "display.h"

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
}


void update(void) {
    // TODO:
}


void render(void) {
    int red = 255;
    int green = 0;
    int blue = 0;
    int opacity = 255;
    SDL_SetRenderDrawColor(renderer, red, green, blue, opacity);
    SDL_RenderClear(renderer);

    draw_grid();

    draw_rect(300, 200, 300, 150, 0xFFFF00FF);

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
