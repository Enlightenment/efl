EWAPI const Efl_Event_Description _EVAS_BOX_EVENT_CHILD_ADDED =
   EFL_EVENT_DESCRIPTION("child,added");
EWAPI const Efl_Event_Description _EVAS_BOX_EVENT_CHILD_REMOVED =
   EFL_EVENT_DESCRIPTION("child,removed");

void _evas_box_align_set(Eo *obj, Evas_Object_Box_Data *pd, double horizontal, double vertical);

EOAPI EFL_VOID_FUNC_BODYV(evas_obj_box_align_set, EFL_FUNC_CALL(horizontal, vertical), double horizontal, double vertical);

void _evas_box_align_get(const Eo *obj, Evas_Object_Box_Data *pd, double *horizontal, double *vertical);

EOAPI EFL_VOID_FUNC_BODYV_CONST(evas_obj_box_align_get, EFL_FUNC_CALL(horizontal, vertical), double *horizontal, double *vertical);

void _evas_box_padding_set(Eo *obj, Evas_Object_Box_Data *pd, int horizontal, int vertical);

EOAPI EFL_VOID_FUNC_BODYV(evas_obj_box_padding_set, EFL_FUNC_CALL(horizontal, vertical), int horizontal, int vertical);

void _evas_box_padding_get(const Eo *obj, Evas_Object_Box_Data *pd, int *horizontal, int *vertical);

EOAPI EFL_VOID_FUNC_BODYV_CONST(evas_obj_box_padding_get, EFL_FUNC_CALL(horizontal, vertical), int *horizontal, int *vertical);

void _evas_box_layout_set(Eo *obj, Evas_Object_Box_Data *pd, Evas_Object_Box_Layout cb, const void *data, Eina_Free_Cb free_data);

EOAPI EFL_VOID_FUNC_BODYV(evas_obj_box_layout_set, EFL_FUNC_CALL(cb, data, free_data), Evas_Object_Box_Layout cb, const void *data, Eina_Free_Cb free_data);

void _evas_box_layout_horizontal(Eo *obj, Evas_Object_Box_Data *pd, Evas_Object_Box_Data *priv, void *data);

EOAPI EFL_VOID_FUNC_BODYV(evas_obj_box_layout_horizontal, EFL_FUNC_CALL(priv, data), Evas_Object_Box_Data *priv, void *data);

void _evas_box_layout_vertical(Eo *obj, Evas_Object_Box_Data *pd, Evas_Object_Box_Data *priv, void *data);

EOAPI EFL_VOID_FUNC_BODYV(evas_obj_box_layout_vertical, EFL_FUNC_CALL(priv, data), Evas_Object_Box_Data *priv, void *data);

void _evas_box_layout_homogeneous_max_size_horizontal(Eo *obj, Evas_Object_Box_Data *pd, Evas_Object_Box_Data *priv, void *data);

EOAPI EFL_VOID_FUNC_BODYV(evas_obj_box_layout_homogeneous_max_size_horizontal, EFL_FUNC_CALL(priv, data), Evas_Object_Box_Data *priv, void *data);

Efl_Canvas_Object *_evas_box_internal_remove(Eo *obj, Evas_Object_Box_Data *pd, Efl_Canvas_Object *child);

EOAPI EFL_FUNC_BODYV(evas_obj_box_internal_remove, Efl_Canvas_Object *, NULL, EFL_FUNC_CALL(child), Efl_Canvas_Object *child);

void _evas_box_layout_flow_vertical(Eo *obj, Evas_Object_Box_Data *pd, Evas_Object_Box_Data *priv, void *data);

EOAPI EFL_VOID_FUNC_BODYV(evas_obj_box_layout_flow_vertical, EFL_FUNC_CALL(priv, data), Evas_Object_Box_Data *priv, void *data);

void _evas_box_internal_option_free(Eo *obj, Evas_Object_Box_Data *pd, Evas_Object_Box_Option *opt);

