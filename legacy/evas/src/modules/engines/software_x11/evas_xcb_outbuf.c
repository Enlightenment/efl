#include "evas_common.h"
#include "evas_macros.h"
#include "evas_xcb_outbuf.h"
#include "evas_xcb_buffer.h"
#include "evas_xcb_color.h"
#include <pixman.h>

/* local structures */
typedef struct _Outbuf_Region Outbuf_Region;
struct _Outbuf_Region 
{
   Xcb_Output_Buffer *xcbob, *mask;
   int x, y, w, h;
};

/* local function prototypes */
static Xcb_Output_Buffer *_find_xcbob(xcb_connection_t *conn, xcb_visualtype_t *vis, int depth, int w, int h, Eina_Bool shm, void *data);
static void _unfind_xcbob(Xcb_Output_Buffer *xcbob, Eina_Bool sync);
static void _clear_xcbob(Eina_Bool sync);
static xcb_format_t *_find_format_by_depth(const xcb_setup_t *setup, uint8_t depth);
static void _xcbob_sync(xcb_connection_t *conn);

/* local variables */
static Eina_List *_shmpool = NULL;
static int _shmsize = 0;
static int _shmlimit = (10 * 1024 * 1024);
static const unsigned int _shmcountlimit = 32;

#ifdef EVAS_FRAME_QUEUING
static LK(lock_shmpool);
# define SHMPOOL_LOCK() LKL(lock_shmpool);
# define SHMPOOL_UNLOCK() LKU(lock_shmpool);
#else
# define SHMPOOL_LOCK()
# define SHMPOOL_UNLOCK()
#endif

void 
evas_software_xcb_outbuf_init(void) 
{
#ifdef EVAS_FRAME_QUEUING
   LKI(lock_shmpool);
#endif
}

void 
evas_software_xcb_outbuf_free(Outbuf *buf) 
{
#ifdef EVAS_FRAME_QUEUING
   LKL(buf->priv.lock);
#endif
   while (buf->priv.pending_writes) 
     {
        RGBA_Image *im = NULL;
        Outbuf_Region *obr = NULL;

        im = buf->priv.pending_writes->data;
        buf->priv.pending_writes = 
          eina_list_remove_list(buf->priv.pending_writes, 
                                buf->priv.pending_writes);
        obr = im->extended_info;
        evas_cache_image_drop(&im->cache_entry);
        if (obr->xcbob) _unfind_xcbob(obr->xcbob, EINA_FALSE);
        if (obr->mask) _unfind_xcbob(obr->mask, EINA_FALSE);
        free(obr);
     }
#ifdef EVAS_FRAME_QUEUING
   LKU(buf->priv.lock);
#endif
   evas_software_xcb_outbuf_idle_flush(buf);
   evas_software_xcb_outbuf_flush(buf);
   if (buf->priv.x11.xcb.gc)
     xcb_free_gc(buf->priv.x11.xcb.conn, buf->priv.x11.xcb.gc);
   if (buf->priv.x11.xcb.gcm)
     xcb_free_gc(buf->priv.x11.xcb.conn, buf->priv.x11.xcb.gcm);
   if (buf->priv.pal)
     evas_software_xcb_color_deallocate(buf->priv.x11.xcb.conn, 
                                        buf->priv.x11.xcb.cmap, 
                                        buf->priv.x11.xcb.visual, 
                                        buf->priv.pal);
#ifdef EVAS_FRAME_QUEUING
   LKD(buf->priv.lock);
#endif
   free(buf);
   _clear_xcbob(EINA_FALSE);
}

