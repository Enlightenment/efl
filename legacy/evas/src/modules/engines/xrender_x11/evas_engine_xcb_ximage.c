#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_XRender_X11.h"

static Eina_List *_image_info_list = NULL;

static int _xcb_err = 0;

enum
{
  xcb_render_pictforminfo_id            = 1 << 0,
  xcb_render_pictforminfo_type_t        = 1 << 1,
  xcb_render_pictforminfo_depth_t       = 1 << 2,
  xcb_render_pictforminfo_red_shift_t   = 1 << 3,
  xcb_render_pictforminfo_red_mask_t    = 1 << 4,
  xcb_render_pictforminfo_green_shift_t = 1 << 5,
  xcb_render_pictforminfo_green_mask_t  = 1 << 6,
  xcb_render_pictforminfo_blue_shift_t  = 1 << 7,
  xcb_render_pictforminfo_blue_mask_t   = 1 << 8,
  xcb_render_pictforminfo_alpha_shift_t = 1 << 9,
  xcb_render_pictforminfo_alpha_mask_t  = 1 << 10,
  xcb_render_pictforminfo_colormap_t    = 1 << 11,
};

enum
{
  xcb_render_standard_pictforminfoargb_32_t,
  xcb_render_standard_pictforminforgb_24_t,
  xcb_render_standard_pictforminfoa_8_t,
  xcb_render_standard_pictforminfoa_4_t,
  xcb_render_standard_pictforminfoa_1_t,
  xcb_render_standard_pictforminfo_count_t
};

static xcb_render_pictforminfo_t *
xcb_render_find_pictforminfo (xcb_connection_t *conn, uint32_t mask, const xcb_render_pictforminfo_t *template, int count)
{
  xcb_render_query_pict_formats_cookie_t cookie;
  xcb_render_query_pict_formats_reply_t *rep;
  xcb_render_pictforminfo_iterator_t     iter_forminfo;

  cookie = xcb_render_query_pict_formats_unchecked (conn);
  rep = xcb_render_query_pict_formats_reply (conn, cookie, NULL);
  iter_forminfo = xcb_render_query_pict_formats_formats_iterator (rep);
  for (; iter_forminfo.rem; xcb_render_pictforminfo_next (&iter_forminfo)) {
    if (mask & xcb_render_pictforminfo_id)
      if (template->id != iter_forminfo.data->id)
        continue;
    if (mask & xcb_render_pictforminfo_type_t)
      if (template->type != iter_forminfo.data->type)
        continue;
    if (mask & xcb_render_pictforminfo_depth_t)
      if (template->depth != iter_forminfo.data->depth)
        continue;
    if (mask & xcb_render_pictforminfo_red_shift_t)
      if (template->direct.red_shift != iter_forminfo.data->direct.red_shift)
        continue;
    if (mask & xcb_render_pictforminfo_red_mask_t)
      if (template->direct.red_mask != iter_forminfo.data->direct.red_mask)
        continue;
    if (mask & xcb_render_pictforminfo_green_shift_t)
      if (template->direct.green_shift != iter_forminfo.data->direct.green_shift)
        continue;
    if (mask & xcb_render_pictforminfo_green_mask_t)
      if (template->direct.green_mask != iter_forminfo.data->direct.green_mask)
        continue;
    if (mask & xcb_render_pictforminfo_blue_shift_t)
      if (template->direct.blue_shift != iter_forminfo.data->direct.blue_shift)
        continue;
    if (mask & xcb_render_pictforminfo_blue_mask_t)
      if (template->direct.blue_mask != iter_forminfo.data->direct.blue_mask)
        continue;
    if (mask & xcb_render_pictforminfo_alpha_shift_t)
      if (template->direct.alpha_shift != iter_forminfo.data->direct.alpha_shift)
        continue;
    if (mask & xcb_render_pictforminfo_alpha_mask_t)
      if (template->direct.alpha_mask != iter_forminfo.data->direct.alpha_mask)
        continue;
    if (mask & xcb_render_pictforminfo_colormap_t)
      if (template->colormap != iter_forminfo.data->colormap)
        continue;
    if (count-- == 0) {
      xcb_render_pictforminfo_t *forminfo;

      forminfo = (xcb_render_pictforminfo_t *)malloc (sizeof (xcb_render_pictforminfo_t));
      memcpy (forminfo, iter_forminfo.data, sizeof (xcb_render_pictforminfo_t));

      free (rep);
      return forminfo;
    }
  }
  free (rep);
  return NULL;
}

