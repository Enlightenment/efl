EWAPI const Efl_Event_Description _ELM_SLIDESHOW_EVENT_CHANGED =
   EFL_EVENT_DESCRIPTION("changed");
EWAPI const Efl_Event_Description _ELM_SLIDESHOW_EVENT_TRANSITION_END =
   EFL_EVENT_DESCRIPTION("transition,end");

void _elm_slideshow_cache_after_set(Eo *obj, Elm_Slideshow_Data *pd, int count);


static Eina_Error
__eolian_elm_slideshow_cache_after_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   int cval;
   if (!eina_value_int_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_slideshow_cache_after_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_slideshow_cache_after_set, EFL_FUNC_CALL(count), int count);

int _elm_slideshow_cache_after_get(const Eo *obj, Elm_Slideshow_Data *pd);


static Eina_Value
__eolian_elm_slideshow_cache_after_get_reflect(Eo *obj)
{
   int val = elm_obj_slideshow_cache_after_get(obj);
   return eina_value_int_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_slideshow_cache_after_get, int, 0);

void _elm_slideshow_cache_before_set(Eo *obj, Elm_Slideshow_Data *pd, int count);


static Eina_Error
__eolian_elm_slideshow_cache_before_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   int cval;
   if (!eina_value_int_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_slideshow_cache_before_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_slideshow_cache_before_set, EFL_FUNC_CALL(count), int count);

int _elm_slideshow_cache_before_get(const Eo *obj, Elm_Slideshow_Data *pd);


static Eina_Value
__eolian_elm_slideshow_cache_before_get_reflect(Eo *obj)
{
   int val = elm_obj_slideshow_cache_before_get(obj);
   return eina_value_int_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_slideshow_cache_before_get, int, 0);

void _elm_slideshow_layout_set(Eo *obj, Elm_Slideshow_Data *pd, const char *layout);


static Eina_Error
__eolian_elm_slideshow_layout_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   const char *cval;
   if (!eina_value_string_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_slideshow_layout_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_slideshow_layout_set, EFL_FUNC_CALL(layout), const char *layout);

const char *_elm_slideshow_layout_get(const Eo *obj, Elm_Slideshow_Data *pd);


static Eina_Value
__eolian_elm_slideshow_layout_get_reflect(Eo *obj)
{
   const char *val = elm_obj_slideshow_layout_get(obj);
   return eina_value_string_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_slideshow_layout_get, const char *, NULL);

void _elm_slideshow_transition_set(Eo *obj, Elm_Slideshow_Data *pd, const char *transition);


static Eina_Error
__eolian_elm_slideshow_transition_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   const char *cval;
   if (!eina_value_string_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_slideshow_transition_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_slideshow_transition_set, EFL_FUNC_CALL(transition), const char *transition);

const char *_elm_slideshow_transition_get(const Eo *obj, Elm_Slideshow_Data *pd);


static Eina_Value
__eolian_elm_slideshow_transition_get_reflect(Eo *obj)
{
   const char *val = elm_obj_slideshow_transition_get(obj);
   return eina_value_string_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_slideshow_transition_get, const char *, NULL);

void _elm_slideshow_items_loop_set(Eo *obj, Elm_Slideshow_Data *pd, Eina_Bool loop);


static Eina_Error
__eolian_elm_slideshow_items_loop_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_slideshow_items_loop_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_slideshow_items_loop_set, EFL_FUNC_CALL(loop), Eina_Bool loop);

Eina_Bool _elm_slideshow_items_loop_get(const Eo *obj, Elm_Slideshow_Data *pd);


static Eina_Value
__eolian_elm_slideshow_items_loop_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_slideshow_items_loop_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_slideshow_items_loop_get, Eina_Bool, 0);

void _elm_slideshow_timeout_set(Eo *obj, Elm_Slideshow_Data *pd, double timeout);


