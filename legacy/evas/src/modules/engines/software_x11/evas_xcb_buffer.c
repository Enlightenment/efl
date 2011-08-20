#include "evas_common.h"
#include "evas_xcb_buffer.h"

/* local function prototypes */
static void _xcbob_sync(xcb_connection_t *conn);
static xcb_image_t *_xcbob_create_native(xcb_connection_t *conn, int w, int h, xcb_image_format_t format, uint8_t depth, void *base, uint32_t bytes, uint8_t *data);
static xcb_format_t *_xcbob_find_format(const xcb_setup_t *setup, uint8_t depth);
static xcb_visualtype_t *_xcbob_find_visual_by_id(xcb_screen_t *screen, xcb_visualid_t id);

void 
evas_software_xcb_write_mask_line(Outbuf *buf, Xcb_Output_Buffer *xcbob, DATA32 *src, int w, int y) 
{
   int x, bpl = 0;
   DATA32 *src_ptr;
   DATA8 *dst_ptr;

   src_ptr = src;
   dst_ptr = evas_software_xcb_output_buffer_data(xcbob, &bpl);
   dst_ptr = dst_ptr + (bpl * y);
   w -= 7;
   if (buf->priv.x11.xcb.bit_swap)
     {
	for (x = 0; x < w; x += 8)
	  {
	     *dst_ptr =
	       ((A_VAL(&(src_ptr[0])) >> 7) << 7) |
	       ((A_VAL(&(src_ptr[1])) >> 7) << 6) |
	       ((A_VAL(&(src_ptr[2])) >> 7) << 5) |
	       ((A_VAL(&(src_ptr[3])) >> 7) << 4) |
	       ((A_VAL(&(src_ptr[4])) >> 7) << 3) |
	       ((A_VAL(&(src_ptr[5])) >> 7) << 2) |
	       ((A_VAL(&(src_ptr[6])) >> 7) << 1) |
	       ((A_VAL(&(src_ptr[7])) >> 7) << 0);
	     src_ptr += 8;
	     dst_ptr++;
	  }
     }
   else
     {
	for (x = 0; x < w; x += 8)
	  {
	     *dst_ptr =
	       ((A_VAL(&(src_ptr[0])) >> 7) << 0) |
	       ((A_VAL(&(src_ptr[1])) >> 7) << 1) |
	       ((A_VAL(&(src_ptr[2])) >> 7) << 2) |
	       ((A_VAL(&(src_ptr[3])) >> 7) << 3) |
	       ((A_VAL(&(src_ptr[4])) >> 7) << 4) |
	       ((A_VAL(&(src_ptr[5])) >> 7) << 5) |
	       ((A_VAL(&(src_ptr[6])) >> 7) << 6) |
	       ((A_VAL(&(src_ptr[7])) >> 7) << 7);
	     src_ptr += 8;
	     dst_ptr++;
	  }
     }
   w += 7;
   for (; x < w; x ++)
     {
        xcb_image_put_pixel(xcbob->xim, x, y, A_VAL(src_ptr) >> 7);
        src_ptr++;
     }
}

