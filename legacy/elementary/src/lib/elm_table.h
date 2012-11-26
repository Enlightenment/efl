/**
 * @defgroup Table Table
 * @ingroup Elementary
 *
 * @image html table_inheritance_tree.png
 * @image latex table_inheritance_tree.eps
 *
 * A container widget to arrange other widgets in a table where items can
 * span multiple columns or rows - even overlap (and then be raised or
 * lowered accordingly to adjust stacking if they do overlap).
 *
 * The row and column count is not fixed. The table widget adjusts itself when
 * subobjects are added to it dynamically.
 *
 * The most common way to use a table is:
 * @code
 * table = elm_table_add(win);
 * evas_object_show(table);
 * elm_table_padding_set(table, space_between_columns, space_between_rows);
 * elm_table_pack(table, table_content_object, x_coord, y_coord, colspan, rowspan);
 * elm_table_pack(table, table_content_object, next_x_coord, next_y_coord, colspan, rowspan);
 * elm_table_pack(table, table_content_object, other_x_coord, other_y_coord, colspan, rowspan);
 * @endcode
 *
 * The following are examples of how to use a table:
 * @li @ref tutorial_table_01
 * @li @ref tutorial_table_02
 *
 * @{
 */

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
 */
#define elm_obj_table_padding_get(horizontal, vertical) ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_PADDING_GET), EO_TYPECHECK(Evas_Coord *, horizontal), EO_TYPECHECK(Evas_Coord *, vertical)

/**
 * @def elm_obj_table_pack
 * @since 1.8
 *
 * @brief Add a subobject on the table with the coordinates passed
 *
 * @param[in] subobj
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 *
 * @see elm_table_pack
 */
#define elm_obj_table_pack(subobj, x, y, w, h) ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_PACK), EO_TYPECHECK(Evas_Object *, subobj), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def elm_obj_table_unpack
 * @since 1.8
 *
 * @brief Remove child from table.
 *
 * @param[in] subobj
 *
 * @see elm_table_unpack
 */
#define elm_obj_table_unpack(subobj) ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_UNPACK), EO_TYPECHECK(Evas_Object *, subobj)

/**
 * @def elm_obj_table_pack_set
 * @since 1.8
 *
 * @brief Set the packing location of an existing child of the table
 *
 * @param[in] subobj
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 *
 * @see elm_table_pack_set
 */
#define elm_obj_table_pack_set(subobj, x, y, w, h) ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_PACK_SET), EO_TYPECHECK(Evas_Object *, subobj), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def elm_obj_table_pack_get
 * @since 1.8
 *
 * @brief Get the packing location of an existing child of the table
 *
 * @param[in] subobj
 * @param[out] x
 * @param[out] y
 * @param[out] w
 * @param[out] h
 *
 * @see elm_table_pack_get
 */
#define elm_obj_table_pack_get(subobj, x, y, w, h) ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_PACK_GET), EO_TYPECHECK(Evas_Object *, subobj), EO_TYPECHECK(int *, x), EO_TYPECHECK(int *, y), EO_TYPECHECK(int *, w), EO_TYPECHECK(int *, h)

/**
 * @def elm_obj_table_clear
 * @since 1.8
 *
 * @brief Faster way to remove all child objects from a table object.
 *
 * @param[in] clear
 *
 * @see elm_table_clear
 */
#define elm_obj_table_clear(clear) ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_CLEAR), EO_TYPECHECK(Eina_Bool, clear)


/**
 * @brief Add a new table to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Table
 */
EAPI Evas_Object *elm_table_add(Evas_Object *parent);

/**
 * @brief Set the homogeneous layout in the table
 *
 * @param obj The layout object
 * @param homogeneous A boolean to set if the layout is homogeneous in the
 * table (EINA_TRUE = homogeneous,  EINA_FALSE = no homogeneous)
 *
 * @ingroup Table
 */
EAPI void      elm_table_homogeneous_set(Evas_Object *obj, Eina_Bool homogeneous);

/**
 * @brief Get the current table homogeneous mode.
 *
 * @param obj The table object
 * @return A boolean to indicating if the layout is homogeneous in the table
 * (EINA_TRUE = homogeneous,  EINA_FALSE = no homogeneous)
 *
 * @ingroup Table
 */
EAPI Eina_Bool elm_table_homogeneous_get(const Evas_Object *obj);

/**
 * @brief Set padding between cells.
 *
 * @param obj The layout object.
 * @param horizontal set the horizontal padding.
 * @param vertical set the vertical padding.
 *
 * Default value is 0.
 *
 * @ingroup Table
 */
EAPI void      elm_table_padding_set(Evas_Object *obj, Evas_Coord horizontal, Evas_Coord vertical);

/**
 * @brief Get padding between cells.
 *
 * @param obj The layout object.
 * @param horizontal set the horizontal padding.
 * @param vertical set the vertical padding.
 *
 * @ingroup Table
 */
EAPI void      elm_table_padding_get(const Evas_Object *obj, Evas_Coord *horizontal, Evas_Coord *vertical);

/**
 * @brief Add a subobject on the table with the coordinates passed
 *
 * @param obj The table object
 * @param subobj The subobject to be added to the table
 * @param x Row number
 * @param y Column number
 * @param w colspan
 * @param h rowspan
 *
 * @note All positioning inside the table is relative to rows and columns, so
 * a value of 0 for x and y, means the top left cell of the table, and a
 * value of 1 for w and h means @p subobj only takes that 1 cell.
 *
 * @ingroup Table
 */
EAPI void      elm_table_pack(Evas_Object *obj, Evas_Object *subobj, int x, int y, int w, int h);

/**
 * @brief Remove child from table.
 *
 * @param obj The table object
 * @param subobj The subobject
 *
 * @ingroup Table
 */
EAPI void      elm_table_unpack(Evas_Object *obj, Evas_Object *subobj);

/**
 * @brief Faster way to remove all child objects from a table object.
 *
 * @param obj The table object
 * @param clear If true, will delete children, else just remove from table.
 *
 * @ingroup Table
 */
EAPI void      elm_table_clear(Evas_Object *obj, Eina_Bool clear);

/**
 * @brief Set the packing location of an existing child of the table
 *
 * @param subobj The subobject to be modified in the table
 * @param x Row number
 * @param y Column number
 * @param w rowspan
 * @param h colspan
 *
 * Modifies the position of an object already in the table.
 *
 * @note All positioning inside the table is relative to rows and columns, so
 * a value of 0 for x and y, means the top left cell of the table, and a
 * value of 1 for w and h means @p subobj only takes that 1 cell.
 *
 * @ingroup Table
 */
EAPI void      elm_table_pack_set(Evas_Object *subobj, int x, int y, int w, int h);

/**
 * @brief Get the packing location of an existing child of the table
 *
 * @param subobj The subobject to be modified in the table
 * @param x Row number
 * @param y Column number
 * @param w rowspan
 * @param h colspan
 *
 * @see elm_table_pack_set()
 *
 * @ingroup Table
 */
EAPI void      elm_table_pack_get(Evas_Object *subobj, int *x, int *y, int *w, int *h);

/**
 * @}
 */
