#ifndef EVAS_ENGINE_H
#define EVAS_ENGINE_H

#include <sys/ipc.h>
#include <sys/shm.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xrender.h>

#ifdef BUILD_ENGINE_XRENDER_XCB
# include <xcb/xcb.h>
# include <xcb/render.h>
# include <xcb/xcb_image.h>
#endif

typedef struct _Ximage_Info      Ximage_Info;
typedef struct _Ximage_Image     Ximage_Image;
typedef struct _Xrender_Surface  Xrender_Surface;

struct _Ximage_Info
{
   struct {
      void        *connection;
      void        *screen;
      unsigned int root;
      unsigned int draw;
      void        *visual;
      void        *fmt32;
      void        *fmt24;
      void        *fmt8;
      void        *fmt4;
      void        *fmt1;
      void        *fmtdef;
   } x11;
   int                              depth;
   int                              pool_mem;
   Eina_List                       *pool;
   unsigned char                    can_do_shm;
   Xrender_Surface                 *mul;
   unsigned char                    mul_r, mul_g, mul_b, mul_a;
   int                              references;
};

struct _Ximage_Image
{
   union {
      struct {
         XImage                 *xim;
         XShmSegmentInfo        *shm_info;
      } xlib;
#ifdef BUILD_ENGINE_XRENDER_XCB
      struct {
         xcb_image_t            *xim;
         xcb_shm_segment_info_t *shm_info;
      } xcb;
#endif
   } x11;
   Ximage_Info                  *xinf;
   int                           width;
   int                           height;
   int                           depth;
   int                           line_bytes;
   unsigned char                *data;
   unsigned char                 available : 1;
};

struct _Xrender_Surface
{
   union {
      struct {
         XRenderPictFormat         *fmt;
         Drawable                   draw;
         Picture                    pic;
      } xlib;
#ifdef BUILD_ENGINE_XRENDER_XCB
      struct {
         xcb_render_pictforminfo_t *fmt;
         xcb_drawable_t             draw;
         xcb_render_picture_t       pic;
      } xcb;
#endif
   } x11;
   Ximage_Info                     *xinf;
   int                              width;
   int                              height;
   int                              depth;
   unsigned char                    alpha : 1;
   unsigned char                    allocated : 1;
   unsigned char                    bordered : 1;
};

/* ximage support calls (ximage vs xshmimage, cache etc.) */
Ximage_Info  *_xr_xlib_image_info_get(Display *disp, Drawable draw, Visual *vis);
void          _xr_xlib_image_info_free(Ximage_Info *xinf);
void          _xr_xlib_image_info_pool_flush(Ximage_Info *xinf, int max_num, int max_mem);
Ximage_Image *_xr_xlib_image_new(Ximage_Info *xinf, int w, int h, int depth);
void          _xr_xlib_image_free(Ximage_Image *xim);
void          _xr_xlib_image_put(Ximage_Image *xim, Drawable draw, int x, int y, int w, int h);

Ximage_Info  *_xr_xcb_image_info_get(xcb_connection_t *conn, xcb_screen_t *screen, xcb_drawable_t draw, xcb_visualtype_t *visual);
void          _xr_xcb_image_info_free(Ximage_Info *xinf);
void          _xr_xcb_image_info_pool_flush(Ximage_Info *xinf, int max_num, int max_mem);
Ximage_Image *_xr_xcb_image_new(Ximage_Info *xinf, int w, int h, int depth);
void          _xr_xcb_image_free(Ximage_Image *xim);
void          _xr_xcb_image_put(Ximage_Image *xim, xcb_drawable_t draw, int x, int y, int w, int h);