void 
evas_software_xcb_write_mask_line_rev(Outbuf *buf, Xcb_Output_Buffer *xcbob, DATA32 *src, int w, int y) 
{
   int x, bpl = 0;
   DATA32 *src_ptr;
   DATA8 *dst_ptr;

   src_ptr = src + w - 1;
   dst_ptr = evas_software_xcb_output_buffer_data(xcbob, &bpl);
   dst_ptr = dst_ptr + (bpl * y);
   w -= 7;
   if (buf->priv.x11.xcb.bit_swap)
     {
	for (x = 0; x < w; x += 8)
	  {
	     *dst_ptr =
	       ((A_VAL(&(src_ptr[ 0])) >> 7) << 7) |
	       ((A_VAL(&(src_ptr[-1])) >> 7) << 6) |
	       ((A_VAL(&(src_ptr[-2])) >> 7) << 5) |
	       ((A_VAL(&(src_ptr[-3])) >> 7) << 4) |
	       ((A_VAL(&(src_ptr[-4])) >> 7) << 3) |
	       ((A_VAL(&(src_ptr[-5])) >> 7) << 2) |
	       ((A_VAL(&(src_ptr[-6])) >> 7) << 1) |
	       ((A_VAL(&(src_ptr[-7])) >> 7) << 0);
	     src_ptr -= 8;
	     dst_ptr++;
	  }
     }
   else
     {
	for (x = 0; x < w; x += 8)
	  {
	     *dst_ptr =
	       ((A_VAL(&(src_ptr[ 0])) >> 7) << 0) |
	       ((A_VAL(&(src_ptr[-1])) >> 7) << 1) |
	       ((A_VAL(&(src_ptr[-2])) >> 7) << 2) |
	       ((A_VAL(&(src_ptr[-3])) >> 7) << 3) |
	       ((A_VAL(&(src_ptr[-4])) >> 7) << 4) |
	       ((A_VAL(&(src_ptr[-5])) >> 7) << 5) |
	       ((A_VAL(&(src_ptr[-6])) >> 7) << 6) |
	       ((A_VAL(&(src_ptr[-7])) >> 7) << 7);
	     src_ptr -= 8;
	     dst_ptr++;
	  }
     }
   w += 7;
   for (; x < w; x ++)
     {
	xcb_image_put_pixel(xcbob->xim, x, y, A_VAL(src_ptr) >> 7);
	src_ptr--;
     }
}

void 
evas_software_xcb_write_mask_line_vert(Outbuf *buf, Xcb_Output_Buffer *xcbob, DATA32 *src, int h, int y, int w) 
{
   int yy, bpl = 0;
   DATA32 *src_ptr;
   DATA8 *dst_ptr;

   src_ptr = src;
   dst_ptr = evas_software_xcb_output_buffer_data(xcbob, &bpl);
   dst_ptr = dst_ptr + (bpl * y);
   h -= 7;
   if (buf->priv.x11.xcb.bit_swap)
     {
	for (yy = 0; yy < h; yy += 8)
	  {
	     *dst_ptr =
	       ((A_VAL(&(src_ptr[0 * w])) >> 7) << 7) |
	       ((A_VAL(&(src_ptr[1 * w])) >> 7) << 6) |
	       ((A_VAL(&(src_ptr[2 * w])) >> 7) << 5) |
	       ((A_VAL(&(src_ptr[3 * w])) >> 7) << 4) |
	       ((A_VAL(&(src_ptr[4 * w])) >> 7) << 3) |
	       ((A_VAL(&(src_ptr[5 * w])) >> 7) << 2) |
	       ((A_VAL(&(src_ptr[6 * w])) >> 7) << 1) |
	       ((A_VAL(&(src_ptr[7 * w])) >> 7) << 0);
	     src_ptr += 8 * w;
	     dst_ptr++;
	  }
     }
   else
     {
	for (yy = 0; yy < h; yy += 8)
	  {
	     *dst_ptr =
	       ((A_VAL(&(src_ptr[0 * w])) >> 7) << 0) |
	       ((A_VAL(&(src_ptr[1 * w])) >> 7) << 1) |
	       ((A_VAL(&(src_ptr[2 * w])) >> 7) << 2) |
	       ((A_VAL(&(src_ptr[3 * w])) >> 7) << 3) |
	       ((A_VAL(&(src_ptr[4 * w])) >> 7) << 4) |
	       ((A_VAL(&(src_ptr[5 * w])) >> 7) << 5) |
	       ((A_VAL(&(src_ptr[6 * w])) >> 7) << 6) |
	       ((A_VAL(&(src_ptr[7 * w])) >> 7) << 7);
	     src_ptr += 8 * w;
	     dst_ptr++;
	  }
     }
   h += 7;
   for (; yy < h; yy ++)
     {
	xcb_image_put_pixel(xcbob->xim, yy, y, A_VAL(src_ptr) >> 7);
	src_ptr += w;
     }
}

