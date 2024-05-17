#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "array.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"

// Array of triangles that should be rendered frame by frame
triangle_t* triangles_to_render = NULL;

vec3_t camera_position = {.x = 0, .y = 0, .z = -3};

float fov_factor = 640;
bool is_running = false;
int previous_frame_time = 0;

void setup(void) {
  // allocate memory for the color buffer
  color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);
  if (!color_buffer) {
    printf("Error allocating memory for color buffer.\n");
    exit(EXIT_FAILURE);
  }

  // create an SDL texture to display the color buffer
  color_buffer_texture = SDL_CreateTexture(
      renderer,
      SDL_PIXELFORMAT_ARGB8888,
      SDL_TEXTUREACCESS_STREAMING,
      window_width,
      window_height);

  if (!color_buffer_texture) {
    printf("Error creating texture for color buffer.\n");
    free(color_buffer);
    exit(EXIT_FAILURE);
  }

  load_obj_file_data("../assets/f22.obj");
}

void process_input(void) {
  SDL_Event event;
  // process all pending events
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:is_running = false;
        break;

      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE) {
          is_running = false;
        }
        //todo:  add more key handling as needed (define constants or enum)
        break;
    }
  }
}

// orthographic projection - receives a pointer to a 3D vector and projects to a 2D point
vec2_t project(const vec3_t* point) {
  vec2_t projected_point;

  // check for division by zero
  if (point->z != 0) {
    projected_point.x = (fov_factor * point->x) / point->z;
    projected_point.y = (fov_factor * point->y) / point->z;
  } else {
    // handle error case (e.g. set to zero or use default value)
    projected_point.x = 0;
    projected_point.y = 0;
    fprintf(stderr, "Warning: Division by zero in projection.\n");
  }

  return projected_point;
}

void update(void) {
  // maintain consistent frame rate
  int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

  // delay execution if we are running to fast
  if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
    SDL_Delay(time_to_wait);
  }

  // reset the array of triangles to render
  triangles_to_render = NULL;

  // update previous time frame
  previous_frame_time = SDL_GetTicks();

  // update mesh rotation
  const float rotation_speed = 0.01f;
  mesh.rotation.x += rotation_speed;
//  mesh.rotation.y += rotation_speed;
//  mesh.rotation.z += rotation_speed;

  // process each face of the mesh
  int num_faces = array_length(mesh.faces);
  for (int i = 0; i < num_faces; i++) {
    face_t mesh_face = mesh.faces[i];

    // retrieve vertices of the current face
    vec3_t face_vertices[3];
    face_vertices[0] = mesh.vertices[mesh_face.a - 1];
    face_vertices[1] = mesh.vertices[mesh_face.b - 1];
    face_vertices[2] = mesh.vertices[mesh_face.c - 1];

    triangle_t projected_triangle;

    // loop all 3 vertices of this current face and apply transformations
    for (int j = 0; j < 3; j++) {
      vec3_t transformed_vertex = face_vertices[j];

      // apply rotation transformations
      transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
      transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
      transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

      // translate vertex away from camera
      transformed_vertex.z = camera_position.z;

      // project current vertex to 2D space
      vec2_t projected_point = project(&transformed_vertex);

      // scale and center projected point on screen
      projected_point.x += (window_width / 2);
      projected_point.y += (window_height / 2);

      // assign projected point to triangle
      projected_triangle.points[j] = projected_point;
    }

    // add projected triangle to array of triangles to render
    array_push(triangles_to_render, projected_triangle);
  }
}

void render(void) {
  bool dots = true;
  draw_grid(dots);

  // get number of triangles to render
  int num_triangles = array_length(triangles_to_render);
  for (int i = 0; i < num_triangles; i++) {
    // gert current triangle
    const triangle_t* triangle = &triangles_to_render[i];

    // draw vertex points of triangle
    draw_rect(triangle->points[0].x, triangle->points[0].y, 3, 3, 0xffff00ff);
    draw_rect(triangle->points[1].x, triangle->points[1].y, 3, 3, 0xffff00ff);
    draw_rect(triangle->points[2].x, triangle->points[2].y, 3, 3, 0xffff00ff);

    // draw unfilled triangle
    draw_triangle(
        triangle->points[0].x,
        triangle->points[0].y,
        triangle->points[1].x,
        triangle->points[1].y,
        triangle->points[2].x,
        triangle->points[2].y,
        0xFFFF00FF
    );
  }
  // Clear the array of triangles to render for next frame
  array_free(triangles_to_render);

  // render color buffer to screen
  render_color_buffer();

  // clear color buffer with black color
  clear_color_buffer(0xFF000000);

  // present backbuffer on screen
  SDL_RenderPresent(renderer);
}

// free dynamically allocated memory
void free_resources(void) {
  free(color_buffer);
  array_free(mesh.faces);
  array_free(mesh.vertices);
}

int main(int argc, char* argv[]) {

  is_running = initialize_window();

  setup();

  while (is_running) {
    process_input();
    update();
    render();
  }

  cleanup();
  free_resources();

  return 0;
}
