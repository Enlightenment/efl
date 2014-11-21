/* EINA - EFL data type library
 * Copyright (C) 2009 Gustavo Sverzut Barbieri
 *
 * This library is a free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
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
 * @defgroup Eina_Matrixsparse_Group Sparse Matrix
 * @ingroup Eina_Containers_Group
 *
 * @brief This group discusses the functions to provide matrix sparse management.
 *
 * @{
 */

/**
 * @typedef Eina_Matrixsparse
 * @brief The structure type for a generic sparse matrix.
 */
typedef struct _Eina_Matrixsparse Eina_Matrixsparse;

/**
 * @typedef Eina_Matrixsparse_Row
 * @brief The structure type for a generic sparse matrix row, opaque for users.
 */
typedef struct _Eina_Matrixsparse_Row Eina_Matrixsparse_Row;

/**
 * @typedef Eina_Matrixsparse_Cell
 * @brief The structure type for a generic sparse matrix cell, opaque for users.
 */
typedef struct _Eina_Matrixsparse_Cell      Eina_Matrixsparse_Cell;

/* Constructors and destructors */

/**
 * @brief Creates a new Sparse Matrix.
 *
 * @since_tizen 2.3
 *
 * @param[in] rows The number of rows in the matrix \n
 *             Operations with rows greater than this
 *             value fail.
 * @param[in] cols The number of columns in the matrix \n
 *             Operations with columns greater
 *             than this value fail.
 * @param[in] free_func The function used to delete cell data contents, used by
 *                  eina_matrixsparse_free(), eina_matrixsparse_size_set(),
 *                  eina_matrixsparse_row_idx_clear(),
 *                  eina_matrixsparse_column_idx_clear(),
 *                  eina_matrixsparse_cell_idx_clear(), and other possible functions.
 * @param[in] user_data The data given to @a free_func as the first parameter
 *
 * @return The newly allocated matrix, otherwise @c NULL if allocation fails and eina_error
 *         is set
 */
EAPI Eina_Matrixsparse *eina_matrixsparse_new(unsigned long rows,
                                              unsigned long cols,
                                              void (*free_func)(void *user_data,
                                                                void *cell_data),
                                              const void *user_data);

/**
 * @brief Frees resources allocated to the Sparse Matrix.
 *
 * @since_tizen 2.3
 *
 * @param[in] m The Sparse Matrix instance to free, must @b not be @c NULL
 */
EAPI void eina_matrixsparse_free(Eina_Matrixsparse *m);

/* Size manipulation */

/**
 * @brief Gets the current size of the Sparse Matrix.
 *
 * @since_tizen 2.3
 *
 * @remarks The given parameters are guaranteed to be set if they're not @c NULL,
 *          even if this function fails (ie: @a m is not a valid matrix instance).
 *
 * @param[in] m The sparse matrix to operate on
 * @param[out] rows The number of rows, may be @c NULL \n 
 *             If @a m is invalid the returned value is zero, otherwise it's a positive integer.
 * @param[out] cols The number of columns, may be @c NULL \n
 *             If @a m is invalid the returned value is zero, otherwise it's a positive integer.
 */
EAPI void eina_matrixsparse_size_get(const Eina_Matrixsparse *m,
                                     unsigned long           *rows,
                                     unsigned long           *cols);

/**
 * @brief Resizes the Sparse Matrix.
 *
 * @details This resizes the sparse matrix, possibly freeing cells of rows
 *          and columns that cease to exist.
 *
 * @since_tizen 2.3
 *
 * @remarks The cells, rows, or columns are not reference counted and thus
 *          after this call any reference might be invalid if the instance is
 *          freed.
 *
 * @param[in] m The sparse matrix to operate on
 * @param[in] rows The new number of rows, must be greater than zero
 * @param[in] cols The new number of columns, must be greater than zero
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 */
EAPI Eina_Bool eina_matrixsparse_size_set(Eina_Matrixsparse *m,
                                          unsigned long      rows,
                                          unsigned long      cols);

/* Obtaining data */

/**
 * @brief Gets the cell reference inside the Sparse Matrix.
 *
 * @since_tizen 2.3
 *
 * @param[in] m The sparse matrix to operate on
 * @param[in] row The new row number to clear
 * @param[in] col The new column number to clear
 * @param[out] cell A pointer to return the cell reference, if it exists
 *
 * @return @c 1 on success, otherwise @c 0 on failure \n
 *         It is considered successful if it does not
 *         exist but its index is inside the matrix size, in this case @c *cell == NULL
 *
 * @see eina_matrixsparse_cell_data_get()
 * @see eina_matrixsparse_data_idx_get()
 */
EAPI Eina_Bool eina_matrixsparse_cell_idx_get(const Eina_Matrixsparse *m, unsigned long row, unsigned long col, Eina_Matrixsparse_Cell **cell);

