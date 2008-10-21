#include "evas_common.h"
#include "evas_engine.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef struct _Convert_Pal_Priv Convert_Pal_Priv;

struct _Convert_Pal_Priv
{
   Display *disp;
   Colormap cmap;
   Visual  *vis;
};

typedef DATA8 * (*X_Func_Alloc_Colors) (Display *d, Colormap cmap, Visual *v);

static X_Func_Alloc_Colors x_color_alloc[PAL_MODE_LAST + 1];
static int                 x_color_count[PAL_MODE_LAST + 1];
static Eina_List          *palettes = NULL;

static DATA8 * x_color_alloc_rgb(int nr, int ng, int nb, Display *d, Colormap cmap, Visual *v);
static DATA8 * x_color_alloc_gray(int ng, Display *d, Colormap cmap, Visual *v);

static DATA8 * x_color_alloc_rgb_332(Display *d, Colormap cmap, Visual *v);
static DATA8 * x_color_alloc_rgb_666(Display *d, Colormap cmap, Visual *v);
static DATA8 * x_color_alloc_rgb_232(Display *d, Colormap cmap, Visual *v);
static DATA8 * x_color_alloc_rgb_222(Display *d, Colormap cmap, Visual *v);
static DATA8 * x_color_alloc_rgb_221(Display *d, Colormap cmap, Visual *v);
static DATA8 * x_color_alloc_rgb_121(Display *d, Colormap cmap, Visual *v);
static DATA8 * x_color_alloc_rgb_111(Display *d, Colormap cmap, Visual *v);
static DATA8 * x_color_alloc_gray_256(Display *d, Colormap cmap, Visual *v);
static DATA8 * x_color_alloc_gray_64(Display *d, Colormap cmap, Visual *v);
static DATA8 * x_color_alloc_gray_16(Display *d, Colormap cmap, Visual *v);
static DATA8 * x_color_alloc_gray_4(Display *d, Colormap cmap, Visual *v);
static DATA8 * x_color_alloc_mono(Display *d, Colormap cmap, Visual *v);

static DATA8 *
x_color_alloc_rgb(int nr, int ng, int nb, Display *d, Colormap cmap, Visual *v)
{
   int r, g, b, i;
   DATA8 *color_lut;
   int sig_mask = 0, delt = 0;

   for (i = 0; i < v->bits_per_rgb; i++) sig_mask |= (0x1 << i);
   sig_mask <<= (16 - v->bits_per_rgb);
   i = 0;
   color_lut = malloc((nr) * (ng) * (nb));
   if (!color_lut) return NULL;
   delt = 0x0101 * 3;
   for (r = 0; r < (nr); r++)
     {
	for (g = 0; g < (ng); g++)
	  {
	     for (b = 0; b < (nb); b++)
	       {
		  XColor xcl;
		  XColor xcl_in;
		  int val;
		  Status ret;
		  int dr, dg, db;

                  val = (int)((((double)r) / ((nr) - 1)) * 255);
		  val = (val << 8) | val;
		  xcl.red = (unsigned short)(val);
		  val = (int)((((double)g) / ((ng) - 1)) * 255);
		  val = (val << 8) | val;
		  xcl.green = (unsigned short)(val);
		  val = (int)((((double)b) / ((nb) - 1)) * 255);
		  val = (val << 8) | val;
		  xcl.blue = (unsigned short)(val);
		  xcl_in = xcl;
		  ret = XAllocColor(d, cmap, &xcl);
		  dr = (int)xcl_in.red - (int)xcl.red;
		  if (dr < 0) dr = -dr;
		  dg = (int)xcl_in.green - (int)xcl.green;
		  if (dg < 0) dg = -dg;
		  db = (int)xcl_in.blue - (int)xcl.blue;
		  if (db < 0) db = -db;
/*		  
		  printf("ASK [%i]: %04x %04x %04x = %04x %04x %04x | dif = %04x / %04x\n",
			 ret,
			 xcl_in.red, xcl_in.green, xcl_in.blue,
			 xcl.red, xcl.green, xcl.blue,
			 (dr + dg +db), delt);
 */
		  if ((ret == 0) ||
		      ((dr + dg + db) > delt)
/*		      
		      ||
		      ((xcl_in.red & sig_mask) != (xcl.red & sig_mask)) ||
		      ((xcl_in.green & sig_mask) != (xcl.green & sig_mask)) ||
		      ((xcl_in.blue & sig_mask) != (xcl.blue & sig_mask))
 */
		      )
		    {
		       unsigned long pixels[256];
		       int j;

		       if (i > 0)
			 {
			    for (j = 0; j < i; j++)
			      pixels[j] = (unsigned long) color_lut[j];
			    XFreeColors(d, cmap, pixels, i, 0);
			 }
		       free(color_lut);
		       return NULL;
		    }
		  color_lut[i] = xcl.pixel;
		  i++;
	       }
	  }
     }
   return color_lut;
}

