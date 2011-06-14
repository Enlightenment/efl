#include "evas_common.h"

#include "evas_xlib_buffer.h"

static int _x_err = 0;

void
evas_software_xlib_x_write_mask_line(Outbuf *buf, X_Output_Buffer *xob, DATA32 *src, int w, int y)
{
   int x;
   DATA32 *src_ptr;
   DATA8 *dst_ptr;
   int bpl = 0;

   src_ptr = src;
   dst_ptr = evas_software_xlib_x_output_buffer_data(xob, &bpl);
   dst_ptr = dst_ptr + (bpl * y);
   w -= 7;
   if (buf->priv.x11.xlib.bit_swap)
     {
	for (x = 0; x < w; x += 8)
	  {
	     *dst_ptr =
	       ((A_VAL(&(src_ptr[0])) >> 7) << 7) |
	       ((A_VAL(&(src_ptr[1])) >> 7) << 6) |
	       ((A_VAL(&(src_ptr[2])) >> 7) << 5) |
	       ((A_VAL(&(src_ptr[3])) >> 7) << 4) |
	       ((A_VAL(&(src_ptr[4])) >> 7) << 3) |
	       ((A_VAL(&(src_ptr[5])) >> 7) << 2) |
	       ((A_VAL(&(src_ptr[6])) >> 7) << 1) |
	       ((A_VAL(&(src_ptr[7])) >> 7) << 0);
	     src_ptr += 8;
	     dst_ptr++;
	  }
     }
   else
     {
	for (x = 0; x < w; x += 8)
	  {
	     *dst_ptr =
	       ((A_VAL(&(src_ptr[0])) >> 7) << 0) |
	       ((A_VAL(&(src_ptr[1])) >> 7) << 1) |
	       ((A_VAL(&(src_ptr[2])) >> 7) << 2) |
	       ((A_VAL(&(src_ptr[3])) >> 7) << 3) |
	       ((A_VAL(&(src_ptr[4])) >> 7) << 4) |
	       ((A_VAL(&(src_ptr[5])) >> 7) << 5) |
	       ((A_VAL(&(src_ptr[6])) >> 7) << 6) |
	       ((A_VAL(&(src_ptr[7])) >> 7) << 7);
	     src_ptr += 8;
	     dst_ptr++;
	  }
     }
   w += 7;
   for (; x < w; x ++)
     {
	XPutPixel(xob->xim, x, y, A_VAL(src_ptr) >> 7);
	src_ptr++;
     }
}

void
evas_software_xlib_x_write_mask_line_rev(Outbuf *buf, X_Output_Buffer *xob, DATA32 *src, int w, int y)
{
   int x;
   DATA32 *src_ptr;
   DATA8 *dst_ptr;
   int bpl = 0;

   src_ptr = src + w - 1;
   dst_ptr = evas_software_xlib_x_output_buffer_data(xob, &bpl);
   dst_ptr = dst_ptr + (bpl * y);
   w -= 7;
   if (buf->priv.x11.xlib.bit_swap)
     {
	for (x = 0; x < w; x += 8)
	  {
	     *dst_ptr =
	       ((A_VAL(&(src_ptr[ 0])) >> 7) << 7) |
	       ((A_VAL(&(src_ptr[-1])) >> 7) << 6) |
	       ((A_VAL(&(src_ptr[-2])) >> 7) << 5) |
	       ((A_VAL(&(src_ptr[-3])) >> 7) << 4) |
	       ((A_VAL(&(src_ptr[-4])) >> 7) << 3) |
	       ((A_VAL(&(src_ptr[-5])) >> 7) << 2) |
	       ((A_VAL(&(src_ptr[-6])) >> 7) << 1) |
	       ((A_VAL(&(src_ptr[-7])) >> 7) << 0);
	     src_ptr -= 8;
	     dst_ptr++;
	  }
     }
   else
     {
	for (x = 0; x < w; x += 8)
	  {
	     *dst_ptr =
	       ((A_VAL(&(src_ptr[ 0])) >> 7) << 0) |
	       ((A_VAL(&(src_ptr[-1])) >> 7) << 1) |
	       ((A_VAL(&(src_ptr[-2])) >> 7) << 2) |
	       ((A_VAL(&(src_ptr[-3])) >> 7) << 3) |
	       ((A_VAL(&(src_ptr[-4])) >> 7) << 4) |
	       ((A_VAL(&(src_ptr[-5])) >> 7) << 5) |
	       ((A_VAL(&(src_ptr[-6])) >> 7) << 6) |
	       ((A_VAL(&(src_ptr[-7])) >> 7) << 7);
	     src_ptr -= 8;
	     dst_ptr++;
	  }
     }
   w += 7;
   for (; x < w; x ++)
     {
	XPutPixel(xob->xim, x, y, A_VAL(src_ptr) >> 7);
	src_ptr--;
     }
}

