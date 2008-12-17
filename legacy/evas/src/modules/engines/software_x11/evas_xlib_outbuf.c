#include <sys/time.h>
#include <sys/utsname.h>

#include "evas_common.h"
#include "evas_macros.h"
#include "evas_xlib_outbuf.h"
#include "evas_xlib_buffer.h"
#include "evas_xlib_color.h"


typedef struct _Outbuf_Region   Outbuf_Region;

struct _Outbuf_Region
{
   X_Output_Buffer *xob;
   X_Output_Buffer *mxob;
   int              x;
   int              y;
   int              w;
   int              h;
};

static Eina_List *shmpool = NULL;
static int shmsize = 0;
static int shmmemlimit = 10 * 1024 * 1024;
static int shmcountlimit = 32;

static X_Output_Buffer *
_find_xob(Display *d, Visual *v, int depth, int w, int h, int shm, void *data)
{
   Eina_List *l, *xl;
   X_Output_Buffer *xob = NULL;
   X_Output_Buffer *xob2;
   int fitness = 0x7fffffff;
   int sz, lbytes, bpp;

//   return evas_software_xlib_x_output_buffer_new(d, v, depth, w, h, shm, data);
   if (!shm)
     return evas_software_xlib_x_output_buffer_new(d, v, depth, w, h, shm, data);
   if (depth > 1)
     {
	bpp = depth / 8;
	if (bpp == 3) bpp = 4;
	lbytes = (((w * bpp) + 3) / 4) * 4;
     }
   else
     lbytes = ((w + 31) / 32) * 4;
   sz = lbytes * h;
   EINA_LIST_FOREACH(shmpool, l, xob2)
     {
	int szdif;

	if ((xob2->xim->depth != depth) || (xob2->visual != v) ||
	    (xob2->display != d))
	  continue;
	szdif = xob2->psize - sz;
	if (szdif < 0) continue;
	if (szdif == 0)
	  {
	     xob = xob2;
	     xl = l;
	     goto have_xob;
	  }
	if (szdif < fitness)
	  {
	     fitness = szdif;
	     xob = xob2;
	     xl = l;
	  }
     }
   if ((fitness > (100 * 100)) || (!xob))
     return evas_software_xlib_x_output_buffer_new(d, v, depth, w, h, shm, data);

   have_xob:
   shmpool = eina_list_remove_list(shmpool, xl);
   xob->w = w;
   xob->h = h;
   xob->bpl = lbytes;
   xob->xim->width = xob->w;
   xob->xim->height = xob->h;
   xob->xim->bytes_per_line = xob->bpl;
   shmsize -= xob->psize * (xob->xim->depth / 8);
   return xob;
}

static void
_unfind_xob(X_Output_Buffer *xob, int sync)
{
//   evas_software_xlib_x_output_buffer_free(xob, sync); return;
   if (xob->shm_info)
     {
	shmpool = eina_list_prepend(shmpool, xob);
	shmsize += xob->psize * xob->xim->depth / 8;
	while ((shmsize > (shmmemlimit)) ||
	       (eina_list_count(shmpool) > shmcountlimit))
	  {
	     Eina_List *xl;

	     xl = eina_list_last(shmpool);
	     if (!xl)
	       {
		  shmsize = 0;
		  break;
	       }
	     xob = xl->data;
	     shmpool = eina_list_remove_list(shmpool, xl);
	     evas_software_xlib_x_output_buffer_free(xob, sync);
	  }
     }
   else
     evas_software_xlib_x_output_buffer_free(xob, sync);
}

static void
_clear_xob(int sync)
{
   while (shmpool)
     {
	X_Output_Buffer *xob;

	xob = shmpool->data;
	shmpool = eina_list_remove_list(shmpool, shmpool);
	evas_software_xlib_x_output_buffer_free(xob, sync);
     }
   shmsize = 0;
}

void
evas_software_xlib_outbuf_init(void)
{
}

