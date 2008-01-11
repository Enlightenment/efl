#include "evas_common.h"
#include "evas_engine.h"

static int _x_err = 0;

void
evas_software_x11_x_write_mask_line(Outbuf *buf, X_Output_Buffer *xob, DATA32 *src, int w, int y)
{
   int x;
   DATA32 *src_ptr;
   DATA8 *dst_ptr;
   int bpl = 0;

   src_ptr = src;
   dst_ptr = evas_software_x11_x_output_buffer_data(xob, &bpl);
   dst_ptr = dst_ptr + (bpl * y);
   w -= 7;
   if (buf->priv.x.bit_swap)
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

//static int creates = 0;

X_Output_Buffer *
evas_software_x11_x_output_buffer_new(Display *d, Visual *v, int depth, int w, int h, int try_shm, void *data)
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
//				 printf("SHM++ ID=%i -> %i bytes [%i creates]\n",
//					xob->shm_info->shmid,
//					xob->xim->bytes_per_line * xob->xim->height,
//					creates);
//				 creates++;
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
evas_software_x11_x_output_buffer_free(X_Output_Buffer *xob, int sync)
{
   if (xob->shm_info)
     {
//	printf("SHM-- ID=%i -> %i bytes, [sync=%i]\n",
//	       xob->shm_info->shmid,
//	       xob->xim->bytes_per_line * xob->xim->height,
//	       sync);
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
evas_software_x11_x_output_buffer_paste(X_Output_Buffer *xob, Drawable d, GC gc, int x, int y, int sync)
{
   if (xob->shm_info)
     {
//	printf("shm\n");
	XShmPutImage(xob->display, d, gc, xob->xim, 0, 0, x, y,
		     xob->w, xob->h, False);
	if (sync) XSync(xob->display, False);
     }
   else
     {
//	printf("NO! shm\n");
	XPutImage(xob->display, d, gc, xob->xim, 0, 0, x, y,
		  xob->w, xob->h);
     }
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
