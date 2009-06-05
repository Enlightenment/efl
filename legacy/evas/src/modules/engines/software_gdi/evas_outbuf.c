#include "evas_common.h"
#include "evas_engine.h"


static Eina_List *gdipool = NULL;
static int gdisize = 0;
static int gdimemlimit = 10 * 1024 * 1024;
static int gdicountlimit = 32;

static Gdi_Output_Buffer *
_find_gdiob(HDC dc, BITMAPINFO_GDI *bitmap_info, int depth, int w, int h, void *data)
{
   Eina_List         *l;
   Eina_List         *gdil;
   Gdi_Output_Buffer *gdiob = NULL;
   Gdi_Output_Buffer *gdiob2;
   int                sz;
   int                lbytes;
   int                bpp;
   int                fitness = 0x7fffffff;

   bpp = depth >> 3;
   if (bpp == 3) bpp = 4;
   lbytes = (((w * bpp) + 3) / 4) * 4;
   sz = lbytes * h;
   EINA_LIST_FOREACH(gdipool, l, gdiob2)
     {
	int szdif;

        if ((gdiob2->dc != dc) ||
            (gdiob2->bitmap_info != bitmap_info) ||
            (gdiob2->depth != depth))
	  continue;
	szdif = gdiob2->psize - sz;
	if (szdif < 0) continue;
	if (szdif == 0)
	  {
	     gdiob = gdiob2;
	     gdil = l;
	     goto have_gdiob;
	  }
	if (szdif < fitness)
	  {
	     fitness = szdif;
	     gdiob = gdiob2;
	     gdil = l;
	  }
     }
   if ((fitness > (100 * 100)) || (!gdiob))
     return evas_software_gdi_output_buffer_new(dc, bitmap_info, depth, w, h, data);

   have_gdiob:
   gdipool = eina_list_remove_list(gdipool, gdil);
   gdiob->width = w;
   gdiob->height = h;
   gdiob->pitch = lbytes;
   gdisize -= gdiob->psize * (gdiob->depth >> 3);

   return gdiob;
}

static void
_unfind_gdiob(Gdi_Output_Buffer *gdiob)
{
   gdipool = eina_list_prepend(gdipool, gdiob);
   gdisize += gdiob->psize * (gdiob->depth >> 3);
   while ((gdisize > (gdimemlimit)) ||
          (eina_list_count(gdipool) > gdicountlimit))
     {
        Eina_List *xl;

        xl = eina_list_last(gdipool);
        if (!xl)
          {
             gdisize = 0;
             break;
          }
        gdiob = xl->data;
        gdipool = eina_list_remove_list(gdipool, xl);
        evas_software_gdi_output_buffer_free(gdiob);
     }
}

static void
_clear_gdiob()
{
   while (gdipool)
     {
	Gdi_Output_Buffer *gdiob;

	gdiob = gdipool->data;
	gdipool = eina_list_remove_list(gdipool, gdipool);
	evas_software_gdi_output_buffer_free(gdiob);
     }
   gdisize = 0;
}

void
evas_software_gdi_outbuf_init(void)
{
}

void
evas_software_gdi_outbuf_free(Outbuf *buf)
{
   if (!buf)
     return;

   while (buf->priv.pending_writes)
     {
	RGBA_Image *im;
	Outbuf_Region *obr;

	im = buf->priv.pending_writes->data;
	buf->priv.pending_writes = eina_list_remove_list(buf->priv.pending_writes, buf->priv.pending_writes);
	obr = im->extended_info;
	evas_cache_image_drop(&im->cache_entry);
	if (obr->gdiob) _unfind_gdiob(obr->gdiob);
/* 	if (obr->mxob) _unfind_xob(obr->mxob, 0); */
	free(obr);
     }
   evas_software_gdi_outbuf_idle_flush(buf);
   evas_software_gdi_outbuf_flush(buf);

   evas_software_gdi_shutdown(buf);
   free(buf);
}

