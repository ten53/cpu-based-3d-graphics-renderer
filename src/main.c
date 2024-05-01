#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "display.h"
#include "vector.h"

#define N_POINTS (9*9*9)

vec3_t cube_points[N_POINTS];// 9*9*9 cube

bool is_running = false;

void setup(void) {
  // Allocate the required memory in bytes to hold the color buffer
  color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);

  if (!color_buffer) {
    printf("Error allocating memory for color buffer.\n");
  }

  // Creating a SDL texture that is used to display the color buffer
  color_buffer_texture = SDL_CreateTexture(
      renderer,
      SDL_PIXELFORMAT_ARGB8888,
      SDL_TEXTUREACCESS_STREAMING,
      window_width,
      window_height);

  if (!color_buffer_texture) {
    printf("Error creating texture for color buffer.\n");
  }

  // destroy texture ??

  int point_count = 0;
  // load array of vectors (from -1 to 1 in 9*9*9 cube)
  // increment 0.25 to get 9 vectors for length 2 (-1 to 1)
  for (float x = -1; x <= 1; x += 0.25) {
    for (float y = -1; y <= 1; y += 0.25) {
      for (float z = -1; z <= 1; z += 0.25) {
        vec3_t new_point = {.x = x, .y = y, .z = z};
        cube_points[point_count++] = new_point;
      }
    }
  }
}

void process_input(void) {
  SDL_Event event;
  SDL_PollEvent(&event);

  switch (event.type) {
    case SDL_QUIT: //
      is_running = false;
      break;
    case SDL_KEYDOWN:
      if (event.key.keysym.sym == SDLK_ESCAPE) {
        is_running = false;
      }
      break;
  }
}

void update(void) {
  // todo
}

void render(void) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1); // RGBA
  SDL_RenderClear(renderer);
  draw_grid();

  render_color_buffer();
  clear_color_buffer(0xFF000000);

  SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {

  is_running = initialize_window();

  setup();

  while (is_running) {
    process_input();
    update();
    render();
  }

  destroy_window();

  return 0;
}