static DATA8 *
x_color_alloc_gray(int ng, Display *d, Colormap cmap, Visual *v)
{
   int g, i;
   DATA8 *color_lut;
   int sig_mask = 0;

   for (i = 0; i < v->bits_per_rgb; i++) sig_mask |= (0x1 << i);
   sig_mask <<= (16 - v->bits_per_rgb);
   i = 0;
   color_lut = malloc(ng);
   if (!color_lut) return NULL;
   for (g = 0; g < (ng); g++)
     {
	XColor xcl;
	XColor xcl_in;
	int val;
	Status ret;

	val = (int)((((double)g) / ((ng) - 1)) * 255);
	val = (val << 8) | val;
	xcl.red = (unsigned short)(val);
	xcl.green = (unsigned short)(val);
	xcl.blue = (unsigned short)(val);
	xcl_in = xcl;
	ret = XAllocColor(d, cmap, &xcl);
	if ((ret == 0) ||
	    ((xcl_in.red & sig_mask) != (xcl.red & sig_mask)) ||
	    ((xcl_in.green & sig_mask) != (xcl.green & sig_mask)) ||
	    ((xcl_in.blue & sig_mask) != (xcl.blue & sig_mask)))
	  {
	     unsigned long pixels[256];
	     int j;

	     if (i > 0)
	       {
		  for (j = 0; j < i; j++)
		    pixels[j] = (unsigned long) color_lut[j];
		  XFreeColors(d, cmap, pixels, i, 0);
	       }
	     free(color_lut);
	     return NULL;
	  }
	color_lut[i] = xcl.pixel;
	i++;
     }
   return color_lut;
}

static DATA8 *
x_color_alloc_rgb_332(Display *d, Colormap cmap, Visual *v)
{
   return x_color_alloc_rgb(8, 8, 4, d, cmap, v);
}

static DATA8 *
x_color_alloc_rgb_666(Display *d, Colormap cmap, Visual *v)
{
   return x_color_alloc_rgb(6, 6, 6, d, cmap, v);
}

static DATA8 *
x_color_alloc_rgb_232(Display *d, Colormap cmap, Visual *v)
{
   return x_color_alloc_rgb(4, 8, 4, d, cmap, v);
}

static DATA8 *
x_color_alloc_rgb_222(Display *d, Colormap cmap, Visual *v)
{
   return x_color_alloc_rgb(4, 4, 4, d, cmap, v);
}

static DATA8 *
x_color_alloc_rgb_221(Display *d, Colormap cmap, Visual *v)
{
   return x_color_alloc_rgb(4, 4, 2, d, cmap, v);
}

static DATA8 *
x_color_alloc_rgb_121(Display *d, Colormap cmap, Visual *v)
{
   return x_color_alloc_rgb(2, 4, 2, d, cmap, v);
}

static DATA8 *
x_color_alloc_rgb_111(Display *d, Colormap cmap, Visual *v)
{
   return x_color_alloc_rgb(2, 2, 2, d, cmap, v);
}

static DATA8 *
x_color_alloc_gray_256(Display *d, Colormap cmap, Visual *v)
{
   return x_color_alloc_gray(256, d, cmap, v);
}

static DATA8 *
x_color_alloc_gray_64(Display *d, Colormap cmap, Visual *v)
{
   return x_color_alloc_gray(64, d, cmap, v);
}

static DATA8 *
x_color_alloc_gray_16(Display *d, Colormap cmap, Visual *v)
{
   return x_color_alloc_gray(32, d, cmap, v);
}

static DATA8 *
x_color_alloc_gray_4(Display *d, Colormap cmap, Visual *v)
{
   return x_color_alloc_gray(16, d, cmap, v);
}

static DATA8 *
x_color_alloc_mono(Display *d, Colormap cmap, Visual *v)
{
   return x_color_alloc_gray(2, d, cmap, v);
}

