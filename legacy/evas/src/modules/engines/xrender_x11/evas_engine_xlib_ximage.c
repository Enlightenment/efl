#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_XRender_X11.h"

static Eina_List *_image_info_list = NULL;

static int _x_err = 0;
static void
_tmp_x_err(Display *d __UNUSED__, XErrorEvent *ev __UNUSED__)
{
   _x_err = 1;
   return;
}

Ximage_Info *
_xr_xlib_image_info_get(Display *display, Drawable draw, Visual *visual)
{
   Ximage_Info *xinf;
   Ximage_Info *xinf2;
   Eina_List   *l;
   int          di;
   unsigned int dui;
   Window       root;

   xinf2 = NULL;
   EINA_LIST_FOREACH(_image_info_list, l, xinf)
     if (xinf->x11.connection == display)
       {
	 xinf2 = xinf;
	 break;
       }
   xinf = calloc(1, sizeof(Ximage_Info));
   if (!xinf) return NULL;

   xinf->references = 1;
   xinf->x11.connection = display;
   xinf->x11.draw = draw;
   XGetGeometry(xinf->x11.connection, xinf->x11.draw,
                &root,
		&di, &di, &dui, &dui, &dui, &dui);
   xinf->x11.root = root;
   xinf->x11.visual = visual;
   xinf->x11.fmt32 = XRenderFindStandardFormat(xinf->x11.connection, PictStandardARGB32);
   xinf->x11.fmt24 = XRenderFindStandardFormat(xinf->x11.connection, PictStandardRGB24);
   xinf->x11.fmt8 = XRenderFindStandardFormat(xinf->x11.connection, PictStandardA8);
   xinf->x11.fmt4 = XRenderFindStandardFormat(xinf->x11.connection, PictStandardA4);
   xinf->x11.fmt1 = XRenderFindStandardFormat(xinf->x11.connection, PictStandardA1);

   /* find fmt for default visual */
   xinf->x11.fmtdef = XRenderFindVisualFormat(xinf->x11.connection, xinf->x11.visual);

   xinf->mul = _xr_xlib_render_surface_new(xinf, 1, 1, xinf->x11.fmt32, 1);
   _xr_xlib_render_surface_repeat_set(xinf->mul, 1);
   xinf->mul_r = xinf->mul_g = xinf->mul_b = xinf->mul_a = 0xff;
   _xr_xlib_render_surface_solid_rectangle_set(xinf->mul, xinf->mul_r, xinf->mul_g, xinf->mul_b, xinf->mul_a, 0, 0, 1, 1);
   if (xinf2)
     {
	xinf->can_do_shm = xinf2->can_do_shm;
	xinf->depth = xinf2->depth;
     }
   else
     {
	XVisualInfo *vi, vit;
	XShmSegmentInfo shm_info;
	XImage *xim;
	int num = 0;

	vit.visualid = XVisualIDFromVisual(xinf->x11.visual);
	vi = XGetVisualInfo(xinf->x11.connection, VisualIDMask, &vit, &num);
	if (!vi) xinf->depth = 32;
	else
	  {
	     xinf->depth = vi->depth;
	     XFree(vi);
	  }
	xinf->can_do_shm = 0;
	xim = XShmCreateImage(xinf->x11.connection, xinf->x11.visual, xinf->depth, ZPixmap, NULL, &shm_info, 1, 1);
	if (xim)
	  {
	     shm_info.shmid = shmget(IPC_PRIVATE, xim->bytes_per_line * xim->height, IPC_CREAT | 0777);
	     if (shm_info.shmid >= 0)
	       {
		  shm_info.shmaddr = xim->data = shmat(shm_info.shmid, 0, 0);
		  if ((shm_info.shmaddr) && (shm_info.shmaddr != (void *) -1))
		    {
		       XErrorHandler ph;

		       XSync(xinf->x11.connection, False);
		       _x_err = 0;
		       ph = XSetErrorHandler((XErrorHandler)_tmp_x_err);
		       XShmAttach(xinf->x11.connection, &shm_info);
		       XSync(xinf->x11.connection, False);
		       XSetErrorHandler((XErrorHandler)ph);
		       if (!_x_err) xinf->can_do_shm = 1;
		       shmdt(shm_info.shmaddr);
		    }
		  shmctl(shm_info.shmid, IPC_RMID, 0);
	       }
	     XDestroyImage(xim);
	  }
     }
   _image_info_list = eina_list_prepend(_image_info_list, xinf);
   return xinf;
}

void
_xr_xlib_image_info_free(Ximage_Info *xinf)
{
   if (xinf->pool) XSync(xinf->x11.connection, False);
   _xr_xlib_image_info_pool_flush(xinf, 0, 0);
   xinf->references--;
   if (xinf->references != 0) return;
   _xr_xlib_render_surface_free(xinf->mul);
   free(xinf);
   _image_info_list = eina_list_remove(_image_info_list, xinf);
}

