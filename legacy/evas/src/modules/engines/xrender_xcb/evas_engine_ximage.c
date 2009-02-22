#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_XRender_Xcb.h"

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
_tmp_xcb_err(xcb_connection_t *conn/* , XErrorEvent *ev */)
{
   _xcb_err = 1;
   return;
}

Xcb_Image_Info *
_xr_image_info_get(xcb_connection_t *conn, xcb_screen_t *screen, xcb_drawable_t draw, xcb_visualtype_t *visual)
{
   xcb_get_geometry_cookie_t cookie;
   xcb_get_geometry_reply_t *rep;
   Xcb_Image_Info           *xcbinf;
   Xcb_Image_Info           *xcbinf2;
   Eina_List                *l;

   xcbinf2 = NULL;
   EINA_LIST_FOREACH(_image_info_list, l, xcbinf)
     {
	if (xcbinf->conn == conn)
	  {
	     xcbinf2 = xcbinf;
	     break;
	  }
     }
   xcbinf = calloc(1, sizeof(Xcb_Image_Info));
   if (!xcbinf) return NULL;

   xcbinf->references = 1;
   xcbinf->conn = conn;
   xcbinf->screen = screen;
   xcbinf->draw = draw;
   cookie = xcb_get_geometry_unchecked(xcbinf->conn, xcbinf->draw);
   rep = xcb_get_geometry_reply(xcbinf->conn, cookie, NULL);
   xcbinf->root = rep->root;
   free(rep);
   xcbinf->visual = visual;
   xcbinf->fmt32 = xcb_render_find_standard_pictforminfo(xcbinf->conn, xcb_render_standard_pictforminfoargb_32_t);
   xcbinf->fmt24 = xcb_render_find_standard_pictforminfo(xcbinf->conn, xcb_render_standard_pictforminforgb_24_t);
   xcbinf->fmt8 = xcb_render_find_standard_pictforminfo(xcbinf->conn, xcb_render_standard_pictforminfoa_8_t);
   xcbinf->fmt4 = xcb_render_find_standard_pictforminfo(xcbinf->conn, xcb_render_standard_pictforminfoa_4_t);
   xcbinf->fmt1 = xcb_render_find_standard_pictforminfo(xcbinf->conn, xcb_render_standard_pictforminfoa_1_t);

   xcbinf->mul = _xr_render_surface_new(xcbinf, 1, 1, xcbinf->fmt32, 1);
   _xr_render_surface_repeat_set(xcbinf->mul, 1);
   xcbinf->mul_r = xcbinf->mul_g = xcbinf->mul_b = xcbinf->mul_a = 0xff;
   _xr_render_surface_solid_rectangle_set(xcbinf->mul, xcbinf->mul_r, xcbinf->mul_g, xcbinf->mul_b, xcbinf->mul_a, 0, 0, 1, 1);
   if (xcbinf2)
     {
	xcbinf->can_do_shm = xcbinf2->can_do_shm;
	xcbinf->depth = xcbinf2->depth;
     }
   else
     {
        xcb_depth_iterator_t iter_depth;
        xcb_shm_segment_info_t shm_info;
        xcb_image_t           *xcbim;

        iter_depth = xcb_screen_allowed_depths_iterator (xcbinf->screen);
        for (; iter_depth.rem ; xcb_depth_next (&iter_depth))
          {
             xcb_visualtype_iterator_t iter_visual;

             iter_visual = xcb_depth_visuals_iterator (iter_depth.data);
             for (; iter_visual.rem ; xcb_visualtype_next (&iter_visual))
               {
                  if (iter_visual.data->visual_id == visual->visual_id)
                    xcbinf->depth = iter_depth.data->depth;
               }
          }

	xcbinf->can_do_shm = 0;


        shm_info.shmseg = xcb_generate_id(xcbinf->conn);
        xcbim = xcb_image_create_native(xcbinf->conn, 1, 1,
                                        XCB_IMAGE_FORMAT_Z_PIXMAP,
                                        xcbinf->depth, NULL, ~0, NULL);
        if (xcbim) {
           shm_info.shmid = shmget(IPC_PRIVATE, xcbim->size, IPC_CREAT | 0777);
           if (shm_info.shmid >= 0) {
              shm_info.shmaddr = xcbim->data = shmat(shm_info.shmid, 0, 0);
              if ((shm_info.shmaddr != NULL) && (shm_info.shmaddr != (void *) -1)) {
                 xcb_get_input_focus_reply_t *reply;
                 /*
                  * FIXME: no error mechanism
                  */
                 /* XErrorHandler ph; */

                 /* we sync */
                 reply = xcb_get_input_focus_reply(xcbinf->conn,
                                               xcb_get_input_focus_unchecked(xcbinf->conn),
                                               NULL);
                 free(reply);
                 _xcb_err = 0;
                 /* ph = XSetErrorHandler((XErrorHandler)_tmp_xcb_err); */
                 xcb_shm_attach(xcbinf->conn, shm_info.shmseg, shm_info.shmid, 0);
                 /* we sync */
                 reply = xcb_get_input_focus_reply(xcbinf->conn,
                                               xcb_get_input_focus_unchecked(xcbinf->conn),
                                               NULL);
                 free(reply);
                 /* XSetErrorHandler((XErrorHandler)ph); */
                 if (!_xcb_err) xcbinf->can_do_shm = 1;
                 shmdt(shm_info.shmaddr);
              }
              shmctl(shm_info.shmid, IPC_RMID, 0);
           }
           xcb_image_destroy(xcbim);
        }
     }
   _image_info_list = eina_list_prepend(_image_info_list, xcbinf);
   return xcbinf;
}

