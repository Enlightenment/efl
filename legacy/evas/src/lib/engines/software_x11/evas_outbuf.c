#include "evas_common.h"
#include "evas_engine.h"
#include <sys/time.h>
#include <sys/utsname.h>

static double       evas_software_x11_outbuf_software_x11_get_time(void);

/* used for performance tester code */
static double
evas_software_x11_outbuf_software_x11_get_time(void)
{
   struct timeval      timev;

   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
}

void
evas_software_x11_outbuf_software_x11_init(void)
{
}

void
evas_software_x11_outbuf_software_x11_free(Outbuf * buf)
{
   XFreeGC(buf->priv.x.disp, buf->priv.x.gc);
   if (buf->priv.x.gcm)
      XFreeGC(buf->priv.x.disp, buf->priv.x.gcm);
   if (buf->priv.pal)
      evas_software_x11_x_software_x11_color_deallocate(buf->priv.x.disp, buf->priv.x.cmap,
				      buf->priv.x.vis, buf->priv.pal);
   if (buf->priv.back_buf)
      evas_common_image_free(buf->priv.back_buf);
   evas_software_x11_outbuf_software_x11_perf_free(buf->perf);
   free(buf);
}

Outbuf             *
evas_software_x11_outbuf_software_x11_setup_x(int w, int h, int rot, Outbuf_Depth depth,
			    Display * disp, Drawable draw, Visual * vis,
			    Colormap cmap, int x_depth, Outbuf_Perf * perf,
			    int grayscale, int max_colors, Pixmap mask,
			    int shape_dither)
{
   Outbuf             *buf;

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

   buf->priv.x.disp = disp;
   buf->priv.x.vis = vis;
   buf->priv.x.cmap = cmap;
   buf->priv.x.depth = x_depth;

   buf->priv.mask_dither = shape_dither;

   {
      Gfx_Func_Convert    conv_func;
      X_Output_Buffer    *xob;

      buf->priv.x.shm = evas_software_x11_x_software_x11_can_do_shm(buf->priv.x.disp);
      xob = evas_software_x11_x_software_x11_output_buffer_new(buf->priv.x.disp,
					     buf->priv.x.vis,
					     buf->priv.x.depth,
					     1, 1, buf->priv.x.shm, NULL);

      conv_func = NULL;
      if (xob)
	{
#ifdef WORDS_BIGENDIAN
	   if (evas_software_x11_x_software_x11_output_buffer_byte_order(xob) == LSBFirst)
	     buf->priv.x.swap = 1;
#else	   
	   if (evas_software_x11_x_software_x11_output_buffer_byte_order(xob) == MSBFirst)
	     buf->priv.x.swap = 1;
#endif	     
	   if ((vis->class == TrueColor) || (vis->class == DirectColor))
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
	   else if ((vis->class == PseudoColor) ||
		    (vis->class == StaticColor) ||
		    (vis->class == GrayScale) || (vis->class == StaticGray))
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
		buf->priv.pal = evas_software_x11_x_software_x11_color_allocate(disp, cmap, vis,
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
		   conv_func = evas_common_convert_func_get(0, buf->w, buf->h,
						evas_software_x11_x_software_x11_output_buffer_depth
						(xob), buf->priv.mask.r,
						buf->priv.mask.g,
						buf->priv.mask.b,
						buf->priv.pal->colors,
						buf->rot);
		else if (buf->rot == 270)
		   conv_func = evas_common_convert_func_get(0, buf->h, buf->w,
						evas_software_x11_x_software_x11_output_buffer_depth
						(xob), buf->priv.mask.r,
						buf->priv.mask.g,
						buf->priv.mask.b,
						buf->priv.pal->colors,
						buf->rot);
		else if (buf->rot == 90)
		   conv_func = evas_common_convert_func_get(0, buf->h, buf->w,
						evas_software_x11_x_software_x11_output_buffer_depth
						(xob), buf->priv.mask.r,
						buf->priv.mask.g,
						buf->priv.mask.b,
						buf->priv.pal->colors,
						buf->rot);
	     }
	   else
	     {
		if (buf->rot == 0)
		   conv_func = evas_common_convert_func_get(0, buf->w, buf->h,
						evas_software_x11_x_software_x11_output_buffer_depth
						(xob), buf->priv.mask.r,
						buf->priv.mask.g,
						buf->priv.mask.b, PAL_MODE_NONE,
						buf->rot);
		else if (buf->rot == 270)
		   conv_func = evas_common_convert_func_get(0, buf->h, buf->w,
						evas_software_x11_x_software_x11_output_buffer_depth
						(xob), buf->priv.mask.r,
						buf->priv.mask.g,
						buf->priv.mask.b, PAL_MODE_NONE,
						buf->rot);
		else if (buf->rot == 90)
		   conv_func = evas_common_convert_func_get(0, buf->h, buf->w,
						evas_software_x11_x_software_x11_output_buffer_depth
						(xob), buf->priv.mask.r,
						buf->priv.mask.g,
						buf->priv.mask.b, PAL_MODE_NONE,
						buf->rot);
	     }
	   evas_software_x11_x_software_x11_output_buffer_free(xob);
	   if (!conv_func)
	     {
		printf(".[ Evas Error ].\n"
		       " {\n"
		       "  At depth         %i:\n",
		       "  RGB format mask: %08x, %08x, %08x\n"
		       "  Palette mode:    %i\n"
		       "  Not supported by and compiled in converters!\n",
		       " }\n",
		       buf->priv.x.depth,
		       buf->priv.mask.r,
		       buf->priv.mask.g,
		       buf->priv.mask.b, buf->priv.pal->colors);
	     }
	}
      buf->priv.x.shm = evas_software_x11_x_software_x11_can_do_shm(buf->priv.x.disp);
      evas_software_x11_outbuf_software_x11_drawable_set(buf, draw);
      evas_software_x11_outbuf_software_x11_mask_set(buf, mask);
   }

#if 0
   if (buf->priv.x.depth < 24)
      buf->priv.back_buf = evas_common_image_create(buf->w, buf->h);
#endif

   buf->perf = perf;

   return buf;
}

