/* -------------------------------------------------------------------- */
/* LINUX FBCON FRAMEBUFFER UTILITY CODE                                 */
/* makes setting up the framebuffer easy. Also makes it eays to port to */
/* some other system if needed.                                         */
/* Copyright (c) 1999 - Carsten Haitzler (The Rasterman)                */
/* -------------------------------------------------------------------- */
#include "evas_common.h"
#include "evas_fb.h"

#include <signal.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <sys/user.h>

#define FB_ACTIVE    0
#define FB_REL_REQ   1
#define FB_INACTIVE  2
#define FB_ACQ_REQ   3

/* -------------------------------------------------------------------- */
/* internal variables                                                   */

static struct fb_fix_screeninfo  fb_fix;
static int                       fb, tty;
static int                       bpp, depth;
static int                       orig_vt_no = 0;
static int                       kd_mode;
static struct vt_mode            vt_omode;
static struct fb_var_screeninfo  fb_ovar;
static unsigned short            ored[256], ogreen[256], oblue[256];
static unsigned short            red[256],  green[256],  blue[256];
static struct fb_cmap            ocmap = { 0, 256, ored, ogreen, oblue, NULL };
static struct fb_cmap            cmap  = { 0, 256, red,  green,  blue, NULL };

/* -------------------------------------------------------------------- */
/* internal function prototypes                                         */

static void fb_cleanup(void);
//static void fb_cleanup_fork(void);
static void fb_setvt(int vtno);
static void fb_init_palette_332(FB_Mode *mode);
static void fb_init_palette_linear(FB_Mode *mode);

/* -------------------------------------------------------------------- */
/* palette setting                                                      */

static void
fb_init_palette_332(FB_Mode *mode)
{
  int r, g, b, i;

  if (mode->fb_var.bits_per_pixel != 8)
    return;
  i = 0;

  if (ioctl(fb, FBIOGETCMAP, &cmap) == -1)
    perror("ioctl FBIOGETCMAP");

  /* generate the palette */
  for (r = 0; r < 8; r++)
    {
      for (g = 0; g < 8; g++)
	{
	  for (b = 0; b < 4; b++)
	    {
	      int val;

	      val = (r << 5) | (r << 2) | (r >> 1);
	      red[i] = (val << 8) | val;
	      val = (g << 5) | (g << 2) | (g >> 1);
	      green[i] = (val << 8) | val;
	      val = (b << 6) | (b << 4) | (b << 2) | (b);
	      blue[i] = (val << 8) | val;
	      i++;
	    }
	}
    }

  /* set colormap */
  if (ioctl(fb, FBIOPUTCMAP, &cmap) == -1)
    perror("ioctl FBIOPUTCMAP");

}

static void
fb_init_palette_linear(FB_Mode *mode)
{
  int i;

  if (mode->fb_var.bits_per_pixel != 8)
    return;

  if (ioctl(fb, FBIOGETCMAP, &cmap) == -1)
    perror("ioctl FBIOGETCMAP");

  /* generate the palette */
  for (i = 0; i < 256; i++)
    red[i] = (i << 8) | i;
  for (i = 0; i < 256; i++)
    green[i] = (i << 8) | i;
  for (i = 0; i < 256; i++)
    blue[i] = (i << 8) | i;

  /* set colormap */
  if (ioctl(fb, FBIOPUTCMAP, &cmap) == -1)
    perror("ioctl FBIOPUTCMAP");

}

/* -------------------------------------------------------------------- */
/* initialisation & cleanup                                             */

