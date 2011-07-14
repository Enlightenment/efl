#include "ecore_xcb_private.h"
# include <xcb/xproto.h>
# include <xcb/xcb_image.h>
#ifdef ECORE_XCB_CURSOR
# include <xcb/render.h>
# include <xcb/xcb_renderutil.h>
#endif

/* local function prototypes */
static xcb_image_t *_ecore_xcb_cursor_image_create(int w, int h, int *pixels);
static Ecore_X_Cursor _ecore_xcb_cursor_image_load_cursor(Ecore_X_Window win, int w, int h, int hot_x, int hot_y, int *pixels, xcb_image_t *img);
#ifdef ECORE_XCB_CURSOR
static Ecore_X_Cursor _ecore_xcb_cursor_image_load_argb_cursor(Ecore_X_Window win, int w, int h, int hot_x, int hot_y, xcb_image_t *img);
static xcb_render_pictforminfo_t *_ecore_xcb_cursor_find_image_format(void);
#endif

/* local variables */
#ifdef ECORE_XCB_CURSOR
static xcb_render_pictforminfo_t *_ecore_xcb_cursor_format = NULL;
#endif
static int _ecore_xcb_cursor_size = 0;
static Eina_Bool _ecore_xcb_cursor = EINA_FALSE;

void 
_ecore_xcb_cursor_init(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_CURSOR
   /* NB: noop */
#endif
}

void 
_ecore_xcb_cursor_finalize(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_CURSOR
   _ecore_xcb_cursor = _ecore_xcb_render_argb_get();
#endif
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
   xcb_create_glyph_cursor(_ecore_xcb_conn, cursor, font, font, 
                           shape, shape + 1, 0, 0, 0, 65535, 65535, 65535);
   xcb_close_font(_ecore_xcb_conn, font);

   return cursor;
}

EAPI void 
ecore_x_cursor_free(Ecore_X_Cursor cursor) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xcb_free_cursor(_ecore_xcb_conn, cursor);
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

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (_ecore_xcb_cursor) // argb
     {
        if ((img = _ecore_xcb_cursor_image_create(w, h, pixels)))
          {
#ifdef ECORE_XCB_CURSOR
             cursor = 
               _ecore_xcb_cursor_image_load_argb_cursor(win, w, h, 
                                                        hot_x, hot_y, img);
#else
             cursor = 
               _ecore_xcb_cursor_image_load_cursor(win, w, h, 
                                                   hot_x, hot_y, pixels, img);
#endif
          }
        else
          DBG("Failed to create new cursor image");
     }
   else 
     {
        if ((img = _ecore_xcb_cursor_image_create(w, h, pixels)))
          {
             cursor = 
               _ecore_xcb_cursor_image_load_cursor(win, w, h, 
                                                   hot_x, hot_y, pixels, img);
          }
        else
          DBG("Failed to create new cursor image");
     }

   if (cursor) 
     {
        uint32_t mask, list;

        mask = XCB_CW_CURSOR;
        list = cursor;
        xcb_change_window_attributes(_ecore_xcb_conn, win, mask, &list);
     }

   return cursor;
}

EAPI void 
ecore_x_cursor_size_set(int size) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   // NB: size_set only needed for non-argb cursors
   _ecore_xcb_cursor_size = size;
}

EAPI int 
ecore_x_cursor_size_get(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return _ecore_xcb_cursor_size;
}

/* local functions */
static xcb_image_t *
_ecore_xcb_cursor_image_create(int w, int h, int *pixels) 
{
   // NB: May be able to use shm here, but the image NEEDS to be in 
   // native format
   if (_ecore_xcb_cursor) 
     {
#ifdef ECORE_XCB_CURSOR
        return xcb_image_create_native(_ecore_xcb_conn, w, h, 
                                       XCB_IMAGE_FORMAT_Z_PIXMAP, 
                                       32, pixels, (w * h * sizeof(int)), // 32
                                       (uint8_t *)pixels);
#else
        return xcb_image_create_native(_ecore_xcb_conn, w, h, 
                                       XCB_IMAGE_FORMAT_Z_PIXMAP, 
                                       1, pixels, (w * h * sizeof(int)), // 32
                                       (uint8_t *)pixels);
#endif
     }
   else 
     {
        return xcb_image_create_native(_ecore_xcb_conn, w, h, 
                                       XCB_IMAGE_FORMAT_Z_PIXMAP, 1, 
                                       NULL, ~0, NULL);
     }
}

