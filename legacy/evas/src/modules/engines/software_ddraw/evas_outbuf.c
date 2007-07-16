#include "evas_engine.h"


static void
_ddraw_surface_flip(HWND                window,
                    LPDIRECTDRAWSURFACE surface_primary,
                    LPDIRECTDRAWSURFACE surface_back,
                    int                 width,
                    int                 height)
{
   HRESULT res;
   RECT    dst_rect;
   RECT    src_rect;
   POINT   p;

   /* we figure out where on the primary surface our window lives */
   p.x = 0;
   p.y = 0;
   ClientToScreen (window, &p);
   GetClientRect (window, &dst_rect);
   OffsetRect (&dst_rect, p.x, p.y);
   SetRect (&src_rect, 0, 0, width, height);
   res = IDirectDrawSurface7_Blt (surface_primary, &dst_rect,
                                  surface_back, &src_rect,
                                  DDBLT_WAIT, NULL);
   if (FAILED(res))
     {
     }
}


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
           DDPIXELFORMAT pixel_format;

           ZeroMemory(&pixel_format, sizeof(pixel_format));
           pixel_format.dwSize = sizeof(pixel_format);
           IDirectDrawSurface7_GetPixelFormat(surface_primary, &pixel_format);
           buf->priv.mask.r = pixel_format.dwRBitMask;
           buf->priv.mask.g = pixel_format.dwGBitMask;
           buf->priv.mask.b = pixel_format.dwBBitMask;

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
   DDSURFACEDESC2 surface_desc;
   HRESULT        res;
   Evas_List     *l;

   ZeroMemory(&surface_desc, sizeof(surface_desc));
   surface_desc.dwSize = sizeof(surface_desc);

   /* lock the back surface */
   res = IDirectDrawSurface7_Lock (buf->priv.dd.surface_back, NULL, &surface_desc,
                                   DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
   if (FAILED(res)) goto free_images;

   /* copy safely the images that need to be drawn onto the back surface */
   for (l = buf->priv.pending_writes; l; l = l->next)
     {
	RGBA_Image          *im;
	DDraw_Output_Buffer *ddob;

	im = l->data;
	ddob = im->extended_info;
	/* paste now */
	evas_software_ddraw_output_buffer_paste(ddob,
						&surface_desc,
						ddob->x,
						ddob->y);
     }

   /* unlock the back surface */
   res = IDirectDrawSurface7_Unlock (buf->priv.dd.surface_back, NULL);
   if (FAILED(res)) goto free_images;

   /* flip the surfaces */
   _ddraw_surface_flip(buf->priv.dd.window,
		       buf->priv.dd.surface_primary,
		       buf->priv.dd.surface_back,
		       buf->width,
		       buf->height);

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