static xcb_render_pictforminfo_t *
xcb_render_find_standard_pictforminfo (xcb_connection_t *conn, int format)
{
  xcb_render_pictforminfo_t *forminfo = NULL;
  struct {
    xcb_render_pictforminfo_t template;
    uint32_t                mask;
  } standardFormats[xcb_render_standard_pictforminfo_count_t] = {
    /* StandardPICTFORMINFOARGB32 */
    {
      {
        0,                                /* id */
        XCB_RENDER_PICT_TYPE_DIRECT,          /* type */
        32,                               /* depth */
        { 0, 0 },                         /* pad */
        {                                 /* direct */
          16,                             /* direct.red_shift */
          0xff,                           /* direct.red_mask */
          8,                              /* direct.green_shift */
          0xff,                           /* direct.green_mask */
          0,                              /* direct.blue_shift */
          0xff,                           /* direct.blue_mask */
          24,                             /* direct.alpha_shift */
          0xff                            /* direct.alpha_mask */
        },
        0                                 /* colormap */
      },
      xcb_render_pictforminfo_type_t        |
      xcb_render_pictforminfo_depth_t       |
      xcb_render_pictforminfo_red_shift_t   |
      xcb_render_pictforminfo_red_mask_t    |
      xcb_render_pictforminfo_green_shift_t |
      xcb_render_pictforminfo_green_mask_t  |
      xcb_render_pictforminfo_blue_shift_t  |
      xcb_render_pictforminfo_blue_mask_t   |
      xcb_render_pictforminfo_alpha_shift_t |
      xcb_render_pictforminfo_alpha_mask_t
    },
    /* StandardPICTFORMINFORGB24 */
    {
      {
        0,                                /* id */
        XCB_RENDER_PICT_TYPE_DIRECT,          /* type */
        24,                               /* depth */
        { 0, 0 },                         /* pad */
        {                                 /* direct */
          16,                             /* direct.red_shift */
          0xff,                           /* direct.red_mask */
          8,                              /* direct.green_shift */
          0xff,                           /* direct.green_mask */
          0,                              /* direct.blue_shift */
          0xff,                           /* direct.blue_mask */
          0,                              /* direct.alpha_shift */
          0x00                            /* direct.alpha_mask */
        },
        0                                 /* colormap */
      },
      xcb_render_pictforminfo_type_t        |
      xcb_render_pictforminfo_depth_t       |
      xcb_render_pictforminfo_red_shift_t   |
      xcb_render_pictforminfo_red_mask_t    |
      xcb_render_pictforminfo_green_shift_t |
      xcb_render_pictforminfo_green_mask_t  |
      xcb_render_pictforminfo_blue_shift_t  |
      xcb_render_pictforminfo_blue_mask_t   |
      xcb_render_pictforminfo_alpha_mask_t
    },
    /* StandardPICTFORMINFOA8 */
    {
      {
        0,                                /* id */
        XCB_RENDER_PICT_TYPE_DIRECT,          /* type */
        8,                                /* depth */
        { 0, 0 },                         /* pad */
        {                                 /* direct */
          0,                              /* direct.red_shift */
          0x00,                           /* direct.red_mask */
          0,                              /* direct.green_shift */
          0x00,                           /* direct.green_mask */
          0,                              /* direct.blue_shift */
          0x00,                           /* direct.blue_mask */
          0,                              /* direct.alpha_shift */
          0xff                            /* direct.alpha_mask */
        },
        0                                 /* colormap */
      },
      xcb_render_pictforminfo_type_t        |
      xcb_render_pictforminfo_depth_t       |
      xcb_render_pictforminfo_red_mask_t    |
      xcb_render_pictforminfo_green_mask_t  |
      xcb_render_pictforminfo_blue_mask_t   |
      xcb_render_pictforminfo_alpha_shift_t |
      xcb_render_pictforminfo_alpha_mask_t
    },
    /* StandardPICTFORMINFOA4 */
    {
      {
        0,                                /* id */
        XCB_RENDER_PICT_TYPE_DIRECT,          /* type */
        4,                                /* depth */
        { 0, 0 },                         /* pad */
        {                                 /* direct */
          0,                              /* direct.red_shift */
          0x00,                           /* direct.red_mask */
          0,                              /* direct.green_shift */
          0x00,                           /* direct.green_mask */
          0,                              /* direct.blue_shift */
          0x00,                           /* direct.blue_mask */
          0,                              /* direct.alpha_shift */
          0x0f                            /* direct.alpha_mask */
        },
        0                                 /* colormap */
      },
      xcb_render_pictforminfo_type_t        |
      xcb_render_pictforminfo_depth_t       |
      xcb_render_pictforminfo_red_mask_t    |
      xcb_render_pictforminfo_green_mask_t  |
      xcb_render_pictforminfo_blue_mask_t   |
      xcb_render_pictforminfo_alpha_shift_t |
      xcb_render_pictforminfo_alpha_mask_t
    },
    /* StandardPICTFORMINFOA1 */
    {
      {
        0,                                /* id */
        XCB_RENDER_PICT_TYPE_DIRECT,          /* type */
        1,                                /* depth */
        { 0, 0 },                         /* pad */
        {                                 /* direct */
          0,                              /* direct.red_shift */
          0x00,                           /* direct.red_mask */
          0,                              /* direct.green_shift */
          0x00,                           /* direct.green_mask */
          0,                              /* direct.blue_shift */
          0x00,                           /* direct.blue_mask */
          0,                              /* direct.alpha_shift */
          0x01                            /* direct.alpha_mask */
        },
        0                                 /* colormap */
      },
      xcb_render_pictforminfo_type_t        |
      xcb_render_pictforminfo_depth_t       |
      xcb_render_pictforminfo_red_mask_t    |
      xcb_render_pictforminfo_green_mask_t  |
      xcb_render_pictforminfo_blue_mask_t   |
      xcb_render_pictforminfo_alpha_shift_t |
      xcb_render_pictforminfo_alpha_mask_t
    },
  };
  if ((format >= 0) && (format < xcb_render_standard_pictforminfo_count_t))
    forminfo = xcb_render_find_pictforminfo (conn,
                                             standardFormats[format].mask,
                                             &standardFormats[format].template,
                                             0);
  return forminfo;
}

