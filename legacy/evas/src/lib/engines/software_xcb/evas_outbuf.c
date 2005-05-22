#include "evas_common.h"
#include "evas_engine.h"
#include <X11/XCB/shm.h>
#include <X11/XCB/xcb_image.h>
#include <X11/XCB/xcb_atom.h>
#include <sys/time.h>
#include <sys/utsname.h>

void
evas_software_xcb_outbuf_init(void)
{
}

void
evas_software_xcb_outbuf_free(Outbuf * buf)
{
   evas_software_xcb_outbuf_flush(buf);
   XCBFreeGC(buf->priv.x.conn, buf->priv.x.gc);
   if (buf->priv.x.gcm.xid)
      XCBFreeGC(buf->priv.x.conn, buf->priv.x.gcm);
   if (buf->priv.pal)
     evas_software_xcb_x_color_deallocate(buf->priv.x.conn,
					  buf->priv.x.cmap,
					  buf->priv.x.vis,
					  buf->priv.pal);
   evas_software_xcb_outbuf_perf_free(buf->perf);
   free(buf);
}

void
evas_software_xcb_outbuf_rotation_set(Outbuf *buf, int rot)
{
   buf->rot = rot;
}

Outbuf *
evas_software_xcb_outbuf_setup_x(int            w,
				 int            h,
				 int            rot,
				 Outbuf_Depth   depth,
				 XCBConnection *conn,
				 XCBDRAWABLE    draw,
				 XCBVISUALTYPE *vis,
				 XCBCOLORMAP    cmap,
				 int            x_depth,
				 Outbuf_Perf   *perf,
				 int            grayscale,
				 int            max_colors,
				 XCBDRAWABLE    mask,
				 int            shape_dither)
{
   Outbuf *buf;

   buf = calloc(1, sizeof(Outbuf));
   if (!buf)
     {
	free(buf);
	return NULL;
     }

   buf->w = w;
   buf->h = h;
   buf->depth = depth;
   buf->rot = rot;

   buf->priv.x.conn = conn;
   buf->priv.x.vis = vis;
   buf->priv.x.cmap = cmap;
   buf->priv.x.depth = x_depth;

   buf->priv.mask_dither = shape_dither;

   {
      Gfx_Func_Convert    conv_func;
      Xcb_Output_Buffer  *xcbob;

      buf->priv.x.shm = evas_software_xcb_x_can_do_shm(buf->priv.x.conn);
      xcbob = evas_software_xcb_x_output_buffer_new(buf->priv.x.conn,
						    buf->priv.x.depth,
						    1, 1,
						    buf->priv.x.shm, NULL);

      conv_func = NULL;
      if (xcbob)
	{
#ifdef WORDS_BIGENDIAN
	   if (evas_software_xcb_x_output_buffer_byte_order(xcbob) == LSBFirst)
	     buf->priv.x.swap = 1;
#else
	   if (evas_software_xcb_x_output_buffer_byte_order(xcbob) == MSBFirst)
	     buf->priv.x.swap = 1;
#endif
	   if ((vis->_class == TrueColor) || (vis->_class == DirectColor))
	     {
		buf->priv.mask.r = (DATA32) vis->red_mask;
		buf->priv.mask.g = (DATA32) vis->green_mask;
		buf->priv.mask.b = (DATA32) vis->blue_mask;
		if (buf->priv.x.swap)
		  {
		     SWAP32(buf->priv.mask.r);
		     SWAP32(buf->priv.mask.g);
		     SWAP32(buf->priv.mask.b);
		  }
	     }
	   else if ((vis->_class == PseudoColor) ||
		    (vis->_class == StaticColor) ||
		    (vis->_class == GrayScale) || (vis->_class == StaticGray))
	     {
		Convert_Pal_Mode pm = PAL_MODE_RGB332;

		if ((vis->_class == GrayScale) || (vis->_class == StaticGray))
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
		buf->priv.pal =
		   evas_software_xcb_x_color_allocate(conn,
						      cmap,
						      vis,
						      PAL_MODE_RGB666);
		if (!buf->priv.pal)
		  {
		     free(buf);
		     return NULL;
		  }
	     }
	   if (buf->priv.pal)
	     {
		if (buf->rot == 0)
		   conv_func = evas_common_convert_func_get(0,
							    buf->w,
							    buf->h,
							    evas_software_xcb_x_output_buffer_depth (xcbob),
							    buf->priv.mask.r,
							    buf->priv.mask.g,
							    buf->priv.mask.b,
							    buf->priv.pal->colors,
							    buf->rot);
		else if (buf->rot == 270)
		   conv_func = evas_common_convert_func_get(0,
							    buf->h,
							    buf->w,
							    evas_software_xcb_x_output_buffer_depth (xcbob),
							    buf->priv.mask.r,
							    buf->priv.mask.g,
							    buf->priv.mask.b,
							    buf->priv.pal->colors,
							    buf->rot);
		else if (buf->rot == 90)
		   conv_func = evas_common_convert_func_get(0,
							    buf->h,
							    buf->w,
							    evas_software_xcb_x_output_buffer_depth(xcbob),
							    buf->priv.mask.r,
							    buf->priv.mask.g,
							    buf->priv.mask.b,
							    buf->priv.pal->colors,
							    buf->rot);
	     }
	   else
	     {
		if (buf->rot == 0)
		   conv_func = evas_common_convert_func_get(0,
							    buf->w,
							    buf->h,
							    evas_software_xcb_x_output_buffer_depth(xcbob),
							    buf->priv.mask.r,
							    buf->priv.mask.g,
							    buf->priv.mask.b,
							    PAL_MODE_NONE,
							    buf->rot);
		else if (buf->rot == 270)
		   conv_func = evas_common_convert_func_get(0,
							    buf->h,
							    buf->w,
							    evas_software_xcb_x_output_buffer_depth(xcbob),
							    buf->priv.mask.r,
							    buf->priv.mask.g,
							    buf->priv.mask.b,
							    PAL_MODE_NONE,
							    buf->rot);
		else if (buf->rot == 90)
		   conv_func = evas_common_convert_func_get(0,
							    buf->h,
							    buf->w,
							    evas_software_xcb_x_output_buffer_depth(xcbob),
							    buf->priv.mask.r,
							    buf->priv.mask.g,
							    buf->priv.mask.b,
							    PAL_MODE_NONE,
							    buf->rot);
	     }
	   evas_software_xcb_x_output_buffer_free(xcbob, 1);
	   if (!conv_func)
	     {
		printf(".[ Evas Error ].\n"
		       " {\n"
		       "  At depth         %i:\n"
		       "  RGB format mask: %08x, %08x, %08x\n"
		       "  Palette mode:    %i\n"
		       "  Not supported by and compiled in converters!\n"
		       " }\n",
		       buf->priv.x.depth,
		       buf->priv.mask.r,
		       buf->priv.mask.g,
		       buf->priv.mask.b,
		       buf->priv.pal->colors);
	     }
	}
      evas_software_xcb_outbuf_drawable_set(buf, draw);
      evas_software_xcb_outbuf_mask_set(buf, mask);
   }

   buf->perf = perf;

   return buf;
}