char               *
evas_software_x11_outbuf_software_x11_perf_serialize_x(Outbuf_Perf * perf)
{
   /* take performance results and turn it inot a munged string that can be */
   /* written out somewhere by a program */
   char                buf[256];

   snprintf(buf, sizeof(buf), "%i", perf->min_shm_image_pixel_count);
   return strdup(buf);
}

void
evas_software_x11_outbuf_software_x11_perf_deserialize_x(Outbuf_Perf * perf, const char *data)
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

Outbuf_Perf        *
evas_software_x11_outbuf_software_x11_perf_new_x(Display * disp, Window draw, Visual * vis,
			       Colormap cmap, int x_depth)
{
   /* create an "empty" perf struct with just the system & display info */
   Outbuf_Perf        *perf;
   Window              root;
   struct utsname      un;
   FILE               *f;

   perf = calloc(1, sizeof(Outbuf_Perf));

   perf->x.disp = disp;

   root = DefaultRootWindow(disp);
   if (draw)
     {
	int                 wx, wy;
	unsigned int        ww, wh, bd, dp;
	Window              wdum;
	XWindowAttributes   wattr;

	XGetGeometry(disp, draw, &root, &wx, &wy, &ww, &wh, &bd, &dp);
	XGetGeometry(disp, root, &wdum, &wx, &wy, &ww, &wh, &bd, &dp);
	XGetWindowAttributes(disp, root, &wattr);
	perf->x.w = (int)ww;
	perf->x.h = (int)wh;
	perf->x.screen_num = XScreenNumberOfScreen(wattr.screen);
     }
   perf->x.root = root;

   perf->x.display = strdup(DisplayString(disp));
   perf->x.vendor = strdup(ServerVendor(disp));
   perf->x.version = ProtocolVersion(disp);
   perf->x.revision = ProtocolRevision(disp);
   perf->x.release = VendorRelease(disp);
   perf->x.screen_count = ScreenCount(disp);
   perf->x.depth = x_depth;

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
   cmap = 0;
}

char               *
evas_software_x11_outbuf_software_x11_perf_serialize_info_x(Outbuf_Perf * perf)
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
evas_software_x11_outbuf_software_x11_perf_store_x(Outbuf_Perf * perf)
{
   /* write performance results to x root property */
   Atom                type, format;
   char               *str;

   type = XInternAtom(perf->x.disp, "__EVAS_PERF_ENGINE_SOFTWARE", False);
   format = XA_STRING;
   str = evas_software_x11_outbuf_software_x11_perf_serialize_x(perf);
   XChangeProperty(perf->x.disp, perf->x.root, type, format, 8,
		   PropModeReplace, (unsigned char *)str, strlen(str));
   XSync(perf->x.disp, False);
   free(str);
}

Outbuf_Perf        *
evas_software_x11_outbuf_software_x11_perf_restore_x(Display * disp, Window draw, Visual * vis,
				   Colormap cmap, int x_depth)
{
   /* read performance results from root window */
   Atom                type, format;
   Outbuf_Perf        *perf;
   char               *retval;
   Atom                type_ret;
   unsigned long       bytes_after, num_ret;
   int                 format_ret;

   perf = evas_software_x11_outbuf_software_x11_perf_new_x(disp, draw, vis, cmap, x_depth);
   type = XInternAtom(disp, "__EVAS_PERF_ENGINE_SOFTWARE", False);
   format = XA_STRING;
   retval = NULL;
   XGetWindowProperty(disp, perf->x.root, type, 0, 16384, False, format,
		      &type_ret, &format_ret, &num_ret, &bytes_after,
		      (unsigned char **)&retval);
   if (retval)
     {
	char               *s;

	if (format_ret != 8)
	   goto out;
	if (type_ret != type)
	   goto out;
	s = malloc(num_ret + 1);
	strncpy(s, retval, num_ret);
	s[num_ret] = 0;
	evas_software_x11_outbuf_software_x11_perf_deserialize_x(perf, s);
	free(s);
      out:
	XFree(retval);
     }
   return perf;
}

