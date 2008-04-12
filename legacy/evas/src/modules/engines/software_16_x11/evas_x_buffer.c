#include "evas_common.h"
#include "evas_engine.h"

static int _x_err = 0;

int
evas_software_x11_x_can_do_shm(Display *d)
{
   static Display *cached_d = NULL;
   static int cached_result = 0;
   
   if (d == cached_d) return cached_result;
   cached_d = d;
   if (XShmQueryExtension(d))
     {
	X_Output_Buffer *xob;

	xob = evas_software_x11_x_output_buffer_new
	  (d, DefaultVisual(d, DefaultScreen(d)),
	   DefaultDepth(d, DefaultScreen(d)), 16, 16, 2, NULL);
	if (!xob)
	  {
	     cached_result = 0;
	     return 0;
	  }
	evas_software_x11_x_output_buffer_free(xob, 1);
	cached_result = 1;
	return 1;
     }
   cached_result = 0;
   return 0;
}

static void
x_output_tmp_x_err(Display * d, XErrorEvent * ev)
{
   _x_err = 1;
   return;
}

X_Output_Buffer *
evas_software_x11_x_output_buffer_new(Display *d, Visual *v, int depth, int w, int h, int try_shm, void *data)
{
   X_Output_Buffer *xob;

   xob = calloc(1, sizeof(X_Output_Buffer));
   if (!xob) return NULL;

   xob->display = d;
   xob->xim = NULL;
   xob->shm_info = NULL;

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
		       if (xob->shm_info->shmaddr != NULL)
			 {
			    XErrorHandler ph;

			    XSync(d, False);
			    _x_err = 0;
			    ph = XSetErrorHandler((XErrorHandler)
						  x_output_tmp_x_err);
			    XShmAttach(d, xob->shm_info);
			    XSync(d, False);
			    XSetErrorHandler((XErrorHandler)ph);
			    if (!_x_err)
			      {
                                 xob->im = (Soft16_Image *) evas_cache_image_data(evas_common_soft16_image_cache_get(), w, h, (DATA32 *) xob->xim->data, 0, EVAS_COLORSPACE_RGB565_A5P);
                                 if (xob->im)
                                   xob->im->stride = xob->xim->bytes_per_line / sizeof(DATA16);
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
   if (xob->im)
     evas_cache_image_drop(&xob->im->cache_entry);

   xob->im =  (Soft16_Image *) evas_cache_image_data(evas_common_soft16_image_cache_get(), w, h, (DATA32 *) xob->xim->data, 0, EVAS_COLORSPACE_RGB565_A5P);
   if (xob->im)
     xob->im->stride = xob->xim->bytes_per_line / sizeof(DATA16);
   return xob;
}

void
evas_software_x11_x_output_buffer_free(X_Output_Buffer *xob, int sync)
{
   if (xob->shm_info)
     {
	if (sync) XSync(xob->display, False);
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
evas_software_x11_x_output_buffer_paste(X_Output_Buffer *xob, Drawable d, GC gc, int x, int y, int w, int h, int sync)
{
   if (xob->shm_info)
     {
	XShmPutImage(xob->display, d, gc, xob->xim, 0, 0, x, y, w, h, False);
	if (sync) XSync(xob->display, False);
     }
   else
     XPutImage(xob->display, d, gc, xob->xim, 0, 0, x, y, w, h);
}

DATA8 *
evas_software_x11_x_output_buffer_data(X_Output_Buffer *xob, int *bytes_per_line_ret)
{
   if (bytes_per_line_ret) *bytes_per_line_ret = xob->xim->bytes_per_line;
   return xob->xim->data;
}

int
evas_software_x11_x_output_buffer_depth(X_Output_Buffer *xob)
{
   return xob->xim->bits_per_pixel;
}

int
evas_software_x11_x_output_buffer_byte_order(X_Output_Buffer *xob)
{
   return xob->xim->byte_order;
}

int
evas_software_x11_x_output_buffer_bit_order(X_Output_Buffer *xob)
{
   return xob->xim->bitmap_bit_order;
}
