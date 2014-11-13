#include "evas_common_private.h"
#include "evas_blend_private.h"

#ifdef BUILD_MMX
#include "evas_mmx.h"
#endif

const DATA32 ALPHA_255 = 255;
const DATA32 ALPHA_256 = 256;

static void
_composite_span_nothing(DATA32 *s EINA_UNUSED, DATA8 *m EINA_UNUSED, DATA32 c EINA_UNUSED, DATA32 *d EINA_UNUSED, int l EINA_UNUSED)
{
}

static void
_composite_pt_nothing(DATA32 s EINA_UNUSED, DATA8 m EINA_UNUSED, DATA32 c EINA_UNUSED, DATA32 *d EINA_UNUSED)
{
}

static RGBA_Gfx_Compositor  *
evas_gfx_compositor_get(unsigned char op)
{
   RGBA_Gfx_Compositor  *comp;

   switch (op)
     {
      case _EVAS_RENDER_BLEND:
        comp = evas_common_gfx_compositor_blend_get();
        break;
      case _EVAS_RENDER_BLEND_REL:
        comp = evas_common_gfx_compositor_blend_rel_get();
        break;
      case _EVAS_RENDER_COPY:
        comp = evas_common_gfx_compositor_copy_get();
        break;
      case _EVAS_RENDER_COPY_REL:
        comp = evas_common_gfx_compositor_copy_rel_get();
        break;
        /*
      case _EVAS_RENDER_ADD:
        comp = evas_common_gfx_compositor_add_get();
        break;
      case _EVAS_RENDER_ADD_REL:
        comp = evas_common_gfx_compositor_add_rel_get();
        break;
      case _EVAS_RENDER_SUB:
        comp = evas_common_gfx_compositor_sub_get();
        break;
      case _EVAS_RENDER_SUB_REL:
        comp = evas_common_gfx_compositor_sub_rel_get();
        break;
      */
      case _EVAS_RENDER_MASK:
        comp = evas_common_gfx_compositor_mask_get();
        break;
      case _EVAS_RENDER_MUL:
        comp = evas_common_gfx_compositor_mul_get();
        break;
      default:
        comp = evas_common_gfx_compositor_blend_get();
        break;
     }
   return comp;
}

EAPI void
evas_common_blend_init(void)
{
   static int gfx_initialised = 0;
   static int mmx = 0;
   static int sse = 0;
   static int sse2 = 0;
   RGBA_Gfx_Compositor  *comp;

   if (gfx_initialised) return;
   gfx_initialised = 1;

   evas_common_cpu_can_do(&mmx, &sse, &sse2);

   comp = evas_common_gfx_compositor_copy_get();
   if (comp) comp->init();
   comp = evas_common_gfx_compositor_copy_rel_get();
   if (comp) comp->init();

   comp = evas_common_gfx_compositor_blend_get();
   if (comp) comp->init();
   comp = evas_common_gfx_compositor_blend_rel_get();
   if (comp) comp->init();

   /*
   comp = evas_common_gfx_compositor_add_get();
   if (comp) comp->init();
   comp = evas_common_gfx_compositor_add_rel_get();
   if (comp) comp->init();
   comp = evas_common_gfx_compositor_sub_get();
   if (comp) comp->init();
   comp = evas_common_gfx_compositor_sub_rel_get();
   if (comp) comp->init();
*/
   comp = evas_common_gfx_compositor_mask_get();
   if (comp) comp->init();

   comp = evas_common_gfx_compositor_mul_get();
   if (comp) comp->init();
}