void
evas_software_x11_outbuf_software_x11_perf_free(Outbuf_Perf * perf)
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

Outbuf_Perf        *
evas_software_x11_outbuf_software_x11_perf_x(Display * disp, Window draw, Visual * vis,
			   Colormap cmap, int x_depth)
{
   Outbuf_Perf        *perf;
   XSetWindowAttributes attr;
   Window              win;
   int                 w, h;
   int                 do_shm = 0;

   perf = evas_software_x11_outbuf_software_x11_perf_new_x(disp, draw, vis, cmap, x_depth);

   attr.backing_store = Always;
   attr.colormap = cmap;
   attr.border_pixel = 0;
   attr.background_pixmap = None;
   attr.event_mask = 0;
   attr.bit_gravity = ForgetGravity;
   attr.override_redirect = True;
   w = perf->x.w;
   h = perf->x.h;
   win = XCreateWindow(disp,
		       perf->x.root,
		       0, 0, w, h, 0,
		       x_depth,
		       InputOutput,
		       vis,
		       CWBackingStore | CWColormap |
		       CWBackPixmap | CWBorderPixel |
		       CWBitGravity | CWEventMask | CWOverrideRedirect, &attr);
   XSync(disp, False);
   XMapRaised(disp, win);

   do_shm = evas_software_x11_x_software_x11_can_do_shm(disp);

   /* set it to something ridiculous to start */
   perf->min_shm_image_pixel_count = w * w;

   if (do_shm)
     {
	X_Output_Buffer    *xob;
	GC                  gc;
	XGCValues           gcv;
	int                 i;
	int                 max;
	int                 error;
	int                 chosen;

	chosen = 0;
	error = 0;
	max = w;
	if (w > h)
	   max = h;
	gc = XCreateGC(disp, win, 0, &gcv);
	for (i = 16; i < max; i += 16)
	  {
	     int                 l;
	     double              t0, t1, t2;
	     int                 loops;

	     loops = (h * h * 5) / (i * i);
	     t0 = evas_software_x11_outbuf_software_x11_get_time();
	     for (l = 0; l < loops; l++)
	       {
		  xob = evas_software_x11_x_software_x11_output_buffer_new(disp, vis, x_depth,
							 i, i, do_shm, NULL);
		  if (!xob)
		     error = 1;
		  else
		    {
		       evas_software_x11_x_software_x11_output_buffer_paste(xob, win, gc, 0, 0);
		       evas_software_x11_x_software_x11_output_buffer_free(xob);
		    }
	       }
	     XSync(disp, False);
	     t1 = evas_software_x11_outbuf_software_x11_get_time() - t0;
	     t0 = evas_software_x11_outbuf_software_x11_get_time();
	     for (l = 0; l < loops; l++)
	       {
		  xob = evas_software_x11_x_software_x11_output_buffer_new(disp, vis, x_depth,
							 i, i, 0, NULL);
		  if (!xob)
		     error = 1;
		  else
		    {
		       evas_software_x11_x_software_x11_output_buffer_paste(xob, win, gc, 0, 0);
		       evas_software_x11_x_software_x11_output_buffer_free(xob);
		    }
	       }
	     XSync(disp, False);
	     t2 = evas_software_x11_outbuf_software_x11_get_time() - t0;
	     if ((!chosen) && (!error))
	       {
		  if ((t1 / t2) < 1.0)
		    {
		       perf->min_shm_image_pixel_count = (i - 8) * (i - 8);
		       chosen = 1;
		    }
	       }
	  }
	XFreeGC(disp, gc);
     }
   XDestroyWindow(disp, win);
   return perf;
}

void
evas_software_x11_outbuf_software_x11_blit(Outbuf * buf, int src_x, int src_y, int w, int h,
			 int dst_x, int dst_y)
{
   if (buf->priv.back_buf)
     {
	evas_common_blit_rectangle(buf->priv.back_buf, buf->priv.back_buf,
		       src_x, src_y, w, h, dst_x, dst_y);
	evas_software_x11_outbuf_software_x11_update(buf, dst_x, dst_y, w, h);
     }
   else
     {
	if (buf->priv.x.disp)
	  {
	     if (buf->rot == 0)
		XCopyArea(buf->priv.x.disp, buf->priv.x.win, buf->priv.x.win,
			  buf->priv.x.gc, src_x, src_y, w, h, dst_x, dst_y);
	     else if (buf->rot == 270)
		XCopyArea(buf->priv.x.disp, buf->priv.x.win, buf->priv.x.win,
			  buf->priv.x.gc, buf->h - src_y - h, src_x, h, w,
			  dst_y, dst_x);
	     else if (buf->rot == 90)
		XCopyArea(buf->priv.x.disp, buf->priv.x.win, buf->priv.x.win,
			  buf->priv.x.gc, src_y, buf->w - src_x - w, h, w,
			  dst_y, dst_x);
	  }
     }
}

