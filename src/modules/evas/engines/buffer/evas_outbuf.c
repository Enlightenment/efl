#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef EVAS_CSERVE2
#include "evas_cs2_private.h"
#endif

#include "evas_common_private.h"
#include "evas_engine.h"

void
evas_buffer_outbuf_buf_init(void)
{
}

void
evas_buffer_outbuf_buf_free(Outbuf *buf)
{
   if (buf->priv.back_buf)
     {
#ifdef EVAS_CSERVE2
        if (evas_cserve2_use_get())
          evas_cache2_image_close(&buf->priv.back_buf->cache_entry);
        else
#endif
        evas_cache_image_drop(&buf->priv.back_buf->cache_entry);
     }
   free(buf);
}

void
evas_buffer_outbuf_buf_update_fb(Outbuf *buf, int w, int h, Outbuf_Depth depth, void *dest, int dest_row_bytes, int use_color_key, DATA32 color_key, int alpha_level,
                                void * (*new_update_region) (int x, int y, int w, int h, int *row_bytes),
                                void   (*free_update_region) (int x, int y, int w, int h, void *data),
                                void * (*switch_buffer) (void *data, void *dest_buffer),
                                void *switch_data)
{
   buf->w = w;
   buf->h = h;
   buf->depth = depth;

   buf->dest = dest;
   buf->dest_row_bytes = dest_row_bytes;

   buf->alpha_level = alpha_level;
   buf->color_key = color_key;
   buf->use_color_key = use_color_key;
   buf->first_frame = 1;

   buf->func.new_update_region = new_update_region;
   buf->func.free_update_region = free_update_region;
   buf->func.switch_buffer = switch_buffer;
   buf->switch_data = switch_data;

   if ((buf->depth == OUTBUF_DEPTH_ARGB_32BPP_8888_8888) &&
       (buf->dest) && (buf->dest_row_bytes == (buf->w * sizeof(DATA32))))
     {
	memset(buf->dest, 0, h * buf->dest_row_bytes);
#ifdef EVAS_CSERVE2
        if (evas_cserve2_use_get())
          buf->priv.back_buf = (RGBA_Image *) evas_cache2_image_data(evas_common_image_cache2_get(),
                                                                     w, h,
                                                                     buf->dest,
                                                                     1, EVAS_COLORSPACE_ARGB8888);
        else
#endif
	buf->priv.back_buf = (RGBA_Image *) evas_cache_image_data(evas_common_image_cache_get(),
                                                                  w, h,
                                                                  buf->dest,
                                                                  1, EVAS_COLORSPACE_ARGB8888);
     }
   else if ((buf->depth == OUTBUF_DEPTH_RGB_32BPP_888_8888) &&
       (buf->dest) && (buf->dest_row_bytes == (buf->w * sizeof(DATA32))))
     {
#ifdef EVAS_CSERVE2
        if (evas_cserve2_use_get())
          buf->priv.back_buf = (RGBA_Image *) evas_cache2_image_data(evas_common_image_cache2_get(),
                                                                     w, h,
                                                                     buf->dest,
                                                                     0, EVAS_COLORSPACE_ARGB8888);
        else
#endif
        buf->priv.back_buf = (RGBA_Image *) evas_cache_image_data(evas_common_image_cache_get(),
                                                                  w, h,
                                                                  buf->dest,
                                                                  0, EVAS_COLORSPACE_ARGB8888);
     }
}

Outbuf *
evas_buffer_outbuf_buf_setup_fb(int w, int h, Outbuf_Depth depth, void *dest, int dest_row_bytes, int use_color_key, DATA32 color_key, int alpha_level,
                                void * (*new_update_region) (int x, int y, int w, int h, int *row_bytes),
                                void   (*free_update_region) (int x, int y, int w, int h, void *data),
                                void * (*switch_buffer) (void *data, void *dest_buffer),
                                void *switch_data)
{
   Outbuf *buf;

   buf = calloc(1, sizeof(Outbuf));
   if (!buf) return NULL;

   evas_buffer_outbuf_buf_update_fb(buf,
                                    w,
                                    h,
                                    depth,
                                    dest,
                                    dest_row_bytes,
                                    use_color_key,
                                    color_key,
                                    alpha_level,
                                    new_update_region,
                                    free_update_region,
                                    switch_buffer,
                                    switch_data);

   return buf;
}