Outbuf *
evas_software_gdi_outbuf_setup(int          width,
                               int          height,
                               int          rotation,
                               Outbuf_Depth depth,
                               HWND         window,
                               HBITMAP      mask,
                               int          w_depth,
                               unsigned int layered,
                               unsigned int fullscreen,
                               int          mask_dither,
                               int          destination_alpha)
{
   Outbuf *buf;

   buf = (Outbuf *)calloc(1, sizeof(Outbuf));
   if (!buf)
      return NULL;

   buf->width = width;
   buf->height = height;
   buf->depth = depth;
   buf->rot = rotation;

   buf->priv.mask_dither = mask_dither;
   buf->priv.destination_alpha = destination_alpha;

   if (!evas_software_gdi_init(window, mask, w_depth, layered, fullscreen, buf))
     {
        free(buf);
        return NULL;
     }

   {
      Gfx_Func_Convert  conv_func;
      Gdi_Output_Buffer *gdiob;

      gdiob = evas_software_gdi_output_buffer_new(buf->priv.gdi.dc, buf->priv.gdi.bitmap_info, w_depth, 1, 1, NULL);

      conv_func = NULL;
      if (gdiob)
        {
           if ((rotation == 0) || (rotation == 180))
             conv_func = evas_common_convert_func_get(0,
                                                      width,
                                                      height,
                                                      evas_software_gdi_output_buffer_depth (gdiob),
                                                      buf->priv.gdi.bitmap_info->masks[0],
                                                      buf->priv.gdi.bitmap_info->masks[1],
                                                      buf->priv.gdi.bitmap_info->masks[2],
                                                      PAL_MODE_NONE,
                                                      rotation);
           else if ((rotation == 90) || (rotation == 270))
             conv_func = evas_common_convert_func_get(0,
                                                      height,
                                                      width,
                                                      evas_software_gdi_output_buffer_depth (gdiob),
                                                      buf->priv.gdi.bitmap_info->masks[0],
                                                      buf->priv.gdi.bitmap_info->masks[1],
                                                      buf->priv.gdi.bitmap_info->masks[2],
                                                      PAL_MODE_NONE,
                                                      rotation);

           evas_software_gdi_output_buffer_free(gdiob);

           if (!conv_func)
             {
                fprintf(stderr, ".[ Evas Error ].\n"
                        " {\n"
                        "  At depth         %i:\n"
                        "  RGB format mask: %08lx, %08lx, %08lx\n"
                        "  Not supported by and compiled in converters!\n"
                        " }\n",
                        buf->priv.gdi.depth,
                        buf->priv.gdi.bitmap_info->masks[0],
                        buf->priv.gdi.bitmap_info->masks[1],
                        buf->priv.gdi.bitmap_info->masks[2]);
             }
        }
      if (buf->priv.gdi.mask != mask)
        buf->priv.gdi.mask = mask;
   }

   return buf;
}

void
evas_software_gdi_outbuf_reconfigure(Outbuf      *buf,
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
   evas_software_gdi_bitmap_resize(buf);
}

