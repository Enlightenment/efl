#include "triangulator_simple.h"

Triangulator_Simple *
triangulator_simple_new(void)
{
   Triangulator_Simple *st = calloc(1, sizeof(Triangulator_Simple));
   st->vertices = eina_inarray_new(sizeof(float), 0);
   st->stops = eina_inarray_new(sizeof(int), 0);
   return st;
}

void
triangulator_simple_free(Triangulator_Simple *st)
{
   eina_inarray_free(st->vertices);
   eina_inarray_free(st->stops);
}

static void
_add_line(Triangulator_Simple *st, const float x, const float y)
{
   float *ptr;

   ptr = eina_inarray_grow(st->vertices, 2);
   ptr[0] = x;
   ptr[1] = y;

   if (x > st->maxx)
     st->maxx = x;
   else if (x < st->minx)
     st->minx = x;
   if (y > st->maxy)
     st->maxy = y;
    else if (y < st->miny)
      st->miny = y;
}

static void
_calculate_centroid(const Efl_Gfx_Path_Command *cmds, const double *pts, double *cx, double *cy)
{
   double sumx = 0, sumy = 0;
   int count = 0;

   sumx += pts[0];
   sumy += pts[1];
   for (cmds++, count++, pts+=2; *cmds != EFL_GFX_PATH_COMMAND_TYPE_END; cmds++)
     {
        switch (*cmds)
          {
           case EFL_GFX_PATH_COMMAND_TYPE_LINE_TO:
              sumx += pts[0];
              sumy += pts[1];
              pts +=2;
              count++;
              break;
           case EFL_GFX_PATH_COMMAND_TYPE_CUBIC_TO:
              sumx += pts[0];
              sumy += pts[1];
              sumx += pts[2];
              sumy += pts[3];
              sumx += pts[4];
              sumy += pts[5];
              pts +=6;
              count +=3;
              break;
           case EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO:
           case EFL_GFX_PATH_COMMAND_TYPE_CLOSE:
              *cx = sumx/count;
              *cy = sumy/count;
              return;
           default:
              break;
          }
      }
   //
   *cx = sumx/count;
   *cy = sumy/count;
}

void
triangulator_simple_process(Triangulator_Simple *st, const Efl_Gfx_Path_Command *cmds, const double *pts, Eina_Bool convex)
{
   double bw, bh, cx, cy, x, y, t, one_over_threshold_minus_1;
   float *ptr;
   int   *stop_ptr, threshold, i;
   Eina_Bezier b;

   eina_inarray_resize(st->vertices, 0);
   eina_inarray_resize(st->stops, 0);
   if (!convex)
     {
        _calculate_centroid(cmds, pts, &cx, &cy);
        _add_line(st, cx, cy);
     }

   cx = pts[0];
   cy = pts[1];
   // The first element is always a moveTo
   _add_line(st, cx, cy);
   pts += 2;
   cmds++;
   for (; *cmds != EFL_GFX_PATH_COMMAND_TYPE_END; cmds++)
     {
        switch (*cmds)
          {
           case EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO:

              // add closing line for the previous contour
              _add_line(st, cx, cy);

              // update stop array
              stop_ptr = eina_inarray_grow(st->stops, 1);
              stop_ptr[0] = eina_inarray_count(st->vertices);

              // add centroid if not convex
              if (!convex)
                {
                   _calculate_centroid(cmds, pts, &cx, &cy);
                   _add_line(st, cx, cy);
                 }
              cx = pts[0];
              cy = pts[1];
              _add_line(st, cx, cy);
              pts += 2;
              break;

           case EFL_GFX_PATH_COMMAND_TYPE_LINE_TO:

              _add_line(st, pts[0], pts[1]);
              pts += 2;
              break;

           case EFL_GFX_PATH_COMMAND_TYPE_CUBIC_TO:
              ptr = eina_inarray_nth(st->vertices, eina_inarray_count(st->vertices) - 2);
              eina_bezier_values_set(&b, ptr[0], ptr[1], pts[0], pts[1], pts[2], pts[3], pts[4], pts[5]);
              eina_bezier_bounds_get(&b, NULL, NULL , &bw, &bh);
              threshold = fminf(64, fmaxf(bw, bh) * 3.14f / 6);
              if (threshold < 3) threshold = 3;
              one_over_threshold_minus_1 = 1.0 / (threshold - 1);
              for (i=1; i<threshold; ++i)
                {
                   t = i * one_over_threshold_minus_1;
                   eina_bezier_point_at(&b, t, &x, &y);
                   _add_line(st, x, y);
                }
              pts += 6;
              break;
           case EFL_GFX_PATH_COMMAND_TYPE_CLOSE:
           case EFL_GFX_PATH_COMMAND_TYPE_LAST:
           case EFL_GFX_PATH_COMMAND_TYPE_END:
              break;
          }
     }
   // add closing line for the previous contour
   _add_line(st, cx, cy);

   // update stop array
   stop_ptr = eina_inarray_grow(st->stops, 1);
   stop_ptr[0] = eina_inarray_count(st->vertices);
}