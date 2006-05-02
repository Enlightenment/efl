#ifndef EVAS_ENGINE_H
#define EVAS_ENGINE_H

#include <sys/ipc.h>
#include <sys/shm.h>

#include <X11/XCB/xcb.h>
#include <X11/XCB/shm.h>
#include <X11/XCB/render.h>
#include <X11/XCB/xcb_image.h>

typedef struct _XCBimage_Info      XCBimage_Info;
typedef struct _XCBimage_Image     XCBimage_Image;
typedef struct _XCBrender_Surface  XCBrender_Surface;

struct _XCBimage_Info
{
   XCBConnection         *conn;
   XCBDRAWABLE            root;
   XCBDRAWABLE            draw;
   int                    depth;
   XCBVISUALID            vis;
   int                    pool_mem;
   Evas_List             *pool;
   unsigned char          can_do_shm;
   XCBRenderPICTFORMINFO *fmt32;
   XCBRenderPICTFORMINFO *fmt24;
   XCBRenderPICTFORMINFO *fmt8;
   XCBRenderPICTFORMINFO *fmt4;
   XCBRenderPICTFORMINFO *fmt1;
   unsigned char          mul_r;
   unsigned char          mul_g;
   unsigned char          mul_b;
   unsigned char          mul_a;
   XCBrender_Surface     *mul;
   int                    references;
};

struct _XCBimage_Image
{
   XCBimage_Info     *xcbinf;
   XCBImage          *xcbim;
   XCBShmSegmentInfo *shm_info;
   int                w;
   int                h;
   int                depth;
   int                line_bytes;
   unsigned char     *data;
   unsigned char      available : 1;
};

struct _XCBrender_Surface
{
   XCBimage_Info         *xcbinf;
   int                    w;
   int                    h;
   int                    depth;
   XCBRenderPICTFORMINFO *fmt;
   XCBDRAWABLE            draw;
   XCBRenderPICTURE       pic;
   unsigned char          alpha : 1;
   unsigned char          allocated : 1;
};

/* ximage support calls (ximage vs xshmimage, cache etc.) */
XCBimage_Info  *_xr_image_info_get(XCBConnection *conn, XCBDRAWABLE draw, XCBVISUALID vis);
void          _xr_image_info_free(XCBimage_Info *xcbinf);
void          _xr_image_info_pool_flush(XCBimage_Info *xcbinf, int max_num, int max_mem);
XCBimage_Image *_xr_image_new(XCBimage_Info *xcbinf, int w, int h, int depth);
void          _xr_image_free(XCBimage_Image *xim);
void          _xr_image_put(XCBimage_Image *xim, XCBDRAWABLE draw, int x, int y, int w, int h);

/* xrender support calls */
XCBrender_Surface *_xr_render_surface_new(XCBimage_Info *xcbinf, int w, int h, XCBRenderPICTFORMINFO *fmt, int alpha);
XCBrender_Surface *_xr_render_surface_adopt(XCBimage_Info *xcbinf, XCBDRAWABLE draw, int w, int h, int alpha);
XCBrender_Surface *_xr_render_surface_format_adopt(XCBimage_Info *xcbinf, XCBDRAWABLE draw, int w, int h, XCBRenderPICTFORMINFO *fmt, int alpha);
void             _xr_render_surface_free(XCBrender_Surface *rs);
void             _xr_render_surface_repeat_set(XCBrender_Surface *rs, int repeat);
void             _xr_render_surface_solid_rectangle_set(XCBrender_Surface *rs, int r, int g, int b, int a, int x, int y, int w, int h);
void             _xr_render_surface_argb_pixels_fill(XCBrender_Surface *rs, int sw, int sh, void *pixels, int x, int y, int w, int h);
void             _xr_render_surface_rgb_pixels_fill(XCBrender_Surface *rs, int sw, int sh, void *pixels, int x, int y, int w, int h);
void             _xr_render_surface_clips_set(XCBrender_Surface *rs, RGBA_Draw_Context *dc, int rx, int ry, int rw, int rh);
void             _xr_render_surface_composite(XCBrender_Surface *srs, XCBrender_Surface *drs, RGBA_Draw_Context *dc, int sx, int sy, int sw, int sh, int x, int y, int w, int h, int smooth);
void             _xr_render_surface_copy(XCBrender_Surface *srs, XCBrender_Surface *drs, int sx, int sy, int x, int y, int w, int h);
void             _xr_render_surface_rectangle_draw(XCBrender_Surface *rs, RGBA_Draw_Context *dc, int x, int y, int w, int h);
void             _xr_render_surface_line_draw(XCBrender_Surface *rs, RGBA_Draw_Context *dc, int x1, int y1, int x2, int y2);
void             _xre_poly_draw(XCBrender_Surface *rs, RGBA_Draw_Context *dc, RGBA_Polygon_Point *points);
  
    
typedef struct _XR_Image XR_Image;