Outbuf *
evas_software_xcb_outbuf_setup(int w, int h, int rot, Outbuf_Depth depth, xcb_connection_t *conn, xcb_screen_t *screen, xcb_drawable_t draw, xcb_visualtype_t *vis, xcb_colormap_t cmap, int xdepth, Eina_Bool grayscale, int max_colors, xcb_drawable_t mask, Eina_Bool shape_dither, Eina_Bool alpha) 
{
   Outbuf *buf = NULL;
   Gfx_Func_Convert func_conv= NULL;
   const xcb_setup_t *setup;
   xcb_format_t *fmt;

   if (!(buf = calloc(1, sizeof(Outbuf)))) 
     return NULL;

   setup = xcb_get_setup(conn);
   fmt = _find_format_by_depth(setup, xdepth);

   buf->w = w;
   buf->h = h;
   buf->depth = depth;
   buf->rot = rot;
   buf->priv.x11.xcb.conn = conn;
   buf->priv.x11.xcb.screen = screen;
   buf->priv.x11.xcb.visual = vis;
   buf->priv.x11.xcb.cmap = cmap;
   buf->priv.x11.xcb.depth = xdepth;
   buf->priv.mask_dither = shape_dither;
   buf->priv.destination_alpha = alpha;
   buf->priv.x11.xcb.shm = evas_software_xcb_can_do_shm(conn, screen);

#ifdef WORDS_BIGENDIAN
   if (setup->image_byte_order == XCB_IMAGE_ORDER_LSB_FIRST)
     buf->priv.x11.xcb.swap = EINA_TRUE;
#else
   if (setup->image_byte_order == XCB_IMAGE_ORDER_MSB_FIRST)
     buf->priv.x11.xcb.swap = EINA_TRUE;
#endif
   if (setup->bitmap_format_bit_order == XCB_IMAGE_ORDER_MSB_FIRST)
     buf->priv.x11.xcb.bit_swap = EINA_TRUE;

   if (((vis->_class == XCB_VISUAL_CLASS_TRUE_COLOR) || 
        (vis->_class == XCB_VISUAL_CLASS_DIRECT_COLOR)) && (xdepth > 8)) 
     {
        buf->priv.mask.r = (DATA32)vis->red_mask;
        buf->priv.mask.g = (DATA32)vis->green_mask;
        buf->priv.mask.b = (DATA32)vis->blue_mask;
        if (buf->priv.x11.xcb.swap) 
          {
             SWAP32(buf->priv.mask.r);
             SWAP32(buf->priv.mask.g);
             SWAP32(buf->priv.mask.b);
          }
     }
   else if ((vis->_class == XCB_VISUAL_CLASS_PSEUDO_COLOR) || 
            (vis->_class == XCB_VISUAL_CLASS_STATIC_COLOR) || 
            (vis->_class == XCB_VISUAL_CLASS_GRAY_SCALE) || 
            (vis->_class == XCB_VISUAL_CLASS_STATIC_GRAY) || 
            (xdepth <= 8)) 
     {
        Convert_Pal_Mode pm = PAL_MODE_RGB332;

        if ((vis->_class == XCB_VISUAL_CLASS_GRAY_SCALE) || 
            (vis->_class == XCB_VISUAL_CLASS_STATIC_GRAY))
          grayscale = EINA_TRUE;
        if (grayscale) 
          {
             if (max_colors >= 256)
               pm = PAL_MODE_GRAY256;
             else if (max_colors >= 64)
               pm = PAL_MODE_GRAY64;
             else if (max_colors >= 16)
               pm = PAL_MODE_GRAY16;
             else if (max_colors >= 4)
               pm = PAL_MODE_GRAY4;
             else
               pm = PAL_MODE_MONO;
          }
        else 
          {
             if (max_colors >= 256)
               pm = PAL_MODE_RGB332;
             else if (max_colors >= 216)
               pm = PAL_MODE_RGB666;
             else if (max_colors >= 128)
               pm = PAL_MODE_RGB232;
             else if (max_colors >= 64)
               pm = PAL_MODE_RGB222;
             else if (max_colors >= 32)
               pm = PAL_MODE_RGB221;
             else if (max_colors >= 16)
               pm = PAL_MODE_RGB121;
             else if (max_colors >= 8)
               pm = PAL_MODE_RGB111;
             else if (max_colors >= 4)
               pm = PAL_MODE_GRAY4;
             else
               pm = PAL_MODE_MONO;
          }
        /* FIXME: Only allocate once per display & colormap */
        buf->priv.pal = 
          evas_software_xcb_color_allocate(conn, cmap, vis, pm);
        if (!buf->priv.pal) 
          {
             free(buf);
             return NULL;
          }
     }
   if ((buf->rot == 0) || (buf->rot == 180))
     {
        w = buf->w;
        h = buf->h;
     }
   else if ((buf->rot == 90) || (buf->rot == 270)) 
     {
        w = buf->h;
        h = buf->w;
     }

   if (buf->priv.pal) 
     {
        func_conv = 
          evas_common_convert_func_get(0, w, h, fmt->bits_per_pixel, 
                                       buf->priv.mask.r, 
                                       buf->priv.mask.g, 
                                       buf->priv.mask.b, 
                                       buf->priv.pal->colors, buf->rot);
     }
   else 
     {
        func_conv = 
          evas_common_convert_func_get(0, w, h, fmt->bits_per_pixel, 
                                       buf->priv.mask.r, 
                                       buf->priv.mask.g, 
                                       buf->priv.mask.b, 
                                       PAL_MODE_NONE, buf->rot);
     }
   if (!func_conv) 
     {
        ERR("XCB Engine"
            " {"
            "  At depth         %i:"
            "  RGB format mask: %08x, %08x, %08x"
            "  Palette mode:    %i"
            "  Not supported by and compiled in converters!"
            " }", buf->priv.x11.xcb.depth, buf->priv.mask.r,
            buf->priv.mask.g, buf->priv.mask.b,
            buf->priv.pal ? (int)buf->priv.pal->colors : -1);
     }

   evas_software_xcb_outbuf_drawable_set(buf, draw);
   evas_software_xcb_outbuf_mask_set(buf, mask);

#ifdef EVAS_FRAME_QUEUING
   LKI(buf->priv.lock);
#endif

   return buf;
}