void
evas_software_xlib_x_write_mask_line_vert(Outbuf *buf, X_Output_Buffer *xob, 
                                          DATA32 *src, 
                                          int h, int ym, int w)
{
   int y;
   DATA32 *src_ptr;
   DATA8 *dst_ptr;
   int bpl = 0;

   src_ptr = src;
   dst_ptr = evas_software_xlib_x_output_buffer_data(xob, &bpl);
   dst_ptr = dst_ptr + (bpl * ym);
   h -= 7;
   if (buf->priv.x11.xlib.bit_swap)
     {
	for (y = 0; y < h; y += 8)
	  {
	     *dst_ptr =
	       ((A_VAL(&(src_ptr[0 * w])) >> 7) << 7) |
	       ((A_VAL(&(src_ptr[1 * w])) >> 7) << 6) |
	       ((A_VAL(&(src_ptr[2 * w])) >> 7) << 5) |
	       ((A_VAL(&(src_ptr[3 * w])) >> 7) << 4) |
	       ((A_VAL(&(src_ptr[4 * w])) >> 7) << 3) |
	       ((A_VAL(&(src_ptr[5 * w])) >> 7) << 2) |
	       ((A_VAL(&(src_ptr[6 * w])) >> 7) << 1) |
	       ((A_VAL(&(src_ptr[7 * w])) >> 7) << 0);
	     src_ptr += 8 * w;
	     dst_ptr++;
	  }
     }
   else
     {
	for (y = 0; y < h; y += 8)
	  {
	     *dst_ptr =
	       ((A_VAL(&(src_ptr[0 * w])) >> 7) << 0) |
	       ((A_VAL(&(src_ptr[1 * w])) >> 7) << 1) |
	       ((A_VAL(&(src_ptr[2 * w])) >> 7) << 2) |
	       ((A_VAL(&(src_ptr[3 * w])) >> 7) << 3) |
	       ((A_VAL(&(src_ptr[4 * w])) >> 7) << 4) |
	       ((A_VAL(&(src_ptr[5 * w])) >> 7) << 5) |
	       ((A_VAL(&(src_ptr[6 * w])) >> 7) << 6) |
	       ((A_VAL(&(src_ptr[7 * w])) >> 7) << 7);
	     src_ptr += 8 * w;
	     dst_ptr++;
	  }
     }
   h += 7;
   for (; y < h; y ++)
     {
	XPutPixel(xob->xim, y, ym, A_VAL(src_ptr) >> 7);
	src_ptr += w;
     }
}

