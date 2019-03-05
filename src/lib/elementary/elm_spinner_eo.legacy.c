
EAPI void
elm_spinner_wrap_set(Elm_Spinner *obj, Eina_Bool wrap)
{
   elm_obj_spinner_wrap_set(obj, wrap);
}

EAPI Eina_Bool
elm_spinner_wrap_get(const Elm_Spinner *obj)
{
   return elm_obj_spinner_wrap_get(obj);
}

EAPI void
elm_spinner_interval_set(Elm_Spinner *obj, double interval)
{
   elm_obj_spinner_interval_set(obj, interval);
}

EAPI double
elm_spinner_interval_get(const Elm_Spinner *obj)
{
   return elm_obj_spinner_interval_get(obj);
}

EAPI void
elm_spinner_round_set(Elm_Spinner *obj, int rnd)
{
   elm_obj_spinner_round_set(obj, rnd);
}

EAPI int
elm_spinner_round_get(const Elm_Spinner *obj)
{
   return elm_obj_spinner_round_get(obj);
}

EAPI void
elm_spinner_editable_set(Elm_Spinner *obj, Eina_Bool editable)
{
   elm_obj_spinner_editable_set(obj, editable);
}

EAPI Eina_Bool
elm_spinner_editable_get(const Elm_Spinner *obj)
{
   return elm_obj_spinner_editable_get(obj);
}

EAPI void
elm_spinner_base_set(Elm_Spinner *obj, double base)
{
   elm_obj_spinner_base_set(obj, base);
}

EAPI double
elm_spinner_base_get(const Elm_Spinner *obj)
{
   return elm_obj_spinner_base_get(obj);
}

EAPI void
elm_spinner_label_format_set(Elm_Spinner *obj, const char *fmt)
{
   elm_obj_spinner_label_format_set(obj, fmt);
}

EAPI const char *
elm_spinner_label_format_get(const Elm_Spinner *obj)
{
   return elm_obj_spinner_label_format_get(obj);
}

EAPI void
elm_spinner_special_value_add(Elm_Spinner *obj, double value, const char *label)
{
   elm_obj_spinner_special_value_add(obj, value, label);
}