void *
evas_buffer_outbuf_buf_new_region_for_update(Outbuf *buf, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch)
{
   RGBA_Image *im;

   if (buf->priv.back_buf)
     {
	*cx = x; *cy = y; *cw = w; *ch = h;
	return buf->priv.back_buf;
     }
   else
     {
	*cx = 0; *cy = 0; *cw = w; *ch = h;
#ifdef EVAS_CSERVE2
        if (evas_cserve2_use_get())
          im = (RGBA_Image *)evas_cache2_image_empty(evas_common_image_cache2_get());
        else
#endif
	im = (RGBA_Image *) evas_cache_image_empty(evas_common_image_cache_get());
        if (im)
          {
	     if (((buf->depth == OUTBUF_DEPTH_ARGB_32BPP_8888_8888)) ||
		 ((buf->depth == OUTBUF_DEPTH_BGRA_32BPP_8888_8888)))
	       {
		  im->cache_entry.flags.alpha = 1;
               }

#ifdef EVAS_CSERVE2
	     if (evas_cserve2_use_get())
	       evas_cache2_image_size_set(&im->cache_entry, w, h);
	     else
#endif
	       im = (RGBA_Image *) evas_cache_image_size_set(&im->cache_entry, w, h);
          }
     }
   return im;
}

void
evas_buffer_outbuf_buf_free_region_for_update(Outbuf *buf, RGBA_Image *update)
{
   if (update != buf->priv.back_buf)
     {
#ifdef EVAS_CSERVE2
        if (evas_cserve2_use_get())
          evas_cache2_image_close(&update->cache_entry);
        else
#endif
          evas_cache_image_drop(&update->cache_entry);
     }
}

