#include "evas_engine.h"


void
evas_software_ddraw_outbuf_init(void)
{
}

void
evas_software_ddraw_outbuf_free(Outbuf *buf)
{
   free(buf);
}

Outbuf *
evas_software_ddraw_outbuf_setup_dd(int                 width,
                                    int                 height,
                                    int                 rotation,
                                    Outbuf_Depth        depth,
                                    HWND                window,
                                    LPDIRECTDRAW        object,
                                    LPDIRECTDRAWSURFACE surface_primary,
                                    LPDIRECTDRAWSURFACE surface_back,
                                    int                 w_depth)
{
   Outbuf *buf;

   buf = (Outbuf *)calloc(1, sizeof(Outbuf));
   if (!buf)
      return NULL;

   buf->width = width;
   buf->height = height;
   buf->depth = depth;
   buf->rot = rotation;

   buf->priv.dd.window = window;
   buf->priv.dd.object = object;
   buf->priv.dd.surface_primary = surface_primary;
   buf->priv.dd.surface_back = surface_back;
   buf->priv.dd.depth = w_depth;

   {
      Gfx_Func_Convert     conv_func;
      DDraw_Output_Buffer *ddob;

      ddob = evas_software_ddraw_output_buffer_new(w_depth, 1, 1, NULL);

      conv_func = NULL;
      if (ddob)
        {
           if (evas_software_ddraw_masks_get(buf))
             {
                if ((rotation == 0) || (rotation == 180))
                  conv_func = evas_common_convert_func_get(0,
                                                           width,
                                                           height,
                                                           evas_software_ddraw_output_buffer_depth (ddob),
                                                           buf->priv.mask.r,
                                                           buf->priv.mask.g,
                                                           buf->priv.mask.b,
                                                           PAL_MODE_NONE,
                                                           rotation);
                else if ((rotation == 90) || (rotation == 270))
                  conv_func = evas_common_convert_func_get(0,
                                                           height,
                                                           width,
                                                           evas_software_ddraw_output_buffer_depth (ddob),
                                                           buf->priv.mask.r,
                                                           buf->priv.mask.g,
                                                           buf->priv.mask.b,
                                                           PAL_MODE_NONE,
                                                           rotation);
             }
           evas_software_ddraw_output_buffer_free(ddob);
           if (!conv_func)
             {
               printf(".[ Evas Error ].\n"
                      " {\n"
                      "  At depth         %i:\n"
                      "  RGB format mask: %08x, %08x, %08x\n"
                      "  Not supported by and compiled in converters!\n"
                      " }\n",
                      buf->priv.dd.depth,
                      buf->priv.mask.r,
                      buf->priv.mask.g,
                      buf->priv.mask.b);
             }
        }
   }

   return buf;
}

RGBA_Image *
evas_software_ddraw_outbuf_new_region_for_update(Outbuf *buf,
                                                 int x,
                                                 int y,
                                                 int width,
                                                 int height,
                                                 int *cx,
                                                 int *cy,
                                                 int *cw,
                                                 int *ch)
{
   RGBA_Image          *im;
   DDraw_Output_Buffer *ddob = NULL;
   int                  bpl = 0;

   *cx = 0;
   *cy = 0;
   *cw = width;
   *ch = height;

   if ((buf->rot == 0) &&
       (buf->priv.mask.r == 0xff0000) &&
       (buf->priv.mask.g == 0x00ff00) &&
       (buf->priv.mask.b == 0x0000ff))
     {
        im = evas_cache_image_empty(evas_common_image_cache_get());
        im->image->w = width;
        im->image->h = height;
        im->image->data = NULL;
        im->image->no_free = 1;
        ddob = evas_software_ddraw_output_buffer_new(buf->priv.dd.depth,
                                                     width,
                                                     height,
                                                     NULL);
        im->extended_info = ddob;
        im->image->data = (DATA32 *)evas_software_ddraw_output_buffer_data(ddob, &bpl);
     }
   else
     {
        im = evas_cache_image_empty(evas_common_image_cache_get());
        im->image->w = width;
        im->image->h = height;
        evas_common_image_surface_alloc(im->image);
        im->extended_info = ddob;
        if ((buf->rot == 0) || (buf->rot == 180))
          ddob = evas_software_ddraw_output_buffer_new(buf->priv.dd.depth,
                                                       width,
                                                       height,
                                                       NULL);
        else if ((buf->rot == 90) || (buf->rot == 270))
          ddob = evas_software_ddraw_output_buffer_new(buf->priv.dd.depth,
                                                       width,
                                                       height,
                                                       NULL);
        im->extended_info = ddob;
     }

   buf->priv.pending_writes = evas_list_append(buf->priv.pending_writes, im);

   return im;
}

void
evas_software_ddraw_outbuf_free_region_for_update(Outbuf     *buf,
						  RGBA_Image *update)
{
   /* no need to do anything - they are cleaned up on flush */
}

