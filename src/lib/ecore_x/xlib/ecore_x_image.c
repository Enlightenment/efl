#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif

#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "ecore_x_private.h"
#include "Ecore_X.h"

#include <X11/extensions/XShm.h>
#include <X11/Xutil.h>

static int _ecore_x_image_shm_can = -1;
static int _ecore_x_image_err = 0;

static int
_ecore_x_image_error_handler(Display *d EINA_UNUSED, XErrorEvent *ev)
{
   _ecore_x_image_err = 1;
   switch (ev->error_code)
     {
      case BadRequest:	/* bad request code */
        ERR("BadRequest");
        break;
      case BadValue:	/* int parameter out of range */
        ERR("BadValue");
        break;
      case BadWindow:	/* parameter not a Window */
        ERR("BadWindow");
        break;
      case BadPixmap:	/* parameter not a Pixmap */
        ERR("BadPixmap");
        break;
      case BadAtom:	/* parameter not an Atom */
        ERR("BadAtom");
        break;
      case BadCursor:	/* parameter not a Cursor */
        ERR("BadCursor");
        break;
      case BadFont:	/* parameter not a Font */
        ERR("BadFont");
        break;
      case BadMatch:	/* parameter mismatch */
        ERR("BadMatch");
        break;
      case BadDrawable:	/* parameter not a Pixmap or Window */
        ERR("BadDrawable");
        break;
      case BadAccess:	/* depending on context */
        ERR("BadAccess");
        break;
      case BadAlloc:	/* insufficient resources */
        ERR("BadAlloc");
        break;
      case BadColor:	/* no such colormap */
        ERR("BadColor");
        break;
      case BadGC:	/* parameter not a GC */
        ERR("BadGC");
        break;
      case BadIDChoice:	/* choice not in range or already used */
        ERR("BadIDChoice");
        break;
      case BadName:	/* font or color name doesn't exist */
        ERR("BadName");
        break;
      case BadLength:	/* Request length incorrect */
        ERR("BadLength");
        break;
      case BadImplementation:	/* server is defective */
        ERR("BadImplementation");
        break;
     }
   return 0;
}

static void
_ecore_x_image_shm_check(void)
{
   XErrorHandler ph;
   XShmSegmentInfo shminfo;
   XImage *xim;

   if (_ecore_x_image_shm_can != -1)
     return;

   if (!XShmQueryExtension(_ecore_x_disp))
     {
        _ecore_x_image_shm_can = 0;
        return;
     }
   
   XSync(_ecore_x_disp, False);
   _ecore_x_image_err = 0;

   xim = XShmCreateImage(_ecore_x_disp,
                         DefaultVisual(_ecore_x_disp,
                                       DefaultScreen(_ecore_x_disp)),
                         DefaultDepth(_ecore_x_disp,
                                      DefaultScreen(_ecore_x_disp)),
                         ZPixmap, NULL,
                         &shminfo, 1, 1);
   if (_ecore_xlib_sync) ecore_x_sync();
   if (!xim)
     {
        _ecore_x_image_shm_can = 0;
        return;
     }

   shminfo.shmid = shmget(IPC_PRIVATE, xim->bytes_per_line * xim->height,
                          IPC_CREAT | 0666);
   if (shminfo.shmid == -1)
     {
        ERR("%s", strerror(errno));
        XDestroyImage(xim);
        _ecore_x_image_shm_can = 0;
        return;
     }

   shminfo.readOnly = False;
   shminfo.shmaddr = shmat(shminfo.shmid, 0, 0);
   xim->data = shminfo.shmaddr;

   if (xim->data == (char *)-1)
     {
        XDestroyImage(xim);
        _ecore_x_image_shm_can = 0;
        return;
     }

   ph = XSetErrorHandler((XErrorHandler)_ecore_x_image_error_handler);
   XShmAttach(_ecore_x_disp, &shminfo);
   XShmGetImage(_ecore_x_disp, DefaultRootWindow(_ecore_x_disp),
                xim, 0, 0, 0xffffffff);
   XSync(_ecore_x_disp, False);
   XSetErrorHandler((XErrorHandler)ph);
   if (_ecore_x_image_err)
     {
        XShmDetach(_ecore_x_disp, &shminfo);
        XDestroyImage(xim);
        shmdt(shminfo.shmaddr);
        shmctl(shminfo.shmid, IPC_RMID, 0);
        _ecore_x_image_shm_can = 0;
        return;
     }

   XShmDetach(_ecore_x_disp, &shminfo);
   XDestroyImage(xim);
   shmdt(shminfo.shmaddr);
   shmctl(shminfo.shmid, IPC_RMID, 0);

   _ecore_x_image_shm_can = 1;
}

