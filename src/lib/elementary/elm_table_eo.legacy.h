#ifndef _ELM_TABLE_EO_LEGACY_H_
#define _ELM_TABLE_EO_LEGACY_H_

#ifndef _ELM_TABLE_EO_CLASS_TYPE
#define _ELM_TABLE_EO_CLASS_TYPE

typedef Eo Elm_Table;

#endif

#ifndef _ELM_TABLE_EO_TYPES
#define _ELM_TABLE_EO_TYPES


#endif

/**
 * @brief Control the homogenous state in a table.
 *
 * @param[in] obj The object.
 * @param[in] homogeneous A boolean to set if the layout is homogeneous in the
 * table.
 *
 * @ingroup Elm_Table_Group
 */
EAPI void elm_table_homogeneous_set(Elm_Table *obj, Eina_Bool homogeneous);

/**
 * @brief Control the homogenous state in a table.
 *
 * @param[in] obj The object.
 *
 * @return A boolean to set if the layout is homogeneous in the table.
 *
 * @ingroup Elm_Table_Group
 */
EAPI Eina_Bool elm_table_homogeneous_get(const Elm_Table *obj);

/**
 * @brief Set padding between cells. Default value is 0.
 *
 * @param[in] obj The object.
 * @param[in] horizontal The horizontal padding.
 * @param[in] vertical The vertical padding.
 *
 * @ingroup Elm_Table_Group
 */
EAPI void elm_table_padding_set(Elm_Table *obj, int horizontal, int vertical);

/**
 * @brief Get padding between cells.
 *
 * @param[in] obj The object.
 * @param[out] horizontal The horizontal padding.
 * @param[out] vertical The vertical padding.
 *
 * @ingroup Elm_Table_Group
 */
EAPI void elm_table_padding_get(const Elm_Table *obj, int *horizontal, int *vertical);

/**
 * @brief Set alignment of table. Default value is 0.5.
 *
 * @param[in] obj The object.
 * @param[in] horizontal The horizontal alignment.
 * @param[in] vertical The vertical alignment.
 *
 * @since 1.13
 *
 * @ingroup Elm_Table_Group
 */
EAPI void elm_table_align_set(Elm_Table *obj, double horizontal, double vertical);

/**
 * @brief Get alignment of table.
 *
 * @param[in] obj The object.
 * @param[out] horizontal The horizontal alignment.
 * @param[out] vertical The vertical alignment.
 *
 * @since 1.13
 *
 * @ingroup Elm_Table_Group
 */
EAPI void elm_table_align_get(const Elm_Table *obj, double *horizontal, double *vertical);

/**
 * @brief Faster way to remove all child objects from a table object.
 *
 * @param[in] obj The object.
 * @param[in] clear If @c true, will delete children, else just remove from
 * table.
 *
 * @ingroup Elm_Table_Group
 */
EAPI void elm_table_clear(Elm_Table *obj, Eina_Bool clear);

/**
 * @brief Get child object of table at given coordinates.
 *
 * @param[in] obj The object.
 * @param[in] col Column number of child object.
 * @param[in] row Row number of child object.
 *
 * @return Child of object if find if not return @c null.
 *
 * @ingroup Elm_Table_Group
 */
EAPI Efl_Canvas_Object *elm_table_child_get(const Elm_Table *obj, int col, int row);



/**
 * @brief Remove child from table.
 *
 * @param[in] obj The object.
 * @param[in] subobj The subobject.
 *
 * @ingroup Elm_Table_Group
 */
EAPI void elm_table_unpack(Elm_Table *obj, Efl_Canvas_Object *subobj);

/**
 * @brief Add a subobject on the table with the coordinates passed.
 *
 * @note All positioning inside the table is relative to rows and columns, so a
 * value of 0 for x and y, means the top left cell of the table, and a value of
 * 1 for w and h means @c subobj only takes that 1 cell.
 *
 * @note Columns and rows only guarantee 16bit unsigned values at best. That
 * means that col + colspan AND row + rowspan must fit inside 16bit unsigned
 * values cleanly. You will be warned once values exceed 15bit storage, and
 * attempting to use values not able to fit in 16bits will result in failure.
 *
 * @param[in] obj The object.
 * @param[in] subobj The subobject to be added to the table.
 * @param[in] column Column number.
 * @param[in] row Row number.
 * @param[in] colspan Columns span.
 * @param[in] rowspan Rows span.
 *
 * @ingroup Elm_Table_Group
 */
EAPI void elm_table_pack(Elm_Table *obj, Efl_Canvas_Object *subobj, int column, int row, int colspan, int rowspan);

#endif
