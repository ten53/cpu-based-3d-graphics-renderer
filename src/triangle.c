#include "triangle.h"
#include "display.h"
#include "swap.h"

// draw a triangle
void draw_triangle(int x0,
                   int y0,
                   int x1,
                   int y1,
                   int x2,
                   int y2,
                   uint32_t color) {
  // draw three sides of triangle
  draw_line(x0, y0, x1, y1, color);
  draw_line(x1, y1, x2, y2, color);
  draw_line(x2, y2, x0, y0, color);
}

// draw a filled a triangle with a flat bottom
//
//        (x0,y0)
//          / \
//         /   \
//        /     \
//       /       \
//      /         \
//  (x1,y1)------(x2,y2)
//

void fill_flat_bottom_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
  // find two slopes (two triangle legs)
  // change in delta x over delta y for left leg (inverse slope)
  float inv_slope_1 = 0;
  float inv_slope_2 = 0;
  if (y1 - y0 != 0) inv_slope_1 = (float) (x1 - x0) / (float) (y1 - y0);
  // change in delta x over delta y for right leg (inverse slope)
  if (y2 - y0) inv_slope_2 = (float) (x2 - x0) / (float) (y2 - y0);

  // start start_x and x_end from the top vertex(x0, y0)
  float x_start = (float) x0;
  float x_end = (float) x0;

  // loop scan lines from top to bottom
  for (int y = y0; y <= y2; y++) {
    draw_line((int) x_start, y, (int) x_end, y, color);
    x_start += inv_slope_1;
    x_end += inv_slope_2;
  }
}

// draw a filled a triangle with a flat top
//
//  (x0,y0)------(x1,y1)
//      \         /
//       \       /
//        \     /
//         \   /
//          \ /
//        (x2,y2)
//

void fill_flat_top_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
  // find two slopes (two triangle legs)
  // change in delta x over delta y for left leg (inverse slope)
  float inv_slope_1 = 0;
  float inv_slope_2 = 0;
  if (y2 - y0 != 0) inv_slope_1 = (float) (x2 - x0) / (float) (y2 - y0);
  // change in delta x over delta y for right leg (inverse slope)
  if (y2 - y1 != 0) inv_slope_2 = (float) (x2 - x1) / (float) (y2 - y1);

  // start start_x and x_end from the bottom vertex(x2, y2)
  float x_start = (float) x2;
  float x_end = (float) x2;

  // loop scan lines from bottom to top
  for (int y = y2; y >= y0; y--) {
    draw_line((int) x_start, y, (int) x_end, y, color);
    x_start -= inv_slope_1;
    x_end -= inv_slope_2;
  }
}

// draw a filled triangle with the flat-top/flat-bottom method
// split the original triangle in two, half flat-bottom and half flat-top
//
//          (x0,y0)
//            / \
//           /   \
//          /     \
//         /       \
//        /         \
//   (x1,y1)------(Mx,My)
//       \_           \
//          \_         \
//             \_       \
//                \_     \
//                   \    \
//                     \_  \
//                        \_\
//                           \
//                         (x2,y2)
//

void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
  // sort vertices by y-coordinate ascending (y0 < y1 < y2)
  if (y0 > y1) {
    int_swap(&y0, &y1);
    int_swap(&x0, &x1);
  }
  if (y1 > y2) {
    int_swap(&y1, &y2);
    int_swap(&x1, &x2);
  }
  // y0 may still end up greater after swaps above
  if (y0 > y1) {
    int_swap(&y0, &y1);
    int_swap(&x0, &x1);
  }

  if (y1 == y2) {
    // just draw the flat-bottom triangle
    fill_flat_bottom_triangle(x0, y0, x1, y1, x2, y2, color);
  } else if (y0 == y1) {
    // just draw the flat-top triangle
    fill_flat_top_triangle(x0, y0, x1, y1, x2, y2, color);
  } else {
    // calculate the new vertex (Mx, My) using triangle similarity
    int My = y1;
    // sorting and conditionals ensure that y2-0 != 0
    int Mx = (int) ((float) ((x2 - x0) * (y1 - y0)) / (float) (y2 - y0)) + x0;

    // draw flat-bottom triangle
    fill_flat_bottom_triangle(x0, y0, x1, y1, Mx, My, color);

    // draw flat-top triangle
    fill_flat_top_triangle(x1, y1, Mx, My, x2, y2, color);
  }

}

// return barycentric weights alpha, beta, and gamma for point p
//
//         (B)
//         /|\
//        / | \
//       /  |  \
//      /  (P)  \
//     /  /   \  \
//    / /       \ \
//   //           \\
//  (A)------------(C)
//
vec3_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p) {
  // Find the vectors between the vertices ABC and point p
  vec2_t ac = vec2_sub(c, a);
  vec2_t ab = vec2_sub(b, a);
  vec2_t ap = vec2_sub(p, a);
  vec2_t pc = vec2_sub(c, p);
  vec2_t pb = vec2_sub(b, p);

  // compute area of full parallelogram/triangle ABC using 2D cross product
  float area_parallelogram_abc = (ac.x * ab.y - ac.y * ab.x); // || AC x AB ||

  // alpha is area of small parallelogram/triangle PBC divided by area of full parallelogram/triangle ABC
  float alpha = (pc.x * pb.y - pc.y * pb.x) / area_parallelogram_abc;

  // beta is area of small parallelogram/triangle APC divided by area of full parallelogram/triangle ABC
  float beta = (ac.x * ap.y - ac.y * ap.x) / area_parallelogram_abc;
  // weight gamma is easily found since barycentric coordinates always add up to 1.0
  float gamma = 1 - alpha - beta;

  vec3_t weights = {alpha, beta, gamma};
  return weights;
}