static void
_tmp_xcb_err(xcb_connection_t *conn __UNUSED__/* , XErrorEvent *ev */)
{
   _xcb_err = 1;
   return;
}

Ximage_Info *
_xr_xcb_image_info_get(xcb_connection_t *conn, xcb_screen_t *screen, xcb_drawable_t draw, xcb_visualtype_t *visual)
{
   xcb_get_geometry_cookie_t cookie;
   xcb_get_geometry_reply_t *rep;
   Ximage_Info              *xinf;
   Ximage_Info              *xinf2;
   Eina_List                *l;

   xinf2 = NULL;
   EINA_LIST_FOREACH(_image_info_list, l, xinf)
     {
	if (xinf->x11.connection == conn)
	  {
	     xinf2 = xinf;
	     break;
	  }
     }
   xinf = calloc(1, sizeof(Ximage_Info));
   if (!xinf) return NULL;

   xinf->references = 1;
   xinf->x11.connection = conn;
   xinf->x11.screen = screen;
   xinf->x11.draw = draw;
   cookie = xcb_get_geometry_unchecked(xinf->x11.connection, xinf->x11.draw);
   rep = xcb_get_geometry_reply(xinf->x11.connection, cookie, NULL);
   xinf->x11.root = rep->root;
   free(rep);
   xinf->x11.visual = visual;
   xinf->x11.fmt32 = xcb_render_find_standard_pictforminfo(xinf->x11.connection, xcb_render_standard_pictforminfoargb_32_t);
   xinf->x11.fmt24 = xcb_render_find_standard_pictforminfo(xinf->x11.connection, xcb_render_standard_pictforminforgb_24_t);
   xinf->x11.fmt8 = xcb_render_find_standard_pictforminfo(xinf->x11.connection, xcb_render_standard_pictforminfoa_8_t);
   xinf->x11.fmt4 = xcb_render_find_standard_pictforminfo(xinf->x11.connection, xcb_render_standard_pictforminfoa_4_t);
   xinf->x11.fmt1 = xcb_render_find_standard_pictforminfo(xinf->x11.connection, xcb_render_standard_pictforminfoa_1_t);

   xinf->mul = _xr_xcb_render_surface_new(xinf, 1, 1, xinf->x11.fmt32, 1);
   _xr_xcb_render_surface_repeat_set(xinf->mul, 1);
   xinf->mul_r = xinf->mul_g = xinf->mul_b = xinf->mul_a = 0xff;
   _xr_xcb_render_surface_solid_rectangle_set(xinf->mul, xinf->mul_r, xinf->mul_g, xinf->mul_b, xinf->mul_a, 0, 0, 1, 1);
   if (xinf2)
     {
	xinf->can_do_shm = xinf2->can_do_shm;
	xinf->depth = xinf2->depth;
     }
   else
     {
        xcb_depth_iterator_t iter_depth;
        xcb_shm_segment_info_t shm_info;
        xcb_image_t           *xim;

        iter_depth = xcb_screen_allowed_depths_iterator (xinf->x11.screen);
        for (; iter_depth.rem ; xcb_depth_next (&iter_depth))
          {
             xcb_visualtype_iterator_t iter_visual;

             iter_visual = xcb_depth_visuals_iterator (iter_depth.data);
             for (; iter_visual.rem ; xcb_visualtype_next (&iter_visual))
               {
                  if (iter_visual.data->visual_id == visual->visual_id)
                    xinf->depth = iter_depth.data->depth;
               }
          }

	xinf->can_do_shm = 0;


        shm_info.shmseg = xcb_generate_id(xinf->x11.connection);
        xim = xcb_image_create_native(xinf->x11.connection, 1, 1,
                                        XCB_IMAGE_FORMAT_Z_PIXMAP,
                                        xinf->depth, NULL, ~0, NULL);
        if (xim) {
           shm_info.shmid = shmget(IPC_PRIVATE, xim->size, IPC_CREAT | 0777);
           if (shm_info.shmid >= 0) {
              shm_info.shmaddr = xim->data = shmat(shm_info.shmid, 0, 0);
              if ((shm_info.shmaddr != NULL) && (shm_info.shmaddr != (void *) -1)) {
                 xcb_get_input_focus_reply_t *reply;
                 /*
                  * FIXME: no error mechanism
                  */
                 /* XErrorHandler ph; */

                 /* we sync */
                 reply = xcb_get_input_focus_reply(xinf->x11.connection,
                                               xcb_get_input_focus_unchecked(xinf->x11.connection),
                                               NULL);
                 free(reply);
                 _xcb_err = 0;
                 /* ph = XSetErrorHandler((XErrorHandler)_tmp_xcb_err); */
                 xcb_shm_attach(xinf->x11.connection, shm_info.shmseg, shm_info.shmid, 0);
                 /* we sync */
                 reply = xcb_get_input_focus_reply(xinf->x11.connection,
                                               xcb_get_input_focus_unchecked(xinf->x11.connection),
                                               NULL);
                 free(reply);
                 /* XSetErrorHandler((XErrorHandler)ph); */
                 if (!_xcb_err) xinf->can_do_shm = 1;
                 shmdt(shm_info.shmaddr);
              }
              shmctl(shm_info.shmid, IPC_RMID, 0);
           }
           xcb_image_destroy(xim);
        }
     }
   _image_info_list = eina_list_prepend(_image_info_list, xinf);
   return xinf;
}