void
evas_common_blend_shutdown(void)
{
   RGBA_Gfx_Compositor  *comp;

   comp = evas_common_gfx_compositor_copy_get();
   if (comp) comp->shutdown();
   comp = evas_common_gfx_compositor_copy_rel_get();
   if (comp) comp->shutdown();

   comp = evas_common_gfx_compositor_blend_get();
   if (comp) comp->shutdown();
   comp = evas_common_gfx_compositor_blend_rel_get();
   if (comp) comp->shutdown();

   /*
   comp = evas_common_gfx_compositor_add_get();
   if (comp) comp->shutdown();
   comp = evas_common_gfx_compositor_add_rel_get();
   if (comp) comp->shutdown();
   comp = evas_common_gfx_compositor_sub_get();
   if (comp) comp->shutdown();
   comp = evas_common_gfx_compositor_sub_rel_get();
   if (comp) comp->shutdown();
   */

   comp = evas_common_gfx_compositor_mask_get();
   if (comp) comp->shutdown();

   comp = evas_common_gfx_compositor_mul_get();
   if (comp) comp->shutdown();
}


RGBA_Gfx_Func
evas_common_gfx_func_composite_pixel_span_get(Eina_Bool src_alpha, Eina_Bool src_sparse_alpha, Eina_Bool dst_alpha, int pixels, int op)
{
   RGBA_Gfx_Compositor  *comp;
   RGBA_Gfx_Func        func = NULL;

   if (!src_alpha)
     {
        if (op == _EVAS_RENDER_BLEND) op = _EVAS_RENDER_COPY;
        else if (op == _EVAS_RENDER_BLEND_REL) op = _EVAS_RENDER_COPY_REL;
     }

   comp = evas_gfx_compositor_get(op);
   if (comp)
     func = comp->composite_pixel_span_get(src_alpha, src_sparse_alpha, dst_alpha, pixels);
   if (func)
     return func;
   return _composite_span_nothing;
}

RGBA_Gfx_Func
evas_common_gfx_func_composite_color_span_get(DATA32 col, Eina_Bool dst_alpha, int pixels, int op)
{
   RGBA_Gfx_Compositor  *comp;
   RGBA_Gfx_Func        func = NULL;

   if ((col & 0xff000000) == 0xff000000)
     {
        if (op == _EVAS_RENDER_BLEND) op = _EVAS_RENDER_COPY;
        else if (op == EVAS_RENDER_BLEND_REL) op = _EVAS_RENDER_COPY_REL;
     }
   comp = evas_gfx_compositor_get(op);
   if (comp)
     func = comp->composite_color_span_get(col, dst_alpha, pixels);
   if (func)
     return func;
   return _composite_span_nothing;
}

RGBA_Gfx_Func
evas_common_gfx_func_composite_pixel_color_span_get(Eina_Bool src_alpha, Eina_Bool src_sparse_alpha, DATA32 col, Eina_Bool dst_alpha, int pixels, int op)
{
   RGBA_Gfx_Compositor  *comp;
   RGBA_Gfx_Func        func = NULL;

   if ((!src_alpha) && ((col & 0xff000000) == 0xff000000))
     {
        if (op == _EVAS_RENDER_BLEND) op = _EVAS_RENDER_COPY;
        else if (op == _EVAS_RENDER_BLEND_REL) op = _EVAS_RENDER_COPY_REL;
     }
   comp = evas_gfx_compositor_get(op);
   if (comp)
     func = comp->composite_pixel_color_span_get(src_alpha, src_sparse_alpha, col, dst_alpha, pixels);
   if (func)
     return func;
   return _composite_span_nothing;
}

RGBA_Gfx_Func
evas_common_gfx_func_composite_mask_color_span_get(DATA32 col, Eina_Bool dst_alpha, int pixels, int op)
{
   RGBA_Gfx_Compositor  *comp;
   RGBA_Gfx_Func        func = NULL;

   comp = evas_gfx_compositor_get(op);
   if (comp)
     func = comp->composite_mask_color_span_get(col, dst_alpha, pixels);
   if (func)
     return func;
   return _composite_span_nothing;
}

