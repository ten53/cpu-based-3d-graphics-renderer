#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "array.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "settings.h"
#include "sorting.h"
#include "matrix.h"

// Array of triangles that should be rendered frame by frame
triangle_t* triangles_to_render = NULL;
vec3_t camera_position = {0, 0, 0};
float fov_factor = 640;
bool is_running = false;
int previous_frame_time = 0;
render_settings_t render_settings;

void setup(void) {
  // init render settings
  render_settings.render_mode = RENDER_MODE_WIRE_FRAME;
  render_settings.culling_mode = CULLING_ENABLED;
  render_settings.vertex_color = 0xFFFFFFFF;
  render_settings.fill_color = 0xFFAF3895;
  render_settings.frame_color = 0xFFFFFFFF;

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

  // load vertex and face values for mesh data structure
  load_cube_mesh_data();
//    load_obj_file_data("../assets/cube.obj");
}

void process_input(void) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:is_running = false;
        break;
      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE) {
          is_running = false;
        }
        if (event.key.keysym.sym == SDLK_1) {
          render_settings.render_mode = RENDER_MODE_WIRE_FRAME;
          render_settings.vertex_color = 0xFFFF0000;
        }
        if (event.key.keysym.sym == SDLK_2) {
          render_settings.render_mode = RENDER_MODE_WIRE_FRAME;
          render_settings.vertex_color = render_settings.frame_color;
        }
        if (event.key.keysym.sym == SDLK_3) {
          render_settings.render_mode = RENDER_MODE_FILLED;
          render_settings.vertex_color = render_settings.fill_color;
        }
        if (event.key.keysym.sym == SDLK_4) {
          render_settings.render_mode = RENDER_MODE_WIRE_FRAME_FILLED;
          render_settings.vertex_color = render_settings.frame_color;
        }
        if (event.key.keysym.sym == SDLK_c) {
          render_settings.culling_mode = CULLING_ENABLED;
        }
        if (event.key.keysym.sym == SDLK_d) {
          render_settings.culling_mode = CULLING_DISABLED;
        }
        break;
    }
  }
}