RGBA_Image *
evas_software_xcb_outbuf_new_region_for_update(Outbuf *buf,
					       int     x,
					       int     y,
					       int     w,
					       int     h,
					       int    *cx,
					       int    *cy,
					       int    *cw,
					       int    *ch)
{
   RGBA_Image         *im;
   Outbuf_Region      *obr;
   int                 bpl = 0;
   int                 use_shm = 1;

   obr = calloc(1, sizeof(Outbuf_Region));
   obr->x = x;
   obr->y = y;
   obr->w = w;
   obr->h = h;
   *cx = 0;
   *cy = 0;
   *cw = w;
   *ch = h;

   use_shm = buf->priv.x.shm;
   if (buf->perf)
     {
	if ((w * h) < buf->perf->min_shm_image_pixel_count) use_shm = 0;
     }
   else
     {
	if ((w * h) < (200 * 200)) use_shm = 0;
     }

   if ((buf->rot == 0) &&
       (buf->priv.mask.r == 0xff0000) &&
       (buf->priv.mask.g == 0x00ff00) &&
       (buf->priv.mask.b == 0x0000ff))
     {
	im = evas_common_image_new();
	im->image = evas_common_image_surface_new(im);
	im->image->w = w;
	im->image->h = h;
	im->image->data = NULL;
	im->image->no_free = 1;
	im->extended_info = obr;
	obr->xcbob = evas_software_xcb_x_output_buffer_new(buf->priv.x.conn,
							   buf->priv.x.depth,
							   w,
							   h,
							   use_shm,
							   NULL);
	im->image->data = evas_software_xcb_x_output_buffer_data(obr->xcbob, &bpl);
	if (buf->priv.x.mask.pixmap.xid)
	  obr->mxcbob = evas_software_xcb_x_output_buffer_new(buf->priv.x.conn,
							      1,
							      w,
							      h,
							      use_shm,
							      NULL);
     }
   else
     {
        im = evas_common_image_create(w, h);
	im->extended_info = obr;
	if ((buf->rot == 0) || (buf->rot == 180))
	   obr->xcbob = evas_software_xcb_x_output_buffer_new(buf->priv.x.conn,
							      buf->priv.x.depth,
							      w,
							      h,
							      use_shm,
							      NULL);
	else if ((buf->rot == 90) || (buf->rot == 270))
	  obr->xcbob = evas_software_xcb_x_output_buffer_new(buf->priv.x.conn,
							     buf->priv.x.depth,
							     h,
							     w,
							     use_shm,
							     NULL);
	if (buf->priv.x.mask.pixmap.xid)
	  obr->mxcbob = evas_software_xcb_x_output_buffer_new(buf->priv.x.conn,
							      1,
							      w,
							      h,
							      use_shm,
							      NULL);
     }
   if (buf->priv.x.mask.pixmap.xid)
     {
	im->flags |= RGBA_IMAGE_HAS_ALPHA;
	/* FIXME: faster memset! */
	memset(im->image->data, 0, w * h * sizeof(DATA32));
     }
   buf->priv.pending_writes = evas_list_append(buf->priv.pending_writes, im);
   return im;
}

