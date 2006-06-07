#include "evas_common.h"
#include <math.h>

typedef struct _Sinusoidal_Data   Sinusoidal_Data;
struct _Sinusoidal_Data
{
   float sa, sp;
   float  off;
};

static Sinusoidal_Data  sinusoidal_data = {32.0, M_PI / 32.0, 0.0};


static void 
sinusoidal_setup_geom(RGBA_Gradient *gr, int spread);

static int 
sinusoidal_has_alpha(RGBA_Gradient *gr, int spread, int op);

static int 
sinusoidal_has_mask(RGBA_Gradient *gr, int spread, int op);

static int 
sinusoidal_get_map_len(RGBA_Gradient *gr, int spread);

static Gfx_Func_Gradient_Fill 
sinusoidal_get_fill_func(RGBA_Gradient *gr, int spread, int op, unsigned char aa);

static RGBA_Gradient_Type  sinusoidal = {"sinusoidal", &sinusoidal_data, sinusoidal_setup_geom, sinusoidal_has_alpha, sinusoidal_has_mask, sinusoidal_get_map_len, sinusoidal_get_fill_func};

/** internal functions **/

static void
sinusoidal_reflect(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_reflect_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_repeat(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                     int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_restrict_reflect(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                            int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_restrict_reflect_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_restrict_reflect_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_restrict_reflect_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_restrict_repeat(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_restrict_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_restrict_repeat_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_restrict_repeat_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                     int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_pad(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
sinusoidal_pad_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
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
   sinusoidal_data.off = gr->range_offset;

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
	    break;
	  }
     }
   if (!err)
     {
	sinusoidal_data.sa *= amp;
	if (per < 0.0) per = -per;
	sinusoidal_data.sp *= per;
     }
   free(s);
}


static int 
sinusoidal_has_alpha(RGBA_Gradient *gr, int spread, int op)
{
   if (!gr || (gr->type.geometer != &sinusoidal)) return 0;

   if (gr->has_alpha | gr->map.has_alpha)
	return 1;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) || 
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
	return 0;
   if ( (spread == _EVAS_TEXTURE_RESTRICT) ||
         (spread == _EVAS_TEXTURE_RESTRICT_REFLECT) ||
         (spread == _EVAS_TEXTURE_RESTRICT_REPEAT) )
	return 1;
   return 0;
}

static int 
sinusoidal_has_mask(RGBA_Gradient *gr, int spread, int op)
{
   if (!gr || (gr->type.geometer != &sinusoidal)) return 0;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) || 
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
     {
	if ( (spread == _EVAS_TEXTURE_RESTRICT) ||
	      (spread == _EVAS_TEXTURE_RESTRICT_REFLECT) ||
	      (spread == _EVAS_TEXTURE_RESTRICT_REPEAT) )
	    return 1;
     }

   return 0;
}

static int 
sinusoidal_get_map_len(RGBA_Gradient *gr, int spread)
{
   if (!gr || (gr->type.geometer != &sinusoidal)) return 0;

   return gr->fill.h;
}

