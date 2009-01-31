/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include "ecore_x_private.h"


EAPI int
ecore_x_cursor_color_supported_get(void)
{
   return _ecore_x_xcursor;
}

EAPI Ecore_X_Cursor
ecore_x_cursor_new(Ecore_X_Window win, int *pixels, int w, int h, int hot_x, int hot_y)
{
#ifdef ECORE_XCURSOR
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
#endif
     {
	XColor c1, c2;
	Cursor c;
	Pixmap pmap, mask;
	GC gc;
	XGCValues gcv;
	XImage *xim;
	unsigned int *pix;
	int fr, fg, fb, br, bg, bb;
	int brightest = 0;
	int darkest = 255 * 3;
	int x, y;
	const int dither[2][2] =
	  {
	       {0, 2},
	       {3, 1}
	  };

	pmap = XCreatePixmap(_ecore_x_disp, win, w, h, 1);
	mask = XCreatePixmap(_ecore_x_disp, win, w, h, 1);
	xim = XCreateImage(_ecore_x_disp,
			   DefaultVisual(_ecore_x_disp, 0),
			   1, ZPixmap, 0, NULL, w, h, 32, 0);
	xim->data = malloc(xim->bytes_per_line * xim->height);

	fr = 0x00; fg = 0x00; fb = 0x00;
	br = 0xff; bg = 0xff; bb = 0xff;
	pix = (unsigned int*)pixels;
	for (y = 0; y < h; y++)
	  {
	     for (x = 0; x < w; x++)
	       {
		  int r, g, b, a;

		  a = (pix[0] >> 24) & 0xff;
		  r = (pix[0] >> 16) & 0xff;
		  g = (pix[0] >> 8 ) & 0xff;
		  b = (pix[0]      ) & 0xff;
		  if (a > 0)
		    {
		       if ((r + g + b) > brightest)
			 {
			    brightest = r + g + b;
			    br = r;
			    bg = g;
			    bb = b;
			 }
		       if ((r + g + b) < darkest)
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

	pix = (unsigned int*)pixels;
	for (y = 0; y < h; y++)
	  {
	     for (x = 0; x < w; x++)
	       {
		  int v;
		  int r, g, b;
		  int d1, d2;

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
		  XPutPixel(xim, x, y, v);
		  pix++;
	       }
	  }
	gc = XCreateGC(_ecore_x_disp, pmap, 0, &gcv);
	XPutImage(_ecore_x_disp, pmap, gc, xim, 0, 0, 0, 0, w, h);
	XFreeGC(_ecore_x_disp, gc);

	pix = (unsigned int*)pixels;
	for (y = 0; y < h; y++)
	  {
	     for (x = 0; x < w; x++)
	       {
		  int v;

		  v = (((pix[0] >> 24) & 0xff) * 5) / 256;
		  if (v > dither[x & 0x1][y & 0x1]) v = 1;
		  else v = 0;
		  XPutPixel(xim, x, y, v);
		  pix++;
	       }
	  }
	gc = XCreateGC(_ecore_x_disp, mask, 0, &gcv);
	XPutImage(_ecore_x_disp, mask, gc, xim, 0, 0, 0, 0, w, h);
	XFreeGC(_ecore_x_disp, gc);

	free(xim->data);
	xim->data = NULL;
	XDestroyImage(xim);

	c1.pixel = 0;
	c1.red   = fr << 8 | fr;
	c1.green = fg << 8 | fg;
	c1.blue  = fb << 8 | fb;
	c1.flags = DoRed | DoGreen | DoBlue;

	c2.pixel = 0;
	c2.red   = br << 8 | br;
	c2.green = bg << 8 | bg;
	c2.blue  = bb << 8 | bb;
	c2.flags = DoRed | DoGreen | DoBlue;

	c = XCreatePixmapCursor(_ecore_x_disp,
				pmap, mask,
				&c1, &c2,
				hot_x, hot_y);
	XFreePixmap(_ecore_x_disp, pmap);
	XFreePixmap(_ecore_x_disp, mask);
	return c;
     }
   return 0;
}

EAPI void
ecore_x_cursor_free(Ecore_X_Cursor c)
{
   XFreeCursor(_ecore_x_disp, c);
}

/*
 * Returns the cursor for the given shape.
 * Note that the return value must not be freed with
 * ecore_x_cursor_free()!
 */
EAPI Ecore_X_Cursor
ecore_x_cursor_shape_get(int shape)
{
   /* Shapes are defined in Ecore_X_Cursor.h */
   return XCreateFontCursor(_ecore_x_disp, shape);
}

EAPI void
ecore_x_cursor_size_set(int size)
{
#ifdef ECORE_XCURSOR
   XcursorSetDefaultSize(_ecore_x_disp, size);
#else
   size = 0;
#endif
}

EAPI int
ecore_x_cursor_size_get(void)
{
#ifdef ECORE_XCURSOR
   return XcursorGetDefaultSize(_ecore_x_disp);
#else
   return 0;
#endif
}
