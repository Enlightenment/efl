#include "triangulator_stroker.h"
#include <math.h>

#define PI 3.1415
#define CURVE_FLATNESS PI / 8

Triangulator_Stroker *
triangulator_stroker_new(void)
{
   Triangulator_Stroker *stroker = calloc(1, sizeof(Triangulator_Stroker));
   stroker->vertices = eina_inarray_new(sizeof(float), 0);
   stroker->arc_pts = eina_inarray_new(sizeof(float), 0);
   stroker->miter_limit = 2;
   return stroker;
}

void
triangulator_stroker_free(Triangulator_Stroker *stroker)
{
   eina_inarray_free(stroker->vertices);
   eina_inarray_free(stroker->arc_pts);
}

void triangulator_stroker_stroke_set(Triangulator_Stroker *stroker, float width,
                                     Efl_Gfx_Cap cap_style, Efl_Gfx_Join join_style, Eina_Matrix3 *m)
{
   float scale_factor = 1.0;
   if (m)
     {
        // get the minimum scale factor from matrix
        scale_factor =  m->xx < m->yy ? m->xx : m->yy;
     }
   stroker->width = (width * scale_factor)/2;
   stroker->join_style = join_style;
   stroker->cap_style = cap_style;
}

// calculate the normal vector
static void
normal_vector(float x1, float y1, float x2, float y2, float width,
              float *nx, float *ny)
{
   float pw;
   float dx = x2 - x1;
   float dy = y2 - y1;

   if (EINA_FLT_EQ(dx, 0))
     pw = width / fabsf(dy);
   else if (EINA_FLT_EQ(dy, 0))
     pw = width / fabsf(dx);
   else
     pw = width / sqrtf(dx*dx + dy*dy);

   *nx = -dy * pw;
   *ny = dx * pw;
}

// add a line segment
static void
add_line_segment(Triangulator_Stroker *stroker, float x, float y, float vx, float vy)
{
   float *ptr;

   ptr = eina_inarray_grow(stroker->vertices, 4);
   ptr[0] = x + vx;
   ptr[1] = y + vy;
   ptr[2] = x - vx;
   ptr[3] = y - vy;
}

static void
add_arc_points(Triangulator_Stroker *stroker, float cx, float cy, float from_x, float from_y, float to_x, float to_y)
{
   float tmp_x, tmp_y, *ptr;
   float dx1 = from_x - cx;
   float dy1 = from_y - cy;
   float dx2 = to_x - cx;
   float dy2 = to_y - cy;
   int size;

   eina_inarray_resize(stroker->arc_pts, 0);

#define ADD_NEW_POINT                                             \
   tmp_x = dx1 * stroker->cos_theta - dy1 * stroker->sin_theta;   \
   tmp_y = dx1 * stroker->sin_theta + dy1 * stroker->cos_theta;   \
   dx1 = tmp_x;                                                   \
   dy1 = tmp_y;                                                   \
   ptr = eina_inarray_grow(stroker->arc_pts, 2);                  \
   ptr[0] = cx + dx1;                                             \
   ptr[1] = cy + dy1;

   // while more than 180 degrees left:
   while (dx1 * dy2 - dx2 * dy1 < 0)
     {
        ADD_NEW_POINT
     }

   // while more than 90 degrees left:
   while (dx1 * dx2 + dy1 * dy2 < 0)
     {
        ADD_NEW_POINT
     }

   // while more than 0 degrees left:
   while (dx1 * dy2 - dx2 * dy1 > 0)
     {
        ADD_NEW_POINT
     }

   // remove last point which was rotated beyond [to_x, to_y].
   size = eina_inarray_count(stroker->arc_pts);
   if (size)
     eina_inarray_resize(stroker->arc_pts, size - 2);
}

