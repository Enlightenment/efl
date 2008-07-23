/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "ecore_xcb_private.h"
#include <xcb/shm.h>
#include <xcb/xcb_image.h>


extern int _ecore_xcb_xcursor;


EAPI int
ecore_x_cursor_color_supported_get(void)
{
   return _ecore_xcb_xcursor;
}

EAPI Ecore_X_Cursor
ecore_x_cursor_new(Ecore_X_Window window,
                   int           *pixels,
                   int            w,
                   int            h,
                   int            hot_x,
                   int            hot_y)
{
   Ecore_X_Cursor cursor = 0;

#ifdef ECORE_XCB_CURSOR
   if (_ecore_x_xcursor)
     {
	Cursor c;
	XcursorImage *xci;

	xci = XcursorImageCreate(w, h);
	if (xci)
	  {
	     int i;

	     xci->xhot = hot_x;
	     xci->yhot = hot_y;
	     xci->delay = 0;
	     for (i = 0; i < (w * h); i++)
	       {
//		  int r, g, b, a;
//
//		  a = (pixels[i] >> 24) & 0xff;
//		  r = (((pixels[i] >> 16) & 0xff) * a) / 0xff;
//		  g = (((pixels[i] >> 8 ) & 0xff) * a) / 0xff;
//		  b = (((pixels[i]      ) & 0xff) * a) / 0xff;
		  xci->pixels[i] = pixels[i];
//		    (a << 24) | (r << 16) | (g << 8) | (b);
	       }
	     c = XcursorImageLoadCursor(_ecore_x_disp, xci);
	     XcursorImageDestroy(xci);
	     return c;
	  }
     }
   else
#endif /* ECORE_XCB_CURSOR */
     {
	const uint32_t dither[2][2] =
	  {
	       {0, 2},
	       {3, 1}
	  };
        Ecore_X_Drawable draw;
        Ecore_X_Pixmap   pixmap;
        Ecore_X_Pixmap   mask;
        Ecore_X_GC       gc;
        xcb_image_t     *image;
        uint32_t        *pix;
        uint8_t          fr;
        uint8_t          fg;
        uint8_t          fb;
        uint8_t          br;
        uint8_t          bg;
        uint8_t          bb;
	uint32_t         brightest = 0;
	uint32_t         darkest = 255 * 3;
        uint16_t         x;
        uint16_t         y;

        draw = window;
        pixmap = xcb_generate_id(_ecore_xcb_conn);
        xcb_create_pixmap(_ecore_xcb_conn,
                          1, pixmap, draw,
                          1, 1);
        mask = xcb_generate_id(_ecore_xcb_conn);
        xcb_create_pixmap(_ecore_xcb_conn,
                          1, mask, draw,
                          1, 1);

        image = xcb_image_create_native(_ecore_xcb_conn, w, h,
                                 XCB_IMAGE_FORMAT_Z_PIXMAP,
                                 32, NULL, ~0, NULL);
        image->data = malloc(image->size);

	fr = 0x00; fg = 0x00; fb = 0x00;
	br = 0xff; bg = 0xff; bb = 0xff;
        pix = (uint32_t *)pixels;
	for (y = 0; y < h; y++)
	  {
	     for (x = 0; x < w; x++)
	       {
		  uint8_t r, g, b, a;

		  a = (pix[0] >> 24) & 0xff;
		  r = (pix[0] >> 16) & 0xff;
		  g = (pix[0] >> 8 ) & 0xff;
		  b = (pix[0]      ) & 0xff;
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
		  uint8_t  r, g, b;
		  int32_t  d1, d2;

		  r = (pix[0] >> 16) & 0xff;
		  g = (pix[0] >> 8 ) & 0xff;
		  b = (pix[0]      ) & 0xff;
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
		       if (v > dither[x & 0x1][y & 0x1]) v = 1;
		       else v = 0;
		    }
		  else
		    {
		       v = 0;
		    }
		  xcb_image_put_pixel(image, x, y, v);
		  pix++;
	       }
	  }
        draw = pixmap;
        gc = xcb_generate_id(_ecore_xcb_conn);
        xcb_create_gc(_ecore_xcb_conn, gc, draw, 0, NULL);
        xcb_image_put(_ecore_xcb_conn, draw, gc, image, 0, 0, 0);
        xcb_free_gc(_ecore_xcb_conn, gc);

	pix = (uint32_t *)pixels;
	for (y = 0; y < h; y++)
	  {
	     for (x = 0; x < w; x++)
	       {
		  uint32_t v;

		  v = (((pix[0] >> 24) & 0xff) * 5) / 256;
		  if (v > dither[x & 0x1][y & 0x1]) v = 1;
		  else v = 0;
		  xcb_image_put_pixel(image, x, y, v);
		  pix++;
	       }
	  }
        draw = mask;
        gc = xcb_generate_id(_ecore_xcb_conn);
        xcb_create_gc (_ecore_xcb_conn, gc, draw, 0, NULL);
        xcb_image_put(_ecore_xcb_conn, draw, gc, image, 0, 0, 0);
        xcb_free_gc(_ecore_xcb_conn, gc);

	free(image->data);
	image->data = NULL;
	xcb_image_destroy(image);

        cursor = xcb_generate_id(_ecore_xcb_conn);
        xcb_create_cursor (_ecore_xcb_conn, cursor,
                           pixmap, mask,
                           fr << 8 | fr,
                           fg << 8 | fg,
                           fb << 8 | fb,
                           br << 8 | br,
                           bg << 8 | bg,
                           bb << 8 | bb,
                           hot_x,
                           hot_y);
	xcb_free_pixmap(_ecore_xcb_conn, pixmap);
	xcb_free_pixmap(_ecore_xcb_conn, mask);

	return cursor;
     }
   return 0;
}

EAPI void
ecore_x_cursor_free(Ecore_X_Cursor cursor)
{
   xcb_free_cursor(_ecore_xcb_conn, cursor);
}

/*
 * Returns the cursor for the given shape.
 * Note that the return value must not be freed with
 * ecore_x_cursor_free()!
 */
EAPI Ecore_X_Cursor
ecore_x_cursor_shape_get(int shape)
{
   Ecore_X_Cursor cursor;
   xcb_font_t     font;

   /* Shapes are defined in Ecore_X_Cursor.h */
   font = xcb_generate_id(_ecore_xcb_conn);
   xcb_open_font(_ecore_xcb_conn, font, strlen("cursor"), "cursor");

   cursor = xcb_generate_id(_ecore_xcb_conn);
   xcb_create_glyph_cursor (_ecore_xcb_conn,
                            cursor,
                            font,
                            font,
                            shape,
                            shape + 1,
                            0, 0, 0,
                            65535, 65535, 65535);

   xcb_close_font(_ecore_xcb_conn, font);

   return cursor;
}

EAPI void
ecore_x_cursor_size_set(int size)
{
#ifdef ECORE_XCB_CURSOR
   XcursorSetDefaultSize(_ecore_x_disp, size);
#else
   size = 0;
#endif /* ECORE_XCB_CURSOR */
}

EAPI int
ecore_x_cursor_size_get(void)
{
#ifdef ECORE_XCB_CURSOR
   return XcursorGetDefaultSize(_ecore_x_disp);
#else
   return 0;
#endif /* ECORE_XCB_CURSOR */
}