void
evas_software_xlib_x_write_mask_line_vert_rev(Outbuf *buf, X_Output_Buffer *xob, 
                                              DATA32 *src, 
                                              int h, int ym, int w)
{
   int y;
   DATA32 *src_ptr;
   DATA8 *dst_ptr;
   int bpl = 0;

   src_ptr = src + ((h - 1) * w);
   dst_ptr = evas_software_xlib_x_output_buffer_data(xob, &bpl);
   dst_ptr = dst_ptr + (bpl * ym);
   h -= 7;
   if (buf->priv.x11.xlib.bit_swap)
     {
	for (y = 0; y < h; y += 8)
	  {
	     *dst_ptr =
	       ((A_VAL(&(src_ptr[ 0 * w])) >> 7) << 7) |
	       ((A_VAL(&(src_ptr[-1 * w])) >> 7) << 6) |
	       ((A_VAL(&(src_ptr[-2 * w])) >> 7) << 5) |
	       ((A_VAL(&(src_ptr[-3 * w])) >> 7) << 4) |
	       ((A_VAL(&(src_ptr[-4 * w])) >> 7) << 3) |
	       ((A_VAL(&(src_ptr[-5 * w])) >> 7) << 2) |
	       ((A_VAL(&(src_ptr[-6 * w])) >> 7) << 1) |
	       ((A_VAL(&(src_ptr[-7 * w])) >> 7) << 0);
	     src_ptr -= 8 * w;
	     dst_ptr++;
	  }
     }
   else
     {
	for (y = 0; y < h; y += 8)
	  {
	     *dst_ptr =
	       ((A_VAL(&(src_ptr[ 0 * w])) >> 7) << 0) |
	       ((A_VAL(&(src_ptr[-1 * w])) >> 7) << 1) |
	       ((A_VAL(&(src_ptr[-2 * w])) >> 7) << 2) |
	       ((A_VAL(&(src_ptr[-3 * w])) >> 7) << 3) |
	       ((A_VAL(&(src_ptr[-4 * w])) >> 7) << 4) |
	       ((A_VAL(&(src_ptr[-5 * w])) >> 7) << 5) |
	       ((A_VAL(&(src_ptr[-6 * w])) >> 7) << 6) |
	       ((A_VAL(&(src_ptr[-7 * w])) >> 7) << 7);
	     src_ptr -= 8 * w;
	     dst_ptr++;
	  }
     }
   h += 7;
   for (; y < h; y ++)
     {
	XPutPixel(xob->xim, y, ym, A_VAL(src_ptr) >> 7);
	src_ptr -= w;
     }
}

int
evas_software_xlib_x_can_do_shm(Display *d)
{
   static Display *cached_d = NULL;
   static int cached_result = 0;
   
   if (d == cached_d) return cached_result;
   cached_d = d;
   if (XShmQueryExtension(d))
     {
	X_Output_Buffer *xob;

	xob = evas_software_xlib_x_output_buffer_new
	  (d, DefaultVisual(d, DefaultScreen(d)),
	   DefaultDepth(d, DefaultScreen(d)), 16, 16, 2, NULL);
	if (!xob)
	  {
	     cached_result = 0;
	     return 0;
	  }
	evas_software_xlib_x_output_buffer_free(xob, 1);
	cached_result = 1;
	return 1;
     }
   cached_result = 0;
   return 0;
}

static void
x_output_tmp_x_err(Display *d __UNUSED__, XErrorEvent *ev __UNUSED__)
{
   _x_err = 1;
   return;
}

//static int creates = 0;