RGBA_Image *
evas_software_gdi_outbuf_new_region_for_update(Outbuf *buf,
                                               int     x,
                                               int     y,
                                               int     w,
                                               int     h,
                                               int    *cx,
                                               int    *cy,
                                               int    *cw,
                                               int    *ch)
{
   RGBA_Image    *im;
   Outbuf_Region *obr;
   int            bpl = 0;
   int            alpha = 0;

   obr = calloc(1, sizeof(Outbuf_Region));
   obr->x = x;
   obr->y = y;
   obr->width = w;
   obr->height = h;
   *cx = 0;
   *cy = 0;
   *cw = w;
   *ch = h;

   alpha = ((buf->priv.gdi.mask) || (buf->priv.destination_alpha));

   if ((buf->rot == 0) &&
       (buf->priv.gdi.bitmap_info->masks[0] == 0xff0000) &&
       (buf->priv.gdi.bitmap_info->masks[1] == 0x00ff00) &&
       (buf->priv.gdi.bitmap_info->masks[2] == 0x0000ff))
     {
        obr->gdiob = _find_gdiob(buf->priv.gdi.dc,
                                 buf->priv.gdi.bitmap_info,
                                 buf->priv.gdi.depth,
                                 w, h, NULL);
/*      obr->gdiob = evas_software_gdi_output_buffer_new(buf->priv.gdi.dc, */
/*                                                         buf->priv.gdi.bitmap_info, */
/*                                                         buf->priv.gdi.depth, */
/*                                                         w, h, */
/*                                                         NULL); */
        im = (RGBA_Image *)evas_cache_image_data(evas_common_image_cache_get(),
                                                 w, h,
                                                 (DATA32 *)evas_software_gdi_output_buffer_data(obr->gdiob, &bpl),
                                                 alpha, EVAS_COLORSPACE_ARGB8888);
        im->extended_info = obr;
/* 	if (buf->priv.gdi.mask) */
/* 	  obr->mgdiob = _find_gdiob(buf->priv.gdi.dc, */
/*                                     buf->priv.gdi.bitmap_info, */
/*                                     1, */
/*                                     w, h, NULL); */
     }
   else
     {
        im = (RGBA_Image *) evas_cache_image_empty(evas_common_image_cache_get());
        im->cache_entry.flags.alpha |= alpha ? 1 : 0;
        evas_cache_image_surface_alloc(&im->cache_entry, w, h);
        im->extended_info = obr;
        if ((buf->rot == 0) || (buf->rot == 180))
          obr->gdiob = _find_gdiob(buf->priv.gdi.dc,
                                   buf->priv.gdi.bitmap_info,
                                   buf->priv.gdi.depth,
                                   w, h, NULL);
/*
          obr->gdiob = evas_software_x11_x_output_buffer_new(buf->priv.dd.disp,
                                                           buf->priv.dd.vis,
                                                           buf->priv.dd.depth,
                                                           w, h,
                                                           use_shm,
                                                           NULL);
 */
        else if ((buf->rot == 90) || (buf->rot == 270))
          obr->gdiob = _find_gdiob(buf->priv.gdi.dc,
                                   buf->priv.gdi.bitmap_info,
                                   buf->priv.gdi.depth,
                                   h, w, NULL);
/*
          obr->gdiob = evas_software_x11_x_output_buffer_new(buf->priv.dd.disp,
                                                           buf->priv.dd.vis,
                                                           buf->priv.dd.depth,
                                                           h, w,
                                                           use_shm,
                                                           NULL);
 */
/* 	if (buf->priv.gdi.mask) */
/* 	  obr->mgdiob = _find_gdiob(buf->priv.gdi.dc, */
/*                                     buf->priv.gdi.bitmap_info, */
/*                                     1, */
/*                                     w, h, NULL); */
     }
   if ((buf->priv.gdi.mask) || (buf->priv.destination_alpha))
     /* FIXME: faster memset! */
     memset(im->image.data, 0, w * h * sizeof(DATA32));

   buf->priv.pending_writes = eina_list_append(buf->priv.pending_writes, im);
   return im;
}

void
evas_software_gdi_outbuf_push_updated_region(Outbuf     *buf,
                                             RGBA_Image *update,
                                             int         x,
                                             int         y,
                                             int         w,
                                             int         h)
{
   Gfx_Func_Convert conv_func;
   Outbuf_Region   *obr;
   DATA32          *src_data;
   void            *data;
   int              bpl = 0;

   conv_func = NULL;
   obr = update->extended_info;

   if ((buf->rot == 0) || (buf->rot == 180))
     conv_func = evas_common_convert_func_get(0, w, h,
                                              evas_software_gdi_output_buffer_depth(obr->gdiob),
                                              buf->priv.gdi.bitmap_info->masks[0],
                                              buf->priv.gdi.bitmap_info->masks[1],
                                              buf->priv.gdi.bitmap_info->masks[2],
                                              PAL_MODE_NONE,
                                              buf->rot);
   else if ((buf->rot == 90) || (buf->rot == 270))
     conv_func = evas_common_convert_func_get(0, h, w,
                                              evas_software_gdi_output_buffer_depth(obr->gdiob),
                                              buf->priv.gdi.bitmap_info->masks[0],
                                              buf->priv.gdi.bitmap_info->masks[1],
                                              buf->priv.gdi.bitmap_info->masks[2],
                                              PAL_MODE_NONE,
                                              buf->rot);
   if (!conv_func) return;

   data = evas_software_gdi_output_buffer_data(obr->gdiob, &bpl);
   src_data = update->image.data;
   if (buf->rot == 0)
     {
	obr->x = x;
	obr->y = y;
     }
   else if (buf->rot == 90)
     {
	obr->x = y;
	obr->y = buf->width - x - w;
     }
   else if (buf->rot == 180)
     {
	obr->x = buf->width - x - w;
	obr->y = buf->height - y - h;
     }
   else if (buf->rot == 270)
     {
	obr->x = buf->height - y - h;
	obr->y = x;
     }
   if ((buf->rot == 0) || (buf->rot == 180))
     {
	obr->width = w;
	obr->height = h;
     }
   else if ((buf->rot == 90) || (buf->rot == 270))
     {
	obr->width = h;
	obr->height = w;
     }

   if (data != src_data)
     conv_func(src_data, data,
               0,
               bpl / (evas_software_gdi_output_buffer_depth(obr->gdiob) >> 3) - obr->width,
               obr->width,
               obr->height,
               x,
               y,
               NULL);
/*    if (obr->mxob) */
/*      { */
/*    int              yy; */
/* 	for (yy = 0; yy < obr->h; yy++) */
/* 	  evas_software_xlib_x_write_mask_line(buf, obr->mxob, */
/* 					      src_data + */
/* 					      (yy * obr->w), obr->w, yy); */
}