FB_Mode *
fb_list_modes(int *num_return)
{
  FILE *f;
  char line[256], label[256], value[256];
  FB_Mode *modes = NULL;
  int num;

  num = 0;
  f = fopen("/etc/fb.modes","r");
  if (!f)
    {
      *num_return = 0;
      return NULL;
    }
  while (fgets(line, sizeof(line) - 1, f))
    {
      if (sscanf(line, "mode \"%250[^\"]\"", label) == 1)
	{
	  char f1[32], f2[32], f3[32], f4[32];

	  f1[0] = 0; f2[0] = 0; f3[0] = 0; f4[0] = 0;
	  sscanf(label, "%30[^x]x%30[^-]-%30[^-]-%30s", f1, f2, f3, f4);
	  if ((f1[0]) && (f2[0]))
	    {
	      int geometry = 0;
	      int timings = 0;

	      num++;
	      modes = realloc(modes, num * sizeof(FB_Mode));
	      modes[num - 1].width = atoi(f1);
	      modes[num - 1].height = atoi(f2);
	      if (f3[0])
		modes[num - 1].refresh = atoi(f3);
	      else
		modes[num - 1].refresh = 0;
	      modes[num - 1].fb_var.sync = 0;
	      while ((fgets(line, sizeof(line) - 1, f)) &&
		     (!strstr(line, "endmode")))
		{

		  if (sscanf(line," geometry %i %i %i %i %i",
			     &modes[num - 1].fb_var.xres,
			     &modes[num - 1].fb_var.yres,
			     &modes[num - 1].fb_var.xres_virtual,
			     &modes[num - 1].fb_var.yres_virtual,
			     &modes[num - 1].fb_var.bits_per_pixel) == 5)
		    geometry = 1;
		  if (sscanf(line," timings %i %i %i %i %i %i %i",
			     &modes[num - 1].fb_var.pixclock,
			     &modes[num - 1].fb_var.left_margin,
			     &modes[num - 1].fb_var.right_margin,
			     &modes[num - 1].fb_var.upper_margin,
			     &modes[num - 1].fb_var.lower_margin,
			     &modes[num - 1].fb_var.hsync_len,
			     &modes[num - 1].fb_var.vsync_len) == 7)
		    timings = 1;
		  if ((sscanf(line, " hsync %15s", value) == 1) &&
		      (!strcmp(value,"high")))
		    modes[num - 1].fb_var.sync |= FB_SYNC_HOR_HIGH_ACT;
		  if ((sscanf(line, " vsync %15s", value) == 1) &&
		      (!strcmp(value,"high")))
		    modes[num - 1].fb_var.sync |= FB_SYNC_VERT_HIGH_ACT;
		  if ((sscanf(line, " csync %15s", value) == 1) &&
		      (!strcmp(value,"high")))
		    modes[num - 1].fb_var.sync |= FB_SYNC_COMP_HIGH_ACT;
		  if ((sscanf(line, " extsync %15s", value) == 1) &&
		      (!strcmp(value,"true")))
		    modes[num - 1].fb_var.sync |= FB_SYNC_EXT;
		  if ((sscanf(line, " laced %15s", value) == 1) &&
		      (!strcmp(value,"true")))
		    modes[num - 1].fb_var.vmode |= FB_VMODE_INTERLACED;
		  if ((sscanf(line, " double %15s",value) == 1) &&
		      (!strcmp(value,"true")))
		    modes[num - 1].fb_var.vmode |= FB_VMODE_DOUBLE;
		}
	      if ((!geometry) || (!timings))
		{
		  num--;
		  if (num == 0)
		    {
		      free(modes);
		      modes = NULL;
		    }
		}
	      else
		{
		  modes[num - 1].fb_var.xoffset = 0;
		  modes[num - 1].fb_var.yoffset = 0;
		}
	    }

	}
    }
  fclose(f);
  *num_return = num;
  return modes;
}

FB_Mode *
fb_setmode(int width, int height, int depth, int refresh)
{
  FB_Mode *modes, *mode = NULL;
  int      i, num_modes;

  modes = fb_list_modes(&num_modes);
  if (modes)
    {
      for (i = 0; i < num_modes; i++)
	{
	  if ((modes[i].width == width) &&
	      (modes[i].height == height) &&
	      (!depth || modes[i].fb_var.bits_per_pixel == depth) &&
	      (modes[i].refresh == refresh))
	    {
	      if (depth) modes[i].fb_var.bits_per_pixel = depth;

	      if (ioctl(fb, FBIOPUT_VSCREENINFO, &modes[i].fb_var) == -1)
		perror("ioctl FBIOPUT_VSCREENINFO");

	      free(modes);
	      return fb_getmode();
	    }
	}
      free(modes);
    }
  return mode;
}

FB_Mode *
fb_changedepth(FB_Mode *cur_mode, int depth)
{
  cur_mode->fb_var.bits_per_pixel = depth;

  if (ioctl(fb, FBIOPUT_VSCREENINFO, &cur_mode->fb_var) == -1)
    perror("ioctl FBIOPUT_VSCREENINFO");

  free(cur_mode);
  return fb_getmode();
}

FB_Mode *
fb_changeres(FB_Mode *cur_mode, int width, int height, int refresh)
{
  FB_Mode *modes;
  int      i, num_modes;

  modes = fb_list_modes(&num_modes);
  if (modes)
    {
      for (i = 0; i < num_modes; i++)
	{
	  if ((modes[i].width == width) &&
	      (modes[i].height == height) &&
	      (modes[i].refresh == refresh))
	    {
	      modes[i].fb_var.bits_per_pixel = cur_mode->depth;

	      if (ioctl(fb, FBIOPUT_VSCREENINFO, &modes[i].fb_var) == -1)
		perror("ioctl FBIOPUT_VSCREENINFO");

	      free(modes);
	      free(cur_mode);
	      return fb_getmode();
	    }
	}
      free(modes);
    }
  return cur_mode;
}