struct _Ecore_X_Image
{
   XShmSegmentInfo shminfo;
   Ecore_X_Visual  vis;
   XImage         *xim;
   int             depth;
   int             w, h;
   int             bpl, bpp, rows;
   unsigned char  *data;
   Eina_Bool       shm : 1;
};

EAPI Ecore_X_Image *
ecore_x_image_new(int w,
                  int h,
                  Ecore_X_Visual vis,
                  int depth)
{
   Ecore_X_Image *im;

   im = calloc(1, sizeof(Ecore_X_Image));
   if (!im)
     return NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   im->w = w;
   im->h = h;
   im->vis = vis;
   im->depth = depth;
   _ecore_x_image_shm_check();
   im->shm = _ecore_x_image_shm_can;
   return im;
}

EAPI void
ecore_x_image_free(Ecore_X_Image *im)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (im->shm)
     {
        if (im->xim)
          {
             XShmDetach(_ecore_x_disp, &(im->shminfo));
             XDestroyImage(im->xim);
             shmdt(im->shminfo.shmaddr);
             shmctl(im->shminfo.shmid, IPC_RMID, 0);
          }
     }
   else if (im->xim)
     {
        free(im->xim->data);
        im->xim->data = NULL;
        XDestroyImage(im->xim);
     }

   free(im);
}

static void
_ecore_x_image_finalize(Ecore_X_Image *im)
{
   im->data = (unsigned char *)im->xim->data;
   im->bpl = im->xim->bytes_per_line;
   im->rows = im->xim->height;
   if (im->xim->bits_per_pixel <= 8) im->bpp = 1;
   else if (im->xim->bits_per_pixel <= 16) im->bpp = 2;
   else if (im->xim->bits_per_pixel <= 24) im->bpp = 3;
   else im->bpp = 4;
}

static void
_ecore_x_image_shm_create(Ecore_X_Image *im)
{
   im->xim = XShmCreateImage(_ecore_x_disp, im->vis, im->depth,
                             ZPixmap, NULL, &(im->shminfo),
                             im->w, im->h);
   if (!im->xim)
     return;

   im->shminfo.shmid = shmget(IPC_PRIVATE,
                              im->xim->bytes_per_line * im->xim->height,
                              IPC_CREAT | 0666);
   if (im->shminfo.shmid == -1)
     {
        ERR("shmget failed: %s", strerror(errno));
        XDestroyImage(im->xim);
        im->xim = NULL;
        return;
     }

   im->shminfo.readOnly = False;
   im->shminfo.shmaddr = shmat(im->shminfo.shmid, 0, 0);
   im->xim->data = im->shminfo.shmaddr;
   if ((im->xim->data == (char *)-1) ||
       (!im->xim->data))
     {
        shmdt(im->shminfo.shmaddr);
        shmctl(im->shminfo.shmid, IPC_RMID, 0);
        XDestroyImage(im->xim);
        im->xim = NULL;
        return;
     }

   XShmAttach(_ecore_x_disp, &im->shminfo);
   _ecore_x_image_finalize(im);
}

static void
_ecore_x_image_create(Ecore_X_Image *im)
{
   im->xim = XCreateImage(_ecore_x_disp, im->vis, im->depth,
                          ZPixmap, 0, NULL, im->w, im->h, 32, 0);
   if (!im->xim) return;
   im->xim->data = malloc(im->xim->bytes_per_line * im->h);
   if (!im->xim->data)
     {
        XDestroyImage(im->xim);
        im->xim = NULL;
        return;
     }
   _ecore_x_image_finalize(im);
}