static void
move_to(Triangulator_Stroker *stroker, const double *pts)
{
   float x2,y2, sx, sy, *ptr=NULL, *ptr1=NULL;
   int pts_count, arc_pts_count, front, end, i=0;
   Eina_Bool jump;

   stroker->cx = pts[0];
   stroker->cy = pts[1];
   x2 = pts[2];
   y2 = pts[3];
   normal_vector(stroker->cx, stroker->cy, x2, y2, stroker->width, &stroker->nvx, &stroker->nvy);

   // To acheive jumps we insert zero-area tringles. This is done by
   // adding two identical points in both the end of previous strip
   // and beginning of next strip
   jump = eina_inarray_count(stroker->vertices);

   switch (stroker->cap_style)
     {
      case EFL_GFX_CAP_BUTT:
         if (jump)
           {
              ptr = eina_inarray_grow(stroker->vertices, 2);
              ptr[0] = stroker->cx + stroker->nvx;
              ptr[1] = stroker->cy + stroker->nvy;
           }
         break;
      case EFL_GFX_CAP_SQUARE:
         {
            sx = stroker->cx - stroker->nvy;
            sy = stroker->cy + stroker->nvx;
            if (jump)
              {
                 ptr = eina_inarray_grow(stroker->vertices, 2);
                 ptr[0] = sx + stroker->nvx;
                 ptr[1] = sy + stroker->nvy;
              }
            add_line_segment(stroker, sx, sy, stroker->nvx, stroker->nvy);
            break;
         }
      case EFL_GFX_CAP_ROUND:
         {
            add_arc_points(stroker, stroker->cx, stroker->cy,
                           stroker->cx + stroker->nvx, stroker->cy + stroker->nvy,
                           stroker->cx - stroker->nvx, stroker->cy - stroker->nvy);
            arc_pts_count = eina_inarray_count(stroker->arc_pts);
            front = 0;
            end = arc_pts_count / 2;
            if (arc_pts_count)
              {
                 eina_inarray_grow(stroker->vertices, eina_inarray_count(stroker->arc_pts) + 2 * jump);
                 pts_count = eina_inarray_count(stroker->vertices);
                 ptr1 = eina_inarray_nth(stroker->arc_pts, 0);
                 ptr = eina_inarray_nth(stroker->vertices, 0);
                 i = pts_count;

                 while (front != end)
                   {
                      ptr[--i] = ptr1[2 * end - 1];
                      ptr[--i] = ptr1[2 * end - 2];
                      --end;
                      if (front == end)
                        break;
                      ptr[--i] = ptr1[2 * front + 1];
                      ptr[--i] = ptr1[2 * front + 0];
                      ++front;
                   }
                 if (jump)
                   {
                      ptr[i - 1] = ptr[i + 1];
                      ptr[i - 2] = ptr[i + 0];
                   }
              }
            break;
         }
      default: break;
     }
   add_line_segment(stroker, stroker->cx, stroker->cy, stroker->nvx, stroker->nvy);
}

static void
line_to(Triangulator_Stroker *stroker, const double *pts)
{
   add_line_segment(stroker, pts[0], pts[1], stroker->nvx, stroker->nvy);
   stroker->cx = pts[0];
   stroker->cy = pts[1];
}

static void
cubic_to(Triangulator_Stroker *stroker, const double *pts)
{
   Eina_Bezier b;
   float rad, vx, vy, cx, cy, threshold_minus_1, t;
   double bw, bh, x, y;
   int i, threshold;

   eina_bezier_values_set(&b, stroker->cx, stroker->cy, pts[0], pts[1], pts[2], pts[3], pts[4], pts[5]);
   eina_bezier_bounds_get(&b, NULL, NULL, &bw, &bh);

   rad = fmaxf(bw, bh);
   threshold = fminf(64, (rad + stroker->curvyness_add) * stroker->curvyness_mul);
   if (threshold < 4)
     threshold = 4;
   threshold_minus_1 = threshold - 1;
   cx = stroker->cx;
   cy = stroker->cy;

   for (i = 1; i < threshold; ++i)
     {
        t = i / threshold_minus_1;
        eina_bezier_point_at(&b, t, &x, &y);
        normal_vector(cx, cy, x, y, stroker->width, &vx, &vy);
        add_line_segment(stroker, x, y, vx, vy);
        cx = x;
        cy = y;
     }

   stroker->cx = cx;
   stroker->cy = cy;

   stroker->nvx = vx;
   stroker->nvy = vy;
}

