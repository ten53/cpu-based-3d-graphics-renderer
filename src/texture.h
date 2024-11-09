#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h>
#include "upng.h"

typedef struct {
    float u;
    float v;
} tex2_t;

extern int texture_width;
extern int texture_height;

// extern const uint8_t REDBRICK_TEXTURE[];
extern uint32_t* mesh_texture;   // pointer to array of texture values
extern upng_t* png_texture;

void load_png_texture_data(char* filename);

#endif
