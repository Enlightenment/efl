/* EINA - EFL data type library
 * Copyright (C) 2008 Gustavo Sverzut Barbieri
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

#ifndef EINA_SAFETY_CHECKS_H_
#define EINA_SAFETY_CHECKS_H_
#ifdef EINA_SAFETY_CHECKS

#include "eina_error.h"

#define EINA_SAFETY_ON_NULL_RETURN(exp)					\
  do									\
    {									\
       if (EINA_UNLIKELY((exp) == NULL))				\
	 {								\
	    EINA_ERROR_PERR("%s", "safety check failed: " #exp " == NULL\n"); \
	    return;							\
	 }								\
    }									\
  while (0)

#define EINA_SAFETY_ON_NULL_RETURN_VAL(exp, val)			\
  do									\
    {									\
       if (EINA_UNLIKELY((exp) == NULL))				\
	 {								\
	    EINA_ERROR_PERR("%s", "safety check failed: " #exp " == NULL\n"); \
	    return (val);						\
	 }								\
    }									\
  while (0)

#define EINA_SAFETY_ON_TRUE_RETURN(exp)					\
  do									\
    {									\
       if (EINA_UNLIKELY(exp))						\
	 {								\
	    EINA_ERROR_PERR("%s", "safety check failed: " #exp " is true\n"); \
	    return;							\
	 }								\
    }									\
  while (0)

#define EINA_SAFETY_ON_TRUE_RETURN_VAL(exp, val)			\
  do									\
    {									\
       if (EINA_UNLIKELY(exp))						\
	 {								\
	    EINA_ERROR_PERR("%s", "safety check failed: " #exp " is true\n"); \
	    return val;							\
	 }								\
    }									\
  while (0)

#define EINA_SAFETY_ON_FALSE_RETURN(exp)				\
  do									\
    {									\
       if (EINA_UNLIKELY(!(exp)))					\
	 {								\
	    EINA_ERROR_PERR("%s", "safety check failed: " #exp " is false\n"); \
	    return;							\
	 }								\
    }									\
  while (0)

#define EINA_SAFETY_ON_FALSE_RETURN_VAL(exp, val)			\
  do									\
    {									\
       if (EINA_UNLIKELY(!(exp)))					\
	 {								\
	    EINA_ERROR_PERR("%s", "safety check failed: " #exp " is false\n"); \
	    return val;							\
	 }								\
    }									\
  while (0)

#ifdef EINA_ARG_NONNULL
/* make EINA_ARG_NONNULL void so GCC does not optimize safety checks */
#undef EINA_ARG_NONNULL
#define EINA_ARG_NONNULL(idx, ...)
#endif


#else /* no safety checks */

#define EINA_SAFETY_ON_NULL_RETURN(exp)					\
  do { (void)((exp) == NULL) } while (0)

#define EINA_SAFETY_ON_NULL_RETURN_VAL(exp, val)			\
  do { if (0 && (exp) == NULL) (void)val; } while (0)

#define EINA_SAFETY_ON_TRUE_RETURN(exp)					\
  do { (void)(exp) } while (0)

#define EINA_SAFETY_ON_TRUE_RETURN_VAL(exp, val)			\
  do { if (0 && (exp)) (void)val; } while (0)

#define EINA_SAFETY_ON_FALSE_RETURN(exp)				\
  do { (void)((!exp)) } while (0)

#define EINA_SAFETY_ON_FALSE_RETURN_VAL(exp, val)			\
  do { if (0 && !(exp)) (void)val; } while (0)

#endif /* safety checks macros */
#endif /* EINA_SAFETY_CHECKS_H_ */