/* xrender support calls */
Xrender_Surface *_xr_xlib_render_surface_new(Ximage_Info *xinf, int w, int h, XRenderPictFormat *fmt, int alpha);
Xrender_Surface *_xr_xlib_render_surface_adopt(Ximage_Info *xinf, Drawable draw, int w, int h, int alpha);
Xrender_Surface *_xr_xlib_render_surface_format_adopt(Ximage_Info *xinf, Drawable draw, int w, int h, XRenderPictFormat *fmt, int alpha);
void             _xr_xlib_render_surface_free(Xrender_Surface *rs);
void             _xr_xlib_render_surface_repeat_set(Xrender_Surface *rs, int repeat);
void             _xr_xlib_render_surface_solid_rectangle_set(Xrender_Surface *rs, int r, int g, int b, int a, int x, int y, int w, int h);
void             _xr_xlib_render_surface_argb_pixels_fill(Xrender_Surface *rs, int sw, int sh, void *pixels, int x, int y, int w, int h, int ox, int oy);
void             _xr_xlib_render_surface_rgb_pixels_fill(Xrender_Surface *rs, int sw, int sh, void *pixels, int x, int y, int w, int h, int ox, int oy);
void             _xr_xlib_render_surface_clips_set(Xrender_Surface *rs, RGBA_Draw_Context *dc, int rx, int ry, int rw, int rh);
void             _xr_xlib_render_surface_composite(Xrender_Surface *srs, Xrender_Surface *drs, RGBA_Draw_Context *dc, int sx, int sy, int sw, int sh, int x, int y, int w, int h, int smooth);
void             _xr_xlib_render_surface_copy(Xrender_Surface *srs, Xrender_Surface *drs, int sx, int sy, int x, int y, int w, int h);
void             _xr_xlib_render_surface_rectangle_draw(Xrender_Surface *rs, RGBA_Draw_Context *dc, int x, int y, int w, int h);
void             _xr_xlib_render_surface_line_draw(Xrender_Surface *rs, RGBA_Draw_Context *dc, int x1, int y1, int x2, int y2);
void             _xr_xlib_render_surface_polygon_draw(Xrender_Surface *rs, RGBA_Draw_Context *dc, RGBA_Polygon_Point *points);

Xrender_Surface *_xr_xcb_render_surface_new(Ximage_Info *xinf, int w, int h, xcb_render_pictforminfo_t *fmt, int alpha);
Xrender_Surface *_xr_xcb_render_surface_adopt(Ximage_Info *xinf, xcb_drawable_t draw, int w, int h, int alpha);
Xrender_Surface *_xr_xcb_render_surface_format_adopt(Ximage_Info *xinf, xcb_drawable_t draw, int w, int h, xcb_render_pictforminfo_t *fmt, int alpha);
void             _xr_xcb_render_surface_free(Xrender_Surface *rs);
void             _xr_xcb_render_surface_repeat_set(Xrender_Surface *rs, int repeat);
void             _xr_xcb_render_surface_solid_rectangle_set(Xrender_Surface *rs, int r, int g, int b, int a, int x, int y, int w, int h);
void             _xr_xcb_render_surface_argb_pixels_fill(Xrender_Surface *rs, int sw, int sh, void *pixels, int x, int y, int w, int h, int ox, int oy);
void             _xr_xcb_render_surface_rgb_pixels_fill(Xrender_Surface *rs, int sw, int sh, void *pixels, int x, int y, int w, int h, int ox, int oy);
void             _xr_xcb_render_surface_clips_set(Xrender_Surface *rs, RGBA_Draw_Context *dc, int rx, int ry, int rw, int rh);
void             _xr_xcb_render_surface_composite(Xrender_Surface *srs, Xrender_Surface *drs, RGBA_Draw_Context *dc, int sx, int sy, int sw, int sh, int x, int y, int w, int h, int smooth);
void             _xr_xcb_render_surface_copy(Xrender_Surface *srs, Xrender_Surface *drs, int sx, int sy, int x, int y, int w, int h);
void             _xr_xcb_render_surface_rectangle_draw(Xrender_Surface *rs, RGBA_Draw_Context *dc, int x, int y, int w, int h);
void             _xr_xcb_render_surface_line_draw(Xrender_Surface *rs, RGBA_Draw_Context *dc, int x1, int y1, int x2, int y2);
void             _xr_xcb_render_surface_polygon_draw(Xrender_Surface *rs, RGBA_Draw_Context *dc, RGBA_Polygon_Point *points);


typedef struct _XR_Image XR_Image;