void
_xr_xcb_image_info_free(Ximage_Info *xinf)
{
  if (!xinf) return;
   if (xinf->pool)
     {
        xcb_get_input_focus_reply_t *reply;

        reply = xcb_get_input_focus_reply(xinf->x11.connection,
                                          xcb_get_input_focus_unchecked(xinf->x11.connection),
                                          NULL);
        free(reply);
     }
   _xr_xcb_image_info_pool_flush(xinf, 0, 0);
   xinf->references--;
   if (xinf->references != 0) return;
   _xr_xcb_render_surface_free(xinf->mul);
   if (xinf->x11.fmt1)
     free(xinf->x11.fmt1);
   if (xinf->x11.fmt4)
     free(xinf->x11.fmt4);
   if (xinf->x11.fmt8)
     free(xinf->x11.fmt8);
   if (xinf->x11.fmt24)
     free(xinf->x11.fmt24);
   if (xinf->x11.fmt32)
     free(xinf->x11.fmt32);
   free(xinf);
   _image_info_list = eina_list_remove(_image_info_list, xinf);
}

void
_xr_xcb_image_info_pool_flush(Ximage_Info *xinf, int max_num, int max_mem)
{
   if ((xinf->pool_mem <= max_mem) && (eina_list_count(xinf->pool) <= max_num)) return;
   while ((xinf->pool_mem > max_mem) || (eina_list_count(xinf->pool) > max_num))
     {
	Ximage_Image *xim;

	if (!xinf->pool) break;
	xim = xinf->pool->data;
	_xr_xcb_image_free(xim);
     }
}

