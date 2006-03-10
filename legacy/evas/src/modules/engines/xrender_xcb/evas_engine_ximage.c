#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_XRender_Xcb.h"

static Evas_List *_image_info_list = NULL;

static int _xcb_err = 0;

enum
{
  XCBRenderPICTFORMINFOId         = 1 << 0,
  XCBRenderPICTFORMINFOType       = 1 << 1,
  XCBRenderPICTFORMINFODepth      = 1 << 2,
  XCBRenderPICTFORMINFORedShift   = 1 << 3,
  XCBRenderPICTFORMINFORedMask    = 1 << 4,
  XCBRenderPICTFORMINFOGreenShift = 1 << 5,
  XCBRenderPICTFORMINFOGreenMask  = 1 << 6,
  XCBRenderPICTFORMINFOBlueShift  = 1 << 7,
  XCBRenderPICTFORMINFOBlueMask   = 1 << 8,
  XCBRenderPICTFORMINFOAlphaShift = 1 << 9,
  XCBRenderPICTFORMINFOAlphaMask  = 1 << 10,
  XCBRenderPICTFORMINFOColormap   = 1 << 11,
};

enum
{
  XCBRenderStandardPICTFORMINFOARGB32,
  XCBRenderStandardPICTFORMINFORGB24,
  XCBRenderStandardPICTFORMINFOA8,
  XCBRenderStandardPICTFORMINFOA4,
  XCBRenderStandardPICTFORMINFOA1,
  XCBRenderStandardPICTFORMINFOCount
};

static XCBRenderPICTFORMINFO *
XCBRenderFindPICTFORMINFO (XCBConnection *conn, CARD32 mask, const XCBRenderPICTFORMINFO *template, int count)
{
  XCBRenderQueryPictFormatsCookie cookie;
  XCBRenderQueryPictFormatsRep   *rep;
  XCBRenderPICTFORMINFOIter       iter_forminfo;

  cookie = XCBRenderQueryPictFormats (conn);
  rep = XCBRenderQueryPictFormatsReply (conn, cookie, NULL);
  iter_forminfo = XCBRenderQueryPictFormatsFormatsIter (rep);
  for (; iter_forminfo.rem; XCBRenderPICTFORMINFONext (&iter_forminfo)) {
    if (mask & XCBRenderPICTFORMINFOId)
      if (template->id.xid != iter_forminfo.data->id.xid)
        continue;
    if (mask & XCBRenderPICTFORMINFOType)
      if (template->type != iter_forminfo.data->type)
        continue;
    if (mask & XCBRenderPICTFORMINFODepth)
      if (template->depth != iter_forminfo.data->depth)
        continue;
    if (mask & XCBRenderPICTFORMINFORedShift)
      if (template->direct.red_shift != iter_forminfo.data->direct.red_shift)
        continue;
    if (mask & XCBRenderPICTFORMINFORedMask)
      if (template->direct.red_mask != iter_forminfo.data->direct.red_mask)
        continue;
    if (mask & XCBRenderPICTFORMINFOGreenShift)
      if (template->direct.green_shift != iter_forminfo.data->direct.green_shift)
        continue;
    if (mask & XCBRenderPICTFORMINFOGreenMask)
      if (template->direct.green_mask != iter_forminfo.data->direct.green_mask)
        continue;
    if (mask & XCBRenderPICTFORMINFOBlueShift)
      if (template->direct.blue_shift != iter_forminfo.data->direct.blue_shift)
        continue;
    if (mask & XCBRenderPICTFORMINFOBlueMask)
      if (template->direct.blue_mask != iter_forminfo.data->direct.blue_mask)
        continue;
    if (mask & XCBRenderPICTFORMINFOAlphaShift)
      if (template->direct.alpha_shift != iter_forminfo.data->direct.alpha_shift)
        continue;
    if (mask & XCBRenderPICTFORMINFOAlphaMask)
      if (template->direct.alpha_mask != iter_forminfo.data->direct.alpha_mask)
        continue;
    if (mask & XCBRenderPICTFORMINFOColormap)
      if (template->colormap.xid != iter_forminfo.data->colormap.xid)
        continue;
    if (count-- == 0) {
      XCBRenderPICTFORMINFO *forminfo;

      forminfo = (XCBRenderPICTFORMINFO *)malloc (sizeof (XCBRenderPICTFORMINFO));
      memcpy (forminfo, iter_forminfo.data, sizeof (XCBRenderPICTFORMINFO));

      free (rep);
      return forminfo;
    }
  }
  free (rep);
  return NULL;
}