void
_xr_xlib_image_info_pool_flush(Ximage_Info *xinf, int max_num, int max_mem)
{
   if ((xinf->pool_mem <= max_mem) && (eina_list_count(xinf->pool) <= max_num)) return;
   while ((xinf->pool_mem > max_mem) || (eina_list_count(xinf->pool) > max_num))
     {
	Ximage_Image *xim;

	if (!xinf->pool) break;
	xim = xinf->pool->data;
	_xr_xlib_image_free(xim);
     }
}

Ximage_Image *
_xr_xlib_image_new(Ximage_Info *xinf, int w, int h, int depth)
{
   Ximage_Image *xim, *xim2;
   Eina_List *l;

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
	     xim->x11.xlib.shm_info = calloc(1, sizeof(XShmSegmentInfo));
	     if (xim->x11.xlib.shm_info)
	       {
		  xim->x11.xlib.xim = XShmCreateImage(xim->xinf->x11.connection, xim->xinf->x11.visual, xim->depth, ZPixmap, NULL, xim->x11.xlib.shm_info, xim->width, xim->height);
		  if (xim->x11.xlib.xim)
		    {
		       xim->x11.xlib.shm_info->shmid = shmget(IPC_PRIVATE, xim->x11.xlib.xim->bytes_per_line * xim->x11.xlib.xim->height, IPC_CREAT | 0777);
		       if (xim->x11.xlib.shm_info->shmid >= 0)
			 {
			    xim->x11.xlib.shm_info->shmaddr = xim->x11.xlib.xim->data = shmat(xim->x11.xlib.shm_info->shmid, 0, 0);
			    if ((xim->x11.xlib.shm_info->shmaddr) && (xim->x11.xlib.shm_info->shmaddr != (void *) -1))
			      {
				 XErrorHandler ph;

				 XSync(xim->xinf->x11.connection, False);
				 _x_err = 0;
				 ph = XSetErrorHandler((XErrorHandler)_tmp_x_err);
				 XShmAttach(xim->xinf->x11.connection, xim->x11.xlib.shm_info);
				 XSync(xim->xinf->x11.connection, False);
				 XSetErrorHandler((XErrorHandler)ph);
				 if (!_x_err) goto xim_ok;
				 shmdt(xim->x11.xlib.shm_info->shmaddr);
			      }
			    shmctl(xim->x11.xlib.shm_info->shmid, IPC_RMID, 0);
			 }
		       XDestroyImage(xim->x11.xlib.xim);
		    }
		  free(xim->x11.xlib.shm_info);
		  xim->x11.xlib.shm_info = NULL;
	       }
	  }
	xim->x11.xlib.xim = XCreateImage(xim->xinf->x11.connection, xim->xinf->x11.visual, xim->depth, ZPixmap, 0, NULL, xim->width, xim->height, 32, 0);
	if (!xim->x11.xlib.xim)
	  {
	     free(xim);
	     return NULL;
	  }
	xim->x11.xlib.xim->data = malloc(xim->x11.xlib.xim->bytes_per_line * xim->x11.xlib.xim->height);
	if (!xim->x11.xlib.xim->data)
	  {
	     XDestroyImage(xim->x11.xlib.xim);
	     free(xim);
	     return NULL;
	  }
     }

   xim_ok:
   _xr_xlib_image_info_pool_flush(xinf, 32, (1600 * 1200 * 32 * 2));

   xim->line_bytes = xim->x11.xlib.xim->bytes_per_line;
   xim->data = (void *)(xim->x11.xlib.xim->data);
   xinf->pool_mem += (xim->width * xim->height * xim->depth);
   xinf->pool = eina_list_append(xinf->pool, xim);
   return xim;
}

void
_xr_xlib_image_free(Ximage_Image *xim)
{
   if (xim->x11.xlib.shm_info)
     {
	if (!xim->available) XSync(xim->xinf->x11.connection, False);
	XShmDetach(xim->xinf->x11.connection, xim->x11.xlib.shm_info);
	XDestroyImage(xim->x11.xlib.xim);
	shmdt(xim->x11.xlib.shm_info->shmaddr);
	shmctl(xim->x11.xlib.shm_info->shmid, IPC_RMID, 0);
	free(xim->x11.xlib.shm_info);
     }
   else
     {
	free(xim->x11.xlib.xim->data);
	xim->x11.xlib.xim->data = NULL;
	XDestroyImage(xim->x11.xlib.xim);
     }
   xim->xinf->pool_mem -= (xim->width * xim->height * xim->depth);
   xim->xinf->pool = eina_list_remove(xim->xinf->pool, xim);
   free(xim);
}

void
_xr_xlib_image_put(Ximage_Image *xim, Drawable draw, int x, int y, int w, int h)
{
   XGCValues gcv;
   GC gc;

   gc = XCreateGC(xim->xinf->x11.connection, draw, 0, &gcv);
   if (xim->x11.xlib.shm_info)
     {
	XShmPutImage(xim->xinf->x11.connection, draw, gc, xim->x11.xlib.xim, 0, 0, x, y, w, h, False);
	XSync(xim->xinf->x11.connection, False);
     }
   else
     XPutImage(xim->xinf->x11.connection, draw, gc, xim->x11.xlib.xim, 0, 0, x, y, w, h);
   xim->available = 1;
   XFreeGC(xim->xinf->x11.connection, gc);
}