void
evas_software_gdi_outbuf_free_region_for_update(Outbuf     *buf __UNUSED__,
                                                RGBA_Image *update __UNUSED__)
{
   /* no need to do anything - they are cleaned up on flush */
}

void
evas_software_gdi_outbuf_flush(Outbuf *buf)
{
   Eina_List     *l;
   RGBA_Image    *im;
   Outbuf_Region *obr;

   /* copy safely the images that need to be drawn onto the back surface */
   EINA_LIST_FOREACH(buf->priv.pending_writes, l, im)
     {
	Gdi_Output_Buffer *gdiob;

        obr = im->extended_info;
        gdiob = obr->gdiob;
        evas_software_gdi_output_buffer_paste(gdiob,
                                              obr->x,
                                              obr->y);
/*         if (obr->mgdiob) */
/*           evas_software_gdi_output_buffer_paste(obr->mgdiob, */
/*                                                 buf->priv.x11.xlib.mask, */
/*                                                 buf->priv.x11.xlib.gcm, */
/*                                                 obr->x, obr->y, 0); */
     }

   while (buf->priv.prev_pending_writes)
     {
        im = buf->priv.prev_pending_writes->data;
        buf->priv.prev_pending_writes =
          eina_list_remove_list(buf->priv.prev_pending_writes,
                                buf->priv.prev_pending_writes);
        obr = im->extended_info;
        evas_cache_image_drop(&im->cache_entry);
        if (obr->gdiob) _unfind_gdiob(obr->gdiob);
/*         if (obr->mgdiob) _unfind_gdiob(obr->mgdiob); */
/*         if (obr->gdiob) evas_software_x11_x_output_buffer_free(obr->gdiob); */
        free(obr);
     }
   buf->priv.prev_pending_writes = buf->priv.pending_writes;
   buf->priv.pending_writes = NULL;

   evas_common_cpu_end_opt();
}

void
evas_software_gdi_outbuf_idle_flush(Outbuf *buf)
{
   while (buf->priv.prev_pending_writes)
     {
        RGBA_Image *im;
        Outbuf_Region *obr;

        im = buf->priv.prev_pending_writes->data;
        buf->priv.prev_pending_writes =
          eina_list_remove_list(buf->priv.prev_pending_writes,
                                buf->priv.prev_pending_writes);
        obr = im->extended_info;
        evas_cache_image_drop((Image_Entry *)im);
        if (obr->gdiob) _unfind_gdiob(obr->gdiob);
/*         if (obr->mxob) _unfind_xob(obr->mxob, 0); */
        free(obr);
     }
   _clear_gdiob();
}

int
evas_software_gdi_outbuf_width_get(Outbuf *buf)
{
   return buf->width;
}

int
evas_software_gdi_outbuf_height_get(Outbuf *buf)
{
   return buf->height;
}

Outbuf_Depth
evas_software_gdi_outbuf_depth_get(Outbuf *buf)
{
   return buf->depth;
}

int
evas_software_gdi_outbuf_rot_get(Outbuf *buf)
{
   return buf->rot;
}
