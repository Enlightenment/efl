#include "ecore_xcb_private.h"
#ifdef ECORE_XCB_CURSOR
# include <xcb/render.h>
# include <xcb/xcb_renderutil.h>
#endif

/* local function prototypes */
#ifdef ECORE_XCB_CURSOR
static xcb_render_pictforminfo_t *_ecore_xcb_cursor_format_get(void);
#endif
static void _ecore_xcb_cursor_default_size_get(void);
static void _ecore_xcb_cursor_dpi_size_get(void);
static void _ecore_xcb_cursor_guess_size(void);
#ifdef ECORE_XCB_CURSOR
static Ecore_X_Cursor _ecore_xcb_cursor_image_load_cursor(xcb_image_t *img, int hot_x, int hot_y);
#endif
static void _ecore_xcb_cursor_image_destroy(xcb_image_t *img);

/* local variables */
static int _ecore_xcb_cursor_size = 0;
static Eina_Bool _ecore_xcb_cursor = EINA_FALSE;
#ifdef ECORE_XCB_CURSOR
static uint32_t _ecore_xcb_cursor_format_id = 0;
//   static xcb_render_pictforminfo_t *_ecore_xcb_cursor_format = NULL;
#endif

void 
_ecore_xcb_cursor_init(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   /* NB: No-op */
}

void 
_ecore_xcb_cursor_finalize(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_CURSOR
   _ecore_xcb_cursor = _ecore_xcb_render_argb_get();

   /* find render pict format */
   if (_ecore_xcb_cursor_format_id <= 0) 
     _ecore_xcb_cursor_format_id = _ecore_xcb_cursor_format_get()->id;
#endif

   /* try to grab cursor size from XDefaults */
   _ecore_xcb_cursor_default_size_get();

   /* if that failed, try to get it from Xft Dpi setting */
   if (_ecore_xcb_cursor_size == 0)
     _ecore_xcb_cursor_dpi_size_get();

   /* if that failed, try to guess from display size */
   if (_ecore_xcb_cursor_size == 0)
     _ecore_xcb_cursor_guess_size();

   /* NB: Would normally add theme stuff here, but E cursor does not support 
    * xcursor themes. Delay parsing that stuff out until such time if/when the 
    * user selects to use X Cursor, rather than E cursor */
}

EAPI Eina_Bool 
ecore_x_cursor_color_supported_get(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return _ecore_xcb_cursor;
}

