#ifndef EVAS_ENGINE_H
#define EVAS_ENGINE_H

#include <sys/ipc.h>
#include <sys/shm.h>

#include <xcb/xcb.h>
#include <xcb/render.h>
#include <xcb/xcb_image.h>

typedef struct _Xcb_Image_Info      Xcb_Image_Info;
typedef struct _Xcb_Image_Image     Xcb_Image_Image;
typedef struct _Xcb_Render_Surface  Xcb_Render_Surface;

struct _Xcb_Image_Info
{
   xcb_connection_t          *conn;
   xcb_drawable_t             root;
   xcb_drawable_t             draw;
   int                        depth;
   xcb_visualid_t             vis;
   int                        pool_mem;
   Evas_List                 *pool;
   unsigned char              can_do_shm;
   xcb_render_pictforminfo_t *fmt32;
   xcb_render_pictforminfo_t *fmt24;
   xcb_render_pictforminfo_t *fmt8;
   xcb_render_pictforminfo_t *fmt4;
   xcb_render_pictforminfo_t *fmt1;
   unsigned char              mul_r;
   unsigned char              mul_g;
   unsigned char              mul_b;
   unsigned char              mul_a;
   Xcb_Render_Surface        *mul;
   int                        references;
};

struct _Xcb_Image_Image
{
   Xcb_Image_Info         *xcbinf;
   xcb_image_t            *xcbim;
   xcb_shm_segment_info_t *shm_info;
   int                     w;
   int                     h;
   int                     depth;
   int                     line_bytes;
   unsigned char          *data;
   unsigned char           available : 1;
};

struct _Xcb_Render_Surface
{
   Xcb_Image_Info            *xcbinf;
   int                        w;
   int                        h;
   int                        depth;
   xcb_render_pictforminfo_t *fmt;
   xcb_drawable_t             draw;
   xcb_render_picture_t       pic;
   unsigned char              alpha : 1;
   unsigned char              allocated : 1;
   unsigned char              bordered : 1;
};

/* ximage support calls (ximage vs xshmimage, cache etc.) */
Xcb_Image_Info  *_xr_image_info_get(xcb_connection_t *conn, xcb_drawable_t draw, xcb_visualid_t vis);
void             _xr_image_info_free(Xcb_Image_Info *xcbinf);
void             _xr_image_info_pool_flush(Xcb_Image_Info *xcbinf, int max_num, int max_mem);
Xcb_Image_Image *_xr_image_new(Xcb_Image_Info *xcbinf, int w, int h, int depth);
void             _xr_image_free(Xcb_Image_Image *xim);
void             _xr_image_put(Xcb_Image_Image *xim, xcb_drawable_t draw, int x, int y, int w, int h);

/* xrender support calls */
Xcb_Render_Surface *_xr_render_surface_new(Xcb_Image_Info *xcbinf, int w, int h, xcb_render_pictforminfo_t *fmt, int alpha);
Xcb_Render_Surface *_xr_render_surface_adopt(Xcb_Image_Info *xcbinf, xcb_drawable_t draw, int w, int h, int alpha);
Xcb_Render_Surface *_xr_render_surface_format_adopt(Xcb_Image_Info *xcbinf, xcb_drawable_t draw, int w, int h, xcb_render_pictforminfo_t *fmt, int alpha);
void                _xr_render_surface_free(Xcb_Render_Surface *rs);
void                _xr_render_surface_repeat_set(Xcb_Render_Surface *rs, int repeat);
void                _xr_render_surface_solid_rectangle_set(Xcb_Render_Surface *rs, int r, int g, int b, int a, int x, int y, int w, int h);
void                _xr_render_surface_argb_pixels_fill(Xcb_Render_Surface *rs, int sw, int sh, void *pixels, int x, int y, int w, int h);
void                _xr_render_surface_rgb_pixels_fill(Xcb_Render_Surface *rs, int sw, int sh, void *pixels, int x, int y, int w, int h);
void                _xr_render_surface_clips_set(Xcb_Render_Surface *rs, RGBA_Draw_Context *dc, int rx, int ry, int rw, int rh);
void                _xr_render_surface_composite(Xcb_Render_Surface *srs, Xcb_Render_Surface *drs, RGBA_Draw_Context *dc, int sx, int sy, int sw, int sh, int x, int y, int w, int h, int smooth);
void                _xr_render_surface_copy(Xcb_Render_Surface *srs, Xcb_Render_Surface *drs, int sx, int sy, int x, int y, int w, int h);
void                _xr_render_surface_rectangle_draw(Xcb_Render_Surface *rs, RGBA_Draw_Context *dc, int x, int y, int w, int h);
void                _xr_render_surface_line_draw(Xcb_Render_Surface *rs, RGBA_Draw_Context *dc, int x1, int y1, int x2, int y2);
void                _xre_poly_draw(Xcb_Render_Surface *rs, RGBA_Draw_Context *dc, RGBA_Polygon_Point *points);
  
    
typedef struct _XR_Image XR_Image;