void
evas_software_x11_x_color_init(void)
{
   static int initialised = 0;

   if (initialised) return;
   x_color_alloc[PAL_MODE_NONE]    = NULL;
   x_color_count[PAL_MODE_NONE]    = 0;

   x_color_alloc[PAL_MODE_MONO]    = x_color_alloc_mono;
   x_color_count[PAL_MODE_MONO]    = 2;

   x_color_alloc[PAL_MODE_GRAY4]   = x_color_alloc_gray_4;
   x_color_count[PAL_MODE_GRAY4]   = 4;

   x_color_alloc[PAL_MODE_GRAY16]  = x_color_alloc_gray_16;
   x_color_count[PAL_MODE_GRAY16]  = 16;

   x_color_alloc[PAL_MODE_GRAY64]  = x_color_alloc_gray_64;
   x_color_count[PAL_MODE_GRAY64]  = 64;

   x_color_alloc[PAL_MODE_GRAY256] = x_color_alloc_gray_256;
   x_color_count[PAL_MODE_GRAY256] = 256;

   x_color_alloc[PAL_MODE_RGB111]  = x_color_alloc_rgb_111;
   x_color_count[PAL_MODE_RGB111]  = 2 * 2 * 2;

   x_color_alloc[PAL_MODE_RGB121]  = x_color_alloc_rgb_121;
   x_color_count[PAL_MODE_RGB121]  = 2 * 4 * 2;

   x_color_alloc[PAL_MODE_RGB221]  = x_color_alloc_rgb_221;
   x_color_count[PAL_MODE_RGB221]  = 4 * 4 * 2;

   x_color_alloc[PAL_MODE_RGB222]  = x_color_alloc_rgb_222;
   x_color_count[PAL_MODE_RGB222]  = 4 * 4 * 4;

   x_color_alloc[PAL_MODE_RGB232]  = x_color_alloc_rgb_232;
   x_color_count[PAL_MODE_RGB232]  = 4 * 8 * 4;

   x_color_alloc[PAL_MODE_RGB666]  = x_color_alloc_rgb_666;
   x_color_count[PAL_MODE_RGB666]  = 6 * 6 * 6;

   x_color_alloc[PAL_MODE_RGB332]  = x_color_alloc_rgb_332;
   x_color_count[PAL_MODE_RGB332]  = 8 * 8 * 4;

   x_color_alloc[PAL_MODE_LAST]    = NULL;
   x_color_count[PAL_MODE_LAST]    = 0;
   initialised = 1;
}

Convert_Pal *
evas_software_x11_x_color_allocate(Display *disp, Colormap cmap, Visual *vis, Convert_Pal_Mode colors)
{
   Convert_Pal_Priv *palpriv;
   Convert_Pal      *pal;
   Convert_Pal_Mode  c;
   Eina_List        *l;

/*   printf("ALLOC cmap=%i vis=%p\n", cmap, vis);*/
   EINA_LIST_FOREACH(palettes, l, pal)
     {
	palpriv = pal->data;
	if ((disp == palpriv->disp) &&
	    (vis == palpriv->vis) &&
	    (cmap == palpriv->cmap))
	  {
	     pal->references++;
	     return pal;
	  }
     }
   pal = calloc(1, sizeof(struct _Convert_Pal));
   if (!pal) return NULL;
   for (c = colors; c > PAL_MODE_NONE; c--)
     {
	if (x_color_alloc[c])
	  {
/*	     printf("TRY PAL %i\n", c);*/
	     pal->lookup = (x_color_alloc[c])(disp, cmap, vis);
	     if (pal->lookup) break;
	  }
     }
   pal->references = 1;
   pal->colors = c;
   pal->count = x_color_count[c];
   palpriv = calloc(1, sizeof(Convert_Pal_Priv));
   pal->data = palpriv;
   if (!palpriv)
     {
	if (pal->lookup) free(pal->lookup);
	free(pal);
	return NULL;
     }
   palpriv->disp = disp;
   palpriv->vis = vis;
   palpriv->cmap = cmap;
   if (pal->colors == PAL_MODE_NONE)
     {
	if (pal->lookup) free(pal->lookup);
	free(pal);
	return NULL;
     }
   palettes = eina_list_append(palettes, pal);
   return pal;
}

void
evas_software_x11_x_color_deallocate(Display *disp, Colormap cmap, Visual *vis, Convert_Pal *pal)
{
   unsigned long pixels[256];
   int j;

   pal->references--;
   if (pal->references > 0) return;
   if (pal->lookup)
     {
	for(j = 0; j < pal->count; j++)
	  pixels[j] = (unsigned long) pal->lookup[j];
	XFreeColors(disp, cmap, pixels, pal->count, 0);
	free(pal->lookup);
     }
   free(pal->data);
   palettes = eina_list_remove(palettes, pal);
   free(pal);
}