EAPI Ecore_X_Cursor 
ecore_x_cursor_new(Ecore_X_Window win, int *pixels, int w, int h, int hot_x, int hot_y) 
{
   Ecore_X_Cursor cursor = 0;
   xcb_image_t *img;

//   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_CURSOR
   if (_ecore_xcb_cursor) 
     {
        img = _ecore_xcb_image_create_native(w, h, XCB_IMAGE_FORMAT_Z_PIXMAP, 
                                             32, NULL, (w * h * sizeof(int)), 
                                             (uint8_t *)pixels);
        cursor = _ecore_xcb_cursor_image_load_cursor(img, hot_x, hot_y);
        _ecore_xcb_cursor_image_destroy(img);
        return cursor;
     }
   else
#endif
     {
        Ecore_X_GC gc;
        xcb_pixmap_t pmap, mask;
        uint32_t *pix;
        uint8_t fr = 0x00, fg = 0x00, fb = 0x00;
        uint8_t br = 0xff, bg = 0xff, bb = 0xff;
        uint32_t brightest = 0, darkest = 255 * 3;
        uint16_t x, y;
        const uint32_t dither[2][2] =
          {
             {0, 2},
             {3, 1}
          };

        img = _ecore_xcb_image_create_native(w, h, XCB_IMAGE_FORMAT_Z_PIXMAP, 
                                             1, NULL, ~0, NULL);
        if (img->data) free(img->data);
        img->data = malloc(img->size);

        pmap = xcb_generate_id(_ecore_xcb_conn);
        xcb_create_pixmap(_ecore_xcb_conn, 1, pmap, win, w, h);
        mask = xcb_generate_id(_ecore_xcb_conn);
        xcb_create_pixmap(_ecore_xcb_conn, 1, mask, win, w, h);

        pix = (uint32_t *)pixels;
        for (y = 0; y < h; y++)
          {
             for (x = 0; x < w; x++)
               {
                  uint8_t r, g, b, a;

                  a = (pix[0] >> 24) & 0xff;
                  r = (pix[0] >> 16) & 0xff;
                  g = (pix[0] >> 8) & 0xff;
                  b = (pix[0]) & 0xff;
                  if (a > 0)
                    {
                       if ((uint32_t)(r + g + b) > brightest)
                         {
                            brightest = r + g + b;
                            br = r;
                            bg = g;
                            bb = b;
                         }

                       if ((uint32_t)(r + g + b) < darkest)
                         {
                            darkest = r + g + b;
                            fr = r;
                            fg = g;
                            fb = b;
                         }
                    }
                  pix++;
               }
          }

        pix = (uint32_t *)pixels;
        for (y = 0; y < h; y++)
          {
             for (x = 0; x < w; x++)
               {
                  uint32_t v;
                  uint8_t r, g, b;
                  int32_t d1, d2;

                  r = (pix[0] >> 16) & 0xff;
                  g = (pix[0] >> 8) & 0xff;
                  b = (pix[0]) & 0xff;
                  d1 =
                    ((r - fr) * (r - fr)) +
                    ((g - fg) * (g - fg)) +
                    ((b - fb) * (b - fb));
                  d2 =
                    ((r - br) * (r - br)) +
                    ((g - bg) * (g - bg)) +
                    ((b - bb) * (b - bb));
                  if (d1 + d2)
                    {
                       v = (((d2 * 255) / (d1 + d2)) * 5) / 256;
                       if (v > dither[x & 0x1][y & 0x1])
                         v = 1;
                       else
                         v = 0;
                    }
                  else
                    v = 0;

                  xcb_image_put_pixel(img, x, y, v);
                  pix++;
               }
          }

        gc = ecore_x_gc_new(pmap, 0, NULL);
        xcb_put_image(_ecore_xcb_conn, img->format, pmap, gc, w, h, 
                      0, 0, 0, img->depth, img->size, img->data);
        ecore_x_gc_free(gc);

        pix = (uint32_t *)pixels;
        for (y = 0; y < h; y++)
          {
             for (x = 0; x < w; x++)
               {
                  uint32_t v;

                  v = (((pix[0] >> 24) & 0xff) * 5) / 256;
                  if (v > dither[x & 0x1][y & 0x1])
                    v = 1;
                  else
                    v = 0;

                  xcb_image_put_pixel(img, x, y, v);
                  pix++;
               }
          }

        gc = ecore_x_gc_new(mask, 0, NULL);
        xcb_put_image(_ecore_xcb_conn, img->format, mask, gc, w, h, 
                      0, 0, 0, img->depth, img->size, img->data);
        ecore_x_gc_free(gc);

        if (img->data) free(img->data);
        _ecore_xcb_cursor_image_destroy(img);

        cursor = xcb_generate_id(_ecore_xcb_conn);
        xcb_create_cursor(_ecore_xcb_conn, cursor, pmap, mask, 
                          fr << 8 | fr, fg << 8 | fg, fb << 8 | fb,
                          br << 8 | br, bg << 8 | bg, bb << 8 | bb, 
                          hot_x, hot_y);

        xcb_free_pixmap(_ecore_xcb_conn, pmap);
        xcb_free_pixmap(_ecore_xcb_conn, mask);

        return cursor;
     }

   return 0;
}

EAPI void 
ecore_x_cursor_free(Ecore_X_Cursor c) 
{
//   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xcb_free_cursor(_ecore_xcb_conn, c);
}

/*
 * Returns the cursor for the given shape.
 * Note that the return value must not be freed with
 * ecore_x_cursor_free()!
 */
