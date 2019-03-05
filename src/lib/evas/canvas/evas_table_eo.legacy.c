
EAPI void
evas_object_table_homogeneous_set(Evas_Table *obj, Evas_Object_Table_Homogeneous_Mode homogeneous)
{
   evas_obj_table_homogeneous_set(obj, homogeneous);
}

EAPI Evas_Object_Table_Homogeneous_Mode
evas_object_table_homogeneous_get(const Evas_Table *obj)
{
   return evas_obj_table_homogeneous_get(obj);
}

EAPI void
evas_object_table_align_set(Evas_Table *obj, double horizontal, double vertical)
{
   evas_obj_table_align_set(obj, horizontal, vertical);
}

EAPI void
evas_object_table_align_get(const Evas_Table *obj, double *horizontal, double *vertical)
{
   evas_obj_table_align_get(obj, horizontal, vertical);
}

EAPI void
evas_object_table_padding_set(Evas_Table *obj, int horizontal, int vertical)
{
   evas_obj_table_padding_set(obj, horizontal, vertical);
}

EAPI void
evas_object_table_padding_get(const Evas_Table *obj, int *horizontal, int *vertical)
{
   evas_obj_table_padding_get(obj, horizontal, vertical);
}

EAPI void
evas_object_table_col_row_size_get(const Evas_Table *obj, int *cols, int *rows)
{
   evas_obj_table_col_row_size_get(obj, cols, rows);
}

EAPI Eina_List *
evas_object_table_children_get(const Evas_Table *obj)
{
   return evas_obj_table_children_get(obj);
}

EAPI Efl_Canvas_Object *
evas_object_table_child_get(const Evas_Table *obj, unsigned short col, unsigned short row)
{
   return evas_obj_table_child_get(obj, col, row);
}

EAPI void
evas_object_table_clear(Evas_Table *obj, Eina_Bool clear)
{
   evas_obj_table_clear(obj, clear);
}

EAPI Eina_Accessor *
evas_object_table_accessor_new(const Evas_Table *obj)
{
   return evas_obj_table_accessor_new(obj);
}

EAPI Eina_Iterator *
evas_object_table_iterator_new(const Evas_Table *obj)
{
   return evas_obj_table_iterator_new(obj);
}

EAPI Efl_Canvas_Object *
evas_object_table_add_to(Evas_Table *obj)
{
   return evas_obj_table_add_to(obj);
}

EAPI Eina_Bool
evas_object_table_pack_get(const Evas_Table *obj, Efl_Canvas_Object *child, unsigned short *col, unsigned short *row, unsigned short *colspan, unsigned short *rowspan)
{
   return evas_obj_table_pack_get(obj, child, col, row, colspan, rowspan);
}

EAPI Eina_Bool
evas_object_table_pack(Evas_Table *obj, Efl_Canvas_Object *child, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan)
{
   return evas_obj_table_pack(obj, child, col, row, colspan, rowspan);
}

EAPI Eina_Bool
evas_object_table_unpack(Evas_Table *obj, Efl_Canvas_Object *child)
{
   return evas_obj_table_unpack(obj, child);
}