void
evas_software_xlib_outbuf_free(Outbuf *buf)
{
   while (buf->priv.pending_writes)
     {
	RGBA_Image *im;
	Outbuf_Region *obr;

	im = buf->priv.pending_writes->data;
	buf->priv.pending_writes = eina_list_remove_list(buf->priv.pending_writes, buf->priv.pending_writes);
	obr = im->extended_info;
	evas_cache_image_drop(&im->cache_entry);
	if (obr->xob) _unfind_xob(obr->xob, 0);
	if (obr->mxob) _unfind_xob(obr->mxob, 0);
	free(obr);
     }
   evas_software_xlib_outbuf_idle_flush(buf);
   evas_software_xlib_outbuf_flush(buf);
   if (buf->priv.x11.xlib.gc)
      XFreeGC(buf->priv.x11.xlib.disp, buf->priv.x11.xlib.gc);
   if (buf->priv.x11.xlib.gcm)
      XFreeGC(buf->priv.x11.xlib.disp, buf->priv.x11.xlib.gcm);
   if (buf->priv.pal)
      evas_software_xlib_x_color_deallocate(buf->priv.x11.xlib.disp, buf->priv.x11.xlib.cmap,
					   buf->priv.x11.xlib.vis, buf->priv.pal);
   free(buf);
   _clear_xob(0);
}

Outbuf *
evas_software_xlib_outbuf_setup_x(int w, int h, int rot, Outbuf_Depth depth,
				 Display *disp, Drawable draw, Visual *vis,
				 Colormap cmap, int x_depth,
				 int grayscale, int max_colors, Pixmap mask,
				 int shape_dither, int destination_alpha)
{
   Outbuf             *buf;

   buf = calloc(1, sizeof(Outbuf));
   if (!buf)
      return NULL;

   buf->w = w;
   buf->h = h;
   buf->depth = depth;
   buf->rot = rot;

   buf->priv.x11.xlib.disp = disp;
   buf->priv.x11.xlib.vis = vis;
   buf->priv.x11.xlib.cmap = cmap;
   buf->priv.x11.xlib.depth = x_depth;

   buf->priv.mask_dither = shape_dither;
   buf->priv.destination_alpha = destination_alpha;

   {
      Gfx_Func_Convert    conv_func;
      X_Output_Buffer    *xob;

      buf->priv.x11.xlib.shm = evas_software_xlib_x_can_do_shm(buf->priv.x11.xlib.disp);
      xob = evas_software_xlib_x_output_buffer_new(buf->priv.x11.xlib.disp,
						  buf->priv.x11.xlib.vis,
						  buf->priv.x11.xlib.depth,
						  1, 1, buf->priv.x11.xlib.shm, NULL);

      conv_func = NULL;
      if (xob)
	{
#ifdef WORDS_BIGENDIAN
	   if (evas_software_xlib_x_output_buffer_byte_order(xob) == LSBFirst)
	     buf->priv.x11.xlib.swap = 1;
	   if (evas_software_xlib_x_output_buffer_bit_order(xob) == MSBFirst)
	     buf->priv.x11.xlib.bit_swap = 1;
#else
	   if (evas_software_xlib_x_output_buffer_byte_order(xob) == MSBFirst)
	     buf->priv.x11.xlib.swap = 1;
	   if (evas_software_xlib_x_output_buffer_bit_order(xob) == MSBFirst)
	     buf->priv.x11.xlib.bit_swap = 1;
#endif
	   if (((vis->class == TrueColor) || (vis->class == DirectColor)) &&
	       (x_depth > 8))
	     {
		buf->priv.mask.r = (DATA32) vis->red_mask;
		buf->priv.mask.g = (DATA32) vis->green_mask;
		buf->priv.mask.b = (DATA32) vis->blue_mask;
		if (buf->priv.x11.xlib.swap)
		  {
		     SWAP32(buf->priv.mask.r);
		     SWAP32(buf->priv.mask.g);
		     SWAP32(buf->priv.mask.b);
		  }
	     }
	   else if ((vis->class == PseudoColor) ||
		    (vis->class == StaticColor) ||
		    (vis->class == GrayScale) ||
		    (vis->class == StaticGray) ||
		    (x_depth <= 8))
	     {
		Convert_Pal_Mode    pm = PAL_MODE_RGB332;

		if ((vis->class == GrayScale) || (vis->class == StaticGray))
		   grayscale = 1;
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
		/* FIXME: only alloc once per display+cmap */
		buf->priv.pal = evas_software_xlib_x_color_allocate(disp, cmap, vis,
								   PAL_MODE_RGB666);
		if (!buf->priv.pal)
		  {
		     free(buf);
		     return NULL;
		  }
	     }
	   if (buf->priv.pal)
	     {
		if (buf->rot == 0 || buf->rot == 180)
		  conv_func = evas_common_convert_func_get(0, buf->w, buf->h,
							   evas_software_xlib_x_output_buffer_depth
							   (xob), buf->priv.mask.r,
							   buf->priv.mask.g,
							   buf->priv.mask.b,
							   buf->priv.pal->colors,
							   buf->rot);
		else if (buf->rot == 90 || buf->rot == 270)
		  conv_func = evas_common_convert_func_get(0, buf->h, buf->w,
							   evas_software_xlib_x_output_buffer_depth
							   (xob), buf->priv.mask.r,
							   buf->priv.mask.g,
							   buf->priv.mask.b,
							   buf->priv.pal->colors,
							   buf->rot);
	     }
	   else
	     {
		if (buf->rot == 0 || buf->rot == 180)
		  conv_func = evas_common_convert_func_get(0, buf->w, buf->h,
							   evas_software_xlib_x_output_buffer_depth
							   (xob), buf->priv.mask.r,
							   buf->priv.mask.g,
						buf->priv.mask.b, PAL_MODE_NONE,
							   buf->rot);
		else if (buf->rot == 90 || buf->rot == 270)
		  conv_func = evas_common_convert_func_get(0, buf->h, buf->w,
							   evas_software_xlib_x_output_buffer_depth
							   (xob), buf->priv.mask.r,
							   buf->priv.mask.g,
							   buf->priv.mask.b, PAL_MODE_NONE,
							   buf->rot);
	     }
	   evas_software_xlib_x_output_buffer_free(xob, 1);
	   if (!conv_func)
	     {
		printf(".[ Evas Error ].\n"
		       " {\n"
		       "  At depth         %i:\n"
		       "  RGB format mask: %08x, %08x, %08x\n"
		       "  Palette mode:    %i\n"
		       "  Not supported by compiled in converters!\n"
		       " }\n",
		       buf->priv.x11.xlib.depth,
		       buf->priv.mask.r,
		       buf->priv.mask.g,
		       buf->priv.mask.b, buf->priv.pal->colors);
	     }
	}
      evas_software_xlib_outbuf_drawable_set(buf, draw);
      evas_software_xlib_outbuf_mask_set(buf, mask);
   }
   return buf;
}