RGBA_Image *
evas_software_xcb_outbuf_new_region_for_update(Outbuf *buf, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch) 
{
   RGBA_Image *im = NULL;
   Outbuf_Region *obr = NULL;
   Eina_Bool use_shm = EINA_TRUE;
   Eina_Bool alpha = EINA_FALSE;
   int bpl = 0;

   if ((buf->onebuf) && (buf->priv.x11.xcb.shm)) 
     {
        Eina_Rectangle *rect;

        RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, buf->w, buf->h);

        if (!(obr = calloc(1, sizeof(Outbuf_Region))))
          return NULL;

        if (!(rect = eina_rectangle_new(x, y, w, h))) 
          {
             free(obr);
             return NULL;
          }

        buf->priv.onebuf_regions = 
          eina_list_append(buf->priv.onebuf_regions, rect);
        if (buf->priv.onebuf) 
          {
             if (cx) *cx = x;
             if (cy) *cy = y;
             if (cw) *cw = w;
             if (ch) *ch = h;
             if (!buf->priv.synced) 
               {
                  _xcbob_sync(buf->priv.x11.xcb.conn);
                  buf->priv.synced = EINA_TRUE;
               }
             return buf->priv.onebuf;
          }
        obr->x = 0;
        obr->y = 0;
        obr->w = buf->w;
        obr->h = buf->h;
        if (cx) *cx = x;
        if (cy) *cy = y;
        if (cw) *cw = w;
        if (ch) *ch = h;

        alpha = ((buf->priv.x11.xcb.mask) || (buf->priv.destination_alpha));
        use_shm = buf->priv.x11.xcb.shm;

        if ((buf->rot == 0) && (buf->priv.mask.r == 0xff0000) && 
            (buf->priv.mask.g == 0x00ff00) && (buf->priv.mask.b == 0x0000ff)) 
          {
             obr->xcbob = 
               evas_software_xcb_output_buffer_new(buf->priv.x11.xcb.conn, 
                                                   buf->priv.x11.xcb.visual, 
                                                   buf->priv.x11.xcb.depth, 
                                                   buf->w, buf->h, use_shm, 
                                                   NULL);
             if (!obr->xcbob) 
               {
                  free(obr);
                  return NULL;
               }
             im = 
               (RGBA_Image *)evas_cache_image_data(evas_common_image_cache_get(), 
                                                   buf->w, buf->h, 
                                                   (DATA32 *)evas_software_xcb_output_buffer_data(obr->xcbob, &bpl), 
                                                   alpha, EVAS_COLORSPACE_ARGB8888);
             if (!im) 
               {
                  evas_software_xcb_output_buffer_free(obr->xcbob, EINA_FALSE);
                  free(obr);
                  return NULL;
               }
             im->extended_info = obr;
             if (buf->priv.x11.xcb.mask) 
               {
                  obr->mask = 
                    evas_software_xcb_output_buffer_new(buf->priv.x11.xcb.conn, 
                                                        buf->priv.x11.xcb.visual, 
                                                        1, buf->w, buf->h, 
                                                        use_shm, NULL);
               }
          }
        else 
          {
             int bw = 0, bh = 0;

             im = 
               (RGBA_Image *)evas_cache_image_empty(evas_common_image_cache_get());
             if (!im) 
               {
                  free(obr);
                  return NULL;
               }
             im->cache_entry.flags.alpha |= (alpha ? 1 : 0);
             evas_cache_image_surface_alloc(&im->cache_entry, buf->w, buf->h);
             im->extended_info = obr;
             if ((buf->rot == 0) || (buf->rot == 180)) 
               {
                  bw = buf->w;
                  bh = buf->h;
               }
             else if ((buf->rot == 90) || (buf->rot == 270)) 
               {
                  bw = buf->h;
                  bh = buf->w;
               }
             obr->xcbob = 
               evas_software_xcb_output_buffer_new(buf->priv.x11.xcb.conn, 
                                                   buf->priv.x11.xcb.visual, 
                                                   buf->priv.x11.xcb.depth, 
                                                   bw, bh, use_shm, NULL);
             if (!obr->xcbob) 
               {
                  evas_cache_image_drop(&im->cache_entry);
                  free(obr);
                  return NULL;
               }
             if (buf->priv.x11.xcb.mask) 
               {
                  obr->mask = 
                    evas_software_xcb_output_buffer_new(buf->priv.x11.xcb.conn, 
                                                        buf->priv.x11.xcb.visual, 
                                                        1, bw, bh, use_shm, 
                                                        NULL);
               }
          }
        /* FIXME: We should be able to remove this memset. */
        if ((alpha) && (im->image.data)) 
          {
             /* FIXME: Faster memset */
//             memset(im->image.data, 0, (w * h * sizeof(DATA32)));
          }
        buf->priv.onebuf = im;
        return im;
     }

   if (!(obr = calloc(1, sizeof(Outbuf_Region))))
     return NULL;

   obr->x = x;
   obr->y = y;
   obr->w = w;
   obr->h = h;
   if (cx) *cx = 0;
   if (cy) *cy = 0;
   if (cw) *cw = w;
   if (ch) *ch = h;

   use_shm = buf->priv.x11.xcb.shm;
   alpha = ((buf->priv.x11.xcb.mask) || (buf->priv.destination_alpha));
   if ((buf->rot == 0) && (buf->priv.mask.r == 0xff0000) && 
       (buf->priv.mask.g == 0x00ff00) && (buf->priv.mask.b == 0x0000ff)) 
     {
        obr->xcbob = 
          _find_xcbob(buf->priv.x11.xcb.conn, buf->priv.x11.xcb.visual, 
                      buf->priv.x11.xcb.depth, w, h, use_shm, NULL);
        if (!obr->xcbob) 
          {
             free(obr);
             return NULL;
          }
        im = 
          (RGBA_Image *)evas_cache_image_data(evas_common_image_cache_get(), 
                                              w, h, 
                                              (DATA32 *)evas_software_xcb_output_buffer_data(obr->xcbob, &bpl), 
                                              alpha, EVAS_COLORSPACE_ARGB8888);
        if (!im) 
          {
             _unfind_xcbob(obr->xcbob, EINA_FALSE);
             free(obr);
             return NULL;
          }
        im->extended_info = obr;
        if (buf->priv.x11.xcb.mask) 
          {
             obr->mask = 
               _find_xcbob(buf->priv.x11.xcb.conn, buf->priv.x11.xcb.visual, 
                           1, w, h, use_shm, NULL);
          }
     }
   else 
     {
        int bw = 0, bh = 0;

        im = 
          (RGBA_Image *)evas_cache_image_empty(evas_common_image_cache_get());
        if (!im) 
          {
             free(obr);
             return NULL;
          }
        im->cache_entry.flags.alpha |= (alpha ? 1 : 0);
        evas_cache_image_surface_alloc(&im->cache_entry, w, h);
        im->extended_info = obr;
        if ((buf->rot == 0) || (buf->rot == 180)) 
          {
             bw = w;
             bh = h;
          }
        else if ((buf->rot == 90) || (buf->rot == 270)) 
          {
             bw = h;
             bh = w;
          }
        obr->xcbob = 
          _find_xcbob(buf->priv.x11.xcb.conn, buf->priv.x11.xcb.visual, 
                      buf->priv.x11.xcb.depth, bw, bh, use_shm, NULL);
        if (!obr->xcbob) 
          {
             evas_cache_image_drop(&im->cache_entry);
             free(obr);
             return NULL;
          }
        if (buf->priv.x11.xcb.mask) 
          {
             obr->mask = 
               _find_xcbob(buf->priv.x11.xcb.conn, buf->priv.x11.xcb.visual, 1, 
                           bw, bh, use_shm, NULL);
          }
     }
   /* FIXME: We should be able to remove this memset. */
   if (((buf->priv.x11.xcb.mask) || (buf->priv.destination_alpha)) && 
       (im->image.data)) 
     {
        /* FIXME: Faster memset */
//        memset(im->image.data, 0, (w * h * sizeof(DATA32)));
     }