void
evas_software_xcb_outbuf_free_region_for_update(Outbuf     *buf,
						RGBA_Image *update)
{
   /* no need to do anything - they are cleaned up on flush */
}

void
evas_software_xcb_outbuf_flush(Outbuf *buf)
{
   Evas_List *l;

   for (l = buf->priv.pending_writes; l; l = l->next)
     {
	RGBA_Image *im;
	Outbuf_Region      *obr;

	im = l->data;
	obr = im->extended_info;
	/* paste now */
	if (buf->priv.debug)
	  evas_software_xcb_outbuf_debug_show(buf,
					      buf->priv.x.win,
					      obr->x,
					      obr->y,
					      obr->w,
					      obr->h);
	evas_software_xcb_x_output_buffer_paste(obr->xcbob,
						buf->priv.x.win,
						buf->priv.x.gc,
						obr->x,
						obr->y, 0);
	if (obr->mxcbob)
	  evas_software_xcb_x_output_buffer_paste(obr->mxcbob,
						  buf->priv.x.mask,
						  buf->priv.x.gcm,
						  obr->x,
						  obr->y, 0);
     }
   XCBSync(buf->priv.x.conn, 0);
   while (buf->priv.pending_writes)
     {
        RGBA_Image    *im;
	Outbuf_Region *obr;

	im = buf->priv.pending_writes->data;
	buf->priv.pending_writes = evas_list_remove_list(buf->priv.pending_writes,
							 buf->priv.pending_writes);
	obr = im->extended_info;
	evas_common_image_free(im);
	if (obr->xcbob) evas_software_xcb_x_output_buffer_free(obr->xcbob, 0);
	if (obr->mxcbob) evas_software_xcb_x_output_buffer_free(obr->mxcbob, 0);
	free(obr);
     }
   evas_common_cpu_end_opt();
}