/**
 * @brief Gets the data associated to the given cell reference.
 *
 * @param[in] cell The given cell reference, must @b not be @c NULL
 *
 * @return The data associated to the given cell
 *
 * @see eina_matrixsparse_cell_idx_get()
 * @see eina_matrixsparse_data_idx_get()
 */
EAPI void     *eina_matrixsparse_cell_data_get(const Eina_Matrixsparse_Cell *cell);

/**
 * @brief Gets the data associated to the given cell given that its indexes are provided.
 *
 * @since_tizen 2.3
 *
 * @param[in] m The sparse matrix to operate on
 * @param[in] row The new row number to clear
 * @param[in] col The new column number to clear
 *
 * @return The data associated to the given cell, otherwise @c NULL if nothing is associated
 *
 * @see eina_matrixsparse_cell_idx_get()
 * @see eina_matrixsparse_cell_data_get()
 */
EAPI void     *eina_matrixsparse_data_idx_get(const Eina_Matrixsparse *m, unsigned long row, unsigned long col);

/**
 * @brief Gets the position (indexes) of the given cell.
 *
 * @since_tizen 2.3
 *
 * @param[in] cell The cell reference, must @b not be @c NULL
 * @param[out] row The location to store the cell row number, may be @c NULL
 * @param[out] col The location to store the column number, may be @c NULL
 *
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE (@c cell is @c NULL)
 */
EAPI Eina_Bool eina_matrixsparse_cell_position_get(const Eina_Matrixsparse_Cell *cell, unsigned long *row, unsigned long *col);

/* Setting data */

/**
 * @brief Changes the cell reference value without freeing a possible existing old value.
 *
 * @since_tizen 2.3
 *
 * @param[in] cell The cell reference, must @b not be @c NULL
 * @param[in] data The new data to set
 * @param[out] p_old The old value that is intact (not freed)
 *
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE (@a cell is @c NULL)
 *
 * @see eina_matrixsparse_cell_data_set()
 * @see eina_matrixsparse_data_idx_replace()
 */
EAPI Eina_Bool eina_matrixsparse_cell_data_replace(Eina_Matrixsparse_Cell *cell, const void *data, void **p_old);

/**
 * @brief Changes the cell value by freeing a possible existing old value.
 *
 * @since_tizen 2.3
 *
 * @remarks In contrast to eina_matrixsparse_cell_data_replace(), this function
 *          calls @c free_func() on the existing value.
 *
 * @param[in] cell The cell reference, must @b not be @c NULL
 * @param[in] data The new data to set
 *
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE (@a cell is @c NULL).
 *
 * @see eina_matrixsparse_cell_data_replace()
 * @see eina_matrixsparse_data_idx_set()
 */
EAPI Eina_Bool eina_matrixsparse_cell_data_set(Eina_Matrixsparse_Cell *cell, const void *data);

/**
 * @brief Changes the cell value without freeing a possible existing old value, using
 *        indexes.
 *
 * @since_tizen 2.3
 *
 * @param[in] m The sparse matrix, must @b not be @c NULL
 * @param[in] row The row number to set the value of
 * @param[in] col The column number to set the value of
 * @param[in] data The new data to set
 * @param[out] p_old The old value that is intact (not freed)
 *
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE (@a m is @c NULL, indexes are not valid)
 *
 * @see eina_matrixsparse_cell_data_replace()
 * @see eina_matrixsparse_data_idx_set()
 */
EAPI Eina_Bool eina_matrixsparse_data_idx_replace(Eina_Matrixsparse *m, unsigned long row, unsigned long col, const void *data, void **p_old);

/**
 * @brief Changes the cell value by freeing a possible existing old value, using
 *        indexes.
 *
 * @since_tizen 2.3
 *
 * @remarks In contrast to eina_matrixsparse_data_idx_replace(), this function
 *          calls @c free_func() on the existing value.
 *
 * @param[in] m The sparse matrix, must @b not be @c NULL
 * @param[in] row The row number to set the value of
 * @param[in] col The column number to set the value of
 * @param[in] data The new data to set
 *
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE (@a m is @c NULL, indexes are not valid)
 *
 * @see eina_matrixsparse_cell_data_replace()
 */
EAPI Eina_Bool eina_matrixsparse_data_idx_set(Eina_Matrixsparse *m, unsigned long row, unsigned long col, const void *data);

/* Deleting data */

/**
 * @brief Clears (erases) all the cells of the row given that its index is provided.
 *
 * @since_tizen 2.3
 *
 * @remarks Existing cells are cleared with @c free_func() given to
 *          eina_matrixsparse_new().
 *
 * @remarks The cells, rows, or columns are not reference counted and thus
 *          after this call any reference might be invalid if an instance is
 *          freed.
 *
 * @param[in] m The sparse matrix to operate on
 * @param[in] row The new row number to clear
 *
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure \n
 *         It is considered successful if the row has no filled cells \n
 *         Failure is asking for a clear row outside the matrix size.
 *
 */
