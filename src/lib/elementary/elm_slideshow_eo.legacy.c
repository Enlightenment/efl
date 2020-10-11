
ELM_API void
elm_slideshow_cache_after_set(Elm_Slideshow *obj, int count)
{
   elm_obj_slideshow_cache_after_set(obj, count);
}

ELM_API int
elm_slideshow_cache_after_get(const Elm_Slideshow *obj)
{
   return elm_obj_slideshow_cache_after_get(obj);
}

ELM_API void
elm_slideshow_cache_before_set(Elm_Slideshow *obj, int count)
{
   elm_obj_slideshow_cache_before_set(obj, count);
}

ELM_API int
elm_slideshow_cache_before_get(const Elm_Slideshow *obj)
{
   return elm_obj_slideshow_cache_before_get(obj);
}

ELM_API void
elm_slideshow_layout_set(Elm_Slideshow *obj, const char *layout)
{
   elm_obj_slideshow_layout_set(obj, layout);
}

ELM_API const char *
elm_slideshow_layout_get(const Elm_Slideshow *obj)
{
   return elm_obj_slideshow_layout_get(obj);
}

ELM_API void
elm_slideshow_transition_set(Elm_Slideshow *obj, const char *transition)
{
   elm_obj_slideshow_transition_set(obj, transition);
}

ELM_API const char *
elm_slideshow_transition_get(const Elm_Slideshow *obj)
{
   return elm_obj_slideshow_transition_get(obj);
}

ELM_API void
elm_slideshow_loop_set(Elm_Slideshow *obj, Eina_Bool loop)
{
   elm_obj_slideshow_items_loop_set(obj, loop);
}

ELM_API Eina_Bool
elm_slideshow_loop_get(const Elm_Slideshow *obj)
{
   return elm_obj_slideshow_items_loop_get(obj);
}

ELM_API void
elm_slideshow_timeout_set(Elm_Slideshow *obj, double timeout)
{
   elm_obj_slideshow_timeout_set(obj, timeout);
}

ELM_API double
elm_slideshow_timeout_get(const Elm_Slideshow *obj)
{
   return elm_obj_slideshow_timeout_get(obj);
}

ELM_API const Eina_List *
elm_slideshow_items_get(const Elm_Slideshow *obj)
{
   return elm_obj_slideshow_items_get(obj);
}

ELM_API const Eina_List *
elm_slideshow_transitions_get(const Elm_Slideshow *obj)
{
   return elm_obj_slideshow_transitions_get(obj);
}

ELM_API unsigned int
elm_slideshow_count_get(const Elm_Slideshow *obj)
{
   return elm_obj_slideshow_count_get(obj);
}

ELM_API Elm_Widget_Item *
elm_slideshow_item_current_get(const Elm_Slideshow *obj)
{
   return elm_obj_slideshow_item_current_get(obj);
}

ELM_API const Eina_List *
elm_slideshow_layouts_get(const Elm_Slideshow *obj)
{
   return elm_obj_slideshow_layouts_get(obj);
}

ELM_API void
elm_slideshow_previous(Elm_Slideshow *obj)
{
   elm_obj_slideshow_previous(obj);
}

ELM_API Elm_Widget_Item *
elm_slideshow_item_nth_get(const Elm_Slideshow *obj, unsigned int nth)
{
   return elm_obj_slideshow_item_nth_get(obj, nth);
}

ELM_API void
elm_slideshow_next(Elm_Slideshow *obj)
{
   elm_obj_slideshow_next(obj);
}

ELM_API void
elm_slideshow_clear(Elm_Slideshow *obj)
{
   elm_obj_slideshow_clear(obj);
}

ELM_API Elm_Widget_Item *
elm_slideshow_item_add(Elm_Slideshow *obj, const Elm_Slideshow_Item_Class *itc, const void *data)
{
   return elm_obj_slideshow_item_add(obj, itc, data);
}

ELM_API Elm_Widget_Item *
elm_slideshow_item_sorted_insert(Elm_Slideshow *obj, const Elm_Slideshow_Item_Class *itc, const void *data, Eina_Compare_Cb func)
{
   return elm_obj_slideshow_item_sorted_insert(obj, itc, data, func);
}