EAPI Eina_Bool
ecore_x_image_get(Ecore_X_Image *im,
                  Ecore_X_Drawable draw,
                  int x,
                  int y,
                  int sx,
                  int sy,
                  int w,
                  int h)
{
   Eina_Bool ret = EINA_TRUE;
   XErrorHandler ph;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (im->shm)
     {
        
        if (!im->xim)
          _ecore_x_image_shm_create(im);

        if (!im->xim)
          {
             fprintf(stderr, "BLAAAAAAAAAAAAH\n");
             abort();
             return EINA_FALSE;
          }

        _ecore_x_image_err = 0;
        
        ecore_x_sync();
        // optimised path
        ph = XSetErrorHandler((XErrorHandler)_ecore_x_image_error_handler);
        if ((sx == 0) && (w == im->w))
          {
             im->xim->data = (char *)
               im->data + (im->xim->bytes_per_line * sy) + (sx * im->bpp);
             im->xim->width = MIN(w, im->w);
             im->xim->height = MIN(h, im->h);
             XGrabServer(_ecore_x_disp);
             if (!XShmGetImage(_ecore_x_disp, draw, im->xim, x, y, 0xffffffff))
               ret = EINA_FALSE;
             XUngrabServer(_ecore_x_disp);
             ecore_x_sync();
          }
        // unavoidable thanks to mit-shm get api - tmp shm buf + copy into it
        else
          {
             Ecore_X_Image *tim;
             unsigned char *spixels, *sp, *pixels, *p;
             int bpp, bpl, rows, sbpp, sbpl, srows;
             int r;

             tim = ecore_x_image_new(w, h, im->vis, im->depth);
             if (tim)
               {
                  ret = ecore_x_image_get(tim, draw, x, y, 0, 0, w, h);
                  if (ret)
                    {
                       spixels = ecore_x_image_data_get(tim,
                                                        &sbpl,
                                                        &srows,
                                                        &sbpp);
                       pixels = ecore_x_image_data_get(im, &bpl, &rows, &bpp);
                       if ((pixels) && (spixels))
                         {
                            p = pixels + (sy * bpl) + (sx * bpp);
                            sp = spixels;
                            for (r = srows; r > 0; r--)
                              {
                                 memcpy(p, sp, sbpl);
                                 p += bpl;
                                 sp += sbpl;
                              }
                         }
                    }

                  ecore_x_image_free(tim);
               }
          }

        XSetErrorHandler((XErrorHandler)ph);
        if (_ecore_x_image_err)
          ret = EINA_FALSE;
     }
   else
     {
        if (!im->xim)
          _ecore_x_image_create(im);

        if (!im->xim)
          return EINA_FALSE;

        if (XGetSubImage(_ecore_x_disp, draw, sx, sy, w, h,
                         0xffffffff, ZPixmap, im->xim, x, y) != im->xim)
          ret = EINA_FALSE;
     }

   return ret;
}

EAPI void
ecore_x_image_put(Ecore_X_Image *im,
                  Ecore_X_Drawable draw,
                  Ecore_X_GC gc,
                  int x,
                  int y,
                  int sx,
                  int sy,
                  int w,
                  int h)
{
   Ecore_X_GC tgc = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (!gc)
     {
        XGCValues gcv;
        memset(&gcv, 0, sizeof(gcv));
        gcv.subwindow_mode = IncludeInferiors;
        tgc = XCreateGC(_ecore_x_disp, draw, GCSubwindowMode, &gcv);
        if (_ecore_xlib_sync) ecore_x_sync();
        gc = tgc;
     }
   if (!im->xim)
     {
        if (im->shm) _ecore_x_image_shm_create(im);
        else _ecore_x_image_create(im);
     }
   if (im->xim)
     {
        if (im->shm)
          XShmPutImage(_ecore_x_disp, draw, gc, im->xim,
                       sx, sy, x, y, w, h, False);
        else
          XPutImage(_ecore_x_disp, draw, gc, im->xim,
                    sx, sy, x, y, w, h);
        if (_ecore_xlib_sync) ecore_x_sync();
     }
   if (tgc) ecore_x_gc_free(tgc);
}

EAPI void *
ecore_x_image_data_get(Ecore_X_Image *im,
                       int *bpl,
                       int *rows,
                       int *bpp)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (!im->xim)
     {
        if (im->shm) _ecore_x_image_shm_create(im);
        else _ecore_x_image_create(im);
        if (!im->xim) return NULL;
     }
   if (bpl) *bpl = im->bpl;
   if (rows) *rows = im->rows;
   if (bpp) *bpp = im->bpp;
   return im->data;
}

EAPI Eina_Bool
ecore_x_image_is_argb32_get(Ecore_X_Image *im)
{
   Visual *vis = im->vis;
   if (!im->xim)
     {
        if (im->shm) _ecore_x_image_shm_create(im);
        else _ecore_x_image_create(im);
        if (!im->xim) return EINA_FALSE;
     }
   if (((vis->class == TrueColor) ||
        (vis->class == DirectColor)) &&
       (im->bpp == 4) &&
       (vis->red_mask == 0xff0000) &&
       (vis->green_mask == 0x00ff00) &&
       (vis->blue_mask == 0x0000ff))
     {
#ifdef WORDS_BIGENDIAN
        if (im->xim->bitmap_bit_order == MSBFirst) return EINA_TRUE;
#else
        if (im->xim->bitmap_bit_order == LSBFirst) return EINA_TRUE;
#endif
     }
   return EINA_FALSE;
}

