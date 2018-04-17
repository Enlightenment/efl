#include "evas_common_private.h"
#include "evas_private.h"

#define MY_CLASS EVAS_CANVAS3D_PRIMITIVE_CLASS

static inline void
_primitive_init(Evas_Canvas3D_Primitive_Data *pd)
{
   pd->form = EVAS_CANVAS3D_MESH_PRIMITIVE_NONE;
   pd->ratio = 3.0;
   pd->precision = 10;
   pd->surface = NULL;
   pd->tex_scale.x = 1.0;
   pd->tex_scale.y = 1.0;
}

EAPI Evas_Canvas3D_Primitive *
evas_canvas3d_primitive_add(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   return efl_add(MY_CLASS, e);
}

EOLIAN static Eo *
_evas_canvas3d_primitive_efl_object_constructor(Eo *obj, Evas_Canvas3D_Primitive_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   evas_canvas3d_object_type_set(obj, EVAS_CANVAS3D_OBJECT_TYPE_PRIMITIVE);
   _primitive_init(pd);

   return obj;
}

EOLIAN static void
_evas_canvas3d_primitive_efl_object_destructor(Eo *obj, Evas_Canvas3D_Primitive_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_evas_canvas3d_primitive_form_set(Eo *obj EINA_UNUSED,
                            Evas_Canvas3D_Primitive_Data *pd,
                            Evas_Canvas3D_Mesh_Primitive form)
{
   if (form >= EVAS_CANVAS3D_MESH_PRIMITIVE_COUNT)
     {
        ERR("Wrong form of primitive.");
        return;
     }

   if (pd->form != form)
     pd->form = form;
}

EOLIAN static Evas_Canvas3D_Mesh_Primitive
_evas_canvas3d_primitive_form_get(const Eo *obj EINA_UNUSED,
                            Evas_Canvas3D_Primitive_Data *pd)
{
   return pd->form;
}

EOLIAN static void
_evas_canvas3d_primitive_mode_set(Eo *obj EINA_UNUSED,
                            Evas_Canvas3D_Primitive_Data *pd,
                            Evas_Canvas3D_Primitive_Mode mode)
{
   if (pd->mode != mode)
     pd->mode = mode;
}

EOLIAN static Evas_Canvas3D_Primitive_Mode
_evas_canvas3d_primitive_mode_get(const Eo *obj EINA_UNUSED,
                            Evas_Canvas3D_Primitive_Data *pd)
{
   return pd->mode;
}

EOLIAN static void
_evas_canvas3d_primitive_ratio_set(Eo *obj EINA_UNUSED,
                             Evas_Canvas3D_Primitive_Data *pd,
                             Evas_Real ratio)
{
   pd->ratio = ratio;
}

EOLIAN static Evas_Real
_evas_canvas3d_primitive_ratio_get(const Eo *obj EINA_UNUSED,
                             Evas_Canvas3D_Primitive_Data *pd)
{
   return pd->ratio;
}

EOLIAN static void
_evas_canvas3d_primitive_precision_set(Eo *obj EINA_UNUSED,
                                 Evas_Canvas3D_Primitive_Data *pd,
                                 int precision)
{
   if ((precision < 3))
     {
        ERR("Precision of Evas_Canvas3D_Primitive cannot be less than 3.");
        return;
     }

   if ((precision >= 100))
     {
        ERR("%s\n%s",
            "Be careful with precision of Evas_Canvas3D_Primitive more than 100.",
            "It can give unacceptable result.");
     }

   if (pd->precision != precision)
     pd->precision = precision;
}

EOLIAN static int
_evas_canvas3d_primitive_precision_get(const Eo *obj EINA_UNUSED,
                                 Evas_Canvas3D_Primitive_Data *pd)
{
   return pd->precision;
}

EOLIAN static void
_evas_canvas3d_primitive_surface_set(Eo *obj EINA_UNUSED,
                               Evas_Canvas3D_Primitive_Data *pd,
                               Evas_Canvas3D_Surface_Func surface_func)
{
   pd->surface = surface_func;
}

EOLIAN static void
_evas_canvas3d_primitive_tex_scale_set(Eo *obj EINA_UNUSED,
                                 Evas_Canvas3D_Primitive_Data *pd,
                                 Evas_Real x,
                                 Evas_Real y)
{
   if (!EINA_DBL_EQ(x, 0.0)) pd->tex_scale.x = x;
   if (!EINA_DBL_EQ(y, 0.0)) pd->tex_scale.y = y;
}

EOLIAN static void
_evas_canvas3d_primitive_tex_scale_get(const Eo *obj EINA_UNUSED,
                                 Evas_Canvas3D_Primitive_Data *pd,
                                 Evas_Real *x,
                                 Evas_Real *y)
{
   if (x) *x = pd->tex_scale.x;
   if (y) *y = pd->tex_scale.y;
}

#include "canvas/evas_canvas3d_primitive.eo.c"