static XCBRenderPICTFORMINFO *
XCBRenderFindStandardPICTFORMINFO (XCBConnection *conn, int format)
{
  XCBRenderPICTFORMINFO *forminfo = NULL;
  struct {
    XCBRenderPICTFORMINFO template;
    CARD32                mask;
  } standardFormats[XCBRenderStandardPICTFORMINFOCount] = {
    /* StandardPICTFORMINFOARGB32 */
    {
      {
        { 0, },                           /* id */
        XCBRenderPictTypeDirect,          /* type */
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
        { 0, }                            /* colormap */
      },
      XCBRenderPICTFORMINFOType       |
      XCBRenderPICTFORMINFODepth      |
      XCBRenderPICTFORMINFORedShift   |
      XCBRenderPICTFORMINFORedMask    |
      XCBRenderPICTFORMINFOGreenShift |
      XCBRenderPICTFORMINFOGreenMask  |
      XCBRenderPICTFORMINFOBlueShift  |
      XCBRenderPICTFORMINFOBlueMask   |
      XCBRenderPICTFORMINFOAlphaShift |
      XCBRenderPICTFORMINFOAlphaMask
    },
    /* StandardPICTFORMINFORGB24 */
    {
      {
        { 0, },                           /* id */
        XCBRenderPictTypeDirect,          /* type */
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
        { 0, }                            /* colormap */
      },
      XCBRenderPICTFORMINFOType       |
      XCBRenderPICTFORMINFODepth      |
      XCBRenderPICTFORMINFORedShift   |
      XCBRenderPICTFORMINFORedMask    |
      XCBRenderPICTFORMINFOGreenShift |
      XCBRenderPICTFORMINFOGreenMask  |
      XCBRenderPICTFORMINFOBlueShift  |
      XCBRenderPICTFORMINFOBlueMask   |
      XCBRenderPICTFORMINFOAlphaMask
    },
    /* StandardPICTFORMINFOA8 */
    {
      {
        { 0, },                           /* id */
        XCBRenderPictTypeDirect,          /* type */
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
        { 0, }                            /* colormap */
      },
      XCBRenderPICTFORMINFOType       |
      XCBRenderPICTFORMINFODepth      |
      XCBRenderPICTFORMINFORedMask    |
      XCBRenderPICTFORMINFOGreenMask  |
      XCBRenderPICTFORMINFOBlueMask   |
      XCBRenderPICTFORMINFOAlphaShift |
      XCBRenderPICTFORMINFOAlphaMask
    },
    /* StandardPICTFORMINFOA4 */
    {
      {
        { 0, },                           /* id */
        XCBRenderPictTypeDirect,          /* type */
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
        { 0, }                            /* colormap */
      },
      XCBRenderPICTFORMINFOType       |
      XCBRenderPICTFORMINFODepth      |
      XCBRenderPICTFORMINFORedMask    |
      XCBRenderPICTFORMINFOGreenMask  |
      XCBRenderPICTFORMINFOBlueMask   |
      XCBRenderPICTFORMINFOAlphaShift |
      XCBRenderPICTFORMINFOAlphaMask
    },
    /* StandardPICTFORMINFOA1 */
    {
      {
        { 0, },                           /* id */
        XCBRenderPictTypeDirect,          /* type */
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
        { 0, }                            /* colormap */
      },
      XCBRenderPICTFORMINFOType       |
      XCBRenderPICTFORMINFODepth      |
      XCBRenderPICTFORMINFORedMask    |
      XCBRenderPICTFORMINFOGreenMask  |
      XCBRenderPICTFORMINFOBlueMask   |
      XCBRenderPICTFORMINFOAlphaShift |
      XCBRenderPICTFORMINFOAlphaMask
    },
  };
  if ((format >= 0) && (format < XCBRenderStandardPICTFORMINFOCount))
    forminfo = XCBRenderFindPICTFORMINFO (conn,
                                          standardFormats[format].mask,
                                          &standardFormats[format].template,
                                          0);
  return forminfo;
}

