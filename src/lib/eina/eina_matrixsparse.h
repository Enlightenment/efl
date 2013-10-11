/* EINA - EFL data type library
 * Copyright (C) 2009 Gustavo Sverzut Barbieri
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EINA_MATRIXSPARSE_H_
#define EINA_MATRIXSPARSE_H_

#include <stdlib.h>

#include "eina_config.h"

#include "eina_types.h"
#include "eina_iterator.h"
#include "eina_accessor.h"

/**
 * @addtogroup Eina_Matrixsparse_Group Sparse Matrix
 *
 * @brief These functions provide matrix sparse management.
 *
 * For more information, you can look at the @ref tutorial_matrixsparse_page.
 */

/**
 * @addtogroup Eina_Data_Types_Group Data Types
 *
 * @{
 */

/**
 * @addtogroup Eina_Containers_Group Containers
 *
 * @{
 */

/**
 * @defgroup Eina_Matrixsparse_Group Sparse Matrix
 *
 * @{
 */

/**
 * @typedef Eina_Matrixsparse
 * Type for a generic sparse matrix.
 */
typedef struct _Eina_Matrixsparse Eina_Matrixsparse;

/**
 * @typedef Eina_Matrixsparse_Row
 * Type for a generic sparse matrix row, opaque for users.
 */
typedef struct _Eina_Matrixsparse_Row Eina_Matrixsparse_Row;

/**
 * @typedef Eina_Matrixsparse_Cell
 * Type for a generic sparse matrix cell, opaque for users.
 */
typedef struct _Eina_Matrixsparse_Cell      Eina_Matrixsparse_Cell;

/* constructors and destructors */

/**
 * @brief Create a new Sparse Matrix.
 *
 * @param rows number of rows in matrix. Operations with rows greater than this
 *        value will fail.
 * @param cols number of columns in matrix. Operations with columns greater
 *        than this value will fail.
 * @param free_func used to delete cell data contents, used by
 *        eina_matrixsparse_free(), eina_matrixsparse_size_set(),
 *        eina_matrixsparse_row_idx_clear(),
 *        eina_matrixsparse_column_idx_clear(),
 *        eina_matrixsparse_cell_idx_clear() and possible others.
 * @param user_data given to @a free_func as first parameter.
 *
 * @return Newly allocated matrix or @c NULL if allocation failed.
 */
EAPI Eina_Matrixsparse *eina_matrixsparse_new(unsigned long rows,
                                              unsigned long cols,
                                              void (*free_func)(void *user_data,
                                                                void *cell_data),
                                              const void *user_data);

/**
 * @brief Free resources allocated to Sparse Matrix.
 *
 * @param m The Sparse Matrix instance to free, must @b not be @c NULL.
 */
EAPI void eina_matrixsparse_free(Eina_Matrixsparse *m);

/* size manipulation */

/**
 * @brief Get the current size of Sparse Matrix.
 *
 * The given parameters are guaranteed to be set if they're not @c NULL,
 * even if this function fails (ie: @a m is not a valid matrix instance).
 *
 * @param m the sparse matrix to operate on.
 * @param rows returns the number of rows, may be @c NULL. If @a m is invalid,
 *        returned value is zero, otherwise it's a positive integer.
 * @param cols returns the number of columns, may be @c NULL. If @a m is
 *        invalid, returned value is zero, otherwise it's a positive integer.
 */
EAPI void eina_matrixsparse_size_get(const Eina_Matrixsparse *m,
                                     unsigned long           *rows,
                                     unsigned long           *cols);

/**
 * @brief Resize the Sparse Matrix.
 *
 * This will resize the sparse matrix, possibly freeing cells on rows
 * and columns that will cease to exist.
 *
 * @param m the sparse matrix to operate on.
 * @param rows the new number of rows, must be greater than zero.
 * @param cols the new number of columns, must be greater than zero.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning cells, rows or columns are not reference counted and thus
 *     after this call any reference might be invalid if instance were
 *     freed.
 */
EAPI Eina_Bool eina_matrixsparse_size_set(Eina_Matrixsparse *m,
                                          unsigned long      rows,
                                          unsigned long      cols);

/* data getting */