void
_xr_image_info_free(Xcb_Image_Info *xcbinf)
{
  if (!xcbinf) return;
   if (xcbinf->pool)
     {
        xcb_get_input_focus_reply_t *reply;

        reply = xcb_get_input_focus_reply(xcbinf->conn,
                                          xcb_get_input_focus_unchecked(xcbinf->conn),
                                          NULL);
        free(reply);
     }
   _xr_image_info_pool_flush(xcbinf, 0, 0);
   xcbinf->references--;
   if (xcbinf->references != 0) return;
   _xr_render_surface_free(xcbinf->mul);
   if (xcbinf->fmt1)
     free(xcbinf->fmt1);
   if (xcbinf->fmt4)
     free(xcbinf->fmt4);
   if (xcbinf->fmt8)
     free(xcbinf->fmt8);
   if (xcbinf->fmt24)
     free(xcbinf->fmt24);
   if (xcbinf->fmt32)
     free(xcbinf->fmt32);
   free(xcbinf);
   _image_info_list = eina_list_remove(_image_info_list, xcbinf);
}

void
_xr_image_info_pool_flush(Xcb_Image_Info *xcbinf, int max_num, int max_mem)
{
   if ((xcbinf->pool_mem <= max_mem) && (eina_list_count(xcbinf->pool) <= max_num)) return;
   while ((xcbinf->pool_mem > max_mem) || (eina_list_count(xcbinf->pool) > max_num))
     {
	Xcb_Image_Image *xcbim;

	if (!xcbinf->pool) break;
	xcbim = xcbinf->pool->data;
	_xr_image_free(xcbim);
     }
}