EOAPI EFL_VOID_FUNC_BODYV(evas_obj_box_internal_option_free, EFL_FUNC_CALL(opt), Evas_Object_Box_Option *opt);

Evas_Object_Box_Option *_evas_box_insert_after(Eo *obj, Evas_Object_Box_Data *pd, Efl_Canvas_Object *child, const Efl_Canvas_Object *reference);

EOAPI EFL_FUNC_BODYV(evas_obj_box_insert_after, Evas_Object_Box_Option *, NULL, EFL_FUNC_CALL(child, reference), Efl_Canvas_Object *child, const Efl_Canvas_Object *reference);

Eina_Bool _evas_box_remove_all(Eo *obj, Evas_Object_Box_Data *pd, Eina_Bool clear);

EOAPI EFL_FUNC_BODYV(evas_obj_box_remove_all, Eina_Bool, 0, EFL_FUNC_CALL(clear), Eina_Bool clear);

Eina_Iterator *_evas_box_iterator_new(const Eo *obj, Evas_Object_Box_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_obj_box_iterator_new, Eina_Iterator *, NULL);

Efl_Canvas_Object *_evas_box_add_to(Eo *obj, Evas_Object_Box_Data *pd);

EOAPI EFL_FUNC_BODY(evas_obj_box_add_to, Efl_Canvas_Object *, NULL);

Evas_Object_Box_Option *_evas_box_append(Eo *obj, Evas_Object_Box_Data *pd, Efl_Canvas_Object *child);

EOAPI EFL_FUNC_BODYV(evas_obj_box_append, Evas_Object_Box_Option *, NULL, EFL_FUNC_CALL(child), Efl_Canvas_Object *child);

int _evas_box_option_property_id_get(const Eo *obj, Evas_Object_Box_Data *pd, const char *name);

EOAPI EFL_FUNC_BODYV_CONST(evas_obj_box_option_property_id_get, int, 0, EFL_FUNC_CALL(name), const char *name);

Evas_Object_Box_Option *_evas_box_prepend(Eo *obj, Evas_Object_Box_Data *pd, Efl_Canvas_Object *child);

EOAPI EFL_FUNC_BODYV(evas_obj_box_prepend, Evas_Object_Box_Option *, NULL, EFL_FUNC_CALL(child), Efl_Canvas_Object *child);

Eina_Accessor *_evas_box_accessor_new(const Eo *obj, Evas_Object_Box_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_obj_box_accessor_new, Eina_Accessor *, NULL);

Evas_Object_Box_Option *_evas_box_internal_append(Eo *obj, Evas_Object_Box_Data *pd, Efl_Canvas_Object *child);

EOAPI EFL_FUNC_BODYV(evas_obj_box_internal_append, Evas_Object_Box_Option *, NULL, EFL_FUNC_CALL(child), Efl_Canvas_Object *child);

Eina_Bool _evas_box_option_property_vset(Eo *obj, Evas_Object_Box_Data *pd, Evas_Object_Box_Option *opt, int property, va_list *args);

EOAPI EFL_FUNC_BODYV(evas_obj_box_option_property_vset, Eina_Bool, 0, EFL_FUNC_CALL(opt, property, args), Evas_Object_Box_Option *opt, int property, va_list *args);

Efl_Canvas_Object *_evas_box_internal_remove_at(Eo *obj, Evas_Object_Box_Data *pd, unsigned int pos);

EOAPI EFL_FUNC_BODYV(evas_obj_box_internal_remove_at, Efl_Canvas_Object *, NULL, EFL_FUNC_CALL(pos), unsigned int pos);

Eina_Bool _evas_box_remove_at(Eo *obj, Evas_Object_Box_Data *pd, unsigned int pos);

EOAPI EFL_FUNC_BODYV(evas_obj_box_remove_at, Eina_Bool, 0, EFL_FUNC_CALL(pos), unsigned int pos);

Eina_Bool _evas_box_option_property_vget(const Eo *obj, Evas_Object_Box_Data *pd, Evas_Object_Box_Option *opt, int property, va_list *args);