X_Output_Buffer *
evas_software_xlib_x_output_buffer_new(Display *d, Visual *v, int depth, int w, int h, int try_shm, void *data)
{
   X_Output_Buffer *xob;

   xob = calloc(1, sizeof(X_Output_Buffer));
   if (!xob) return NULL;

   xob->display = d;
   xob->visual = v;
   xob->xim = NULL;
   xob->shm_info = NULL;
   xob->w = w;
   xob->h = h;

   if (try_shm > 0)
     {
	xob->shm_info = malloc(sizeof(XShmSegmentInfo));
	if (xob->shm_info)
	  {
	     xob->xim = XShmCreateImage(d, v, depth, ZPixmap, NULL,
					xob->shm_info, w, h);
	     if (xob->xim)
	       {
		  xob->shm_info->shmid = shmget(IPC_PRIVATE,
						xob->xim->bytes_per_line *
						xob->xim->height,
						IPC_CREAT | 0777);
		  if (xob->shm_info->shmid >= 0)
		    {
		       xob->shm_info->readOnly = False;
		       xob->shm_info->shmaddr = xob->xim->data =
			 shmat(xob->shm_info->shmid, 0, 0);
		       if (xob->shm_info->shmaddr)
			 {
			    XErrorHandler ph;
                            
                            if (try_shm == 2) // only needed during testing
                              {
                                 XSync(d, False);
                                 _x_err = 0;
                                 ph = XSetErrorHandler((XErrorHandler)
                                                       x_output_tmp_x_err);
                              }
#if defined(EVAS_FRAME_QUEUING) && defined(LIBXEXT_VERSION_LOW)
                    /* workaround for libXext of lower then 1.1.1 */
                    if (evas_common_frameq_enabled())
                       XLockDisplay(d);
#endif
			    XShmAttach(d, xob->shm_info);
#if defined(EVAS_FRAME_QUEUING) && defined(LIBXEXT_VERSION_LOW)
                    /* workaround for libXext of lower then 1.1.1 */
                    if (evas_common_frameq_enabled())
                       XUnlockDisplay(d);
#endif

                            if (try_shm == 2) // only needed during testing
                              {
                                 XSync(d, False); 
                                 XSetErrorHandler((XErrorHandler)ph);
                              }
                            if (!_x_err)
                              {
				 xob->bpl = xob->xim->bytes_per_line;
				 xob->psize = xob->bpl * xob->h;
				 return xob;
			      }
			 }
		       shmdt(xob->shm_info->shmaddr);
		       shmctl(xob->shm_info->shmid, IPC_RMID, 0);
		    }
		  if (xob->xim) XDestroyImage(xob->xim);
		  xob->xim = NULL;
	       }
	     if (xob->shm_info) free(xob->shm_info);
	     xob->shm_info = NULL;
	  }
     }

   if (try_shm > 1) return NULL;

   xob->xim = XCreateImage(d, v, depth, ZPixmap, 0, data, w, h, 32, 0);
   if (!xob->xim)
     {
	free(xob);
	return NULL;
     }

   xob->data = data;

   if (!xob->xim->data)
     {
	xob->xim->data = malloc(xob->xim->bytes_per_line * xob->xim->height);
	if (!xob->xim->data)
	  {
	     XDestroyImage(xob->xim);
	     free(xob);
	     return NULL;
	  }
     }
   xob->bpl = xob->xim->bytes_per_line;
   xob->psize = xob->bpl * xob->h;
   return xob;
}

void
evas_software_xlib_x_output_buffer_free(X_Output_Buffer *xob, int psync)
{
   if (xob->shm_info)
     {
	if (psync) XSync(xob->display, False);
	XShmDetach(xob->display, xob->shm_info);
	XDestroyImage(xob->xim);
	shmdt(xob->shm_info->shmaddr);
	shmctl(xob->shm_info->shmid, IPC_RMID, 0);
	free(xob->shm_info);
     }
   else
     {
	if (xob->data) xob->xim->data = NULL;
	XDestroyImage(xob->xim);
     }
   free(xob);
}

void
evas_software_xlib_x_output_buffer_paste(X_Output_Buffer *xob, Drawable d, GC gc, int x, int y, int psync)
{
   if (xob->shm_info)
     {
	XShmPutImage(xob->display, d, gc, xob->xim, 0, 0, x, y,
		     xob->w, xob->h, False);
	if (psync) XSync(xob->display, False);
     }
   else
     {
	XPutImage(xob->display, d, gc, xob->xim, 0, 0, x, y,
		  xob->w, xob->h);
     }
}

DATA8 *
evas_software_xlib_x_output_buffer_data(X_Output_Buffer *xob, int *bytes_per_line_ret)
{
   if (bytes_per_line_ret) *bytes_per_line_ret = xob->xim->bytes_per_line;
   return (DATA8 *)xob->xim->data;
}

int
evas_software_xlib_x_output_buffer_depth(X_Output_Buffer *xob)
{
   return xob->xim->bits_per_pixel;
}

int
evas_software_xlib_x_output_buffer_byte_order(X_Output_Buffer *xob)
{
   return xob->xim->byte_order;
}

int
evas_software_xlib_x_output_buffer_bit_order(X_Output_Buffer *xob)
{
   return xob->xim->bitmap_bit_order;
}