RGBA_Gfx_Func
evas_common_gfx_func_composite_pixel_mask_span_get(Eina_Bool src_alpha, Eina_Bool src_sparse_alpha, Eina_Bool dst_alpha, int pixels, int op)
{
   RGBA_Gfx_Compositor  *comp;
   RGBA_Gfx_Func        func = NULL;

   comp = evas_gfx_compositor_get(op);
   if (comp)
     func = comp->composite_pixel_mask_span_get(src_alpha, src_sparse_alpha, dst_alpha, pixels);
   if (func)
     return func;
   return _composite_span_nothing;
}

RGBA_Gfx_Pt_Func
evas_common_gfx_func_composite_pixel_pt_get(Eina_Bool src_alpha, Eina_Bool dst_alpha, int op)
{
   RGBA_Gfx_Compositor  *comp;
   RGBA_Gfx_Pt_Func     func = NULL;

   if (!src_alpha)
     {
        if (op == _EVAS_RENDER_BLEND) op = _EVAS_RENDER_COPY;
        else if (op == _EVAS_RENDER_BLEND_REL) op = _EVAS_RENDER_COPY_REL;
     }
   comp = evas_gfx_compositor_get(op);
   if (comp)
     func = comp->composite_pixel_pt_get(src_alpha, dst_alpha);
   if (func)
     return func;
   return _composite_pt_nothing;
}

RGBA_Gfx_Pt_Func
evas_common_gfx_func_composite_color_pt_get(DATA32 col, Eina_Bool dst_alpha, int op)
{
   RGBA_Gfx_Compositor  *comp;
   RGBA_Gfx_Pt_Func     func = NULL;

   if ((col & 0xff000000) == 0xff000000)
     {
        if (op == _EVAS_RENDER_BLEND) op = _EVAS_RENDER_COPY;
        else if (op == EVAS_RENDER_BLEND_REL) op = _EVAS_RENDER_COPY_REL;
     }
   comp = evas_gfx_compositor_get(op);
   if (comp)
     func = comp->composite_color_pt_get(col, dst_alpha);
   if (func)
     return func;
   return _composite_pt_nothing;
}

RGBA_Gfx_Pt_Func
evas_common_gfx_func_composite_pixel_color_pt_get(Eina_Bool src_alpha, DATA32 col, Eina_Bool dst_alpha, int op)
{
   RGBA_Gfx_Compositor  *comp;
   RGBA_Gfx_Pt_Func     func = NULL;

   if ((!src_alpha) && ((col & 0xff000000) == 0xff000000))
     {
        if (op == _EVAS_RENDER_BLEND) op = _EVAS_RENDER_COPY;
        else if (op == _EVAS_RENDER_BLEND_REL) op = _EVAS_RENDER_COPY_REL;
     }
   comp = evas_gfx_compositor_get(op);
   if (comp)
     func = comp->composite_pixel_color_pt_get(src_alpha, col, dst_alpha);
   if (func)
     return func;
   return _composite_pt_nothing;
}

RGBA_Gfx_Pt_Func
evas_common_gfx_func_composite_mask_color_pt_get(DATA32 col, Eina_Bool dst_alpha, int op)
{
   RGBA_Gfx_Compositor  *comp;
   RGBA_Gfx_Pt_Func     func = NULL;

   comp = evas_gfx_compositor_get(op);
   if (comp)
     func = comp->composite_mask_color_pt_get(col, dst_alpha);
   if (func)
     return func;
   return _composite_pt_nothing;
}

RGBA_Gfx_Pt_Func
evas_common_gfx_func_composite_pixel_mask_pt_get(Eina_Bool src_alpha, Eina_Bool dst_alpha, int op)
{
   RGBA_Gfx_Compositor  *comp;
   RGBA_Gfx_Pt_Func     func = NULL;

   comp = evas_gfx_compositor_get(op);
   if (comp)
     func = comp->composite_pixel_mask_pt_get(src_alpha, dst_alpha);
   if (func)
     return func;
   return _composite_pt_nothing;
}
