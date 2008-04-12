#include "evas_engine.h"


void
evas_direct3d_outbuf_init(void)
{
}

void
evas_direct3d_outbuf_free(Outbuf *buf)
{
   free(buf);
}

Outbuf *
evas_direct3d_outbuf_setup_d3d(int                width,
                               int                height,
                               int                rotation,
                               Outbuf_Depth       depth,
                               HWND               window,
                               LPDIRECT3D9        object,
                               LPDIRECT3DDEVICE9  device,
                               LPD3DXSPRITE       sprite,
                               LPDIRECT3DTEXTURE9 texture,
                               int                w_depth)
{
   Outbuf *buf;

   buf = (Outbuf *)calloc(1, sizeof(Outbuf));
   if (!buf)
      return NULL;

   buf->width = width;
   buf->height = height;
   buf->depth = depth;
   buf->rot = rotation;

   buf->priv.d3d.window = window;
   buf->priv.d3d.object = object;
   buf->priv.d3d.device = device;
   buf->priv.d3d.sprite = sprite;
   buf->priv.d3d.texture = texture;
   buf->priv.d3d.depth = w_depth;

   {
      Gfx_Func_Convert        conv_func;
      Direct3D_Output_Buffer *d3dob;

      d3dob = evas_direct3d_output_buffer_new(buf->priv.d3d.depth, 1, 1, NULL);

      conv_func = NULL;
      if (d3dob)
        {
           if (evas_direct3d_masks_get(buf))
             {
                if ((rotation == 0) || (rotation == 180))
                  conv_func = evas_common_convert_func_get(0,
                                                           width,
                                                           height,
                                                           evas_direct3d_output_buffer_depth (d3dob),
                                                           buf->priv.mask.r,
                                                           buf->priv.mask.g,
                                                           buf->priv.mask.b,
                                                           PAL_MODE_NONE,
                                                           rotation);
                else if ((rotation == 90) || (rotation == 270))
                  conv_func = evas_common_convert_func_get(0,
                                                           height,
                                                           width,
                                                           evas_direct3d_output_buffer_depth (d3dob),
                                                           buf->priv.mask.r,
                                                           buf->priv.mask.g,
                                                           buf->priv.mask.b,
                                                           PAL_MODE_NONE,
                                                           rotation);
             }
           evas_direct3d_output_buffer_free(d3dob);
           if (!conv_func)
             {
               printf(".[ Evas Error ].\n"
                      " {\n"
                      "  At depth         %i:\n"
                      "  RGB format mask: %08x, %08x, %08x\n"
                      "  Not supported by and compiled in converters!\n"
                      " }\n",
                      buf->priv.d3d.depth,
                      buf->priv.mask.r,
                      buf->priv.mask.g,
                      buf->priv.mask.b);
             }
        }
   }

   return buf;
}

RGBA_Image *
evas_direct3d_outbuf_new_region_for_update(Outbuf *buf,
                                           int x,
                                           int y,
                                           int width,
                                           int height,
                                           int *cx,
                                           int *cy,
                                           int *cw,
                                           int *ch)
{
   RGBA_Image             *im;
   Direct3D_Output_Buffer *d3dob = NULL;
   int                     bpl = 0;

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
        d3dob = evas_direct3d_output_buffer_new(buf->priv.d3d.depth,
                                                width,
                                                height,
                                                NULL);
        im->extended_info = d3dob;
        im->image->data = (DATA32 *)evas_direct3d_output_buffer_data(d3dob, &bpl);
     }
   else
     {
        im = (RGBA_Image*) evas_cache_image_empty(evas_common_image_cache_get());
        evas_cache_image_surface_alloc(&im->cache_entry, width, height);
        im->extended_info = d3dob;
        if ((buf->rot == 0) || (buf->rot == 180))
          d3dob = evas_direct3d_output_buffer_new(buf->priv.d3d.depth,
                                                  width,
                                                  height,
                                                  NULL);
        else if ((buf->rot == 90) || (buf->rot == 270))
          d3dob = evas_direct3d_output_buffer_new(buf->priv.d3d.depth,
                                                  width,
                                                  height,
                                                  NULL);
        im->extended_info = d3dob;
     }

   buf->priv.pending_writes = evas_list_append(buf->priv.pending_writes, im);

   return im;
}

void
evas_direct3d_outbuf_free_region_for_update(Outbuf     *buf,
                                            RGBA_Image *update)
{
   /* no need to do anything - they are cleaned up on flush */
}