void
evas_software_xcb_outbuf_push_updated_region(Outbuf     *buf,
					     RGBA_Image *update,
					     int         x,
					     int         y,
					     int         w,
					     int         h)
{
   Gfx_Func_Convert    conv_func = NULL;
   Outbuf_Region      *obr;
   DATA32             *src_data;
   void               *data;
   int                 bpl = 0;
   int                 yy;

   obr = update->extended_info;
   if (buf->priv.pal)
     {
	if ((buf->rot == 0) || (buf->rot == 180))
	  conv_func = evas_common_convert_func_get(0, w, h,
						   evas_software_xcb_x_output_buffer_depth
						   (obr->xcbob), buf->priv.mask.r,
						   buf->priv.mask.g, buf->priv.mask.b,
						   buf->priv.pal->colors, buf->rot);
	else if ((buf->rot == 90) || (buf->rot == 270))
	  conv_func = evas_common_convert_func_get(0, h, w,
						   evas_software_xcb_x_output_buffer_depth
						   (obr->xcbob), buf->priv.mask.r,
						   buf->priv.mask.g, buf->priv.mask.b,
						   buf->priv.pal->colors, buf->rot);
     }
   else
     {
	if ((buf->rot == 0) || (buf->rot == 180))
	  conv_func = evas_common_convert_func_get(0, w, h,
						   evas_software_xcb_x_output_buffer_depth
						   (obr->xcbob), buf->priv.mask.r,
						   buf->priv.mask.g, buf->priv.mask.b,
						   PAL_MODE_NONE, buf->rot);
	else if ((buf->rot == 90) || (buf->rot == 270))
	  conv_func = evas_common_convert_func_get(0, h, w,
						   evas_software_xcb_x_output_buffer_depth
						   (obr->xcbob), buf->priv.mask.r,
						   buf->priv.mask.g, buf->priv.mask.b,
						   PAL_MODE_NONE, buf->rot);
     }
   if (!conv_func) return;

   data = evas_software_xcb_x_output_buffer_data(obr->xcbob, &bpl);
   src_data = update->image->data;
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
		    ((evas_software_xcb_x_output_buffer_depth(obr->xcbob) /
		      8)) - obr->w, obr->w, obr->h, x, y,
		    buf->priv.pal->lookup);
     }
   else
     {
	if (data != src_data)
	  conv_func(src_data, data,
		    0,
		    bpl /
		    ((evas_software_xcb_x_output_buffer_depth(obr->xcbob) /
		      8)) - obr->w, obr->w, obr->h, x, y, NULL);
     }
   if (obr->mxcbob)
     {
	for (yy = 0; yy < obr->h; yy++)
	  evas_software_xcb_x_write_mask_line(obr->mxcbob,
					      src_data +
					      (yy * obr->w), obr->w, yy);
     }
}

void
evas_software_xcb_outbuf_reconfigure(Outbuf      *buf,
				     int          w,
				     int          h,
				     int          rot,
				     Outbuf_Depth depth)
{
   if ((w == buf->w) &&
       (h == buf->h) &&
       (rot == buf->rot) &&
       (depth == buf->depth)) return;
   buf->w = w;
   buf->h = h;
   buf->rot = rot;
}

int
evas_software_xcb_outbuf_get_width(Outbuf * buf)
{
   return buf->w;
}

int
evas_software_xcb_outbuf_get_height(Outbuf * buf)
{
   return buf->h;
}

Outbuf_Depth
evas_software_xcb_outbuf_get_depth(Outbuf * buf)
{
   return buf->depth;
}

int
evas_software_xcb_outbuf_get_rot(Outbuf * buf)
{
   return buf->rot;
}

void
evas_software_xcb_outbuf_drawable_set(Outbuf     *buf,
				      XCBDRAWABLE draw)
{
   if (buf->priv.x.win.window.xid == draw.window.xid) return;
   if (buf->priv.x.gc.xid)
     {
	XCBFreeGC(buf->priv.x.conn, buf->priv.x.gc);
	buf->priv.x.gc.xid = 0;
     }
   buf->priv.x.win = draw;
   buf->priv.x.gc = XCBGCONTEXTNew(buf->priv.x.conn);
   XCBCreateGC(buf->priv.x.conn, buf->priv.x.gc, buf->priv.x.win, 0, NULL);
}

