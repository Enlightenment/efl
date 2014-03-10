/* -------------------------------------------------------------------- */
/* LINUX FBCON FRAMEBUFFER UTILITY CODE                                 */
/* makes setting up the framebuffer easy. Also makes it eays to port to */
/* some other system if needed.                                         */
/* Copyright (c) 1999 - Carsten Haitzler (The Rasterman)                */
/* -------------------------------------------------------------------- */
#include "evas_common_private.h"
#include "evas_fb.h"

#include <sys/ioctl.h>
#include <sys/wait.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <sys/user.h>

extern int _evas_engine_fb_log_dom;
#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_fb_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_fb_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_engine_fb_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_fb_log_dom, __VA_ARGS__)

#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_evas_engine_fb_log_dom, __VA_ARGS__)

#define WILL_LOG(_level) \
  (eina_log_domain_registered_level_get(_evas_engine_fb_log_dom) >= _level)


#define FB_ACTIVE    0
#define FB_REL_REQ   1
#define FB_INACTIVE  2
#define FB_ACQ_REQ   3

/* -------------------------------------------------------------------- */
/* internal variables                                                   */

static struct fb_fix_screeninfo  fb_fix;
static int                       fb = -1, tty = -1;
static int                       bpp, depth;
//static int                       orig_vt_no = 0;
static int                       kd_mode;
static struct vt_mode            vt_omode;
static struct fb_var_screeninfo  fb_ovar;
static unsigned short            ored[256], ogreen[256], oblue[256];
static unsigned short            red[256],  green[256],  blue[256];
static struct fb_cmap            ocmap = { 0, 256, ored, ogreen, oblue, NULL };
static struct fb_cmap            cmap  = { 0, 256, red,  green,  blue, NULL };

/* -------------------------------------------------------------------- */
/* internal function prototypes                                         */

//static void fb_cleanup_fork(void);
//static void fb_setvt(int vtno);
static void fb_init_palette_332(FB_Mode *mode);
static void fb_init_palette_linear(FB_Mode *mode);

static char *
fb_cmap_str_convert(const struct fb_cmap *fb_cmap)
{
   Eina_Strbuf *buf = eina_strbuf_new();
   unsigned int i;
   char *ret;

   eina_strbuf_append_printf(buf, "start=%u, len=%u, red={",
                             fb_cmap->start, fb_cmap->len);

   for (i = 0; i < fb_cmap->len; i++)
     {
        if (i == 0)
          eina_strbuf_append_printf(buf, "%hd", fb_cmap->red[i]);
        else
          eina_strbuf_append_printf(buf, ",%hd", fb_cmap->red[i]);
     }

   eina_strbuf_append(buf, "}, green={");
   for (i = 0; i < fb_cmap->len; i++)
     {
        if (i == 0)
          eina_strbuf_append_printf(buf, "%hd", fb_cmap->green[i]);
        else
          eina_strbuf_append_printf(buf, ",%hd", fb_cmap->green[i]);
     }
   eina_strbuf_append(buf, "}, blue={");
   for (i = 0; i < fb_cmap->len; i++)
     {
        if (i == 0)
          eina_strbuf_append_printf(buf, "%hd", fb_cmap->blue[i]);
        else
          eina_strbuf_append_printf(buf, ",%hd", fb_cmap->blue[i]);
     }
   eina_strbuf_append(buf, "}, transp={");
   if (fb_cmap->transp)
     {
        for (i = 0; i < fb_cmap->len; i++)
          {
             if (i == 0)
               eina_strbuf_append_printf(buf, "%hd", fb_cmap->transp[i]);
             else
               eina_strbuf_append_printf(buf, ",%hd", fb_cmap->transp[i]);
          }
     }
   eina_strbuf_append(buf, "}");

   ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);

   return ret;
}

static unsigned int
fb_bitfield_mask_get(const struct fb_bitfield *fbb)
{
   unsigned int i, mask = 0;
   for (i = 0; i < fbb->length; i++)
     mask |= (1 << (fbb->offset + i));
   return mask;
}