struct _XR_Image
{
   XCBimage_Info     *xcbinf;
   char              *file;
   char              *key;
   char              *fkey;
   RGBA_Image        *im;
   void              *data;
   int                w, h;
   XCBrender_Surface *surface;
   int                references;
   char              *format;
   char              *comment;
   Tilebuf           *updates;
   unsigned char      alpha : 1;
   unsigned char      dirty : 1;
   unsigned char      free_data : 1;
};

XR_Image *_xre_image_load(XCBimage_Info *xcbinf, char *file, char *key);
XR_Image *_xre_image_new_from_data(XCBimage_Info *xcbinf, int w, int h, void *data);
XR_Image *_xre_image_new_from_copied_data(XCBimage_Info *xcbinf, int w, int h, void *data);
XR_Image *_xre_image_new(XCBimage_Info *xcbinf, int w, int h);
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
void      _xre_image_surface_gen(XR_Image *im);
void      _xre_image_cache_set(int size);
int       _xre_image_cache_get(void);

typedef struct _XR_Font_Surface XR_Font_Surface;

struct _XR_Font_Surface
{
   XCBimage_Info     *xcbinf;
   RGBA_Font_Glyph   *fg;
   int                w;
   int                h;
   XCBDRAWABLE        draw;
   XCBRenderPICTURE   pic;
};

XR_Font_Surface *_xre_font_surface_new (XCBimage_Info      *xcbinf,
                                        RGBA_Font_Glyph    *fg);

void             _xre_font_surface_free (XR_Font_Surface   *fs);

void             _xre_font_surface_draw (XCBimage_Info     *xcbinf,
                                         RGBA_Image        *surface,
                                         RGBA_Draw_Context *dc,
                                         RGBA_Font_Glyph   *fg,
                                         int                x,
                                         int                y);

typedef struct _XR_Gradient XR_Gradient;

struct _XR_Gradient
{
   XCBimage_Info     *xcbinf;
   XCBrender_Surface *surface;
   RGBA_Gradient     *grad;
   double             angle;
   int                spread;
   unsigned char      changed;
};

/* gradient */
XR_Gradient *_xre_gradient_color_add       (XCBimage_Info    *xcbinf,
                                            XR_Gradient      *gr,
                                            int               r,
                                            int               g,
                                            int               b,
                                            int               a,
                                            int               distance);

XR_Gradient *_xre_gradient_colors_clear    (XR_Gradient      *gr);

XR_Gradient *_xre_gradient_data_set        (XCBimage_Info *xcbinf,
                                            XR_Gradient *gr,
                                            void *map,
                                            int len,
                                            int has_alpha);

XR_Gradient *_xre_gradient_data_unset      (XR_Gradient *gr);

void         _xre_gradient_free            (XR_Gradient      *gr);

void         _xre_gradient_fill_set        (XR_Gradient      *gr,
                                            int               x,
                                            int               y,
                                            int               w,
                                            int               h);

void         _xre_gradient_range_offset_set(XR_Gradient *gr,
                                            float offset);

void         _xre_gradient_type_set        (XR_Gradient      *gr,
                                            char             *name);

void         _xre_gradient_type_params_set (XR_Gradient      *gr,
                                            char             *params);

void        *_xre_gradient_geometry_init   (XR_Gradient      *gr,
                                            int               spread);

int          _xre_gradient_alpha_get       (XR_Gradient       *gr,
                                            int                spread,
                                            int op);

void         _xre_gradient_map             (RGBA_Draw_Context *dc,
                                            XR_Gradient       *gr,
                                            int                spread);

void         _xre_gradient_draw            (XCBrender_Surface *rs,
                                            RGBA_Draw_Context *dc,
                                            XR_Gradient       *gr,
                                            int                x,
                                            int                y,
                                            int                w,
                                            int                h,
                                            double             angle,
                                            int                spread);
    
#endif
