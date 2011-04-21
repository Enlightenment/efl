/*
 * Filter implementation for evas
 */
#include <stddef.h>     // offsetof

#include "evas_common.h"
#include "evas_private.h"

#include <assert.h>
/* disable neon - even after fixes:
 * Error: ARM register expected -- vdup.u32 q14,$0xff000000'
 * not going to fix now
#ifdef BUILD_NEON
# define BUILD_NEON0 1
#else
# define BUILD_NEON0 0
#endif
*/

#define BUILD_NEON0 0

typedef struct Evas_Filter_Info_Blur
{
   double quality;
   int radius;
} Evas_Filter_Info_Blur;

typedef struct Evas_Filter_Info_GreyScale
{
   double r,g,b;
} Evas_Filter_Info_GreyScale;

typedef struct Evas_Filter_Info_Brightness
{
   double adjust;
} Evas_Filter_Info_Brightness;

typedef struct Evas_Filter_Info_Contrast
{
   double adjust;
} Evas_Filter_Info_Contrast;

typedef int (*Filter_Size_FN)(Evas_Filter_Info *,int,int,int*,int*,Eina_Bool);
typedef uint8_t *(*Key_FN)(const Evas_Filter_Info  *, uint32_t *);

struct fieldinfo
{
   const char *field;
   int type;
   size_t offset;
};

struct filterinfo
{
   Evas_Software_Filter_Fn filter;
   const size_t datasize;
   Filter_Size_FN sizefn;
   Key_FN keyfn;
   Eina_Bool alwaysalpha;
};

enum
{
   TYPE_INT,
   TYPE_FLOAT
};

static int blur_size_get(Evas_Filter_Info*, int, int, int *, int *, Eina_Bool);
static uint8_t *gaussian_key_get(const Evas_Filter_Info *, uint32_t *);

static Eina_Bool gaussian_filter(Evas_Filter_Info *, RGBA_Image*, RGBA_Image*);
static Eina_Bool negation_filter(Evas_Filter_Info *, RGBA_Image*, RGBA_Image*);
static Eina_Bool sepia_filter(Evas_Filter_Info *, RGBA_Image*, RGBA_Image*);
static Eina_Bool greyscale_filter(Evas_Filter_Info*, RGBA_Image*, RGBA_Image*);
static Eina_Bool brightness_filter(Evas_Filter_Info*, RGBA_Image*, RGBA_Image*);
static Eina_Bool contrast_filter(Evas_Filter_Info *, RGBA_Image*, RGBA_Image*);

struct filterinfo filterinfo[] =
{
   /* None */
   { NULL, 0, NULL, NULL, EINA_FALSE},
   /* Blur */
   { gaussian_filter, sizeof(Evas_Filter_Info_Blur), blur_size_get, gaussian_key_get, EINA_TRUE },
   /* Negation */
   { negation_filter, 0, NULL, NULL, EINA_FALSE },
   /* Sepia */
   { sepia_filter, 0, NULL, NULL, EINA_FALSE },
   /* Greyscale */
   { greyscale_filter, sizeof(Evas_Filter_Info_GreyScale), NULL, NULL, EINA_FALSE },
   /* Brightness */
   { brightness_filter, sizeof(Evas_Filter_Info_Brightness), NULL, NULL, EINA_FALSE },
   /* Contrast */
   { contrast_filter, sizeof(Evas_Filter_Info_Contrast), NULL, NULL, EINA_FALSE},
};


static struct fieldinfo blurfields[] =
{
   { "quality",  TYPE_FLOAT, offsetof(Evas_Filter_Info_Blur, quality) },
   { "radius",  TYPE_INT, offsetof(Evas_Filter_Info_Blur, radius) },
   { NULL, 0, 0 },
};

static struct fieldinfo greyfields[] =
{
   { "red",  TYPE_FLOAT, offsetof(Evas_Filter_Info_GreyScale, r) },
   { "green",  TYPE_FLOAT, offsetof(Evas_Filter_Info_GreyScale, g) },
   { "blue",  TYPE_FLOAT, offsetof(Evas_Filter_Info_GreyScale, b) },

