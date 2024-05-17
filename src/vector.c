# include "vector.h"
# include <math.h>

// rotate around the x-axis
vec3_t vec3_rotate_x(const vec3_t v, float angle) {
  vec3_t rotated_vector = {
      .x = v.x,
      .y = v.y * cos(angle) - v.z * sin(angle),
      .z = v.y * sin(angle) + v.z * cos(angle)
  };
  return rotated_vector;
}

// rotate around the y-axis
vec3_t vec3_rotate_y(const vec3_t v, float angle) {
  vec3_t rotated_vector = {
      .x = v.x * cos(angle) - v.z * sin(angle),
      .y = v.y,
      .z = v.x * sin(angle) + v.z * cos(angle)
  };
  return rotated_vector;
}

// rotate around the z-axis
vec3_t vec3_rotate_z(const vec3_t v, float angle) {
  vec3_t rotated_vector = {
      .x = v.x * cos(angle) - v.y * sin(angle),
      .y = v.x * sin(angle) + v.y * cos(angle),
      .z = v.z
  };
  return rotated_vector;
}