Ximage_Image *
_xr_xcb_image_new(Ximage_Info *xinf, int w, int h, int depth)
{
   Ximage_Image *xim;
   Ximage_Image *xim2;
   Eina_List    *l;

   xim2 = NULL;
   EINA_LIST_FOREACH(xinf->pool, l, xim)
     {
	if ((xim->width >= w) && (xim->height >= h) && (xim->depth == depth) && (xim->available))
	  {
	     if (!xim2) xim2 = xim;
	     else if ((xim->width * xim->height) < (xim2->width * xim2->height)) xim2 = xim;
	  }
     }
   if (xim2)
     {
	xim2->available = 0;
	return xim2;
     }
   xim = calloc(1, sizeof(Ximage_Image));
   if (xim)
     {
	xim->xinf = xinf;
	xim->width = w;
	xim->height = h;
	xim->depth = depth;
	xim->available = 0;
	if (xim->xinf->can_do_shm)
	  {
	     xim->x11.xcb.shm_info = calloc(1, sizeof(xcb_shm_segment_info_t));
	     if (xim->x11.xcb.shm_info)
	       {
                  xim->x11.xcb.shm_info->shmseg = xcb_generate_id(xinf->x11.connection);
		  xim->x11.xcb.xim = xcb_image_create_native(xim->xinf->x11.connection, xim->width, xim->height, XCB_IMAGE_FORMAT_Z_PIXMAP, xim->depth, NULL, ~0, NULL);
		  if (xim->x11.xcb.xim)
		    {
		       xim->x11.xcb.shm_info->shmid = shmget(IPC_PRIVATE, xim->x11.xcb.xim->size, IPC_CREAT | 0777);
		       if (xim->x11.xcb.shm_info->shmid >= 0)
			 {
			    xim->x11.xcb.shm_info->shmaddr = xim->x11.xcb.xim->data = shmat(xim->x11.xcb.shm_info->shmid, 0, 0);
			    if ((xim->x11.xcb.shm_info->shmaddr) && (xim->x11.xcb.shm_info->shmaddr != (void *) -1))
			      {
                                 xcb_get_input_focus_reply_t *reply;
                                 /*
                                  * FIXME: no error mechanism
                                  */
                                 /*				 XErrorHandler ph; */

                                 /* we sync */
                                 reply = xcb_get_input_focus_reply(xim->xinf->x11.connection,
                                                               xcb_get_input_focus_unchecked(xim->xinf->x11.connection),
                                                               NULL);
                                 free(reply);
				 _xcb_err = 0;
/*				 ph = XSetErrorHandler((XErrorHandler)_tmp_xcb_err); */
				 xcb_shm_attach(xim->xinf->x11.connection, xim->x11.xcb.shm_info->shmseg, xim->x11.xcb.shm_info->shmid, 0);
                                 /* we sync */
                                 reply = xcb_get_input_focus_reply(xim->xinf->x11.connection,
                                                               xcb_get_input_focus_unchecked(xim->xinf->x11.connection),
                                                               NULL);
                                 free(reply);
/*				 XSetErrorHandler((XErrorHandler)ph); */
				 if (!_xcb_err) goto xim_ok;
				 shmdt(xim->x11.xcb.shm_info->shmaddr);
			      }
			    shmctl(xim->x11.xcb.shm_info->shmid, IPC_RMID, 0);
			 }
		       xcb_image_destroy(xim->x11.xcb.xim);
		    }
		  free(xim->x11.xcb.shm_info);
		  xim->x11.xcb.shm_info = NULL;
	       }
	  }
	xim->x11.xcb.xim = xcb_image_create_native(xim->xinf->x11.connection, xim->width, xim->height, XCB_IMAGE_FORMAT_Z_PIXMAP, xim->depth, NULL, ~0, NULL);
	if (!xim->x11.xcb.xim)
	  {
	     free(xim);
	     return NULL;
	  }
	xim->x11.xcb.xim->data = malloc(xim->x11.xcb.xim->size);
	if (!xim->x11.xcb.xim->data)
	  {
	     xcb_image_destroy(xim->x11.xcb.xim);
	     free(xim);
	     return NULL;
	  }
     }

   xim_ok:
   _xr_xcb_image_info_pool_flush(xinf, 32, (1600 * 1200 * 32 * 2));

   xim->line_bytes = xim->x11.xcb.xim->stride;
   xim->data = (void *)(xim->x11.xcb.xim->data);
   xinf->pool_mem += (xim->width * xim->height * xim->depth);
   xinf->pool = eina_list_append(xinf->pool, xim);
   return xim;
}