void
evas_software_x11_outbuf_software_x11_update(Outbuf * buf, int x, int y, int w, int h)
{
   Gfx_Func_Convert    conv_func;
   DATA8              *data;
   X_Output_Buffer    *xob;
   int                 bpl;
   int                 use_shm;

   if (!(buf->priv.back_buf))
      return;
   use_shm = buf->priv.x.shm;
   if (buf->perf)
     {
	if ((w * h) < buf->perf->min_shm_image_pixel_count)
	   use_shm = 0;
     }
   else
     {
	if ((w * h) < (200 * 200))
	   use_shm = 0;
     }
   xob = NULL;
   if (buf->rot == 0)
      xob = evas_software_x11_x_software_x11_output_buffer_new(buf->priv.x.disp, buf->priv.x.vis,
					     buf->priv.x.depth, w, h, use_shm,
					     NULL);
   else if (buf->rot == 270)
      xob = evas_software_x11_x_software_x11_output_buffer_new(buf->priv.x.disp, buf->priv.x.vis,
					     buf->priv.x.depth, h, w, use_shm,
					     NULL);
   else if (buf->rot == 90)
      xob = evas_software_x11_x_software_x11_output_buffer_new(buf->priv.x.disp, buf->priv.x.vis,
					     buf->priv.x.depth, h, w, use_shm,
					     NULL);
   if (!xob)
      return;
   data = evas_software_x11_x_software_x11_output_buffer_data(xob, &bpl);
   conv_func = NULL;
   if (buf->priv.pal)
     {
	if (buf->rot == 0)
	   conv_func = evas_common_convert_func_get(0, w, h,
					evas_software_x11_x_software_x11_output_buffer_depth(xob),
					buf->priv.mask.r,
					buf->priv.mask.g,
					buf->priv.mask.b,
					buf->priv.pal->colors, buf->rot);
	else if (buf->rot == 270)
	   conv_func = evas_common_convert_func_get(0, h, w,
					evas_software_x11_x_software_x11_output_buffer_depth(xob),
					buf->priv.mask.r,
					buf->priv.mask.g,
					buf->priv.mask.b,
					buf->priv.pal->colors, buf->rot);
	else if (buf->rot == 90)
	   conv_func = evas_common_convert_func_get(0, h, w,
					evas_software_x11_x_software_x11_output_buffer_depth(xob),
					buf->priv.mask.r,
					buf->priv.mask.g,
					buf->priv.mask.b,
					buf->priv.pal->colors, buf->rot);
     }
   else
     {
	if (buf->rot == 0)
	   conv_func = evas_common_convert_func_get(0, w, h,
					evas_software_x11_x_software_x11_output_buffer_depth(xob),
					buf->priv.mask.r,
					buf->priv.mask.g,
					buf->priv.mask.b,
					PAL_MODE_NONE, buf->rot);
	else if (buf->rot == 270)
	   conv_func = evas_common_convert_func_get(0, h, w,
					evas_software_x11_x_software_x11_output_buffer_depth(xob),
					buf->priv.mask.r,
					buf->priv.mask.g,
					buf->priv.mask.b,
					PAL_MODE_NONE, buf->rot);
	else if (buf->rot == 90)
	   conv_func = evas_common_convert_func_get(0, h, w,
					evas_software_x11_x_software_x11_output_buffer_depth(xob),
					buf->priv.mask.r,
					buf->priv.mask.g,
					buf->priv.mask.b,
					PAL_MODE_NONE, buf->rot);
     }
   if (conv_func)
     {
	DATA32             *src_data;

	src_data = buf->priv.back_buf->image->data + (y * buf->w) + x;
	if (buf->priv.pal)
	  {
	     if (buf->rot == 0)
	       {
		  conv_func(src_data, data,
			    buf->w - w,
			    bpl /
			    ((evas_software_x11_x_software_x11_output_buffer_depth(xob) / 8)) - w,
			    w, h, x, y, buf->priv.pal->lookup);
		  if (buf->priv.debug)
		     evas_software_x11_outbuf_software_x11_debug_show(buf, buf->priv.x.win, x, y,
						    w, h);
		  evas_software_x11_x_software_x11_output_buffer_paste(xob, buf->priv.x.win,
						     buf->priv.x.gc, x, y);
	       }
	     else if (buf->rot == 270)
	       {
		  conv_func(src_data, data,
			    buf->w - w,
			    bpl /
			    ((evas_software_x11_x_software_x11_output_buffer_depth(xob) / 8)) - h,
			    h, w, x, y, buf->priv.pal->lookup);
		  if (buf->priv.debug)
		     evas_software_x11_outbuf_software_x11_debug_show(buf, buf->priv.x.win, x, y,
						    w, h);
		  evas_software_x11_x_software_x11_output_buffer_paste(xob, buf->priv.x.win,
						     buf->priv.x.gc,
						     buf->h - y - h, x);
	       }
	     else if (buf->rot == 90)
	       {
		  conv_func(src_data, data,
			    buf->w - w,
			    bpl /
			    ((evas_software_x11_x_software_x11_output_buffer_depth(xob) / 8)) - h,
			    h, w, x, y, buf->priv.pal->lookup);
		  if (buf->priv.debug)
		     evas_software_x11_outbuf_software_x11_debug_show(buf, buf->priv.x.win, x, y,
						    w, h);
		  evas_software_x11_x_software_x11_output_buffer_paste(xob, buf->priv.x.win,
						     buf->priv.x.gc, y,
						     buf->w - x - w);
	       }
	  }
	else
	  {
	     if (buf->rot == 0)
	       {
		  conv_func(src_data, data,
			    buf->w - w,
			    bpl /
			    ((evas_software_x11_x_software_x11_output_buffer_depth(xob) / 8)) - w,
			    w, h, x, y, NULL);
		  if (buf->priv.debug)
		     evas_software_x11_outbuf_software_x11_debug_show(buf, buf->priv.x.win, x, y,
						    w, h);
		  evas_software_x11_x_software_x11_output_buffer_paste(xob, buf->priv.x.win,
						     buf->priv.x.gc, x, y);
	       }
	     else if (buf->rot == 270)
	       {
		  conv_func(src_data, data,
			    buf->w - w,
			    bpl /
			    ((evas_software_x11_x_software_x11_output_buffer_depth(xob) / 8)) - h,
			    h, w, x, y, NULL);
		  if (buf->priv.debug)
		     evas_software_x11_outbuf_software_x11_debug_show(buf, buf->priv.x.win, x, y,
						    w, h);
		  evas_software_x11_x_software_x11_output_buffer_paste(xob, buf->priv.x.win,
						     buf->priv.x.gc,
						     buf->h - y - h, x);
	       }
	     else if (buf->rot == 90)
	       {
		  conv_func(src_data, data,
			    buf->w - w,
			    bpl /
			    ((evas_software_x11_x_software_x11_output_buffer_depth(xob) / 8)) - h,
			    h, w, x, y, NULL);
		  if (buf->priv.debug)
		     evas_software_x11_outbuf_software_x11_debug_show(buf, buf->priv.x.win, x, y,
						    w, h);
		  evas_software_x11_x_software_x11_output_buffer_paste(xob, buf->priv.x.win,
						     buf->priv.x.gc, y,
						     buf->w - x - w);
	       }
	  }
     }
   else
     {
	evas_software_x11_x_software_x11_output_buffer_free(xob);
	return;
     }
   evas_software_x11_x_software_x11_output_buffer_free(xob);
}

