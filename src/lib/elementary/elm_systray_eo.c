
void _elm_systray_id_set(Eo *obj, void *pd, const char *id);


static Eina_Error
__eolian_elm_systray_id_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   const char *cval;
   if (!eina_value_string_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_systray_id_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_systray_id_set, EFL_FUNC_CALL(id), const char *id);

const char *_elm_systray_id_get(const Eo *obj, void *pd);


static Eina_Value
__eolian_elm_systray_id_get_reflect(const Eo *obj)
{
   const char *val = elm_obj_systray_id_get(obj);
   return eina_value_string_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_systray_id_get, const char *, NULL);

void _elm_systray_category_set(Eo *obj, void *pd, Elm_Systray_Category cat);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_systray_category_set, EFL_FUNC_CALL(cat), Elm_Systray_Category cat);

Elm_Systray_Category _elm_systray_category_get(const Eo *obj, void *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_systray_category_get, Elm_Systray_Category, 0);

void _elm_systray_icon_theme_path_set(Eo *obj, void *pd, const char *icon_theme_path);


static Eina_Error
__eolian_elm_systray_icon_theme_path_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   const char *cval;
   if (!eina_value_string_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_systray_icon_theme_path_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_systray_icon_theme_path_set, EFL_FUNC_CALL(icon_theme_path), const char *icon_theme_path);

const char *_elm_systray_icon_theme_path_get(const Eo *obj, void *pd);


static Eina_Value
__eolian_elm_systray_icon_theme_path_get_reflect(const Eo *obj)
{
   const char *val = elm_obj_systray_icon_theme_path_get(obj);
   return eina_value_string_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_systray_icon_theme_path_get, const char *, NULL);

void _elm_systray_menu_set(Eo *obj, void *pd, const Efl_Object *menu);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_systray_menu_set, EFL_FUNC_CALL(menu), const Efl_Object *menu);

const Efl_Object *_elm_systray_menu_get(const Eo *obj, void *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_systray_menu_get, const Efl_Object *, NULL);

void _elm_systray_att_icon_name_set(Eo *obj, void *pd, const char *att_icon_name);


static Eina_Error
__eolian_elm_systray_att_icon_name_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   const char *cval;
   if (!eina_value_string_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_systray_att_icon_name_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_systray_att_icon_name_set, EFL_FUNC_CALL(att_icon_name), const char *att_icon_name);

const char *_elm_systray_att_icon_name_get(const Eo *obj, void *pd);


static Eina_Value
__eolian_elm_systray_att_icon_name_get_reflect(const Eo *obj)
{
   const char *val = elm_obj_systray_att_icon_name_get(obj);
   return eina_value_string_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_systray_att_icon_name_get, const char *, NULL);

void _elm_systray_status_set(Eo *obj, void *pd, Elm_Systray_Status st);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_systray_status_set, EFL_FUNC_CALL(st), Elm_Systray_Status st);

Elm_Systray_Status _elm_systray_status_get(const Eo *obj, void *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_systray_status_get, Elm_Systray_Status, 0);

void _elm_systray_icon_name_set(Eo *obj, void *pd, const char *icon_name);


static Eina_Error
__eolian_elm_systray_icon_name_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   const char *cval;
   if (!eina_value_string_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_systray_icon_name_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_systray_icon_name_set, EFL_FUNC_CALL(icon_name), const char *icon_name);

const char *_elm_systray_icon_name_get(const Eo *obj, void *pd);


static Eina_Value
__eolian_elm_systray_icon_name_get_reflect(const Eo *obj)
{
   const char *val = elm_obj_systray_icon_name_get(obj);
   return eina_value_string_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_systray_icon_name_get, const char *, NULL);

void _elm_systray_title_set(Eo *obj, void *pd, const char *title);


static Eina_Error
__eolian_elm_systray_title_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   const char *cval;
   if (!eina_value_string_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_systray_title_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_systray_title_set, EFL_FUNC_CALL(title), const char *title);

const char *_elm_systray_title_get(const Eo *obj, void *pd);


static Eina_Value
__eolian_elm_systray_title_get_reflect(const Eo *obj)
{
   const char *val = elm_obj_systray_title_get(obj);
   return eina_value_string_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_systray_title_get, const char *, NULL);

Eina_Bool _elm_systray_register(Eo *obj, void *pd);

EOAPI EFL_FUNC_BODY(elm_obj_systray_register, Eina_Bool, 0);

static Eina_Bool
_elm_systray_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_SYSTRAY_EXTRA_OPS
#define ELM_SYSTRAY_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_systray_id_set, _elm_systray_id_set),
      EFL_OBJECT_OP_FUNC(elm_obj_systray_id_get, _elm_systray_id_get),
      EFL_OBJECT_OP_FUNC(elm_obj_systray_category_set, _elm_systray_category_set),
      EFL_OBJECT_OP_FUNC(elm_obj_systray_category_get, _elm_systray_category_get),
      EFL_OBJECT_OP_FUNC(elm_obj_systray_icon_theme_path_set, _elm_systray_icon_theme_path_set),
      EFL_OBJECT_OP_FUNC(elm_obj_systray_icon_theme_path_get, _elm_systray_icon_theme_path_get),
      EFL_OBJECT_OP_FUNC(elm_obj_systray_menu_set, _elm_systray_menu_set),
      EFL_OBJECT_OP_FUNC(elm_obj_systray_menu_get, _elm_systray_menu_get),
      EFL_OBJECT_OP_FUNC(elm_obj_systray_att_icon_name_set, _elm_systray_att_icon_name_set),
      EFL_OBJECT_OP_FUNC(elm_obj_systray_att_icon_name_get, _elm_systray_att_icon_name_get),
      EFL_OBJECT_OP_FUNC(elm_obj_systray_status_set, _elm_systray_status_set),
      EFL_OBJECT_OP_FUNC(elm_obj_systray_status_get, _elm_systray_status_get),
      EFL_OBJECT_OP_FUNC(elm_obj_systray_icon_name_set, _elm_systray_icon_name_set),
      EFL_OBJECT_OP_FUNC(elm_obj_systray_icon_name_get, _elm_systray_icon_name_get),
      EFL_OBJECT_OP_FUNC(elm_obj_systray_title_set, _elm_systray_title_set),
      EFL_OBJECT_OP_FUNC(elm_obj_systray_title_get, _elm_systray_title_get),
      EFL_OBJECT_OP_FUNC(elm_obj_systray_register, _elm_systray_register),
      ELM_SYSTRAY_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"id", __eolian_elm_systray_id_set_reflect, __eolian_elm_systray_id_get_reflect},
      {"icon_theme_path", __eolian_elm_systray_icon_theme_path_set_reflect, __eolian_elm_systray_icon_theme_path_get_reflect},
      {"att_icon_name", __eolian_elm_systray_att_icon_name_set_reflect, __eolian_elm_systray_att_icon_name_get_reflect},
      {"icon_name", __eolian_elm_systray_icon_name_set_reflect, __eolian_elm_systray_icon_name_get_reflect},
      {"title", __eolian_elm_systray_title_set_reflect, __eolian_elm_systray_title_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_systray_class_desc = {
   EO_VERSION,
   "Elm.Systray",
   EFL_CLASS_TYPE_REGULAR,
   0,
   _elm_systray_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_systray_class_get, &_elm_systray_class_desc, EFL_OBJECT_CLASS, NULL);

#include "elm_systray_eo.legacy.c"
