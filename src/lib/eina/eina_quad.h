/* EINA - EFL data type library
 * Copyright (C) 2007-2014 Jorge Luis Zapata
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
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef EINA_QUAD_H_
#define EINA_QUAD_H_

#include <eina_rectangle.h>

/**
 * @file
 * @ender_group{Eina_Quad}
 */

/**
 * @addtogroup Eina_Data_Types_Group Data Types
 *
 * @{
 */

/**
 * @defgroup Eina_Quad_Group Quadrangles
 *
 * @brief Quadrangles operations
 *
 * @{
 */

/**
 * Quadrangle definition
 */
typedef struct _Eina_Quad
{
   double x0; /**< Top left x coordinate */
   double y0; /**< Top left y coordinate */
   double x1; /**< Top right x coordinate */
   double y1; /**< Top right y coordinate */
   double x2; /**< Bottom right x coordinate */
   double y2; /**< Bottom right y coordinate */
   double x3; /**< Bottom left x coordinate */
   double y3; /**< Bottom left y coordinate */
} Eina_Quad;

/**
 * @since 1.14
 * */
EAPI void eina_quad_rectangle_to(const Eina_Quad *q,
                                 Eina_Rectangle *r);
/**
 * @since 1.14
 * */
EAPI void eina_quad_rectangle_from(Eina_Quad *q,
                                   const Eina_Rectangle *r);
/**
 * @since 1.14
 * */
EAPI void eina_quad_coords_set(Eina_Quad *q,
                               double x1, double y1,
                               double x2, double y2,
                               double x3, double y3,
                               double x4, double y4);

/**
 * @since 1.14
 * */
EAPI void eina_quad_coords_get(const Eina_Quad *q,
                               double *x1, double *y1,
                               double *x2, double *y2,
                               double *x3, double *y3,
                               double *x4, double *y4);

/**
 * @}
 */

/**
 * @}
 */

#endif
