#include "evas_common.h"

void
blit_init(void)
{
}

void
blit_rectangle(RGBA_Image *src, RGBA_Image *dst, int src_x, int src_y, int w, int h, int dst_x, int dst_y)
{
   int y;
   Gfx_Func_Blend_Src_Dst func;
   DATA32 *src_ptr, *dst_ptr;
   
   /* clip clip clip */
   if (w <= 0) return;
   if (src_x + w > src->image->w) w = src->image->w - src_x;
   if (w <= 0) return;
   if (src_x < 0)
     {
	dst_x -= src_x;
	w += src_x;
	src_x = 0;
     }
   if (w <= 0) return;
   
   if (h <= 0) return;
   if (src_y + h > src->image->h) h = src->image->h - src_y;
   if (h <= 0) return;
   if (src_y < 0)
     {
	dst_y -= src_y;
	h += src_y;
	src_y = 0;
     }
   if (h <= 0) return;
   
   if (w <= 0) return;
   if (dst_x + w > dst->image->w) w = dst->image->w - dst_x;
   if (w <= 0) return;
   if (dst_x < 0)
     {
	src_x -= dst_x;
	w += dst_x;
	dst_x = 0;
     }
   if (w <= 0) return;
   
   if (h <= 0) return;
   if (dst_y + h > dst->image->h) h = dst->image->h - dst_y;
   if (h <= 0) return;
   if (dst_y < 0)
     {
	src_y -= dst_y;
	h += dst_y;
	dst_y = 0;
     }
   if (h <= 0) return;
   
   if (dst == src)
     {
	/* src after dst - go forward */
	if (((src_y * src->image->w) + src_x) > ((dst_y * dst->image->w) + dst_x))
	  {
	     func = draw_func_copy_get(w, 0);
	     for (y = 0; y < h; y++)
	       {
		  src_ptr = src->image->data + ((y + src_y) * src->image->w) + src_x;
		  dst_ptr = dst->image->data + ((y + dst_y) * dst->image->w) + dst_x;
		  func(src_ptr, dst_ptr, w);
	       }
	  }
	/* reverse */
	else
	  {
	     func = draw_func_copy_get(w, 1);
	     for (y = h - 1; y >= 0; y--)
	       {
		  src_ptr = src->image->data + ((y + src_y) * src->image->w) + src_x;
		  dst_ptr = dst->image->data + ((y + dst_y) * dst->image->w) + dst_x;
		  func(src_ptr, dst_ptr, w);
	       }
	  }
     }
   else
     {
	func = draw_func_copy_get(w, 0);   
	for (y = 0; y < h; y++)
	  {
	     src_ptr = src->image->data + ((y + src_y) * src->image->w) + src_x;
	     dst_ptr = dst->image->data + ((y + dst_y) * dst->image->w) + dst_x;
	     func(src_ptr, dst_ptr, w);
	  }
     }
}