struct _XR_Image
{
   Xcb_Image_Info       *xcbinf;
   const char           *file;
   const char           *key;
   char                 *fkey;
   RGBA_Image           *im;
   void                 *data;
   int                   w, h;
   Xcb_Render_Surface   *surface;
   int                   references;
   char                 *format;
   const char           *comment;
   Tilebuf              *updates;
   RGBA_Image_Loadopts   load_opts;
   unsigned char         alpha : 1;
   unsigned char         dirty : 1;
   unsigned char         free_data : 1;
};

XR_Image *_xre_image_load(Xcb_Image_Info *xcbinf, const char *file, const char *key, Evas_Image_Load_Opts *lo);
XR_Image *_xre_image_new_from_data(Xcb_Image_Info *xcbinf, int w, int h, void *data);
XR_Image *_xre_image_new_from_copied_data(Xcb_Image_Info *xcbinf, int w, int h, void *data);
XR_Image *_xre_image_new(Xcb_Image_Info *xcbinf, int w, int h);
void      _xre_image_resize(XR_Image *im, int w, int h);
void      _xre_image_free(XR_Image *im);
void      _xre_image_region_dirty(XR_Image *im, int x, int y, int w, int h);
void      _xre_image_dirty(XR_Image *im);
XR_Image *_xre_image_copy(XR_Image *im);
void     *_xre_image_data_get(XR_Image *im);
XR_Image *_xre_image_data_find(void *data);
void      _xre_image_data_put(XR_Image *im, void *data);
void      _xre_image_alpha_set(XR_Image *im, int alpha);
int       _xre_image_alpha_get(XR_Image *im);
void      _xre_image_border_set(XR_Image *im, int l, int r, int t, int b);
void      _xre_image_border_get(XR_Image *im, int *l, int *r, int *t, int *b);
void      _xre_image_surface_gen(XR_Image *im);
void      _xre_image_cache_set(int size);
int       _xre_image_cache_get(void);

typedef struct _XR_Font_Surface XR_Font_Surface;

struct _XR_Font_Surface
{
   Xcb_Image_Info      *xcbinf;
   RGBA_Font_Glyph     *fg;
   int                  w;
   int                  h;
   xcb_drawable_t       draw;
   xcb_render_picture_t pic;
};

XR_Font_Surface *_xre_font_surface_new (Xcb_Image_Info   *xcbinf,
                                        RGBA_Font_Glyph  *fg);

void             _xre_font_surface_free (XR_Font_Surface   *fs);

void             _xre_font_surface_draw (Xcb_Image_Info    *xcbinf,
                                         RGBA_Image        *surface,
                                         RGBA_Draw_Context *dc,
                                         RGBA_Font_Glyph   *fg,
                                         int                x,
                                         int                y);

typedef struct _XR_Gradient XR_Gradient;

struct _XR_Gradient
{
   Xcb_Image_Info     *xcbinf;
   Xcb_Render_Surface *surface;
   RGBA_Gradient      *grad;
   unsigned char       changed;
   int                 sw, sh;
};

/* gradient */
XR_Gradient *_xre_gradient_new(Xcb_Image_Info *xcbinf);
void         _xre_gradient_free(XR_Gradient *gr);
void         _xre_gradient_color_stop_add(XR_Gradient *gr, int r, int g, int b, int a, int delta);
void         _xre_gradient_alpha_stop_add(XR_Gradient *gr, int a, int delta);
void         _xre_gradient_color_data_set(XR_Gradient *gr, void *map, int len, int has_alpha);
void         _xre_gradient_alpha_data_set(XR_Gradient *gr, void *alpha_map, int len);
void         _xre_gradient_clear(XR_Gradient *gr);
void         _xre_gradient_fill_set(XR_Gradient *gr, int x, int y, int w, int h);
void         _xre_gradient_fill_angle_set(XR_Gradient *gr, double angle);
void         _xre_gradient_fill_spread_set(XR_Gradient *gr, int spread);
void         _xre_gradient_angle_set(XR_Gradient *gr, double angle);
void         _xre_gradient_offset_set(XR_Gradient *gr, float offset);
void         _xre_gradient_direction_set(XR_Gradient *gr, int direction);
void         _xre_gradient_type_set(XR_Gradient *gr, char *name, char *params);
void         _xre_gradient_draw(Xcb_Render_Surface *rs, RGBA_Draw_Context *dc, XR_Gradient *gr, int x, int y, int w, int h);
    
#endif