void
_xr_xcb_image_free(Ximage_Image *xim)
{
   if (xim->x11.xcb.shm_info)
     {
	if (!xim->available)
          {
            xcb_get_input_focus_reply_t *reply;

            reply = xcb_get_input_focus_reply(xim->xinf->x11.connection,
                                              xcb_get_input_focus_unchecked(xim->xinf->x11.connection),
                                              NULL);
            free(reply);
          }
	xcb_shm_detach(xim->xinf->x11.connection, xim->x11.xcb.shm_info->shmseg);
	xcb_image_destroy(xim->x11.xcb.xim);
	shmdt(xim->x11.xcb.shm_info->shmaddr);
	shmctl(xim->x11.xcb.shm_info->shmid, IPC_RMID, 0);
	free(xim->x11.xcb.shm_info);
     }
   else
     {
	free(xim->x11.xcb.xim->data);
	xim->x11.xcb.xim->data = NULL;
	xcb_image_destroy(xim->x11.xcb.xim);
     }
   xim->xinf->pool_mem -= (xim->width * xim->height * xim->depth);
   xim->xinf->pool = eina_list_remove(xim->xinf->pool, xim);
   free(xim);
}

void
_xr_xcb_image_put(Ximage_Image *xim, xcb_drawable_t draw, int x, int y, int w, int h)
{
   xcb_get_input_focus_reply_t *reply;
   xcb_gcontext_t               gc;

   gc = xcb_generate_id(xim->xinf->x11.connection);
   xcb_create_gc(xim->xinf->x11.connection, gc, draw, 0, NULL);
   if (xim->x11.xcb.shm_info)
     {
	xcb_shm_put_image(xim->xinf->x11.connection, draw, gc,
                       xim->x11.xcb.xim->width, xim->x11.xcb.xim->height,
                       0, 0,
                       w, h,
                       x, y,
                       xim->x11.xcb.xim->depth, xim->x11.xcb.xim->format,
                       0,
                       xim->x11.xcb.shm_info->shmseg,
                       xim->x11.xcb.xim->data - xim->x11.xcb.shm_info->shmaddr);
        /* we sync */
        reply = xcb_get_input_focus_reply(xim->xinf->x11.connection,
                                      xcb_get_input_focus_unchecked(xim->xinf->x11.connection),
                                      NULL);
        free(reply);
     }
   else
     xcb_image_put(xim->xinf->x11.connection, draw, gc, xim->x11.xcb.xim, x, y, 0);
   xim->available = 1;
   xcb_free_gc(xim->xinf->x11.connection, gc);
}
