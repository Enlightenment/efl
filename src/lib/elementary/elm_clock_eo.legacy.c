
EAPI void
elm_clock_show_am_pm_set(Elm_Clock *obj, Eina_Bool am_pm)
{
   elm_obj_clock_show_am_pm_set(obj, am_pm);
}

EAPI Eina_Bool
elm_clock_show_am_pm_get(const Elm_Clock *obj)
{
   return elm_obj_clock_show_am_pm_get(obj);
}

EAPI void
elm_clock_first_interval_set(Elm_Clock *obj, double interval)
{
   elm_obj_clock_first_interval_set(obj, interval);
}

EAPI double
elm_clock_first_interval_get(const Elm_Clock *obj)
{
   return elm_obj_clock_first_interval_get(obj);
}

EAPI void
elm_clock_show_seconds_set(Elm_Clock *obj, Eina_Bool seconds)
{
   elm_obj_clock_show_seconds_set(obj, seconds);
}

EAPI Eina_Bool
elm_clock_show_seconds_get(const Elm_Clock *obj)
{
   return elm_obj_clock_show_seconds_get(obj);
}

EAPI void
elm_clock_edit_set(Elm_Clock *obj, Eina_Bool edit)
{
   elm_obj_clock_edit_set(obj, edit);
}

EAPI Eina_Bool
elm_clock_edit_get(const Elm_Clock *obj)
{
   return elm_obj_clock_edit_get(obj);
}

EAPI void
elm_clock_pause_set(Elm_Clock *obj, Eina_Bool paused)
{
   elm_obj_clock_pause_set(obj, paused);
}

EAPI Eina_Bool
elm_clock_pause_get(const Elm_Clock *obj)
{
   return elm_obj_clock_pause_get(obj);
}

EAPI void
elm_clock_time_set(Elm_Clock *obj, int hrs, int min, int sec)
{
   elm_obj_clock_time_set(obj, hrs, min, sec);
}

EAPI void
elm_clock_time_get(const Elm_Clock *obj, int *hrs, int *min, int *sec)
{
   elm_obj_clock_time_get(obj, hrs, min, sec);
}

EAPI void
elm_clock_edit_mode_set(Elm_Clock *obj, Elm_Clock_Edit_Mode digedit)
{
   elm_obj_clock_edit_mode_set(obj, digedit);
}

EAPI Elm_Clock_Edit_Mode
elm_clock_edit_mode_get(const Elm_Clock *obj)
{
   return elm_obj_clock_edit_mode_get(obj);
}