void
evas_software_xcb_outbuf_mask_set(Outbuf     *buf,
				  XCBDRAWABLE mask)
{
   if (buf->priv.x.mask.pixmap.xid == mask.pixmap.xid) return;
   if (buf->priv.x.gcm.xid)
     {
	XCBFreeGC(buf->priv.x.conn, buf->priv.x.gcm);
	buf->priv.x.gcm.xid = 0;
     }
   buf->priv.x.mask = mask;
   if (buf->priv.x.mask.pixmap.xid)
      {
	 buf->priv.x.gcm = XCBGCONTEXTNew(buf->priv.x.conn);
	 XCBCreateGC(buf->priv.x.conn, buf->priv.x.gcm, buf->priv.x.win, 0, NULL);
      }
}

void
evas_software_xcb_outbuf_debug_set(Outbuf *buf,
				   int     debug)
{
   buf->priv.debug = debug;
}
void
evas_software_xcb_outbuf_debug_show(Outbuf     *buf,
				    XCBDRAWABLE draw,
				    int         x,
				    int         y,
				    int         w,
				    int         h)
{
   int                 i;
   XCBSCREEN          *screen;

   {
      XCBGetGeometryRep *geom;
      XCBDRAWABLE        root;
      XCBSCREENIter      i;
      int                cur;

      geom = XCBGetGeometryReply (buf->priv.x.conn, XCBGetGeometry (buf->priv.x.conn, draw), 0);
      root.window = geom->root;
      free (geom);
      geom = XCBGetGeometryReply (buf->priv.x.conn, XCBGetGeometry (buf->priv.x.conn, root), 0);

      i = XCBConnSetupSuccessRepRootsIter(XCBGetSetup(buf->priv.x.conn));
      for (cur = 0; cur < i.rem; XCBSCREENNext(&i), ++cur)
	 if (i.data->root.xid == geom->root.xid)
	    {
	       screen = i.data;
	       break;
	    }
      free (geom);
   }
   for (i = 0; i < 10; i++)
     {
	XCBImage     *image;
	XCBRECTANGLE  rect = { x, y, w, h};
	CARD32        mask;
	CARD32        value[2];

	mask = GCForeground | GCGraphicsExposures;
	value[0] = screen->black_pixel;
	value[1] = 0; /* no graphics exposures */
	XCBChangeGC(buf->priv.x.conn, buf->priv.x.gc, mask, value);
	XCBPolyFillRectangle (buf->priv.x.conn, draw, buf->priv.x.gc, 1, &rect);
	XCBSync(buf->priv.x.conn, 0);
	image = XCBImageGet(buf->priv.x.conn, draw, x, y, w, h, 0xffffffff, ZPixmap);
	if (image)
	   XCBImageDestroy(image);
	XCBSync(buf->priv.x.conn, 0);
	mask = GCForeground | GCGraphicsExposures;
	value[0] = screen->white_pixel;
	value[1] = 0; /* no graphics exposures */
	XCBChangeGC(buf->priv.x.conn, buf->priv.x.gc, mask, value);
	XCBPolyFillRectangle (buf->priv.x.conn, draw, buf->priv.x.gc, 1, &rect);
	XCBSync(buf->priv.x.conn, 0);
	image = XCBImageGet(buf->priv.x.conn, draw, x, y, w, h, 0xffffffff, ZPixmap);
	if (image)
	   XCBImageDestroy(image);
	XCBSync(buf->priv.x.conn, 0);
     }
}






















/* used for performance tester code */
static double
_evas_get_time(void)
{
   struct timeval      timev;

   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
}


char *
evas_software_xcb_outbuf_perf_serialize_x(Outbuf_Perf *perf)
{
   /* take performance results and turn it inot a munged string that can be */
   /* written out somewhere by a program */
   char                buf[256];

   snprintf(buf, sizeof(buf), "%i", perf->min_shm_image_pixel_count);
   return strdup(buf);
}

void
evas_software_xcb_outbuf_perf_deserialize_x(Outbuf_Perf *perf,
					    const char  *data)
{
   /* take a munged string that is the result of outbuf_perf_serialize_x() */
   /* and turn it back into a structure  and fill the provided perf struct */
   /* with it. the perf struct is assumed to be pristine from */
   /* outbuf_perf_new_x() */
   int                 val;

   val = 200 * 200;
   if (sscanf(data, "%i", &val) != 1)
      val = 200 * 200;
   if (val < 0)
      val = 200 * 200;
   perf->min_shm_image_pixel_count = val;
   return;
}