void 
evas_software_xcb_write_mask_line_vert_rev(Outbuf *buf, Xcb_Output_Buffer *xcbob, DATA32 *src, int h, int y, int w) 
{
   int yy, bpl = 0;
   DATA32 *src_ptr;
   DATA8 *dst_ptr;

   src_ptr = src + ((h - 1) * w);
   dst_ptr = evas_software_xcb_output_buffer_data(xcbob, &bpl);
   dst_ptr = dst_ptr + (bpl * y);
   h -= 7;
   if (buf->priv.x11.xcb.bit_swap)
     {
	for (yy = 0; yy < h; yy += 8)
	  {
	     *dst_ptr =
	       ((A_VAL(&(src_ptr[ 0 * w])) >> 7) << 7) |
	       ((A_VAL(&(src_ptr[-1 * w])) >> 7) << 6) |
	       ((A_VAL(&(src_ptr[-2 * w])) >> 7) << 5) |
	       ((A_VAL(&(src_ptr[-3 * w])) >> 7) << 4) |
	       ((A_VAL(&(src_ptr[-4 * w])) >> 7) << 3) |
	       ((A_VAL(&(src_ptr[-5 * w])) >> 7) << 2) |
	       ((A_VAL(&(src_ptr[-6 * w])) >> 7) << 1) |
	       ((A_VAL(&(src_ptr[-7 * w])) >> 7) << 0);
	     src_ptr -= 8 * w;
	     dst_ptr++;
	  }
     }
   else
     {
	for (yy = 0; yy < h; yy += 8)
	  {
	     *dst_ptr =
	       ((A_VAL(&(src_ptr[ 0 * w])) >> 7) << 0) |
	       ((A_VAL(&(src_ptr[-1 * w])) >> 7) << 1) |
	       ((A_VAL(&(src_ptr[-2 * w])) >> 7) << 2) |
	       ((A_VAL(&(src_ptr[-3 * w])) >> 7) << 3) |
	       ((A_VAL(&(src_ptr[-4 * w])) >> 7) << 4) |
	       ((A_VAL(&(src_ptr[-5 * w])) >> 7) << 5) |
	       ((A_VAL(&(src_ptr[-6 * w])) >> 7) << 6) |
	       ((A_VAL(&(src_ptr[-7 * w])) >> 7) << 7);
	     src_ptr -= 8 * w;
	     dst_ptr++;
	  }
     }
   h += 7;
   for (; yy < h; yy ++)
     {
	xcb_image_put_pixel(xcbob->xim, yy, y, A_VAL(src_ptr) >> 7);
	src_ptr -= w;
     }
}

Eina_Bool 
evas_software_xcb_can_do_shm(xcb_connection_t *conn, xcb_screen_t *screen) 
{
   const xcb_query_extension_reply_t *reply;
   static xcb_connection_t *cached_conn = NULL;
   static int cached_result = 0;

   if (conn == cached_conn) return cached_result;
   cached_conn = conn;

   reply = xcb_get_extension_data(conn, &xcb_shm_id);
   if ((reply) && (reply->present)) 
     {
        xcb_visualtype_t *visual;
        Xcb_Output_Buffer *xcbob = NULL;

        visual = _xcbob_find_visual_by_id(screen, screen->root_visual);
        xcbob = 
          evas_software_xcb_output_buffer_new(conn, visual, screen->root_depth,
                                              16, 16, 2, NULL);
        if (!xcbob)
          cached_result = 0;
        else 
          {
             evas_software_xcb_output_buffer_free(xcbob, EINA_TRUE);
             cached_result = 1;
          }
     }
   else
     cached_result = 0;

   return cached_result;
}

