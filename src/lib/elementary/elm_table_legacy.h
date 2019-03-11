/**
 * @brief Add a new table to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Elm_Table
 */
EAPI Evas_Object *elm_table_add(Evas_Object *parent);

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
 * @ingroup Elm_Table
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
 * @ingroup Elm_Table
 */
EAPI void      elm_table_pack_get(Evas_Object *subobj, int *col, int *row, int *colspan, int *rowspan);

#include "elm_table_eo.legacy.h"