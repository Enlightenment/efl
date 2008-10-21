#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_XRender_X11.h"

static Eina_List *_image_info_list = NULL;

static int _x_err = 0;
static void
_tmp_x_err(Display *d, XErrorEvent *ev)
{
   _x_err = 1;
   return;
}

Ximage_Info *
_xr_image_info_get(Display *disp, Drawable draw, Visual *vis)
{
   Ximage_Info *xinf, *xinf2;
   Eina_List *l;
   int di;
   unsigned int dui;
   
   xinf2 = NULL;
   EINA_LIST_FOREACH(_image_info_list, l, xinf)
     if (xinf->disp == disp)
       {
	 xinf2 = xinf;
	 break;
       }
   xinf = calloc(1, sizeof(Ximage_Info));
   if (!xinf) return NULL;
   
   xinf->references = 1;
   xinf->disp = disp;
   xinf->draw = draw;
   XGetGeometry(xinf->disp, xinf->draw, &(xinf->root),
		&di, &di, &dui, &dui, &dui, &dui);
   xinf->vis = vis;
   xinf->fmt32 = XRenderFindStandardFormat(xinf->disp, PictStandardARGB32);
   xinf->fmt24 = XRenderFindStandardFormat(xinf->disp, PictStandardRGB24);
   xinf->fmt8 = XRenderFindStandardFormat(xinf->disp, PictStandardA8);
   xinf->fmt4 = XRenderFindStandardFormat(xinf->disp, PictStandardA4);
   xinf->fmt1 = XRenderFindStandardFormat(xinf->disp, PictStandardA1);
   
   /* find fmt for default visual */
   xinf->fmtdef = XRenderFindVisualFormat(xinf->disp, xinf->vis);
   
   xinf->mul = _xr_render_surface_new(xinf, 1, 1, xinf->fmt32, 1);
   _xr_render_surface_repeat_set(xinf->mul, 1);
   xinf->mul_r = xinf->mul_g = xinf->mul_b = xinf->mul_a = 0xff;
   _xr_render_surface_solid_rectangle_set(xinf->mul, xinf->mul_r, xinf->mul_g, xinf->mul_b, xinf->mul_a, 0, 0, 1, 1);
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
	
	vit.visualid = XVisualIDFromVisual(xinf->vis);
	vi = XGetVisualInfo(xinf->disp, VisualIDMask, &vit, &num);
	if (!vi) xinf->depth = 32;
	else
	  {
	     xinf->depth = vi->depth;
	     XFree(vi);
	  }
	xinf->can_do_shm = 0;
	xim = XShmCreateImage(xinf->disp, xinf->vis, xinf->depth, ZPixmap, NULL, &shm_info, 1, 1);
	if (xim)
	  {
	     shm_info.shmid = shmget(IPC_PRIVATE, xim->bytes_per_line * xim->height, IPC_CREAT | 0777);
	     if (shm_info.shmid >= 0)
	       {
		  shm_info.shmaddr = xim->data = shmat(shm_info.shmid, 0, 0);
		  if ((shm_info.shmaddr) && (shm_info.shmaddr != (void *) -1))
		    {
		       XErrorHandler ph;
		       
		       XSync(xinf->disp, False);
		       _x_err = 0;
		       ph = XSetErrorHandler((XErrorHandler)_tmp_x_err);
		       XShmAttach(xinf->disp, &shm_info);
		       XSync(xinf->disp, False);
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
_xr_image_info_free(Ximage_Info *xinf)
{
   if (xinf->pool) XSync(xinf->disp, False);
   _xr_image_info_pool_flush(xinf, 0, 0);
   xinf->references--;
   if (xinf->references != 0) return;
   _xr_render_surface_free(xinf->mul);
   free(xinf);
   _image_info_list = eina_list_remove(_image_info_list, xinf);
}

void
_xr_image_info_pool_flush(Ximage_Info *xinf, int max_num, int max_mem)
{
   if ((xinf->pool_mem <= max_mem) && (eina_list_count(xinf->pool) <= max_num)) return;
   while ((xinf->pool_mem > max_mem) || (eina_list_count(xinf->pool) > max_num))
     {
	Ximage_Image *xim;
	
	if (!xinf->pool) break;
	xim = xinf->pool->data;
	_xr_image_free(xim);
     }
}

Ximage_Image *
_xr_image_new(Ximage_Info *xinf, int w, int h, int depth)
{
   Ximage_Image *xim, *xim2;
   Eina_List *l;

   xim2 = NULL;
   EINA_LIST_FOREACH(xinf->pool, l, xim)
     {
	if ((xim->w >= w) && (xim->h >= h) && (xim->depth == depth) && (xim->available))
	  {
	     if (!xim2) xim2 = xim;
	     else if ((xim->w * xim->h) < (xim2->w * xim2->h)) xim2 = xim;
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
	xim->w = w;
	xim->h = h;
	xim->depth = depth;
	xim->available = 0;
	if (xim->xinf->can_do_shm)
	  {
	     xim->shm_info = calloc(1, sizeof(XShmSegmentInfo));
	     if (xim->shm_info)
	       {
		  xim->xim = XShmCreateImage(xim->xinf->disp, xim->xinf->vis, xim->depth, ZPixmap, NULL, xim->shm_info, xim->w, xim->h);
		  if (xim->xim)
		    {
		       xim->shm_info->shmid = shmget(IPC_PRIVATE, xim->xim->bytes_per_line * xim->xim->height, IPC_CREAT | 0777);
		       if (xim->shm_info->shmid >= 0)
			 {
			    xim->shm_info->shmaddr = xim->xim->data = shmat(xim->shm_info->shmid, 0, 0);
			    if ((xim->shm_info->shmaddr) && (xim->shm_info->shmaddr != (void *) -1))
			      {
				 XErrorHandler ph;
				 
				 XSync(xim->xinf->disp, False);
				 _x_err = 0;
				 ph = XSetErrorHandler((XErrorHandler)_tmp_x_err);
				 XShmAttach(xim->xinf->disp, xim->shm_info);
				 XSync(xim->xinf->disp, False);
				 XSetErrorHandler((XErrorHandler)ph);
				 if (!_x_err) goto xim_ok;
				 shmdt(xim->shm_info->shmaddr);
			      }
			    shmctl(xim->shm_info->shmid, IPC_RMID, 0);
			 }
		       XDestroyImage(xim->xim);
		    }
		  free(xim->shm_info);
		  xim->shm_info = NULL;
	       }
	  }
	xim->xim = XCreateImage(xim->xinf->disp, xim->xinf->vis, xim->depth, ZPixmap, 0, NULL, xim->w, xim->h, 32, 0);
	if (!xim->xim)
	  {
	     free(xim);
	     return NULL;
	  }
	xim->xim->data = malloc(xim->xim->bytes_per_line * xim->xim->height);
	if (!xim->xim->data)
	  {
	     XDestroyImage(xim->xim);
	     free(xim);
	     return NULL;
	  }
     }
   
   xim_ok:
   _xr_image_info_pool_flush(xinf, 32, (1600 * 1200 * 32 * 2));
   
   xim->line_bytes = xim->xim->bytes_per_line;
   xim->data = (void *)(xim->xim->data);
   xinf->pool_mem += (xim->w * xim->h * xim->depth);
   xinf->pool = eina_list_append(xinf->pool, xim);
   return xim;
}

void
_xr_image_free(Ximage_Image *xim)
{
   if (xim->shm_info)
     {
	if (!xim->available) XSync(xim->xinf->disp, False);
	XShmDetach(xim->xinf->disp, xim->shm_info);
	XDestroyImage(xim->xim);
	shmdt(xim->shm_info->shmaddr);
	shmctl(xim->shm_info->shmid, IPC_RMID, 0);
	free(xim->shm_info);
     }
   else
     {
	free(xim->xim->data);
	xim->xim->data = NULL;
	XDestroyImage(xim->xim);
     }
   xim->xinf->pool_mem -= (xim->w * xim->h * xim->depth);
   xim->xinf->pool = eina_list_remove(xim->xinf->pool, xim);
   free(xim);
}

void
_xr_image_put(Ximage_Image *xim, Drawable draw, int x, int y, int w, int h)
{
   XGCValues gcv;
   GC gc;
   
   gc = XCreateGC(xim->xinf->disp, draw, 0, &gcv);
   if (xim->shm_info)
     {
	XShmPutImage(xim->xinf->disp, draw, gc, xim->xim, 0, 0, x, y, w, h, False);
	XSync(xim->xinf->disp, False);
     }
   else
     XPutImage(xim->xinf->disp, draw, gc, xim->xim, 0, 0, x, y, w, h);
   xim->available = 1;
   XFreeGC(xim->xinf->disp, gc);
}
