/* EINA - EFL data type library
 * Copyright (C) 20011 Cedric Bail
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

#ifndef EINA_REFCOUNT_H_
#define EINA_REFCOUNT_H_

/**
 * @addtogroup Eina_Refcount References counting
 *
 * @brief Small macro that simplify references counting.
 *
 * References counting is not a difficult task, but you must
 * handle it correctly every where, and that the issue. This
 * set of macro do provide helper that will force to use the
 * correct code in most case and reduce the bug likeliness.
 * Of course this without affecting speed !
 */

/**
 * @addtogroup Eina_Data_Types_Group Data Types
 *
 * @{
 */

/**
 * @defgroup Eina_Refcount References counting
 *
 * @{
 */

/**
 * @typedef Eina_Refcount
 * Inlined references counting type.
 */
typedef int Eina_Refcount;

/** Used for declaring a reference counting member in a struct */
#define EINA_REFCOUNT Eina_Refcount __refcount

/** Used just after allocating a object */
#define EINA_REFCOUNT_INIT(Variable) (Variable)->__refcount = 1

/** Used when using referring to an object one more time */
#define EINA_REFCOUNT_REF(Variable) (Variable)->__refcount++

/** Used when removing a reference to an object. The code just after will automatically be called when necessary */
#define EINA_REFCOUNT_UNREF(Variable)		     \
  if (--((Variable)->__refcount) == 0)

/** Get refcounting value */
#define EINA_REFCOUNT_GET(Variable) (Variable)->__refcount

/**
 * @}
 */

/**
 * @}
 */

#endif /* EINA_REFCOUNT_H_ */