RGBA_Image         *
evas_software_x11_outbuf_software_x11_new_region_for_update(Outbuf * buf, int x, int y, int w,
					  int h, int *cx, int *cy, int *cw,
					  int *ch)
{
   if (buf->priv.back_buf)
     {
	*cx = x;
	*cy = y;
	*cw = w;
	*ch = h;
	return buf->priv.back_buf;
     }
   else
     {
	RGBA_Image         *im;

	*cx = 0;
	*cy = 0;
	*cw = w;
	*ch = h;
	im = evas_common_image_create(w, h);
	if (buf->priv.x.mask)
	  {
	     im->flags |= RGBA_IMAGE_HAS_ALPHA;
	     memset(im->image->data, 0, w * h * sizeof(DATA32));
	  }
	return im;
     }
   return NULL;
}

void
evas_software_x11_outbuf_software_x11_free_region_for_update(Outbuf * buf, RGBA_Image * update)
{
   if (update != buf->priv.back_buf)
      evas_common_image_free(update);
}

void
evas_software_x11_outbuf_software_x11_push_updated_region(Outbuf * buf, RGBA_Image * update,
					int x, int y, int w, int h)
{
   if (buf->priv.back_buf)
     {
	if (update != buf->priv.back_buf)
	   evas_common_blit_rectangle(update, buf->priv.back_buf, 0, 0, w, h, x, y);
	evas_software_x11_outbuf_software_x11_update(buf, x, y, w, h);
     }
   else
     {
	Gfx_Func_Convert    conv_func;
	DATA8              *data;
	X_Output_Buffer    *xob;
	int                 bpl;
	int                 use_shm = 1;
	void               *orig_data;
	int                 direct_data = 0;

	use_shm = buf->priv.x.shm;
	if (buf->perf)
	  {
	     if ((w * h) < buf->perf->min_shm_image_pixel_count)
		use_shm = 0;
	  }
	else
	  {
	     if ((w * h) < (200 * 200))
		use_shm = 0;
	  }
	xob = NULL;
	orig_data = update->image->data;
	/* Punch thru - if our output format is our */
	/* input format - then avoid convert since we used */
	/* the image data directly */
	if ((buf->rot == 0) &&
	    (buf->priv.mask.r == 0xff0000) &&
	    (buf->priv.mask.g == 0x00ff00) &&
	    (buf->priv.mask.b == 0x0000ff) &&
	    (!use_shm) && (w == update->image->w) && (h == update->image->h))
	   direct_data = 1;
	if (!direct_data)
	   orig_data = NULL;
	if (buf->rot == 0)
	   xob = evas_software_x11_x_software_x11_output_buffer_new(buf->priv.x.disp,
						  buf->priv.x.vis,
						  buf->priv.x.depth, w,
						  h, use_shm, orig_data);
	else if (buf->rot == 270)
	   xob = evas_software_x11_x_software_x11_output_buffer_new(buf->priv.x.disp,
						  buf->priv.x.vis,
						  buf->priv.x.depth, h,
						  w, use_shm, orig_data);
	else if (buf->rot == 90)
	   xob = evas_software_x11_x_software_x11_output_buffer_new(buf->priv.x.disp,
						  buf->priv.x.vis,
						  buf->priv.x.depth, h,
						  w, use_shm, orig_data);
	if (!xob)
	   return;
	data = evas_software_x11_x_software_x11_output_buffer_data(xob, &bpl);
	conv_func = NULL;
	if (buf->priv.pal)
	  {
	     if (buf->rot == 0)
		conv_func = evas_common_convert_func_get(0, w, h,
					     evas_software_x11_x_software_x11_output_buffer_depth
					     (xob), buf->priv.mask.r,
					     buf->priv.mask.g, buf->priv.mask.b,
					     buf->priv.pal->colors, buf->rot);
	     else if (buf->rot == 270)
		conv_func = evas_common_convert_func_get(0, h, w,
					     evas_software_x11_x_software_x11_output_buffer_depth
					     (xob), buf->priv.mask.r,
					     buf->priv.mask.g, buf->priv.mask.b,
					     buf->priv.pal->colors, buf->rot);
	     else if (buf->rot == 90)
		conv_func = evas_common_convert_func_get(0, h, w,
					     evas_software_x11_x_software_x11_output_buffer_depth
					     (xob), buf->priv.mask.r,
					     buf->priv.mask.g, buf->priv.mask.b,
					     buf->priv.pal->colors, buf->rot);
	  }
	else
	  {
	     if (buf->rot == 0)
		conv_func = evas_common_convert_func_get(0, w, h,
					     evas_software_x11_x_software_x11_output_buffer_depth
					     (xob), buf->priv.mask.r,
					     buf->priv.mask.g, buf->priv.mask.b,
					     PAL_MODE_NONE, buf->rot);
	     else if (buf->rot == 270)
		conv_func = evas_common_convert_func_get(0, h, w,
					     evas_software_x11_x_software_x11_output_buffer_depth
					     (xob), buf->priv.mask.r,
					     buf->priv.mask.g, buf->priv.mask.b,
					     PAL_MODE_NONE, buf->rot);
	     else if (buf->rot == 90)
		conv_func = evas_common_convert_func_get(0, h, w,
					     evas_software_x11_x_software_x11_output_buffer_depth
					     (xob), buf->priv.mask.r,
					     buf->priv.mask.g, buf->priv.mask.b,
					     PAL_MODE_NONE, buf->rot);
	  }
	if (conv_func)
	  {
	     DATA32             *src_data;
	     X_Output_Buffer    *mxob = NULL;

	     if (buf->priv.x.mask)
	       {
		  if (buf->rot == 0)
		     mxob = evas_software_x11_x_software_x11_output_buffer_new(buf->priv.x.disp,
							     buf->priv.x.vis,
							     1, w, h, 0, NULL);
		  else if (buf->rot == 270)
		     mxob = evas_software_x11_x_software_x11_output_buffer_new(buf->priv.x.disp,
							     buf->priv.x.vis,
							     1, h, w, 0, NULL);
		  else if (buf->rot == 90)
		     mxob = evas_software_x11_x_software_x11_output_buffer_new(buf->priv.x.disp,
							     buf->priv.x.vis,
							     1, h, w, 0, NULL);
	       }
	     src_data = update->image->data;
	     if (buf->priv.pal)
	       {
		  if (buf->rot == 0)
		    {
		       if (!direct_data)
			  conv_func(src_data, data,
				    0,
				    bpl /
				    ((evas_software_x11_x_software_x11_output_buffer_depth(xob) /
				      8)) - w, w, h, x, y,
				    buf->priv.pal->lookup);
		       if (buf->priv.debug)
			  evas_software_x11_outbuf_software_x11_debug_show(buf, buf->priv.x.win,
							 x, y, w, h);
		       evas_software_x11_x_software_x11_output_buffer_paste(xob, buf->priv.x.win,
							  buf->priv.x.gc, x, y);
		       if (mxob)
			 {
			    int                 yy;

			    for (yy = 0; yy < h; yy++)
			       evas_software_x11_x_software_x11_write_mask_line(mxob,
							      src_data +
							      (yy * w), w, yy);
			    evas_software_x11_x_software_x11_output_buffer_paste(mxob,
							       buf->priv.x.mask,
							       buf->priv.x.gcm,
							       x, y);
			 }
		    }
		  else if (buf->rot == 270)
		    {
		       if (!direct_data)
			  conv_func(src_data, data,
				    0,
				    bpl /
				    ((evas_software_x11_x_software_x11_output_buffer_depth(xob) /
				      8)) - h, h, w, x, y,
				    buf->priv.pal->lookup);
		       if (buf->priv.debug)
			  evas_software_x11_outbuf_software_x11_debug_show(buf, buf->priv.x.win,
							 x, y, w, h);
		       evas_software_x11_x_software_x11_output_buffer_paste(xob, buf->priv.x.win,
							  buf->priv.x.gc,
							  buf->h - y - h, x);
		       if (mxob)
			 {
			    int                 yy;

/*			    for (yy = 0; yy < h; yy++)*/
/*			      evas_software_x11_x_software_x11_write_mask_line(mxob, src_data + (yy * w), w, yy);*/
			    evas_software_x11_x_software_x11_output_buffer_paste(mxob,
							       buf->priv.x.mask,
							       buf->priv.x.gcm,
							       buf->h - y - h,
							       x);
			 }
		    }
		  else if (buf->rot == 90)
		    {
		       if (!direct_data)
			  conv_func(src_data, data,
				    0,
				    bpl /
				    ((evas_software_x11_x_software_x11_output_buffer_depth(xob) /
				      8)) - h, h, w, x, y,
				    buf->priv.pal->lookup);
		       if (buf->priv.debug)
			  evas_software_x11_outbuf_software_x11_debug_show(buf, buf->priv.x.win,
							 x, y, w, h);
		       evas_software_x11_x_software_x11_output_buffer_paste(xob, buf->priv.x.win,
							  buf->priv.x.gc, y,
							  buf->w - x - w);
		       if (mxob)
			 {
			    int                 yy;

/*			    for (yy = 0; yy < h; yy++)*/
/*			      evas_software_x11_x_software_x11_write_mask_line(mxob, src_data + (yy * w), w, yy);*/
			    evas_software_x11_x_software_x11_output_buffer_paste(mxob,
							       buf->priv.x.mask,
							       buf->priv.x.gcm,
							       y,
							       buf->w - x - w);
			 }
		    }
	       }
	     else
	       {
		  if (buf->rot == 0)
		    {
		       if (!direct_data)
			  conv_func(src_data, data,
				    0,
				    bpl /
				    ((evas_software_x11_x_software_x11_output_buffer_depth(xob) /
				      8)) - w, w, h, x, y, NULL);
		       if (buf->priv.debug)
			  evas_software_x11_outbuf_software_x11_debug_show(buf, buf->priv.x.win,
							 x, y, w, h);
		       evas_software_x11_x_software_x11_output_buffer_paste(xob, buf->priv.x.win,
							  buf->priv.x.gc, x, y);
		       if (mxob)
			 {
			    int                 yy;

			    for (yy = 0; yy < h; yy++)
			       evas_software_x11_x_software_x11_write_mask_line(mxob,
							      src_data +
							      (yy * w), w, yy);
			    evas_software_x11_x_software_x11_output_buffer_paste(mxob,
							       buf->priv.x.mask,
							       buf->priv.x.gcm,
							       x, y);
			 }
		    }
		  else if (buf->rot == 270)
		    {
		       if (!direct_data)
			  conv_func(src_data, data,
				    0,
				    bpl /
				    ((evas_software_x11_x_software_x11_output_buffer_depth(xob) /
				      8)) - h, h, w, x, y, NULL);
		       if (buf->priv.debug)
			  evas_software_x11_outbuf_software_x11_debug_show(buf, buf->priv.x.win,
							 x, y, w, h);
		       evas_software_x11_x_software_x11_output_buffer_paste(xob, buf->priv.x.win,
							  buf->priv.x.gc,
							  buf->h - y - h, x);
		       if (mxob)
			 {
			    int                 yy;

/*			    for (yy = 0; yy < h; yy++)*/
/*			      evas_software_x11_x_software_x11_write_mask_line(mxob, src_data + (yy * w), w, yy);*/
			    evas_software_x11_x_software_x11_output_buffer_paste(mxob,
							       buf->priv.x.mask,
							       buf->priv.x.gcm,
							       buf->h - y - h,
							       x);
			 }
		    }
		  else if (buf->rot == 90)
		    {
		       if (!direct_data)
			  conv_func(src_data, data,
				    0,
				    bpl /
				    ((evas_software_x11_x_software_x11_output_buffer_depth(xob) /
				      8)) - h, h, w, x, y, NULL);
		       if (buf->priv.debug)
			  evas_software_x11_outbuf_software_x11_debug_show(buf, buf->priv.x.win,
							 x, y, w, h);
		       evas_software_x11_x_software_x11_output_buffer_paste(xob, buf->priv.x.win,
							  buf->priv.x.gc, y,
							  buf->w - x - w);
		       if (mxob)
			 {
			    int                 yy;

/*			    for (yy = 0; yy < h; yy++)*/
/*			      evas_software_x11_x_software_x11_write_mask_line(mxob, src_data + (yy * w), w, yy);*/
			    evas_software_x11_x_software_x11_output_buffer_paste(mxob,
							       buf->priv.x.mask,
							       buf->priv.x.gcm,
							       y,
							       buf->w - x - w);
			 }
		    }
	       }
	     if (mxob)
		evas_software_x11_x_software_x11_output_buffer_free(mxob);
	  }
	else
	  {
	     evas_software_x11_x_software_x11_output_buffer_free(xob);
	     return;
	  }
	evas_software_x11_x_software_x11_output_buffer_free(xob);
     }
}

