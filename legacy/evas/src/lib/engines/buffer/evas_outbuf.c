#include "evas_common.h"
#include "evas_engine.h"
#include <sys/time.h>
#include <sys/utsname.h>

void
evas_buffer_outbuf_buf_init(void)
{
}

void
evas_buffer_outbuf_buf_free(Outbuf *buf)
{
   free(buf);
}

Outbuf *
evas_buffer_outbuf_buf_setup_fb(int w, int h, Outbuf_Depth depth, void *dest, int dest_row_bytes, int use_color_key, DATA32 color_key, int alpha_level)
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
   
   return buf;
}

RGBA_Image *
evas_buffer_outbuf_buf_new_region_for_update(Outbuf *buf, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch)
{
   RGBA_Image *im;
   
   *cx = 0; *cy = 0; *cw = w; *ch = h;
   im = evas_common_image_create(w, h);
   if (im)
     {
	im->flags |= RGBA_IMAGE_HAS_ALPHA;
	memset(im->image->data, 0, w * h * sizeof(DATA32));
     }
   return im;
}

void
evas_buffer_outbuf_buf_free_region_for_update(Outbuf *buf, RGBA_Image *update)
{
   evas_common_image_free(update);
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
	     DATA32 colorkey;
	     DATA32 *src;
	     DATA8 *dst;
	     
	     colorkey = buf->color_key;
	     thresh = buf->alpha_level;
	     if (buf->use_color_key)
	       {
		  for (yy = 0; yy < h; yy++)
		    {
		       dst = buf->dest + ((y + yy) * buf->dest_row_bytes);
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
		       dst = buf->dest + ((y + yy) * buf->dest_row_bytes);
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
	  }
	break;
      case OUTBUF_DEPTH_RGB_32BPP_888_8888:
	/* simple memcpy */
	/* FIXME: write this */
	break;
      default:
	break;
     }
}
