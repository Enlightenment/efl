/* EINA - EFL data type library
 * Copyright (C) 20011 Cedric Bail
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

#ifndef EINA_REFCOUNT_H_
#define EINA_REFCOUNT_H_

/**
 * @internal
 * @defgroup Eina_Refcount References counting
 * @ingroup Eina_Data_Types_Group
 *
 * @brief This group discusses the functions of the small macro that simplifies references counting.
 *
 * References counting is not a difficult task, but you must
 * handle it correctly every where, and that is the issue. This
 * set of macros do provide a helper that forces to use the
 * correct code in most cases and reduces the likeliness of a bug.
 * Of course, this is achieved without affecting the speed.
 *
 * @{
 */

/**
 * @typedef Eina_Refcount
 * @brief The integer type containing inlined references counting type.
 */
typedef int Eina_Refcount;

/** The definition used for declaring a reference counting member in a struct */
#define EINA_REFCOUNT Eina_Refcount __refcount

/** The definition used just after allocating an object */
#define EINA_REFCOUNT_INIT(Variable) (Variable)->__refcount = 1

/** The definition used when referring to an object one more time */
#define EINA_REFCOUNT_REF(Variable) (Variable)->__refcount++

/** The definition used when removing a reference to an object. The code just after this is automatically called when necessary */
#define EINA_REFCOUNT_UNREF(Variable)		     \
  if (--((Variable)->__refcount) == 0)

/** The definition to get the reference counting value */
#define EINA_REFCOUNT_GET(Variable) (Variable)->__refcount

/**
 * @}
 */

#endif /* EINA_REFCOUNT_H_ */