void
evas_direct3d_outbuf_flush(Outbuf *buf)
{
   Evas_List *l;
   void      *d3d_data;
   int        d3d_width;
   int        d3d_height;
   int        d3d_pitch;

   /* lock the texture */
   if (!(d3d_data = evas_direct3d_lock(buf,
                                       &d3d_width, &d3d_height, &d3d_pitch)))
     goto free_images;

   /* copy safely the images that need to be drawn onto the back surface */
   for (l = buf->priv.pending_writes; l; l = l->next)
     {
	RGBA_Image             *im;
	Direct3D_Output_Buffer *d3dob;

	im = l->data;
	d3dob = im->extended_info;
	/* paste now */
	evas_direct3d_output_buffer_paste(d3dob,
                                          d3d_data,
                                          d3d_width,
                                          d3d_height,
                                          d3d_pitch,
                                          d3dob->x,
                                          d3dob->y);
     }

   /* unlock the texture */
   evas_direct3d_unlock(buf);

 free_images:
   while (buf->priv.pending_writes)
     {
        RGBA_Image             *im;
        Direct3D_Output_Buffer *d3dob;

        im = buf->priv.pending_writes->data;
        buf->priv.pending_writes = evas_list_remove_list(buf->priv.pending_writes,
                                                         buf->priv.pending_writes);
        d3dob = im->extended_info;
        evas_cache_image_drop(im);
        if (d3dob) evas_direct3d_output_buffer_free(d3dob);
     }
   evas_common_cpu_end_opt();
}

void
evas_direct3d_outbuf_push_updated_region(Outbuf     *buf,
                                         RGBA_Image *update,
                                         int         x,
                                         int         y,
                                         int         width,
                                         int         height)
{
   Gfx_Func_Convert        conv_func;
   Direct3D_Output_Buffer *d3dob;
   DATA32                 *src_data;
   void                   *data;
   int                     bpl = 0;

   conv_func = NULL;
   d3dob = update->extended_info;

   if ((buf->rot == 0) || (buf->rot == 180))
     conv_func = evas_common_convert_func_get(NULL,
                                              width,
                                              height,
                                              evas_direct3d_output_buffer_depth(d3dob),
                                              buf->priv.mask.r,
                                              buf->priv.mask.g,
                                              buf->priv.mask.b,
                                              PAL_MODE_NONE,
                                              buf->rot);
   else if ((buf->rot == 90) || (buf->rot == 270))
     conv_func = evas_common_convert_func_get(NULL,
                                              height,
                                              width,
                                              evas_direct3d_output_buffer_depth(d3dob),
                                              buf->priv.mask.r,
                                              buf->priv.mask.g,
                                              buf->priv.mask.b,
                                              PAL_MODE_NONE,
                                              buf->rot);

   if (!conv_func) return;

   data = evas_direct3d_output_buffer_data(d3dob, &bpl);
   src_data = update->image->data;
   if (buf->rot == 0)
     {
        d3dob->x = x;
        d3dob->y = y;
     }
   else if (buf->rot == 90)
     {
        d3dob->x = y;
        d3dob->y = buf->width - x - width;
     }
   else if (buf->rot == 180)
     {
        d3dob->x = buf->width - x - width;
        d3dob->y = buf->height - y - height;
     }
   else if (buf->rot == 270)
     {
        d3dob->x = buf->height - y - height;
        d3dob->y = x;
     }
   if ((buf->rot == 0) || (buf->rot == 180))
     {
        d3dob->width = width;
        d3dob->height = height;
     }
   else if ((buf->rot == 90) || (buf->rot == 270))
     {
        d3dob->width = height;
        d3dob->height = width;
     }

   if (data != src_data)
     conv_func(src_data, data,
               0,
               bpl /
               ((evas_direct3d_output_buffer_depth(d3dob))) - d3dob->width,
               d3dob->width, d3dob->height, x, y, NULL);
}

void
evas_direct3d_outbuf_reconfigure(Outbuf      *buf,
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
}

int
evas_direct3d_outbuf_width_get(Outbuf *buf)
{
   return buf->width;
}

int
evas_direct3d_outbuf_height_get(Outbuf *buf)
{
   return buf->height;
}

Outbuf_Depth
evas_direct3d_outbuf_depth_get(Outbuf *buf)
{
   return buf->depth;
}

int
evas_direct3d_outbuf_rot_get(Outbuf *buf)
{
   return buf->rot;
}
