#include "evas_common.h"
#include <math.h>

typedef struct _Sinusoidal_Data   Sinusoidal_Data;
struct _Sinusoidal_Data
{
   float sa, sp;
};

static Sinusoidal_Data  sinusoidal_data = {32.0, M_PI / 32.0};


static void 
sinusoidal_setup_geom(RGBA_Gradient *gr, int spread);

static int 
sinusoidal_has_alpha(RGBA_Gradient *gr, int spread);

static int 
sinusoidal_get_map_len(RGBA_Gradient *gr, int spread);

static Gfx_Func_Gradient_Span 
sinusoidal_get_span_func(RGBA_Gradient *gr, int spread, unsigned char aa);

static RGBA_Gradient_Type  sinusoidal = {"sinusoidal", &sinusoidal_data, sinusoidal_setup_geom, sinusoidal_has_alpha, sinusoidal_get_map_len, sinusoidal_get_span_func};

/** internal functions **/

static void
sinusoidal_reflect(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                     int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_repeat(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_restrict_aa(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                    int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_restrict(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);


RGBA_Gradient_Type  *
evas_common_gradient_sinusoidal_get(void)
{
    return &sinusoidal;
}

static void 
sinusoidal_setup_geom(RGBA_Gradient *gr, int spread)
{
   int    err = 1;
   char   *s, *p, key[256];
   float  amp = 1.0, per = 1.0, val;
   
   if (!gr || (gr->type.geometer != &sinusoidal)) return;

   sinusoidal_data.sa = gr->fill.h;
   sinusoidal_data.sp = M_PI / gr->fill.w;

   if (!gr->type.params || !*(gr->type.params))
	return;

   s = strdup(gr->type.params);
   if (!s) return;
   
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
	    goto done;
	  }
     }
   done:
   if (!err)
     {
	sinusoidal_data.sa *= amp;
	if (per < 0.0) per = -per;
	sinusoidal_data.sp *= per;
     }
   free(s);
}


static int 
sinusoidal_has_alpha(RGBA_Gradient *gr, int spread)
{
   if (!gr || (gr->type.geometer != &sinusoidal)) return 0;

   if (gr->map.has_alpha)
	return 1;
   if (spread == _EVAS_TEXTURE_RESTRICT)
	return 1;
   return 0;
}

static int 
sinusoidal_get_map_len(RGBA_Gradient *gr, int spread)
{
   if (!gr || (gr->type.geometer != &sinusoidal)) return 0;

   return gr->fill.h;
}

static Gfx_Func_Gradient_Span 
sinusoidal_get_span_func(RGBA_Gradient *gr, int spread, unsigned char aa)
{
   Gfx_Func_Gradient_Span  sfunc = NULL;

   if (!gr || (gr->type.geometer != &sinusoidal)) return sfunc;
   switch (spread)
     {
      case _EVAS_TEXTURE_REPEAT:
	{
	 if (aa)
	    sfunc = sinusoidal_repeat_aa;
	 else
	    sfunc = sinusoidal_repeat;
	}
      break;
      case _EVAS_TEXTURE_RESTRICT:
	{
	 if (aa)
	    sfunc = sinusoidal_restrict_aa;
	 else
	    sfunc = sinusoidal_restrict;
	}
      break;
      default:
	sfunc = sinusoidal_reflect;
      break;
     }
   return sfunc;
}

static void
sinusoidal_reflect(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;

   a00 = gdata->sp * (axx / 65536.0);
   a01 = gdata->sp * (axy / 65536.0);
   a10 = ayx / 65536.0;
   a11 = ayy / 65536.0;

   xf = (a00 * x) + (a01 * y);
   yf = (a10 * x) + (a11 * y);

   while (dst < dst_end)
     {
	int  l = (yf - (sa * sin(xf)));

	if (l < 0)  l = -l;
        if (l >= map_len)
          {
	    int  m = (l % (2 * map_len));

	    l = (l % map_len);
	    if (m >= map_len)
		l = map_len - l - 1;
          }
	*dst++ = map[l];  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                     int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;

   a00 = gdata->sp * (axx / 65536.0);
   a01 = gdata->sp * (axy / 65536.0);
   a10 = ayx / 65536.0;
   a11 = ayy / 65536.0;

   xf = (a00 * x) + (a01 * y);
   yf = (a10 * x) + (a11 * y);

   while (dst < dst_end)
     {
	float  r = (yf - (sa * sin(xf)));
	int    l = r;

	l = (l % map_len);
	if (l < 0)
	   l += map_len;

	*dst = map[l];
	if (l == 0)
	  {
	    int a = 0;
	    DATA32  *c = map + (map_len - 1);

	    if ((r > 0) && (r >= map_len))
	      {
		r -= (int)r;
		a = 256 - (int)(255 * r);
	      }
	    if (r < 0)
	      {
		r = -r;  r -= (int)r;
		a = 1 + (int)(255 * r);
	      }
	    *dst += ARGB_JOIN((a * (A_VAL(c) - A_VAL(dst))) >> 8,
	                      (a * (R_VAL(c) - R_VAL(dst))) >> 8,
	                      (a * (G_VAL(c) - G_VAL(dst))) >> 8,
	                      (a * (B_VAL(c) - B_VAL(dst))) >> 8);		
	  }
	dst++;  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_repeat(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;

   a00 = gdata->sp * (axx / 65536.0);
   a01 = gdata->sp * (axy / 65536.0);
   a10 = ayx / 65536.0;
   a11 = ayy / 65536.0;

   xf = (a00 * x) + (a01 * y);
   yf = (a10 * x) + (a11 * y);

   while (dst < dst_end)
     {
	int  l = (yf - (sa * sin(xf)));

	l = (l % map_len);
	if (l < 0)
	    l += map_len;

	*dst++ = map[l];  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_restrict_aa(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                    int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;

   a00 = gdata->sp * (axx / 65536.0);
   a01 = gdata->sp * (axy / 65536.0);
   a10 = ayx / 65536.0;
   a11 = ayy / 65536.0;

   xf = (a00 * x) + (a01 * y);
   yf = (a10 * x) + (a11 * y);

   while (dst < dst_end)
     {
	float  r = (yf - (sa * sin(xf)));
	int    l = r;

	*dst = 0;
	if ((unsigned)l < map_len)
	  {
	   *dst = map[l];
	   if (l == (map_len - 1))
	     {
		int a = 1 + (int)(255 * (map_len - r));

		A_VAL(dst) = (A_VAL(dst) * a) >> 8;
	     }
	   if (l == 0)
	     {
		int a = 256;

                if (r < 0)
		  a = 1 + (int)(255 * (1.0 + r));

		A_VAL(dst) = (A_VAL(dst) * a) >> 8;
	     }
	  }
	dst++;  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_restrict(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;

   a00 = gdata->sp * (axx / 65536.0);
   a01 = gdata->sp * (axy / 65536.0);
   a10 = ayx / 65536.0;
   a11 = ayy / 65536.0;

   xf = (a00 * x) + (a01 * y);
   yf = (a10 * x) + (a11 * y);

   while (dst < dst_end)
     {
	int  l = (yf - (sa * sin(xf)));

	*dst = 0;
	if ((unsigned)l < map_len)
	   *dst = map[l];
	dst++;  xf += a00;  yf += a10;
     }
}

