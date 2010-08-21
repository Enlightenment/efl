#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include "ecore_x_private.h"
#include "Ecore_X.h"

#include <X11/extensions/XShm.h>
#include <X11/Xutil.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

static int _ecore_x_image_shm_can = -1;
static int _ecore_x_image_err = 0;

static void
_ecore_x_image_error_handler(Display *d __UNUSED__, XErrorEvent *ev __UNUSED__)
{
   _ecore_x_image_err = 1;
} /* _ecore_x_image_error_handler */

static void
_ecore_x_image_shm_check(void)
{
   XErrorHandler ph;
   XShmSegmentInfo shminfo;
   XImage *xim;

   if (_ecore_x_image_shm_can != -1)
      return;

   XSync(_ecore_x_disp, False);
   _ecore_x_image_err = 0;

   xim = XShmCreateImage(_ecore_x_disp,
                         DefaultVisual(_ecore_x_disp,
                                       DefaultScreen(_ecore_x_disp)),
                         DefaultDepth(_ecore_x_disp,
                                      DefaultScreen(_ecore_x_disp)),
                         ZPixmap, NULL,
                         &shminfo, 1, 1);
   if (!xim)
     {
        _ecore_x_image_shm_can = 0;
        return;
     }

   shminfo.shmid = shmget(IPC_PRIVATE, xim->bytes_per_line * xim->height,
                          IPC_CREAT | 0666);
   if (shminfo.shmid == -1)
     {
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
} /* _ecore_x_image_shm_check */

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
ecore_x_image_new(int w, int h, Ecore_X_Visual vis, int depth)
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
} /* ecore_x_image_new */

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
} /* ecore_x_image_free */

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
        XDestroyImage(im->xim);
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
        return;
     }

   XShmAttach(_ecore_x_disp, &im->shminfo);

   im->data = (unsigned char *)im->xim->data;

   im->bpl = im->xim->bytes_per_line;
   im->rows = im->xim->height;
   if (im->xim->bits_per_pixel <= 8)
      im->bpp = 1;
   else if (im->xim->bits_per_pixel <= 16)
      im->bpp = 2;
   else
      im->bpp = 4;
} /* _ecore_x_image_shm_create */

EAPI Eina_Bool
ecore_x_image_get(Ecore_X_Image *im, Ecore_X_Drawable draw,
                  int x, int y, int sx, int sy, int w, int h)
{
   Eina_Bool ret = EINA_TRUE;
   XErrorHandler ph;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (im->shm)
     {
        if (!im->xim)
           _ecore_x_image_shm_create(im);

        if (!im->xim)
           return 0;

        _ecore_x_image_err = 0;
        // optimised path
        ph = XSetErrorHandler((XErrorHandler)_ecore_x_image_error_handler);
        if ((sx == 0) && (w == im->w))
          {
             im->xim->data = (char *)
                im->data + (im->xim->bytes_per_line * sy) + (sx * im->bpp);
             im->xim->width = w;
             im->xim->height = h;
             if (!XShmGetImage(_ecore_x_disp, draw, im->xim, x, y, 0xffffffff))
                ret = EINA_FALSE;

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
        printf("currently unimplemented ecore_x_image_get without shm\n");
        ret = EINA_FALSE;
     }

   return ret;
} /* ecore_x_image_get */

EAPI void
ecore_x_image_put(Ecore_X_Image *im     __UNUSED__,
                  Ecore_X_Drawable draw __UNUSED__,
                  int x                 __UNUSED__,
                  int y                 __UNUSED__,
                  int sx                __UNUSED__,
                  int sy                __UNUSED__,
                  int w                 __UNUSED__,
                  int h                 __UNUSED__)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   printf("ecore_x_image_put: unimplemented!\n");
} /* ecore_x_image_put */

EAPI void *
ecore_x_image_data_get(Ecore_X_Image *im, int *bpl, int *rows, int *bpp)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (!im->xim)
      _ecore_x_image_shm_create(im);

   if (!im->xim)
      return NULL;

   if (bpl)
      *bpl = im->bpl;

   if (rows)
      *rows = im->rows;

   if (bpp)
      *bpp = im->bpp;

   return im->data;
} /* ecore_x_image_data_get */