void
evas_buffer_outbuf_buf_switch_buffer(Outbuf *buf, Tilebuf_Rect *surface_damage EINA_UNUSED, Tilebuf_Rect *buffer_damage EINA_UNUSED, Evas_Render_Mode render_mode EINA_UNUSED)
{
   if (buf->func.switch_buffer)
     {
        buf->dest = buf->func.switch_buffer(buf->switch_data, buf->dest);
        if (buf->priv.back_buf)
          {
#ifdef EVAS_CSERVE2
             if (evas_cserve2_use_get())
               {
                  evas_cache2_image_close(&buf->priv.back_buf->cache_entry);
                  buf->priv.back_buf = (RGBA_Image *) evas_cache2_image_data(evas_common_image_cache2_get(),
                                                                            buf->w, buf->h,
                                                                            buf->dest,
                                                                            buf->depth == OUTBUF_DEPTH_ARGB_32BPP_8888_8888 ? 1 : 0,
                                                                            EVAS_COLORSPACE_ARGB8888);
               }
             else
#endif
               {
                  evas_cache_image_drop(&buf->priv.back_buf->cache_entry);
                  buf->priv.back_buf = (RGBA_Image *) evas_cache_image_data(evas_common_image_cache_get(),
                                                                            buf->w, buf->h,
                                                                            buf->dest,
                                                                            buf->depth == OUTBUF_DEPTH_ARGB_32BPP_8888_8888 ? 1 : 0,
                                                                            EVAS_COLORSPACE_ARGB8888);
               }
          }
     }
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
		       src = update->image.data + (yy * update->cache_entry.w);
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
		       src = update->image.data + (yy * update->cache_entry.w);
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
		       src = update->image.data + (yy * update->cache_entry.w);
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
		       src = update->image.data + (yy * update->cache_entry.w);
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
      case OUTBUF_DEPTH_ARGB_32BPP_8888_8888:
	  {
	     DATA32 *dest, *src, *dst;
	     int yy, row_bytes;

	     row_bytes = buf->dest_row_bytes;
	     dest = (DATA32 *)((DATA8 *)(buf->dest) + (y * row_bytes) + (x * 4));
	     if (buf->func.new_update_region)
	       {
		  dest = buf->func.new_update_region(x, y, w, h, &row_bytes);
	       }
	     /* no need src == dest */
	     if (!buf->priv.back_buf)
	       {
		  Gfx_Func_Copy func;
		  
		  func = evas_common_draw_func_copy_get(w, 0);
		  if (func)
		    {
		       for (yy = 0; yy < h; yy++)
			 {
			    src = update->image.data + (yy * update->cache_entry.w);
			    dst = (DATA32 *)((DATA8 *)(buf->dest) + ((y + yy) * row_bytes));
			    func(src, dst, w);
			 }
		       
		    }
	       }
	     if (buf->func.free_update_region)
	       {
		  buf->func.free_update_region(x, y, w, h, dest);
	       }
	  }
	break;
      case OUTBUF_DEPTH_BGR_32BPP_888_8888:
	  {
	     DATA32 *src, *dst;
	     DATA8 *dest;
	     int xx, yy, row_bytes;
	     
	     row_bytes = buf->dest_row_bytes;
	     dest = (DATA8 *)(buf->dest) + (y * row_bytes) + (x * 4);
	     if (buf->func.new_update_region)
	       {
		  dest = buf->func.new_update_region(x, y, w, h, &row_bytes);
	       }
	     for (yy = 0; yy < h; yy++)
	       {
		  dst = (DATA32 *)(dest + (yy * row_bytes));
		  src = update->image.data + (yy * update->cache_entry.w);
		  for (xx = 0; xx < w; xx++)
		    {
		       A_VAL(dst) = B_VAL(src);
		       R_VAL(dst) = G_VAL(src);
		       G_VAL(dst) = R_VAL(src);
		       dst++;
		       src++;
		    }
	       }
	     if (buf->func.free_update_region)
	       {
		  buf->func.free_update_region(x, y, w, h, dest);
	       }
	 }
	break;
      case OUTBUF_DEPTH_BGRA_32BPP_8888_8888:
	  {
	     DATA32 *src, *dst;
	     DATA8 *dest;
	     int xx, yy, row_bytes;
	     
	     row_bytes = buf->dest_row_bytes;
	     dest = (DATA8 *)(buf->dest) + (y * row_bytes) + (x * 4);
	     if (buf->func.new_update_region)
	       {
		  dest = buf->func.new_update_region(x, y, w, h, &row_bytes);
	       }
	     for (yy = 0; yy < h; yy++)
	       {
		  dst = (DATA32 *)(dest + (yy * row_bytes));
		  src = update->image.data + (yy * update->cache_entry.w);
		  for (xx = 0; xx < w; xx++)
		    {
		       A_VAL(dst) = B_VAL(src);
		       R_VAL(dst) = G_VAL(src);
		       G_VAL(dst) = R_VAL(src);
		       dst++;
		       src++;
		    }
	       }
	     if (buf->func.free_update_region)
	       {
		  buf->func.free_update_region(x, y, w, h, dest);
	       }
	 }
	break;
      default:
	break;
     }
}

void
evas_buffer_outbuf_reconfigure(Outbuf *ob, int w, int h, int rot EINA_UNUSED, Outbuf_Depth depth)
{
   void    *dest;
   int      dest_row_bytes;
   int      alpha_level;
   DATA32   color_key;
   char     use_color_key;
   void * (*new_update_region) (int x, int y, int w, int h, int *row_bytes);
   void   (*free_update_region) (int x, int y, int w, int h, void *data);
   void * (*switch_buffer) (void *switch_data, void *dest);
   void    *switch_data;

   if (depth == OUTBUF_DEPTH_INHERIT) depth = ob->depth;
   dest = ob->dest;
   dest_row_bytes = ob->dest_row_bytes;
   alpha_level = ob->alpha_level;
   color_key = ob->color_key;
   use_color_key = ob->use_color_key;
   new_update_region = ob->func.new_update_region;
   free_update_region = ob->func.free_update_region;
   switch_buffer = ob->func.switch_buffer;
   switch_data = ob->switch_data;

   evas_buffer_outbuf_buf_update_fb(ob,
                                    w,
                                    h,
                                    depth,
                                    dest,
                                    dest_row_bytes,
                                    use_color_key,
                                    color_key,
                                    alpha_level,
                                    new_update_region,
                                    free_update_region,
                                    switch_buffer,
                                    switch_data);
}

Render_Output_Swap_Mode
evas_buffer_outbuf_buf_swap_mode_get(Outbuf *ob)
{
   if (ob->func.switch_buffer) return MODE_DOUBLE;
   return MODE_FULL;
}

int
evas_buffer_outbuf_buf_rot_get(Outbuf *buf EINA_UNUSED)
{
   return 0;
}