EAPI Eina_Bool eina_matrixsparse_row_idx_clear(Eina_Matrixsparse *m, unsigned long row);

/**
 * @brief Clears (erases) all the cells of the column given that its index is provided.
 *
 * @since_tizen 2.3
 *
 * @remarks Existing cells are cleared with @c free_func() given to
 *          eina_matrixsparse_new().
 *
 * @remarks The cells, rows, or columns are not reference counted and thus
 *          after this call any reference might be invalid if an instance is
 *          freed.
 *
 * @param[in] m The sparse matrix to operate on
 * @param[in] col The new column number to clear
 *
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure \n
 *         It is considered successful if the column has no filled cells \n
 *         Failure is asking for a clear column outside the matrix size.
 *
 */
EAPI Eina_Bool eina_matrixsparse_column_idx_clear(Eina_Matrixsparse *m, unsigned long col);

/**
 * @brief Clears (erases) a cell given that its indexes are provided.
 *
 * @since_tizen 2.3
 *
 * @remarks Existing cells are cleared with @c free_func() given to
 *          eina_matrixsparse_new().
 *
 * @remarks The cells, rows, or columns are not reference counted and thus
 *          after this call any reference might be invalid if an instance is
 *          freed.
 *
 * @remarks This call might delete a container column and row if this cell is the
 *          last remainder.
 *
 * @param[in] m The sparse matrix to operate on
 * @param[in] row The new row number to clear
 * @param[in] col The new column number to clear
 *
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure \n
 *         It is considered successful if it does not exist but its index is inside the matrix size.
 *
 */
EAPI Eina_Bool eina_matrixsparse_cell_idx_clear(Eina_Matrixsparse *m, unsigned long row, unsigned long col);

/**
 * @brief Clears (erases) a cell given that its referenceis provided.
 *
 * @since_tizen 2.3
 *
 * @remarks The cells, rows, or columns are not reference counted and thus
 *          after this call any reference might be invalid if an instance is
 *          freed.
 *
 * @remarks This call might delete a container column and row if this cell is the
 *          last remainder.
 *
 * @param[in] cell The cell reference, must @b not be @c NULL
 *
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 */
EAPI Eina_Bool eina_matrixsparse_cell_clear(Eina_Matrixsparse_Cell *cell);

/* Iterators */

/**
 * @brief Creates a new iterator over existing matrix cells.
 *
 * @since_tizen 2.3
 *
 * @remarks This is a cheap walk, it just reports existing cells and holes
 *          in the sparse matrix are ignored. That means the reported
 *          indexes are not sequential.
 *
 * @remarks The iterator data is the cell reference, one may query the current
 *          position with eina_matrixsparse_cell_position_get() and cell value
 *          with eina_matrixsparse_cell_data_get().
 *
 * @remarks If the matrix structure changes then the iterator becomes
 *          invalid. That is, if you add or remove cells this iterator's
 *          behavior is undefined and your program may crash.
 *
 * @param[in] m The Sparse Matrix reference, must @b not be @c NULL
 * @return A new iterator
 *
 */
EAPI Eina_Iterator *eina_matrixsparse_iterator_new(const Eina_Matrixsparse *m);

/**
 * @brief Creates a new iterator over all the matrix cells.
 *
 * @since_tizen 2.3
 *
 * @remarks Unlike eina_matrixsparse_iterator_new(), this one reports all
 *          the matrix cells, even those that are still empty (holes). These are
 *          reported as dummy cells that contain no data.
 *
 * @remarks Be aware that iterating a big matrix (1000x1000) calls your
 *          function that number of times (1000000 times in that case) even if
 *          your matrix have no elements at all.
 *
 * @remarks The iterator data is the cell reference, one may query the current
 *          position with eina_matrixsparse_cell_position_get() and cell value
 *          with eina_matrixsparse_cell_data_get(). If the cell is empty then the
 *          reference is a dummy/placeholder, thus setting a value with
 *          eina_matrixsparse_cell_data_set() leaves the pointer unreferenced.
 *
 * @remarks If the matrix structure changes then the iterator becomes
 *          invalid. That is, if you add or remove cells this iterator's
 *          behavior is undefined and your program may crash.
 *
 * @param[in] m The Sparse Matrix reference, must @b not be @c NULL
 * @return A new iterator
 *
 */
EAPI Eina_Iterator *eina_matrixsparse_iterator_complete_new(const Eina_Matrixsparse *m);

/**
 * @}
 */

#endif /* EINA_MATRIXSPARSE_H_ */
