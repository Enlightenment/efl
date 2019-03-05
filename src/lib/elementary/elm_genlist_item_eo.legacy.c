
EAPI Elm_Widget_Item *
elm_genlist_item_prev_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_prev_get(obj);
}

EAPI Elm_Widget_Item *
elm_genlist_item_next_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_next_get(obj);
}

EAPI Elm_Widget_Item *
elm_genlist_item_parent_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_parent_item_get(obj);
}

EAPI const Eina_List *
elm_genlist_item_subitems_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_subitems_get(obj);
}

EAPI void
elm_genlist_item_selected_set(Elm_Genlist_Item *obj, Eina_Bool selected)
{
   elm_obj_genlist_item_selected_set(obj, selected);
}

EAPI Eina_Bool
elm_genlist_item_selected_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_selected_get(obj);
}

EAPI void
elm_genlist_item_expanded_set(Elm_Genlist_Item *obj, Eina_Bool expanded)
{
   elm_obj_genlist_item_expanded_set(obj, expanded);
}

EAPI Eina_Bool
elm_genlist_item_expanded_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_expanded_get(obj);
}

EAPI int
elm_genlist_item_expanded_depth_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_expanded_depth_get(obj);
}

EAPI const Elm_Genlist_Item_Class *
elm_genlist_item_item_class_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_class_get(obj);
}

EAPI int
elm_genlist_item_index_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_index_get(obj);
}

EAPI const char *
elm_genlist_item_decorate_mode_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_decorate_mode_get(obj);
}

EAPI void
elm_genlist_item_flip_set(Elm_Genlist_Item *obj, Eina_Bool flip)
{
   elm_obj_genlist_item_flip_set(obj, flip);
}

EAPI Eina_Bool
elm_genlist_item_flip_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_flip_get(obj);
}

EAPI void
elm_genlist_item_select_mode_set(Elm_Genlist_Item *obj, Elm_Object_Select_Mode mode)
{
   elm_obj_genlist_item_select_mode_set(obj, mode);
}

EAPI Elm_Object_Select_Mode
elm_genlist_item_select_mode_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_select_mode_get(obj);
}

EAPI Elm_Genlist_Item_Type
elm_genlist_item_type_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_type_get(obj);
}

EAPI void
elm_genlist_item_pin_set(Elm_Genlist_Item *obj, Eina_Bool pin)
{
   elm_obj_genlist_item_pin_set(obj, pin);
}

EAPI Eina_Bool
elm_genlist_item_pin_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_pin_get(obj);
}

EAPI unsigned int
elm_genlist_item_subitems_count(Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_subitems_count(obj);
}

EAPI void
elm_genlist_item_subitems_clear(Elm_Genlist_Item *obj)
{
   elm_obj_genlist_item_subitems_clear(obj);
}

EAPI void
elm_genlist_item_promote(Elm_Genlist_Item *obj)
{
   elm_obj_genlist_item_promote(obj);
}

EAPI void
elm_genlist_item_demote(Elm_Genlist_Item *obj)
{
   elm_obj_genlist_item_demote(obj);
}

EAPI void
elm_genlist_item_show(Elm_Genlist_Item *obj, Elm_Genlist_Item_Scrollto_Type type)
{
   elm_obj_genlist_item_show(obj, type);
}

EAPI void
elm_genlist_item_bring_in(Elm_Genlist_Item *obj, Elm_Genlist_Item_Scrollto_Type type)
{
   elm_obj_genlist_item_bring_in(obj, type);
}

EAPI void
elm_genlist_item_all_contents_unset(Elm_Genlist_Item *obj, Eina_List **l)
{
   elm_obj_genlist_item_all_contents_unset(obj, l);
}

EAPI void
elm_genlist_item_update(Elm_Genlist_Item *obj)
{
   elm_obj_genlist_item_update(obj);
}

EAPI void
elm_genlist_item_fields_update(Elm_Genlist_Item *obj, const char *parts, Elm_Genlist_Item_Field_Type itf)
{
   elm_obj_genlist_item_fields_update(obj, parts, itf);
}

EAPI void
elm_genlist_item_item_class_update(Elm_Genlist_Item *obj, const Elm_Genlist_Item_Class *itc)
{
   elm_obj_genlist_item_class_update(obj, itc);
}

EAPI void
elm_genlist_item_decorate_mode_set(Elm_Genlist_Item *obj, const char *decorate_it_type, Eina_Bool decorate_it_set)
{
   elm_obj_genlist_item_decorate_mode_set(obj, decorate_it_type, decorate_it_set);
}