FB_Mode *
fb_changemode(FB_Mode *cur_mode, int width, int height, int depth, int refresh)
{
  FB_Mode *modes;
  int      i, num_modes;

  modes = fb_list_modes(&num_modes);
  if (modes)
    {
      for (i = 0; i < num_modes; i++)
	{
	  if ((modes[i].width == width) &&
	      (modes[i].height == height) &&
	      (!depth || modes[i].fb_var.bits_per_pixel == depth) &&
	      (modes[i].refresh == refresh))
	    {
	      if (depth) modes[i].fb_var.bits_per_pixel = depth;

	      if (ioctl(fb, FBIOPUT_VSCREENINFO, &modes[i].fb_var) == -1)
		perror("ioctl FBIOPUT_VSCREENINFO");

	      free(modes);
	      free(cur_mode);
	      return fb_getmode();
	    }
	}
      free(modes);
    }
  return cur_mode;
}

FB_Mode *
fb_getmode(void)
{
  FB_Mode *mode = NULL;
  int      hpix, lines, clockrate;

  mode = malloc(sizeof(FB_Mode));
  /* look what we have now ... */

  if (ioctl(fb, FBIOGET_VSCREENINFO, &mode->fb_var) == -1)
    {
      perror("ioctl FBIOGET_VSCREENINFO");
      exit(1);
    }

  mode->width = mode->fb_var.xres;
  mode->height = mode->fb_var.yres;
  hpix =
    mode->fb_var.left_margin +
    mode->fb_var.xres +
    mode->fb_var.right_margin +
    mode->fb_var.hsync_len;
  lines =
    mode->fb_var.upper_margin +
    mode->fb_var.yres +
    mode->fb_var.lower_margin +
    mode->fb_var.vsync_len;
   if (mode->fb_var.pixclock > 0)
     clockrate = 1000000 / mode->fb_var.pixclock;
   else
     clockrate = 0;
   if ((lines > 0) && (hpix > 0))
     mode->refresh = clockrate * 1000000 / (lines * hpix);
  switch (mode->fb_var.bits_per_pixel)
    {
     case 1:
      bpp = 1;
      depth = 1;
      break;
     case 4:
      bpp = 1;
      depth = 4;
      break;
     case 8:
      bpp = 1;
      depth = 8;
      break;
     case 15:
     case 16:
      if (mode->fb_var.green.length == 6)
	depth = 16;
      else
	depth = 15;
      bpp = 2;
      break;
     case 24:
      depth = 24;
      bpp = mode->fb_var.bits_per_pixel / 8;
      break;
     case 32:
      depth = 32;
      bpp = mode->fb_var.bits_per_pixel / 8;
      break;
     default:
      fprintf(stderr, "Cannot handle framebuffer of depth %i\n",
	      mode->fb_var.bits_per_pixel);
      fb_cleanup();
       free(mode);
      return NULL;
  }
  mode->depth = depth;
  mode->bpp = bpp;
  if (mode->depth == 8)
    fb_init_palette_332(mode);
  else
    fb_init_palette_linear(mode);
  return mode;
}

static void
fb_setvt(int vtno)
{
  struct vt_stat vts;
  char vtname[32];

  if (vtno < 0)
    {

      if ((ioctl(tty,VT_OPENQRY, &vtno) == -1))
	{
	  perror("ioctl VT_OPENQRY");
	  exit(1);
	}
      if (vtno <= 0 )
	{
	  perror("ioctl VT_OPENQRY vtno <= 0");
	  exit(1);
	}

    }
  vtno &= 0xff;
  sprintf(vtname, "/dev/tty%i", vtno);
  chown(vtname, getuid(), getgid());
  if (access(vtname,R_OK | W_OK) == -1)
    {
      fprintf(stderr,"access %s: %s\n",vtname,strerror(errno));
      exit(1);
    }
  open(vtname,O_RDWR);

  if (ioctl(tty, VT_GETSTATE, &vts) == -1)
    {
      perror("ioctl VT_GETSTATE");
      exit(1);
    }

  orig_vt_no = vts.v_active;
/*
  if (ioctl(tty, VT_ACTIVATE, vtno) == -1)
    {
      perror("ioctl VT_ACTIVATE");
      exit(1);
    }
  if (ioctl(tty, VT_WAITACTIVE, vtno) == -1)
    {
      perror("ioctl VT_WAITACTIVE");
      exit(1);
    }
*/

}

