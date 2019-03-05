
EAPI void
elm_textpath_circle_set(Efl_Ui_Textpath *obj, double x, double y, double radius, double start_angle, Efl_Ui_Textpath_Direction direction)
{
   efl_ui_textpath_circle_set(obj, x, y, radius, start_angle, direction);
}

EAPI void
elm_textpath_slice_number_set(Efl_Ui_Textpath *obj, int slice_no)
{
   efl_ui_textpath_slice_number_set(obj, slice_no);
}

EAPI int
elm_textpath_slice_number_get(const Efl_Ui_Textpath *obj)
{
   return efl_ui_textpath_slice_number_get(obj);
}

EAPI void
elm_textpath_ellipsis_set(Efl_Ui_Textpath *obj, Eina_Bool ellipsis)
{
   efl_ui_textpath_ellipsis_set(obj, ellipsis);
}

EAPI Eina_Bool
elm_textpath_ellipsis_get(const Efl_Ui_Textpath *obj)
{
   return efl_ui_textpath_ellipsis_get(obj);
}
