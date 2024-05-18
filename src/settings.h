#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdlib.h>

// render modes
typedef enum {
  RENDER_MODE_WIRE_FRAME,
  RENDER_MODE_FILLED,
  RENDER_MODE_WIRE_FRAME_FILLED
} render_mode_t;

// culling modes
typedef enum {
  CULLING_ENABLED,
  CULLING_DISABLED
} culling_mode_t;

typedef struct {
  render_mode_t render_mode;
  culling_mode_t culling_mode;
  uint32_t vertex_color;
  uint32_t fill_color;
  uint32_t frame_color;
} render_settings_t;

#endif
