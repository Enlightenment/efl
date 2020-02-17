#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#define MY_CLASS EFL_CANVAS_VG_IMAGE_CLASS

typedef struct _Efl_Canvas_Vg_Image_Data Efl_Canvas_Vg_Image_Data;
struct _Efl_Canvas_Vg_Image_Data
{
   Ector_Buffer *buffer;
   void *image;
   int w;
   int h;
};

static void
_efl_canvas_vg_image_render_pre(Evas_Object_Protected_Data *vg_pd,
                                Efl_VG *obj EINA_UNUSED,
                                Efl_Canvas_Vg_Node_Data *nd,
                                void *engine EINA_UNUSED, void *output EINA_UNUSED, void *context EINA_UNUSED,
                                Ector_Surface *surface,
                                Eina_Matrix3 *ptransform,
                                int p_opacity,
                                Ector_Buffer *comp,
                                Efl_Gfx_Vg_Composite_Method comp_method,
                                void *data)
{
   Efl_Canvas_Vg_Image_Data *pd = data;
   int a;

   if (nd->flags == EFL_GFX_CHANGE_FLAG_NONE) return;

   efl_gfx_color_get(obj, NULL, NULL, NULL, &a);
   if (a <= 0) return;

   nd->flags = EFL_GFX_CHANGE_FLAG_NONE;

   EFL_CANVAS_VG_COMPUTE_MATRIX(ctransform, ptransform, nd);
   EFL_CANVAS_VG_COMPUTE_ALPHA(c_r, c_g, c_b, c_a, p_opacity, nd);

   if (!nd->renderer)
     {
        efl_domain_current_push(EFL_ID_DOMAIN_SHARED);
        nd->renderer = ector_surface_renderer_factory_new(surface, ECTOR_RENDERER_IMAGE_MIXIN);
        efl_domain_current_pop();
     }

   if (!pd->buffer && pd->image)
     {
        Evas_Object_Protected_Data *obj = vg_pd;
        pd->buffer = ENFN->ector_buffer_new(ENC, obj->layer->evas->evas,
                                            pd->w, pd->h,
                                            EFL_GFX_COLORSPACE_ARGB8888,
                                            ECTOR_BUFFER_FLAG_DRAWABLE |
                                            ECTOR_BUFFER_FLAG_CPU_READABLE |
                                            ECTOR_BUFFER_FLAG_CPU_WRITABLE);
        ector_buffer_pixels_set(pd->buffer, pd->image,
                                pd->w, pd->h, 0,
                                EFL_GFX_COLORSPACE_ARGB8888, EINA_TRUE);
     }
   ector_renderer_image_buffer_set(nd->renderer, pd->buffer);
   ector_renderer_transformation_set(nd->renderer, ctransform);
   ector_renderer_origin_set(nd->renderer, nd->x, nd->y);
   ector_renderer_color_set(nd->renderer, c_r, c_g, c_b, c_a);
   ector_renderer_visibility_set(nd->renderer, nd->visibility);
   ector_renderer_comp_method_set(nd->renderer, comp, comp_method);
   ector_renderer_prepare(nd->renderer);
}

static Eo *
_efl_canvas_vg_image_efl_object_constructor(Eo *obj, Efl_Canvas_Vg_Image_Data *pd)
{
   Efl_Canvas_Vg_Node_Data *nd;

   obj = efl_constructor(efl_super(obj, MY_CLASS));

   nd = efl_data_scope_get(obj, EFL_CANVAS_VG_NODE_CLASS);
   nd->render_pre = _efl_canvas_vg_image_render_pre;
   nd->data = pd;

   efl_gfx_color_set(obj , 255, 255, 255, 255);

   return obj;
}

static void
_efl_canvas_vg_image_efl_object_destructor(Eo *obj, Efl_Canvas_Vg_Image_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
   if (pd->buffer)
     {
        efl_unref(pd->buffer);
        pd->buffer = NULL;
     }
}

static void
_efl_canvas_vg_image_data_set(Eo *obj EINA_UNUSED, Efl_Canvas_Vg_Image_Data *pd, void *data, Eina_Size2D size)
{
   if (!data || size.w <= 0 || size.h <= 0)
     return;

   if ((pd->image != data || pd->w != size.w || pd->h != size.h) && pd->buffer)
     {
        efl_unref(pd->buffer);
        pd->buffer= NULL;
     }

   pd->image = data;
   pd->w = size.w;
   pd->h = size.h;
}

#include "efl_canvas_vg_image.eo.c"
