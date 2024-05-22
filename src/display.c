# include "display.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

uint32_t* color_buffer = NULL;
SDL_Texture* color_buffer_texture = NULL;

int window_width = 800;
int window_height = 600;

bool initialize_window(void) {
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
    return false;
  }

  // query fullscreen max width & height of user screen
  SDL_DisplayMode display_mode;
  if (SDL_GetCurrentDisplayMode(0, &display_mode)) {
    fprintf(stderr, "Error getting display mode: %s\n", SDL_GetError());
    cleanup();
    return false;
  };

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
    fprintf(stderr, "Error creating SDL window: %s\n", SDL_GetError());
    cleanup();
    return false;
  }

  // create SDL renderer
  renderer = SDL_CreateRenderer(window, -1, 0);

  if (!renderer) {
    fprintf(stderr, "Error creating SDL renderer: %s\n", SDL_GetError());
    cleanup();
    return false;
  }

  // set window to fullscreen
  if (SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN) != 0) {
    cleanup();
    return false;
  };

  return true;
}

void clear_color_buffer(uint32_t color) {
  uint32_t* buffer_ptr = color_buffer;
  for (int i = 0; i < window_width * window_height; i++) {
    *buffer_ptr++ = color;
  }
}

void render_color_buffer(void) {
  // update texture with color buffer
  if (SDL_UpdateTexture(
      color_buffer_texture,
      NULL,
      color_buffer,
      (int) (window_width * sizeof(uint32_t))) != 0) {
    fprintf(stderr, "Error updating texfture: %s", SDL_GetError());
    return;
  }

  // copy texture to rendering target
  if (SDL_RenderCopy(
      renderer,
      color_buffer_texture,
      NULL,
      NULL) != 0) {
    fprintf(stderr, "Error copying texture to renderer: %s\n", SDL_GetError());
    return;
  }
}

void draw_grid(bool dots) {
  const uint32_t color = 0xFF3A3B3C;
  const int grid_spacing = 25;

  if (dots) {
    // draw dotted grid
    for (int y = 0; y < window_height; y += grid_spacing) {
      for (int x = 0; x < window_width; x += grid_spacing) {
        color_buffer[(window_width * y) + x] = color;
      }
    }
  } else {
    // draw vertical grid lines
    for (int x = 0; x < window_width; x += grid_spacing) {
      for (int y = 0; y < window_height; y++) {
        color_buffer[(window_width * y) + x] = color;
      }
    }

    // draw horizontal grid lines
    for (int y = 0; y < window_height; y += grid_spacing) {
      for (int x = 0; x < window_width; x++) {
        color_buffer[(window_width * y) + x] = color;
      }
    }
  }
}

void draw_pixel(int x, int y, uint32_t color) {
  // keep pixel within screen boundaries
  if (x >= 0 && x < window_width && y >= 0 && y < window_height) {
    color_buffer[(window_width * y) + x] = color;
  }
}

// Bresenham's line algo
void draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
  int delta_x = abs(x1 - x0);
  int delta_y = abs(y1 - y0);

  // determine direction of line
  int sign_x = (x0 < x1) ? 1 : -1;
  int sign_y = (y0 < y1) ? 1 : -1;

  // err term to help decide when to step in x-direction or y-direction
  int err = delta_x - delta_y;
  int err2;

  // draw a pixel at each position and update err term and positions
  while (true) {
    draw_pixel(x0, y0, color);

    if (x0 == x1 && y0 == y1) break;

    err2 = 2 * err;

    if (err2 > -delta_y) {
      err -= delta_y;
      x0 += sign_x;
    }

    if (err2 < delta_x) {
      err += delta_x;
      y0 += sign_y;
    }
  }
}

void draw_rect(int x, int y, int width, int height, uint32_t color) {
  // iterate over width & height of rectangle
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      int current_x = x + i;
      int current_y = y + j;
      draw_pixel(current_x, current_y, color);
    }
  }
}

void cleanup(void) {
  if (renderer) {
    SDL_DestroyRenderer(renderer);
    renderer = NULL;
  }

  if (window) {
    SDL_DestroyWindow(window);
    window = NULL;
  }

  SDL_Quit();
}
