#include "sorting.h"

// Comparison function for quicksort
int compare_triangle_depth(const void* a, const void* b) {
  triangle_t* triangle_a = (triangle_t*) a;
  triangle_t* triangle_b = (triangle_t*) b;
  if (triangle_a->avg_depth < triangle_b->avg_depth) return 1;
  if (triangle_a->avg_depth > triangle_b->avg_depth) return -1;
  return 0;
}

// Quicksort function
void quicksort(triangle_t* triangles, int low, int high, int (* compare)(const void*, const void*)) {
  if (low < high) {
    int pivot_index = partition(triangles, low, high, compare);
    quicksort(triangles, low, pivot_index - 1, compare);
    quicksort(triangles, pivot_index + 1, high, compare);
  }
}

// Partition function for quicksort
int partition(triangle_t* triangles, int low, int high, int (* compare)(const void*, const void*)) {
  triangle_t pivot = triangles[high];
  int i = low - 1;
  for (int j = low; j < high; j++) {
    if (compare(&triangles[j], &pivot) < 0) {
      i++;
      triangle_t temp = triangles[i];
      triangles[i] = triangles[j];
      triangles[j] = temp;
    }
  }
  triangle_t temp = triangles[i + 1];
  triangles[i + 1] = triangles[high];
  triangles[high] = temp;
  return i + 1;
}