static void
add_join(Triangulator_Stroker *stroker, float x , float y)
{
   int arc_pts_count, pts_count, i;
   float prev_nvx, prev_nvy, xprod, px, py, qx, qy, pu, qv, ix, iy, *ptr;

   // Creates a join to the next segment (cx, cy) -> (x, y)
   normal_vector(stroker->cx, stroker->cy, x, y, stroker->width, &stroker->nvx, &stroker->nvy);

   switch (stroker->join_style)
     {
      case EFL_GFX_JOIN_BEVEL:
         break;
      case EFL_GFX_JOIN_MITER:
         {
            // Find out on which side the join should be.
            pts_count = eina_inarray_count(stroker->vertices);
            ptr = eina_inarray_nth(stroker->vertices, pts_count - 2);
            prev_nvx = ptr[0] - stroker->cx;
            prev_nvy = ptr[1] - stroker->cy;
            xprod = prev_nvx * stroker->nvy - prev_nvy * stroker->nvx;

            // If the segments are parallel, use bevel join.
            if (xprod < 0.001)
              break;

            // Find the corners of the previous and next segment to join.
            if (xprod < 0)
              {
                 ptr = eina_inarray_nth(stroker->vertices, pts_count - 2);
                 px = ptr[0];
                 py = ptr[1];
                 qx = stroker->cx - stroker->nvx;
                 qy = stroker->cy - stroker->nvy;
              }
            else
              {
                 ptr = eina_inarray_nth(stroker->vertices, pts_count - 4);
                 px = ptr[0];
                 py = ptr[1];
                 qx = stroker->cx + stroker->nvx;
                 qy = stroker->cy - stroker->nvy;
              }

            // Find intersection point.
            pu = px * prev_nvx + py * prev_nvy;
            qv = qx * stroker->nvx + qy * stroker->nvy;
            ix = (stroker->nvx * pu - prev_nvy * qv) / xprod;
            iy = (prev_nvx * qv - stroker->nvx * pu) / xprod;

            // Check that the distance to the intersection point is less than the miter limit.
            if ((ix - px) * (ix - px) + (iy - py) * (iy - py) <= stroker->miter_limit * stroker->miter_limit)
              {
                 ptr = eina_inarray_grow(stroker->vertices, 4);
                 ptr[0] = ix;
                 ptr[1] = iy;
                 ptr[2] = ix;
                 ptr[3] = iy;
              }
            break;
         }
      case EFL_GFX_JOIN_ROUND:
         {
            pts_count = eina_inarray_count(stroker->vertices);
            ptr = eina_inarray_nth(stroker->vertices, pts_count - 2);
            prev_nvx = ptr[0] - stroker->cx;
            prev_nvy = ptr[1] - stroker->cy;
            if (stroker->nvx * prev_nvx - stroker->nvy * prev_nvy < 0)
              {
                 add_arc_points(stroker, 0, 0, stroker->nvx, stroker->nvy, -prev_nvx, -prev_nvy);
                 arc_pts_count = eina_inarray_count(stroker->arc_pts);
                 if (arc_pts_count)
                   ptr = eina_inarray_nth(stroker->arc_pts, 0);
                 for (i = arc_pts_count / 2; i > 0; --i)
                   add_line_segment(stroker, stroker->cx, stroker->cy, ptr[2 * i - 2], ptr[2 * i - 1]);
              }
            else
              {
                 add_arc_points(stroker, 0, 0, -prev_nvx, -prev_nvy, stroker->nvx, stroker->nvy);
                 arc_pts_count = eina_inarray_count(stroker->arc_pts) / 2;
                 if (arc_pts_count)
                   ptr = eina_inarray_nth(stroker->arc_pts, 0);
                 for (i = 0; i < arc_pts_count / 2; ++i)
                   add_line_segment(stroker, stroker->cx, stroker->cy, ptr[2 * i + 0], ptr[2 * i + 1]);
              }
            break;
         }
      default: break;
     }
   add_line_segment(stroker, stroker->cx, stroker->cy, stroker->nvx, stroker->nvy);
}

