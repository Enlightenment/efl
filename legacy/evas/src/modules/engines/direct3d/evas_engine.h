#ifndef __EVAS_ENGINE_H__
#define __EVAS_ENGINE_H__


#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EVAS_INLINE_ARRAY_H  // We dont need that and it is buggy

#include "evas_common.h"
#include "evas_private.h"

#ifdef __cplusplus
}
#endif

//#define ENABLE_LOG_PRINTF
#ifdef ENABLE_LOG_PRINTF
#define Log(str, ...) printf("D3D "str"\n", __VA_ARGS__)
#else
#define Log(str, ...)
#endif

typedef void * Direct3DDeviceHandler;
typedef void * Direct3DImageHandler;
typedef void * Direct3DFontGlyphHandler;

#ifdef __cplusplus
extern "C" {
#endif

// Main engine functions

Direct3DDeviceHandler evas_direct3d_init(HWND window, int depth, int fullscreen);
void         evas_direct3d_free(Direct3DDeviceHandler d3d);
void         evas_direct3d_render_all(Direct3DDeviceHandler d3d);
void         evas_direct3d_resize(Direct3DDeviceHandler d3d, int width, int height);
void         evas_direct3d_set_fullscreen(Direct3DDeviceHandler d3d,
   int width, int height, int fullscreen);
void         evas_direct3d_set_layered(Direct3DDeviceHandler d3d, int layered,
   int mask_width, int mask_height, unsigned char *mask);


// Context manipulations

void         evas_direct3d_context_color_set(Direct3DDeviceHandler d3d, int r, int g, int b, int a);
void         evas_direct3d_context_set_multiplier(Direct3DDeviceHandler d3d, int r, int g, int b, int a);

// Simple objects

void         evas_direct3d_line_draw(Direct3DDeviceHandler d3d, int x1, int y1, int x2, int y2);
void         evas_direct3d_rectangle_draw(Direct3DDeviceHandler d3d, int x, int y, int w, int h);

// Images

Direct3DImageHandler evas_direct3d_image_load(Direct3DDeviceHandler d3d,
   const char *file, const char *key, int *error, Evas_Image_Load_Opts *lo);
Direct3DImageHandler evas_direct3d_image_new_from_data(Direct3DDeviceHandler d3d,
   int w, int h, DWORD *image_data, int alpha, int cspace);
Direct3DImageHandler evas_direct3d_image_new_from_copied_data(Direct3DDeviceHandler d3d,
   int w, int h, DWORD *image_data, int alpha, int cspace);
void evas_direct3d_image_free(Direct3DDeviceHandler d3d, Direct3DImageHandler image);
void evas_direct3d_image_data_put(Direct3DDeviceHandler d3d, Direct3DImageHandler image,
   DWORD *image_data);
void evas_direct3d_image_data_get(Direct3DDeviceHandler d3d, Direct3DImageHandler image,
   int to_write, DATA32 **image_data);
void evas_direct3d_image_draw(Direct3DDeviceHandler d3d, Direct3DImageHandler image,
   int src_x, int src_y, int src_w, int src_h,
   int dst_x, int dst_y, int dst_w, int dst_h, int smooth);
void evas_direct3d_image_size_get(Direct3DImageHandler image, int *w, int *h);
void evas_direct3d_image_border_set(Direct3DDeviceHandler d3d, Direct3DImageHandler image,
   int l, int r, int t, int b);
void evas_direct3d_image_border_get(Direct3DDeviceHandler d3d, Direct3DImageHandler image,
   int *l, int *r, int *t, int *b);

// Fonts

Direct3DFontGlyphHandler evas_direct3d_font_texture_new(Direct3DDeviceHandler d3d,
   RGBA_Font_Glyph *fg);
void evas_direct3d_font_texture_free(Direct3DFontGlyphHandler ft);
void evas_direct3d_font_texture_draw(Direct3DDeviceHandler d3d, void *dest, void *context,
   RGBA_Font_Glyph *fg, int x, int y);
void evas_direct3d_select_or_create_font(Direct3DDeviceHandler d3d, void *font);
void evas_direct3d_font_free(Direct3DDeviceHandler d3d, void *font);

#ifdef __cplusplus
}
#endif


#endif /* __EVAS_ENGINE_H__ */