RGBA_Image *
evas_software_xlib_outbuf_new_region_for_update(Outbuf *buf, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch)
{
   RGBA_Image         *im;
   Outbuf_Region      *obr;
   int                 bpl = 0;
   int                 use_shm = 1;
   int                 alpha;

   if ((buf->onebuf) && (buf->priv.x11.xlib.shm))
     {
	Evas_Rectangle *rect;

	rect = malloc(sizeof(Evas_Rectangle));
	RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, buf->w, buf->h);
	rect->x = x;
	rect->y = y;
	rect->w = w;
	rect->h = h;
	buf->priv.onebuf_regions = eina_list_append(buf->priv.onebuf_regions, rect);
	if (buf->priv.onebuf)
	  {
	     *cx = x;
	     *cy = y;
	     *cw = w;
	     *ch = h;
	     if (!buf->priv.synced)
	       {
		  XSync(buf->priv.x11.xlib.disp, False);
		  buf->priv.synced = 1;
	       }
	     if ((buf->priv.x11.xlib.mask) || (buf->priv.destination_alpha))
	       {
		  int yy;

		  im = buf->priv.onebuf;
		  for (yy = y; yy < (y + h); yy++)
		    {
		       memset(im->image.data + (im->cache_entry.w * yy) + x,
			      0, w * sizeof(DATA32));
		    }
	       }
	     return buf->priv.onebuf;
	  }
	obr = calloc(1, sizeof(Outbuf_Region));
	obr->x = 0;
	obr->y = 0;
	obr->w = buf->w;
	obr->h = buf->h;
	*cx = x;
	*cy = y;
	*cw = w;
	*ch = h;

        alpha = ((buf->priv.x11.xlib.mask) || (buf->priv.destination_alpha));

	use_shm = buf->priv.x11.xlib.shm;
	if ((buf->rot == 0) &&
	    (buf->priv.mask.r == 0xff0000) &&
	    (buf->priv.mask.g == 0x00ff00) &&
	    (buf->priv.mask.b == 0x0000ff))
	  {
	     obr->xob = evas_software_xlib_x_output_buffer_new(buf->priv.x11.xlib.disp,
							      buf->priv.x11.xlib.vis,
							      buf->priv.x11.xlib.depth,
							      buf->w, buf->h,
							      use_shm,
							      NULL);
             im = (RGBA_Image *) evas_cache_image_data(evas_common_image_cache_get(),
                                                       buf->w, buf->h,
                                                       (DATA32 *) evas_software_xlib_x_output_buffer_data(obr->xob, &bpl),
                                                       alpha, EVAS_COLORSPACE_ARGB8888);
	     im->extended_info = obr;
	     if (buf->priv.x11.xlib.mask)
	       obr->mxob = evas_software_xlib_x_output_buffer_new(buf->priv.x11.xlib.disp,
								 buf->priv.x11.xlib.vis,
								 1,
								 buf->w, buf->h,
								 use_shm,
								 NULL);
	  }
	else
	  {
	     im = (RGBA_Image *) evas_cache_image_empty(evas_common_image_cache_get());
             im->cache_entry.flags.alpha |= alpha ? 1 : 0;
             evas_cache_image_surface_alloc(&im->cache_entry, buf->w, buf->h);
	     im->extended_info = obr;
	     if ((buf->rot == 0) || (buf->rot == 180))
	       obr->xob = evas_software_xlib_x_output_buffer_new(buf->priv.x11.xlib.disp,
								buf->priv.x11.xlib.vis,
								buf->priv.x11.xlib.depth,
								buf->w, buf->h,
								use_shm,
								NULL);
	     else if ((buf->rot == 90) || (buf->rot == 270))
	       obr->xob = evas_software_xlib_x_output_buffer_new(buf->priv.x11.xlib.disp,
								buf->priv.x11.xlib.vis,
								buf->priv.x11.xlib.depth,
								buf->h, buf->w,
								use_shm,
								NULL);
	     if (buf->priv.x11.xlib.mask)
	       obr->mxob = evas_software_xlib_x_output_buffer_new(buf->priv.x11.xlib.disp,
								 buf->priv.x11.xlib.vis,
								 1, buf->w, buf->h,
								 use_shm,
								 NULL);
	  }
	if (alpha)
          /* FIXME: faster memset! */
          memset(im->image.data, 0, w * h * sizeof(DATA32));

        buf->priv.onebuf = im;
	return im;
     }

   obr = calloc(1, sizeof(Outbuf_Region));
   obr->x = x;
   obr->y = y;
   obr->w = w;
   obr->h = h;
   *cx = 0;
   *cy = 0;
   *cw = w;
   *ch = h;

   use_shm = buf->priv.x11.xlib.shm;
   /* FIXME: magic - i found if shm regions are smaller than 200x200 its
    * faster to use ximages over unix sockets - trial and error
    */