Xcb_Output_Buffer *
evas_software_xcb_output_buffer_new(xcb_connection_t *conn, xcb_visualtype_t *vis, int depth, int w, int h, int try_shm, unsigned char *data) 
{
   Xcb_Output_Buffer *xcbob = NULL;

   if (!(xcbob = calloc(1, sizeof(Xcb_Output_Buffer))))
     return NULL;

   xcbob->connection = conn;
   xcbob->visual = vis;
   xcbob->xim = NULL;
   xcbob->shm_info = NULL;
   xcbob->w = w;
   xcbob->h = h;

   if (try_shm > 0) 
     {
        xcbob->shm_info = malloc(sizeof(xcb_shm_segment_info_t));
        if (xcbob->shm_info) 
          {
             xcbob->shm_info->shmseg = xcb_generate_id(conn);
             xcbob->xim = 
               _xcbob_create_native(conn, w, h, XCB_IMAGE_FORMAT_Z_PIXMAP, 
                                    depth, NULL, ~0, NULL);
             if (xcbob->xim) 
               {
                  xcbob->shm_info->shmid = 
                    shmget(IPC_PRIVATE, 
                           xcbob->xim->stride * xcbob->xim->height, 
                           (IPC_CREAT | 0777));
                  if (xcbob->shm_info->shmid == (uint32_t)-1) 
                    {
                       xcb_image_destroy(xcbob->xim);
                       free(xcbob->shm_info);
                       free(xcbob);
                       return NULL;
                    }
                  xcbob->shm_info->shmaddr = xcbob->xim->data = 
                    shmat(xcbob->shm_info->shmid, 0, 0);
                  if (xcbob->shm_info->shmaddr != ((void *)-1))
                    {
                       /* Sync only needed for testing */
                       if (try_shm == 2) _xcbob_sync(conn);

#if defined(EVAS_FRAME_QUEUING) && defined(LIBXEXT_VERSION_LOW)
                       if (evas_common_frameq_enabled())
                         xcb_grab_server(conn);
#endif
                       xcb_shm_attach(conn, xcbob->shm_info->shmseg, 
                                      xcbob->shm_info->shmid, 0);
#if defined(EVAS_FRAME_QUEUING) && defined(LIBXEXT_VERSION_LOW)
                       if (evas_common_frameq_enabled()) 
                         xcb_ungrab_server(conn);
#endif
                       if (try_shm == 2) _xcbob_sync(conn);

                       xcbob->bpl = xcbob->xim->stride;
                       xcbob->psize = (xcbob->bpl * xcbob->h);
                       return xcbob;
                    }
                  shmdt(xcbob->shm_info->shmaddr);
                  shmctl(xcbob->shm_info->shmid, IPC_RMID, 0);
               }
             if (xcbob->xim) xcb_image_destroy(xcbob->xim);
             xcbob->xim = NULL;
          }
        if (xcbob->shm_info) free(xcbob->shm_info);
        xcbob->shm_info = NULL;
     }

   if (try_shm > 1) return NULL;

   /* no shm */
   xcbob->xim = 
     _xcbob_create_native(conn, w, h, XCB_IMAGE_FORMAT_Z_PIXMAP, 
                          depth, NULL, ~0, NULL);
   if (!xcbob->xim) 
     {
        free(xcbob);
        return NULL;
     }

   xcbob->data = data;

   if (!xcbob->xim->data) 
     {
        xcbob->xim->data = malloc(xcbob->xim->stride * xcbob->xim->height);
        if (!xcbob->xim->data) 
          {
             xcb_image_destroy(xcbob->xim);
             free(xcbob);
             return NULL;
          }
     }
   xcbob->bpl = xcbob->xim->stride;
   xcbob->psize = (xcbob->bpl * xcbob->h);
   return xcbob;
}

void 
evas_software_xcb_output_buffer_free(Xcb_Output_Buffer *xcbob, Eina_Bool sync) 
{
   if (xcbob->shm_info) 
     {
        if (sync) _xcbob_sync(xcbob->connection);
        xcb_shm_detach(xcbob->connection, xcbob->shm_info->shmseg);
        xcb_image_destroy(xcbob->xim);
        shmdt(xcbob->shm_info->shmaddr);
        shmctl(xcbob->shm_info->shmid, IPC_RMID, 0);
        free(xcbob->shm_info);
     }
   else 
     {
        if (xcbob->data) xcbob->xim->data = NULL;
//        free(xcbob->xim->data);
        xcb_image_destroy(xcbob->xim);
     }
   free(xcbob);
}