Outbuf_Perf *
evas_software_xcb_outbuf_perf_new_x(XCBConnection *conn,
				    XCBDRAWABLE    draw,
				    XCBVISUALTYPE *vis,
				    XCBCOLORMAP    cmap,
				    int            x_depth)
{
   /* create an "empty" perf struct with just the system & display info */
   Outbuf_Perf    *perf;
   XCBDRAWABLE     root;
   struct utsname  un;
   FILE           *f;

   perf = calloc(1, sizeof(Outbuf_Perf));

   perf->x.conn = conn;

   root.window = XCBConnSetupSuccessRepRootsIter (XCBGetSetup (conn)).data->root;
   if (draw.window.xid)
     {
	XCBGetGeometryRep *geom;
        XCBSCREENIter      i;
	int                cur;

	geom = XCBGetGeometryReply (conn, XCBGetGeometry (conn, draw), NULL);
	root.window = geom->root;
	free (geom);
	geom = XCBGetGeometryReply (conn, XCBGetGeometry (conn, root), 0);
	perf->x.w = (int)geom->width;
	perf->x.h = (int)geom->height;

	perf->x.screen_num = 0;
	i = XCBConnSetupSuccessRepRootsIter(XCBGetSetup(conn));
	for (cur = 0; cur < i.rem; XCBSCREENNext(&i), ++cur)
	  if (i.data->root.xid == geom->root.xid)
	    {
	      perf->x.screen_num = cur;
	      break;
	    }
	free (geom);
     }
   perf->x.root = root;

   perf->x.display      = strdup (":0"); /* FIXME: strdup(DisplayString(disp)); in XCB ? */
   perf->x.vendor       = strdup(XCBConnSetupSuccessRepVendor(XCBGetSetup(conn)));
   perf->x.version      = (int)XCBGetSetup(conn)->protocol_major_version;
   perf->x.revision     = (int)XCBGetSetup(conn)->protocol_minor_version;
   perf->x.release      = (int)XCBGetSetup(conn)->release_number;
   perf->x.screen_count = XCBConnSetupSuccessRepRootsIter(XCBGetSetup(conn)).rem;
   perf->x.depth        = x_depth;

   perf->min_shm_image_pixel_count = 200 * 200;	/* default hard-coded */

   if (!uname(&un))
     {
	perf->os.name = strdup(un.sysname);
	perf->os.version = strdup(un.release);
	perf->os.machine = strdup(un.machine);
     }
   /* for linux */
   f = fopen("/proc/cpuinfo", "r");
   if (f)
     {
	char                buf[16384];
	size_t              sz;

	/* read up tothe first 16k of it... shoudl be nice and fast and easy */
	sz = fread(buf, 1, 16380, f);
	if (sz > 0)
	  {
	     perf->cpu.info = malloc(sz + 1);
	     strncpy(perf->cpu.info, buf, sz);
	     perf->cpu.info[sz] = 0;
	  }
	fclose(f);
     }
   else
     {
	/* for FreeBSD... maybe */
	f = fopen("/var/run/dmesg.boot", "r");
	/* for NetBSD... maybe */
	if (!f)
	   f = fopen("/kern/msgbuf", "r");
	if (f)
	  {
	     char                buf[4096];
	     int                 l;

	     l = 0;
	     while (fgets(buf, sizeof(buf), f))
	       {
		  int                 len;

		  /* to read lines like: */
		  /* CPU: AMD-K7(tm) Processor (698.65-MHz 686-class CPU) */
		  /* Features=0x81f9ff<FPU,VME,DE,PSE,TSC,MSR,PAE,MCE,CX8,SEP,MTRR,PGE,MCA,CMOV,PAT,MMX> */
		  if ((!strncmp(buf, "CPU: ", 5)) ||
		      (!strncmp(buf, "Features=", 9)))
		    {
		       len = strlen(buf);
		       l += len;
		       if (!perf->cpu.info)
			  perf->cpu.info = strdup(buf);
		       else
			 {
			    perf->cpu.info = realloc(perf->cpu.info, l + 1);
			    if (perf->cpu.info)
			       strcat(perf->cpu.info, buf);
			 }
		    }
	       }
	     fclose(f);
	  }
     }
   if (!perf->cpu.info)
      perf->cpu.info = strdup("");
   return perf;
   vis = NULL;
   cmap.xid = 0;
}