#ifdef EVAS_FRAME_QUEUING
   if (!evas_common_frameq_enabled())
#endif
     buf->priv.pending_writes = eina_list_append(buf->priv.pending_writes, im);

   return im;
}

void 
evas_software_xcb_outbuf_free_region_for_update(Outbuf *buf __UNUSED__, RGBA_Image *update __UNUSED__) 
{
   /* NOOP: Cleaned up on flush */
}

void 
evas_software_xcb_outbuf_flush(Outbuf *buf) 
{
   Eina_List *l = NULL;
   RGBA_Image *im = NULL;
   Outbuf_Region *obr = NULL;

   if ((buf->priv.onebuf) && (buf->priv.onebuf_regions)) 
     {
        pixman_region16_t tmpr;

        im = buf->priv.onebuf;
        obr = im->extended_info;
        pixman_region_init(&tmpr);
        while (buf->priv.onebuf_regions) 
          {
             Eina_Rectangle *rect, xr = { 0, 0, 0, 0 };

             rect = buf->priv.onebuf_regions->data;
             buf->priv.onebuf_regions = 
               eina_list_remove_list(buf->priv.onebuf_regions, 
                                     buf->priv.onebuf_regions);
             if (buf->rot == 0)
               {
                  xr.x = rect->x;
                  xr.y = rect->y;
                  xr.w = rect->w;
                  xr.h = rect->h;
               }
             else if (buf->rot == 90)
               {
                  xr.x = rect->y;
                  xr.y = buf->w - rect->x - rect->w;
                  xr.w = rect->h;
                  xr.h = rect->w;
               }
             else if (buf->rot == 180)
               {
                  xr.x = buf->w - rect->x - rect->w;
                  xr.y = buf->h - rect->y - rect->h;
                  xr.w = rect->w;
                  xr.h = rect->h;
               }
             else if (buf->rot == 270)
               {
                  xr.x = buf->h - rect->y - rect->h;
                  xr.y = rect->x;
                  xr.w = rect->h;
                  xr.h = rect->w;
               }
             pixman_region_union_rect(&tmpr, &tmpr, xr.x, xr.y, xr.w, xr.h);
             if (buf->priv.debug)
               evas_software_xcb_outbuf_debug_show(buf, buf->priv.x11.xcb.win, 
                                                   xr.x, xr.y, xr.w, xr.h);
             eina_rectangle_free(rect);
          }
        xcb_set_clip_rectangles(buf->priv.x11.xcb.conn, 
                                XCB_CLIP_ORDERING_YX_BANDED, 
                                buf->priv.x11.xcb.gc, 0, 0, 
                                pixman_region_n_rects(&tmpr), 
                                (const xcb_rectangle_t *)pixman_region_rectangles(&tmpr, NULL));
        if (obr->xcbob)
          evas_software_xcb_output_buffer_paste(obr->xcbob, 
                                                buf->priv.x11.xcb.win, 
                                                buf->priv.x11.xcb.gc, 0, 0, 0);
        if (obr->mask) 
          {
             xcb_set_clip_rectangles(buf->priv.x11.xcb.conn, 
                                     XCB_CLIP_ORDERING_YX_BANDED, 
                                     buf->priv.x11.xcb.gcm, 0, 0, 
                                     pixman_region_n_rects(&tmpr), 
                                     (const xcb_rectangle_t *)pixman_region_rectangles(&tmpr, NULL));
             evas_software_xcb_output_buffer_paste(obr->mask, 
                                                   buf->priv.x11.xcb.mask, 
                                                   buf->priv.x11.xcb.gcm, 
                                                   0, 0, 0);
          }
        pixman_region_fini(&tmpr);
        buf->priv.synced = EINA_FALSE;
     }
   else 
     {
#if 1
        _xcbob_sync(buf->priv.x11.xcb.conn);
        EINA_LIST_FOREACH(buf->priv.pending_writes, l, im) 
          {
             obr = im->extended_info;
             if (buf->priv.debug)
               evas_software_xcb_outbuf_debug_show(buf, buf->priv.x11.xcb.win, 
                                                   obr->x, obr->y, obr->w, obr->h);
             if (obr->xcbob)
               evas_software_xcb_output_buffer_paste(obr->xcbob, 
                                                     buf->priv.x11.xcb.win, 
                                                     buf->priv.x11.xcb.gc, 
                                                     obr->x, obr->y, 0);
             if (obr->mask)
               evas_software_xcb_output_buffer_paste(obr->mask, 
                                                     buf->priv.x11.xcb.mask, 
                                                     buf->priv.x11.xcb.gcm, 
                                                     obr->x, obr->y, 0);
          }
# ifdef EVAS_FRAME_QUEUING
        LKL(buf->priv.lock);
# endif
        while (buf->priv.prev_pending_writes) 
          {
             im = buf->priv.prev_pending_writes->data;
             buf->priv.prev_pending_writes = 
               eina_list_remove_list(buf->priv.prev_pending_writes, 
                                     buf->priv.prev_pending_writes);
             obr = im->extended_info;
             evas_cache_image_drop(&im->cache_entry);
             if (obr->xcbob) _unfind_xcbob(obr->xcbob, EINA_FALSE);
             if (obr->mask) _unfind_xcbob(obr->mask, EINA_FALSE);
             free(obr);
          }
        buf->priv.prev_pending_writes = buf->priv.pending_writes;
# ifdef EVAS_FRAME_QUEUING
        LKU(buf->priv.lock);
# endif
        buf->priv.pending_writes = NULL;
        xcb_flush(buf->priv.x11.xcb.conn);
#else
        /* FIXME: Async Push Disabled */

        _xcbob_sync(buf->priv.x11.xcb.conn);

        while (buf->priv.pending_writes) 
          {
             im = eina_list_data_get(buf->priv.pending_writes);
             buf->priv.pending_writes = 
               eina_list_remove_list(buf->priv.pending_writes, 
                                     buf->priv.pending_writes);
             obr = im->extended_info;
             evas_cache_image_drop(&im->cache_entry);
             if (obr->xcbob) _unfind_xcbob(obr->xcbob, EINA_FALSE);
             if (obr->mask) _unfind_xcbob(obr->mask, EINA_FALSE);
             free(obr);
//             evas_cache_image_drop(&im->cache_entry);
          }
#endif
     }
   evas_common_cpu_end_opt();
}

