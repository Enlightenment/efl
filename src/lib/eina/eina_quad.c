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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eina_private.h"

#include <math.h>
#include <stdio.h>

#include "eina_rectangle.h"
#include "eina_quad.h"

#define QUAD_X0(q) q->x0
#define QUAD_Y0(q) q->y0
#define QUAD_X1(q) q->x1
#define QUAD_Y1(q) q->y1
#define QUAD_X2(q) q->x2
#define QUAD_Y2(q) q->y2
#define QUAD_X3(q) q->x3
#define QUAD_Y3(q) q->y3

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/
#if 0
/** @cond internal */
/* FIXME make this function on API */
static inline void _quad_dump(Eina_Quad *q)
{
   printf("Q = %f %f, %f %f, %f %f, %f %f\n", QUAD_X0(q), QUAD_Y0(q), QUAD_X1(q), QUAD_Y1(q), QUAD_X2(q), QUAD_Y2(q), QUAD_X3(q), QUAD_Y3(q));
}
/** @endcond */
#endif

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/
EAPI void
eina_quad_rectangle_to(const Eina_Quad *q,
                       Eina_Rectangle *r)
{
   double xmin, ymin, xmax, ymax;
   /* FIXME this code is very ugly, for sure there must be a better
    * implementation */
   xmin = QUAD_X0(q) < QUAD_X1(q) ? QUAD_X0(q) : QUAD_X1(q);
   xmin = xmin < QUAD_X2(q) ? xmin : QUAD_X2(q);
   xmin = xmin < QUAD_X3(q) ? xmin : QUAD_X3(q);

   ymin = QUAD_Y0(q) < QUAD_Y1(q) ? QUAD_Y0(q) : QUAD_Y1(q);
   ymin = ymin < QUAD_Y2(q) ? ymin : QUAD_Y2(q);
   ymin = ymin < QUAD_Y3(q) ? ymin : QUAD_Y3(q);

   xmax = QUAD_X0(q) > QUAD_X1(q) ? QUAD_X0(q) : QUAD_X1(q);
   xmax = xmax > QUAD_X2(q) ? xmax : QUAD_X2(q);
   xmax = xmax > QUAD_X3(q) ? xmax : QUAD_X3(q);

   ymax = QUAD_Y0(q) > QUAD_Y1(q) ? QUAD_Y0(q) : QUAD_Y1(q);
   ymax = ymax > QUAD_Y2(q) ? ymax : QUAD_Y2(q);
   ymax = ymax > QUAD_Y3(q) ? ymax : QUAD_Y3(q);

   r->x = lround(xmin);
   r->w = lround(xmax) - r->x;
   r->y = lround(ymin);
   r->h = lround(ymax) - r->y;
}

EAPI void
eina_quad_rectangle_from(Eina_Quad *q,
                         const Eina_Rectangle *r)
{
   QUAD_X0(q) = r->x;
   QUAD_Y0(q) = r->y;
   QUAD_X1(q) = r->x + r->w;
   QUAD_Y1(q) = r->y;
   QUAD_X2(q) = r->x + r->w;
   QUAD_Y2(q) = r->y + r->h;
   QUAD_X3(q) = r->x;
   QUAD_Y3(q) = r->y + r->h;
}

EAPI void eina_quad_coords_get(const Eina_Quad *q,
                               double *qx0, double *qy0,
                               double *qx1, double *qy1,
                               double *qx2, double *qy2,
                               double *qx3, double *qy3)
{
   if (qx0) *qx0 = q->x0;
   if (qy0) *qy0 = q->y0;
   if (qx1) *qx1 = q->x1;
   if (qy1) *qy1 = q->y1;
   if (qx2) *qx2 = q->x2;
   if (qy2) *qy2 = q->y2;
   if (qx3) *qx3 = q->x3;
   if (qy3) *qy3 = q->y3;
}

EAPI void eina_quad_coords_set(Eina_Quad *q,
                               double qx0, double qy0,
                               double qx1, double qy1,
                               double qx2, double qy2,
                               double qx3, double qy3)
{
   QUAD_X0(q) = qx0;
   QUAD_Y0(q) = qy0;
   QUAD_X1(q) = qx1;
   QUAD_Y1(q) = qy1;
   QUAD_X2(q) = qx2;
   QUAD_Y2(q) = qy2;
   QUAD_X3(q) = qx3;
   QUAD_Y3(q) = qy3;
}