static Eina_Error
__eolian_elm_slideshow_timeout_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   double cval;
   if (!eina_value_double_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_slideshow_timeout_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_slideshow_timeout_set, EFL_FUNC_CALL(timeout), double timeout);

double _elm_slideshow_timeout_get(const Eo *obj, Elm_Slideshow_Data *pd);


static Eina_Value
__eolian_elm_slideshow_timeout_get_reflect(Eo *obj)
{
   double val = elm_obj_slideshow_timeout_get(obj);
   return eina_value_double_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_slideshow_timeout_get, double, 0);

const Eina_List *_elm_slideshow_items_get(const Eo *obj, Elm_Slideshow_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_slideshow_items_get, const Eina_List *, NULL);

const Eina_List *_elm_slideshow_transitions_get(const Eo *obj, Elm_Slideshow_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_slideshow_transitions_get, const Eina_List *, NULL);

unsigned int _elm_slideshow_count_get(const Eo *obj, Elm_Slideshow_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_slideshow_count_get, unsigned int, 0);

Elm_Widget_Item *_elm_slideshow_item_current_get(const Eo *obj, Elm_Slideshow_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_slideshow_item_current_get, Elm_Widget_Item *, NULL);

const Eina_List *_elm_slideshow_layouts_get(const Eo *obj, Elm_Slideshow_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_slideshow_layouts_get, const Eina_List *, NULL);

void _elm_slideshow_previous(Eo *obj, Elm_Slideshow_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_slideshow_previous);

Elm_Widget_Item *_elm_slideshow_item_nth_get(const Eo *obj, Elm_Slideshow_Data *pd, unsigned int nth);

EOAPI EFL_FUNC_BODYV_CONST(elm_obj_slideshow_item_nth_get, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(nth), unsigned int nth);

void _elm_slideshow_next(Eo *obj, Elm_Slideshow_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_slideshow_next);

void _elm_slideshow_clear(Eo *obj, Elm_Slideshow_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_slideshow_clear);

Elm_Widget_Item *_elm_slideshow_item_add(Eo *obj, Elm_Slideshow_Data *pd, const Elm_Slideshow_Item_Class *itc, const void *data);

EOAPI EFL_FUNC_BODYV(elm_obj_slideshow_item_add, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(itc, data), const Elm_Slideshow_Item_Class *itc, const void *data);

Elm_Widget_Item *_elm_slideshow_item_sorted_insert(Eo *obj, Elm_Slideshow_Data *pd, const Elm_Slideshow_Item_Class *itc, const void *data, Eina_Compare_Cb func);

EOAPI EFL_FUNC_BODYV(elm_obj_slideshow_item_sorted_insert, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(itc, data, func), const Elm_Slideshow_Item_Class *itc, const void *data, Eina_Compare_Cb func);

Efl_Object *_elm_slideshow_efl_object_constructor(Eo *obj, Elm_Slideshow_Data *pd);


Eina_Bool _elm_slideshow_efl_ui_widget_widget_input_event_handler(Eo *obj, Elm_Slideshow_Data *pd, const Efl_Event *eo_event, Efl_Canvas_Object *source);


const Efl_Access_Action_Data *_elm_slideshow_efl_access_widget_action_elm_actions_get(const Eo *obj, Elm_Slideshow_Data *pd);