//   use_shm = 0; /* 630 -> 1006 fps */
//   if ((w * h) < (200 * 200)) use_shm = 0; /* 630 -> 962 fps */

   alpha = ((buf->priv.x11.xlib.mask) || (buf->priv.destination_alpha));

   if ((buf->rot == 0) &&
       (buf->priv.mask.r == 0xff0000) &&
       (buf->priv.mask.g == 0x00ff00) &&
       (buf->priv.mask.b == 0x0000ff))
     {
	obr->xob = _find_xob(buf->priv.x11.xlib.disp,
			     buf->priv.x11.xlib.vis,
			     buf->priv.x11.xlib.depth,
			     w, h,
			     use_shm,
			     NULL);
/*      obr->xob = evas_software_xlib_x_output_buffer_new(buf->priv.x11.xlib.disp, */
/*							 buf->priv.x11.xlib.vis, */
/*							 buf->priv.x11.xlib.depth, */
/*							 w, h, */
/*							 use_shm, */
/*							 NULL); */
        im = (RGBA_Image *) evas_cache_image_data(evas_common_image_cache_get(),
                                                  w, h,
                                                  (DATA32 *) evas_software_xlib_x_output_buffer_data(obr->xob, &bpl),
                                                  alpha, EVAS_COLORSPACE_ARGB8888);
	im->extended_info = obr;
	if (buf->priv.x11.xlib.mask)
	  obr->mxob = _find_xob(buf->priv.x11.xlib.disp,
				buf->priv.x11.xlib.vis,
				1, w, h,
				use_shm,
				NULL);
/*
	  obr->mxob = evas_software_xlib_x_output_buffer_new(buf->priv.x11.xlib.disp,
							    buf->priv.x11.xlib.vis,
							    1, w, h,
							    use_shm,
							    NULL);
 */
     }
   else
     {
        im = (RGBA_Image *) evas_cache_image_empty(evas_common_image_cache_get());
        im->cache_entry.flags.alpha |= alpha ? 1 : 0;
        evas_cache_image_surface_alloc(&im->cache_entry, w, h);
	im->extended_info = obr;
	if ((buf->rot == 0) || (buf->rot == 180))
	  obr->xob = _find_xob(buf->priv.x11.xlib.disp,
			       buf->priv.x11.xlib.vis,
			       buf->priv.x11.xlib.depth,
			       w, h,
			       use_shm,
			       NULL);
/*
	  obr->xob = evas_software_xlib_x_output_buffer_new(buf->priv.x11.xlib.disp,
							   buf->priv.x11.xlib.vis,
							   buf->priv.x11.xlib.depth,
							   w, h,
							   use_shm,
							   NULL);
 */
	else if ((buf->rot == 90) || (buf->rot == 270))
	  obr->xob = _find_xob(buf->priv.x11.xlib.disp,
			       buf->priv.x11.xlib.vis,
			       buf->priv.x11.xlib.depth,
			       h, w,
			       use_shm,
			       NULL);
/*
	  obr->xob = evas_software_xlib_x_output_buffer_new(buf->priv.x11.xlib.disp,
							   buf->priv.x11.xlib.vis,
							   buf->priv.x11.xlib.depth,
							   h, w,
							   use_shm,
							   NULL);
 */
	if (buf->priv.x11.xlib.mask)
	  obr->mxob = _find_xob(buf->priv.x11.xlib.disp,
				buf->priv.x11.xlib.vis,
				1, w, h,
				use_shm,
				NULL);
/*
	  obr->mxob = evas_software_xlib_x_output_buffer_new(buf->priv.x11.xlib.disp,
							    buf->priv.x11.xlib.vis,
							    1, w, h,
							    use_shm,
							    NULL);
 */
     }
   if ((buf->priv.x11.xlib.mask) || (buf->priv.destination_alpha))
     /* FIXME: faster memset! */
     memset(im->image.data, 0, w * h * sizeof(DATA32));

   buf->priv.pending_writes = eina_list_append(buf->priv.pending_writes, im);
   return im;
}

