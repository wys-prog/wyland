#ifndef ___WYLAND_INTERFACE_H___
#define ___WYLAND_INTERFACE_H___

#include "../wyland-runtime/wylrt.h"

/* USES BIG ENDIAN */
typedef struct {
  wuchar a, r, g, b;
} wcolor;

/* USES BIG ENDIAN */
typedef struct {
  wuint x, y;
} wvec2;

/* USES BIG ENDIAN */
typedef struct {
  wvec2 position, size;
} wwin;

/* USES BIG ENDIAN */
typedef struct {
  wvec2  position;
  wcolor color;
} wpixel;

/* USES BIG ENDIAN */
typedef struct {
  wwin *windows;
  wpixel *pixels;
} wframe;

typedef struct {
  wuint *pixels;
  wvec2  size;
} wpacked_frame;

wuint encode_color(const wcolor *color) {
  return (color->a << 24) | (color->r << 16) | (color->g << 8) | color->b;
}

#endif // ___WYLAND_INTERFACE_H___ ?