struct _XR_Image
{
   Ximage_Info     *xinf;
   const char      *file;
   const char      *key;
   char            *fkey;
   RGBA_Image      *im;
   void            *data;
   int              w, h;
   Xrender_Surface *surface;
   int              references;
   char            *format;
   const char      *comment;
   Tilebuf         *updates;
   RGBA_Image_Loadopts load_opts;
   struct {
      int           space;
      void         *data;
      unsigned char no_free : 1;
   } cs;
   unsigned char    alpha : 1;
   unsigned char    dirty : 1;
   unsigned char    free_data : 1;
};

XR_Image *_xre_xlib_image_load(Ximage_Info *xinf, const char *file, const char *key, Evas_Image_Load_Opts *lo);
XR_Image *_xre_xlib_image_new_from_data(Ximage_Info *xinf, int w, int h, void *data, int alpha, int cspace);
XR_Image *_xre_xlib_image_new_from_copied_data(Ximage_Info *xinf, int w, int h, void *data, int alpha, int cspace);
XR_Image *_xre_xlib_image_new(Ximage_Info *xinf, int w, int h);
void      _xre_xlib_image_resize(XR_Image *im, int w, int h);
void      _xre_xlib_image_free(XR_Image *im);
void      _xre_xlib_image_region_dirty(XR_Image *im, int x, int y, int w, int h);
void      _xre_xlib_image_dirty(XR_Image *im);
XR_Image *_xre_xlib_image_copy(XR_Image *im);
void     *_xre_xlib_image_data_get(XR_Image *im);
XR_Image *_xre_xlib_image_data_find(void *data);
void      _xre_xlib_image_data_put(XR_Image *im, void *data);
void      _xre_xlib_image_alpha_set(XR_Image *im, int alpha);
int       _xre_xlib_image_alpha_get(XR_Image *im);
void      _xre_xlib_image_border_set(XR_Image *im, int l, int r, int t, int b);
void      _xre_xlib_image_border_get(XR_Image *im, int *l, int *r, int *t, int *b);
void      _xre_xlib_image_surface_gen(XR_Image *im);
void      _xre_xlib_image_cache_set(int size);
int       _xre_xlib_image_cache_get(void);

XR_Image *_xre_xcb_image_load(Ximage_Info *xinf, const char *file, const char *key, Evas_Image_Load_Opts *lo);
XR_Image *_xre_xcb_image_new_from_data(Ximage_Info *xinf, int w, int h, void *data, int alpha, int cspace);
XR_Image *_xre_xcb_image_new_from_copied_data(Ximage_Info *xinf, int w, int h, void *data, int alpha, int cspace);
XR_Image *_xre_xcb_image_new(Ximage_Info *xinf, int w, int h);
void      _xre_xcb_image_resize(XR_Image *im, int w, int h);
void      _xre_xcb_image_free(XR_Image *im);
void      _xre_xcb_image_region_dirty(XR_Image *im, int x, int y, int w, int h);
void      _xre_xcb_image_dirty(XR_Image *im);
XR_Image *_xre_xcb_image_copy(XR_Image *im);
void     *_xre_xcb_image_data_get(XR_Image *im);
XR_Image *_xre_xcb_image_data_find(void *data);
void      _xre_xcb_image_data_put(XR_Image *im, void *data);
void      _xre_xcb_image_alpha_set(XR_Image *im, int alpha);
int       _xre_xcb_image_alpha_get(XR_Image *im);
void      _xre_xcb_image_border_set(XR_Image *im, int l, int r, int t, int b);
void      _xre_xcb_image_border_get(XR_Image *im, int *l, int *r, int *t, int *b);
void      _xre_xcb_image_surface_gen(XR_Image *im);
void      _xre_xcb_image_cache_set(int size);
int       _xre_xcb_image_cache_get(void);

typedef struct _XR_Font_Surface XR_Font_Surface;

struct _XR_Font_Surface
{
   Ximage_Info     *xinf;
   RGBA_Font_Glyph *fg;
   int              w, h;
   Drawable         draw;
   Picture          pic;
};