// draw textured pixel at position x and y using interpolation
void draw_texel(
    int x, int y, uint32_t* texture,
    vec4_t point_a, vec4_t point_b, vec4_t point_c,
    float u0, float v0, float u1, float v1, float u2, float v2
) {
  vec2_t p = {x, y};
  vec2_t a = vec2_from_vec4(point_a);
  vec2_t b = vec2_from_vec4(point_b);
  vec2_t c = vec2_from_vec4(point_c);

  vec3_t weights = barycentric_weights(a, b, c, p);

  float alpha = weights.x;
  float beta = weights.y;
  float gamma = weights.z;

  float interpolated_u;
  float interpolated_v;
  float interpolated_reciprocal_w; // 1/w


  // perform the interpolation of all U/w and V/w values using barycentric weights and a factor of 1/w
  interpolated_u = (u0 / point_a.w) * alpha + (u1 / point_b.w) * beta + (u2 / point_c.w) * gamma;
  interpolated_v = (v0 / point_a.w) * alpha + (v1 / point_b.w) * beta + (v2 / point_c.w) * gamma;

  // interpolate value of 1/w for the current pixel
  interpolated_reciprocal_w = (1 / point_a.w) * alpha + (1 / point_b.w) * beta + (1 / point_c.w) * gamma;

  // divide back both interpolated values by 1/w
  interpolated_u /= interpolated_reciprocal_w;
  interpolated_v /= interpolated_reciprocal_w;

  // Map the UV coordinate to the full texture width and height
  int tex_x = abs((int) (interpolated_u * texture_width));
  int tex_y = abs((int) (interpolated_v * texture_height));

  draw_pixel(x, y, texture[(texture_width * tex_y) + tex_x]);
}

// draw textured triangle based on texture array of colors
// split original triangle in two, half flat-bottom & half flat-top
//
//        v0
//        /\
//       /  \
//      /    \
//     /      \
//   v1--------\
//     \_       \
//        \_     \
//           \_   \
//              \_ \
//                 \\
//                   \
//                    v2
//
void draw_textured_triangle(
    int x0, int y0, float z0, float w0, float u0, float v0,
    int x1, int y1, float z1, float w1, float u1, float v1,
    int x2, int y2, float z2, float w2, float u2, float v2,
    uint32_t* texture
) {
  // sort vertices by  y-coordinate ascending (y0 < y1 < y2)
  if (y0 > y1) {
    int_swap(&y0, &y1);
    int_swap(&x0, &x1);
    float_swap(&z0, &z1);
    float_swap(&w0, &w1);
    float_swap(&u0, &u1);
    float_swap(&v0, &v1);
  }
  if (y1 > y2) {
    int_swap(&y1, &y2);
    int_swap(&x1, &x2);
    float_swap(&z1, &z2);
    float_swap(&w1, &w2);
    float_swap(&u1, &u2);
    float_swap(&v1, &v2);
  }
  // y0 may still end up greater after swaps above
  if (y0 > y1) {
    int_swap(&y0, &y1);
    int_swap(&x0, &x1);
    float_swap(&z0, &z1);
    float_swap(&w0, &w1);
    float_swap(&u0, &u1);
    float_swap(&v0, &v1);
  }

  // create vector points after vertices have been sorted
  vec4_t point_a = {x0, y0, z0, w0};
  vec4_t point_b = {x1, y1, z1, w1};
  vec4_t point_c = {x2, y2, z2, w2};

  // render upper part of triangle (flat-bottom)
  float inv_slope_1 = 0;
  float inv_slope_2 = 0;

  if (y1 - y0 != 0) inv_slope_1 = (float) (x1 - x0) / (float) (abs)(y1 - y0);
  if (y2 - y0 != 0) inv_slope_2 = (float) (x2 - x0) / (float) (abs)(y2 - y0);

  if (y1 - y0 != 0) {
    for (int y = y0; y <= y1; y++) {
      int x_start = x1 + (y - y1) * inv_slope_1;
      int x_end = x0 + (y - y0) * inv_slope_2;

      if (x_end < x_start) {
        // swap if x_start is to the end of x_end
        int_swap(&x_start, &x_end);
      }
      for (int x = x_start; x < x_end; x++) {
        // draw pixel with color from texture
        draw_texel(x, y, texture, point_a, point_b, point_c, u0, v0, u1, v1, u2, v2);
      }
    }
  }

  // render bottom part of triangle (flat top)
  inv_slope_1 = 0;
  inv_slope_2 = 0;

  if (y2 - y1 != 0) inv_slope_1 = (float) (x2 - x1) / (float) (abs)(y2 - y1);
  if (y2 - y0 != 0) inv_slope_2 = (float) (x2 - x0) / (float) (abs)(y2 - y0);

  if (y2 - y1 != 0) {
    for (int y = y1; y <= y2; y++) {
      int x_start = x1 + (y - y1) * inv_slope_1;
      int x_end = x0 + (y - y0) * inv_slope_2;

      if (x_end < x_start) {
        // swap if x_start is to the end of x_end
        int_swap(&x_start, &x_end);
      }
      for (int x = x_start; x < x_end; x++) {
        // draw pixel with color from texture
        draw_texel(x, y, texture, point_a, point_b, point_c, u0, v0, u1, v1, u2, v2);
      }
    }
  }

}