static void
end_cap(Triangulator_Stroker *stroker)
{
   float *ptr, *ptr1;
   int front, end, pts_count, arc_pts_count, i;

   switch (stroker->cap_style)
     {
      case EFL_GFX_CAP_BUTT:
         break;
      case EFL_GFX_CAP_SQUARE:
         add_line_segment(stroker, stroker->cx + stroker->nvy, stroker->cy - stroker->nvx, stroker->nvx, stroker->nvy);
         break;
      case EFL_GFX_CAP_ROUND:
         {
            pts_count = eina_inarray_count(stroker->vertices);
            ptr = eina_inarray_nth(stroker->vertices, pts_count-4);
            add_arc_points(stroker, stroker->cx, stroker->cy, ptr[2], ptr[3], ptr[0], ptr[1]);
            arc_pts_count = eina_inarray_count(stroker->arc_pts);
            if (arc_pts_count)
              {
                 ptr = eina_inarray_grow(stroker->vertices, arc_pts_count);
                 ptr1 = eina_inarray_nth(stroker->arc_pts, 0);
              }
            front = 0;
            end = arc_pts_count / 2;
            i = 0;
            while (front != end)
              {
                 ptr[i++] = ptr1[2 * end - 2];
                 ptr[i++] = ptr1[2 * end - 1];
                 --end;
                 if (front == end)
                   break;
                 ptr[i++] = ptr1[2 * front + 0];
                 ptr[i++] = ptr1[2 * front + 1];
                 ++front;
              }
            break;
         }
      default: break;
     }
}

static void
_end_cap_or_join_closed(Triangulator_Stroker *stroker,
                        const double *start,
                        Eina_Bool implicit_close, Eina_Bool ends_at_start)
{
   int count;
   float x, y, *ptr;

   if (ends_at_start)
     {
        add_join(stroker, start[2], start[3]);
     }
   else if (implicit_close)
     {
        add_join(stroker, start[0], start[1]);
        line_to(stroker, start);
        add_join(stroker, start[2], start[3]);
     }
   else
     {
        end_cap(stroker);
     }
   // add the invisible triangle
   count = eina_inarray_count(stroker->vertices);
   ptr = eina_inarray_nth(stroker->vertices, 0);
   x = ptr[count-2];
   y = ptr[count-1];
   ptr = eina_inarray_grow(stroker->vertices, 2);
   ptr[0] = x;
   ptr[1] = y;
}

static inline void
_skip_duplicate_points(const double **pts, const double *end_pts)
{
   while ((*pts + 2) < end_pts && EINA_DBL_EQ((*pts)[0], (*pts)[2]) &&
          EINA_DBL_EQ((*pts)[1], (*pts)[3]))
     {
        *pts += 2;
     }
}

static void
_path_info_get(const Efl_Gfx_Path_Command *cmds, const double *pts, Eina_Bool *implicit_close, Eina_Bool *ends_at_start)
{
   int i = 0;

   *implicit_close = EINA_FALSE;
   *ends_at_start = EINA_FALSE;
   for (++cmds; *cmds != EFL_GFX_PATH_COMMAND_TYPE_END ; ++cmds)
     {
        switch (*cmds)
          {
           case EFL_GFX_PATH_COMMAND_TYPE_LINE_TO:
              i += 2;
              break;
           case EFL_GFX_PATH_COMMAND_TYPE_CUBIC_TO:
              i += 6;
              break;
           case EFL_GFX_PATH_COMMAND_TYPE_CLOSE:
              // this path has a implicit close
              *implicit_close = EINA_TRUE;
              // fall through
           case EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO:
              if (EINA_FLT_EQ(pts[0], pts[i]) && EINA_FLT_EQ(pts[1], pts[i+1]))
                *ends_at_start = EINA_TRUE;
              return;
           default:
              break;
          }
     }
   // this path is the last path with out implicit close.
   *ends_at_start = EINA_FLT_EQ(pts[0], pts[i]) &&
                    EINA_FLT_EQ(pts[1], pts[i+1]);
}