   { "all",  TYPE_FLOAT, offsetof(Evas_Filter_Info_GreyScale, r) },
   { "all",  TYPE_FLOAT, offsetof(Evas_Filter_Info_GreyScale, g) },
   { "all",  TYPE_FLOAT, offsetof(Evas_Filter_Info_GreyScale, b) },
   { NULL, 0, 0 },
};

static struct fieldinfo brightnessfields[] =
{
   { "adjust",  TYPE_FLOAT, offsetof(Evas_Filter_Info_Brightness, adjust) },
   { NULL, 0, 0 },
};

static struct fieldinfo contrastfields[] =
{
   { "adjust",  TYPE_FLOAT, offsetof(Evas_Filter_Info_Contrast, adjust) },
   { NULL, 0, 0 },
};

static struct fieldinfo *filterfields[] =
{
   NULL,
   blurfields,
   NULL,
   NULL,
   greyfields,
   brightnessfields,
   contrastfields,
};

static Evas_Filter_Info *filter_alloc(Evas_Object *o);

EAPI Eina_Bool
evas_object_filter_mode_set(Evas_Object *o, Evas_Filter_Mode mode)
{
   Evas_Filter_Info *info;

   MAGIC_CHECK(o, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();

   if ((mode != EVAS_FILTER_MODE_OBJECT) && (mode != EVAS_FILTER_MODE_BELOW))
      return EINA_FALSE;

   if (!o->filter)
     {
        filter_alloc(o);
     }
   if (!o->filter) return EINA_FALSE;
   info = o->filter;

   if (info->mode == mode) return EINA_TRUE; /* easy case */
   info->mode = mode;
   info->dirty = 1;
   return EINA_TRUE;
}

EAPI Evas_Filter_Mode
evas_object_filter_mode_get(Evas_Object *o)
{
   MAGIC_CHECK(o, Evas_Object, MAGIC_OBJ);
   return EVAS_FILTER_MODE_OBJECT;
   MAGIC_CHECK_END();

   if (!o->filter) return EVAS_FILTER_MODE_OBJECT;
   return o->filter->mode;
}

EAPI Eina_Bool
evas_object_filter_set(Evas_Object *o, Evas_Filter filter)
{
   Evas_Filter_Info *info;
   struct filterinfo *finfo;

   MAGIC_CHECK(o, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();

   /* force filter to be signed: else gcc complains, but enums may always be
    * signed */
   if (((int)filter < (int)EVAS_FILTER_NONE) || (filter > EVAS_FILTER_LAST))
      return EINA_FALSE;

   /* Don't alloc on no-op */
   if (!o-filter && filter == EVAS_FILTER_NONE) return EINA_TRUE;

   if (!o->filter) filter_alloc(o);
   if (!o->filter) return EINA_FALSE;

   info = o->filter;

   if (info->filter == filter) return EINA_TRUE;

   finfo = filterinfo + filter;
   info->filter = filter;
   info->dirty = 1;
   if (info->data)
     {
        if (info->data_free)
           info->data_free(info->data);
        else
           free(info->data);
     }
   info->datalen = finfo->datasize;
   if (finfo->datasize)
     {
        info->data = calloc(1, finfo->datasize);
        if (!info->data)
          {
             o->filter = EVAS_FILTER_NONE;
             return EINA_FALSE;
          }
     }
   else
      info->data = NULL;
   info->data_free = NULL;

   return EINA_TRUE;
}

EAPI Evas_Filter
evas_object_filter_get(Evas_Object *o)
{
   MAGIC_CHECK(o, Evas_Object, MAGIC_OBJ);
   return EVAS_FILTER_NONE;
   MAGIC_CHECK_END();

   if (!o->filter) return EVAS_FILTER_NONE;
   return o->filter->filter;
}

EAPI Eina_Bool
evas_object_filter_param_int_set(Evas_Object *o, const char *param, int val)
{
   char *data;
   const struct fieldinfo *fields;
   Eina_Bool found;
   int i;

   MAGIC_CHECK(o, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();

   if ((!o->filter) || (!o->filter->data)) return EINA_FALSE;

   fields = filterfields[o->filter->filter];
   data = o->filter->data;
   found = EINA_FALSE;
   for (i = 0; fields[i].field; i++)
     {
        if (!strcmp(fields[i].field, param))
          {
             if (fields[i].type != TYPE_INT) continue;
             *(int *)(data + fields[i].offset) = val;
             o->filter->dirty = 1;
             evas_object_change(o);
             found = EINA_TRUE;
          }
     }
   return found;
}

EAPI int
evas_object_filter_param_int_get(Evas_Object *o, const char *param)
{
   char *data;
   const struct fieldinfo *fields;
   int val;
   int i;

   MAGIC_CHECK(o, Evas_Object, MAGIC_OBJ);
   return -1;
   MAGIC_CHECK_END();

   if ((!o->filter) || (!o->filter->data)) return -1;

   fields = blurfields;
   data = o->filter->data;

   for (i = 0; fields[i].field; i++)
     {
        if (!strcmp(fields[i].field, param))
          {
             if (fields[i].type != TYPE_INT) continue;
             val = *(int *)(data + fields[i].offset);
             return val;
          }
     }
   return -1;
}

EAPI Eina_Bool
evas_object_filter_param_str_set(Evas_Object *o __UNUSED__,
                                 const char *param __UNUSED__,
                                 const char *val __UNUSED__)
{
   return EINA_FALSE;
}

EAPI const char *
evas_object_filter_param_str_get(Evas_Object *o __UNUSED__,
                                 const char *param __UNUSED__)
{
   return NULL;
}

EAPI Eina_Bool
evas_object_filter_param_obj_set(Evas_Object *o __UNUSED__,
                                 const char *param __UNUSED__,
                                 Evas_Object *val __UNUSED__)
{
   return EINA_FALSE;
}

EAPI Evas_Object *
evas_object_filter_param_obj_get(Evas_Object *o __UNUSED__,
                                 const char *param __UNUSED__)
{
   return NULL;
}

EAPI Eina_Bool
evas_object_filter_param_float_set(Evas_Object *o, const char *param,
                                   double val)
{
   char *data;
   const struct fieldinfo *fields;
   int i;
   Eina_Bool rv;

   MAGIC_CHECK(o, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();

   if ((!o->filter) || (!o->filter->data)) return EINA_FALSE;

   rv = EINA_FALSE;
   fields = blurfields;
   data = o->filter->data;

   for (i = 0; fields[i].field; i++)
     {
        if (!strcmp(fields[i].field, param))
          {
             if (fields[i].type != TYPE_FLOAT) continue;
             *(double *)(data + fields[i].offset) = val;
             o->filter->dirty = 1;
             o->changed = 1;
             evas_object_change(o);
             rv = EINA_TRUE;
          }
     }
   return rv;
}

EAPI double
evas_object_filter_param_float_get(Evas_Object *o, const char *param)
{
   char *data;
   const struct fieldinfo *fields;
   double val;
   int i;

   MAGIC_CHECK(o, Evas_Object, MAGIC_OBJ);
   return -1;
   MAGIC_CHECK_END();

   if ((!o->filter) || (!o->filter->data)) return -1;

   fields = blurfields;
   data = o->filter->data;

   for (i = 0; fields[i].field; i++)
     {
        if (!strcmp(fields[i].field, param))
          {
             if (fields[i].type != TYPE_FLOAT) continue;
             val = *(double *)(data + fields[i].offset);
             return val;
          }
     }
   return -1;
}





/*
 * Internal call
 */
int
evas_filter_get_size(Evas_Filter_Info *info, int inw, int inh,
                     int *outw, int *outh, Eina_Bool inv)
{
   if (!info) return -1;
   if ((!outw) && (!outh)) return 0;

   if (filterinfo[info->filter].sizefn)
      return filterinfo[info->filter].sizefn(info, inw, inh, outw, outh, inv);

   if (outw) *outw = inw;
   if (outh) *outh = inh;
   return 0;
}

Eina_Bool
evas_filter_always_alpha(Evas_Filter_Info *info)
{
   if (!info) return EINA_FALSE;
   return filterinfo[info->filter].alwaysalpha;
}

/*
 * Another internal call:
 *    Given a filterinfo, generate a unique key for it
 *
 * For simple filters, it's just the filter type.
 * for more complex filters, it's the type, with it's params.
 *
 * Note management of the key data is up to the caller, that is it should
 * probably be freed after use.
 *
 * Note the automatic fallback generation places the single byte at the end so
 * the memcpy will be aligned.  Micro-optimisations FTW!
 *
 * @param info Filter info to generate from
 * @param len Length of the buffer returned.
 * @return key Key buffer
 */
uint8_t *
evas_filter_key_get(const Evas_Filter_Info *info, uint32_t *lenp)
{
   struct filterinfo *finfo;
   uint8_t *key;
   int len;

   if (!info) return NULL;

   finfo = filterinfo + info->filter;
   if (finfo->keyfn) return finfo->keyfn(info, lenp);

   len = 1 + finfo->datasize;
   key = malloc(len);
   if (!key) return NULL;
   if (finfo->datasize) memcpy(key, info->data, finfo->datasize);
   key[finfo->datasize] = info->filter;
   if (lenp) *lenp = len;
   return key;
}

Evas_Software_Filter_Fn
evas_filter_software_get(Evas_Filter_Info *info)
{
   return filterinfo[info->filter].filter;
}

void
evas_filter_free(Evas_Object *o)
{
   if (!o->filter) return;
   if (o->filter->key) free(o->filter->key);
   free(o->filter);
   o->filter = NULL;
}




/*
 * Private calls
 */
static Evas_Filter_Info *
filter_alloc(Evas_Object *o)
{
   Evas_Filter_Info *info;

   if (!o) return NULL;
   info = calloc(1,sizeof(struct Evas_Filter_Info));
   if (!info) return NULL;
   info->dirty = 1;
   info->filter = EVAS_FILTER_NONE;
   info->mode = EVAS_FILTER_MODE_OBJECT;
   info->datalen = 0;

   o->filter = info;

   return info;
}

static int
blur_size_get(Evas_Filter_Info *info, int inw, int inh, int *outw, int *outh,
              Eina_Bool inv)
{
   Evas_Filter_Info_Blur *blur = info->data;

   if (inv)
     {
        if (outw) *outw = MAX(inw - (blur->radius * 2), 0);
        if (outh) *outh = MAX(inh - (blur->radius * 2), 0);
     }
   else
     {
        if (outw) *outw = inw + (blur->radius * 2);
        if (outh) *outh = inh + (blur->radius * 2);
     }
   return 0;
}

/*
 * Generate a key for the Gaussian generator.
 *
 * The size is:
 *    - 1 byte for the type (blur)
 *    - 1 byte for the quality (0-1 -> 0-255)
 *    - 2 bytes for radius (max is 508 anyway)
 *
 * @param info Filter info
 * @param len Length of the returned buffer
 * @return new buffer
 */
static uint8_t *
gaussian_key_get(const Evas_Filter_Info *info, uint32_t *lenp)
{
   struct Evas_Filter_Info_Blur *blur;
   uint8_t *key;

   if ((!info) || (!info->data)) return NULL;
   blur = info->data;

   if (lenp) *lenp = 4;
   key = malloc(4);
   if (!key) return NULL;
   key[0] = EVAS_FILTER_BLUR;
   key[1] = blur->quality * 255;
   key[2] = blur->radius >> 8;
   key[3] = blur->radius;

   return key;
}



















/**
 * Software implementations
 */
#define all(OP, A, R, G, B, W, I) \
   do { \
      A OP A_VAL(I) * W; \
      R OP R_VAL(I) * W; \
      G OP G_VAL(I) * W; \
      B OP B_VAL(I) * W; \
   } while (0)
#define wavg(x,n)        (((x) / (n)) & 0xff)
#define wavgd(x,n)       ((uint32_t)((x) / (n)) & 0xff)

typedef int (*FilterH)(int, uint32_t *, int, uint32_t *);
typedef int (*FilterV)(int, uint32_t *, int, int, uint32_t *);

static int gaussian_filter_h(int rad, uint32_t *in, int w, uint32_t *out);
static int gaussian_filter_h64(int rad, uint32_t *in, int w, uint32_t *out);
static int gaussian_filter_hd(int rad, uint32_t *in, int w, uint32_t *out);
static int gaussian_filter_v(int rad, uint32_t *in, int h, int skip, uint32_t *out);
static int gaussian_filter_v64(int rad, uint32_t *in, int h, int skip, uint32_t *out);
static int gaussian_filter_vd(int rad, uint32_t *in, int h, int skip, uint32_t *out);
static const uint32_t *gaussian_row_get(int row, int *npoints, uint32_t *weight);
static const uint64_t *gaussian_row_get64(int row, int *npoints, uint64_t *weight);
static const double *gaussian_row_getd(int row, int *npoints, double *weight);

static Eina_Bool
gaussian_filter(Evas_Filter_Info *filter, RGBA_Image *src, RGBA_Image *dst)
{
   int i;
   uint32_t nw, nh;
   uint32_t *in, *tmp, *out;
   FilterV filter_v = gaussian_filter_v;
   FilterH filter_h = gaussian_filter_h;
   Evas_Filter_Info_Blur *blur;
   int w, h;

   blur = filter->data;

   /* Use 64 bit version if we are going to overflow */
   if (blur->radius > 508) /** too big for doubles: Bail out */
      return EINA_FALSE;
   else if (blur->radius > 28)
     {
        filter_v = gaussian_filter_vd;
        filter_h = gaussian_filter_hd;
     } 
   else if (blur->radius > 12)
     {
        filter_v = gaussian_filter_v64;
        filter_h = gaussian_filter_h64;
     }
   
   w = src->cache_entry.w;
   h = src->cache_entry.h;
   in = src->image.data;
   
   if (!in) return EINA_FALSE;
   
   nw = w + 2 * blur->radius;
   nh = h + 2 * blur->radius;
   
   out = dst->image.data;
   if (!out) return EINA_FALSE;
   tmp = malloc(nw * h * sizeof(uint32_t));
   
   for (i = 0; i < h; i++)
      filter_h(blur->radius,in + (i * w), w, tmp + (i * nw));
   
   for (i = 0; i < (int)nw; i++)
      filter_v(blur->radius,tmp + i, h, nw, out + i);

   free(tmp);
   return EINA_TRUE;
}

/* Blur only horizontally */
static int
gaussian_filter_h(int rad, uint32_t *in, int w, uint32_t *out)
{
   const uint32_t *points;
   int npoints;
   uint32_t weight;
   int i, k;
   uint32_t r, g, b, a;
   
   /* Get twice the radius: even rows have 1 element */
   points = gaussian_row_get(rad * 2, &npoints, &weight);
   for (i = -rad; i < (w + rad); i++)
     {
        r = g = b = a = 0;
        for (k = -rad; k <= rad; k++)
          {
             if ((k + i) < 0) continue;
             if ((k + i) >= w) continue;
             all(+=, a, r, g, b, points[k + rad], in + k + i);
          }
        *(out) = ARGB_JOIN(wavg(a, weight),
                           wavg(r, weight),
                           wavg(g, weight),
                           wavg(b, weight));
        out++;
     }
   return 0;
}

/* Blur only horizontally */
static int
gaussian_filter_hd(int rad, uint32_t *in, int w, uint32_t *out)
{
   const double *points;
   int npoints;
   double weight;
   int i, k;
   double r, g, b, a;

   /* Get twice the radius: even rows have 1 element */
   points = gaussian_row_getd(rad * 2, &npoints, &weight);
   for (i = -rad; i < (w + rad); i++)
     {
        r = g = b = a = 0;
        for (k = -rad; k <= rad; k++)
          {
             if ((k + i) < 0) continue;
             if ((k + i) >= w) continue;
             all(+=, a, r, g, b, points[k + rad], in + k + i);
          }
        *(out) = ARGB_JOIN(wavgd(a, weight),
                           wavgd(r, weight),
                           wavgd(g, weight),
                           wavgd(b, weight));
        out++;
   }
   return 0;
}


/* Blur only horizontally */
static int
gaussian_filter_h64(int rad, uint32_t *in, int w, uint32_t *out)
{
   const uint64_t *points;
   int npoints;
   uint64_t weight;
   int i, k;
   uint64_t r, g, b, a;

   /* Get twice the radius: even rows have 1 element */
   points = gaussian_row_get64(rad * 2, &npoints, &weight);
   for (i = -rad ; i < w + rad; i ++){
      r = g = b = a = 0;
      for (k = -rad ; k <= rad ; k ++){
         if ((k + i) < 0) continue;
         if ((k + i) >= w) continue;
         all(+=, a, r, g, b, points[k + rad], in + k + i);
      }
      *(out) = ARGB_JOIN(wavg(a, weight),
                         wavg(r, weight),
                         wavg(g, weight),
                         wavg(b, weight));
      out++;
   }
   return 0;
}

static int
gaussian_filter_v(int rad, uint32_t *in, int h, int skip, uint32_t *out)
{
   const uint32_t *points;
   int npoints;
   uint32_t weight;
   int i, k;
   uint32_t r, g, b, a;

   /* Get twice the radius: even rows have 1 element */
   points = gaussian_row_get(rad * 2, &npoints, &weight);
   weight = 0;
   for (i = 0; i < npoints; i++) weight += points[i];

   for (i = -rad; i < (h + rad); i++)
     {
        r = g = b = a = 0;
        for (k = -rad; k <= rad; k++)
          {
             if ((k + i) < 0) continue;
             if ((k + i) >= h) continue;
             all(+=, a, r, g, b, points[k + rad], in + (skip * (k + i)));
          }
        *(out) = ARGB_JOIN(wavg(a, weight),
                           wavg(r, weight),
                           wavg(g, weight),
                           wavg(b, weight));
        out += skip;
     }
   return 0;
}

static int
gaussian_filter_v64(int rad, uint32_t *in, int h, int skip, uint32_t *out)
{
   const uint64_t *points;
   int npoints;
   uint64_t weight;
   int i, k;
   uint64_t r, g, b, a;

   /* Get twice the radius: even rows have 1 element */
   points = gaussian_row_get64(rad * 2, &npoints, &weight);
   weight = 0;
   for (i = 0; i < npoints; i++) weight += points[i];

   for (i = -rad; i < (h + rad); i++)
     {
        r = g = b = a = 0;
        for (k = -rad ; k <= rad ; k++)
          {
             if ((k + i) < 0) continue;
             if ((k + i) >= h) continue;
             all(+=, a, r, g, b, points[k + rad], in + (skip * (k + i)));
          }
        *(out) = ARGB_JOIN(wavg(a, weight),
                           wavg(r, weight),
                           wavg(g, weight),
                           wavg(b, weight));
        out += skip;
     }
   return 0;
}

static int
gaussian_filter_vd(int rad, uint32_t *in, int h, int skip, uint32_t *out)
{
   const double *points;
   int npoints;
   double weight;
   int i, k;
   double r, g, b, a;

   /* Get twice the radius: even rows have 1 element */
   points = gaussian_row_getd(rad * 2, &npoints, &weight);
   weight = 0;
   for (i = 0 ; i < npoints ; i ++) weight += points[i];

   for (i = -rad ; i < h + rad; i ++)
     {
        r = g = b = a = 0;
        for (k = -rad ; k <= rad ; k ++)
          {
             if ((k + i) < 0) continue;
             if ((k + i) >= h) continue;
             all(+=, a, r, g, b, points[k + rad], in + (skip * (k + i)));
          }
        *(out) = ARGB_JOIN(wavgd(a, weight),
                           wavgd(r, weight),
                           wavgd(g, weight),
                           wavgd(b, weight));
        out += skip;
     }
   return 0;
}

static const uint32_t *
gaussian_row_get(int row, int *npoints, uint32_t *weight)
{
   static uint32_t *points = NULL;
   static int last = -1;
   static uint32_t lastweight = -1;
   int c, k;

   if (row < 0) return NULL;

   if (npoints) *npoints = row + 1;

   if (last == row)
     {
        if (weight) *weight = lastweight;
        return points;
     }
   if (points) free(points);

   points = malloc((row + 1) * sizeof(uint32_t));
   if (!points)
     {
        last = -1;
        return NULL;
     }
   last = row;

   c = 1;
   for (k = 0; k <= row; k++)
     {
        points[k] = c;
        c = c * (row - k) / (k + 1);
     }

   for (k = 0, lastweight = 0; k <= row; k++) lastweight += points[k];
   if (weight) *weight = lastweight;
   return points;
}

static const uint64_t *
gaussian_row_get64(int row, int *npoints, uint64_t *weight)
{
   static uint64_t *points = NULL;
   static int last = -1;
   static uint64_t lastweight = -1;
   uint64_t c;
   int k;

   if (row < 0) return NULL;

   if (npoints) *npoints = row + 1;
   if (last == row)
     {
        if (weight) *weight = lastweight;
        return points;
     }
   if (points) free(points);
   
   points = malloc((row + 1) * sizeof(uint64_t));
   if (!points)
     {
        last = -1;
        return NULL;
     }
   last = row;
   
   c = 1;
   for (k = 0; k <= row; k++)
     {
        points[k] = c;
        c = c * (row - k) / (k + 1);
     }
   
   for (k = 0, lastweight = 0; k <= row; k ++) lastweight += points[k];
   if (weight) *weight = lastweight;
   
   return points;
}

static const double *
gaussian_row_getd(int row, int *npoints, double *weight)
{
   static double *points = NULL;
   static int last = -1;
   static double lastweight = -1;
   double c;
   int k;
   
   if (row < 0) return NULL;
   
   if (last == row)
     {
        if (weight) *weight = lastweight;
        return points;
     }
   
   if (points) free(points);
   points = malloc((row + 1) * sizeof(double));
   if (!points)
     {
        last = -1;
        return NULL;
     }
   last = row;
   
   if (npoints) *npoints = row + 1;
   
   c = 1;
   for (k = 0; k <= row; k++)
     {
        points[k] = c;
        c = c * (row - k) / (k + 1);
     }
   
   for (k = 0, lastweight = 0; k <= row; k++) lastweight += points[k];
   if (weight) *weight = lastweight;
   
   return points;
}

#if BUILD_NEON0
static Eina_Bool
negation_filter_neon(Evas_Filter_Info *info, RGBA_Image *src, RGBA_Image *dst)
{
   uint32_t tmp;

   if (src->cache_entry.flags.alpha)
     {
        // FIXME: not implemented
     }
   else
     {
        /* No alpha */
#define AP  "NEG_FILTER_NA"
        asm volatile (

           ".fpu neon                                       \n\t"
           "vdup.u32    q14,   $0xff000000                   \n\t"
           "vmvn.u32    q15,   q1                            \n\t"

           // fixme: do check for small loops
           AP"loopinit:                                     \n\t"
               "sub     %[tmp], %[e], #31                   \n\t"

           AP"loop:                                         \n\t"
               "vldm    %[s]!,    {d0,d1,d2,d3}             \n\t"
               "vand    q2,   q0, q15                       \n\t"
               "vand    q3,   q1, q15                       \n\t"
               "vand    q4,   q0, q14                       \n\t"
               "vand    q5,   q1, q14                       \n\t"
                        // fixme: can i do this with xor
               "cmp     %[tmp], %[s]                        \n\t"

               "vmvn    q6,   q2                            \n\t"
               "vmvn    q7,   q3                            \n\t"

               "vorr     q0,   q6,q4                         \n\t"
               "vorr     q1,   q7,q5                         \n\t"

               "vstm    %[d]!,  {d0,d1,d2,d3}               \n\t"

               "bhi     "AP"loop                            \n\t"

            : // no out
            : // input
               [e] "r" (src->image.data+ src->cache_entry.w*src->cache_entry.h),
               [s] "r" (src->image.data),
               [tmp] "r" (tmp),
               [d] "r" (dst->image.data)
            : "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q14", "q15",
                  "memory"
         );
#undef AP
     }
   return EINA_TRUE;
}
#endif

static Eina_Bool
negation_filter(Evas_Filter_Info *info, RGBA_Image *src, RGBA_Image *dst)
{
   uint32_t *in, *out;
   int i,j;
   int w,h;
   uint32_t mask,a;

#if BUILD_NEON0
   if (evas_common_cpu_has_feature(CPU_FEATURE_NEON) &&
      (!src->cache_entry.flags.alpha))
      return negation_filter_neon(info, src, dst);
#endif
   
   in = src->image.data;
   out = dst->image.data;
   w = src->cache_entry.w;
   h = src->cache_entry.h;

   if (src->cache_entry.flags.alpha)
     {
        for (i = 0; i < h; i++)
          {
             for (j = 0; j < w; j++)
               {
                  a = (*in >> 24) & 0xff;
                  mask = a | (a << 8) | (a << 16);
                  // FIXME: use *out = ARGB_JOIN(a, r, g, b);
                  *out = (mask - (*in & 0xffffff)) | (a << 24);
                  out++;
                  in++;
               }
          }
     }
   else
     {
        for (i = 0; i < h; i++)
          {
             for (j = 0; j < w; j++)
               {
                  // FIXME: use *out = ARGB_JOIN(a, r, g, b);
                  *out = ~(*in & ~0xff000000) | ((*in) & 0xff000000);
                  out++;
                  in++;
               }
          }
     }
   return EINA_TRUE;
   info = NULL;
}

static Eina_Bool
sepia_filter(Evas_Filter_Info *info __UNUSED__, RGBA_Image *src, RGBA_Image *dst)
{
   uint32_t *in, *out;
   int i, j;
   int w, h;
   uint32_t r, g, b, nr, ng, nb;

   in = src->image.data;
   out = dst->image.data;
   w = src->cache_entry.w;
   h = src->cache_entry.h;

   for (i = 0; i < h; i++)
     {
        for (j = 0; j < w; j++)
          {
             r = R_VAL(in);
             g = G_VAL(in);
             b = B_VAL(in);
             nr = ((uint32_t)((r * 0.393) + (g * 0.769) + (b * 0.189)));
             ng = ((uint32_t)((r * 0.349) + (g * 0.686) + (b * 0.168)));
             nb = ((uint32_t)((r * 0.272) + (g * 0.534) + (b * 0.131)));
             if (nr > 255) nr = 255;
             if (ng > 255) ng = 255;
             if (nb > 255) nb = 255;
             // FIXME: use *out = ARGB_JOIN(a, r, g, b);
             *out = (*in & 0xff000000) | (nr << 16) | (ng << 8) | nb;
             out++;
             in++;
          }
     }

   return EINA_TRUE;

}

static Eina_Bool
greyscale_filter(Evas_Filter_Info *info __UNUSED__, RGBA_Image *src, RGBA_Image *dst)
{
   uint32_t *in, *out;
   int i, j;
   int w, h;
   uint32_t cur;
   uint32_t a, r, g, b;

   in = src->image.data;
   out = dst->image.data;
   w = src->cache_entry.w;
   h = src->cache_entry.h;

   if (src->cache_entry.flags.alpha)
     {
        for (i = 0; i < h; i++)
          {
             for (j = 0; j < w; j++)
               {
                  a = A_VAL(in);
                  r = R_VAL(in);
                  g = G_VAL(in);
                  b = B_VAL(in);
                  cur = (r * 0.3) + (g * 0.59) + (b * 0.11);
                  cur |= cur << 16;
                  // FIXME: use *out = ARGB_JOIN(a, r, g, b);
                  *out = (a << 24) | cur | (cur << 8);
                  out++;
                  in++;
               }
          }
     }
   else
     {
        for (i = 0 ; i < h ; i ++)
          {
             for (j = 0; j < w ; j ++)
               {
                  r = R_VAL(in);
                  g = G_VAL(in);
                  b = B_VAL(in);
                  cur = r * 0.3 + g * 0.59 + b * 0.11;
                  cur |= cur << 16;
                  // FIXME: use *out = ARGB_JOIN(a, r, g, b);
                  *out = 0xff000000 | cur | (cur << 8);
                  out++;
                  in++;
               }
          }
     }
   return EINA_TRUE;
}

static Eina_Bool
brightness_filter(Evas_Filter_Info *info __UNUSED__, RGBA_Image *src, RGBA_Image *dst)
{
   uint32_t *in, *out;
   int i, j;
   int w, h;

   in = src->image.data;
   out = dst->image.data;
   w = src->cache_entry.w;
   h = src->cache_entry.h;

   for (i = 0; i < h; i++)
     {
        for (j = 0; j < w; j++)
          {
             // FIXME: not even implemented
             out++;
             in++;
          }
     }


   return EINA_TRUE;

}

static Eina_Bool
contrast_filter(Evas_Filter_Info *info __UNUSED__, RGBA_Image *src, RGBA_Image *dst)
{
   uint32_t *in, *out;
   int i, j;
   int w, h;

   in = src->image.data;
   out = dst->image.data;
   w = src->cache_entry.w;
   h = src->cache_entry.h;

   for (i = 0; i < h; i++)
     {
        for (j = 0; j < w; j++)
          {
             // FIXME: not even implemented
             out++;
             in++;
          }
     }

   return EINA_TRUE;

}

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/