static void
_tmp_xcb_err(XCBConnection *conn/* , XErrorEvent *ev */)
{
   _xcb_err = 1;
   return;
}

XCBimage_Info *
_xr_image_info_get(XCBConnection *conn, XCBDRAWABLE draw, XCBVISUALID vis)
{
   XCBGetGeometryCookie cookie;
   XCBGetGeometryRep   *rep;
   XCBimage_Info       *xcbinf;
   XCBimage_Info       *xcbinf2;
   Evas_List           *l;

   xcbinf2 = NULL;
   for (l = _image_info_list; l; l = l->next)
     {
	xcbinf = l->data;
	if (xcbinf->conn == conn)
	  {
	     xcbinf2 = xcbinf;
	     break;
	  }
     }
   xcbinf = calloc(1, sizeof(XCBimage_Info));
   if (!xcbinf) return NULL;

   xcbinf->references = 1;
   xcbinf->conn = conn;
   xcbinf->draw = draw;
   cookie = XCBGetGeometry(xcbinf->conn, xcbinf->draw);
   rep = XCBGetGeometryReply(xcbinf->conn, cookie, NULL);
   xcbinf->root.window = rep->root;
   free(rep);
   xcbinf->vis = vis;
   xcbinf->fmt32 = XCBRenderFindStandardPICTFORMINFO(xcbinf->conn, XCBRenderStandardPICTFORMINFOARGB32);
   xcbinf->fmt24 = XCBRenderFindStandardPICTFORMINFO(xcbinf->conn, XCBRenderStandardPICTFORMINFORGB24);
   xcbinf->fmt8 = XCBRenderFindStandardPICTFORMINFO(xcbinf->conn, XCBRenderStandardPICTFORMINFOA8);
   xcbinf->fmt4 = XCBRenderFindStandardPICTFORMINFO(xcbinf->conn, XCBRenderStandardPICTFORMINFOA4);
   xcbinf->fmt1 = XCBRenderFindStandardPICTFORMINFO(xcbinf->conn, XCBRenderStandardPICTFORMINFOA1);
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
        const XCBQueryExtensionRep *rep_shm;
        const XCBQueryExtensionRep *rep_xrender;

        xcbinf->depth = 32;
        {
          XCBConnSetupSuccessRep *rep;
          XCBSCREENIter iter_screen;

          rep = XCBGetSetup(xcbinf->conn);
          iter_screen = XCBConnSetupSuccessRepRootsIter(rep);
          for (; iter_screen.rem ; XCBSCREENNext (&iter_screen)) {
            XCBDEPTHIter iter_depth;

            iter_depth = XCBSCREENAllowedDepthsIter (iter_screen.data);
            for (; iter_depth.rem ; XCBDEPTHNext (&iter_depth)) {
              XCBVISUALTYPEIter iter_visual;

              iter_visual = XCBDEPTHVisualsIter (iter_depth.data);
              for (; iter_visual.rem ; XCBVISUALTYPENext (&iter_visual)) {
                if (iter_visual.data->visual_id.id == vis.id)
                  xcbinf->depth = iter_depth.data->depth;
              }
            }
          }
        }

	xcbinf->can_do_shm = 0;

        XCBPrefetchExtensionData (xcbinf->conn, &XCBShmId);
        XCBPrefetchExtensionData (xcbinf->conn, &XCBRenderId);
        rep_shm = XCBGetExtensionData(xcbinf->conn, &XCBShmId);
        rep_xrender = XCBGetExtensionData(xcbinf->conn, &XCBRenderId);

        if (rep_shm && rep_xrender && rep_shm->present && rep_xrender->present) {
           XCBShmSegmentInfo shm_info;
           XCBImage *xcbim;

           shm_info.shmseg = XCBShmSEGNew(xcbinf->conn);
           xcbim = XCBImageSHMCreate(xcbinf->conn, xcbinf->depth, ZPixmap, NULL, 1, 1);
           if (xcbim) {
              shm_info.shmid = shmget(IPC_PRIVATE, xcbim->bytes_per_line * xcbim->height, IPC_CREAT | 0777);
              if (shm_info.shmid >= 0) {
                 shm_info.shmaddr = xcbim->data = shmat(shm_info.shmid, 0, 0);
                 if ((shm_info.shmaddr != NULL) && (shm_info.shmaddr != (void *) -1)) {
                    /*
                     * FIXME: no error mechanism
                     */
                    /*		       XErrorHandler ph; */

                    XCBSync(xcbinf->conn, 0);
                    _xcb_err = 0;
                    /*		       ph = XSetErrorHandler((XErrorHandler)_tmp_xcb_err); */
                    XCBShmAttach(xcbinf->conn, shm_info.shmseg, shm_info.shmid, 0);
                    XCBSync(xcbinf->conn, 0);
                    /*		       XSetErrorHandler((XErrorHandler)ph); */
                    if (!_xcb_err) xcbinf->can_do_shm = 1;
                    shmdt(shm_info.shmaddr);
                 }
                 shmctl(shm_info.shmid, IPC_RMID, 0);
              }
              XCBImageSHMDestroy(xcbim);
            }
        }
     }
   _image_info_list = evas_list_prepend(_image_info_list, xcbinf);
   return xcbinf;
}

