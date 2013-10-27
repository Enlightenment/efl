#define ELM_OBJ_TABLE_CLASS elm_obj_table_class_get()

const Eo_Class *elm_obj_table_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_TABLE_BASE_ID;

enum
{
   ELM_OBJ_TABLE_SUB_ID_HOMOGENEOUS_SET,
   ELM_OBJ_TABLE_SUB_ID_HOMOGENEOUS_GET,
   ELM_OBJ_TABLE_SUB_ID_PADDING_SET,
   ELM_OBJ_TABLE_SUB_ID_PADDING_GET,
   ELM_OBJ_TABLE_SUB_ID_PACK,
   ELM_OBJ_TABLE_SUB_ID_UNPACK,
   ELM_OBJ_TABLE_SUB_ID_PACK_SET,
   ELM_OBJ_TABLE_SUB_ID_PACK_GET,
   ELM_OBJ_TABLE_SUB_ID_CLEAR,
   ELM_OBJ_TABLE_SUB_ID_CHILD_GET,
   ELM_OBJ_TABLE_SUB_ID_LAST
};

#define ELM_OBJ_TABLE_ID(sub_id) (ELM_OBJ_TABLE_BASE_ID + sub_id)


/**
 * @def elm_obj_table_homogeneous_set
 * @since 1.8
 *
 * @brief Set the homogeneous layout in the table
 *
 * @param[in] homogeneous
 *
 * @see elm_table_homogeneous_set
 *
 * @ingroup Table
 */
#define elm_obj_table_homogeneous_set(homogeneous) ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_HOMOGENEOUS_SET), EO_TYPECHECK(Eina_Bool, homogeneous)

/**
 * @def elm_obj_table_homogeneous_get
 * @since 1.8
 *
 * @brief Get the current table homogeneous mode.
 *
 * @param[out] ret
 *
 * @see elm_table_homogeneous_get
 *
 * @ingroup Table
 */
#define elm_obj_table_homogeneous_get(ret) ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_HOMOGENEOUS_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_table_padding_set
 * @since 1.8
 *
 * @brief Set padding between cells.
 *
 * @param[in] horizontal
 * @param[in] vertical
 *
 * @see elm_table_padding_set
 *
 * @ingroup Table
 */
#define elm_obj_table_padding_set(horizontal, vertical) ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_PADDING_SET), EO_TYPECHECK(Evas_Coord, horizontal), EO_TYPECHECK(Evas_Coord, vertical)

/**
 * @def elm_obj_table_padding_get
 * @since 1.8
 *
 * @brief Get padding between cells.
 *
 * @param[out] horizontal
 * @param[out] vertical
 *
 * @see elm_table_padding_get
 *
 * @ingroup Table
 */
#define elm_obj_table_padding_get(horizontal, vertical) ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_PADDING_GET), EO_TYPECHECK(Evas_Coord *, horizontal), EO_TYPECHECK(Evas_Coord *, vertical)

/**
 * @def elm_obj_table_pack
 * @since 1.8
 *
 * @brief Add a subobject on the table with the coordinates passed
 *
 * @param[in] subobj
 * @param[in] column
 * @param[in] row
 * @param[in] colspan
 * @param[in] rowspan
 *
 * @see elm_table_pack
 *
 * @ingroup Table
 */
#define elm_obj_table_pack(subobj, column, row, colspan, rowspan) ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_PACK), EO_TYPECHECK(Evas_Object *, subobj), EO_TYPECHECK(int, column), EO_TYPECHECK(int, row), EO_TYPECHECK(int, colspan), EO_TYPECHECK(int, rowspan)

/**
 * @def elm_obj_table_unpack
 * @since 1.8
 *
 * @brief Remove child from table.
 *
 * @param[in] subobj
 *
 * @see elm_table_unpack
 *
 * @ingroup Table
 */
#define elm_obj_table_unpack(subobj) ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_UNPACK), EO_TYPECHECK(Evas_Object *, subobj)

/**
 * @def elm_obj_table_pack_set
 * @since 1.8
 *
 * @brief Set the packing location of an existing child of the table
 *
 * @param[in] subobj
 * @param[in] column
 * @param[in] row
 * @param[in] colspan
 * @param[in] rowspan
 *
 * @see elm_table_pack_set
 *
 * @ingroup Table
 */
#define elm_obj_table_pack_set(subobj, column, row, colspan, rowspan) ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_PACK_SET), EO_TYPECHECK(Evas_Object *, subobj), EO_TYPECHECK(int, column), EO_TYPECHECK(int, row), EO_TYPECHECK(int, colspan), EO_TYPECHECK(int, rowspan)

/**
 * @def elm_obj_table_pack_get
 * @since 1.8
 *
 * @brief Get the packing location of an existing child of the table
 *
 * @param[in] subobj
 * @param[out] column
 * @param[out] row
 * @param[out] colspan
 * @param[out] rowspan
 *
 * @see elm_table_pack_get
 *
 * @ingroup Table
 */
#define elm_obj_table_pack_get(subobj, column, row, colspan, rowspan) ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_PACK_GET), EO_TYPECHECK(Evas_Object *, subobj), EO_TYPECHECK(int *, column), EO_TYPECHECK(int *, row), EO_TYPECHECK(int *, colspan), EO_TYPECHECK(int *, rowspan)

/**
 * @def elm_obj_table_clear
 * @since 1.8
 *
 * @brief Faster way to remove all child objects from a table object.
 *
 * @param[in] clear
 *
 * @see elm_table_clear
 *
 * @ingroup Table
 */
#define elm_obj_table_clear(clear) ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_CLEAR), EO_TYPECHECK(Eina_Bool, clear)

/**
 * @def elm_obj_table_child_get
 * @since 1.8
 *
 * @brief Get child object of table at given coordinates.
 *
 * @param[in] col column number of child object
 * @param[in] row row number of child object
 * @param[out] ret object at given col and row
 *
 * @ingroup Table
 */
#define elm_obj_table_child_get(col, row, ret) ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_CHILD_GET), EO_TYPECHECK(int, col), EO_TYPECHECK(int, row), EO_TYPECHECK(Evas_Object **, ret)