void
evas_software_x11_outbuf_software_x11_reconfigure(Outbuf * buf, int w, int h, int rot,
				Outbuf_Depth depth)
{
   if ((w == buf->w) && (h == buf->h) &&
       (rot == buf->rot) && (depth == buf->depth))
      return;
   buf->w = w;
   buf->h = h;
   buf->rot = rot;
   if (buf->priv.back_buf)
     {
	evas_common_image_free(buf->priv.back_buf);
	buf->priv.back_buf = NULL;
     }
   if (buf->priv.x.disp)
     {
	if (buf->priv.x.depth < 24)
	   buf->priv.back_buf = evas_common_image_create(buf->w, buf->h);
     }
}

int
evas_software_x11_outbuf_software_x11_get_width(Outbuf * buf)
{
   return buf->w;
}

int
evas_software_x11_outbuf_software_x11_get_height(Outbuf * buf)
{
   return buf->h;
}

Outbuf_Depth
evas_software_x11_outbuf_software_x11_get_depth(Outbuf * buf)
{
   return buf->depth;
}

int
evas_software_x11_outbuf_software_x11_get_rot(Outbuf * buf)
{
   return buf->rot;
}

int
evas_software_x11_outbuf_software_x11_get_have_backbuf(Outbuf * buf)
{
   if (buf->priv.back_buf)
      return 1;
   return 0;
}