/**
 * Get the cell reference inside Sparse Matrix.
 *
 * @param m the sparse matrix to operate on.
 * @param row the new number of row to clear.
 * @param col the new number of column to clear.
 * @param cell pointer to return cell reference, if any exists.
 *
 * @return @c 1 on success, @c 0 on failure. It is considered success if did not
 *     exist but index is inside matrix size, in this case @c *cell == NULL
 *
 * @see eina_matrixsparse_cell_data_get()
 * @see eina_matrixsparse_data_idx_get()
 */
EAPI Eina_Bool eina_matrixsparse_cell_idx_get(const Eina_Matrixsparse *m, unsigned long row, unsigned long col, Eina_Matrixsparse_Cell **cell);

/**
 * Get data associated with given cell reference.
 *
 * @param cell given cell reference, must @b not be @c NULL.
 *
 * @return data associated with given cell.
 *
 * @see eina_matrixsparse_cell_idx_get()
 * @see eina_matrixsparse_data_idx_get()
 */
EAPI void     *eina_matrixsparse_cell_data_get(const Eina_Matrixsparse_Cell *cell);

/**
 * Get data associated with given cell given its indexes.
 *
 * @param m the sparse matrix to operate on.
 * @param row the new number of row to clear.
 * @param col the new number of column to clear.
 *
 * @return Data associated with given cell or @c NULL if nothing is associated.
 *
 * @see eina_matrixsparse_cell_idx_get()
 * @see eina_matrixsparse_cell_data_get()
 */
EAPI void     *eina_matrixsparse_data_idx_get(const Eina_Matrixsparse *m, unsigned long row, unsigned long col);

/**
 * Get position (indexes) of the given cell.
 *
 * @param cell the cell reference, must @b not be @c NULL.
 * @param row where to store cell row number, may be @c NULL.
 * @param col where to store cell column number, may be @c NULL.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise (@c cell is @c NULL).
 */
EAPI Eina_Bool eina_matrixsparse_cell_position_get(const Eina_Matrixsparse_Cell *cell, unsigned long *row, unsigned long *col);

/* data setting */

/**
 * Change cell reference value without freeing the possibly existing old value.
 *
 * @param cell the cell reference, must @b not be @c NULL.
 * @param data new data to set.
 * @param p_old returns the old value intact (not freed).
 *
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise (@a cell is @c NULL).
 *
 * @see eina_matrixsparse_cell_data_set()
 * @see eina_matrixsparse_data_idx_replace()
 */
EAPI Eina_Bool eina_matrixsparse_cell_data_replace(Eina_Matrixsparse_Cell *cell, const void *data, void **p_old);

/**
 * Change cell value freeing the possibly existing old value.
 *
 * In contrast to eina_matrixsparse_cell_data_replace(), this function will
 * call @c free_func() on existing value.
 *
 * @param cell the cell reference, must @b not be @c NULL.
 * @param data new data to set.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise (@a cell is @c NULL).
 *
 * @see eina_matrixsparse_cell_data_replace()
 * @see eina_matrixsparse_data_idx_set()
 */
EAPI Eina_Bool eina_matrixsparse_cell_data_set(Eina_Matrixsparse_Cell *cell, const void *data);

/**
 * Change cell value without freeing the possibly existing old value, using
 * indexes.
 *
 * @param m the sparse matrix, must @b not be @c NULL.
 * @param row the row number to set the value.
 * @param col the column number to set the value.
 * @param data new data to set.
 * @param p_old returns the old value intact (not freed).
 *
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise (@a m is @c NULL, indexes are not valid).
 *
 * @see eina_matrixsparse_cell_data_replace()
 * @see eina_matrixsparse_data_idx_set()
 */
EAPI Eina_Bool eina_matrixsparse_data_idx_replace(Eina_Matrixsparse *m, unsigned long row, unsigned long col, const void *data, void **p_old);

/**
 * Change cell value freeing the possibly existing old value, using
 * indexes.
 *
 * In contrast to eina_matrixsparse_data_idx_replace(), this function will
 * call @c free_func() on existing value.
 *
 * @param m the sparse matrix, must @b not be @c NULL.
 * @param row the row number to set the value.
 * @param col the column number to set the value.
 * @param data new data to set.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise (@a m is @c NULL, indexes are not valid).
 *
 * @see eina_matrixsparse_cell_data_replace()
 */
EAPI Eina_Bool eina_matrixsparse_data_idx_set(Eina_Matrixsparse *m, unsigned long row, unsigned long col, const void *data);

/* data deleting */