EAPI Eina_Bool
ecore_x_image_to_argb_convert(void *src,
                              int sbpp,
                              int sbpl,
                              Ecore_X_Colormap c,
                              Ecore_X_Visual v,
                              int x,
                              int y,
                              int w,
                              int h,
                              unsigned int *dst,
                              int dbpl,
                              int dx,
                              int dy)
{
   Visual *vis = v;
   XColor *cols = NULL;
   int n = 0, nret = 0, i, row;
   unsigned int pal[256], r, g, b;
   enum
   {
      rgbnone = 0,
      rgb565,
      bgr565,
      rgbx555,
      rgb888,
      bgr888,
      argbx888,
      abgrx888,
      rgba888x,
      bgra888x,
      argbx666
   };
   int mode = 0;

   sbpp *= 8;

   n = vis->map_entries;
   if ((n <= 256) &&
       ((vis->class == PseudoColor) ||
        (vis->class == StaticColor) ||
        (vis->class == GrayScale) ||
        (vis->class == StaticGray)))
     {
        if (!c)
          c = DefaultColormap(_ecore_x_disp,
                              DefaultScreen(_ecore_x_disp));
        cols = alloca(n * sizeof(XColor));
        for (i = 0; i < n; i++)
          {
             cols[i].pixel = i;
             cols[i].flags = DoRed | DoGreen | DoBlue;
             cols[i].red = 0;
             cols[i].green = 0;
             cols[i].blue = 0;
          }
        XQueryColors(_ecore_x_disp, c, cols, n);
        for (i = 0; i < n; i++)
          {
             pal[i] = 0xff000000 |
               ((cols[i].red >> 8) << 16) |
               ((cols[i].green >> 8) << 8) |
               ((cols[i].blue >> 8));
          }
        nret = n;
     }
   else if ((vis->class == TrueColor) ||
            (vis->class == DirectColor))
     {
        if (sbpp == 24)
          {
             if ((vis->red_mask == 0x00ff0000) &&
                 (vis->green_mask == 0x0000ff00) &&
                 (vis->blue_mask == 0x000000ff))
               mode = rgb888;
             else if ((vis->red_mask == 0x000000ff) &&
                      (vis->green_mask == 0x0000ff00) &&
                      (vis->blue_mask == 0x00ff0000))
               mode = bgr888;
             else
               return EINA_FALSE;
          }
        else
          {
             if ((vis->red_mask == 0x00ff0000) &&
                 (vis->green_mask == 0x0000ff00) &&
                 (vis->blue_mask == 0x000000ff))
               mode = argbx888;
             else if ((vis->red_mask == 0x000000ff) &&
                      (vis->green_mask == 0x0000ff00) &&
                      (vis->blue_mask == 0x00ff0000))
               mode = abgrx888;
             else if ((vis->red_mask == 0xff000000) &&
                      (vis->green_mask == 0x00ff0000) &&
                      (vis->blue_mask == 0x0000ff00))
               mode = rgba888x;
             else if ((vis->red_mask == 0x0000ff00) &&
                      (vis->green_mask == 0x00ff0000) &&
                      (vis->blue_mask == 0xff000000))
               mode = bgra888x;
             else if ((vis->red_mask == 0x0003f000) &&
                      (vis->green_mask == 0x00000fc0) &&
                      (vis->blue_mask == 0x0000003f))
               mode = argbx666;
             else if ((vis->red_mask == 0x0000f800) &&
                      (vis->green_mask == 0x000007e0) &&
                      (vis->blue_mask == 0x0000001f))
               mode = rgb565;
             else if ((vis->red_mask == 0x0000001f) &&
                      (vis->green_mask == 0x000007e0) &&
                      (vis->blue_mask == 0x0000f800))
               mode = bgr565;
             else if ((vis->red_mask == 0x00007c00) &&
                      (vis->green_mask == 0x000003e0) &&
                      (vis->blue_mask == 0x0000001f))
               mode = rgbx555;
             else
               return EINA_FALSE;
          }
     }
   for (row = 0; row < h; row++)
     {
        unsigned char *s8;
        unsigned short *s16;
        unsigned int *s32;
        unsigned int *dp, *de;

        dp = ((unsigned int *)(((unsigned char *)dst) +
                               ((dy + row) * dbpl))) + dx;
        de = dp + w;
        switch (sbpp)
          {
           case 8:
             s8 = ((unsigned char *)(((unsigned char *)src) + ((y + row) * sbpl))) + x;
             if (nret > 0)
               {
                  while (dp < de)
                    {
                       *dp = pal[*s8];
                       s8++; dp++;
                    }
               }
             else
               return EINA_FALSE;
             break;

           case 16:
             s16 = ((unsigned short *)(((unsigned char *)src) + ((y + row) * sbpl))) + x;
             switch (mode)
               {
                case rgb565:
                  while (dp < de)
                    {
                       r = (*s16 & 0xf800) << 8;
                       g = (*s16 & 0x07e0) << 5;
                       b = (*s16 & 0x001f) << 3;
                       r |= (r >> 5) & 0xff0000;
                       g |= (g >> 6) & 0x00ff00;
                       b |= (b >> 5);
                       *dp = 0xff000000 | r | g | b;
                       s16++; dp++;
                    }
                  break;

                case bgr565:
                  while (dp < de)
                    {
                       r = (*s16 & 0x001f) << 19;
                       g = (*s16 & 0x07e0) << 5;
                       b = (*s16 & 0xf800) >> 8;
                       r |= (r >> 5) & 0xff0000;
                       g |= (g >> 6) & 0x00ff00;
                       b |= (b >> 5);
                       *dp = 0xff000000 | r | g | b;
                       s16++; dp++;
                    }
                  break;

                case rgbx555:
                  while (dp < de)
                    {
                       r = (*s16 & 0x7c00) << 9;
                       g = (*s16 & 0x03e0) << 6;
                       b = (*s16 & 0x001f) << 3;
                       r |= (r >> 5) & 0xff0000;
                       g |= (g >> 5) & 0x00ff00;
                       b |= (b >> 5);
                       *dp = 0xff000000 | r | g | b;
                       s16++; dp++;
                    }
                  break;

                default:
                  return EINA_FALSE;
                  break;
               }
             break;

           case 24:
             s8 = ((unsigned char *)(((unsigned char *)src) + ((y + row) * sbpl))) + (x * (sbpp / 8));
             switch (mode)
               {
                case rgb888:
                  while (dp < de)
                    {
                       *dp = 0xff000000 | (s8[2] << 16) | (s8[1] << 8) | s8[0];
                       s8 += 3; dp++;
                    }
                  break;
                case bgr888:
                  while (dp < de)
                    {
                       *dp = 0xff000000 | (s8[0] << 16) | (s8[1] << 8) | s8[2];
                       s8 += 3; dp++;
                    }
                  break;
                default:
                  return EINA_FALSE;
                  break;
               }
             break;
             
           case 32:
             s32 = ((unsigned int *)(((unsigned char *)src) + ((y + row) * sbpl))) + x;
             switch (mode)
               {
                case argbx888:
                  while (dp < de)
                    {
                       *dp = 0xff000000 | *s32;
                       s32++; dp++;
                    }
                  break;

                case abgrx888:
                  while (dp < de)
                    {
                       r = *s32 & 0x000000ff;
                       g = *s32 & 0x0000ff00;
                       b = *s32 & 0x00ff0000;
                       *dp = 0xff000000 | (r << 16) | (g) | (b >> 16);
                       s32++; dp++;
                    }
                  break;

                case rgba888x:
                  while (dp < de)
                    {
                       *dp = 0xff000000 | (*s32 >> 8);
                       s32++; dp++;
                    }
                  break;

                case bgra888x:
                  while (dp < de)
                    {
                       r = *s32 & 0x0000ff00;
                       g = *s32 & 0x00ff0000;
                       b = *s32 & 0xff000000;
                       *dp = 0xff000000 | (r << 8) | (g >> 8) | (b >> 24);
                       s32++; dp++;
                    }
                  break;

                case argbx666:
                  while (dp < de)
                    {
                       r = (*s32 & 0x3f000) << 6;
                       g = (*s32 & 0x00fc0) << 4;
                       b = (*s32 & 0x0003f) << 2;
                       r |= (r >> 6) & 0xff0000;
                       g |= (g >> 6) & 0x00ff00;
                       b |= (b >> 6);
                       *dp = 0xff000000 | r | g | b;
                       s32++; dp++;
                    }
                  break;

                default:
                  return EINA_FALSE;
                  break;
               }
             break;

           default:
             return EINA_FALSE;
             break;
          }
     }
   return EINA_TRUE;
}
