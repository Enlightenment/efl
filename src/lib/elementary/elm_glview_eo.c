EWAPI const Efl_Event_Description _ELM_GLVIEW_EVENT_CREATED =
   EFL_EVENT_DESCRIPTION("created");
EWAPI const Efl_Event_Description _ELM_GLVIEW_EVENT_DESTROYED =
   EFL_EVENT_DESCRIPTION("destroyed");
EWAPI const Efl_Event_Description _ELM_GLVIEW_EVENT_RESIZED =
   EFL_EVENT_DESCRIPTION("resized");
EWAPI const Efl_Event_Description _ELM_GLVIEW_EVENT_RENDER =
   EFL_EVENT_DESCRIPTION("render");

void _elm_glview_version_constructor(Eo *obj, Elm_Glview_Data *pd, Evas_GL_Context_Version version);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_glview_version_constructor, EFL_FUNC_CALL(version), Evas_GL_Context_Version version);

Eina_Bool _elm_glview_resize_policy_set(Eo *obj, Elm_Glview_Data *pd, Elm_GLView_Resize_Policy policy);

EOAPI EFL_FUNC_BODYV(elm_obj_glview_resize_policy_set, Eina_Bool, 0, EFL_FUNC_CALL(policy), Elm_GLView_Resize_Policy policy);

Eina_Bool _elm_glview_render_policy_set(Eo *obj, Elm_Glview_Data *pd, Elm_GLView_Render_Policy policy);

EOAPI EFL_FUNC_BODYV(elm_obj_glview_render_policy_set, Eina_Bool, 0, EFL_FUNC_CALL(policy), Elm_GLView_Render_Policy policy);

Eina_Bool _elm_glview_mode_set(Eo *obj, Elm_Glview_Data *pd, Elm_GLView_Mode mode);

EOAPI EFL_FUNC_BODYV(elm_obj_glview_mode_set, Eina_Bool, 0, EFL_FUNC_CALL(mode), Elm_GLView_Mode mode);

Evas_GL_API *_elm_glview_gl_api_get(const Eo *obj, Elm_Glview_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_glview_gl_api_get, Evas_GL_API *, NULL);

Evas_GL *_elm_glview_evas_gl_get(const Eo *obj, Elm_Glview_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_glview_evas_gl_get, Evas_GL *, NULL);

int _elm_glview_rotation_get(const Eo *obj, Elm_Glview_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_glview_rotation_get, int, 0);

void _elm_glview_draw_request(Eo *obj, Elm_Glview_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_glview_draw_request);

Efl_Object *_elm_glview_efl_object_constructor(Eo *obj, Elm_Glview_Data *pd);


Efl_Object *_elm_glview_efl_object_finalize(Eo *obj, Elm_Glview_Data *pd);


void _elm_glview_efl_gfx_entity_size_set(Eo *obj, Elm_Glview_Data *pd, Eina_Size2D size);


Eina_Bool _elm_glview_efl_ui_focus_object_on_focus_update(Eo *obj, Elm_Glview_Data *pd);


void _elm_glview_efl_gfx_view_view_size_set(Eo *obj, Elm_Glview_Data *pd, Eina_Size2D size);


Eina_Size2D _elm_glview_efl_gfx_view_view_size_get(const Eo *obj, Elm_Glview_Data *pd);


static Eina_Bool
_elm_glview_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_GLVIEW_EXTRA_OPS
#define ELM_GLVIEW_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_glview_version_constructor, _elm_glview_version_constructor),
      EFL_OBJECT_OP_FUNC(elm_obj_glview_resize_policy_set, _elm_glview_resize_policy_set),
      EFL_OBJECT_OP_FUNC(elm_obj_glview_render_policy_set, _elm_glview_render_policy_set),
      EFL_OBJECT_OP_FUNC(elm_obj_glview_mode_set, _elm_glview_mode_set),
      EFL_OBJECT_OP_FUNC(elm_obj_glview_gl_api_get, _elm_glview_gl_api_get),
      EFL_OBJECT_OP_FUNC(elm_obj_glview_evas_gl_get, _elm_glview_evas_gl_get),
      EFL_OBJECT_OP_FUNC(elm_obj_glview_rotation_get, _elm_glview_rotation_get),
      EFL_OBJECT_OP_FUNC(elm_obj_glview_draw_request, _elm_glview_draw_request),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_glview_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_finalize, _elm_glview_efl_object_finalize),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_size_set, _elm_glview_efl_gfx_entity_size_set),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_object_on_focus_update, _elm_glview_efl_ui_focus_object_on_focus_update),
      EFL_OBJECT_OP_FUNC(efl_gfx_view_size_set, _elm_glview_efl_gfx_view_view_size_set),
      EFL_OBJECT_OP_FUNC(efl_gfx_view_size_get, _elm_glview_efl_gfx_view_view_size_get),
      ELM_GLVIEW_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_glview_class_desc = {
   EO_VERSION,
   "Elm.Glview",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Glview_Data),
   _elm_glview_class_initializer,
   _elm_glview_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_glview_class_get, &_elm_glview_class_desc, EFL_UI_WIDGET_CLASS, EFL_GFX_VIEW_INTERFACE, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_glview_eo.legacy.c"