void
evas_software_x11_outbuf_software_x11_set_have_backbuf(Outbuf * buf, int have_backbuf)
{
   if (buf->priv.back_buf)
     {
	if (have_backbuf)
	   return;
	evas_common_image_free(buf->priv.back_buf);
	buf->priv.back_buf = NULL;
     }
   else
     {
	if (!have_backbuf)
	   return;
	if (buf->priv.x.disp)
	  {
	     if (buf->priv.x.depth < 24)
		buf->priv.back_buf = evas_common_image_create(buf->w, buf->h);
	  }
     }
}

void
evas_software_x11_outbuf_software_x11_drawable_set(Outbuf * buf, Drawable draw)
{
   XGCValues           gcv;

   if (buf->priv.x.win == draw)
      return;

   if (buf->priv.x.gc)
     {
	XFreeGC(buf->priv.x.disp, buf->priv.x.gc);
	buf->priv.x.gc = NULL;
     }

   buf->priv.x.win = draw;
   buf->priv.x.gc = XCreateGC(buf->priv.x.disp, buf->priv.x.win, 0, &gcv);
}

void
evas_software_x11_outbuf_software_x11_mask_set(Outbuf * buf, Pixmap mask)
{
   XGCValues           gcv;

   if (buf->priv.x.mask == mask)
      return;

   if (buf->priv.x.gcm)
     {
	XFreeGC(buf->priv.x.disp, buf->priv.x.gcm);
	buf->priv.x.gcm = NULL;
     }

   buf->priv.x.mask = mask;
   if (buf->priv.x.mask)
      buf->priv.x.gcm = XCreateGC(buf->priv.x.disp, buf->priv.x.mask, 0, &gcv);
}