void
evas_software_xlib_outbuf_free_region_for_update(Outbuf *buf, RGBA_Image *update)
{
   /* no need to do anything - they are cleaned up on flush */
}

void
evas_software_xlib_outbuf_flush(Outbuf *buf)
{
   Eina_List *l;
   RGBA_Image *im;
   Outbuf_Region *obr;

   if ((buf->priv.onebuf) && (buf->priv.onebuf_regions))
     {
	Region tmpr;

	im = buf->priv.onebuf;
	obr = im->extended_info;
	tmpr = XCreateRegion();
	while (buf->priv.onebuf_regions)
	  {
	     Evas_Rectangle *rect;
	     XRectangle xr;

	     rect = buf->priv.onebuf_regions->data;
	     buf->priv.onebuf_regions = eina_list_remove_list(buf->priv.onebuf_regions, buf->priv.onebuf_regions);
	     xr.x = rect->x;
	     xr.y = rect->y;
	     xr.width = rect->w;
	     xr.height = rect->h;
	     XUnionRectWithRegion(&xr, tmpr, tmpr);
	     if (buf->priv.debug)
	       evas_software_xlib_outbuf_debug_show(buf, buf->priv.x11.xlib.win,
						   rect->x, rect->y, rect->w, rect->h);
	     free(rect);
	  }
	XSetRegion(buf->priv.x11.xlib.disp, buf->priv.x11.xlib.gc, tmpr);
	evas_software_xlib_x_output_buffer_paste(obr->xob, buf->priv.x11.xlib.win,
						buf->priv.x11.xlib.gc,
						0, 0, 0);
	if (obr->mxob)
	  {
	     XSetRegion(buf->priv.x11.xlib.disp, buf->priv.x11.xlib.gcm, tmpr);
	     evas_software_xlib_x_output_buffer_paste(obr->mxob,
						     buf->priv.x11.xlib.mask,
						     buf->priv.x11.xlib.gcm,
						     0, 0, 0);
	  }
	XDestroyRegion(tmpr);
	buf->priv.synced = 0;
     }
   else
     {
#if 1
        XSync(buf->priv.x11.xlib.disp, False);
	EINA_LIST_FOREACH(buf->priv.pending_writes, l, im)
	  {
	     obr = im->extended_info;
	     if (buf->priv.debug)
	       evas_software_xlib_outbuf_debug_show(buf, buf->priv.x11.xlib.win,
						   obr->x, obr->y, obr->w, obr->h);
	     evas_software_xlib_x_output_buffer_paste(obr->xob, buf->priv.x11.xlib.win,
						     buf->priv.x11.xlib.gc,
						     obr->x, obr->y, 0);
	     if (obr->mxob)
	       evas_software_xlib_x_output_buffer_paste(obr->mxob,
						       buf->priv.x11.xlib.mask,
						       buf->priv.x11.xlib.gcm,
						       obr->x, obr->y, 0);
	  }
	while (buf->priv.prev_pending_writes)
	  {
	     im = buf->priv.prev_pending_writes->data;
	     buf->priv.prev_pending_writes =
	       eina_list_remove_list(buf->priv.prev_pending_writes,
				     buf->priv.prev_pending_writes);
	     obr = im->extended_info;
	     evas_cache_image_drop(&im->cache_entry);
	     if (obr->xob) _unfind_xob(obr->xob, 0);
	     if (obr->mxob) _unfind_xob(obr->mxob, 0);
/*
	     if (obr->xob) evas_software_xlib_x_output_buffer_free(obr->xob, 0);
	     if (obr->mxob) evas_software_xlib_x_output_buffer_free(obr->mxob, 0);
 */
	     free(obr);
	  }
	buf->priv.prev_pending_writes = buf->priv.pending_writes;
	buf->priv.pending_writes = NULL;
	XFlush(buf->priv.x11.xlib.disp);
#else
	/* XX async push - disable */
	/*
        EINA_LIST_FOREACH(buf->priv.pending_writes, l, im)
	  {
	     obr = im->extended_info;
	     if (buf->priv.debug)
	       evas_software_xlib_outbuf_debug_show(buf, buf->priv.x11.xlib.win,
						   obr->x, obr->y, obr->w, obr->h);
	     evas_software_xlib_x_output_buffer_paste(obr->xob, buf->priv.x11.xlib.win,
						     buf->priv.x11.xlib.gc,
						     obr->x, obr->y, 0);
	     if (obr->mxob)
	       evas_software_xlib_x_output_buffer_paste(obr->mxob,
						       buf->priv.x11.xlib.mask,
						       buf->priv.x11.xlib.gcm,
						       obr->x, obr->y, 0);
	  }
	 */
	XSync(buf->priv.x11.xlib.disp, False);

	while (buf->priv.pending_writes)
	  {
	     RGBA_Image *im;
	     Outbuf_Region *obr;

	     im = eina_list_data_get(buf->priv.pending_writes);
	     buf->priv.pending_writes = eina_list_remove_list(buf->priv.pending_writes, buf->priv.pending_writes);
	     obr = im->extended_info;
	     evas_cache_image_drop(&im->cache_entry);
	     if (obr->xob) _unfind_xob(obr->xob, 0);
	     if (obr->mxob) _unfind_xob(obr->mxob, 0);
/*
	     if (obr->xob) evas_software_xlib_x_output_buffer_free(obr->xob, 0);
	     if (obr->mxob) evas_software_xlib_x_output_buffer_free(obr->mxob, 0);
 */
	     free(obr);
	     evas_cache_image_drop(&im->cache_entry);
	  }
#endif
     }
   evas_common_cpu_end_opt();
}

