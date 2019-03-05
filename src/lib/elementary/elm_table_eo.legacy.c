
EAPI void
elm_table_homogeneous_set(Elm_Table *obj, Eina_Bool homogeneous)
{
   elm_obj_table_homogeneous_set(obj, homogeneous);
}

EAPI Eina_Bool
elm_table_homogeneous_get(const Elm_Table *obj)
{
   return elm_obj_table_homogeneous_get(obj);
}

EAPI void
elm_table_padding_set(Elm_Table *obj, int horizontal, int vertical)
{
   elm_obj_table_padding_set(obj, horizontal, vertical);
}

EAPI void
elm_table_padding_get(const Elm_Table *obj, int *horizontal, int *vertical)
{
   elm_obj_table_padding_get(obj, horizontal, vertical);
}

EAPI void
elm_table_align_set(Elm_Table *obj, double horizontal, double vertical)
{
   elm_obj_table_align_set(obj, horizontal, vertical);
}

EAPI void
elm_table_align_get(const Elm_Table *obj, double *horizontal, double *vertical)
{
   elm_obj_table_align_get(obj, horizontal, vertical);
}

EAPI void
elm_table_clear(Elm_Table *obj, Eina_Bool clear)
{
   elm_obj_table_clear(obj, clear);
}

EAPI Efl_Canvas_Object *
elm_table_child_get(const Elm_Table *obj, int col, int row)
{
   return elm_obj_table_child_get(obj, col, row);
}

EAPI void
elm_table_unpack(Elm_Table *obj, Efl_Canvas_Object *subobj)
{
   elm_obj_table_unpack(obj, subobj);
}

EAPI void
elm_table_pack(Elm_Table *obj, Efl_Canvas_Object *subobj, int column, int row, int colspan, int rowspan)
{
   elm_obj_table_pack(obj, subobj, column, row, colspan, rowspan);
}
