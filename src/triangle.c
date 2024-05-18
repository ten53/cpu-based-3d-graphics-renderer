#include "triangle.h"
#include "display.h"

void int_swap(int* a, int* b) {
  int tmp = *a;
  *a = *b;
  *b = tmp;
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
  float inv_slope1 = (float) (x1 - x0) / (float) (y1 - y0);
  // change in delta x over delta y for right leg (inverse slope)
  float inv_slope2 = (float) (x2 - x0) / (float) (y2 - y0);

  // start start_x and x_end from the top vertex(x0, y0)
  float x_start = (float) x0;
  float x_end = (float) x0;

  // loop scan lines from top to bottom
  for (int y = y0; y <= y2; y++) {
    draw_line((int) x_start, y, (int) x_end, y, color);
    x_start += inv_slope1;
    x_end += inv_slope2;
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
  float inv_slope1 = (float) (x2 - x0) / (float) (y2 - y0);
  // change in delta x over delta y for right leg (inverse slope)
  float inv_slope2 = (float) (x2 - x1) / (float) (y2 - y1);

  // start start_x and x_end from the bottom vertex(x2, y2)
  float x_start = (float) x2;
  float x_end = (float) x2;

  // loop scan lines from bottom to top
  for (int y = y2; y >= y0; y--) {
    draw_line((int) x_start, y, (int) x_end, y, color);
    x_start -= inv_slope1;
    x_end -= inv_slope2;
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
  // y0 may still end up greater after the above swaps
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
    int Mx = (int) ((float) ((x2 - x0) * (y1 - y0)) / (float) (y2 - y0)) + x0;

    // draw flat-bottom triangle
    fill_flat_bottom_triangle(x0, y0, x1, y1, Mx, My, color);

    // draw flat-top triangle
    fill_flat_top_triangle(x1, y1, Mx, My, x2, y2, color);
  }

}