void
evas_software_xlib_outbuf_idle_flush(Outbuf *buf)
{
   if (buf->priv.onebuf)
     {
        RGBA_Image *im;
	Outbuf_Region *obr;

	im = buf->priv.onebuf;
	buf->priv.onebuf = NULL;
	obr = im->extended_info;
	if (obr->xob) evas_software_xlib_x_output_buffer_free(obr->xob, 0);
	if (obr->mxob) evas_software_xlib_x_output_buffer_free(obr->mxob, 0);
	free(obr);
	evas_cache_image_drop(&im->cache_entry);
     }
   else
     {
	if (buf->priv.prev_pending_writes) XSync(buf->priv.x11.xlib.disp, False);
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
	     if (obr->xob) _unfind_xob(obr->xob, 0);
	     if (obr->mxob) _unfind_xob(obr->mxob, 0);
	     free(obr);
	  }
	_clear_xob(0);
     }
}

void
evas_software_xlib_outbuf_push_updated_region(Outbuf *buf, RGBA_Image *update, int x, int y, int w, int h)
{
   Gfx_Func_Convert    conv_func = NULL;
   Outbuf_Region      *obr;
   DATA32             *src_data;
   void               *data;
   int                 bpl = 0, yy;

   obr = update->extended_info;
   if (buf->priv.pal)
     {
	if ((buf->rot == 0) || (buf->rot == 180))
	  conv_func = evas_common_convert_func_get(0, w, h,
						   evas_software_xlib_x_output_buffer_depth
						   (obr->xob), buf->priv.mask.r,
						   buf->priv.mask.g, buf->priv.mask.b,
						   buf->priv.pal->colors, buf->rot);
	else if ((buf->rot == 90) || (buf->rot == 270))
	  conv_func = evas_common_convert_func_get(0, h, w,
						   evas_software_xlib_x_output_buffer_depth
						   (obr->xob), buf->priv.mask.r,
						   buf->priv.mask.g, buf->priv.mask.b,
						   buf->priv.pal->colors, buf->rot);
     }
   else
     {
	if ((buf->rot == 0) || (buf->rot == 180))
	  conv_func = evas_common_convert_func_get(0, w, h,
						   evas_software_xlib_x_output_buffer_depth
						   (obr->xob), buf->priv.mask.r,
						   buf->priv.mask.g, buf->priv.mask.b,
						   PAL_MODE_NONE, buf->rot);
	else if ((buf->rot == 90) || (buf->rot == 270))
	  conv_func = evas_common_convert_func_get(0, h, w,
						   evas_software_xlib_x_output_buffer_depth
						   (obr->xob), buf->priv.mask.r,
						   buf->priv.mask.g, buf->priv.mask.b,
						   PAL_MODE_NONE, buf->rot);
     }
   if (!conv_func) return;

   data = evas_software_xlib_x_output_buffer_data(obr->xob, &bpl);
   src_data = update->image.data;
   if (buf->rot == 0)
     {
	obr->x = x;
	obr->y = y;
     }
   else if (buf->rot == 90)
     {
	obr->x = y;
	obr->y = buf->w - x - w;
     }
   else if (buf->rot == 180)
     {
	obr->x = buf->w - x - w;
	obr->y = buf->h - y - h;
     }
   else if (buf->rot == 270)
     {
	obr->x = buf->h - y - h;
	obr->y = x;
     }
   if ((buf->rot == 0) || (buf->rot == 180))
     {
	obr->w = w;
	obr->h = h;
     }
   else if ((buf->rot == 90) || (buf->rot == 270))
     {
	obr->w = h;
	obr->h = w;
     }
   if (buf->priv.pal)
     {
	if (data != src_data)
	  conv_func(src_data, data,
		    0,
		    bpl /
		    ((evas_software_xlib_x_output_buffer_depth(obr->xob) /
		      8)) - obr->w, obr->w, obr->h, x, y,
		    buf->priv.pal->lookup);
     }
   else
     {
	if (data != src_data)
	  conv_func(src_data, data,
		    0,
		    bpl /
		    ((evas_software_xlib_x_output_buffer_depth(obr->xob) /
		      8)) - obr->w, obr->w, obr->h, x, y, NULL);
     }
#if 1
#else
   /* XX async push */
   if (!((buf->priv.onebuf) && (buf->priv.onebuf_regions)))
     {
	if (buf->priv.debug)
	  evas_software_xlib_outbuf_debug_show(buf, buf->priv.x11.xlib.win,
					      obr->x, obr->y, obr->w, obr->h);
	evas_software_xlib_x_output_buffer_paste(obr->xob, buf->priv.x11.xlib.win,
						buf->priv.x11.xlib.gc,
						obr->x, obr->y, 0);
     }
#endif
   if (obr->mxob)
     {
	for (yy = 0; yy < obr->h; yy++)
	  evas_software_xlib_x_write_mask_line(buf, obr->mxob,
					      src_data +
					      (yy * obr->w), obr->w, yy);
#if 1
#else
	/* XX async push */
	if (!((buf->priv.onebuf) && (buf->priv.onebuf_regions)))
	  evas_software_xlib_x_output_buffer_paste(obr->mxob,
						  buf->priv.x11.xlib.mask,
						  buf->priv.x11.xlib.gcm,
						  obr->x, obr->y, 0);
#endif
     }
#if 1
#else
   XFlush(buf->priv.x11.xlib.disp);
#endif
}

