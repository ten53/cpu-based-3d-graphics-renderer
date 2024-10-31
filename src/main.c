#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

// Globals
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
int window_width = 800;
int window_height = 600;
int default_driver = -1;
int default_flags = 0;
bool is_running = false;

bool  initialize_window(void){
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    }

    // create SDL window
    window = SDL_CreateWindow(
            NULL,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            window_width,
            window_height,
            SDL_WINDOW_BORDERLESS);

    if (!window) {
        fprintf(stderr, "Error creating SDL window.\n");
        return false;
    }

    // create SDL renderer
    renderer = SDL_CreateRenderer(
            window,
            default_driver,
            default_flags);

    if (!renderer) {
        fprintf(stderr, "Error creating SDL renderer.\n");
        return false;
    }

    return true;
}




int main(void) {

    is_running = initialize_window();

    return 0;
}
