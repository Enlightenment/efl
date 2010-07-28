#include "evas_common.h"
#include "evas_engine.h"

static int _xcb_err = 0;

int
evas_software_x11_x_can_do_shm(xcb_connection_t * c, xcb_screen_t * screen)
{
   static xcb_connection_t *cached_c = NULL;
   static int cached_result = 0;

   if (c == cached_c)
      return cached_result;
   cached_c = c;

   if (xcb_get_extension_data(c, &xcb_shm_id))
     {
        X_Output_Buffer *xob;

        xob = evas_software_x11_x_output_buffer_new(c,
                                                    screen,
                                                    screen->root_depth,
                                                    (unsigned char *)NULL,
                                                    16, 16, 2, NULL);
        if (!xob)
          {
             cached_result = 0;
             return 0;
          }
        evas_software_x11_x_output_buffer_free(xob, 1);
        cached_result = 1;
        return 1;
     }
   cached_result = 0;
   return 0;
}

X_Output_Buffer *
evas_software_x11_x_output_buffer_new(xcb_connection_t * c,
                                      xcb_screen_t * s,
                                      int depth,
                                      unsigned char *pal,
                                      int w, int h, int try_shm, void *data)
{
   X_Output_Buffer *xob;

   xob = calloc(1, sizeof(X_Output_Buffer));
   if (!xob)
      return NULL;

   xob->connection = c;
   xob->screen = s;
   xob->xim = NULL;
   xob->shm_info = NULL;
   xob->pal = pal;

   if (try_shm > 0)
     {
        xob->shm_info = malloc(sizeof(xcb_shm_segment_info_t));
        if (xob->shm_info)
          {
             xob->shm_info->shmseg = xcb_generate_id(c);
             xob->xim = xcb_image_create_native(c, w, h,
                                                XCB_IMAGE_FORMAT_Z_PIXMAP,
                                                depth, NULL, ~0, NULL);
             if (xob->xim)
               {
                  xob->shm_info->shmid = shmget(IPC_PRIVATE,
                                                xob->xim->size,
                                                IPC_CREAT | 0777);

                  if (xob->shm_info->shmid >= 0)
                    {
                       xob->shm_info->shmaddr = xob->xim->data =
                           shmat(xob->shm_info->shmid, 0, 0);

                       if (xob->shm_info->shmaddr != NULL)
                         {
                            /*
                             * FIXME: no error mechanism
                             */

                            /* XErrorHandler ph; */
                            /* EventHandlers eh; */

                            free(xcb_get_input_focus_reply
                                 (c, xcb_get_input_focus_unchecked(c), NULL));
                            _xcb_err = 0;
                            /* ph = XSetErrorHandler((XErrorHandler) */
                            /* x_output_tmp_x_err); */
                            xcb_shm_attach(c,
                                           xob->shm_info->shmseg,
                                           xob->shm_info->shmid, 0);
                            free(xcb_get_input_focus_reply
                                 (c, xcb_get_input_focus_unchecked(c), NULL));
                            /* XSetErrorHandler((XErrorHandler)ph); */
                            if (!_xcb_err)
                              {
                                 xob->im =
                                     (Soft8_Image *)
                                     evas_cache_image_data
                                     (evas_common_soft8_image_cache_get(), w, h,
                                      (DATA32 *) xob->xim->data, 0,
                                      EVAS_COLORSPACE_GRY8);

                                 if (xob->im)
                                    xob->im->stride =
                                        xob->xim->stride / sizeof(DATA8);
                                 return xob;
                              }
                         }
                       shmdt(xob->shm_info->shmaddr);
                       shmctl(xob->shm_info->shmid, IPC_RMID, 0);
                    }
                  if (xob->xim)
                     xcb_image_destroy(xob->xim);
                  xob->xim = NULL;
               }
             if (xob->shm_info)
                free(xob->shm_info);
             xob->shm_info = NULL;
          }
     }

   if (try_shm > 1)
      return NULL;

   xob->xim = xcb_image_create_native(c, w, h, XCB_IMAGE_FORMAT_Z_PIXMAP,
                                      depth, NULL, ~0, NULL);
   if (!xob->xim)
     {
        free(xob);
        return NULL;
     }

   xob->data = data;

   if (!xob->xim->data)
     {
        xob->xim->data = malloc(xob->xim->stride * xob->xim->height);
        if (!xob->xim->data)
          {
             xcb_image_destroy(xob->xim);
             free(xob);
             return NULL;
          }
     }
   if (xob->im)
      evas_cache_image_drop(&xob->im->cache_entry);

   xob->im =
       (Soft8_Image *)
       evas_cache_image_data(evas_common_soft8_image_cache_get(), w, h,
                             (DATA32 *) xob->xim->data, 0,
                             EVAS_COLORSPACE_GRY8);
   if (xob->im)
      xob->im->stride = xob->xim->stride / sizeof(DATA8);
   return xob;
}

void
evas_software_x11_x_output_buffer_free(X_Output_Buffer * xob, int sync)
{
   if (xob->shm_info)
     {
        if (sync)
           free(xcb_get_input_focus_reply(xob->connection,
                                          xcb_get_input_focus_unchecked(xob->
                                                                        connection),
                                          NULL));
        xcb_shm_detach(xob->connection, xob->shm_info->shmseg);
        xcb_image_destroy(xob->xim);
        shmdt(xob->shm_info->shmaddr);
        shmctl(xob->shm_info->shmid, IPC_RMID, 0);
        free(xob->shm_info);
     }
   else
     {
        if (xob->data)
           xob->xim->data = NULL;
        free(xob->xim->data);
        xcb_image_destroy(xob->xim);
     }
   free(xob);
}

void
evas_software_x11_x_output_buffer_paste(X_Output_Buffer * xob,
                                        xcb_drawable_t d,
                                        xcb_gcontext_t gc,
                                        int x, int y, int w, int h, int sync)
{
   unsigned char *data;
   int i, j;

   if (xob->shm_info)
     {
        for (i = y; i < y + h; i++)
          {
             data = xob->xim->data + i * xob->xim->width + x;
             for (j = x; j < x + w; j++, data++)
                *data = xob->pal[*data];
          }

        xcb_image_shm_put(xob->connection, d, gc,
                          xob->xim, *xob->shm_info, x, y, x, y, w, h, 0);

        if (sync)
           free(xcb_get_input_focus_reply(xob->connection,
                                          xcb_get_input_focus_unchecked(xob->
                                                                        connection),
                                          NULL));
     }
   else
     {
        xcb_image_put(xob->connection, d, gc, xob->xim, x, y, 0);
     }
}

DATA8 *
evas_software_x11_x_output_buffer_data(X_Output_Buffer * xob,
                                       int *bytes_per_line_ret)
{
   if (bytes_per_line_ret)
      *bytes_per_line_ret = xob->xim->stride;
   return (DATA8 *) xob->xim->data;
}

int
evas_software_x11_x_output_buffer_depth(X_Output_Buffer * xob)
{
   return xob->xim->bpp;
}

int
evas_software_x11_x_output_buffer_byte_order(X_Output_Buffer * xob)
{
   return xob->xim->byte_order;
}

int
evas_software_x11_x_output_buffer_bit_order(X_Output_Buffer * xob)
{
   return xob->xim->bit_order;
}