void
evas_software_xlib_outbuf_reconfigure(Outbuf * buf, int w, int h, int rot,
				     Outbuf_Depth depth)
{
   if ((w == buf->w) &&
       (h == buf->h) &&
       (rot == buf->rot) &&
       (depth == buf->depth)) return;
   buf->w = w;
   buf->h = h;
   buf->rot = rot;
   evas_software_xlib_outbuf_idle_flush(buf);
}

int
evas_software_xlib_outbuf_get_width(Outbuf * buf)
{
   return buf->w;
}

int
evas_software_xlib_outbuf_get_height(Outbuf * buf)
{
   return buf->h;
}

Outbuf_Depth
evas_software_xlib_outbuf_get_depth(Outbuf * buf)
{
   return buf->depth;
}

int
evas_software_xlib_outbuf_get_rot(Outbuf * buf)
{
   return buf->rot;
}

void
evas_software_xlib_outbuf_drawable_set(Outbuf * buf, Drawable draw)
{
   XGCValues           gcv;

   if (buf->priv.x11.xlib.win == draw) return;
   if (buf->priv.x11.xlib.gc)
     {
	XFreeGC(buf->priv.x11.xlib.disp, buf->priv.x11.xlib.gc);
	buf->priv.x11.xlib.gc = NULL;
     }
   buf->priv.x11.xlib.win = draw;
   buf->priv.x11.xlib.gc = XCreateGC(buf->priv.x11.xlib.disp, buf->priv.x11.xlib.win, 0, &gcv);
}

