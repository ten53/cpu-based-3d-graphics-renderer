#ifndef SORTING_H
#define SORTING_H

#include "triangle.h"

int compare_triangle_depth(const void* a, const void* b);

void quicksort(triangle_t* triangles, int low, int high, int (* compare)(const void*, const void*));

int partition(triangle_t* triangles, int low, int high, int (* compare)(const void*, const void*));

#endif
