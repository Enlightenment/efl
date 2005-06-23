#include "evas_common.h"
#include "evas_engine.h"
#include <sys/time.h>

void
evas_buffer_outbuf_buf_init(void)
{
}

void
evas_buffer_outbuf_buf_free(Outbuf *buf)
{
   if (buf->priv.back_buf)
     {
	evas_common_image_free(buf->priv.back_buf);
     }
   free(buf);
}

Outbuf *
evas_buffer_outbuf_buf_setup_fb(int w, int h, Outbuf_Depth depth, void *dest, int dest_row_bytes, int use_color_key, DATA32 color_key, int alpha_level,
				void * (*new_update_region) (int x, int y, int w, int h, int *row_bytes),
				void   (*free_update_region) (int x, int y, int w, int h, void *data)
				)
{
   Outbuf *buf;

   buf = calloc(1, sizeof(Outbuf));
   if (!buf) return NULL;

   buf->w = w;
   buf->h = h;
   buf->depth = depth;

   buf->dest = dest;
   buf->dest_row_bytes = dest_row_bytes;

   buf->alpha_level = alpha_level;
   buf->color_key = color_key;
   buf->use_color_key = use_color_key;

   buf->func.new_update_region = new_update_region;
   buf->func.free_update_region = free_update_region;

   if ((buf->depth == OUTBUF_DEPTH_RGB_32BPP_888_8888) &&
       (buf->dest) && (buf->dest_row_bytes == (buf->w * sizeof(DATA32))))
     {
	buf->priv.back_buf = evas_common_image_new();
	buf->priv.back_buf->image = evas_common_image_surface_new( buf->priv.back_buf);
	buf->priv.back_buf->image->w = w;
	buf->priv.back_buf->image->h = h;
	buf->priv.back_buf->image->data = buf->dest;
	buf->priv.back_buf->image->no_free = 1;
	buf->priv.back_buf->flags |= RGBA_IMAGE_HAS_ALPHA;
     }

   return buf;
}

RGBA_Image *
evas_buffer_outbuf_buf_new_region_for_update(Outbuf *buf, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch)
{
   RGBA_Image *im;

   if (buf->priv.back_buf)
     {
	int xx, yy;

	*cx = x; *cy = y; *cw = w; *ch = h;
	for (yy = 0; yy < h; yy++)
	  {
	     DATA32 *ptr;

	     ptr = buf->priv.back_buf->image->data +
	       (y * buf->priv.back_buf->image->w) + x;
	     for (xx = 0; xx < w; xx++)
	       {
		  A_VAL(ptr) = 0;
		  ptr++;
	       }
	  }
	return buf->priv.back_buf;
     }
   else
     {
	*cx = 0; *cy = 0; *cw = w; *ch = h;
	im = evas_common_image_create(w, h);
	if (im)
	  {
	     im->flags |= RGBA_IMAGE_HAS_ALPHA;
	     memset(im->image->data, 0, w * h * sizeof(DATA32));
	  }
     }
   return im;
}

void
evas_buffer_outbuf_buf_free_region_for_update(Outbuf *buf, RGBA_Image *update)
{
   if (update != buf->priv.back_buf) evas_common_image_free(update);
}

