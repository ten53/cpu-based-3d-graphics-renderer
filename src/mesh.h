#ifndef MESH_H #define MESH_H

#include "vector.h"
#include "triangle.h"

#define N_CUBE_VERTICES 8
vec3_t cube_vertices[N_CUBE_VERTICES];

#define N_CUBE_FACES (6*2) // 6 cube faces, 2 triangles per face
face_t cube_faces[N_CUBE_FACES];

extern vec3_t cube_vertices[N_CUBE_VERTICES];
extern face_t cube_faces[N_CUBE_FACES];

typedef struct {
  vec3_t* vertices; // dynamic array of vertices
  face_t* faces; // dynamic array of faces
  vec3_t rotation; // rotation with x, y, and z values
  vec3_t scale; // scale x,y,z values
  vec3_t translation; // translate x,y,z values
} mesh_t;

extern mesh_t mesh;

void load_cube_mesh_data(void);

void load_obj_file_data(const char* filename);

#endif
