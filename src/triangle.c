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
    int x0, int y0, float u0, float v0,
    int x1, int y1, float u1, float v1,
    int x2, int y2, float u2, float v2,
    uint32_t* texture
) {
  // sort vertices by  y-coordinate ascending (y0 < y1 < y2)
  if (y0 > y1) {
    int_swap(&y0, &y1);
    int_swap(&x0, &x1);
    float_swap(&u0, &u1);
    float_swap(&v0, &v1);
  }
  if (y1 > y2) {
    int_swap(&y1, &y2);
    int_swap(&x1, &x2);
    float_swap(&u1, &u2);
    float_swap(&v1, &v2);
  }
  // y0 may still end up greater after swaps above
  if (y0 > y1) {
    int_swap(&y0, &y1);
    int_swap(&x0, &x1);
    float_swap(&u0, &u1);
    float_swap(&v0, &v1);
  }

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
        draw_pixel(x, y, (x % 2 == 0 && y % 2 == 0) ? 0xFF00FF00 : 0xFF000000);
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
        draw_pixel(x, y, (x % 2 == 0 && y % 2 == 0) ? 0xFFF00FF00 : 0xFF00FFFF);
      }
    }
  }

}




