#include "draw_private.h"

/** default op: d = d*(1-sa) + s */
static void
_alpha_func_blend(uint8_t *dst, const uint8_t *src, int len)
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
_alpha_func_copy(uint8_t *dst, const uint8_t *src, int len)
{
   EINA_SAFETY_ON_NULL_RETURN(src);
   EINA_SAFETY_ON_NULL_RETURN(dst);

   memcpy(dst, src, len);
}

/** d = d*s */
static void
_alpha_func_mul(uint8_t *dst, const uint8_t *src, int len)
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

Draw_Func_Alpha
efl_draw_alpha_func_get(Efl_Gfx_Render_Op op, Eina_Bool has_mask)
{
   if (!has_mask)
     {
        if (op == EFL_GFX_RENDER_OP_BLEND)
          return _alpha_func_blend;
        else
          return _alpha_func_copy;
     }
   else
     return _alpha_func_mul;
}
