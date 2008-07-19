/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <math.h>

#include "evas_common.h"
#include "evas_gradient_private.h"

typedef struct _Angular_Data   Angular_Data;
struct _Angular_Data
{
   float  an, cy;

   int    sx, sy, s;
   float  off;
   int    len;
};

static void
angular_init(void);

static void
angular_shutdown(void);

static void
angular_init_geom(RGBA_Gradient *gr);

static void
angular_setup_geom(RGBA_Gradient *gr);

static void
angular_free_geom(void *gdata);

static int
angular_has_alpha(RGBA_Gradient *gr, int op);

static int
angular_has_mask(RGBA_Gradient *gr, int op);

static int
angular_get_map_len(RGBA_Gradient *gr);

static Gfx_Func_Gradient_Fill
angular_get_fill_func(RGBA_Gradient *gr, int op, unsigned char aa);

static RGBA_Gradient_Type  angular = {"angular", angular_init, angular_shutdown,
				      angular_init_geom, angular_setup_geom, angular_free_geom,
				      angular_has_alpha, angular_has_mask,
				      angular_get_map_len, angular_get_fill_func};


/** internal functions **/

static void
angular_reflect(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_reflect_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_reflect_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                        int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_reflect_aa_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_reflect_masked_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_reflect_aa_masked_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_repeat(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_repeat_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_repeat_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_repeat_aa_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_repeat_masked_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_repeat_aa_masked_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_reflect(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                         int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_reflect_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                            int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_reflect_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_reflect_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_reflect_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_reflect_aa_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                    int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_reflect_masked_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                        int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_reflect_aa_masked_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_repeat(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                         int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_repeat_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_repeat_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_repeat_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_repeat_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_repeat_aa_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_repeat_masked_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_repeat_aa_masked_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_pad(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
            int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_pad_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_pad_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                    int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_pad_aa_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_pad_masked_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_pad_aa_masked_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);



RGBA_Gradient_Type  *
evas_common_gradient_angular_get(void)
{
    return &angular;
}

static void
angular_init(void)
{
}

static void
angular_shutdown(void)
{
}

static void
angular_free_geom(void *gdata)
{
   Angular_Data *data = (Angular_Data *)gdata;
   if (data) free(data);
}

static void
angular_setup_geom(RGBA_Gradient *gr)
{
   Angular_Data   *angular_data;

   if (!gr || (gr->type.geometer != &angular)) return;

   angular_data = (Angular_Data *)gr->type.gdata;
   if (!angular_data) return;
   angular_data->sx = gr->fill.w;
   angular_data->sy = gr->fill.h;
   angular_data->s = angular_data->sx;
   if (angular_data->sy > angular_data->sx)
	angular_data->s = angular_data->sy;
   angular_data->off = gr->map.offset;
   angular_data->len = (2 * M_PI) * angular_data->s * angular_data->cy;
}

static void
angular_init_geom(RGBA_Gradient *gr)
{
   Angular_Data   *angular_data;
   int    err = 1;
   char   *s, *p, key[256];
   float  val, an, cy;

   if (!gr || (gr->type.geometer != &angular)) return;

   angular_data = (Angular_Data *)gr->type.gdata;
   if (!angular_data)
     {
	angular_data = calloc(1, sizeof(Angular_Data));
	if (!angular_data)  return;
	angular_data->an = -1.0;
	angular_data->cy = 1.0;
	angular_data->sx = 32;
	angular_data->sy = 32;
	angular_data->s = 32;
	angular_data->off = 0.0;
	angular_data->len = (2 * M_PI) * 32;
     }
   gr->type.gdata = angular_data;
   if (!gr->type.params || !*(gr->type.params))
	return;

   s = strdup(gr->type.params);
   if (!s) return;

   an = angular_data->an;
   cy = angular_data->cy;
   p = s;
   while ((p = evas_common_gradient_get_key_fval(p, key, &val)))
     {
	if (!strcmp(key, "annulus"))
	  {
	    err = 0;
	    an = val;
	  }
	else if (!strcmp(key, "wrap"))
	  {
	    err = 0;
	    cy = val;
	  }
	else
	  {
	    err = 1;
	    break;
	  }
     }
   if (!err)
     {
	if (an < 0.0) an = 0.0;
	if (an > 1.0) an = 1.0;
	angular_data->an = an;
	if (cy < 0.0) cy = 0.0;
	if (cy > 1.0) cy = 1.0;
	angular_data->cy = cy;
     }
   free(s);
}


static int
angular_has_alpha(RGBA_Gradient *gr, int op)
{
   Angular_Data   *angular_data;

   if (!gr || (gr->type.geometer != &angular)) return 0;

   if (gr->has_alpha | gr->map.has_alpha)
	return 1;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) ||
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
	return 0;
   angular_data = (Angular_Data *)gr->type.gdata;
   if (!angular_data)  return 0;
   if ((int)angular_data->an >= 0)
	return 1;
   if ( ((gr->fill.spread == _EVAS_TEXTURE_RESTRICT) ||
         (gr->fill.spread == _EVAS_TEXTURE_RESTRICT_REFLECT) ||
         (gr->fill.spread == _EVAS_TEXTURE_RESTRICT_REPEAT))
	 && (angular_data->cy < 1.0) )
	return 1;
   return 0;
}

static int
angular_has_mask(RGBA_Gradient *gr, int op)
{
   Angular_Data   *angular_data;

   if (!gr || (gr->type.geometer != &angular)) return 0;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) ||
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
     {
	angular_data = (Angular_Data *)gr->type.gdata;
	if (!angular_data)  return 0;
	if ((int)angular_data->an >= 0)
	    return 1;
	if ( ((gr->fill.spread == _EVAS_TEXTURE_RESTRICT) ||
	      (gr->fill.spread == _EVAS_TEXTURE_RESTRICT_REFLECT) ||
	      (gr->fill.spread == _EVAS_TEXTURE_RESTRICT_REPEAT))
	      && (angular_data->cy < 1.0) )
	    return 1;
     }
   return 0;
}

static int
angular_get_map_len(RGBA_Gradient *gr)
{
   Angular_Data   *angular_data;

   if (!gr || (gr->type.geometer != &angular)) return 0;
   angular_data = (Angular_Data *)gr->type.gdata;
   if (!angular_data)  return 0;
   return angular_data->len;
}

static Gfx_Func_Gradient_Fill
angular_get_fill_func(RGBA_Gradient *gr, int op, unsigned char aa)
{
   Gfx_Func_Gradient_Fill  sfunc = NULL;
   int masked_op = 0;
   Angular_Data   *angular_data;

   if (!gr || (gr->type.geometer != &angular)) return sfunc;
   angular_data = (Angular_Data *)gr->type.gdata;
   if (!angular_data)  return sfunc;

   angular_data->off = gr->map.offset;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) ||
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
	masked_op = 1;

   switch (gr->fill.spread)
     {
      case _EVAS_TEXTURE_REFLECT:
	{
	 if (aa)
	   {
	    if ((int)angular_data->an >= 0)
	      {
		if (masked_op)
		   sfunc = angular_reflect_aa_masked_annulus;
		else
		   sfunc = angular_reflect_aa_annulus;
	      }
	    else
	       sfunc = angular_reflect_aa;
	   }
	 else
	   {
	    if ((int)angular_data->an >= 0)
	      {
		if (masked_op)
		   sfunc = angular_reflect_masked_annulus;
		else
		   sfunc = angular_reflect_annulus;
	      }
	    else
	       sfunc = angular_reflect;
	   }
	}
      break;
      case _EVAS_TEXTURE_REPEAT:
	{
	 if (aa)
	   {
	    if ((int)angular_data->an >= 0)
	      {
		if (masked_op)
		   sfunc = angular_repeat_aa_masked_annulus;
		else
		   sfunc = angular_repeat_aa_annulus;
	      }
	    else
	       sfunc = angular_repeat_aa;
	   }
	 else
	   {
	    if ((int)angular_data->an >= 0)
	      {
		if (masked_op)
		   sfunc = angular_repeat_masked_annulus;
		else
		   sfunc = angular_repeat_annulus;
	      }
	    else
	       sfunc = angular_repeat;
	   }
	}
      break;
      case _EVAS_TEXTURE_RESTRICT:
	 angular_data->off = 0;
      case _EVAS_TEXTURE_RESTRICT_REFLECT:
	{
	 if (aa)
	   {
	    if ((int)angular_data->an >= 0)
	      {
		if (masked_op)
		   sfunc = angular_restrict_reflect_aa_masked_annulus;
		else
		   sfunc = angular_restrict_reflect_aa_annulus;
	      }
	    else if (angular_data->cy < 1.0)
	      {
		if (masked_op)
		   sfunc = angular_restrict_reflect_aa_masked;
		else
		   sfunc = angular_restrict_reflect_aa;
	      }
	    else
	       sfunc = angular_restrict_reflect_aa;
	   }
	 else
	   {
	    if ((int)angular_data->an >= 0)
	      {
		if (masked_op)
		   sfunc = angular_restrict_reflect_masked_annulus;
		else
		   sfunc = angular_restrict_reflect_annulus;
	      }
	    else if (angular_data->cy < 1.0)
	      {
		if (masked_op)
		   sfunc = angular_restrict_reflect_masked;
		else
		   sfunc = angular_restrict_reflect;
	      }
	    else
	       sfunc = angular_restrict_reflect;
	   }
	}
      break;
      case _EVAS_TEXTURE_RESTRICT_REPEAT:
	{
	 if (aa)
	   {
	    if ((int)angular_data->an >= 0)
	      {
		if (masked_op)
		   sfunc = angular_restrict_repeat_aa_masked_annulus;
		else
		   sfunc = angular_restrict_repeat_aa_annulus;
	      }
	    else if (angular_data->cy < 1.0)
	      {
		if (masked_op)
		   sfunc = angular_restrict_repeat_aa_masked;
		else
		   sfunc = angular_restrict_repeat_aa;
	      }
	    else
	       sfunc = angular_restrict_repeat_aa;
	   }
	 else
	   {
	    if ((int)angular_data->an >= 0)
	      {
		if (masked_op)
		   sfunc = angular_restrict_repeat_masked_annulus;
		else
		   sfunc = angular_restrict_repeat_annulus;
	      }
	    else if (angular_data->cy < 1.0)
	      {
		if (masked_op)
		   sfunc = angular_restrict_repeat_masked;
		else
		   sfunc = angular_restrict_repeat;
	      }
	    else
	       sfunc = angular_restrict_repeat;
	   }
	}
      break;
      case _EVAS_TEXTURE_PAD:
	{
	 if (aa)
	   {
	    if ((int)angular_data->an >= 0)
	      {
		if (masked_op)
		   sfunc = angular_pad_aa_masked_annulus;
		else
		   sfunc = angular_pad_aa_annulus;
	      }
	    else
	       sfunc = angular_pad_aa;
	   }
	 else
	   {
	    if ((int)angular_data->an >= 0)
	      {
		if (masked_op)
		   sfunc = angular_pad_masked_annulus;
		else
		   sfunc = angular_pad_annulus;
	      }
	    else
	       sfunc = angular_pad;
	   }
	}
      default:
	sfunc = angular_reflect_aa;
      break;
     }
   return sfunc;
}

#define SETUP_ANGULAR_FILL \
   if (gdata->sx != gdata->s) \
     { \
	axx = (gdata->s * axx) / gdata->sx; \
	axy = (gdata->s * axy) / gdata->sx; \
     } \
   if (gdata->sy != gdata->s) \
     { \
	ayy = (gdata->s * ayy) / gdata->sy; \
	ayx = (gdata->s * ayx) / gdata->sy; \
     } \
   xx = (axx * x) + (axy * y); \
   yy = (ayx * x) + (ayy * y);


static void
angular_reflect(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  ll = ss * (M_PI + atan2(yy, xx));
	int  l = (ll >> 16);

	l += ((ll - (l << 16)) >> 15) + off;
	if (l < 0) l = -l;
	if (l >= src_len)
	  {
	    int  m = (l % (2 * src_len));

	    l = (l % src_len);
	    if (m >= src_len)
		l = src_len - l - 1;
	  }
	*dst = src[l];
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_reflect_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  ll = ss * (M_PI + atan2(yy, xx));
	int  l = (ll >> 16), lp;
	DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	lp = l + off;
	if (lp < 0) { lp = -lp;  a = 257 - a; }
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
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_reflect_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                        int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;
	if ((r >= r0) && (r <= r1))
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = (ll >> 16);

	    l += ((ll - (l << 16)) >> 15) + off;
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
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_reflect_aa_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   int     rr0 = r0 << 16, rr1 = r1 << 16;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;
	if ((r >= r0) && (r <= r1))
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = (ll >> 16), lp;
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	    lp = l + off;
	    if (lp < 0) { lp = -lp;  a = 257 - a; }
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
	    if (r == r0)
	      {
		a = 1 + ((rr - rr0) >> 8);
		*dst = MUL_256(a, *dst);
	      }
	    if (r == r1)
	      {
		a = 256 - ((rr - rr1) >> 8);
		*dst = MUL_256(a, *dst);
	      }
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_reflect_masked_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;  *mask = 0;
	if ((r >= r0) && (r <= r1))
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx)), l = ll >> 16;

	    l += ((ll - (l << 16)) >> 15) + off;
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
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_reflect_aa_masked_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   int     rr0 = r0 << 16, rr1 = r1 << 16;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;  *mask = 0;
	if ((r >= r0) && (r <= r1))
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = (ll >> 16), lp;
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	    lp = l + off;
	    if (lp < 0) { lp = -lp;  a = 257 - a; }
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
	    if (r == r0)
		*mask = ((rr - rr0) >> 8);
	    if (r == r1)
		*mask = 255 - ((rr - rr1) >> 8);
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_repeat(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  ll = ss * (M_PI + atan2(yy, xx));
	int  l = ll >> 16;

	l += ((ll - (l << 16)) >> 15) + off;
	l = l % src_len;
	if (l < 0)
	   l += src_len;
	*dst = src[l];
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_repeat_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  ll = ss * (M_PI + atan2(yy, xx));
	int  l = (ll >> 16), lp;
	DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	lp = l + off;
	lp = lp % src_len;
	if (lp < 0)
	   lp += src_len;
	*dst = src[lp];
	if (lp + 1 < src_len)
	   *dst = INTERP_256(a, src[lp + 1], *dst);
	if (lp == (src_len - 1))
	   *dst = INTERP_256(a, src[0], *dst);
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_repeat_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;
	if ((r >= r0) && (r <= r1))
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = ll >> 16;

	    l += ((ll - (l << 16)) >> 15) + off;
	    l = l % src_len;
	    if (l < 0)
		l += src_len;
	   *dst = src[l];
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_repeat_aa_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   int     rr0 = r0 << 16, rr1 = r1 << 16;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;
	if ((r >= r0) && (r <= r1))
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = ll >> 16, lp;
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	    lp = l + off;
	    lp = lp % src_len;
	    if (lp < 0)
		lp += src_len;
	    *dst = src[lp];
	    if (lp + 1 < src_len)
		*dst = INTERP_256(a, src[lp + 1], *dst);
	    if (lp == (src_len - 1))
		*dst = INTERP_256(a, src[0], *dst);
	    if (r == r0)
	      {
		a = 1 + ((rr - rr0) >> 8);
		*dst = MUL_256(a, *dst);
	      }
	    if (r == r1)
	      {
		a = 256 - ((rr - rr1) >> 8);
		*dst = MUL_256(a, *dst);
	      }
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_repeat_masked_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;  *mask = 0;
	if ((r >= r0) && (r <= r1))
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = ll >> 16;

	    l += ((ll - (l << 16)) >> 15) + off;
	    l = l % src_len;
	    if (l < 0)
		l += src_len;
	    *dst = src[l];  *mask = 255;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_repeat_aa_masked_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   int     rr0 = r0 << 16, rr1 = r1 << 16;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;  *mask = 0;
	if ((r >= r0) && (r <= r1))
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = ll >> 16, lp;
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	    lp = l + off;
	    lp = lp % src_len;
	    if (lp < 0)
		lp += src_len;
	    *dst = src[lp];  *mask = 255;
	    if (lp + 1 < src_len)
		*dst = INTERP_256(a, src[lp + 1], *dst);
	    if (lp == (src_len - 1))
		*dst = INTERP_256(a, src[0], *dst);
	    if (r == r0)
		*mask = ((rr - rr0) >> 8);
	    if (r == r1)
		*mask = 255 - ((rr - rr1) >> 8);
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_reflect(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                         int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  ll = ss * (M_PI + atan2(yy, xx));
	int  l = ll >> 16;

	*dst = 0;
	l += ((ll - (l << 16)) >> 15);
	if (l < src_len)
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
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_reflect_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                            int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  ll = ss * (M_PI + atan2(yy, xx));
	int  l = ll >> 16, lp;

	*dst = 0;
	if (l < src_len)
	  {
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8), a0 = a;

	    lp = l + off;
	    if (lp < 0) { lp = -lp;  a = 257 - a; }
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
	    if (l == (src_len - 1))
		*dst = MUL_256(257 - a0, *dst);
	    if (l == 0)
		*dst = MUL_256(a0, *dst);
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_reflect_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  ll = ss * (M_PI + atan2(yy, xx)), l = ll >> 16;

	*dst = 0;  *mask = 0;
	l += (ll - (l << 16)) >> 15;
	if (l < src_len)
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
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_reflect_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  ll = ss * (M_PI +  atan2(yy, xx));
	int  l = ll >> 16, lp;

	*dst = 0;  *mask = 0;
	if (l < src_len)
	  {
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8), a0 = a - 1;

	    lp = l + off;
	    if (lp < 0) { lp = -lp;  a = 257 - a; }
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
	    if (l == (src_len - 1))
		*mask = 255 - a0;
	    if (l == 0)
		*mask = a0;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_reflect_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;
	if ( (r >= r0) && (r <= r1) )
	  {
	    int  ll = ss * (M_PI +  atan2(yy, xx));
	    int  l = ll >> 16;

	    l += (ll - (l << 16)) >> 15;
	    if (l < src_len)
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
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_reflect_aa_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                    int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   int     rr0 = r0 << 16, rr1 = r1 << 16;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;
	if ( (r >= r0) && (r <= r1) )
	  {
	   int  ll = ss * (M_PI + atan2(yy, xx));
	   int  l = ll >> 16, lp;

	   if (l < src_len)
	     {
		DATA32 a = 1 + ((ll - (l << 16)) >> 8), a0 = a;

		lp = l + off;
		if (lp < 0) { lp = -lp;  a = 257 - a; }
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
	       if (l == (src_len - 1))
		   *dst = MUL_256(257 - a0, *dst);
	       if (l == 0)
		   *dst = MUL_256(a0, *dst);
	       if (r == r0)
	         {
		   a = 1 + ((rr - rr0) >> 8);
		   *dst = MUL_256(a, *dst);
	         }
	       if (r == r1)
	         {
		   a = 256 - ((rr - rr1) >> 8);
		   *dst = MUL_256(a, *dst);
	         }
	     }
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_reflect_masked_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                        int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;  *mask = 0;
	if ( (r >= r0) && (r <= r1) )
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = ll >> 16;

	    l += (ll - (l << 16)) >> 15;
	    if (l < src_len)
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
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_reflect_aa_masked_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   int     rr0 = r0 << 16, rr1 = r1 << 16;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;  *mask = 0;
	if ( (r >= r0) && (r <= r1) )
	  {
	   int  ll = ss * (M_PI + atan2(yy, xx));
	   int  l = ll >> 16, lp;

	   if (l < src_len)
	     {
		DATA32 a = 1 + ((ll - (l << 16)) >> 8), a0 = a - 1;

		lp = l + off;
		if (lp < 0) { lp = -lp;  a = 257 - a; }
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
	       if (l == (src_len - 1))
		   *mask = 255 - a0;
	       if (l == 0)
		   *mask = a0;
	       if (r == r0)
		   *mask = ((rr - rr0) >> 8);
	       if (r == r1)
		   *mask = 255 - ((rr - rr1) >> 8);
	     }
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_repeat(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                         int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  ll = ss * (M_PI + atan2(yy, xx));
	int  l = ll >> 16;

	*dst = 0;
	l += ((ll - (l << 16)) >> 15);
	if (l < src_len)
	  {
	    l += off;
	    l = l % src_len;
	    if (l < 0)
		l += src_len;
	    *dst = src[l];
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_repeat_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                            int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  ll = ss * (M_PI + atan2(yy, xx));
	int  l = ll >> 16, lp;

	*dst = 0;
	if (l < src_len)
	  {
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8), a0 = a;

	    lp = l + off;
	    lp = lp % src_len;
	    if (lp < 0)
		lp += src_len;
	    *dst = src[lp];
	    if (lp + 1 < src_len)
		*dst = INTERP_256(a, src[lp + 1], *dst);
	    if (lp == (src_len - 1))
		*dst = INTERP_256(a, src[0], *dst);
	    if (l == (src_len - 1))
		*dst = MUL_256(257 - a0, *dst);
	    if (l == 0)
		*dst = MUL_256(a0, *dst);
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_repeat_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  ll = ss * (M_PI + atan2(yy, xx)), l = ll >> 16;

	*dst = 0;  *mask = 0;
	l += (ll - (l << 16)) >> 15;
	if (l < src_len)
	  {
	    l += off;
	    l = l % src_len;
	    if (l < 0)
		l += src_len;
	    *dst = src[l];  *mask = 255;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_repeat_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  ll = ss * (M_PI +  atan2(yy, xx));
	int  l = ll >> 16, lp;

	*dst = 0;  *mask = 0;
	if (l < src_len)
	  {
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8), a0 = a - 1;

	    lp = l + off;
	    lp = lp % src_len;
	    if (lp < 0)
		lp += src_len;
	    *dst = src[lp];  *mask = 255;
	    if (lp + 1 < src_len)
		*dst = INTERP_256(a, src[lp + 1], *dst);
	    if (lp == (src_len - 1))
		*dst = INTERP_256(a, src[0], *dst);
	    if (l == (src_len - 1))
		*mask = 255 - a0;
	    if (l == 0)
		*mask = a0;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_repeat_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;
	if ( (r >= r0) && (r <= r1) )
	  {
	    int  ll = ss * (M_PI +  atan2(yy, xx));
	    int  l = ll >> 16;

	    l += (ll - (l << 16)) >> 15;
	    if (l < src_len)
	      {
		l += off;
		l = l % src_len;
		if (l < 0)
		   l += src_len;
		*dst = src[l];
	      }
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_repeat_aa_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                    int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   int     rr0 = r0 << 16, rr1 = r1 << 16;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;
	if ( (r >= r0) && (r <= r1) )
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = ll >> 16, lp;

	    if (l < src_len)
	      {
		DATA32 a = 1 + ((ll - (l << 16)) >> 8), a0 = a;

		lp = l + off;
		lp = lp % src_len;
		if (lp < 0)
		   lp += src_len;
		*dst = src[lp];
		if (lp + 1 < src_len)
		   *dst = INTERP_256(a, src[lp + 1], *dst);
		if (lp == (src_len - 1))
		   *dst = INTERP_256(a, src[0], *dst);
		if (l == (src_len - 1))
		   *dst = MUL_256(257 - a0, *dst);
		if (l == 0)
		   *dst = MUL_256(a0, *dst);
		if (r == r0)
	          {
		    a = 1 + ((rr - rr0) >> 8);
		    *dst = MUL_256(a, *dst);
	          }
		if (r == r1)
	          {
		    a = 256 - ((rr - rr1) >> 8);
		    *dst = MUL_256(a, *dst);
	          }
	      }
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_repeat_masked_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                        int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;  *mask = 0;
	if ( (r >= r0) && (r <= r1) )
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = ll >> 16;

	    l += (ll - (l << 16)) >> 15;
	    if (l < src_len)
	      {
		l += off;
		l = l % src_len;
		if (l < 0)
		   l += src_len;
		*dst = src[l];  *mask = 255;
	      }
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_repeat_aa_masked_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   int     rr0 = r0 << 16, rr1 = r1 << 16;
   float   off = gdata->off * (src_len - 1);

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;  *mask = 0;
	if ( (r >= r0) && (r <= r1) )
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = ll >> 16, lp;

	    if (l < src_len)
	      {
		DATA32 a = 1 + ((ll - (l << 16)) >> 8), a0 = a - 1;

		lp = l + off;
		lp = lp % src_len;
		if (lp < 0)
		   lp += src_len;
		*dst = src[lp];  *mask = 255;
		if (lp + 1 < src_len)
		   *dst = INTERP_256(a, src[lp + 1], *dst);
		if (lp == (src_len - 1))
		   *dst = INTERP_256(a, src[0], *dst);
		if (l == (src_len - 1))
		   *mask = 255 - a0;
		if (l == 0)
		   *mask = a0;
		if (r == r0)
		   *mask = ((rr - rr0) >> 8);
		if (r == r1)
		   *mask = 255 - ((rr - rr1) >> 8);
	      }
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_pad(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
            int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  ll = ss * (M_PI +  atan2(yy, xx));
	int  l = ll >> 16;

	l += (ll - (l << 16)) >> 15;
	if (l >= src_len)
	    l = src_len - 1;
	*dst++ = src[l];  xx += axx;  yy += ayx;
     }
}

static void
angular_pad_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  ll = ss * (M_PI +  atan2(yy, xx));
	int  l = ll >> 16;
	DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	if (l + 1 < src_len)
	   *dst = INTERP_256(a, src[l + 1], src[l]);
	else
	   *dst = src[src_len - 1];
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_pad_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                    int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;
	if ( (r >= r0) && (r <= r1) )
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = ll >> 16;

	    l += (ll - (l << 16)) >> 15;
	    if (l >= src_len)
		l = src_len - 1;
	    *dst = src[l];
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_pad_aa_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   int     rr0 = r0 << 16, rr1 = r1 << 16;

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;
	if ( (r >= r0) && (r <= r1) )
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = ll >> 16;
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	    if (l + 1 < src_len)
		*dst = INTERP_256(a, src[l + 1], src[l]);
	    else
		*dst = src[src_len - 1];
	    if (r == r0)
	      {
		a = 1 + ((rr - rr0) >> 8);
		*dst = MUL_256(a, *dst);
	      }
	    if (r == r1)
	      {
		a = 256 - ((rr - rr1) >> 8);
		*dst = MUL_256(a, *dst);
	      }
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_pad_masked_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int  rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;  *mask = 0;
	if ( (r >= r0) && (r <= r1) )
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = ll >> 16;

	    l += (ll - (l << 16)) >> 15;
	    if (l >= src_len)
		l = src_len - 1;
	    *dst = src[l];  *mask = 255;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_pad_aa_masked_annulus(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   int     rr0 = r0 << 16, rr1 = r1 << 16;

   SETUP_ANGULAR_FILL

   while (dst < dst_end)
     {
	int rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;  *mask = 0;
	if ( (r >= r0) && (r <= r1) )
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = ll >> 16;
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	    if (l + 1 < src_len)
		*dst = INTERP_256(a, src[l + 1], src[l]);
	    else
		*dst = src[src_len - 1];

	    *mask = 255;
	    if (r == r0)
		*mask = ((rr - rr0) >> 8);
	    if (r == r1)
		*mask = 255 - ((rr - rr1) >> 8);
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}