char *
evas_software_xcb_outbuf_perf_serialize_info_x(Outbuf_Perf *perf)
{
   /* get a seriazed string that is a unique identifier for your */
   /* hardware/x/connection setup. */
   char                buf[32768];
   int                 sum1, sum2, i;
   char               *p;

   sum1 = 0;
   sum2 = 0;
   snprintf(buf, sizeof(buf),
	    "%s|%s|%i|%i|%i|%i|%i|%i|%i|%i|%s|%s|%s|%s",
	    perf->x.display, perf->x.vendor, perf->x.version, perf->x.revision,
	    perf->x.release, perf->x.w, perf->x.h, perf->x.screen_count,
	    perf->x.depth, perf->x.screen_num,
	    perf->os.name, perf->os.version, perf->os.machine, perf->cpu.info);
   p = buf;
   i = 0;
   while (*p)
     {
	sum1 += (int)(*p) << (i % 24);
	sum2 ^= ((int)(*p) << (i % 24)) * ((int)(*p));
	i++;
	p++;
     }
   snprintf(buf, sizeof(buf), "%08x%08x", sum1, sum2);
   return strdup(buf);
}

void
evas_software_xcb_outbuf_perf_store_x(Outbuf_Perf *perf)
{
   /* write performance results to x root property */
   XCBInternAtomRep   *rep;
   XCBATOM             type, format;
   char               *type_str;
   char               *str;

   type_str = "__EVAS_PERF_ENGINE_SOFTWARE";
   rep = XCBInternAtomReply(perf->x.conn,
			    XCBInternAtom(perf->x.conn,
					  0,
					  strlen (type_str),
					  type_str),
			    NULL);
   type = rep->atom;
   format = STRING;

   str = evas_software_xcb_outbuf_perf_serialize_x(perf);
   XCBChangeProperty(perf->x.conn, PropModeReplace, perf->x.root.window,
		     type, format, 8,
		     strlen(str), str);
/*    XSync(perf->x.disp, False); */
   free(str);
}

Outbuf_Perf *
evas_software_xcb_outbuf_perf_restore_x(XCBConnection *conn,
					XCBDRAWABLE    draw,
					XCBVISUALTYPE *vis,
					XCBCOLORMAP    cmap,
					int            x_depth)
{
   /* read performance results from root window */
   XCBInternAtomRep    *type_rep;
   XCBGetPropertyCookie cookie;
   XCBGetPropertyRep   *prop_rep;
   char                *type_str;
   XCBATOM              type, format;
   Outbuf_Perf         *perf;
   char                *retval;
   int                  retnum;

   perf = evas_software_xcb_outbuf_perf_new_x(conn, draw, vis, cmap, x_depth);

   type_str = "__EVAS_PERF_ENGINE_SOFTWARE";
   type_rep = XCBInternAtomReply(conn,
				 XCBInternAtom(conn,
					       0,
					       strlen (type_str),
					       type_str),
				 NULL);
   type = type_rep->atom;
   format = STRING;
   retval = NULL;

   cookie = XCBGetProperty(conn, 0, perf->x.root.window,
			   type, format,
			   0, 16384);
   prop_rep = XCBGetPropertyReply(conn, cookie, NULL);

   retval = XCBGetPropertyValue(prop_rep);
   retnum = XCBGetPropertyValueLength(prop_rep);
   if (retval)
     {
        if ((prop_rep->format == 8) &&
	    (prop_rep->type.xid == type.xid))
	{
	   char *s;

	   s = malloc(retnum + 1);
	   strncpy(s, retval, retnum);
	   s[retnum] = 0;
	   evas_software_xcb_outbuf_perf_deserialize_x(perf, s);
	   free(s);
	}
	/* FIXME: doesn't seem to be need (from valgrind) */
/* 	free(retval); */
     }
   return perf;
}

