#include "evas_common_private.h"
#include "evas_blend_private.h"

/** default op: d = d*(1-sa) + s */
static void
_alpha_func_blend(DATA8 *src, DATA8 *dst, int len)
{
   int k;

   EINA_SAFETY_ON_NULL_RETURN(src);
   EINA_SAFETY_ON_NULL_RETURN(dst);

   for (k = len; k; k--)
     {
        int val = (*dst * (255 - *src)) / 255 + *src;
        *dst++ = val;
        src++;
     }
}

/** d = s */
static void
_alpha_func_copy(DATA8 *src, DATA8 *dst, int len)
{
   EINA_SAFETY_ON_NULL_RETURN(src);
   EINA_SAFETY_ON_NULL_RETURN(dst);

   memcpy(dst, src, len);
}

/** d = d*s */
static void
_alpha_func_mul(DATA8 *src, DATA8 *dst, int len)
{
   int k;

   EINA_SAFETY_ON_NULL_RETURN(src);
   EINA_SAFETY_ON_NULL_RETURN(dst);

   for (k = len; k; k--)
     {
        int val = (*dst * *src) / 255;
        *dst++ = val;
        src++;
     }
}


#if 0
// Reference ops. In case of alpha, s == sa.
EVAS_RENDER_BLEND = 0, /**< default op: d = d*(1-sa) + s */
EVAS_RENDER_BLEND_REL = 1, /**< d = d*(1 - sa) + s*da */
EVAS_RENDER_COPY = 2, /**< d = s */
EVAS_RENDER_COPY_REL = 3, /**< d = s*da */
EVAS_RENDER_ADD = 4, /* d = d + s */
EVAS_RENDER_ADD_REL = 5, /**< d = d + s*da */
EVAS_RENDER_SUB = 6, /**< d = d - s */
EVAS_RENDER_SUB_REL = 7, /* d = d - s*da */
EVAS_RENDER_TINT = 8, /**< d = d*s + d*(1 - sa) + s*(1 - da) */
EVAS_RENDER_TINT_REL = 9, /**< d = d*(1 - sa + s) */
EVAS_RENDER_MASK = 10, /**< d = d*sa */
EVAS_RENDER_MUL = 11, /**< d = d*s */
#endif

Alpha_Gfx_Func
evas_common_alpha_func_get(int op)
{
   switch (op)
     {
      case EVAS_RENDER_BLEND:
        return _alpha_func_blend;
      case EVAS_RENDER_COPY:
        return _alpha_func_copy;
      case EVAS_RENDER_MASK:
      case EVAS_RENDER_MUL:
        return _alpha_func_mul;
      default:
        ERR("Not implemented yet.");
        return NULL;
     }
}