void
evas_buffer_outbuf_buf_push_updated_region(Outbuf *buf, RGBA_Image *update, int x, int y, int w, int h)
{
   /* copy update image to out buf & convert */
   switch (buf->depth)
     {
      case OUTBUF_DEPTH_RGB_24BPP_888_888:
	/* copy & pack into 24bpp - if colorkey is enabled... etc. */
	  {
	     DATA8 thresh;
	     int xx, yy;
	     int row_bytes;
	     DATA8 *dest;
	     DATA32 colorkey;
	     DATA32 *src;
	     DATA8 *dst;

	     colorkey = buf->color_key;
	     thresh = buf->alpha_level;
	     row_bytes = buf->dest_row_bytes;
	     dest = (DATA8 *)(buf->dest) + (y * row_bytes) + (x * 3);
	     if (buf->func.new_update_region)
	       {
		  dest = buf->func.new_update_region(x, y, w, h, &row_bytes);
	       }
	     if (!dest) break;
	     if (buf->use_color_key)
	       {
		  for (yy = 0; yy < h; yy++)
		    {
		       dst = dest + (yy * row_bytes);
		       src = update->image->data + (yy * update->image->w);
		       for (xx = 0; xx < w; xx++)
			 {
			    if (A_VAL(src) > thresh)
			      {
				 *dst++ = R_VAL(src);
				 *dst++ = G_VAL(src);
				 *dst++ = B_VAL(src);
			      }
			    else
			      {
				 *dst++ = R_VAL(&colorkey);
				 *dst++ = G_VAL(&colorkey);
				 *dst++ = B_VAL(&colorkey);
			      }
			    src++;
			 }
		    }
	       }
	     else
	       {
		  for (yy = 0; yy < h; yy++)
		    {
		       dst = dest + (yy * row_bytes);
		       src = update->image->data + (yy * update->image->w);
		       for (xx = 0; xx < w; xx++)
			 {
			    *dst++ = R_VAL(src);
			    *dst++ = G_VAL(src);
			    *dst++ = B_VAL(src);
			    src++;
			 }
		    }
	       }
	     if (buf->func.free_update_region)
	       {
		  buf->func.free_update_region(x, y, w, h, dest);
	       }
	  }
	break;
      case OUTBUF_DEPTH_BGR_24BPP_888_888:
	/* copy & pack into 24bpp - if colorkey is enabled... etc. */
	  {
	     DATA8 thresh;
	     int xx, yy;
	     int row_bytes;
	     DATA8 *dest;
	     DATA32 colorkey;
	     DATA32 *src;
	     DATA8 *dst;

	     colorkey = buf->color_key;
	     thresh = buf->alpha_level;
	     row_bytes = buf->dest_row_bytes;
	     dest = (DATA8 *)(buf->dest) + (y * row_bytes) + (x * 3);
	     if (buf->func.new_update_region)
	       {
		  dest = buf->func.new_update_region(x, y, w, h, &row_bytes);
	       }
	     if (!dest) break;
	     if (buf->use_color_key)
	       {
		  for (yy = 0; yy < h; yy++)
		    {
		       dst = dest + (yy * row_bytes);
		       src = update->image->data + (yy * update->image->w);
		       for (xx = 0; xx < w; xx++)
			 {
			    if (A_VAL(src) > thresh)
			      {
				 *dst++ = B_VAL(src);
				 *dst++ = G_VAL(src);
				 *dst++ = R_VAL(src);
			      }
			    else
			      {
				 *dst++ = B_VAL(&colorkey);
				 *dst++ = G_VAL(&colorkey);
				 *dst++ = R_VAL(&colorkey);
			      }
			    src++;
			 }
		    }
	       }
	     else
	       {
		  for (yy = 0; yy < h; yy++)
		    {
		       dst = dest + (yy * row_bytes);
		       src = update->image->data + (yy * update->image->w);
		       for (xx = 0; xx < w; xx++)
			 {
			    *dst++ = B_VAL(src);
			    *dst++ = G_VAL(src);
			    *dst++ = R_VAL(src);
			    src++;
			 }
		    }
	       }
	     if (buf->func.free_update_region)
	       {
		  buf->func.free_update_region(x, y, w, h, dest);
	       }
	  }
	break;
      case OUTBUF_DEPTH_RGB_32BPP_888_8888:
	printf("NOT IMPLEMENTED\n");
	break;
      case OUTBUF_DEPTH_BGR_32BPP_888_8888:
	  {
	     DATA32 *src;
	     DATA8 *dest;
	     int yy, row_bytes;
	     Gfx_Func_Blend_Src_Dst func;
	     
	     row_bytes = buf->dest_row_bytes;
	     dest = (DATA8 *)(buf->dest) + (y * row_bytes) + (x * 4);
	     func = evas_common_draw_func_copy_get(w, 0);
	     if (func)
	       {
		  for (yy = 0; yy < h; yy++) 
		    {
		       src = update->image->data + (yy * update->image->w);
		       func(src, dest, w);
		       dest += row_bytes;
		    }
	       }
	 }
	break;
      default:
	break;
     }
}