Xcb_Image_Image *
_xr_image_new(Xcb_Image_Info *xcbinf, int w, int h, int depth)
{
   Xcb_Image_Image *xcbim, *xcbim2;
   Eina_List       *l;

   xcbim2 = NULL;
   EINA_LIST_FOREACH(xcbinf->pool, l, xcbim)
     {
	if ((xcbim->w >= w) && (xcbim->h >= h) && (xcbim->depth == depth) && (xcbim->available))
	  {
	     if (!xcbim2) xcbim2 = xcbim;
	     else if ((xcbim->w * xcbim->h) < (xcbim2->w * xcbim2->h)) xcbim2 = xcbim;
	  }
     }
   if (xcbim2)
     {
	xcbim2->available = 0;
	return xcbim2;
     }
   xcbim = calloc(1, sizeof(Xcb_Image_Image));
   if (xcbim)
     {
	xcbim->xcbinf = xcbinf;
	xcbim->w = w;
	xcbim->h = h;
	xcbim->depth = depth;
	xcbim->available = 0;
	if (xcbim->xcbinf->can_do_shm)
	  {
	     xcbim->shm_info = calloc(1, sizeof(xcb_shm_segment_info_t));
	     if (xcbim->shm_info)
	       {
                  xcbim->shm_info->shmseg = xcb_generate_id(xcbinf->conn);
		  xcbim->xcbim = xcb_image_create_native(xcbim->xcbinf->conn, xcbim->w, xcbim->h, XCB_IMAGE_FORMAT_Z_PIXMAP, xcbim->depth, NULL, ~0, NULL);
		  if (xcbim->xcbim)
		    {
		       xcbim->shm_info->shmid = shmget(IPC_PRIVATE, xcbim->xcbim->size, IPC_CREAT | 0777);
		       if (xcbim->shm_info->shmid >= 0)
			 {
			    xcbim->shm_info->shmaddr = xcbim->xcbim->data = shmat(xcbim->shm_info->shmid, 0, 0);
			    if ((xcbim->shm_info->shmaddr) && (xcbim->shm_info->shmaddr != (void *) -1))
			      {
                                 xcb_get_input_focus_reply_t *reply;
                                 /*
                                  * FIXME: no error mechanism
                                  */
                                 /*				 XErrorHandler ph; */

                                 /* we sync */
                                 reply = xcb_get_input_focus_reply(xcbim->xcbinf->conn,
                                                               xcb_get_input_focus_unchecked(xcbim->xcbinf->conn),
                                                               NULL);
                                 free(reply);
				 _xcb_err = 0;
/*				 ph = XSetErrorHandler((XErrorHandler)_tmp_xcb_err); */
				 xcb_shm_attach(xcbim->xcbinf->conn, xcbim->shm_info->shmseg, xcbim->shm_info->shmid, 0);
                                 /* we sync */
                                 reply = xcb_get_input_focus_reply(xcbim->xcbinf->conn,
                                                               xcb_get_input_focus_unchecked(xcbim->xcbinf->conn),
                                                               NULL);
                                 free(reply);
/*				 XSetErrorHandler((XErrorHandler)ph); */
				 if (!_xcb_err) goto xcbim_ok;
				 shmdt(xcbim->shm_info->shmaddr);
			      }
			    shmctl(xcbim->shm_info->shmid, IPC_RMID, 0);
			 }
		       xcb_image_destroy(xcbim->xcbim);
		    }
		  free(xcbim->shm_info);
		  xcbim->shm_info = NULL;
	       }
	  }
	xcbim->xcbim = xcb_image_create_native(xcbim->xcbinf->conn, xcbim->w, xcbim->h, XCB_IMAGE_FORMAT_Z_PIXMAP, xcbim->depth, NULL, ~0, NULL);
	if (!xcbim->xcbim)
	  {
	     free(xcbim);
	     return NULL;
	  }
	xcbim->xcbim->data = malloc(xcbim->xcbim->size);
	if (!xcbim->xcbim->data)
	  {
	     xcb_image_destroy(xcbim->xcbim);
	     free(xcbim);
	     return NULL;
	  }
     }

   xcbim_ok:
   _xr_image_info_pool_flush(xcbinf, 32, (1600 * 1200 * 32 * 2));

   xcbim->line_bytes = xcbim->xcbim->stride;
   xcbim->data = (void *)(xcbim->xcbim->data);
   xcbinf->pool_mem += (xcbim->w * xcbim->h * xcbim->depth);
   xcbinf->pool = eina_list_append(xcbinf->pool, xcbim);
   return xcbim;
}

void
_xr_image_free(Xcb_Image_Image *xcbim)
{
   if (xcbim->shm_info)
     {
	if (!xcbim->available)
          {
            xcb_get_input_focus_reply_t *reply;

            reply = xcb_get_input_focus_reply(xcbim->xcbinf->conn,
                                              xcb_get_input_focus_unchecked(xcbim->xcbinf->conn),
                                              NULL);
            free(reply);
          }
	xcb_shm_detach(xcbim->xcbinf->conn, xcbim->shm_info->shmseg);
	xcb_image_destroy(xcbim->xcbim);
	shmdt(xcbim->shm_info->shmaddr);
	shmctl(xcbim->shm_info->shmid, IPC_RMID, 0);
	free(xcbim->shm_info);
     }
   else
     {
	free(xcbim->xcbim->data);
	xcbim->xcbim->data = NULL;
	xcb_image_destroy(xcbim->xcbim);
     }
   xcbim->xcbinf->pool_mem -= (xcbim->w * xcbim->h * xcbim->depth);
   xcbim->xcbinf->pool = eina_list_remove(xcbim->xcbinf->pool, xcbim);
   free(xcbim);
}

void
_xr_image_put(Xcb_Image_Image *xcbim, xcb_drawable_t draw, int x, int y, int w, int h)
{
   xcb_get_input_focus_reply_t *reply;
   xcb_gcontext_t               gc;

   gc = xcb_generate_id(xcbim->xcbinf->conn);
   xcb_create_gc(xcbim->xcbinf->conn, gc, draw, 0, NULL);
   if (xcbim->shm_info)
     {
	xcb_shm_put_image(xcbim->xcbinf->conn, draw, gc,
                       xcbim->xcbim->width, xcbim->xcbim->height,
                       0, 0,
                       w, h,
                       x, y,
                       xcbim->xcbim->depth, xcbim->xcbim->format,
                       0,
                       xcbim->shm_info->shmseg,
                       xcbim->xcbim->data - xcbim->shm_info->shmaddr);
        /* we sync */
        reply = xcb_get_input_focus_reply(xcbim->xcbinf->conn,
                                      xcb_get_input_focus_unchecked(xcbim->xcbinf->conn),
                                      NULL);
        free(reply);
     }
   else
     xcb_image_put(xcbim->xcbinf->conn, draw, gc, xcbim->xcbim, x, y, 0);
   xcbim->available = 1;
   xcb_free_gc(xcbim->xcbinf->conn, gc);
}
