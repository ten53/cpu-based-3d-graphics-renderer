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


void draw_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < window_width && y >= 0 && y < window_height) {
        color_buffer[(window_width * y) + x] = color;
    }
}



void draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
    int delta_x = (x1 - x0);
    int delta_y = (y1 - y0);

    // sometimes delta_y is grater than delta_x
    int side_length = abs(delta_x) >= abs(delta_y) ? abs(delta_x) : abs(delta_y);

    // determine value to increment both x and y each step
    float x_inc = delta_x / (float) side_length;
    float y_inc = delta_y / (float) side_length;

    float current_x = x0;
    float current_y = y0;

    for (int i = 0; i <= side_length; i++) {
        draw_pixel((int) round(current_x), (int) round(current_y), color);
        current_x += x_inc;
        current_y += y_inc;
    }
}


void draw_rect(int x, int y, int width, int height, uint32_t color) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            int current_x = x + i;
            int current_y = y + j;
            draw_pixel(current_x, current_y, color);
        }
    }
}


void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    draw_line(x0, y0, x1, y1, color);
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y2, x0, y0, color);
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
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}