static char *
fb_var_str_convert(const struct fb_var_screeninfo *fbv)
{
   Eina_Strbuf *buf = eina_strbuf_new();
   char *ret;

   eina_strbuf_append_printf(buf,
                             "xres=%u, yres=%u, "
                             "xres_virtual=%u, yres_virtual=%u, "
                             "xoffset=%u, yoffset=%u, "
                             "bits_per_pixel=%u, "
                             "grayscale=%u ",
                             fbv->xres, fbv->yres,
                             fbv->xres_virtual, fbv->yres_virtual,
                             fbv->xoffset, fbv->yoffset,
                             fbv->bits_per_pixel,
                             fbv->grayscale);

   if (fbv->grayscale == 0) eina_strbuf_append(buf, "color");
   else if (fbv->grayscale == 1) eina_strbuf_append(buf, "grayscale");
   else eina_strbuf_append_n(buf, (const char *)&(fbv->grayscale), 4);

   eina_strbuf_append_printf(buf,
                             ", "
                             "red={"
                             "offset=%u, length=%u, msb_right=%u, %#010x}, "
                             "green={"
                             "offset=%u, length=%u, msb_right=%u, %#010x}, "
                             "blue={"
                             "offset=%u, length=%u, msb_right=%u, %#010x}, "
                             "transp={"
                             "offset=%u, length=%u, msb_right=%u, %#010x}, "
                             "nonstd=%u, "
                             "activate=%u",
                             fbv->red.offset,
                             fbv->red.length,
                             fbv->red.msb_right,
                             fb_bitfield_mask_get(&(fbv->red)),
                             fbv->green.offset,
                             fbv->green.length,
                             fbv->green.msb_right,
                             fb_bitfield_mask_get(&(fbv->green)),
                             fbv->blue.offset,
                             fbv->blue.length,
                             fbv->blue.msb_right,
                             fb_bitfield_mask_get(&(fbv->blue)),
                             fbv->transp.offset,
                             fbv->transp.length,
                             fbv->transp.msb_right,
                             fb_bitfield_mask_get(&(fbv->transp)),
                             fbv->nonstd,
                             fbv->activate);

#define F2S(f, s) \
   if (fbv->activate & f) eina_strbuf_append(buf, " "#s)
   F2S(FB_ACTIVATE_NOW, "now");
   F2S(FB_ACTIVATE_NXTOPEN, "next-open");
   F2S(FB_ACTIVATE_TEST, "test");
   F2S(FB_ACTIVATE_VBL, "vbl");
   F2S(FB_ACTIVATE_ALL, "all");
   F2S(FB_ACTIVATE_FORCE, "force");
   F2S(FB_ACTIVATE_INV_MODE, "inv-mode");
#undef F2S

   eina_strbuf_append_printf(buf,
                             ", "
                             "height=%u, width=%u, "
                             "accel_flags=%#x, "
                             "pixclock=%u, "
                             "left_margin=%u, right_margin=%u, "
                             "upper_margin=%u, lower_margin=%u, "
                             "hsync_len=%u, vsync_len=%u, "
                             "sync=%u",
                             fbv->height, fbv->width,
                             fbv->accel_flags,
                             fbv->pixclock,
                             fbv->left_margin, fbv->right_margin,
                             fbv->upper_margin, fbv->lower_margin,
                             fbv->hsync_len, fbv->vsync_len,
                             fbv->sync);

#define F2S(f, s) \
   if (fbv->sync & f) eina_strbuf_append(buf, " "#s)
   F2S(FB_SYNC_HOR_HIGH_ACT, "hor-high");
   F2S(FB_SYNC_VERT_HIGH_ACT, "vert-high");
   F2S(FB_SYNC_EXT, "external");
   F2S(FB_SYNC_COMP_HIGH_ACT, "comp-high");
   F2S(FB_SYNC_BROADCAST, "broadcast");
   F2S(FB_SYNC_ON_GREEN, "on-green");
#undef F2S

   eina_strbuf_append_printf(buf,
                             ", "
                             "vmode=%u",
                             fbv->vmode);

#define F2S(f, s) \
   if (fbv->vmode & f) eina_strbuf_append(buf, " "#s)
   F2S(FB_VMODE_NONINTERLACED, "non-interlaced");
   F2S(FB_VMODE_INTERLACED, "interlaced");
   F2S(FB_VMODE_DOUBLE, "double");
   F2S(FB_VMODE_ODD_FLD_FIRST, "interlaced-top-first");
   F2S(FB_VMODE_YWRAP, "yrwap");
   F2S(FB_VMODE_SMOOTH_XPAN, "smooth-xpan");
   F2S(FB_VMODE_CONUPDATE, "conupdate");
#undef F2S

   eina_strbuf_append_printf(buf,
                             ", "
                             "rotate=%u, ",
                             fbv->rotate);

   /* eina_strbuf_append_printf(buf, */
   /*                           ", " */
   /*                           "rotate=%u, " */
   /*                           "colorspace=%u", */
   /*                           fbv->rotate, */
   /*                           fbv->colorspace); */

   /* if (fbv->colorspace) */
   /*   eina_strbuf_append_n(buf, (const char *)&(fbv->colorspace), 4); */

   ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return ret;
}


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
     ERR("could not get colormap: ioctl(%d, FBIOGETCMAP) = %s",
         fb, strerror(errno));
   
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
     {
        const char *errmsg = strerror(errno);
        char *cmap_str = fb_cmap_str_convert(&cmap);
        ERR("could not set colormap: ioctl(%d, FBIOPUTCMAP, {%s}) = %s",
            fb, cmap_str, errmsg);
        free(cmap_str);
     }
}

static void
fb_init_palette_linear(FB_Mode *mode)
{
   int i;
   
   if (mode->fb_var.bits_per_pixel != 8)
      return;
   
   if (ioctl(fb, FBIOGETCMAP, &cmap) == -1)
     ERR("could not get colormap: ioctl(%d, FBIOGETCMAP) = %s",
         fb, strerror(errno));
   
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
fb_list_modes(unsigned int *num_return)
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
                  memset(modes + (num - 1), 0, sizeof(FB_Mode));
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

static Eina_Bool
_fb_vscreeninfo_put(const struct fb_var_screeninfo *fb_var)
{
   struct fb_var_screeninfo v;

   memcpy(&v, fb_var, sizeof(v));
   if (ioctl(fb, FBIOPUT_VSCREENINFO, &v) == -1)
     {
        const char *errmsg = strerror(errno);
        char *var_str = fb_var_str_convert(fb_var);
        ERR("could not set screeninfo: "
            "ioctl(%d, FBIOPUT_VSCREENINFO, {%s}) = %s",
            fb, var_str, errmsg);
        free(var_str);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

FB_Mode *
fb_setmode(unsigned int width, unsigned int height, unsigned int pdepth, unsigned int refresh)
{
   FB_Mode *modes;
   unsigned int i, num_modes;
   
   modes = fb_list_modes(&num_modes);
   DBG("want %ux%u, bitdepth=%u, refresh=%u, modes=%p, num_modes=%u",
       width, height, pdepth, refresh, modes, num_modes);
   if (modes)
     {
        for (i = 0; i < num_modes; i++)
          {
             DBG("match modes[%d] %ux%u, bitdepth=%u, refresh=%u",
                 i, modes[i].width, modes[i].height,
                 modes[i].fb_var.bits_per_pixel,
                 modes[i].refresh);
             if ((modes[i].width == width) &&
                 (modes[i].height == height) &&
                 (!pdepth || modes[i].fb_var.bits_per_pixel == pdepth) &&
                 (modes[i].refresh == refresh))
               {
                  INF("use modes[%d] %ux%u, bitdepth=%u, refresh=%u",
                      i, width, height, modes[i].fb_var.bits_per_pixel,
                      refresh);

                  if (pdepth) modes[i].fb_var.bits_per_pixel = pdepth;

                  _fb_vscreeninfo_put(&modes[i].fb_var);

                  free(modes);
                  return fb_getmode();
               }
          }
        free(modes);
     }

   INF("no /etc/fb.modes (%u entries) match %ux%u, bitdepth=%u, refresh=%u",
       num_modes, width, height, pdepth, refresh);
   return NULL;
}

FB_Mode *
fb_changedepth(FB_Mode *cur_mode, unsigned int pdepth)
{
   cur_mode->fb_var.bits_per_pixel = pdepth;

   _fb_vscreeninfo_put(&cur_mode->fb_var);

   free(cur_mode);
   return fb_getmode();
}

FB_Mode *
fb_changeres(FB_Mode *cur_mode, unsigned int width, unsigned int height, unsigned int refresh)
{
   FB_Mode *modes;
   unsigned int i, num_modes;
   
   modes = fb_list_modes(&num_modes);
   DBG("want %ux%u, bitdepth=%u, refresh=%u, modes=%p, num_modes=%u",
       width, height, cur_mode->depth, refresh, modes, num_modes);
   if (modes)
     {
        for (i = 0; i < num_modes; i++)
          {
             DBG("match modes[%d] %ux%u, bitdepth=%u, refresh=%u",
                 i, modes[i].width, modes[i].height,
                 modes[i].fb_var.bits_per_pixel,
                 modes[i].refresh);

             if ((modes[i].width == width) &&
                 (modes[i].height == height) &&
                 (modes[i].refresh == refresh))
               {
                  modes[i].fb_var.bits_per_pixel = cur_mode->depth;

                  INF("use modes[%d] %ux%u, bitdepth=%u, refresh=%u",
                      i, width, height, modes[i].fb_var.bits_per_pixel,
                      refresh);

                  _fb_vscreeninfo_put(&modes[i].fb_var);

                  free(modes);
                  free(cur_mode);
                  return fb_getmode();
               }
          }
        free(modes);
     }

   INF("no /etc/fb.modes (%u entries) match %ux%u, bitdepth=%u, refresh=%u. "
       "Keep current mode %ux%u, bitdepth=%u, refresh=%u.",
       num_modes, width, height, cur_mode->depth, refresh,
       cur_mode->width, cur_mode->height, cur_mode->fb_var.bits_per_pixel,
       cur_mode->refresh);
   return cur_mode;
}

FB_Mode *
fb_changemode(FB_Mode *cur_mode, unsigned int width, unsigned int height, unsigned int pdepth, unsigned int refresh)
{
   FB_Mode *modes;
   unsigned int i, num_modes;
   
   modes = fb_list_modes(&num_modes);
   DBG("want %ux%u, bitdepth=%u, refresh=%u, modes=%p, num_modes=%u",
       width, height, pdepth, refresh, modes, num_modes);
   if (modes)
     {
        for (i = 0; i < num_modes; i++)
          {
             DBG("match modes[%d] %ux%u, bitdepth=%u, refresh=%u",
                 i, modes[i].width, modes[i].height,
                 modes[i].fb_var.bits_per_pixel,
                 modes[i].refresh);
             if ((modes[i].width == width) &&
                 (modes[i].height == height) &&
                 (!pdepth || modes[i].fb_var.bits_per_pixel == pdepth) &&
                 (modes[i].refresh == refresh))
               {
                  if (pdepth) modes[i].fb_var.bits_per_pixel = pdepth;

                  INF("use modes[%d] %ux%u, bitdepth=%u, refresh=%u",
                      i, width, height, modes[i].fb_var.bits_per_pixel,
                      refresh);

                  _fb_vscreeninfo_put(&modes[i].fb_var);

                  free(modes);
                  free(cur_mode);
                  return fb_getmode();
               }
          }
        free(modes);
     }


   INF("no /etc/fb.modes (%u entries) match %ux%u, bitdepth=%u, refresh=%u. "
       "Keep current mode %ux%u, bitdepth=%u, refresh=%u.",
       num_modes, width, height, pdepth, refresh,
       cur_mode->width, cur_mode->height, cur_mode->fb_var.bits_per_pixel,
       cur_mode->refresh);
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
        ERR("could not get screeninfo: ioctl(%d, FBIOGET_VSCREENINFO) = %s",
            fb, strerror(errno));
        free(mode);
        return NULL;
     }
   if (WILL_LOG(EINA_LOG_LEVEL_DBG))
     {
        char *var_str = fb_var_str_convert(&mode->fb_var);
        DBG("FBIOGET_VSCREENINFO: %s", var_str);
        free(var_str);
     }

   mode->width = mode->fb_var.xres_virtual;
   mode->height = mode->fb_var.yres_virtual;
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
        ERR("Cannot handle framebuffer of depth %i",
            mode->fb_var.bits_per_pixel);
        fb_cleanup();
        free(mode);
        return NULL;
     }
   mode->depth = depth;
   mode->bpp = bpp;
   if (mode->depth == 8) fb_init_palette_332(mode);
   else fb_init_palette_linear(mode);

   INF("%ux%u, bpp=%u (%u bits), depth=%u, refresh=%u",
       mode->width, mode->height, mode->bpp,
       mode->fb_var.bits_per_pixel, mode->depth, mode->refresh);
   return mode;
}

void
fb_freemode(FB_Mode *mode)
{
   free(mode);
}

/* XXX: unused
static void
fb_setvt(int vtno)
{
   struct vt_stat vts;
   char vtname[32];
   int vtfd;
   
   if (vtno < 0)
     {
        if ((ioctl(tty,VT_OPENQRY, &vtno) == -1))
          {
             perror("ioctl VT_OPENQRY");
             return;
          }
        if (vtno <= 0 )
          {
             perror("ioctl VT_OPENQRY vtno <= 0");
             return;
          }
     }
   vtno &= 0xff;
   sprintf(vtname, "/dev/tty%i", vtno);
   if (chown(vtname, getuid(), getgid()) != 0)
     {
        vtfd = 0; // do nothing - don't worry about chown
     }
   if (access(vtname,R_OK | W_OK) == -1)
     {
        CRI("Access %s: %s",vtname,strerror(errno));
        return;
     }
   vtfd = open(vtname,O_RDWR);
   
   if (ioctl(tty, VT_GETSTATE, &vts) == -1)
     {
        perror("ioctl VT_GETSTATE");
        close(vtfd);
        return;
     }
   
   orig_vt_no = vts.v_active;
   close(vtfd);
#if 0
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
#endif
}
*/

void
fb_init(int vt EINA_UNUSED, int device)
{
   char dev[PATH_MAX];
   

   DBG("device=%d, $EVAS_FB_DEV=%s", device, getenv("EVAS_FB_DEV"));
   tty = -1;
#if 0
   if (vt != 0) fb_setvt(vt);
#endif

   if (
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
       (getuid() == geteuid()) &&
#endif
       (getenv("EVAS_FB_DEV")))
     {
        eina_strlcpy(dev, getenv("EVAS_FB_DEV"), sizeof(dev));
        fb = open(dev, O_RDWR);
     }
   else
     {
        sprintf(dev, "/dev/fb/%i", device);
        fb = open(dev, O_RDWR);
        if ( fb == -1 )
          {
             sprintf(dev, "/dev/fb%i", device);
             fb = open(dev, O_RDWR);
          }
     }
   if (fb == -1)
     {
	CRI("open %s: %s", dev, strerror(errno));
	fb_cleanup();
        return;
     }
   DBG("opened fb=%d (%s)", fb, dev);
   
   if (ioctl(fb, FBIOGET_VSCREENINFO, &fb_ovar) == -1)
     {
        ERR("could not get screeninfo: ioctl(%d, FBIOGET_VSCREENINFO) = %s",
            fb, strerror(errno));
        return;
     }
   if (ioctl(fb, FBIOGET_FSCREENINFO, &fb_fix) == -1)
     {
        ERR("could not get fix screeninfo: ioctl(%d, FBIOGET_FSCREENINFO) = %s",
            fb, strerror(errno));
        return;
     }

   if ((fb_ovar.bits_per_pixel == 8) ||
       (fb_fix.visual == FB_VISUAL_DIRECTCOLOR))
     {
	if (ioctl(fb,FBIOGETCMAP , &ocmap) == -1)
	  {
             ERR("could not get colormap: ioctl(%d, FBIOGETCMAP) = %s",
                 fb, strerror(errno));
             return;
	  }
     }
#if 0
   if (isatty(0))
      tty = 0;
   else if ((tty = open("/dev/tty",O_RDWR)) == -1)
     {
	CRI("open %s: %s", "/dev/tty", strerror(errno));
        return;
     }
   if (tty >= 0)
     {
	if (ioctl(tty, KDGETMODE, &kd_mode) == -1)
	  {
             ERR("could not get KD mode: ioctl(%d, KDGETMODE) = %s",
                 tty, strerror(errno));
             return;
	  }
	if (ioctl(tty, VT_GETMODE, &vt_omode) == -1)
	  {
             ERR("could not get VT mode: ioctl(%d, VT_GETMODE) = %s",
                 tty, strerror(errno));
             return;
	  }
     }
#endif

   if (WILL_LOG(EINA_LOG_LEVEL_INFO))
     {
        char *var_str = fb_var_str_convert(&fb_ovar);
        INF("fb=%d, FBIOGET_VSCREENINFO: %s", fb, var_str);
        free(var_str);
     }
}

int
fb_postinit(FB_Mode *mode)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(mode, -1);
   if (fb < 0)
     {
        ERR("could no set mode %ux%u: no working fb",
            mode->width, mode->height);
        return -1;
     }

   DBG("%ux%u, bpp=%u (%u bits), depth=%u, refresh=%u, fb=%d",
       mode->width, mode->height, mode->bpp,
       mode->fb_var.bits_per_pixel, mode->depth, mode->refresh, fb);

   if (ioctl(fb,FBIOGET_FSCREENINFO, &fb_fix) == -1)
     {
        CRI("could not get fix screeninfo: ioctl(%d, FBIOGET_FSCREENINFO) = %s",
            fb, strerror(errno));
        fb_cleanup();
        return -1;
     }
   
   if (fb_fix.type != FB_TYPE_PACKED_PIXELS)
     {
        CRI("can handle only packed pixel frame buffers (want %#x, got %#x)",
            FB_TYPE_PACKED_PIXELS, fb_fix.type);
        fb_cleanup();
        return -1;
     }
   mode->mem_offset = (unsigned)(fb_fix.smem_start) & (getpagesize()-1);
   mode->mem = (unsigned char *)mmap(NULL, fb_fix.smem_len + mode->mem_offset,
                                     PROT_WRITE | PROT_READ, MAP_SHARED, fb, 0);
   if (mode->mem == MAP_FAILED)
     {
        const char *errmsg = strerror(errno);
        CRI("could not mmap(NULL, %u + %u, PROT_WRITE | PROT_READ, MAP_SHARED, "
            "%d, 0) = %s",
            fb_fix.smem_len, mode->mem_offset, fb, errmsg);
        fb_cleanup();
        return -1;
     }

   mode->stride = fb_fix.line_length / mode->bpp;
   if (mode->stride < mode->width)
     {
        CRI("stride=%u < width=%u", mode->stride, mode->width);
        fb_cleanup();
        return -1;
     }
   if (mode->stride * mode->bpp != fb_fix.line_length)
     {
        CRI("FSCREENINFO line_length=%u is not multiple of bpp=%u",
            fb_fix.line_length, mode->bpp);
        fb_cleanup();
        return -1;
     }

   /* move viewport to upper left corner */
   if ((mode->fb_var.xoffset != 0) || (mode->fb_var.yoffset != 0))
     {
        mode->fb_var.xoffset = 0;
        mode->fb_var.yoffset = 0;
        
        if (ioctl(fb, FBIOPAN_DISPLAY, &(mode->fb_var)) == -1)
          {
             const char *errmsg = strerror(errno);
             char *var_str = fb_var_str_convert(&(mode->fb_var));
             CRI("could not pan display: ioctl(%d, FBIOPAN_DISPLAY, {%s}) = %s",
                 fb, var_str, errmsg);
             free(var_str);
             fb_cleanup();
             return -1;
          }
     }
#if 0
   if (tty >= 0)
     {
	if (ioctl(tty,KDSETMODE, KD_GRAPHICS) == -1)
	  {
             const char *errmsg = strerror(errno);
             CRI("could not set KD mode: ioctl(%d, KDSETMODE, KD_GRAPHICS) = %s",
                 tty, errmsg);
	     fb_cleanup();
             return -1;
	  }
     }
#endif
  mode->fb_fd = fb;

  INF("%ux%u, bpp=%u (%u bits), depth=%u, refresh=%u, fb=%d, mem=%p, "
      "mem_offset=%u, stride=%u pixels, offset=%u, yoffset=%u",
       mode->width, mode->height, mode->bpp,
      mode->fb_var.bits_per_pixel, mode->depth, mode->refresh, fb,
      mode->mem, mode->mem_offset, mode->stride,
      mode->fb_var.xoffset, mode->fb_var.yoffset);

  return fb;
}

void
fb_cleanup(void)
{
   DBG("fb=%d", fb);
   /* restore console */
   if (fb < 0) return;

   if (WILL_LOG(EINA_LOG_LEVEL_INFO))
     {
        char *var_str = fb_var_str_convert(&fb_ovar);
        INF("fb=%d, FBIOSET_VSCREENINFO: %s", fb, var_str);
        free(var_str);
     }
   _fb_vscreeninfo_put(&fb_ovar);
   if (ioctl(fb, FBIOGET_FSCREENINFO, &fb_fix) == -1)
     ERR("could not get fix screeninfo: ioctl(%d, FBIOGET_FSCREENINFO) = %s",
         fb, strerror(errno));
   if ((fb_ovar.bits_per_pixel == 8) ||
       (fb_fix.visual == FB_VISUAL_DIRECTCOLOR))
     {
        if (ioctl(fb, FBIOPUTCMAP, &ocmap) == -1)
          {
             const char *errmsg = strerror(errno);
             char *cmap_str = fb_cmap_str_convert(&cmap);
             ERR("could not set colormap: ioctl(%d, FBIOPUTCMAP, {%s}) = %s",
                 fb, cmap_str, errmsg);
             free(cmap_str);
          }
     }
   close(fb);
   fb = -1;
   if (tty >= 0)
     {
	if (ioctl(tty, KDSETMODE, kd_mode) == -1)
          ERR("could not set KD mode: ioctl(%d, KDSETMODE, %d) = %s",
              tty, kd_mode, strerror(errno));

	if (ioctl(tty, VT_SETMODE, &vt_omode) == -1)
          ERR("could not set VT mode: ioctl(%d, VT_SETMODE, {"
              "mode=%hhd, waitv=%hhd, relsig=%hd, acqsig=%hd, frsig=%hd}) = %s",
              tty, vt_omode.mode, vt_omode.waitv, vt_omode.relsig,
              vt_omode.acqsig, vt_omode.frsig, strerror(errno));
#if 0        
	if ((ioctl(tty, VT_ACTIVATE, orig_vt_no) == -1) && (orig_vt_no))
          ERR("could not activate: ioctl(%d, VT_ACTIVATE, %d) = %s",
              tty, orig_vt_no, strerror(errno));
#endif        
        if (tty > 0) /* don't close if got from isatty(0) */
          close(tty);
     }
   tty = -1;
}
