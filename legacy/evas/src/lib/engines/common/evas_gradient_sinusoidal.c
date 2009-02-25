/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <math.h>

#include "evas_common.h"
#include "evas_gradient_private.h"

typedef struct _Sinusoidal_Data   Sinusoidal_Data;
struct _Sinusoidal_Data
{
   float  amp, per;

   float  sa, sp;
   float  off;
   int    len;
};

static void
sinusoidal_init(void);

static void
sinusoidal_shutdown(void);

static void
sinusoidal_init_geom(RGBA_Gradient *gr);

static void
sinusoidal_setup_geom(RGBA_Gradient *gr);

static void
sinusoidal_free_geom(void *gdata);

static int
sinusoidal_has_alpha(RGBA_Gradient *gr, int op);

static int
sinusoidal_has_mask(RGBA_Gradient *gr, int op);

static int
sinusoidal_get_map_len(RGBA_Gradient *gr);

static Gfx_Func_Gradient_Fill
sinusoidal_get_fill_func(RGBA_Gradient *gr, int op, unsigned char aa);

static RGBA_Gradient_Type  sinusoidal = {"sinusoidal", sinusoidal_init, sinusoidal_shutdown,
					 sinusoidal_init_geom, sinusoidal_setup_geom, sinusoidal_free_geom,
					 sinusoidal_has_alpha, sinusoidal_has_mask,
					 sinusoidal_get_map_len, sinusoidal_get_fill_func};


/** internal functions **/