void
fb_init(int vt, int device)
{
   char dev[32];

   tty = 0;
#if 0
   if (vt != 0) fb_setvt(vt);
#endif
   sprintf(dev, "/dev/fb/%i", device);
   fb = open(dev, O_RDWR);
   if ( fb == -1 )
     {
       sprintf(dev, "/dev/fb%i", device);
       fb = open(dev, O_RDWR);
     }
   if (fb == -1)
     {
	fprintf(stderr,"open %s: %s\n", dev, strerror(errno));
	fb_cleanup();
	exit(1);
     }

   if (ioctl(fb, FBIOGET_VSCREENINFO, &fb_ovar) == -1)
     {
	perror("ioctl FBIOGET_VSCREENINFO");
	exit(1);
     }
   if (ioctl(fb, FBIOGET_FSCREENINFO, &fb_fix) == -1)
     {
	perror("ioctl FBIOGET_FSCREENINFO");
	exit(1);
     }

   if ((fb_ovar.bits_per_pixel == 8) ||
       (fb_fix.visual == FB_VISUAL_DIRECTCOLOR))
     {

	if (ioctl(fb,FBIOGETCMAP , &ocmap) == -1)
	  {
	     perror("ioctl FBIOGETCMAP");
	     exit(1);
	  }

     }
#if 0
   if (isatty(0))
      tty = 0;
   else if ((tty = open("/dev/tty",O_RDWR)) == -1)
     {
	fprintf(stderr,"open %s: %s\n", "/dev/tty", strerror(errno));
	exit(1);
     }
   if (tty)
     {
	if (ioctl(tty, KDGETMODE, &kd_mode) == -1)
	  {
	     perror("ioctl KDGETMODE");
	     exit(1);
	  }
	if (ioctl(tty, VT_GETMODE, &vt_omode) == -1)
	  {
	     perror("ioctl VT_GETMODE");
	     exit(1);
	  }
     }
#endif
}

int
fb_postinit(FB_Mode *mode)
{

  if (ioctl(fb,FBIOGET_FSCREENINFO, &fb_fix) == -1)
    {
      perror("ioctl FBIOGET_FSCREENINFO");
      fb_cleanup();
      exit(1);
    }

  if (fb_fix.type != FB_TYPE_PACKED_PIXELS)
    {
      fprintf(stderr,"can handle only packed pixel frame buffers\n");
      fb_cleanup();
      exit(1);
    }
  mode->mem_offset = (unsigned)(fb_fix.smem_start) & (getpagesize()-1);
  mode->mem = (unsigned char *)mmap(NULL, fb_fix.smem_len + mode->mem_offset,
				 PROT_WRITE | PROT_READ, MAP_SHARED, fb, 0);
  if (mode->mem == MAP_FAILED)
    {
      perror("mmap");
      fb_cleanup();
  }
  /* move viewport to upper left corner */
  if ((mode->fb_var.xoffset != 0) || (mode->fb_var.yoffset != 0))
    {
      mode->fb_var.xoffset = 0;
      mode->fb_var.yoffset = 0;

      if (ioctl(fb, FBIOPAN_DISPLAY, &(mode->fb_var)) == -1)
	{
	  perror("ioctl FBIOPAN_DISPLAY");
	  fb_cleanup();
	}
    }
#if 0
   if (tty)
     {
	if (ioctl(tty,KDSETMODE, KD_GRAPHICS) == -1)
	  {
	     perror("ioctl KDSETMODE");
	     fb_cleanup();
	  }
     }
#endif
  mode->fb_fd = fb;
  return fb;
}

static void
fb_cleanup(void)
{
  /* restore console */

  if (ioctl(fb, FBIOPUT_VSCREENINFO, &fb_ovar) == -1)
    perror("ioctl FBIOPUT_VSCREENINFO");
  if (ioctl(fb, FBIOGET_FSCREENINFO, &fb_fix) == -1)
    perror("ioctl FBIOGET_FSCREENINFO");

  if ((fb_ovar.bits_per_pixel == 8) ||
      (fb_fix.visual == FB_VISUAL_DIRECTCOLOR))
    {

      if (ioctl(fb, FBIOPUTCMAP, &ocmap) == -1)
	perror("ioctl FBIOPUTCMAP");

    }
  close(fb);


   if (tty)
     {
	if (ioctl(tty, KDSETMODE, kd_mode) == -1)
	  perror("ioctl KDSETMODE");
	if (ioctl(tty, VT_SETMODE, &vt_omode) == -1)
	  perror("ioctl VT_SETMODE");
/*	if ((ioctl(tty, VT_ACTIVATE, orig_vt_no) == -1) && (orig_vt_no))
	  perror("ioctl VT_ACTIVATE");
*/     }

  close(tty);
}