EOAPI EFL_FUNC_BODYV_CONST(evas_obj_box_option_property_vget, Eina_Bool, 0, EFL_FUNC_CALL(opt, property, args), Evas_Object_Box_Option *opt, int property, va_list *args);

Evas_Object_Box_Option *_evas_box_internal_insert_at(Eo *obj, Evas_Object_Box_Data *pd, Efl_Canvas_Object *child, unsigned int pos);

EOAPI EFL_FUNC_BODYV(evas_obj_box_internal_insert_at, Evas_Object_Box_Option *, NULL, EFL_FUNC_CALL(child, pos), Efl_Canvas_Object *child, unsigned int pos);

Evas_Object_Box_Option *_evas_box_insert_before(Eo *obj, Evas_Object_Box_Data *pd, Efl_Canvas_Object *child, const Efl_Canvas_Object *reference);

EOAPI EFL_FUNC_BODYV(evas_obj_box_insert_before, Evas_Object_Box_Option *, NULL, EFL_FUNC_CALL(child, reference), Efl_Canvas_Object *child, const Efl_Canvas_Object *reference);

const char *_evas_box_option_property_name_get(const Eo *obj, Evas_Object_Box_Data *pd, int property);

EOAPI EFL_FUNC_BODYV_CONST(evas_obj_box_option_property_name_get, const char *, NULL, EFL_FUNC_CALL(property), int property);

Evas_Object_Box_Option *_evas_box_internal_insert_before(Eo *obj, Evas_Object_Box_Data *pd, Efl_Canvas_Object *child, const Efl_Canvas_Object *reference);

EOAPI EFL_FUNC_BODYV(evas_obj_box_internal_insert_before, Evas_Object_Box_Option *, NULL, EFL_FUNC_CALL(child, reference), Efl_Canvas_Object *child, const Efl_Canvas_Object *reference);

void _evas_box_layout_homogeneous_horizontal(Eo *obj, Evas_Object_Box_Data *pd, Evas_Object_Box_Data *priv, void *data);

EOAPI EFL_VOID_FUNC_BODYV(evas_obj_box_layout_homogeneous_horizontal, EFL_FUNC_CALL(priv, data), Evas_Object_Box_Data *priv, void *data);

Evas_Object_Box_Option *_evas_box_internal_option_new(Eo *obj, Evas_Object_Box_Data *pd, Efl_Canvas_Object *child);

EOAPI EFL_FUNC_BODYV(evas_obj_box_internal_option_new, Evas_Object_Box_Option *, NULL, EFL_FUNC_CALL(child), Efl_Canvas_Object *child);

void _evas_box_layout_homogeneous_max_size_vertical(Eo *obj, Evas_Object_Box_Data *pd, Evas_Object_Box_Data *priv, void *data);

EOAPI EFL_VOID_FUNC_BODYV(evas_obj_box_layout_homogeneous_max_size_vertical, EFL_FUNC_CALL(priv, data), Evas_Object_Box_Data *priv, void *data);

Evas_Object_Box_Option *_evas_box_internal_insert_after(Eo *obj, Evas_Object_Box_Data *pd, Efl_Canvas_Object *child, const Efl_Canvas_Object *reference);

EOAPI EFL_FUNC_BODYV(evas_obj_box_internal_insert_after, Evas_Object_Box_Option *, NULL, EFL_FUNC_CALL(child, reference), Efl_Canvas_Object *child, const Efl_Canvas_Object *reference);

Evas_Object_Box_Option *_evas_box_insert_at(Eo *obj, Evas_Object_Box_Data *pd, Efl_Canvas_Object *child, unsigned int pos);

EOAPI EFL_FUNC_BODYV(evas_obj_box_insert_at, Evas_Object_Box_Option *, NULL, EFL_FUNC_CALL(child, pos), Efl_Canvas_Object *child, unsigned int pos);

