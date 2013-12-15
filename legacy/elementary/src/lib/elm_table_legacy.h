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
 * @param col Column number
 * @param row Row number
 * @param colspan colspan
 * @param rowspan rowspan
 *
 * @note All positioning inside the table is relative to rows and columns, so
 * a value of 0 for x and y, means the top left cell of the table, and a
 * value of 1 for w and h means @p subobj only takes that 1 cell.
 *
 * Note that columns and rows only guarantee 16bit unsigned values at best.
 * That means that col + colspan AND row + rowspan must fit inside 16bit
 * unsigned values cleanly. You will be warned once values exceed 15bit
 * storage, and attempting to use values not able to fit in 16bits will
 * result in failure.
 *
 * @ingroup Table
 */
EAPI void      elm_table_pack(Evas_Object *obj, Evas_Object *subobj, int col, int row, int colspan, int rowspan);

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
 * @brief Set the packing location of an existing child of the table
 *
 * @param subobj The subobject to be modified in the table
 * @param col Column number
 * @param row Row number
 * @param colspan colspan
 * @param rowspan rowspan
 *
 * Modifies the position of an object already in the table.
 *
 * @note All positioning inside the table is relative to rows and columns, so
 * a value of 0 for col and row, means the top left cell of the table, and a
 * value of 1 for colspan and rowspan means @p subobj only takes that 1 cell.
 *
 * @ingroup Table
 */
EAPI void      elm_table_pack_set(Evas_Object *subobj, int col, int row, int colspan, int rowspan);

/**
 * @brief Get the packing location of an existing child of the table
 *
 * @param subobj The subobject to be modified in the table
 * @param col Column number
 * @param row Row number
 * @param colspan colspan
 * @param rowspan rowspan
 *
 * @see elm_table_pack_set()
 *
 * @ingroup Table
 */
EAPI void      elm_table_pack_get(Evas_Object *subobj, int *col, int *row, int *colspan, int *rowspan);

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
 * @brief Get child object of table at given coordinates.
 *
 * @param obj The table object
 * @param col Column number of child object
 * @param row Row number of child object
 *
 * @return Child of object if find if not return NULL.
 */
EAPI Evas_Object *elm_table_child_get(const Evas_Object *obj, int col, int row);
