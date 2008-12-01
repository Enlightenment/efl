#include "evas_common.h"
#include "evas_engine.h"


static Eina_List *ddpool = NULL;
static int ddsize = 0;
static int ddmemlimit = 10 * 1024 * 1024;
static int ddcountlimit = 32;

static DD_Output_Buffer *
_find_ddob(int depth, int w, int h, void *data)
{
   Eina_List        *l;
   Eina_List        *ddl;
   DD_Output_Buffer *ddob = NULL;
   DD_Output_Buffer *ddob2;
   int               sz;
   int               lbytes;
   int               bpp;

   bpp = depth / 8;
   if (bpp == 3) bpp = 4;
   lbytes = (((w * bpp) + 3) / 4) * 4;
   sz = lbytes * h;
   EINA_LIST_FOREACH(ddpool, l, ddob2)
     {
	if (ddob2->depth != depth)
	  continue;
	if (ddob2->psize == sz)
	  {
	     ddob = ddob2;
	     ddl = l;
	     goto have_ddob;
	  }
     }
   if (!ddob)
     return evas_software_ddraw_output_buffer_new(depth, w, h, data);

   have_ddob:
   ddpool = eina_list_remove_list(ddpool, ddl);
   ddob->width = w;
   ddob->height = h;
   ddob->pitch = lbytes;
   ddsize -= ddob->psize * (ddob->depth / 8);

   return ddob;
}

static void
_unfind_ddob(DD_Output_Buffer *ddob)
{
   ddpool = eina_list_prepend(ddpool, ddob);
   ddsize += ddob->psize * ddob->depth / 8;
   while ((ddsize > (ddmemlimit)) ||
          (eina_list_count(ddpool) > ddcountlimit))
     {
        Eina_List *xl;

        xl = eina_list_last(ddpool);
        if (!xl)
          {
             ddsize = 0;
             break;
          }
        ddob = xl->data;
        ddpool = eina_list_remove_list(ddpool, xl);
        evas_software_ddraw_output_buffer_free(ddob);
     }
}

static void
_clear_ddob(int sync)
{
   while (ddpool)
     {
	DD_Output_Buffer *ddob;

	ddob = ddpool->data;
	ddpool = eina_list_remove_list(ddpool, ddpool);
	evas_software_ddraw_output_buffer_free(ddob);
     }
   ddsize = 0;
}

void
evas_software_ddraw_outbuf_init(void)
{
}

void
evas_software_ddraw_outbuf_free(Outbuf *buf)
{
   if (!buf)
     return;

   evas_software_ddraw_shutdown(buf);
   free(buf);
}