void
evas_software_xcb_outbuf_perf_free(Outbuf_Perf *perf)
{
   /* free the perf struct */
   free(perf->x.display);
   free(perf->x.vendor);
   free(perf->os.name);
   free(perf->os.version);
   free(perf->os.machine);
   free(perf->cpu.info);
   free(perf);
}

Outbuf_Perf *
evas_software_xcb_outbuf_perf_x(XCBConnection *conn,
				XCBDRAWABLE    draw,
				XCBVISUALTYPE *vis,
				XCBCOLORMAP    cmap,
				int            x_depth)
{
   Outbuf_Perf   *perf;
   XCBDRAWABLE    win;
   CARD32         mask;
   CARD32         value[7];
   int            w, h;
   int            do_shm = 0;

   perf = evas_software_xcb_outbuf_perf_new_x(conn, draw, vis, cmap, x_depth);

   mask = CWBackingStore | CWColormap |
     CWBackPixmap | CWBorderPixel |
     CWBitGravity | CWEventMask | CWOverrideRedirect;
   value[0] = Always;
   value[1] = cmap.xid;
   value[2] = None;
   value[3] = 0;
   value[4] = ForgetGravity;
   value[5] = 0;
   value[6] = 1;
   w = perf->x.w;
   h = perf->x.h;
   win.window = XCBWINDOWNew (conn);
   XCBCreateWindow (conn, x_depth,
		    win.window, perf->x.root.window,
		    0, 0,
		    w, h,
		    0,
		    InputOutput,
		    vis->visual_id,
		    mask, value);
   XCBSync(conn, 0);
   XCBMapWindow (conn, win.window);

   do_shm = evas_software_xcb_x_can_do_shm(conn);

   /* set it to something ridiculous to start */
   perf->min_shm_image_pixel_count = w * w;

   if (do_shm)
     {
	Xcb_Output_Buffer  *xcbob;
	XCBGCONTEXT         gc;
	CARD32              gcv;
	int                 i;
	int                 max;
	int                 error;
	int                 chosen;

	chosen = 0;
	error = 0;
	max = w;
	if (w > h)
	   max = h;
	gc = XCBGCONTEXTNew (conn);
	XCBCreateGC (conn, gc, win, 0, &gcv);
	for (i = 16; i < max; i += 16)
	  {
	     int                 l;
	     double              t0, t1, t2;
	     int                 loops;

	     loops = (h * h * 5) / (i * i);
	     t0 = _evas_get_time();
	     for (l = 0; l < loops; l++)
	       {
		  xcbob = evas_software_xcb_x_output_buffer_new(conn,
								x_depth,
								i, i,
								do_shm,
								NULL);
		  if (!xcbob)
		     error = 1;
		  else
		    {
		       evas_software_xcb_x_output_buffer_paste(xcbob,
							       win,
							       gc,
							       0, 0,
							       1);
		       evas_software_xcb_x_output_buffer_free(xcbob, 1);
		    }
	       }
	     XCBSync(conn, 0);
	     t1 = _evas_get_time() - t0;
	     t0 = _evas_get_time();
	     for (l = 0; l < loops; l++)
	       {
		  xcbob = evas_software_xcb_x_output_buffer_new(conn,
								x_depth,
								i, i,
								0,
								NULL);
		  if (!xcbob)
		     error = 1;
		  else
		    {
		       evas_software_xcb_x_output_buffer_paste(xcbob,
							       win,
							       gc,
							       0, 0,
							       1);
		       evas_software_xcb_x_output_buffer_free(xcbob, 1);
		    }
	       }
	     XCBSync(conn, 0);
	     t2 = _evas_get_time() - t0;
	     if ((!chosen) && (!error))
	       {
		  if ((t1 / t2) < 1.0)
		    {
		       perf->min_shm_image_pixel_count = (i - 8) * (i - 8);
		       chosen = 1;
		    }
	       }
	  }
	XCBFreeGC(conn, gc);
     }
   XCBDestroyWindow(conn, win.window);
   return perf;
}
