
EAPI void
elm_actionslider_indicator_pos_set(Elm_Actionslider *obj, Elm_Actionslider_Pos pos)
{
   elm_obj_actionslider_indicator_pos_set(obj, pos);
}

EAPI Elm_Actionslider_Pos
elm_actionslider_indicator_pos_get(const Elm_Actionslider *obj)
{
   return elm_obj_actionslider_indicator_pos_get(obj);
}

EAPI void
elm_actionslider_magnet_pos_set(Elm_Actionslider *obj, Elm_Actionslider_Pos pos)
{
   elm_obj_actionslider_magnet_pos_set(obj, pos);
}

EAPI Elm_Actionslider_Pos
elm_actionslider_magnet_pos_get(const Elm_Actionslider *obj)
{
   return elm_obj_actionslider_magnet_pos_get(obj);
}

EAPI void
elm_actionslider_enabled_pos_set(Elm_Actionslider *obj, Elm_Actionslider_Pos pos)
{
   elm_obj_actionslider_enabled_pos_set(obj, pos);
}

EAPI Elm_Actionslider_Pos
elm_actionslider_enabled_pos_get(const Elm_Actionslider *obj)
{
   return elm_obj_actionslider_enabled_pos_get(obj);
}

EAPI const char *
elm_actionslider_selected_label_get(const Elm_Actionslider *obj)
{
   return elm_obj_actionslider_selected_label_get(obj);
}
