EWAPI const Efl_Event_Description _ELM_PLUG_EVENT_IMAGE_DELETED =
   EFL_EVENT_DESCRIPTION("image,deleted");
EWAPI const Efl_Event_Description _ELM_PLUG_EVENT_IMAGE_RESIZED =
   EFL_EVENT_DESCRIPTION("image,resized");

Efl_Canvas_Object *_elm_plug_image_object_get(const Eo *obj, void *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_plug_image_object_get, Efl_Canvas_Object *, NULL);

Eina_Bool _elm_plug_connect(Eo *obj, void *pd, const char *svcname, int svcnum, Eina_Bool svcsys);

EOAPI EFL_FUNC_BODYV(elm_obj_plug_connect, Eina_Bool, 0, EFL_FUNC_CALL(svcname, svcnum, svcsys), const char *svcname, int svcnum, Eina_Bool svcsys);

Efl_Object *_elm_plug_efl_object_constructor(Eo *obj, void *pd);


Eina_Error _elm_plug_efl_ui_widget_theme_apply(Eo *obj, void *pd);


Eina_Bool _elm_plug_efl_ui_focus_object_on_focus_update(Eo *obj, void *pd);


static Eina_Bool
_elm_plug_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_PLUG_EXTRA_OPS
#define ELM_PLUG_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_plug_image_object_get, _elm_plug_image_object_get),
      EFL_OBJECT_OP_FUNC(elm_obj_plug_connect, _elm_plug_connect),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_plug_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_plug_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_object_on_focus_update, _elm_plug_efl_ui_focus_object_on_focus_update),
      ELM_PLUG_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_plug_class_desc = {
   EO_VERSION,
   "Elm.Plug",
   EFL_CLASS_TYPE_REGULAR,
   0,
   _elm_plug_class_initializer,
   _elm_plug_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_plug_class_get, &_elm_plug_class_desc, EFL_UI_WIDGET_CLASS, EFL_UI_CLICKABLE_MIXIN, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_plug_eo.legacy.c"
