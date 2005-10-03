#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "evas_engine_api_xrender_x11.h"
#include "Evas_Engine_XRender_X11.h"
#include <math.h>

XR_Gradient *
_xre_gradient_color_add(Ximage_Info *xinf, XR_Gradient *gr, int r, int g, int b, int a, int distance)
{
   if (!gr)
     {
	gr = calloc(1, sizeof(XR_Gradient));
	if (!gr) return NULL;
	gr->xinf = xinf;
	gr->xinf->references++;
	gr->grad = evas_common_gradient_new();
	if (!gr->grad)
	  {
	     gr->xinf->references--;
	     free(gr);
	     return NULL;
	  }
     }
   evas_common_gradient_color_add(gr->grad, r, g, b, a, distance);
   if (gr->surface)
     {
	_xr_render_surface_free(gr->surface);
	gr->surface = NULL;
     }
   return gr;
}

XR_Gradient *
_xre_gradient_colors_clear(XR_Gradient *gr)
{
   if (!gr) return NULL;
   if (gr->grad)
     {
	evas_common_gradient_free(gr->grad);
	gr->grad = NULL;
     }
   if (gr->surface)
     {
	_xr_render_surface_free(gr->surface);
	gr->surface = NULL;
     }
   _xr_image_info_free(gr->xinf);
   free(gr);
   return NULL;
}

void
_xre_gradient_draw(Xrender_Surface *rs, RGBA_Draw_Context *dc, XR_Gradient *gr, int x, int y, int w, int h, double angle)
{
   XTransform xf;
   XRectangle rect;
   XRenderPictureAttributes att;
   Picture mask;
   int r, g, b, a, op;
   RGBA_Image *im;
   double a2;
   
   if ((w <= 0) || (h <= 0)) return;
   
   if (angle != gr->angle)
     {
	if (gr->surface)
	  {
	     _xr_render_surface_free(gr->surface);
	     gr->surface = NULL;
	  }
     }
   if (!gr->surface)
     {
	im = evas_common_image_create(256, 256);
	if (im)
	  {
	     RGBA_Draw_Context *dc2;
	     
	     dc2 = evas_common_draw_context_new();
	     if (dc2)
	       {
		  im->flags |= RGBA_IMAGE_HAS_ALPHA;
		  memset(im->image->data, 0, im->image->w * im->image->h * sizeof(DATA32));
		  evas_common_gradient_draw(im, dc2, 0, 0, 256, 256, gr->grad, angle);
		  gr->surface = _xr_render_surface_new(gr->xinf, 256, 256, gr->xinf->fmt32, 1);
		  if (gr->surface)
		    _xr_render_surface_argb_pixels_fill(gr->surface, 256, 256, im->image->data, 0, 0, 256, 256);
		  evas_common_draw_context_free(dc2);
		  gr->angle = angle;
	       }
	     evas_common_image_free(im);
	  }
     }
   if (gr->surface)
     _xr_render_surface_composite(gr->surface, rs, dc, 0, 0, gr->surface->w, gr->surface->h, x, y, w, h, 1);
}
