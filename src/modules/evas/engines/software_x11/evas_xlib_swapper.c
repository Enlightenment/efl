#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "evas_common.h"
#include "evas_macros.h"
#include "evas_xlib_swapper.h"

typedef struct
{
   XImage          *xim;
   XShmSegmentInfo  shm_info;
   void            *data;
   int              w, h, bpl;
   Eina_Bool        shm : 1;
   Eina_Bool        valid : 1;
} Buffer;

struct _X_Swapper
{
   Display  *disp;
   Drawable  draw;
   Visual   *vis;
   GC        gc;
   Buffer    buf[3];
   int       w, h, depth;
   int       buf_cur, buf_num;
   Eina_Bool mapped: 1;
};

static Eina_Bool _x_err = EINA_FALSE;

static void
_x_err_hand(Display *d EINA_UNUSED, XErrorEvent *ev EINA_UNUSED)
{
   _x_err = 1;
   return;
}

static Eina_Bool
_buf_new(X_Swapper *swp, Buffer *buf)
{
   buf->w = swp->w;
   buf->h = swp->h;
   
   // try shm first
   buf->xim = XShmCreateImage(swp->disp, swp->vis, swp->depth, ZPixmap,
                              NULL, &(buf->shm_info), buf->w, buf->h);
   if (buf->xim)
     {
        buf->bpl = buf->xim->bytes_per_line;
        
        buf->shm_info.shmid = shmget(IPC_PRIVATE, buf->bpl * buf->h,
                                     IPC_CREAT | 0777);
        if (buf->shm_info.shmid >= 0)
          {
             buf->shm_info.readOnly = False;
             buf->shm_info.shmaddr = buf->data = buf->xim->data =
               shmat(buf->shm_info.shmid, 0, 0);
             if (buf->shm_info.shmaddr != ((void *)-1))
               {
                  XErrorHandler ph;
                  
                  XSync(swp->disp, False);
                  _x_err = EINA_FALSE;
                  ph = XSetErrorHandler((XErrorHandler)_x_err_hand);
                  XShmAttach(swp->disp, &(buf->shm_info));
                  XSync(swp->disp, False);
                  XSetErrorHandler((XErrorHandler)ph);
                  if (!_x_err)
                    {
                       buf->shm = EINA_TRUE;
                    }
                  else
                    {
                       shmdt(buf->shm_info.shmaddr);
                       shmctl(buf->shm_info.shmid, IPC_RMID, 0);
                       XDestroyImage(buf->xim);
                       buf->xim = NULL;
                    }
               }
             else
               {
                  shmdt(buf->shm_info.shmaddr);
                  shmctl(buf->shm_info.shmid, IPC_RMID, 0);
                  XDestroyImage(buf->xim);
                  buf->xim = NULL;
               }
          }
        else
          {
             XDestroyImage(buf->xim);
             buf->xim = NULL;
          }
     }

   if (!buf->xim) // shm failed - try normal ximage
     {
        buf->xim = XCreateImage(swp->disp, swp->vis, swp->depth, ZPixmap, 
                                0, NULL, buf->w, buf->h, 32, 0);
        if (!buf->xim) return EINA_FALSE;
        buf->bpl = buf->xim->bytes_per_line;
        buf->data = buf->xim->data = malloc(buf->bpl * buf->h);
        if (!buf->data)
          {
             XDestroyImage(buf->xim);
             buf->xim = NULL;
             return EINA_FALSE;
          }
     }
   return EINA_TRUE;
}

static void
_buf_free(X_Swapper *swp, Buffer *buf)
{
   if (!buf->xim) return;
   if (buf->shm)
     {
        XShmDetach(swp->disp, &(buf->shm_info));
        XDestroyImage(buf->xim);
        shmdt(buf->shm_info.shmaddr);
        shmctl(buf->shm_info.shmid, IPC_RMID, 0);
     }
   else
     {
        XDestroyImage(buf->xim);
     }
   buf->xim = NULL;
   buf->shm = EINA_FALSE;
}