Outbuf *
evas_software_ddraw_outbuf_setup(int          width,
                                 int          height,
                                 int          rotation,
                                 Outbuf_Depth depth,
                                 HWND         window,
                                 int          w_depth,
                                 int          fullscreen)
{
   Outbuf *buf;

   buf = (Outbuf *)calloc(1, sizeof(Outbuf));
   if (!buf)
      return NULL;

   buf->width = width;
   buf->height = height;
   buf->depth = depth;
   buf->rot = rotation;

   if (!evas_software_ddraw_init(window, w_depth, fullscreen, buf))
     {
        free(buf);
        return NULL;
     }

   {
      Gfx_Func_Convert  conv_func;
      DD_Output_Buffer *ddob;

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
                fprintf(stderr, ".[ Evas Error ].\n"
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

RGBA_Image *
evas_software_ddraw_outbuf_new_region_for_update(Outbuf *buf,
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

   if ((buf->rot == 0) &&
       (buf->priv.mask.r == 0xff0000) &&
       (buf->priv.mask.g == 0x00ff00) &&
       (buf->priv.mask.b == 0x0000ff))
     {
        obr->ddob = _find_ddob(buf->priv.dd.depth, w, h, NULL);
/*      obr->ddob = evas_software_x11_x_output_buffer_new(buf->priv.dd.disp, */
/*                                                         buf->priv.dd.vis, */
/*                                                         buf->priv.dd.depth, */
/*                                                         w, h, */
/*                                                         use_shm, */
/*                                                         NULL); */
        im = (RGBA_Image *)evas_cache_image_data(evas_common_image_cache_get(),
                                                 w, h,
                                                 (DATA32 *) evas_software_ddraw_output_buffer_data(obr->ddob, &bpl),
                                                 alpha, EVAS_COLORSPACE_ARGB8888);
        im->extended_info = obr;
     }
   else
     {
        im = (RGBA_Image *) evas_cache_image_empty(evas_common_image_cache_get());
        im->cache_entry.flags.alpha |= alpha ? 1 : 0;
        evas_cache_image_surface_alloc(&im->cache_entry, w, h);
        im->extended_info = obr;
        if ((buf->rot == 0) || (buf->rot == 180))
          obr->ddob = _find_ddob(buf->priv.dd.depth, w, h, NULL);
/*
          obr->ddob = evas_software_x11_x_output_buffer_new(buf->priv.dd.disp,
                                                           buf->priv.dd.vis,
                                                           buf->priv.dd.depth,
                                                           w, h,
                                                           use_shm,
                                                           NULL);
 */
        else if ((buf->rot == 90) || (buf->rot == 270))
          obr->ddob = _find_ddob(buf->priv.dd.depth, h, w, NULL);
/*
          obr->ddob = evas_software_x11_x_output_buffer_new(buf->priv.dd.disp,
                                                           buf->priv.dd.vis,
                                                           buf->priv.dd.depth,
                                                           h, w,
                                                           use_shm,
                                                           NULL);
 */
     }

   buf->priv.pending_writes = eina_list_append(buf->priv.pending_writes, im);
   return im;
}

void
evas_software_ddraw_outbuf_push_updated_region(Outbuf     *buf,
                                               RGBA_Image *update,
                                               int        x,
                                               int        y,
                                               int        w,
                                               int        h)
{
   Gfx_Func_Convert    conv_func;
   Outbuf_Region      *obr;
   DATA32             *src_data;
   void               *data;
   int                 bpl = 0, yy;

   conv_func = NULL;
   obr = update->extended_info;

   if ((buf->rot == 0) || (buf->rot == 180))
     conv_func = evas_common_convert_func_get(0, w, h,
                                              evas_software_ddraw_output_buffer_depth(obr->ddob),
                                              buf->priv.mask.r,
                                              buf->priv.mask.g,
                                              buf->priv.mask.b,
                                              PAL_MODE_NONE,
                                              buf->rot);
   else if ((buf->rot == 90) || (buf->rot == 270))
     conv_func = evas_common_convert_func_get(0, h, w,
                                              evas_software_ddraw_output_buffer_depth(obr->ddob),
                                              buf->priv.mask.r,
                                              buf->priv.mask.g,
                                              buf->priv.mask.b,
                                              PAL_MODE_NONE, buf->rot);
   if (!conv_func) return;

   data = evas_software_ddraw_output_buffer_data(obr->ddob, &bpl);
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
               bpl / ((evas_software_ddraw_output_buffer_depth(obr->ddob) / 8)) - obr->width,
               obr->width,
               obr->height,
               x,
               y,
               NULL);
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
   Eina_List *l;
   RGBA_Image       *im;
   Outbuf_Region    *obr;
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
   EINA_LIST_FOREACH(buf->priv.pending_writes, l, im)
     {
	DD_Output_Buffer *ddob;

        obr = im->extended_info;
        ddob = obr->ddob;
        evas_software_ddraw_output_buffer_paste(ddob,
                                                ddraw_data,
                                                ddraw_width,
                                                ddraw_height,
                                                ddraw_pitch,
                                                ddraw_depth,
						obr->x,
						obr->y);
     }

   /* unlock the back surface and flip the surface */
   evas_software_ddraw_unlock_and_flip(buf);

 free_images:
   while (buf->priv.prev_pending_writes)
     {
        im = buf->priv.prev_pending_writes->data;
        buf->priv.prev_pending_writes =
          eina_list_remove_list(buf->priv.prev_pending_writes,
                                buf->priv.prev_pending_writes);
        obr = im->extended_info;
        evas_cache_image_drop((Image_Entry *)im);
        if (obr->ddob) _unfind_ddob(obr->ddob);
/*
	     if (obr->ddob) evas_software_x11_x_output_buffer_free(obr->ddob);
 */
        free(obr);
     }
   buf->priv.prev_pending_writes = buf->priv.pending_writes;
   buf->priv.pending_writes = NULL;

   evas_common_cpu_end_opt();
}

void
evas_software_ddraw_outbuf_idle_flush(Outbuf *buf)
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
        if (obr->ddob) _unfind_ddob(obr->ddob);
        free(obr);
     }
   _clear_ddob(0);
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