void
evas_software_x11_outbuf_software_x11_debug_set(Outbuf * buf, int debug)
{
   buf->priv.debug = debug;
}

void
evas_software_x11_outbuf_software_x11_debug_show(Outbuf * buf, Drawable draw, int x, int y, int w,
			       int h)
{
   int                 i;
   int                 screen_num = 0;

   {
      int                 wx, wy;
      unsigned int        ww, wh, bd, dp;
      Window              wdum, root;
      XWindowAttributes   wattr;

      XGetGeometry(buf->priv.x.disp, draw, &root, &wx, &wy, &ww, &wh, &bd, &dp);
      XGetGeometry(buf->priv.x.disp, root, &wdum, &wx, &wy, &ww, &wh, &bd, &dp);
      XGetWindowAttributes(buf->priv.x.disp, root, &wattr);
      screen_num = XScreenNumberOfScreen(wattr.screen);
   }
   for (i = 0; i < 10; i++)
     {
	XImage             *xim;

	XSetForeground(buf->priv.x.disp, buf->priv.x.gc,
		       BlackPixel(buf->priv.x.disp, screen_num));
	XFillRectangle(buf->priv.x.disp, draw, buf->priv.x.gc, x, y, w, h);
	XSync(buf->priv.x.disp, False);
	xim =
	   XGetImage(buf->priv.x.disp, draw, x, y, w, h, 0xffffffff, ZPixmap);
	if (xim)
	   XDestroyImage(xim);
	XSync(buf->priv.x.disp, False);
	XSetForeground(buf->priv.x.disp, buf->priv.x.gc,
		       WhitePixel(buf->priv.x.disp, screen_num));
	XFillRectangle(buf->priv.x.disp, draw, buf->priv.x.gc, x, y, w, h);
	XSync(buf->priv.x.disp, False);
	xim =
	   XGetImage(buf->priv.x.disp, draw, x, y, w, h, 0xffffffff, ZPixmap);
	if (xim)
	   XDestroyImage(xim);
	XSync(buf->priv.x.disp, False);
     }
}