void 
evas_software_xcb_output_buffer_paste(Xcb_Output_Buffer *xcbob, xcb_drawable_t drawable, xcb_gcontext_t gc, int x, int y, Eina_Bool sync) 
{
   if (xcbob->shm_info) 
     {
        xcb_image_shm_put(xcbob->connection, drawable, gc, xcbob->xim, 
                          *xcbob->shm_info, 0, 0, x, y, xcbob->w, xcbob->h, 0);
        if (sync) _xcbob_sync(xcbob->connection);
     }
   else 
     xcb_image_put(xcbob->connection, drawable, gc, xcbob->xim, x, y, 0);
}

DATA8 *
evas_software_xcb_output_buffer_data(Xcb_Output_Buffer *xcbob, int *bpl_ret) 
{
   if (bpl_ret) *bpl_ret = xcbob->xim->stride;
   return (DATA8 *)xcbob->xim->data;
}

int 
evas_software_xcb_output_buffer_depth(Xcb_Output_Buffer *xcbob) 
{
   return xcbob->xim->bpp;
}

int 
evas_software_xcb_output_buffer_byte_order(Xcb_Output_Buffer *xcbob) 
{
   return xcbob->xim->byte_order;
}

int 
evas_software_xcb_output_buffer_bit_order(Xcb_Output_Buffer *xcbob) 
{
   return xcbob->xim->bit_order;
}

/* local functions */
static void 
_xcbob_sync(xcb_connection_t *conn) 
{
   free(xcb_get_input_focus_reply(conn, 
                                  xcb_get_input_focus_unchecked(conn), NULL));
}

static xcb_image_t *
_xcbob_create_native(xcb_connection_t *conn, int w, int h, xcb_image_format_t format, uint8_t depth, void *base, uint32_t bytes, uint8_t *data) 
{
   static uint8_t dpth = 0;
   static xcb_format_t *fmt = NULL;
   const xcb_setup_t *setup;
   xcb_image_format_t xif;

   /* NB: We cannot use xcb_image_create_native as it only creates images 
    * using MSB_FIRST, so this routine recreates that function and uses 
    * the endian-ness of the server setup */
   setup = xcb_get_setup(conn);
   xif = format;

   if ((xif == XCB_IMAGE_FORMAT_Z_PIXMAP) && (depth == 1))
     xif = XCB_IMAGE_FORMAT_XY_PIXMAP;

   if (dpth != depth) 
     {
        dpth = depth;
        fmt = _xcbob_find_format(setup, depth);
        if (!fmt) return 0;
     }

   switch (xif) 
     {
      case XCB_IMAGE_FORMAT_XY_BITMAP:
        if (depth != 1) return 0;
      case XCB_IMAGE_FORMAT_XY_PIXMAP:
      case XCB_IMAGE_FORMAT_Z_PIXMAP:
        return xcb_image_create(w, h, xif, 
                                fmt->scanline_pad, 
                                fmt->depth, fmt->bits_per_pixel, 
                                setup->bitmap_format_scanline_unit, 
                                setup->image_byte_order, 
                                setup->bitmap_format_bit_order, 
                                base, bytes, data);
      default:
        break;
     }

   return 0;
}

static xcb_format_t *
_xcbob_find_format(const xcb_setup_t *setup, uint8_t depth) 
{
   xcb_format_t *fmt, *fmtend;

   fmt = xcb_setup_pixmap_formats(setup);
   fmtend = fmt + xcb_setup_pixmap_formats_length(setup);
   for (; fmt != fmtend; ++fmt)
     if (fmt->depth == depth) 
       return fmt;

   return 0;
}

static xcb_visualtype_t *
_xcbob_find_visual_by_id(xcb_screen_t *screen, xcb_visualid_t id) 
{
   xcb_depth_iterator_t diter;
   xcb_visualtype_iterator_t viter;

   diter = xcb_screen_allowed_depths_iterator(screen);
   for (; diter.rem; xcb_depth_next(&diter)) 
     {
        viter = xcb_depth_visuals_iterator(diter.data);
        for (; viter.rem; xcb_visualtype_next(&viter)) 
          {
             if (viter.data->visual_id == id)
               return viter.data;
          }
     }

   return 0;
}