void 
evas_software_xcb_outbuf_idle_flush(Outbuf *buf) 
{
   if (buf->priv.onebuf) 
     {
        RGBA_Image *im;
        Outbuf_Region *obr;

        im = buf->priv.onebuf;
        buf->priv.onebuf = NULL;
        obr = im->extended_info;
        evas_cache_image_drop(&im->cache_entry);
        if (obr->xcbob) 
          evas_software_xcb_output_buffer_free(obr->xcbob, EINA_FALSE);
        if (obr->mask) 
          evas_software_xcb_output_buffer_free(obr->mask, EINA_FALSE);
        free(obr);
     }
   else 
     {
#ifdef EVAS_FRAME_QUEUING
        LKL(buf->priv.lock);
#endif
        if (buf->priv.prev_pending_writes)
          _xcbob_sync(buf->priv.x11.xcb.conn);
        while (buf->priv.prev_pending_writes) 
          {
             RGBA_Image *im;
             Outbuf_Region *obr;

             im = buf->priv.prev_pending_writes->data;
             buf->priv.prev_pending_writes = 
               eina_list_remove_list(buf->priv.prev_pending_writes, 
                                     buf->priv.prev_pending_writes);
             obr = im->extended_info;
             evas_cache_image_drop(&im->cache_entry);
             if (obr->xcbob) _unfind_xcbob(obr->xcbob, EINA_FALSE);
             if (obr->mask) _unfind_xcbob(obr->mask, EINA_FALSE);
             free(obr);
          }
#ifdef EVAS_FRAME_QUEUING
        LKU(buf->priv.lock);
#endif
        _clear_xcbob(EINA_FALSE);
     }
}