static Ecore_X_Cursor 
_ecore_xcb_cursor_image_load_cursor(Ecore_X_Window win, int w, int h, int hot_x, int hot_y, int *pixels, xcb_image_t *img) 
{
   xcb_pixmap_t pixmap, mask;
   Ecore_X_Cursor cursor;
   Ecore_X_GC gc;
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

   pixmap = xcb_generate_id(_ecore_xcb_conn);
   xcb_create_pixmap(_ecore_xcb_conn, 1, pixmap, win, w, h);

   mask = xcb_generate_id(_ecore_xcb_conn);
   xcb_create_pixmap(_ecore_xcb_conn, 1, mask, win, w, h);

   img->data = malloc(img->size);

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

   // img->depth was 1
   gc = ecore_x_gc_new(pixmap, 0, NULL);
   xcb_put_image(_ecore_xcb_conn, XCB_IMAGE_FORMAT_Z_PIXMAP, 
                 pixmap, gc, w, h, 0, 0, 0, img->depth, img->size, img->data);
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

   // img->depth was 1
   gc = ecore_x_gc_new(mask, 0, NULL);
   xcb_put_image(_ecore_xcb_conn, XCB_IMAGE_FORMAT_Z_PIXMAP, 
                 mask, gc, w, h, 0, 0, 0, img->depth, img->size, img->data);
   ecore_x_gc_free(gc);

   cursor = xcb_generate_id(_ecore_xcb_conn);
   xcb_create_cursor(_ecore_xcb_conn, cursor, pixmap, mask, 
                     fr << 8 | fr, fg << 8 | fg, fb << 8 | fb,
                     br << 8 | br, bg << 8 | bg, bb << 8 | bb, 
                     hot_x, hot_y);

   xcb_free_pixmap(_ecore_xcb_conn, pixmap);
   xcb_free_pixmap(_ecore_xcb_conn, mask);

   return cursor;
}

#ifdef ECORE_XCB_CURSOR
static Ecore_X_Cursor 
_ecore_xcb_cursor_image_load_argb_cursor(Ecore_X_Window win, int w, int h, int hot_x, int hot_y, xcb_image_t *img) 
{
   xcb_pixmap_t pixmap;
   xcb_render_picture_t pict;
   Ecore_X_Cursor cursor;
   Ecore_X_GC gc;

   if (!_ecore_xcb_cursor_format) 
     _ecore_xcb_cursor_format = _ecore_xcb_cursor_find_image_format();

   pixmap = xcb_generate_id(_ecore_xcb_conn);
   xcb_create_pixmap(_ecore_xcb_conn, 32, pixmap, win, w, h);

   // img->depth was 32
   gc = ecore_x_gc_new(pixmap, 0, NULL);
   xcb_put_image(_ecore_xcb_conn, XCB_IMAGE_FORMAT_Z_PIXMAP, 
                 pixmap, gc, w, h, 0, 0, 0, 
                 img->depth, img->size, img->data);
   ecore_x_gc_free(gc);

   pict = xcb_generate_id(_ecore_xcb_conn);
   xcb_render_create_picture(_ecore_xcb_conn, pict, pixmap, 
                             _ecore_xcb_cursor_format->id, 0, NULL);
   xcb_free_pixmap(_ecore_xcb_conn, pixmap);

   cursor = xcb_generate_id(_ecore_xcb_conn);
   xcb_render_create_cursor(_ecore_xcb_conn, cursor, pict, hot_x, hot_y);
   xcb_render_free_picture(_ecore_xcb_conn, pict);

   return cursor;
}

static xcb_render_pictforminfo_t *
_ecore_xcb_cursor_find_image_format(void)
{
   const xcb_render_query_pict_formats_reply_t *reply;
   xcb_render_pictforminfo_t *ret = NULL;

   reply = xcb_render_util_query_formats(_ecore_xcb_conn);
   if (reply) 
     {
        ret = xcb_render_util_find_standard_format(reply, 
                                                   XCB_PICT_STANDARD_ARGB_32);
//        free(reply);
     }

   return ret;
}
#endif