void
triangulator_stroker_process(Triangulator_Stroker *stroker,
                              const Efl_Gfx_Path_Command *cmds, const double *pts, int cmd_count, int pt_count)
{
   const double *end_pts = pts + pt_count;
   const double *start_pts = 0;
   Eina_Bool ends_at_start, implicit_close;
   Efl_Gfx_Cap cap;
   Efl_Gfx_Path_Command previous_type;

   if (cmd_count < 2)
     return;

   eina_inarray_resize(stroker->vertices, 0);
   stroker->curvyness_add = stroker->width;
   stroker->curvyness_mul = CURVE_FLATNESS;
   stroker->roundness = fmax(4, 2 * stroker->width * stroker->curvyness_mul);
   // Over this level of segmentation, there doesn't seem to be any
   // benefit, even for huge penWidth
   if (stroker->roundness > 24)
     stroker->roundness = 24;

   stroker->sin_theta = sinf(PI / stroker->roundness);
   stroker->cos_theta = cosf(PI / stroker->roundness);

   cap = stroker->cap_style;
   ends_at_start = EINA_FALSE;
   implicit_close = EINA_FALSE;
   previous_type = EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO;
   for (; *cmds != EFL_GFX_PATH_COMMAND_TYPE_END; cmds++)
     {
        switch (*cmds)
          {
           case EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO:
              {
                 if (previous_type != EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO)
                   _end_cap_or_join_closed(stroker, start_pts, implicit_close, ends_at_start);

                 // get the sub path deatils like closed path or start at end info.
                 _path_info_get(cmds, pts, &implicit_close, &ends_at_start);

                 start_pts = pts;
                 _skip_duplicate_points(&start_pts, end_pts); // Skip duplicates to find correct normal.
                 if (start_pts + 2 >= end_pts)
                   return; // Nothing to see here...

                 if (ends_at_start || implicit_close)
                   stroker->cap_style = EFL_GFX_CAP_BUTT;

                 move_to(stroker, start_pts);
                 stroker->cap_style = cap;
                 previous_type = EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO;
                 pts+=2;
                 break;
              }
           case EFL_GFX_PATH_COMMAND_TYPE_LINE_TO:
              if (!EINA_FLT_EQ(stroker->cx, pts[0]) || !EINA_FLT_EQ(stroker->cy, (float)pts[1]))
                {
                   if (previous_type != EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO)
                     add_join(stroker, pts[0], pts[1]);
                   line_to(stroker, pts);
                   previous_type = EFL_GFX_PATH_COMMAND_TYPE_LINE_TO;
                }
              pts+=2;
              break;
           case EFL_GFX_PATH_COMMAND_TYPE_CUBIC_TO:
              if (!EINA_FLT_EQ(stroker->cx, pts[0]) ||
                  !EINA_FLT_EQ(stroker->cy, pts[1]) ||
                  !EINA_FLT_EQ(pts[0], pts[2]) ||
                  !EINA_FLT_EQ(pts[1], pts[3]) ||
                  !EINA_FLT_EQ(pts[2], pts[4]) ||
                  !EINA_FLT_EQ(pts[3], pts[5]))
                {
                   if (!EINA_FLT_EQ(stroker->cx, pts[0]) ||
                       !EINA_FLT_EQ(stroker->cy, pts[1]))
                     {
                        if (previous_type != EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO)
                          add_join(stroker, pts[0], pts[1]);
                     }
                   cubic_to(stroker, pts);
                   previous_type = EFL_GFX_PATH_COMMAND_TYPE_CUBIC_TO;
                }
              pts+=6;
              break;
           default:
              break;
          }
     }

   if (previous_type != EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO)
     _end_cap_or_join_closed(stroker, start_pts, implicit_close, ends_at_start);
}