void 
evas_software_xcb_outbuf_push_updated_region(Outbuf *buf, RGBA_Image *update, int x, int y, int w, int h) 
{
   Gfx_Func_Convert func_conv = NULL;
   Outbuf_Region *obr = NULL;
   DATA32 *src_data = NULL;
   unsigned char *data = NULL;
   int bpl = 0, yy = 0;
   int bw = 0, bh = 0;
   int bpp = 0;

   obr = update->extended_info;
   if (!obr->xcbob) return;

   if (obr->xcbob->xim)
     bpp = obr->xcbob->xim->bpp;
   else 
     {
        const xcb_setup_t *setup;
        xcb_format_t *fmt;

        setup = xcb_get_setup(buf->priv.x11.xcb.conn);
        fmt = _find_format_by_depth(setup, buf->priv.x11.xcb.depth);
        bpp = fmt->bits_per_pixel;
     }

   if ((buf->rot == 0) || (buf->rot == 180)) 
     {
        bw = w;
        bh = h;
     }
   else if ((buf->rot == 90) || (buf->rot == 270)) 
     {
        bw = h;
        bh = w;
     }
   if (buf->priv.pal) 
     {
        func_conv = 
          evas_common_convert_func_get(0, bw, bh, bpp, buf->priv.mask.r, 
                                       buf->priv.mask.g, buf->priv.mask.b, 
                                       buf->priv.pal->colors, buf->rot);
     }
   else 
     {
        func_conv = 
          evas_common_convert_func_get(0, bw, bh, bpp, buf->priv.mask.r, 
                                       buf->priv.mask.g, buf->priv.mask.b, 
                                       PAL_MODE_NONE, buf->rot);
     }
   if (!func_conv) return;
   if (!(data = evas_software_xcb_output_buffer_data(obr->xcbob, &bpl)))
     return;
   if (!(src_data = update->image.data)) return;
   if (buf->rot == 0) 
     {
        obr->x = x;
        obr->y = y;
        obr->w = w;
        obr->h = h;
     }
   else if (buf->rot == 90) 
     {
        obr->x = y;
        obr->y = (buf->w - x - w);
        obr->w = h;
        obr->h = w;
     }
   else if (buf->rot == 180) 
     {
        obr->x = (buf->w - x - w);
        obr->y = (buf->h - y - h);
        obr->w = w;
        obr->h = h;
     }
   else if (buf->rot == 270) 
     {
        obr->x = (buf->h - y - h);
        obr->y = x;
        obr->w = h;
        obr->h = w;
     }
   if (buf->onebuf)
     {
        src_data += x + (y * update->cache_entry.w);
        data += (bpl * obr->y) +
           (obr->x * (evas_software_xcb_output_buffer_depth(obr->xcbob) / 8));
     }
   if (data != (unsigned char *)src_data) 
     {
        if (buf->priv.pal)
          func_conv(src_data, data, update->cache_entry.w - w, 
                    (bpl / (bpp / 8)) - obr->w, 
                    obr->w, obr->h, x, y, buf->priv.pal->lookup);
        else
          func_conv(src_data, data, update->cache_entry.w - w, 
                    (bpl / (bpp / 8)) - obr->w, 
                    obr->w, obr->h, x, y, NULL);
     }
#if 1
#else
   /* Async Push */
   if (!((buf->priv.onebuf) && (buf->priv.onebuf_regions))) 
     {
        if (buf->priv.debug)
          evas_software_xcb_outbuf_debug_show(buf, buf->priv.x11.xcb.win, 
                                              obr->x, obr->y, obr->w, obr->h);
        if (obr->xcbob)
          evas_software_xcb_output_buffer_paste(obr->xcbob, 
                                                buf->priv.x11.xcb.win, 
                                                buf->priv.x11.xcb.gc, 
                                                obr->x, obr->y, 0);
     }
#endif
   if (obr->mask) 
     {
        if (buf->rot == 0) 
          {
             for (yy = 0; yy < obr->h; yy++)
               evas_software_xcb_write_mask_line(buf, obr->mask, 
                                                 src_data + (yy * obr->w), 
                                                 obr->w, yy);
          }
        else if (buf->rot == 90) 
          {
             for (yy = 0; yy < obr->h; yy++)
               evas_software_xcb_write_mask_line_vert(buf, obr->mask, 
                                                      src_data + yy, 
                                                      h, (obr->h - yy - 1), w);
          }
        else if (buf->rot == 180) 
          {
             for (yy = 0; yy < obr->h; yy++)
               evas_software_xcb_write_mask_line_rev(buf, obr->mask, 
                                                     src_data + (yy * obr->w), 
                                                     obr->w, (obr->h - yy - 1));
          }
        else if (buf->rot == 270) 
          {
             for (yy = 0; yy < obr->h; yy++)
               evas_software_xcb_write_mask_line_vert_rev(buf, obr->mask, 
                                                          src_data + yy, 
                                                          h, yy, w);
          }
#if 1
#else
        /* Async Push */
        if (!((buf->priv.onebuf) && (buf->priv.onebuf_regions))) 
          evas_software_xcb_output_buffer_paste(obr->xcbob, 
                                                buf->priv.x11.xcb.mask, 
                                                buf->priv.x11.xcb.gcm, 
                                                obr->x, obr->y, 0);
#endif
     }
#if 1
#else
   xcb_flush(buf->priv.x11.xcb.conn);
#endif
}