// orthographic projection - receives a pointer to a 3D vector and projects to a 2D point
vec2_t project(vec3_t* point) {
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
  int elapsed_time = (int) SDL_GetTicks() - previous_frame_time;
  int time_to_wait = FRAME_TARGET_TIME - elapsed_time;

  // delay execution if running to fast
  if (time_to_wait > 0) {
    SDL_Delay(time_to_wait);
  }

  // reset the array of triangles to render
  triangles_to_render = NULL;

  // update previous time frame
  previous_frame_time = (int) SDL_GetTicks();

  // update mesh rotation, translation, & scale values per animation frame
  mesh.rotation.x += 0.02f;
  mesh.rotation.y += 0.02f;
  mesh.rotation.z += 0.02f;
//  mesh.scale.x += 0.002f;
//  mesh.scale.y += 0.001f;
//  mesh.translation.x += 0.01f;
  // translate vertex away from camera (left-handed coordinate system)
  mesh.translation.z = 5.0f;

  // create a scale, rotation, and translation matrices
  mat4_t scale_matrix = mat4_create_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
  mat4_t rotation_matrix_x = mat_create_rotation_x(mesh.rotation.x);
  mat4_t rotation_matrix_y = mat_create_rotation_y(mesh.rotation.y);
  mat4_t rotation_matrix_z = mat_create_rotation_z(mesh.rotation.z);
  mat4_t translation_matrix = mat4_create_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);

  // create a world matrix combining scale, rotation, and translation matrices
  // order matters: first scale, then rotate, then translate.
  mat4_t world_matrix = mat4_identity();
  world_matrix = mat4_multiply_mat4(scale_matrix, world_matrix);
  world_matrix = mat4_multiply_mat4(rotation_matrix_x, world_matrix);
  world_matrix = mat4_multiply_mat4(rotation_matrix_y, world_matrix);
  world_matrix = mat4_multiply_mat4(rotation_matrix_z, world_matrix);
  world_matrix = mat4_multiply_mat4(translation_matrix, world_matrix);

  // process each face of the mesh
  int num_faces = array_length(mesh.faces);
  for (int i = 0; i < num_faces; i++) {
    face_t mesh_face = mesh.faces[i];

    // retrieve vertices of the current face
    vec3_t face_vertices[3];
    face_vertices[0] = mesh.vertices[mesh_face.a - 1];
    face_vertices[1] = mesh.vertices[mesh_face.b - 1];
    face_vertices[2] = mesh.vertices[mesh_face.c - 1];

    vec4_t transformed_vertices[3];

    //  apply transformations to each vertex of the face
    for (int j = 0; j < 3; j++) {
      vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

    // multiply world matrix by original vector
    transformed_vertex = mat4_multiply_vec4(world_matrix, transformed_vertex);

      // store transformed vertex
      transformed_vertices[j] = transformed_vertex;
    }

    if (render_settings.culling_mode == CULLING_ENABLED) {
      //check backface culling
      vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]);    /*   A   */
      vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]);    /*  / \  */
      vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]);    /* C---B */

      // vector subtraction B-A and C-A
      vec3_t vector_ab = vec3_sub(vector_b, vector_a);
      vec3_t vector_ac = vec3_sub(vector_c, vector_a);
      vec3_normalize(&vector_ab);
      vec3_normalize(&vector_ac);

      // compute the face normal (using cross product to find perpendicular)
      vec3_t normal = vec3_cross(vector_ab, vector_ac); // left-handed coordinate system
      vec3_normalize(&normal);

      // find vector between a point in the triangle and the camera origin
      vec3_t camera_ray = vec3_sub(camera_position, vector_a);

      // compute the alignment of camera ray and face normal (dot product)
      float face_camera_alignment = vec3_dot(normal, camera_ray);

      // bypass triangles that are looking away from the camera
      if (face_camera_alignment < 0.0f) {
        continue;
      }
    }

    // project triangle vertices to 2D space
    vec2_t projected_points[3];
    for (int j = 0; j < 3; j++) {
      projected_points[j] = project((const vec3_t*) &transformed_vertices[j]);

      // scale and center projected point on screen
      projected_points[j].x += ((float) window_width / 2.0f);
      projected_points[j].y += ((float) window_height / 2.0f);
    }

    // calculate the average depth for each face based on the vertices z-value after transformation
    float avg_depth = (float) (transformed_vertices[0].z + transformed_vertices[1].z + transformed_vertices[2].z) / 3;

    // assign projected points to triangle
    triangle_t projected_triangle = {
        .points = {
            {projected_points[0].x, projected_points[0].y},
            {projected_points[1].x, projected_points[1].y},
            {projected_points[2].x, projected_points[2].y},
        },
        .color = mesh_face.color,
        .avg_depth = avg_depth
    };

    // add projected triangle to array of triangles to render
    array_push(triangles_to_render, projected_triangle);
  }

  // Sort the triangles to render by their avg_depth
  quicksort(triangles_to_render, 0, array_length(triangles_to_render) - 1, compare_triangle_depth);
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
    draw_rect((int) triangle->points[0].x, (int) triangle->points[0].y, 3, 3, render_settings.vertex_color);
    draw_rect((int) triangle->points[1].x, (int) triangle->points[1].y, 3, 3, render_settings.vertex_color);
    draw_rect((int) triangle->points[2].x, (int) triangle->points[2].y, 3, 3, render_settings.vertex_color);

    if (render_settings.render_mode == RENDER_MODE_FILLED
        || render_settings.render_mode == RENDER_MODE_WIRE_FRAME_FILLED) {
      // draw filled triangle
      draw_filled_triangle(
          (int) triangle->points[0].x,
          (int) triangle->points[0].y,
          (int) triangle->points[1].x,
          (int) triangle->points[1].y,
          (int) triangle->points[2].x,
          (int) triangle->points[2].y,
          triangle->color
//          render_settings.fill_color
      );
    }

    if (render_settings.render_mode == RENDER_MODE_WIRE_FRAME
        || render_settings.render_mode == RENDER_MODE_WIRE_FRAME_FILLED) {
      // draw unfilled triangle (wireframe)
      draw_triangle(
          (int) triangle->points[0].x,
          (int) triangle->points[0].y,
          (int) triangle->points[1].x,
          (int) triangle->points[1].y,
          (int) triangle->points[2].x,
          (int) triangle->points[2].y,
          render_settings.frame_color
      );
    }

  }

  // Clear the array of triangles to render for next frame
  array_free(triangles_to_render);

  // render color buffer to screen
  render_color_buffer();

  // clear color buffer with black color
  clear_color_buffer(0xFF000000);

  // present back buffer on screen
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