XR_Font_Surface *_xre_xlib_font_surface_new(Ximage_Info *xinf, RGBA_Font_Glyph *fg);
void             _xre_xlib_font_surface_free(XR_Font_Surface *fs);
void             _xre_xlib_font_surface_draw(Ximage_Info *xinf, RGBA_Image *surface, RGBA_Draw_Context *dc, RGBA_Font_Glyph *fg, int x, int y);

XR_Font_Surface *_xre_xcb_font_surface_new(Ximage_Info *xinf, RGBA_Font_Glyph *fg);
void             _xre_xcb_font_surface_free(XR_Font_Surface *fs);
void             _xre_xcb_font_surface_draw(Ximage_Info *xinf, RGBA_Image *surface, RGBA_Draw_Context *dc, RGBA_Font_Glyph *fg, int x, int y);

typedef struct _XR_Gradient XR_Gradient;

struct _XR_Gradient
{
   Ximage_Info     *xinf;
   Xrender_Surface *surface;
   RGBA_Gradient   *grad;
   unsigned char    changed : 1;
   int              sw, sh;
};

XR_Gradient *_xre_xlib_gradient_new(Ximage_Info *xinf);
void         _xre_xlib_gradient_free(XR_Gradient *gr);
void         _xre_xlib_gradient_color_stop_add(XR_Gradient *gr, int r, int g, int b, int a, int delta);
void         _xre_xlib_gradient_alpha_stop_add(XR_Gradient *gr, int a, int delta);
void         _xre_xlib_gradient_color_data_set(XR_Gradient *gr, void *map, int len, int has_alpha);
void         _xre_xlib_gradient_alpha_data_set(XR_Gradient *gr, void *alpha_map, int len);
void         _xre_xlib_gradient_clear(XR_Gradient *gr);
void         _xre_xlib_gradient_fill_set(XR_Gradient *gr, int x, int y, int w, int h);
void         _xre_xlib_gradient_fill_angle_set(XR_Gradient *gr, double angle);
void         _xre_xlib_gradient_fill_spread_set(XR_Gradient *gr, int spread);
void         _xre_xlib_gradient_angle_set(XR_Gradient *gr, double angle);
void         _xre_xlib_gradient_offset_set(XR_Gradient *gr, float offset);
void         _xre_xlib_gradient_direction_set(XR_Gradient *gr, int direction);
void         _xre_xlib_gradient_type_set(XR_Gradient *gr, char *name, char *params);
void         _xre_xlib_gradient_draw(Xrender_Surface *rs, RGBA_Draw_Context *dc, XR_Gradient *gr, int x, int y, int w, int h);

XR_Gradient *_xre_xcb_gradient_new(Ximage_Info *xinf);
void         _xre_xcb_gradient_free(XR_Gradient *gr);
void         _xre_xcb_gradient_color_stop_add(XR_Gradient *gr, int r, int g, int b, int a, int delta);
void         _xre_xcb_gradient_alpha_stop_add(XR_Gradient *gr, int a, int delta);
void         _xre_xcb_gradient_color_data_set(XR_Gradient *gr, void *map, int len, int has_alpha);
void         _xre_xcb_gradient_alpha_data_set(XR_Gradient *gr, void *alpha_map, int len);
void         _xre_xcb_gradient_clear(XR_Gradient *gr);
void         _xre_xcb_gradient_fill_set(XR_Gradient *gr, int x, int y, int w, int h);
void         _xre_xcb_gradient_fill_angle_set(XR_Gradient *gr, double angle);
void         _xre_xcb_gradient_fill_spread_set(XR_Gradient *gr, int spread);
void         _xre_xcb_gradient_angle_set(XR_Gradient *gr, double angle);
void         _xre_xcb_gradient_offset_set(XR_Gradient *gr, float offset);
void         _xre_xcb_gradient_direction_set(XR_Gradient *gr, int direction);
void         _xre_xcb_gradient_type_set(XR_Gradient *gr, char *name, char *params);
void         _xre_xcb_gradient_draw(Xrender_Surface *rs, RGBA_Draw_Context *dc, XR_Gradient *gr, int x, int y, int w, int h);

#endif