void 
evas_software_xcb_outbuf_reconfigure(Outbuf *buf, int w, int h, int rot, Outbuf_Depth depth) 
{
   if ((w == buf->w) && (h == buf->h) && (rot == buf->rot) && 
       (depth == buf->depth)) return;
   buf->w = w;
   buf->h = h;
   buf->rot = rot;
   evas_software_xcb_outbuf_idle_flush(buf);
}

int 
evas_software_xcb_outbuf_width_get(Outbuf *buf) 
{
   return buf->w;
}

int 
evas_software_xcb_outbuf_height_get(Outbuf *buf) 
{
   return buf->h;
}

Outbuf_Depth 
evas_software_xcb_outbuf_depth_get(Outbuf *buf) 
{
   return buf->depth;
}

void 
evas_software_xcb_outbuf_drawable_set(Outbuf *buf, xcb_drawable_t drawable) 
{
   if (buf->priv.x11.xcb.win == drawable) return;
   if (buf->priv.x11.xcb.gc) 
     {
        xcb_free_gc(buf->priv.x11.xcb.conn, buf->priv.x11.xcb.gc);
        buf->priv.x11.xcb.gc = 0;
     }
   buf->priv.x11.xcb.win = drawable;
   buf->priv.x11.xcb.gc = xcb_generate_id(buf->priv.x11.xcb.conn);
   xcb_create_gc(buf->priv.x11.xcb.conn, 
                 buf->priv.x11.xcb.gc, buf->priv.x11.xcb.win, 0, NULL);
}

void 
evas_software_xcb_outbuf_mask_set(Outbuf *buf, xcb_drawable_t mask) 
{
   if (buf->priv.x11.xcb.mask == mask) return;
   if (buf->priv.x11.xcb.gcm) 
     {
        xcb_free_gc(buf->priv.x11.xcb.conn, buf->priv.x11.xcb.gcm);
        buf->priv.x11.xcb.gcm = 0;
     }
   buf->priv.x11.xcb.mask = mask;
   if (buf->priv.x11.xcb.mask) 
     {
        buf->priv.x11.xcb.gcm = xcb_generate_id(buf->priv.x11.xcb.conn);
        xcb_create_gc(buf->priv.x11.xcb.conn, 
                      buf->priv.x11.xcb.gcm, buf->priv.x11.xcb.win, 0, NULL);
     }
}

int 
evas_software_xcb_outbuf_rotation_get(Outbuf *buf) 
{
   return buf->rot;
}

void 
evas_software_xcb_outbuf_rotation_set(Outbuf *buf, int rotation) 
{
   buf->rot = rotation;
}

Eina_Bool 
evas_software_xcb_outbuf_alpha_get(Outbuf *buf) 
{
   return buf->priv.x11.xcb.mask;
}

void 
evas_software_xcb_outbuf_debug_set(Outbuf *buf, Eina_Bool debug) 
{
   buf->priv.debug = debug;
}

void 
evas_software_xcb_outbuf_debug_show(Outbuf *buf, xcb_drawable_t drawable, int x, int y, int w, int h) 
{
   int i;
   xcb_screen_t *screen = NULL;
   xcb_get_geometry_reply_t *geom;
   xcb_drawable_t root;
   xcb_screen_iterator_t si;

   geom = 
     xcb_get_geometry_reply(buf->priv.x11.xcb.conn, 
                            xcb_get_geometry_unchecked(buf->priv.x11.xcb.conn, 
                                                       drawable), 0);
   root = geom->root;
   free(geom);
   geom = 
     xcb_get_geometry_reply(buf->priv.x11.xcb.conn, 
                            xcb_get_geometry_unchecked(buf->priv.x11.xcb.conn, 
                                                       root), 0);

   si = xcb_setup_roots_iterator((xcb_setup_t *)xcb_get_setup(buf->priv.x11.xcb.conn));
   for (; si.rem; xcb_screen_next(&si))
     {
        if (si.data->root == geom->root)
          {
             screen = si.data;
             break;
          }
     }
   free(geom);

   for (i = 0; i < 20; i++)
     {
	xcb_rectangle_t rect = { x, y, w, h};
	uint32_t mask;
	uint32_t value[2];

	mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
	value[0] = screen->black_pixel;
	value[1] = XCB_EXPOSURES_NOT_ALLOWED;
	xcb_change_gc(buf->priv.x11.xcb.conn, buf->priv.x11.xcb.gc, 
                      mask, value);
	xcb_poly_fill_rectangle(buf->priv.x11.xcb.conn, drawable, 
                                buf->priv.x11.xcb.gc, 1, &rect);
        _xcbob_sync(buf->priv.x11.xcb.conn);
        _xcbob_sync(buf->priv.x11.xcb.conn);

	mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
	value[0] = screen->white_pixel;
	value[1] = XCB_EXPOSURES_NOT_ALLOWED;
	xcb_change_gc(buf->priv.x11.xcb.conn, buf->priv.x11.xcb.gc, 
                      mask, value);
	xcb_poly_fill_rectangle(buf->priv.x11.xcb.conn, drawable, 
                                buf->priv.x11.xcb.gc, 1, &rect);
        _xcbob_sync(buf->priv.x11.xcb.conn);
        _xcbob_sync(buf->priv.x11.xcb.conn);
     }
}

