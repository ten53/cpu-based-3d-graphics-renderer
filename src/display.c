#include "display.h"


SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
uint32_t* color_buffer = NULL;
SDL_Texture* color_buffer_texture = NULL;
int window_width = 800;
int window_height = 600;


bool  initialize_window(void){
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    }

    // determine fullscreen max width & height
    SDL_DisplayMode display_mode;
    int display_index = 0;
    SDL_GetCurrentDisplayMode(display_index, &display_mode);

    window_width = display_mode.w;
    window_height = display_mode.h;

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
    int default_driver = -1;
    int default_flags = 0;
    renderer = SDL_CreateRenderer(
            window,
            default_driver,
            default_flags);

    if (!renderer) {
        fprintf(stderr, "Error creating SDL renderer.\n");
        return false;
    }

    // change video mode to fullscreen
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    // success
    return true;
}


void draw_grid(void) {
    for (int y = 0; y < window_height; y += 10) {       // rows
        for (int x = 0; x < window_width; x += 10) {    // columns
                color_buffer[(window_width * y) + x] = 0xFF333333;
        }
    }
}


void draw_rect(int x, int y, int width, int height, uint32_t color) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            int current_x = x + i;
            int current_y = y + j;
            color_buffer[(window_width * current_y) + current_x] = color;
        }
    }
}


void render_color_buffer(void) {
    SDL_Rect* rect = NULL;
    int pitch = (int) (window_width * sizeof(uint32_t));
    SDL_UpdateTexture(
            color_buffer_texture,
            rect,
            color_buffer,
            pitch);


    SDL_Rect* srcrect = NULL;
    SDL_Rect* dstrect = NULL;
    SDL_RenderCopy(
            renderer,
            color_buffer_texture,
            srcrect,
            dstrect);

}


void clear_color_buffer(uint32_t color) {
    for (int y = 0; y < window_height; y++) {       // rows
        for (int x = 0; x < window_width; x++) {    // columns
            color_buffer[(window_width * y) + x] = color;
        }
    }
}


void destroy_window(void) {
    free(color_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}