static Gfx_Func_Gradient_Fill 
sinusoidal_get_fill_func(RGBA_Gradient *gr, int spread, int op, unsigned char aa)
{
   Gfx_Func_Gradient_Fill  sfunc = NULL;
   int masked_op = 0;

   if (!gr || (gr->type.geometer != &sinusoidal))
	return sfunc;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) || 
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
	masked_op = 1;

   switch (spread)
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
	 sinusoidal_data.off = 0;
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
sinusoidal_reflect(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;
   float  off = gdata->off * (map_len - 1);

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	int  l = (yf - (sa * sin(xf))) + off;

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
sinusoidal_reflect_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;
   float  off = gdata->off * (map_len - 1);

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	float  r = (yf - (sa * sin(xf))) + off;
	int    l = r, a;

	if (r < 0)  r = -r;
	a = 1 + (int)(255 * (r - (int)r));

	if (l < 0)  l = -l;
	if (l >= map_len)
	  {
	    int  m = (l % (2 * map_len));

	    l = (l % map_len);
	    if (m >= map_len)
		{ l = map_len - l - 1;  a = 257 - a; }
	  }
	*dst = map[l];
	if (l + 1 < map_len)
	  *dst = INTERP_256(a, map[l + 1], *dst);

	dst++;  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_repeat(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;
   float  off = gdata->off * (map_len - 1);

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	int  l = (yf - (sa * sin(xf))) + off;

	l = (l % map_len);
	if (l < 0)
	    l += map_len;
	*dst++ = map[l];  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                     int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;
   float  off = gdata->off * (map_len - 1);

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	float  r = (yf - (sa * sin(xf))) + off;
	int    l = r, a;

	if (r < 0) r = -r;
	a = 1 + (int)(255 * (r - (int)r));

	l = l % map_len;
	if (l < 0)
	  { l += map_len;  a = 257 - a; }

	*dst = map[l];
	if (l + 1 < map_len)
	   *dst = INTERP_256(a, map[l + 1], *dst);
	if (l == (map_len - 1))
	   *dst = INTERP_256(a, map[0], *dst);
	dst++;  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_restrict_reflect(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                            int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;
   float  off = gdata->off * (map_len - 1);

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	int  l = (yf - (sa * sin(xf)));

	*dst = 0;
	if ((unsigned)l < map_len)
	  {
	    l += off;
	    if (l < 0) l = -l;
	    if (l >= map_len)
	      {
		int  m = (l % (2 * map_len));

		l = (l % map_len);
		if (m >= map_len)
		   l = map_len - l - 1;
	      }
	    *dst = map[l];
	  }
	dst++;  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_restrict_reflect_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;
   float  off = gdata->off * (map_len - 1);

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	float  r = (yf - (sa * sin(xf))) + off, s = r - off;;
	int    l = s;

	*dst = 0;
	if ((unsigned)l < map_len)
	  {
	    int  a, lp = r;

	    if (r < 0)  r = -r;
	    a = 1 + (int)(255 * (r - (int)r));
	    if (lp < 0) lp = -lp;
	    if (lp >= map_len)
	      {
	        int  m = (lp % (2 * map_len));

		lp = (lp % map_len);
		if (m >= map_len)
		  { lp = map_len - lp - 1;  a = 257 - a; }
	      }
	    *dst = map[lp];
	    if (lp + 1 < map_len)
		*dst = INTERP_256(a, map[lp + 1], *dst);
	    if ((l == 0) && (s < 0))
	      {
		a = 256 + (255 * s);
		*dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
	      }
	    if (l == (map_len - 1))
	      {
		a = 256 - (255 * (s - l));
		*dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
	      }
	  }
	dst++;  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_restrict_reflect_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;
   float  off = gdata->off * (map_len - 1);

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	int  l = (yf - (sa * sin(xf)));

	*dst = 0;  *mask = 0;
	if ((unsigned)l < map_len)
	  {
	    l += off;
	    if (l < 0) l = -l;
	    if (l >= map_len)
	      {
		int  m = (l % (2 * map_len));

		l = (l % map_len);
		if (m >= map_len)
		   l = map_len - l - 1;
	      }
	    *dst = map[l];  *mask = 255;
	  }
	dst++;  mask++;  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_restrict_reflect_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;
   float  off = gdata->off * (map_len - 1);

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	float  r = (yf - (sa * sin(xf))) + off, s = r - off;;
	int    l = s;

	*dst = 0;  *mask = 0;
	if ((unsigned)l < map_len)
	  {
	    int  a, lp = r;

	    if (r < 0)  r = -r;
	    a = 1 + (int)(255 * (r - (int)r));
	    if (lp < 0) lp = -lp;
	    if (lp >= map_len)
	      {
	        int  m = (lp % (2 * map_len));

		lp = (lp % map_len);
		if (m >= map_len)
		  { lp = map_len - lp - 1;  a = 257 - a; }
	      }
	    *dst = map[lp];  *mask = 255;
	    if (lp + 1 < map_len)
		*dst = INTERP_256(a, map[lp + 1], *dst);
	    if ((l == 0) && (s < 0))
		*mask = 255 + (255 * s);
	    if (l == (map_len - 1))
		*mask = 255 - (255 * (s - l));
	  }
	dst++;  mask++;  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_restrict_repeat(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;
   float  off = gdata->off * (map_len - 1);

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	int  l = (yf - (sa * sin(xf)));

	*dst = 0;
	if ((unsigned)l < map_len)
	  {
	    l += off;
	    l = l % map_len;
	    if (l < 0)
		l += map_len;
	    *dst = map[l];
	  }
	dst++;  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_restrict_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;
   float  off = gdata->off * (map_len - 1);

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	float  r = (yf - (sa * sin(xf))) + off, s = r - off;;
	int    l = s;

	*dst = 0;
	if ((unsigned)l < map_len)
	  {
	    int  a, lp = r;

	    if (r < 0)  r = -r;
	    a = 1 + (int)(255 * (r - (int)r));
	    lp = lp % map_len;
	    if (lp < 0)
	      { lp += map_len;  a = 257 - a; }
	    *dst = map[lp];
	    if (lp + 1 < map_len)
		*dst = INTERP_256(a, map[lp + 1], *dst);
	    if (lp == (map_len - 1))
		*dst = INTERP_256(a, map[0], *dst);
	    if ((l == 0) && (s < 0))
	      {
		a = 256 + (255 * s);
		*dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
	      }
	    if (l == (map_len - 1))
	      {
		a = 256 - (255 * (s - l));
		*dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
	      }
	  }
	dst++;  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_restrict_repeat_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;
   float  off = gdata->off * (map_len - 1);

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	int  l = (yf - (sa * sin(xf)));

	*dst = 0;  *mask = 0;
	if ((unsigned)l < map_len)
	  {
	    l += off;
	    l = l % map_len;
	    if (l < 0)
		l += map_len;
	    *dst = map[l];  *mask = 255;
	  }
	dst++;  mask++;  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_restrict_repeat_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                     int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Sinusoidal_Data  *gdata = (Sinusoidal_Data *)params_data;
   float  xf, yf, sa = gdata->sa;
   float  a00, a01, a10, a11;
   float  off = gdata->off * (map_len - 1);

   SETUP_SINU_FILL

   while (dst < dst_end)
     {
	float  r = (yf - (sa * sin(xf))) + off, s = r - off;;
	int    l = s;

	*dst = 0;  *mask = 0;
	if ((unsigned)l < map_len)
	  {
	    int  a, lp = r;

	    if (r < 0)  r = -r;
	    a = 1 + (int)(255 * (r - (int)r));
	    lp = lp % map_len;
	    if (lp < 0)
	      { lp += map_len;  a = 257 - a; }
	    *dst = map[lp];  *mask = 255;
	    if (lp + 1 < map_len)
		*dst = INTERP_256(a, map[lp + 1], *dst);
	    if (lp == (map_len - 1))
		*dst = INTERP_256(a, map[0], *dst);
	    if ((l == 0) && (s < 0))
		*mask = 255 + (255 * s);
	    if (l == (map_len - 1))
		*mask = 255 - (255 * (s - l));
	  }
	dst++;  mask++;  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_pad(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
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
	if (l >= map_len)
	    l = map_len - 1;
	*dst = map[l];
	dst++;  xf += a00;  yf += a10;
     }
}

static void
sinusoidal_pad_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
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
	if ((unsigned)l < (map_len - 1))
	  {
	    int  a;

	    if (r < 0) r = -r;
	    a = 1 + (int)(255 * (r - (int)r));
	    *dst = INTERP_256(a, map[l + 1], map[l]);
	  }
	if (l < 0) *dst = map[0];
	if (l >= map_len) *dst = map[map_len - 1];
	dst++;  xf += a00;  yf += a10;
     }
}