void
_xr_image_info_free(XCBimage_Info *xcbinf)
{
   if (xcbinf->pool) XCBSync(xcbinf->conn, 0);
   _xr_image_info_pool_flush(xcbinf, 0, 0);
   xcbinf->references--;
   if (xcbinf->references != 0) return;
   _xr_render_surface_free(xcbinf->mul);
   free(xcbinf);
   _image_info_list = evas_list_remove(_image_info_list, xcbinf);
}

void
_xr_image_info_pool_flush(XCBimage_Info *xcbinf, int max_num, int max_mem)
{
   if ((xcbinf->pool_mem <= max_mem) && (evas_list_count(xcbinf->pool) <= max_num)) return;
   while ((xcbinf->pool_mem > max_mem) || (evas_list_count(xcbinf->pool) > max_num))
     {
	XCBimage_Image *xcbim;

	if (!xcbinf->pool) break;
	xcbim = xcbinf->pool->data;
	_xr_image_free(xcbim);
     }
}

XCBimage_Image *
_xr_image_new(XCBimage_Info *xcbinf, int w, int h, int depth)
{
   XCBimage_Image *xcbim, *xcbim2;
   Evas_List *l;

   xcbim2 = NULL;
   for (l = xcbinf->pool; l; l = l->next)
     {
	xcbim = l->data;
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
   xcbim = calloc(1, sizeof(XCBimage_Image));
   if (xcbim)
     {
	xcbim->xcbinf = xcbinf;
	xcbim->w = w;
	xcbim->h = h;
	xcbim->depth = depth;
	xcbim->available = 0;
	if (xcbim->xcbinf->can_do_shm)
	  {
	     xcbim->shm_info = calloc(1, sizeof(XCBShmSegmentInfo));
	     if (xcbim->shm_info)
	       {
                  xcbim->shm_info->shmseg = XCBShmSEGNew(xcbinf->conn);
		  xcbim->xcbim = XCBImageSHMCreate(xcbim->xcbinf->conn, xcbim->depth, ZPixmap, NULL, xcbim->w, xcbim->h);
		  if (xcbim->xcbim)
		    {
		       xcbim->shm_info->shmid = shmget(IPC_PRIVATE, xcbim->xcbim->bytes_per_line * xcbim->xcbim->height, IPC_CREAT | 0777);
		       if (xcbim->shm_info->shmid >= 0)
			 {
			    xcbim->shm_info->shmaddr = xcbim->xcbim->data = shmat(xcbim->shm_info->shmid, 0, 0);
			    if ((xcbim->shm_info->shmaddr) && (xcbim->shm_info->shmaddr != (void *) -1))
			      {
                                /*
                                 * FIXME: no error mechanism
                                 */
/*				 XErrorHandler ph; */

				 XCBSync(xcbim->xcbinf->conn, 0);
				 _xcb_err = 0;
/*				 ph = XSetErrorHandler((XErrorHandler)_tmp_xcb_err); */
				 XCBShmAttach(xcbim->xcbinf->conn, xcbim->shm_info->shmseg, xcbim->shm_info->shmid, 0);
				 XCBSync(xcbim->xcbinf->conn, 0);
/*				 XSetErrorHandler((XErrorHandler)ph); */
				 if (!_xcb_err) goto xcbim_ok;
				 shmdt(xcbim->shm_info->shmaddr);
			      }
			    shmctl(xcbim->shm_info->shmid, IPC_RMID, 0);
			 }
		       XCBImageSHMDestroy(xcbim->xcbim);
		    }
		  free(xcbim->shm_info);
		  xcbim->shm_info = NULL;
	       }
	  }
	xcbim->xcbim = XCBImageCreate(xcbim->xcbinf->conn, xcbim->depth, ZPixmap, 0, NULL, xcbim->w, xcbim->h, 32, 0);
	if (!xcbim->xcbim)
	  {
	     free(xcbim);
	     return NULL;
	  }
	xcbim->xcbim->data = malloc(xcbim->xcbim->bytes_per_line * xcbim->xcbim->height);
	if (!xcbim->xcbim->data)
	  {
	     XCBImageDestroy(xcbim->xcbim);
	     free(xcbim);
	     return NULL;
	  }
     }

   xcbim_ok:
   _xr_image_info_pool_flush(xcbinf, 32, (1600 * 1200 * 32 * 2));

   xcbim->line_bytes = xcbim->xcbim->bytes_per_line;
   xcbim->data = (void *)(xcbim->xcbim->data);
   xcbinf->pool_mem += (xcbim->w * xcbim->h * xcbim->depth);
   xcbinf->pool = evas_list_append(xcbinf->pool, xcbim);
   return xcbim;
}

void
_xr_image_free(XCBimage_Image *xcbim)
{
   if (xcbim->shm_info)
     {
	if (!xcbim->available) XCBSync(xcbim->xcbinf->conn, 0);
	XCBShmDetach(xcbim->xcbinf->conn, xcbim->shm_info->shmseg);
	XCBImageSHMDestroy(xcbim->xcbim);
	shmdt(xcbim->shm_info->shmaddr);
	shmctl(xcbim->shm_info->shmid, IPC_RMID, 0);
	free(xcbim->shm_info);
     }
   else
     {
	free(xcbim->xcbim->data);
	xcbim->xcbim->data = NULL;
	XCBImageDestroy(xcbim->xcbim);
     }
   xcbim->xcbinf->pool_mem -= (xcbim->w * xcbim->h * xcbim->depth);
   xcbim->xcbinf->pool = evas_list_remove(xcbim->xcbinf->pool, xcbim);
   free(xcbim);
}

void
_xr_image_put(XCBimage_Image *xcbim, XCBDRAWABLE draw, int x, int y, int w, int h)
{
   XCBGCONTEXT gc;

   gc = XCBGCONTEXTNew(xcbim->xcbinf->conn);
   XCBCreateGC(xcbim->xcbinf->conn, gc, draw, 0, NULL);
   if (xcbim->shm_info)
     {
       XCBImageSHMPut (xcbim->xcbinf->conn, draw, gc,
                       xcbim->xcbim, *xcbim->shm_info,
                       0, 0,
                       x, y,
                       xcbim->xcbim->width, xcbim->xcbim->height,
                       0);
/* 	XCBShmPutImage(xcbim->xcbinf->conn, draw, gc, */
/*                        xcbim->xcbim->width, xcbim->xcbim->height, */
/*                        0, 0, */
/*                        w, h, */
/*                        x, y, */
/*                        xcbim->xcbim->depth, xcbim->xcbim->format, */
/*                        0, */
/*                        xcbim->shm_info->shmseg, */
/*                        xcbim->xcbim->data - xcbim->shm_info->shmaddr); */
	XCBSync(xcbim->xcbinf->conn, 0);
     }
   else
     XCBImagePut(xcbim->xcbinf->conn, draw, gc, xcbim->xcbim, 0, 0, x, y, w, h);
   xcbim->available = 1;
   XCBFreeGC(xcbim->xcbinf->conn, gc);
}