Evas_Object_Box_Option *_evas_box_internal_prepend(Eo *obj, Evas_Object_Box_Data *pd, Efl_Canvas_Object *child);

EOAPI EFL_FUNC_BODYV(evas_obj_box_internal_prepend, Evas_Object_Box_Option *, NULL, EFL_FUNC_CALL(child), Efl_Canvas_Object *child);

Eina_Bool _evas_box_remove(Eo *obj, Evas_Object_Box_Data *pd, Efl_Canvas_Object *child);

EOAPI EFL_FUNC_BODYV(evas_obj_box_remove, Eina_Bool, 0, EFL_FUNC_CALL(child), Efl_Canvas_Object *child);

void _evas_box_layout_stack(Eo *obj, Evas_Object_Box_Data *pd, Evas_Object_Box_Data *priv, void *data);

EOAPI EFL_VOID_FUNC_BODYV(evas_obj_box_layout_stack, EFL_FUNC_CALL(priv, data), Evas_Object_Box_Data *priv, void *data);

void _evas_box_layout_homogeneous_vertical(Eo *obj, Evas_Object_Box_Data *pd, Evas_Object_Box_Data *priv, void *data);

EOAPI EFL_VOID_FUNC_BODYV(evas_obj_box_layout_homogeneous_vertical, EFL_FUNC_CALL(priv, data), Evas_Object_Box_Data *priv, void *data);

void _evas_box_layout_flow_horizontal(Eo *obj, Evas_Object_Box_Data *pd, Evas_Object_Box_Data *priv, void *data);

EOAPI EFL_VOID_FUNC_BODYV(evas_obj_box_layout_flow_horizontal, EFL_FUNC_CALL(priv, data), Evas_Object_Box_Data *priv, void *data);

int _evas_box_count(Eo *obj, Evas_Object_Box_Data *pd);

EOAPI EFL_FUNC_BODY(evas_obj_box_count, int, 0);

Efl_Object *_evas_box_efl_object_constructor(Eo *obj, Evas_Object_Box_Data *pd);


void _evas_box_efl_gfx_entity_size_set(Eo *obj, Evas_Object_Box_Data *pd, Eina_Size2D size);


void _evas_box_efl_gfx_entity_position_set(Eo *obj, Evas_Object_Box_Data *pd, Eina_Position2D pos);


void _evas_box_efl_canvas_group_group_calculate(Eo *obj, Evas_Object_Box_Data *pd);