/**
 * Clear (erase all cells) of row given its index.
 *
 * Existing cells will be cleared with @c free_func() given to
 * eina_matrixsparse_new().
 *
 * @param m the sparse matrix to operate on.
 * @param row the new number of row to clear.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure. It is considered success if row
 *     had no cells filled. Failure is asking for clear row outside
 *     matrix size.
 *
 * @warning cells, rows or columns are not reference counted and thus
 *     after this call any reference might be invalid if instance were
 *     freed.
 */
EAPI Eina_Bool eina_matrixsparse_row_idx_clear(Eina_Matrixsparse *m, unsigned long row);

/**
 * Clear (erase all cells) of column given its index.
 *
 * Existing cells will be cleared with @c free_func() given to
 * eina_matrixsparse_new().
 *
 * @param m the sparse matrix to operate on.
 * @param col the new number of column to clear.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure. It is considered success if column
 *     had no cells filled. Failure is asking for clear column outside
 *     matrix size.
 *
 * @warning cells, rows or columns are not reference counted and thus
 *     after this call any reference might be invalid if instance were
 *     freed.
 */
EAPI Eina_Bool eina_matrixsparse_column_idx_clear(Eina_Matrixsparse *m, unsigned long col);

/**
 * Clear (erase) cell given its indexes.
 *
 * Existing cell will be cleared with @c free_func() given to
 * eina_matrixsparse_new().
 *
 * @param m the sparse matrix to operate on.
 * @param row the new number of row to clear.
 * @param col the new number of column to clear.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure. It is considered success if did not
 *     exist but index is inside matrix size.
 *
 * @warning cells, rows or columns are not reference counted and thus
 *     after this call any reference might be invalid if instance were
 *     freed.
 *
 * @note This call might delete container column and row if this cell was the
 * last remainder.
 */
EAPI Eina_Bool eina_matrixsparse_cell_idx_clear(Eina_Matrixsparse *m, unsigned long row, unsigned long col);

/**
 * Clear (erase) cell given its reference.
 *
 * @param cell the cell reference, must @b not be @c NULL.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning cells, rows or columns are not reference counted and thus
 *     after this call any reference might be invalid if instance were
 *     freed.
 *
 * @note This call might delete container column and row if this cell was the
 * last remainder.
 */
EAPI Eina_Bool eina_matrixsparse_cell_clear(Eina_Matrixsparse_Cell *cell);

/* iterators */

/**
 * Creates a new iterator over existing matrix cells.
 *
 * This is a cheap walk, it will just report existing cells and holes
 * in the sparse matrix will be ignored. That means the reported
 * indexes will not be sequential.
 *
 * The iterator data will be the cell reference, one may query current
 * position with eina_matrixsparse_cell_position_get() and cell value
 * with eina_matrixsparse_cell_data_get().
 *
 * @param m The Sparse Matrix reference, must @b not be @c NULL.
 * @return A new iterator.
 *
 * @warning if the matrix structure changes then the iterator becomes
 *    invalid! That is, if you add or remove cells this iterator
 *    behavior is undefined and your program may crash!
 */
EAPI Eina_Iterator *eina_matrixsparse_iterator_new(const Eina_Matrixsparse *m);

/**
 * Creates a new iterator over all matrix cells.
 *
 * Unlike eina_matrixsparse_iterator_new() this one will report all
 * matrix cells, even those that are still empty (holes). These will
 * be reported as dummy cells that contains no data.
 *
 * Be aware that iterating a big matrix (1000x1000) will call your
 * function that number of times (1000000 times in that case) even if
 * your matrix have no elements at all!
 *
 * The iterator data will be the cell reference, one may query current
 * position with eina_matrixsparse_cell_position_get() and cell value
 * with eina_matrixsparse_cell_data_get(). If cell is empty then the
 * reference will be a dummy/placeholder, thus setting value with
 * eina_matrixsparse_cell_data_set() will leave pointer unreferenced.
 *
 * @param m The Sparse Matrix reference, must @b not be @c NULL.
 * @return A new iterator.
 *
 * @warning if the matrix structure changes then the iterator becomes
 *    invalid! That is, if you add or remove cells this iterator
 *    behavior is undefined and your program may crash!
 */
EAPI Eina_Iterator *eina_matrixsparse_iterator_complete_new(const Eina_Matrixsparse *m);

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#endif /* EINA_MATRIXSPARSE_H_ */