static void
_buf_put(X_Swapper *swp, Buffer *buf, Eina_Rectangle *rects, int nrects)
{
   Region tmpr;
   int i;
   
   if (!buf->xim) return;
   tmpr = XCreateRegion();
   if (rects)
     {
        for (i = 0; i < nrects; i++)
          {
             XRectangle xr;
             
             xr.x = rects[i].x; xr.y = rects[i].y;
             xr.width = rects[i].w; xr.height = rects[i].h;
             XUnionRectWithRegion(&xr, tmpr, tmpr);
          }
     }
   else
     {
        XRectangle xr;
        
        xr.x = 0; xr.y = 0;
        xr.width = buf->w; xr.height = buf->h;
        XUnionRectWithRegion(&xr, tmpr, tmpr);
     }
   XSetRegion(swp->disp, swp->gc, tmpr);
   XDestroyRegion(tmpr);
   if (buf->shm)
     {
        XShmPutImage(swp->disp, swp->draw, swp->gc, buf->xim, 0, 0, 0, 0,
                     buf->w, buf->h, False);
     }
   else
     {
        XPutImage(swp->disp, swp->draw, swp->gc, buf->xim, 0, 0, 0, 0,
                  buf->w, buf->h);
     }
   XSync(swp->disp, False);
}

////////////////////////////////////

X_Swapper *
evas_xlib_swapper_new(Display *disp, Drawable draw, Visual *vis,
                      int depth, int w, int h)
{
   X_Swapper *swp;
   XGCValues gcv;
   int i;
   
   int nbuf = 3; // pretend we are triple buffered
   
   swp = calloc(1, sizeof(X_Swapper));
   if (!swp) return NULL;
   swp->disp = disp;
   swp->draw = draw;
   swp->vis = vis;
   swp->depth = depth;
   swp->w = w;
   swp->h = h;
   swp->buf_num = nbuf;
   swp->gc = XCreateGC(swp->disp, swp->draw, 0, &gcv);
   for (i = 0; i < swp->buf_num; i++)
     {
        if (!_buf_new(swp, &(swp->buf[i])))
          {
             evas_xlib_swapper_free(swp);
             return NULL;
          }
     }
   return swp;
}

void
evas_xlib_swapper_free(X_Swapper *swp)
{
   int i;

   for (i = 0; i < swp->buf_num; i++)
     {
        _buf_free(swp, &(swp->buf[i]));
     }
   free(swp);
}

void *
evas_xlib_swapper_buffer_map(X_Swapper *swp, int *bpl)
{
   swp->mapped = EINA_TRUE;
   if (bpl) *bpl = swp->buf[swp->buf_cur].bpl;
   return swp->buf[swp->buf_cur].data;
}

void
evas_xlib_swapper_buffer_unmap(X_Swapper *swp)
{
   swp->mapped = EINA_FALSE;
}

void
evas_xlib_swapper_swap(X_Swapper *swp, Eina_Rectangle *rects, int nrects)
{
   int n;
   
   n = swp->buf_cur;
   _buf_put(swp, &(swp->buf[n]), rects, nrects);
   swp->buf[n].valid = 1;
   swp->buf_cur = (swp->buf_cur + 1) % swp->buf_num;
}

int
evas_xlib_swapper_buffer_state_get(X_Swapper *swp)
{
   int i, n, count = 0;
   
   for (i = 0; i < swp->buf_num; i++)
     {
        n = (swp->buf_num + swp->buf_cur - (i)) % swp->buf_num;
        if (swp->buf[n].valid) count++;
        else break;
     }
   if (count == swp->buf_num)
     {
        if (count == 1) return MODE_COPY;
        else if (count == 2) return MODE_DOUBLE;
        else if (count == 3) return MODE_TRIPLE;
     }
   return MODE_FULL;
}

int
evas_xlib_swapper_depth_get(X_Swapper *swp)
{
   return swp->buf[0].xim->bits_per_pixel;
}

int
evas_xlib_swapper_byte_order_get(X_Swapper *swp)
{
   return swp->buf[0].xim->byte_order;
}

int
evas_xlib_swapper_bit_order_get(X_Swapper *swp)
{
   return swp->buf[0].xim->bitmap_bit_order;
}