EAPI Ecore_X_Cursor 
ecore_x_cursor_shape_get(int shape) 
{
   Ecore_X_Cursor cursor = 0;
   xcb_font_t font;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   font = xcb_generate_id(_ecore_xcb_conn);
   xcb_open_font(_ecore_xcb_conn, font, strlen("cursor"), "cursor");

   cursor = xcb_generate_id(_ecore_xcb_conn);
   /* FIXME: Add request check ?? */
   xcb_create_glyph_cursor(_ecore_xcb_conn, cursor, font, font, 
                           shape, shape + 1, 0, 0, 0, 65535, 65535, 65535);

   xcb_close_font(_ecore_xcb_conn, font);
   return cursor;
}

EAPI void 
ecore_x_cursor_size_set(int size) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_xcb_cursor_size = size;
   /* NB: May need to adjust size of current cursors here */
}

EAPI int 
ecore_x_cursor_size_get(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return _ecore_xcb_cursor_size;
}

/* local functions */
#ifdef ECORE_XCB_CURSOR
static xcb_render_pictforminfo_t *
_ecore_xcb_cursor_format_get(void) 
{
   const xcb_render_query_pict_formats_reply_t *reply;
   xcb_render_pictforminfo_t *ret = NULL;

   reply = xcb_render_util_query_formats(_ecore_xcb_conn);
   if (reply) 
     ret = xcb_render_util_find_standard_format(reply, 
                                                XCB_PICT_STANDARD_ARGB_32);

   return ret;
}
#endif

static void 
_ecore_xcb_cursor_default_size_get(void) 
{
   char *s = NULL;
   int v = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   s = getenv("XCURSOR_SIZE");
   if (!s) 
     {
        _ecore_xcb_xdefaults_init();
        v = _ecore_xcb_xdefaults_int_get("Xcursor", "size");
        _ecore_xcb_xdefaults_shutdown();
     }
   else
     v = atoi(s);
   if (v) _ecore_xcb_cursor_size = ((v * 16) / 72);
}

static void 
_ecore_xcb_cursor_dpi_size_get(void) 
{
   int v = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_xcb_xdefaults_init();
   v = _ecore_xcb_xdefaults_int_get("Xft", "dpi");
   if (v) _ecore_xcb_cursor_size = ((v * 16) / 72);
   _ecore_xcb_xdefaults_shutdown();
}

static void 
_ecore_xcb_cursor_guess_size(void) 
{
   int w = 0, h = 0, s = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_screen_size_get(_ecore_xcb_screen, &w, &h);
   if (h < w) s = h;
   else s = w;
   _ecore_xcb_cursor_size = (s / 48);
}

#ifdef ECORE_XCB_CURSOR
static Ecore_X_Cursor 
_ecore_xcb_cursor_image_load_cursor(xcb_image_t *img, int hot_x, int hot_y) 
{
   Ecore_X_Cursor cursor = 0;
   Ecore_X_GC gc;
   xcb_pixmap_t pmap;
   xcb_render_picture_t pict;

   pmap = xcb_generate_id(_ecore_xcb_conn);
   xcb_create_pixmap(_ecore_xcb_conn, img->depth, pmap, 
                     ((xcb_screen_t *)_ecore_xcb_screen)->root, 
                     img->width, img->height);

   gc = ecore_x_gc_new(pmap, 0, NULL);
   xcb_put_image(_ecore_xcb_conn, img->format, pmap, gc, 
                 img->width, img->height, 0, 0, 0, img->depth, 
                 img->size, img->data);
   ecore_x_gc_free(gc);

   pict = xcb_generate_id(_ecore_xcb_conn);
   xcb_render_create_picture(_ecore_xcb_conn, pict, pmap, 
                             _ecore_xcb_cursor_format_id, 0, NULL);
   xcb_free_pixmap(_ecore_xcb_conn, pmap);

   cursor = xcb_generate_id(_ecore_xcb_conn);
   xcb_render_create_cursor(_ecore_xcb_conn, cursor, pict, hot_x, hot_y);
   xcb_render_free_picture(_ecore_xcb_conn, pict);

   return cursor;
}
#endif

static void 
_ecore_xcb_cursor_image_destroy(xcb_image_t *img) 
{
   if (img) xcb_image_destroy(img);
}