#ifdef EVAS_FRAME_QUEUING
void 
evas_software_xcb_outbuf_priv_set(Outbuf *buf, void *cur, void *prev __UNUSED__) 
{
   buf->priv.pending_writes = (Eina_List *)cur;
}
#endif

/* local functions */
static Xcb_Output_Buffer *
_find_xcbob(xcb_connection_t *conn, xcb_visualtype_t *vis, int depth, int w, int h, Eina_Bool shm, void *data) 
{
   Eina_List *l = NULL, *xl = NULL;
   Xcb_Output_Buffer *xcbob = NULL, *xcbob2 = NULL;
   int lbytes = 0, bpp = 0, sz = 0;
   int fitness = 0x7fffffff;

   if (!shm)
     return evas_software_xcb_output_buffer_new(conn, vis, depth, w, h, 
                                                shm, data);

   lbytes = (((w + 31) / 32) * 4);
   if (depth > 1) 
     {
        bpp = (depth / 8);
        if (bpp == 3) bpp = 4;
        lbytes = ((((w * bpp) + 3) / 4) * 4);
     }

   sz = (lbytes * h);
   SHMPOOL_LOCK();
   EINA_LIST_FOREACH(_shmpool, l, xcbob2) 
     {
        int szdif = 0;

        if ((xcbob2->xim->depth != depth) || (xcbob2->visual != vis) || 
            (xcbob2->connection != conn)) continue;
        szdif = (xcbob2->psize - sz);
        if (szdif < 0) continue;
        if (szdif == 0) 
          {
             xcbob = xcbob2;
             xl = l;
             goto have_xcbob;
          }
        if (szdif < fitness) 
          {
             xcbob = xcbob2;
             xl = l;
             fitness = szdif;
          }
     }
   if ((fitness > (100 * 100)) || (!xcbob)) 
     {
        SHMPOOL_UNLOCK();
        return evas_software_xcb_output_buffer_new(conn, vis, depth, 
                                                   w, h, shm, data);
     }

have_xcbob:
   _shmpool = eina_list_remove_list(_shmpool, xl);
   xcbob->w = w;
   xcbob->h = h;
   xcbob->bpl = lbytes;
   xcbob->xim->width = xcbob->w;
   xcbob->xim->height = xcbob->h;
   xcbob->xim->stride = xcbob->bpl;
   _shmsize -= (xcbob->psize * (xcbob->xim->depth / 8));
   SHMPOOL_UNLOCK();
   return xcbob;
}

static void 
_unfind_xcbob(Xcb_Output_Buffer *xcbob, Eina_Bool sync) 
{
   if (xcbob->shm_info) 
     {
        SHMPOOL_LOCK();
        _shmpool = eina_list_prepend(_shmpool, xcbob);
        _shmsize += xcbob->psize * xcbob->xim->depth / 8;
        while ((_shmsize > _shmlimit) || 
               ((int)eina_list_count(_shmpool) > _shmcountlimit))
          {
             Eina_List *xl = NULL;

             if (!(xl = eina_list_last(_shmpool))) 
               {
                  _shmsize = 0;
                  break;
               }
             xcbob = xl->data;
             _shmpool = eina_list_remove_list(_shmpool, xl);
             _shmsize -= xcbob->psize * xcbob->xim->depth / 8;
             evas_software_xcb_output_buffer_free(xcbob, sync);
          }
        SHMPOOL_UNLOCK();
     }
   else
     evas_software_xcb_output_buffer_free(xcbob, sync);
}

static void 
_clear_xcbob(Eina_Bool sync) 
{
   SHMPOOL_LOCK();
   while (_shmpool) 
     {
        Xcb_Output_Buffer *xcbob;
 
        xcbob = _shmpool->data;
        _shmpool = eina_list_remove_list(_shmpool, _shmpool);
        evas_software_xcb_output_buffer_free(xcbob, sync);
     }
   _shmsize = 0;
   SHMPOOL_UNLOCK();
}

static xcb_format_t *
_find_format_by_depth(const xcb_setup_t *setup, uint8_t depth) 
{
   xcb_format_t *fmt, *fmt_end;

   fmt = xcb_setup_pixmap_formats(setup);
   fmt_end = fmt + xcb_setup_pixmap_formats_length(setup);

   for (; fmt != fmt_end; ++fmt)
     if (fmt->depth == depth)
       return fmt;

   return 0;
}

static void 
_xcbob_sync(xcb_connection_t *conn) 
{
   free(xcb_get_input_focus_reply(conn, 
                                  xcb_get_input_focus_unchecked(conn), NULL));
}
