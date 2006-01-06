#include "evas_common.h"
#include <math.h>


typedef struct _Linear_Data   Linear_Data;
struct _Linear_Data
{
   int    sx;
};

static Linear_Data  linear_data = {-1};


static void
linear_setup_geom(RGBA_Gradient *gr, int spread);

static int
linear_has_alpha(RGBA_Gradient *gr, int spread);

static int
linear_get_map_len(RGBA_Gradient *gr, int spread);

static Gfx_Func_Gradient_Span
linear_get_span_func(RGBA_Gradient *gr, int spread, unsigned char aa);

static RGBA_Gradient_Type  linear = {"linear", &linear_data, linear_setup_geom, linear_has_alpha, linear_get_map_len, linear_get_span_func};


/** internal functions **/
static void
linear_reflect(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_reflect_aa_cropped(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_reflect_cropped(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_repeat(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_repeat_aa_cropped(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                         int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_repeat_cropped(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_cropped(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                        int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_aa(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_aa_cropped(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);



RGBA_Gradient_Type  *
evas_common_gradient_linear_get(void)
{
    return &linear;
}


static void
linear_setup_geom(RGBA_Gradient *gr, int spread)
{
   int    err = 1;
   char   *s, *p, key[256];
   float  val = -1;

   if (!gr || (gr->type.geometer != &linear)) return;

   linear_data.sx = -1;
   if (!gr->type.params || !*(gr->type.params))
	return;

   s = strdup(gr->type.params);
   if (!s) return;

   p = s;
   while ((p = evas_common_gradient_get_key_fval(p, key, &val)))
     {
	if (!strcmp(key, "extent"))
	    err = 0;
	else
	  {
	    err = 1;
	    goto done;
	  }
     }
   done:
   if (!err)
     {
	if (val < 0.0)
	   val = -val;   
	linear_data.sx = val * gr->fill.w;
     }
   free(s);
}


static int
linear_has_alpha(RGBA_Gradient *gr, int spread)
{
   if (!gr || (gr->type.geometer != &linear)) return 0;
   if (gr->map.has_alpha)
	return 1;
   if (linear_data.sx >= 0)
	return 1;
   if (spread == _EVAS_TEXTURE_RESTRICT)
	return 1;
   return 0;
}

static int
linear_get_map_len(RGBA_Gradient *gr, int spread)
{
   if (!gr || (gr->type.geometer != &linear)) return 0;
   return gr->fill.h;
}

static Gfx_Func_Gradient_Span
linear_get_span_func(RGBA_Gradient *gr, int spread, unsigned char aa)
{
   Gfx_Func_Gradient_Span  sfunc = NULL;

   if (!gr || (gr->type.geometer != &linear)) return sfunc;
   switch (spread)
     {
      case _EVAS_TEXTURE_REFLECT:
	{
	 if (aa)
	   {
	    if (linear_data.sx >= 0)
	       sfunc = linear_reflect_aa_cropped;
	    else
	       sfunc = linear_reflect;
	   }
	 else
	   {
	    if (linear_data.sx >= 0)
	       sfunc = linear_reflect_cropped;
	    else
	       sfunc = linear_reflect;
	   }
	}
      break;
      case _EVAS_TEXTURE_REPEAT:
	{
	 if (aa)
	   {
	    if (linear_data.sx >= 0)
	       sfunc = linear_repeat_aa_cropped;
	    else
	       sfunc = linear_repeat_aa;
	   }
	 else
	   {
	    if (linear_data.sx >= 0)
	       sfunc = linear_repeat_cropped;
	    else
	       sfunc = linear_repeat;
	   }
	}
      break;
      case _EVAS_TEXTURE_RESTRICT:
	{
	 if (aa)
	   {
	    if (linear_data.sx >= 0)
	       sfunc = linear_restrict_aa_cropped;
	    else
	       sfunc = linear_restrict_aa;
	   }
	 else
	   {
	    if (linear_data.sx >= 0)
	       sfunc = linear_restrict_cropped;
	    else
	       sfunc = linear_restrict;
	   }
	}
      break;
      default:
	sfunc = linear_reflect;
      break;
     }
   return sfunc;
}

static void
linear_reflect(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   int      yy = ayx * x + ayy * y;
   
   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	l += (yy - (l << 16)) >> 15;
	if (l < 0)  l = -l;

	if (l >= map_len)
	  {
	    int  m = (l % (2 * map_len));

	    l = (l % map_len);
	    if (m >= map_len)
	        l = map_len - l - 1;
	  }
	*dst++ = map[l];  yy += ayx;
     }
}

static void
linear_reflect_aa_cropped(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;

   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	*dst = 0;
	if ((unsigned)ex < w)
	  {
	   int  l = (yy >> 16);

	   l += (yy - (l << 16)) >> 15;
	   if (l < 0)  l = -l;

	   if (l >= map_len)
	     {
	        int  m = (l % (2 * map_len));

		l = (l % map_len);
		if (m >= map_len)
		    l = map_len - l - 1;
	     }
	   *dst = map[l];
	    if (ex == 0)
	      {
		int a = 1 + (xx >> 8);

		A_VAL(dst) = (A_VAL(dst) * a) >> 8;
	      }
	    if (ex == (w - 1))
	      {
		int a = 256 - ((xx - (ex << 16)) >> 8);

		A_VAL(dst) = (A_VAL(dst) * a) >> 8;
	      }
	  }
	dst++;  yy += ayx;  xx += axx;
     }
}

static void
linear_reflect_cropped(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;

   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	ex += (xx - (ex << 16)) >> 15;
	*dst = 0;
	if ((unsigned)ex < w)
	  {
	   int  l = (yy >> 16);

	   l += (yy - (l << 16)) >> 15;
	   if (l < 0)  l = -l;
	   if (l >= map_len)
	     {
	        int m = (l % (2 * map_len));

		l = (l % map_len);
		if (m >= map_len)
		    l = map_len - l - 1;
	     }
	   *dst = map[l];
	  }
	dst++;  yy += ayx;  xx += axx;
     }
}

static void
linear_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   int      yy = ayx * x + ayy * y;

   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	l = l % map_len;
	if (l < 0)
	   l += map_len;

	*dst = map[l];
	if (l == 0)
	  {
	    int  a = 256 - ((yy - ((yy >> 16) << 16)) >> 8);
	    DATA32  *c = map + (map_len - 1);

	    *dst += ARGB_JOIN((a * (A_VAL(c) - A_VAL(dst))) >> 8,
	                      (a * (R_VAL(c) - R_VAL(dst))) >> 8,
	                      (a * (G_VAL(c) - G_VAL(dst))) >> 8,
	                      (a * (B_VAL(c) - B_VAL(dst))) >> 8);
	  }
	dst++;  yy += ayx;
     }
}

static void
linear_repeat_aa_cropped(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                         int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;

   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	*dst = 0;
	if ((unsigned)ex < w)
	  {
	   int  l = (yy >> 16);

	   l = l % map_len;
	   if (l < 0)
	      l += map_len;

	   *dst = map[l];
	   if (ex == 0)
	     {
		int a = 1 + (xx >> 8);

		A_VAL(dst) = (A_VAL(dst) * a) >> 8;
	     }
	   if (ex == (w - 1))
	     {
		int a = 256 - ((xx - (ex << 16)) >> 8);

		A_VAL(dst) = (A_VAL(dst) * a) >> 8;
	     }
	   if ((l == 0) && (ex != 0))
	     {
		int  a = 256 - ((yy - ((yy >> 16) << 16)) >> 8);
		DATA32  *c = map + (map_len - 1);

		*dst += ARGB_JOIN((a * (A_VAL(c) - A_VAL(dst))) >> 8,
		                  (a * (R_VAL(c) - R_VAL(dst))) >> 8,
		                  (a * (G_VAL(c) - G_VAL(dst))) >> 8,
		                  (a * (B_VAL(c) - B_VAL(dst))) >> 8);
	     }
	  }
	dst++;  yy += ayx;  xx += axx;
     }
}

static void
linear_repeat(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   int      yy = ayx * x + ayy * y;

   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	l += (yy - (l << 16)) >> 15;
	l = l % map_len;
	if (l < 0)
	   l += map_len;
	*dst++ = map[l];  yy += ayx;
     }
}

static void
linear_repeat_cropped(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;

   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	ex += (xx - (ex << 16)) >> 15;
	*dst = 0;
	if ((unsigned)ex < w)
	  {
	   int  l = (yy >> 16);

	   l += (yy - (l << 16)) >> 15;
	   l = l % map_len;
	   if (l < 0)
	      l += map_len;
	   *dst = map[l];
	  }
	dst++;  yy += ayx;  xx += axx;
     }
}