void
evas_software_ddraw_outbuf_flush(Outbuf *buf)
{
   Evas_List *l;
   void      *ddraw_data;
   int        ddraw_width;
   int        ddraw_height;
   int        ddraw_pitch;
   int        ddraw_depth;

   /* lock the back surface */
   if (!(ddraw_data = evas_software_ddraw_lock(buf,
                                               &ddraw_width,
                                               &ddraw_height,
                                               &ddraw_pitch,
                                               &ddraw_depth)))
     goto free_images;

   /* copy safely the images that need to be drawn onto the back surface */
   for (l = buf->priv.pending_writes; l; l = l->next)
     {
	RGBA_Image          *im;
	DDraw_Output_Buffer *ddob;

	im = l->data;
	ddob = im->extended_info;
	/* paste now */
	evas_software_ddraw_output_buffer_paste(ddob,
                                                ddraw_data,
                                                ddraw_width,
                                                ddraw_height,
                                                ddraw_pitch,
                                                ddraw_depth,
						ddob->x,
						ddob->y);
     }

   /* unlock the back surface and flip the surface */
   evas_software_ddraw_unlock_and_flip(buf);

 free_images:
   while (buf->priv.pending_writes)
     {
        RGBA_Image          *im;
        DDraw_Output_Buffer *ddob;

        im = buf->priv.pending_writes->data;
        buf->priv.pending_writes = evas_list_remove_list(buf->priv.pending_writes,
                                                         buf->priv.pending_writes);
        ddob = im->extended_info;
        evas_cache_image_drop(im);
        if (ddob) evas_software_ddraw_output_buffer_free(ddob);
     }
   evas_common_cpu_end_opt();
}

void
evas_software_ddraw_outbuf_push_updated_region(Outbuf     *buf,
                                               RGBA_Image *update,
                                               int         x,
                                               int         y,
                                               int         width,
                                               int         height)
{
   Gfx_Func_Convert     conv_func;
   DDraw_Output_Buffer *ddob;
   DATA32              *src_data;
   void                *data;
   int                  bpl = 0;

   conv_func = NULL;
   ddob = update->extended_info;

   if ((buf->rot == 0) || (buf->rot == 180))
     conv_func = evas_common_convert_func_get(NULL,
                                              width,
                                              height,
                                              evas_software_ddraw_output_buffer_depth(ddob),
                                              buf->priv.mask.r,
                                              buf->priv.mask.g,
                                              buf->priv.mask.b,
                                              PAL_MODE_NONE,
                                              buf->rot);
   else if ((buf->rot == 90) || (buf->rot == 270))
     conv_func = evas_common_convert_func_get(NULL,
                                              height,
                                              width,
                                              evas_software_ddraw_output_buffer_depth(ddob),
                                              buf->priv.mask.r,
                                              buf->priv.mask.g,
                                              buf->priv.mask.b,
                                              PAL_MODE_NONE,
                                              buf->rot);

   if (!conv_func) return;

   data = evas_software_ddraw_output_buffer_data(ddob, &bpl);
   src_data = update->image->data;
   if (buf->rot == 0)
     {
        ddob->x = x;
        ddob->y = y;
     }
   else if (buf->rot == 90)
     {
        ddob->x = y;
        ddob->y = buf->width - x - width;
     }
   else if (buf->rot == 180)
     {
        ddob->x = buf->width - x - width;
        ddob->y = buf->height - y - height;
     }
   else if (buf->rot == 270)
     {
        ddob->x = buf->height - y - height;
        ddob->y = x;
     }
   if ((buf->rot == 0) || (buf->rot == 180))
     {
        ddob->width = width;
        ddob->height = height;
     }
   else if ((buf->rot == 90) || (buf->rot == 270))
     {
        ddob->width = height;
        ddob->height = width;
     }

   if (data != src_data)
     conv_func(src_data, data,
               0,
               bpl /
               ((evas_software_ddraw_output_buffer_depth(ddob))) - ddob->width,
               ddob->width, ddob->height, x, y, NULL);
}

void
evas_software_ddraw_outbuf_reconfigure(Outbuf      *buf,
                                       int          width,
                                       int          height,
                                       int          rotation,
                                       Outbuf_Depth depth)
{
   if ((width == buf->width) && (height == buf->height) &&
       (rotation == buf->rot) && (depth == buf->depth))
     return;
   buf->width = width;
   buf->height = height;
   buf->rot = rotation;
   evas_software_ddraw_surface_resize(buf);
}

int
evas_software_ddraw_outbuf_width_get(Outbuf *buf)
{
   return buf->width;
}

int
evas_software_ddraw_outbuf_height_get(Outbuf *buf)
{
   return buf->height;
}

Outbuf_Depth
evas_software_ddraw_outbuf_depth_get(Outbuf *buf)
{
   return buf->depth;
}

int
evas_software_ddraw_outbuf_rot_get(Outbuf *buf)
{
   return buf->rot;
}
