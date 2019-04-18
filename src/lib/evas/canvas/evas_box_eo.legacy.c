
EAPI void
evas_object_box_align_set(Evas_Box *obj, double horizontal, double vertical)
{
   evas_obj_box_align_set(obj, horizontal, vertical);
}

EAPI void
evas_object_box_align_get(const Evas_Box *obj, double *horizontal, double *vertical)
{
   evas_obj_box_align_get(obj, horizontal, vertical);
}

EAPI void
evas_object_box_padding_set(Evas_Box *obj, int horizontal, int vertical)
{
   evas_obj_box_padding_set(obj, horizontal, vertical);
}

EAPI void
evas_object_box_padding_get(const Evas_Box *obj, int *horizontal, int *vertical)
{
   evas_obj_box_padding_get(obj, horizontal, vertical);
}

EAPI void
evas_object_box_layout_set(Evas_Box *obj, Evas_Object_Box_Layout cb, const void *data, Eina_Free_Cb free_data)
{
   evas_obj_box_layout_set(obj, cb, data, free_data);
}

EAPI void
evas_object_box_layout_horizontal(Evas_Box *obj, Evas_Object_Box_Data *priv, void *data)
{
   evas_obj_box_layout_horizontal(obj, priv, data);
}

EAPI void
evas_object_box_layout_vertical(Evas_Box *obj, Evas_Object_Box_Data *priv, void *data)
{
   evas_obj_box_layout_vertical(obj, priv, data);
}

EAPI void
evas_object_box_layout_homogeneous_max_size_horizontal(Evas_Box *obj, Evas_Object_Box_Data *priv, void *data)
{
   evas_obj_box_layout_homogeneous_max_size_horizontal(obj, priv, data);
}

EAPI void
evas_object_box_layout_flow_vertical(Evas_Box *obj, Evas_Object_Box_Data *priv, void *data)
{
   evas_obj_box_layout_flow_vertical(obj, priv, data);
}

EAPI Evas_Object_Box_Option *
evas_object_box_insert_after(Evas_Box *obj, Efl_Canvas_Object *child, const Efl_Canvas_Object *reference)
{
   return evas_obj_box_insert_after(obj, child, reference);
}

EAPI Eina_Bool
evas_object_box_remove_all(Evas_Box *obj, Eina_Bool clear)
{
   return evas_obj_box_remove_all(obj, clear);
}

EAPI Eina_Iterator *
evas_object_box_iterator_new(const Evas_Box *obj)
{
   return evas_obj_box_iterator_new(obj);
}

EAPI Efl_Canvas_Object *
evas_object_box_add_to(Evas_Box *obj)
{
   return evas_obj_box_add_to(obj);
}

EAPI Evas_Object_Box_Option *
evas_object_box_append(Evas_Box *obj, Efl_Canvas_Object *child)
{
   return evas_obj_box_append(obj, child);
}

EAPI int
evas_object_box_option_property_id_get(const Evas_Box *obj, const char *name)
{
   return evas_obj_box_option_property_id_get(obj, name);
}

EAPI Evas_Object_Box_Option *
evas_object_box_prepend(Evas_Box *obj, Efl_Canvas_Object *child)
{
   return evas_obj_box_prepend(obj, child);
}

EAPI Eina_Accessor *
evas_object_box_accessor_new(const Evas_Box *obj)
{
   return evas_obj_box_accessor_new(obj);
}

EAPI Eina_Bool
evas_object_box_remove_at(Evas_Box *obj, unsigned int pos)
{
   return evas_obj_box_remove_at(obj, pos);
}

EAPI Evas_Object_Box_Option *
evas_object_box_insert_before(Evas_Box *obj, Efl_Canvas_Object *child, const Efl_Canvas_Object *reference)
{
   return evas_obj_box_insert_before(obj, child, reference);
}

EAPI const char *
evas_object_box_option_property_name_get(const Evas_Box *obj, int property)
{
   return evas_obj_box_option_property_name_get(obj, property);
}

EAPI void
evas_object_box_layout_homogeneous_horizontal(Evas_Box *obj, Evas_Object_Box_Data *priv, void *data)
{
   evas_obj_box_layout_homogeneous_horizontal(obj, priv, data);
}

EAPI void
evas_object_box_layout_homogeneous_max_size_vertical(Evas_Box *obj, Evas_Object_Box_Data *priv, void *data)
{
   evas_obj_box_layout_homogeneous_max_size_vertical(obj, priv, data);
}

EAPI Evas_Object_Box_Option *
evas_object_box_insert_at(Evas_Box *obj, Efl_Canvas_Object *child, unsigned int pos)
{
   return evas_obj_box_insert_at(obj, child, pos);
}

EAPI Eina_Bool
evas_object_box_remove(Evas_Box *obj, Efl_Canvas_Object *child)
{
   return evas_obj_box_remove(obj, child);
}

EAPI void
evas_object_box_layout_stack(Evas_Box *obj, Evas_Object_Box_Data *priv, void *data)
{
   evas_obj_box_layout_stack(obj, priv, data);
}

EAPI void
evas_object_box_layout_homogeneous_vertical(Evas_Box *obj, Evas_Object_Box_Data *priv, void *data)
{
   evas_obj_box_layout_homogeneous_vertical(obj, priv, data);
}

EAPI void
evas_object_box_layout_flow_horizontal(Evas_Box *obj, Evas_Object_Box_Data *priv, void *data)
{
   evas_obj_box_layout_flow_horizontal(obj, priv, data);
}