void
evas_software_xlib_outbuf_mask_set(Outbuf * buf, Pixmap mask)
{
   XGCValues           gcv;

   if (buf->priv.x11.xlib.mask == mask) return;
   if (buf->priv.x11.xlib.gcm)
     {
	XFreeGC(buf->priv.x11.xlib.disp, buf->priv.x11.xlib.gcm);
	buf->priv.x11.xlib.gcm = NULL;
     }
   buf->priv.x11.xlib.mask = mask;
   if (buf->priv.x11.xlib.mask)
     buf->priv.x11.xlib.gcm = XCreateGC(buf->priv.x11.xlib.disp, buf->priv.x11.xlib.mask, 0, &gcv);
}

void
evas_software_xlib_outbuf_debug_set(Outbuf * buf, int debug)
{
   buf->priv.debug = debug;
}

void
evas_software_xlib_outbuf_debug_show(Outbuf * buf, Drawable draw, int x, int y, int w,
			       int h)
{
   int                 i;
   int                 screen_num = 0;

     {
	int                 wx, wy;
	unsigned int        ww, wh, bd, dp;
	Window              wdum, root;
	XWindowAttributes   wattr;

	XGetGeometry(buf->priv.x11.xlib.disp, draw, &root, &wx, &wy, &ww, &wh, &bd, &dp);
	XGetGeometry(buf->priv.x11.xlib.disp, root, &wdum, &wx, &wy, &ww, &wh, &bd, &dp);
	XGetWindowAttributes(buf->priv.x11.xlib.disp, root, &wattr);
	screen_num = XScreenNumberOfScreen(wattr.screen);
     }
   for (i = 0; i < 20; i++)
     {
//	XImage             *xim;

	XSetForeground(buf->priv.x11.xlib.disp, buf->priv.x11.xlib.gc,
		       BlackPixel(buf->priv.x11.xlib.disp, screen_num));
	XFillRectangle(buf->priv.x11.xlib.disp, draw, buf->priv.x11.xlib.gc, x, y, w, h);
	XSync(buf->priv.x11.xlib.disp, False);
//	xim =
//	  XGetImage(buf->priv.x11.xlib.disp, draw, x, y, w, h, 0xffffffff, ZPixmap);
//	if (xim)
//	  XDestroyImage(xim);
	XSync(buf->priv.x11.xlib.disp, False);
	XSetForeground(buf->priv.x11.xlib.disp, buf->priv.x11.xlib.gc,
		       WhitePixel(buf->priv.x11.xlib.disp, screen_num));
	XFillRectangle(buf->priv.x11.xlib.disp, draw, buf->priv.x11.xlib.gc, x, y, w, h);
	XSync(buf->priv.x11.xlib.disp, False);
//	xim =
//	  XGetImage(buf->priv.x11.xlib.disp, draw, x, y, w, h, 0xffffffff, ZPixmap);
//	if (xim)
//	  XDestroyImage(xim);
	XSync(buf->priv.x11.xlib.disp, False);
     }
}