static void
linear_restrict_aa(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   int      yy = ayx * x + ayy * y;

   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	*dst = 0;
	if ((unsigned)l < map_len)
	  {
	    *dst = map[l];
	    if (l == (map_len - 1))
	      {
		int a = 256 - ((yy - (l << 16)) >> 8);

		A_VAL(dst) = (A_VAL(dst) * a) >> 8;
	      }
	    if (l == 0)
	      {
		int a = 1 + (yy >> 8);

		A_VAL(dst) = (A_VAL(dst) * a) >> 8;
	      }
	  }
	dst++;  yy += ayx;
     }
}

static void
linear_restrict_aa_cropped(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                        int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;

   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	*dst = 0;
	if ((unsigned)ex < w)
	  {
	   int  l = (yy >> 16);

	   if ((unsigned)l < map_len)
	     {
		*dst = map[l];
		if (ex == 0)
		  {
		   int a = 1 + (xx >> 8);

		   A_VAL(dst) = (A_VAL(dst) * a) >> 8;
		  }
		if (ex == (w - 1))
		  {
		   int a = 256 - ((xx - (ex << 16)) >> 8);

		   A_VAL(dst) = (A_VAL(dst) * a) >> 8;
		  }
		if (l == 0)
		  {
		   int a = 1 + (yy >> 8);

		   A_VAL(dst) = (A_VAL(dst) * a) >> 8;
		  }
		if (l == (map_len - 1))
		  {
		   int a = 256 - ((yy - (l << 16)) >> 8);

		   A_VAL(dst) = (A_VAL(dst) * a) >> 8;
		  }
	     }
	  }
	dst++;  yy += ayx;  xx += axx;
     }
}

static void
linear_restrict(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
             int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   int      yy = (ayx * x) + (ayy * y);

   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	l += (yy - (l << 16)) >> 15;
	*dst = 0;
	if ((unsigned)l < map_len)
	   *dst = map[l];
	dst++;  yy += ayx;
     }
}

static void
linear_restrict_cropped(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                     int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;

   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	ex += (xx - (ex << 16)) >> 15;

	*dst = 0;
	if ((unsigned)ex < w)
	  {
	   int  l = (yy >> 16);

	   l += (yy - (l << 16)) >> 15;
	   if ((unsigned)l < map_len)
	      *dst = map[l];
	  }
	dst++;  yy += ayx;  xx += axx;
     }
}


