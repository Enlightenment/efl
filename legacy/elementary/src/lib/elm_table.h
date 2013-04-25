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
 * elm_table_pack(table, table_content_object, column, row, colspan, rowspan);
 * elm_table_pack(table, table_content_object, next_column, next_row, colspan, rowspan);
 * elm_table_pack(table, table_content_object, other_column, other_row, colspan, rowspan);
 * @endcode
 *
 * The following are examples of how to use a table:
 * @li @ref tutorial_table_01
 * @li @ref tutorial_table_02
 *
 * @{
 */

#ifdef EFL_EO_API_SUPPORT
#include "elm_table_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_table_legacy.h"
#endif
/**
 * @}
 */