static void
sinusoidal_reflect(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_reflect_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_repeat(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_repeat_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                     int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_restrict_reflect(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                            int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_restrict_reflect_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_restrict_reflect_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_restrict_reflect_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_restrict_repeat(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_restrict_repeat_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_restrict_repeat_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_restrict_repeat_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                     int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_pad(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_pad_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);


RGBA_Gradient_Type  *
evas_common_gradient_sinusoidal_get(void)
{
    return &sinusoidal;
}

static void
sinusoidal_init(void)
{
}

static void
sinusoidal_shutdown(void)
{
}

static void
sinusoidal_free_geom(void *gdata)
{
   Sinusoidal_Data *data = (Sinusoidal_Data *)gdata;
   if (data) free(data);
}

static void
sinusoidal_setup_geom(RGBA_Gradient *gr)
{
   Sinusoidal_Data   *sinusoidal_data;

   if (!gr || (gr->type.geometer != &sinusoidal)) return;

   sinusoidal_data = (Sinusoidal_Data *)gr->type.gdata;
   if (!sinusoidal_data) return;
   sinusoidal_data->sa = sinusoidal_data->amp * gr->fill.h;
   sinusoidal_data->sp = sinusoidal_data->per * (M_PI / gr->fill.w);
   sinusoidal_data->off = gr->map.offset;
   sinusoidal_data->len = gr->fill.h;
}

static void
sinusoidal_init_geom(RGBA_Gradient *gr)
{
   Sinusoidal_Data   *sinusoidal_data;
   int    err = 1;
   char   *s, *p, key[256];
   float  amp, per, val;

   if (!gr || (gr->type.geometer != &sinusoidal)) return;

   sinusoidal_data = (Sinusoidal_Data *)gr->type.gdata;
   if (!sinusoidal_data)
     {
	sinusoidal_data = calloc(1, sizeof(Sinusoidal_Data));
	if (!sinusoidal_data)  return;
	sinusoidal_data->amp = 1.0;
	sinusoidal_data->per = 1.0;
	sinusoidal_data->sa = 32;
	sinusoidal_data->sp = M_PI / 32;
	sinusoidal_data->off = 0.0;
	sinusoidal_data->len = 32;
     }
   gr->type.gdata = sinusoidal_data;

   if (!gr->type.params || !*(gr->type.params))
	return;

   s = strdup(gr->type.params);
   if (!s) return;

   amp = sinusoidal_data->amp;
   per = sinusoidal_data->per;
   p = s;
   while ((p = evas_common_gradient_get_key_fval(p, key, &val)))
     {
	if (!strcmp(key, "amplitude"))
	  {
	    err = 0;
	    amp = val;
	  }
	else if (!strcmp(key, "period"))
	  {
	    err = 0;
	    per = val;
	  }
	else
	  {
	    err = 1;
	    break;
	  }
     }
   if (!err)
     {
	sinusoidal_data->amp = amp;
	if (per < 0.0) per = -per;
	sinusoidal_data->per = per;
     }
   free(s);
}


static int
sinusoidal_has_alpha(RGBA_Gradient *gr, int op)
{
   if (!gr || (gr->type.geometer != &sinusoidal)) return 0;

   if (gr->has_alpha | gr->map.has_alpha)
	return 1;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) ||
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
	return 0;
   if ( (gr->fill.spread == _EVAS_TEXTURE_RESTRICT) ||
         (gr->fill.spread == _EVAS_TEXTURE_RESTRICT_REFLECT) ||
         (gr->fill.spread == _EVAS_TEXTURE_RESTRICT_REPEAT) )
	return 1;
   return 0;
}

static int
sinusoidal_has_mask(RGBA_Gradient *gr, int op)
{
   if (!gr || (gr->type.geometer != &sinusoidal)) return 0;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) ||
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
     {
	if ( (gr->fill.spread == _EVAS_TEXTURE_RESTRICT) ||
	      (gr->fill.spread == _EVAS_TEXTURE_RESTRICT_REFLECT) ||
	      (gr->fill.spread == _EVAS_TEXTURE_RESTRICT_REPEAT) )
	    return 1;
     }

   return 0;
}

static int
sinusoidal_get_map_len(RGBA_Gradient *gr)
{
   Sinusoidal_Data   *sinusoidal_data;

   if (!gr || (gr->type.geometer != &sinusoidal)) return 0;
   sinusoidal_data = (Sinusoidal_Data *)gr->type.gdata;
   if (!sinusoidal_data) return 0;
   return sinusoidal_data->len;
}

static Gfx_Func_Gradient_Fill
sinusoidal_get_fill_func(RGBA_Gradient *gr, int op, unsigned char aa)
{
   Sinusoidal_Data   *sinusoidal_data;
   Gfx_Func_Gradient_Fill  sfunc = NULL;
   int masked_op = 0;

   if (!gr || (gr->type.geometer != &sinusoidal))
	return sfunc;
   sinusoidal_data = (Sinusoidal_Data *)gr->type.gdata;
   if (!sinusoidal_data) return sfunc;

   sinusoidal_data->off = gr->map.offset;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) ||
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
	masked_op = 1;

   switch (gr->fill.spread)
     {
      case _EVAS_TEXTURE_REFLECT:
	{
	 if (aa)
	    sfunc = sinusoidal_reflect_aa;
	 else
	    sfunc = sinusoidal_reflect;
	}
      break;
      case _EVAS_TEXTURE_REPEAT:
	{
	 if (aa)
	    sfunc = sinusoidal_repeat_aa;
	 else
	    sfunc = sinusoidal_repeat;
	}
      break;
      case _EVAS_TEXTURE_RESTRICT:
	 sinusoidal_data->off = 0;
      case _EVAS_TEXTURE_RESTRICT_REFLECT:
	{
	 if (aa)
	   {
	    if (masked_op)
		sfunc = sinusoidal_restrict_reflect_aa_masked;
	    else
		sfunc = sinusoidal_restrict_reflect_aa;
	   }
	 else
	   {
	    if (masked_op)
		sfunc = sinusoidal_restrict_reflect_masked;
	    else
		sfunc = sinusoidal_restrict_reflect;
	   }
	}
      break;
      case _EVAS_TEXTURE_RESTRICT_REPEAT:
	{
	 if (aa)
	   {
	    if (masked_op)
		sfunc = sinusoidal_restrict_repeat_aa_masked;
	    else
		sfunc = sinusoidal_restrict_repeat_aa;
	   }
	 else
	   {
	    if (masked_op)
		sfunc = sinusoidal_restrict_repeat_masked;
	    else
		sfunc = sinusoidal_restrict_repeat;
	   }
	}
      break;
      case _EVAS_TEXTURE_PAD:
	{
	 if (aa)
	    sfunc = sinusoidal_pad_aa;
	 else
	    sfunc = sinusoidal_pad;
	}
      break;
      default:
	sfunc = sinusoidal_reflect_aa;
      break;
     }
   return sfunc;
}

#define SETUP_SINU_FILL \
   a00 = gdata->sp * (axx / 65536.0); \
   a01 = gdata->sp * (axy / 65536.0); \
   a10 = ayx / 65536.0; \
   a11 = ayy / 65536.0; \
   xf = (a00 * x) + (a01 * y); \
   yf = (a10 * x) + (a11 * y);

static void
sinusoidal_reflect(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;
   float  off = gdata->off * (src_len - 1);

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	int  l = (yf - (sa * sin(xf))) + off;

	if (l < 0)  l = -l;
        if (l >= src_len)
          {
	    int  m = (l % (2 * src_len));

	    l = (l % src_len);
	    if (m >= src_len)
		l = src_len - l - 1;
          }
	*dst++ = src[l];  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_reflect_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;
   float  off = gdata->off * (src_len - 1);

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	float  r = (yf - (sa * sin(xf))) + off;
	int    l = r, a;

	if (r < 0)  r = -r;
	a = 1 + (int)(255 * (r - (int)r));

	if (l < 0)  l = -l;
	if (l >= src_len)
	  {
	    int  m = (l % (2 * src_len));

	    l = (l % src_len);
	    if (m >= src_len)
		{ l = src_len - l - 1;  a = 257 - a; }
	  }
	*dst = src[l];
	if (l + 1 < src_len)
	  *dst = INTERP_256(a, src[l + 1], *dst);

	dst++;  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_repeat(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;
   float  off = gdata->off * (src_len - 1);

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	int  l = (yf - (sa * sin(xf))) + off;

	l = (l % src_len);
	if (l < 0)
	    l += src_len;
	*dst++ = src[l];  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_repeat_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
                     int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;
   float  off = gdata->off * (src_len - 1);

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	float  r = (yf - (sa * sin(xf))) + off;
	int    l = r, a;

	if (r < 0) r = -r;
	a = 1 + (int)(255 * (r - (int)r));

	l = l % src_len;
	if (l < 0)
	  { l += src_len;  a = 257 - a; }

	*dst = src[l];
	if (l + 1 < src_len)
	   *dst = INTERP_256(a, src[l + 1], *dst);
	if (l == (src_len - 1))
	   *dst = INTERP_256(a, src[0], *dst);
	dst++;  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_restrict_reflect(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
                            int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;
   float  off = gdata->off * (src_len - 1);

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	int  l = (yf - (sa * sin(xf)));

	*dst = 0;
	if ((unsigned)l < src_len)
	  {
	    l += off;
	    if (l < 0) l = -l;
	    if (l >= src_len)
	      {
		int  m = (l % (2 * src_len));

		l = (l % src_len);
		if (m >= src_len)
		   l = src_len - l - 1;
	      }
	    *dst = src[l];
	  }
	dst++;  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_restrict_reflect_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
                               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;
   float  off = gdata->off * (src_len - 1);

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	float  r = (yf - (sa * sin(xf))) + off, s = r - off;
	int    l = s;

	*dst = 0;
	if ((unsigned)l < src_len)
	  {
	    int  a, lp = r;

	    if (r < 0)  r = -r;
	    a = 1 + (int)(255 * (r - (int)r));
	    if (lp < 0) lp = -lp;
	    if (lp >= src_len)
	      {
	        int  m = (lp % (2 * src_len));

		lp = (lp % src_len);
		if (m >= src_len)
		  { lp = src_len - lp - 1;  a = 257 - a; }
	      }
	    *dst = src[lp];
	    if (lp + 1 < src_len)
		*dst = INTERP_256(a, src[lp + 1], *dst);
	    if ((l == 0) && (s < 0))
	      {
		a = 256 + (255 * s);
		*dst = MUL_256(a, *dst);
	      }
	    if (l == (src_len - 1))
	      {
		a = 256 - (255 * (s - l));
		*dst = MUL_256(a, *dst);
	      }
	  }
	dst++;  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_restrict_reflect_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;
   float  off = gdata->off * (src_len - 1);

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	int  l = (yf - (sa * sin(xf)));

	*dst = 0;  *mask = 0;
	if ((unsigned)l < src_len)
	  {
	    l += off;
	    if (l < 0) l = -l;
	    if (l >= src_len)
	      {
		int  m = (l % (2 * src_len));

		l = (l % src_len);
		if (m >= src_len)
		   l = src_len - l - 1;
	      }
	    *dst = src[l];  *mask = 255;
	  }
	dst++;  mask++;  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_restrict_reflect_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;
   float  off = gdata->off * (src_len - 1);

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	float  r = (yf - (sa * sin(xf))) + off, s = r - off;
	int    l = s;

	*dst = 0;  *mask = 0;
	if ((unsigned)l < src_len)
	  {
	    int  a, lp = r;

	    if (r < 0)  r = -r;
	    a = 1 + (int)(255 * (r - (int)r));
	    if (lp < 0) lp = -lp;
	    if (lp >= src_len)
	      {
	        int  m = (lp % (2 * src_len));

		lp = (lp % src_len);
		if (m >= src_len)
		  { lp = src_len - lp - 1;  a = 257 - a; }
	      }
	    *dst = src[lp];  *mask = 255;
	    if (lp + 1 < src_len)
		*dst = INTERP_256(a, src[lp + 1], *dst);
	    if ((l == 0) && (s < 0))
		*mask = 255 + (255 * s);
	    if (l == (src_len - 1))
		*mask = 255 - (255 * (s - l));
	  }
	dst++;  mask++;  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_restrict_repeat(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;
   float  off = gdata->off * (src_len - 1);

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	int  l = (yf - (sa * sin(xf)));

	*dst = 0;
	if ((unsigned)l < src_len)
	  {
	    l += off;
	    l = l % src_len;
	    if (l < 0)
		l += src_len;
	    *dst = src[l];
	  }
	dst++;  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_restrict_repeat_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
                              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;
   float  off = gdata->off * (src_len - 1);

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	float  r = (yf - (sa * sin(xf))) + off, s = r - off;
	int    l = s;

	*dst = 0;
	if ((unsigned)l < src_len)
	  {
	    int  a, lp = r;

	    if (r < 0)  r = -r;
	    a = 1 + (int)(255 * (r - (int)r));
	    lp = lp % src_len;
	    if (lp < 0)
	      { lp += src_len;  a = 257 - a; }
	    *dst = src[lp];
	    if (lp + 1 < src_len)
		*dst = INTERP_256(a, src[lp + 1], *dst);
	    if (lp == (src_len - 1))
		*dst = INTERP_256(a, src[0], *dst);
	    if ((l == 0) && (s < 0))
	      {
		a = 256 + (255 * s);
		*dst = MUL_256(a, *dst);
	      }
	    if (l == (src_len - 1))
	      {
		a = 256 - (255 * (s - l));
		*dst = MUL_256(a, *dst);
	      }
	  }
	dst++;  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_restrict_repeat_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;
   float  off = gdata->off * (src_len - 1);

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	int  l = (yf - (sa * sin(xf)));

	*dst = 0;  *mask = 0;
	if ((unsigned)l < src_len)
	  {
	    l += off;
	    l = l % src_len;
	    if (l < 0)
		l += src_len;
	    *dst = src[l];  *mask = 255;
	  }
	dst++;  mask++;  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_restrict_repeat_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                     int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;
   float  off = gdata->off * (src_len - 1);

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	float  r = (yf - (sa * sin(xf))) + off, s = r - off;
	int    l = s;

	*dst = 0;  *mask = 0;
	if ((unsigned)l < src_len)
	  {
	    int  a, lp = r;

	    if (r < 0)  r = -r;
	    a = 1 + (int)(255 * (r - (int)r));
	    lp = lp % src_len;
	    if (lp < 0)
	      { lp += src_len;  a = 257 - a; }
	    *dst = src[lp];  *mask = 255;
	    if (lp + 1 < src_len)
		*dst = INTERP_256(a, src[lp + 1], *dst);
	    if (lp == (src_len - 1))
		*dst = INTERP_256(a, src[0], *dst);
	    if ((l == 0) && (s < 0))
		*mask = 255 + (255 * s);
	    if (l == (src_len - 1))
		*mask = 255 - (255 * (s - l));
	  }
	dst++;  mask++;  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_pad(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	int  l = (yf - (sa * sin(xf)));

	if (l < 0)
	    l = 0;
	if (l >= src_len)
	    l = src_len - 1;
	*dst = src[l];
	dst++;  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_pad_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	float  r = (yf - (sa * sin(xf)));
	int    l = r;

	*dst = 0;
	if ((unsigned)l < (src_len - 1))
	  {
	    int  a;

	    if (r < 0) r = -r;
	    a = 1 + (int)(255 * (r - (int)r));
	    *dst = INTERP_256(a, src[l + 1], src[l]);
	  }
	if (l < 0) *dst = src[0];
	if (l >= src_len) *dst = src[src_len - 1];
	dst++;  xf += a00;  yf += a10;
     }
}