static Eina_Bool
_elm_slideshow_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_SLIDESHOW_EXTRA_OPS
#define ELM_SLIDESHOW_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_slideshow_cache_after_set, _elm_slideshow_cache_after_set),
      EFL_OBJECT_OP_FUNC(elm_obj_slideshow_cache_after_get, _elm_slideshow_cache_after_get),
      EFL_OBJECT_OP_FUNC(elm_obj_slideshow_cache_before_set, _elm_slideshow_cache_before_set),
      EFL_OBJECT_OP_FUNC(elm_obj_slideshow_cache_before_get, _elm_slideshow_cache_before_get),
      EFL_OBJECT_OP_FUNC(elm_obj_slideshow_layout_set, _elm_slideshow_layout_set),
      EFL_OBJECT_OP_FUNC(elm_obj_slideshow_layout_get, _elm_slideshow_layout_get),
      EFL_OBJECT_OP_FUNC(elm_obj_slideshow_transition_set, _elm_slideshow_transition_set),
      EFL_OBJECT_OP_FUNC(elm_obj_slideshow_transition_get, _elm_slideshow_transition_get),
      EFL_OBJECT_OP_FUNC(elm_obj_slideshow_items_loop_set, _elm_slideshow_items_loop_set),
      EFL_OBJECT_OP_FUNC(elm_obj_slideshow_items_loop_get, _elm_slideshow_items_loop_get),
      EFL_OBJECT_OP_FUNC(elm_obj_slideshow_timeout_set, _elm_slideshow_timeout_set),
      EFL_OBJECT_OP_FUNC(elm_obj_slideshow_timeout_get, _elm_slideshow_timeout_get),
      EFL_OBJECT_OP_FUNC(elm_obj_slideshow_items_get, _elm_slideshow_items_get),
      EFL_OBJECT_OP_FUNC(elm_obj_slideshow_transitions_get, _elm_slideshow_transitions_get),
      EFL_OBJECT_OP_FUNC(elm_obj_slideshow_count_get, _elm_slideshow_count_get),
      EFL_OBJECT_OP_FUNC(elm_obj_slideshow_item_current_get, _elm_slideshow_item_current_get),
      EFL_OBJECT_OP_FUNC(elm_obj_slideshow_layouts_get, _elm_slideshow_layouts_get),
      EFL_OBJECT_OP_FUNC(elm_obj_slideshow_previous, _elm_slideshow_previous),
      EFL_OBJECT_OP_FUNC(elm_obj_slideshow_item_nth_get, _elm_slideshow_item_nth_get),
      EFL_OBJECT_OP_FUNC(elm_obj_slideshow_next, _elm_slideshow_next),
      EFL_OBJECT_OP_FUNC(elm_obj_slideshow_clear, _elm_slideshow_clear),
      EFL_OBJECT_OP_FUNC(elm_obj_slideshow_item_add, _elm_slideshow_item_add),
      EFL_OBJECT_OP_FUNC(elm_obj_slideshow_item_sorted_insert, _elm_slideshow_item_sorted_insert),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_slideshow_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_input_event_handler, _elm_slideshow_efl_ui_widget_widget_input_event_handler),
      EFL_OBJECT_OP_FUNC(efl_access_widget_action_elm_actions_get, _elm_slideshow_efl_access_widget_action_elm_actions_get),
      ELM_SLIDESHOW_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"cache_after", __eolian_elm_slideshow_cache_after_set_reflect, __eolian_elm_slideshow_cache_after_get_reflect},
      {"cache_before", __eolian_elm_slideshow_cache_before_set_reflect, __eolian_elm_slideshow_cache_before_get_reflect},
      {"layout", __eolian_elm_slideshow_layout_set_reflect, __eolian_elm_slideshow_layout_get_reflect},
      {"transition", __eolian_elm_slideshow_transition_set_reflect, __eolian_elm_slideshow_transition_get_reflect},
      {"items_loop", __eolian_elm_slideshow_items_loop_set_reflect, __eolian_elm_slideshow_items_loop_get_reflect},
      {"timeout", __eolian_elm_slideshow_timeout_set_reflect, __eolian_elm_slideshow_timeout_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_slideshow_class_desc = {
   EO_VERSION,
   "Elm.Slideshow",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Slideshow_Data),
   _elm_slideshow_class_initializer,
   _elm_slideshow_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_slideshow_class_get, &_elm_slideshow_class_desc, EFL_UI_LAYOUT_BASE_CLASS, EFL_ACCESS_WIDGET_ACTION_MIXIN, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_slideshow_eo.legacy.c"