static Eina_Bool
_evas_box_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef EVAS_BOX_EXTRA_OPS
#define EVAS_BOX_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(evas_obj_box_align_set, _evas_box_align_set),
      EFL_OBJECT_OP_FUNC(evas_obj_box_align_get, _evas_box_align_get),
      EFL_OBJECT_OP_FUNC(evas_obj_box_padding_set, _evas_box_padding_set),
      EFL_OBJECT_OP_FUNC(evas_obj_box_padding_get, _evas_box_padding_get),
      EFL_OBJECT_OP_FUNC(evas_obj_box_layout_set, _evas_box_layout_set),
      EFL_OBJECT_OP_FUNC(evas_obj_box_layout_horizontal, _evas_box_layout_horizontal),
      EFL_OBJECT_OP_FUNC(evas_obj_box_layout_vertical, _evas_box_layout_vertical),
      EFL_OBJECT_OP_FUNC(evas_obj_box_layout_homogeneous_max_size_horizontal, _evas_box_layout_homogeneous_max_size_horizontal),
      EFL_OBJECT_OP_FUNC(evas_obj_box_internal_remove, _evas_box_internal_remove),
      EFL_OBJECT_OP_FUNC(evas_obj_box_layout_flow_vertical, _evas_box_layout_flow_vertical),
      EFL_OBJECT_OP_FUNC(evas_obj_box_internal_option_free, _evas_box_internal_option_free),
      EFL_OBJECT_OP_FUNC(evas_obj_box_insert_after, _evas_box_insert_after),
      EFL_OBJECT_OP_FUNC(evas_obj_box_remove_all, _evas_box_remove_all),
      EFL_OBJECT_OP_FUNC(evas_obj_box_iterator_new, _evas_box_iterator_new),
      EFL_OBJECT_OP_FUNC(evas_obj_box_add_to, _evas_box_add_to),
      EFL_OBJECT_OP_FUNC(evas_obj_box_append, _evas_box_append),
      EFL_OBJECT_OP_FUNC(evas_obj_box_option_property_id_get, _evas_box_option_property_id_get),
      EFL_OBJECT_OP_FUNC(evas_obj_box_prepend, _evas_box_prepend),
      EFL_OBJECT_OP_FUNC(evas_obj_box_accessor_new, _evas_box_accessor_new),
      EFL_OBJECT_OP_FUNC(evas_obj_box_internal_append, _evas_box_internal_append),
      EFL_OBJECT_OP_FUNC(evas_obj_box_option_property_vset, _evas_box_option_property_vset),
      EFL_OBJECT_OP_FUNC(evas_obj_box_internal_remove_at, _evas_box_internal_remove_at),
      EFL_OBJECT_OP_FUNC(evas_obj_box_remove_at, _evas_box_remove_at),
      EFL_OBJECT_OP_FUNC(evas_obj_box_option_property_vget, _evas_box_option_property_vget),
      EFL_OBJECT_OP_FUNC(evas_obj_box_internal_insert_at, _evas_box_internal_insert_at),
      EFL_OBJECT_OP_FUNC(evas_obj_box_insert_before, _evas_box_insert_before),
      EFL_OBJECT_OP_FUNC(evas_obj_box_option_property_name_get, _evas_box_option_property_name_get),
      EFL_OBJECT_OP_FUNC(evas_obj_box_internal_insert_before, _evas_box_internal_insert_before),
      EFL_OBJECT_OP_FUNC(evas_obj_box_layout_homogeneous_horizontal, _evas_box_layout_homogeneous_horizontal),
      EFL_OBJECT_OP_FUNC(evas_obj_box_internal_option_new, _evas_box_internal_option_new),
      EFL_OBJECT_OP_FUNC(evas_obj_box_layout_homogeneous_max_size_vertical, _evas_box_layout_homogeneous_max_size_vertical),
      EFL_OBJECT_OP_FUNC(evas_obj_box_internal_insert_after, _evas_box_internal_insert_after),
      EFL_OBJECT_OP_FUNC(evas_obj_box_insert_at, _evas_box_insert_at),
      EFL_OBJECT_OP_FUNC(evas_obj_box_internal_prepend, _evas_box_internal_prepend),
      EFL_OBJECT_OP_FUNC(evas_obj_box_remove, _evas_box_remove),
      EFL_OBJECT_OP_FUNC(evas_obj_box_layout_stack, _evas_box_layout_stack),
      EFL_OBJECT_OP_FUNC(evas_obj_box_layout_homogeneous_vertical, _evas_box_layout_homogeneous_vertical),
      EFL_OBJECT_OP_FUNC(evas_obj_box_layout_flow_horizontal, _evas_box_layout_flow_horizontal),
      EFL_OBJECT_OP_FUNC(evas_obj_box_count, _evas_box_count),
      EFL_OBJECT_OP_FUNC(efl_constructor, _evas_box_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_size_set, _evas_box_efl_gfx_entity_size_set),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_position_set, _evas_box_efl_gfx_entity_position_set),
      EFL_OBJECT_OP_FUNC(efl_canvas_group_calculate, _evas_box_efl_canvas_group_group_calculate),
      EVAS_BOX_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _evas_box_class_desc = {
   EO_VERSION,
   "Evas.Box",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Evas_Object_Box_Data),
   _evas_box_class_initializer,
   _evas_box_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(evas_box_class_get, &_evas_box_class_desc, EFL_CANVAS_GROUP_CLASS, NULL);

#include "evas_box_eo.legacy.c"
