#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <math.h>
#include <float.h>
#include <ctype.h>
#include <locale.h>

#include <Efl.h>

typedef struct _Efl_Gfx_Path_Data Efl_Gfx_Path_Data;
struct _Efl_Gfx_Path_Data
{
   struct {
      double x;
      double y;
   } current, current_ctrl;

   Efl_Gfx_Path_Command *commands;
   double *points;

   unsigned int commands_count;
   unsigned int points_count;

   unsigned int reserved_pts_cnt;   //Reserved Points Count
   unsigned int reserved_cmd_cnt;   //Reserved Commands Count

   char *path_data;
   Eina_Bool convex;
};

static void _path_interpolation(Eo *obj, Efl_Gfx_Path_Data *pd, char *from, char *to, double pos);
static void _efl_gfx_path_reset(Eo *obj, Efl_Gfx_Path_Data *pd);

static inline unsigned int
_efl_gfx_path_command_length(Efl_Gfx_Path_Command command)
{
   switch (command)
     {
      case EFL_GFX_PATH_COMMAND_TYPE_END: return 0;
      case EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO: return 2;
      case EFL_GFX_PATH_COMMAND_TYPE_LINE_TO: return 2;
      case EFL_GFX_PATH_COMMAND_TYPE_CUBIC_TO: return 6;
      case EFL_GFX_PATH_COMMAND_TYPE_CLOSE: return 0;
      case EFL_GFX_PATH_COMMAND_TYPE_LAST: return 0;
     }
   return 0;
}

static inline void
_efl_gfx_path_length(const Efl_Gfx_Path_Command *commands,
                     unsigned int *cmd_length,
                     unsigned int *pts_length)
{
   if (commands)
     {
        while (commands[*cmd_length] != EFL_GFX_PATH_COMMAND_TYPE_END)
          {
             *pts_length += _efl_gfx_path_command_length(commands[*cmd_length]);
             (*cmd_length)++;
          }
     }

   /* Accounting for END command and handle gracefully the NULL case
      at the same time */
   (*cmd_length)++;
}

static inline Eina_Bool
efl_gfx_path_grow(Efl_Gfx_Path_Command command,
                  Efl_Gfx_Path_Data *pd,
                  double **offset_point)
{
   unsigned int cmd_length = 0, pts_length = 0;

   cmd_length = pd->commands_count ? pd->commands_count : 1;
   pts_length = pd->points_count;

   if (_efl_gfx_path_command_length(command))
     {
        pts_length += _efl_gfx_path_command_length(command);

        //grow up twice
        if (pts_length > pd->reserved_pts_cnt)
          {
             double *pts_tmp = realloc(pd->points, sizeof(double) * (pts_length * 2));
             if (!pts_tmp) return EINA_FALSE;
             pd->reserved_pts_cnt = pts_length * 2;
             pd->points = pts_tmp;
          }

        *offset_point =
           pd->points + pts_length - _efl_gfx_path_command_length(command);
     }

   //grow up twice
   if ((cmd_length + 1) > pd->reserved_cmd_cnt)
     {
        Efl_Gfx_Path_Command *cmd_tmp =
           realloc(pd->commands, (cmd_length  * 2) * sizeof (Efl_Gfx_Path_Command));
        if (!cmd_tmp) return EINA_FALSE;
        pd->reserved_cmd_cnt = (cmd_length * 2);
        pd->commands = cmd_tmp;
     }

   pd->commands_count = cmd_length + 1;
   pd->points_count = pts_length;

   // Append the command
   pd->commands[cmd_length - 1] = command;

   // NULL terminate the stream
   pd->commands[cmd_length] = EFL_GFX_PATH_COMMAND_TYPE_END;
   pd->convex = EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_efl_gfx_path_current_search(const Efl_Gfx_Path_Command *cmd,
                             const double *points,
                             double *current_x, double *current_y,
                             double *current_ctrl_x, double *current_ctrl_y)
{
   unsigned int i;

   if (current_x) *current_x = 0;
   if (current_y) *current_y = 0;
   if (current_ctrl_x) *current_ctrl_x = 0;
   if (current_ctrl_y) *current_ctrl_y = 0;

   if (!cmd || !points) return EINA_FALSE;

   for (i = 0; cmd[i] != EFL_GFX_PATH_COMMAND_TYPE_END; i++)
     {
        switch (cmd[i])
          {
           case EFL_GFX_PATH_COMMAND_TYPE_END:
              break;
           case EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO:
           case EFL_GFX_PATH_COMMAND_TYPE_LINE_TO:
              if (current_x) *current_x = points[0];
              if (current_y) *current_y = points[1];
              points += 2;
              break;
           case EFL_GFX_PATH_COMMAND_TYPE_CUBIC_TO:
              if (current_x) *current_x = points[0];
              if (current_y) *current_y = points[1];
              if (current_ctrl_x) *current_ctrl_x = points[4];
              if (current_ctrl_y) *current_ctrl_y = points[5];
              points += 6;
              break;
           case EFL_GFX_PATH_COMMAND_TYPE_CLOSE:
              break;
           case EFL_GFX_PATH_COMMAND_TYPE_LAST:
           default:
              return EINA_FALSE;
          }
     }

   return EINA_TRUE;
}

EOLIAN static void
_efl_gfx_path_path_set(Eo *obj, Efl_Gfx_Path_Data *pd,
                       const Efl_Gfx_Path_Command *commands,
                       const double *points)
{
   if (!commands)
     {
         _efl_gfx_path_reset(obj, pd);
         return;
     }

   Efl_Gfx_Path_Change_Event ev = { EFL_GFX_CHANGE_FLAG_PATH };
   Efl_Gfx_Path_Command *cmds;
   double *pts;
   unsigned int cmds_length = 0, pts_length = 0;

   _efl_gfx_path_length(commands, &cmds_length, &pts_length);

   cmds = realloc(pd->commands, sizeof (Efl_Gfx_Path_Command) * cmds_length);
   if (!cmds) return;

   pd->commands = cmds;

   pts = realloc(pd->points, sizeof (double) * pts_length);
   if (!pts) return;

   pd->points = pts;
   pd->commands_count = cmds_length;
   pd->points_count = pts_length;

   //full reserved memory
   pd->reserved_cmd_cnt = cmds_length;
   pd->reserved_pts_cnt = pts_length;

   memcpy(pd->commands, commands, sizeof(Efl_Gfx_Path_Command) * cmds_length);
   memcpy(pd->points, points, sizeof(double) * pts_length);

   _efl_gfx_path_current_search(pd->commands, pd->points,
                                &pd->current.x, &pd->current.y,
                                &pd->current_ctrl.x, &pd->current_ctrl.y);

   efl_event_callback_call(obj, EFL_GFX_PATH_EVENT_CHANGED, &ev);
}

EOLIAN static void
_efl_gfx_path_path_get(const Eo *obj EINA_UNUSED, Efl_Gfx_Path_Data *pd,
                  const Efl_Gfx_Path_Command **commands,
                  const double **points)
{
   if (commands) *commands = pd->commands;
   if (points) *points = pd->points;
}

EOLIAN static void
_efl_gfx_path_length_get(const Eo *obj EINA_UNUSED, Efl_Gfx_Path_Data *pd,
                         unsigned int *commands, unsigned int *points)
{
   if (commands) *commands = pd->commands_count;
   if (points) *points = pd->points_count;
}

EOLIAN static void
_efl_gfx_path_bounds_get(const Eo *obj EINA_UNUSED, Efl_Gfx_Path_Data *pd, Eina_Rect *r)
{
   double minx, miny, maxx, maxy;
   unsigned int i;

   EINA_RECTANGLE_SET(r, 0, 0, 0, 0);

   if (pd->points_count <= 0) return;

   minx = pd->points[0];
   miny = pd->points[1];
   maxx = pd->points[0];
   maxy = pd->points[1];

   for (i = 2; i < pd->points_count; i += 2)
     {
        minx = minx < pd->points[i] ? minx : pd->points[i];
        miny = miny < pd->points[i + 1] ? miny : pd->points[i + 1];
        maxx = maxx > pd->points[i] ? maxx : pd->points[i];
        maxy = maxy > pd->points[i + 1] ? maxy : pd->points[i + 1];
     }

   EINA_RECTANGLE_SET(r, minx, miny, (maxx - minx), (maxy - miny));
}

EOLIAN static void
_efl_gfx_path_current_get(const Eo *obj EINA_UNUSED, Efl_Gfx_Path_Data *pd,
                           double *x, double *y)
{
   if (x) *x = pd->current.x;
   if (y) *y = pd->current.y;
}

EOLIAN static void
_efl_gfx_path_current_ctrl_get(const Eo *obj EINA_UNUSED, Efl_Gfx_Path_Data *pd,
                                double *x, double *y)
{
   if (x) *x = pd->current_ctrl.x;
   if (y) *y = pd->current_ctrl.y;
}

EOLIAN static Eina_Bool
_efl_gfx_path_equal_commands_internal(Efl_Gfx_Path_Data *a,
                                       Efl_Gfx_Path_Data *b)
{
   unsigned int i;

   if (a->commands_count != b->commands_count) return EINA_FALSE;
   if (a->commands_count <= 0) return EINA_TRUE;

   for (i = 0; a->commands[i] == b->commands[i] &&
          a->commands[i] != EFL_GFX_PATH_COMMAND_TYPE_END; i++)
     {
        ;
     }

   return (a->commands[i] == b->commands[i]);
}

static inline double
interpolate(double from, double to, double pos_map)
{
   return (from * (1.0 - pos_map)) + (to * pos_map);
}

EOLIAN static Eina_Bool
_efl_gfx_path_interpolate(Eo *obj, Efl_Gfx_Path_Data *pd,
                          const Eo *from, const Eo *to, double pos_map)
{
   Efl_Gfx_Path_Change_Event ev = { EFL_GFX_CHANGE_FLAG_PATH };
   Efl_Gfx_Path_Data *from_pd, *to_pd;
   Efl_Gfx_Path_Command *cmds;
   double interv;    //interpolated value
   double *pts;

   if (!efl_isa(from, EFL_GFX_PATH_MIXIN) || !efl_isa(to, EFL_GFX_PATH_MIXIN))
     return EINA_FALSE;

   from_pd = efl_data_scope_get(from, EFL_GFX_PATH_MIXIN);
   to_pd = efl_data_scope_get(to, EFL_GFX_PATH_MIXIN);

   //just in case
   if (pd == from_pd || pd == to_pd) return EINA_FALSE;

   if (from_pd->path_data && to_pd->path_data)
     {
        _efl_gfx_path_reset(obj, pd);
        _path_interpolation(obj, pd,
                            from_pd->path_data, to_pd->path_data, pos_map);
     }
   else
     {
        if (!_efl_gfx_path_equal_commands_internal(from_pd, to_pd))
          return EINA_FALSE;

        cmds = realloc(pd->commands,
                       sizeof(Efl_Gfx_Path_Command) * from_pd->commands_count);
        if (!cmds && (from_pd->commands_count > 0)) return EINA_FALSE;

        pd->commands = cmds;

        pts = realloc(pd->points,
                      sizeof(double) * from_pd->points_count);
        if (!pts && (from_pd->points_count > 0)) return EINA_FALSE;

        pd->points = pts;

        if (cmds)
          {
             memcpy(cmds, from_pd->commands,
                    sizeof (Efl_Gfx_Path_Command) * from_pd->commands_count);

             if (pts)
               {
                  double *to_pts = to_pd->points;
                  double *from_pts = from_pd->points;
                  unsigned int i, j;

                  for (i = 0; cmds[i] != EFL_GFX_PATH_COMMAND_TYPE_END; i++)
                    for (j = 0; j < _efl_gfx_path_command_length(cmds[i]); j++)
                      {
                         *pts = interpolate(*from_pts, *to_pts, pos_map);
                         pts++;
                         from_pts++;
                         to_pts++;
                      }
               }
          }

        pd->points_count = from_pd->points_count;
        pd->commands_count = from_pd->commands_count;
        pd->reserved_cmd_cnt = from_pd->commands_count;
        pd->reserved_pts_cnt = from_pd->points_count;

        interv = interpolate(from_pd->current.x, to_pd->current.x, pos_map);
        pd->current.x = interv;

        interv = interpolate(from_pd->current.y, to_pd->current.y, pos_map);
        pd->current.y = interv;

        interv = interpolate(from_pd->current_ctrl.x, to_pd->current_ctrl.x,
                             pos_map);
        pd->current_ctrl.x = interv;

        interv = interpolate(from_pd->current_ctrl.y, to_pd->current_ctrl.y,
                             pos_map);
        pd->current_ctrl.y = interv;

   }

   efl_event_callback_legacy_call(obj, EFL_GFX_PATH_EVENT_CHANGED, &ev);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_gfx_path_equal_commands(Eo *obj EINA_UNUSED,
                              Efl_Gfx_Path_Data *pd,
                              const Eo *with)
{
   Efl_Gfx_Path_Data *with_pd;

   with_pd = efl_data_scope_get(with, EFL_GFX_PATH_MIXIN);
   if (!with_pd) return EINA_FALSE;

   return _efl_gfx_path_equal_commands_internal(with_pd, pd);
}

EOLIAN static void
_efl_gfx_path_reserve(Eo *obj EINA_UNUSED, Efl_Gfx_Path_Data *pd,
                      unsigned int cmd_count, unsigned int pts_count)
{
   if (pd->reserved_cmd_cnt < cmd_count)
      {
         //+1 for path close.
         pd->reserved_cmd_cnt = cmd_count + 1;
         pd->commands = realloc(pd->commands, sizeof(Efl_Gfx_Path_Command) * pd->reserved_cmd_cnt);
      }

   if (pd->reserved_pts_cnt < pts_count)
      {
         pd->reserved_pts_cnt = pts_count;
         pd->points = realloc(pd->points, sizeof(double) * pts_count);
      }
}

EOLIAN static void
_efl_gfx_path_reset(Eo *obj, Efl_Gfx_Path_Data *pd)
{
   Efl_Gfx_Path_Change_Event ev = { EFL_GFX_CHANGE_FLAG_PATH };

   free(pd->commands);
   pd->reserved_cmd_cnt = 0;
   pd->commands = NULL;
   pd->commands_count = 0;

   free(pd->points);
   pd->reserved_pts_cnt = 0;
   pd->points = NULL;
   pd->points_count = 0;

   free(pd->path_data);
   pd->path_data = NULL;

   pd->current.x = 0;
   pd->current.y = 0;
   pd->current_ctrl.x = 0;
   pd->current_ctrl.y = 0;
   pd->convex = EINA_FALSE;

   efl_event_callback_legacy_call(obj, EFL_GFX_PATH_EVENT_CHANGED, &ev);
}

EOLIAN static void
_efl_gfx_path_append_move_to(Eo *obj, Efl_Gfx_Path_Data *pd,
                              double x, double y)
{
   Efl_Gfx_Path_Change_Event ev = { EFL_GFX_CHANGE_FLAG_PATH };
   double *offset_point;

   if (!efl_gfx_path_grow(EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO, pd, &offset_point))
     return;

   offset_point[0] = x;
   offset_point[1] = y;

   pd->current.x = x;
   pd->current.y = y;

   efl_event_callback_legacy_call(obj, EFL_GFX_PATH_EVENT_CHANGED, &ev);
}

EOLIAN static void
_efl_gfx_path_append_line_to(Eo *obj, Efl_Gfx_Path_Data *pd,
                              double x, double y)
{
   Efl_Gfx_Path_Change_Event ev = { EFL_GFX_CHANGE_FLAG_PATH };
   double *offset_point;

   if (!efl_gfx_path_grow(EFL_GFX_PATH_COMMAND_TYPE_LINE_TO, pd, &offset_point))
     return;

   offset_point[0] = x;
   offset_point[1] = y;

   pd->current.x = x;
   pd->current.y = y;

   efl_event_callback_legacy_call(obj, EFL_GFX_PATH_EVENT_CHANGED, &ev);
}

EOLIAN static void
_efl_gfx_path_append_cubic_to(Eo *obj, Efl_Gfx_Path_Data *pd,
                               double ctrl_x0, double ctrl_y0,
                               double ctrl_x1, double ctrl_y1,
                               double x, double y)
{
   Efl_Gfx_Path_Change_Event ev = { EFL_GFX_CHANGE_FLAG_PATH };
   double *offset_point;

   if (!efl_gfx_path_grow(EFL_GFX_PATH_COMMAND_TYPE_CUBIC_TO,
                          pd, &offset_point))
     return;

   offset_point[0] = ctrl_x0;
   offset_point[1] = ctrl_y0;
   offset_point[2] = ctrl_x1;
   offset_point[3] = ctrl_y1;
   offset_point[4] = x;
   offset_point[5] = y;

   pd->current.x = x;
   pd->current.y = y;
   pd->current_ctrl.x = ctrl_x1;
   pd->current_ctrl.y = ctrl_y1;

   efl_event_callback_legacy_call(obj, EFL_GFX_PATH_EVENT_CHANGED, &ev);
}

EOLIAN static void
_efl_gfx_path_append_scubic_to(Eo *obj, Efl_Gfx_Path_Data *pd,
                                double x, double y,
                                double ctrl_x, double ctrl_y)
{
   double ctrl_x0, ctrl_y0;
   double current_x = 0, current_y = 0;
   double current_ctrl_x = 0, current_ctrl_y = 0;

   current_x = pd->current.x;
   current_y = pd->current.y;
   current_ctrl_x = pd->current_ctrl.x;
   current_ctrl_y = pd->current_ctrl.y;

   /* if previous command is cubic then use reflection point of current control
      point as the first control point */
   if ((pd->commands_count > 1) && (pd->commands[pd->commands_count-2] ==
        EFL_GFX_PATH_COMMAND_TYPE_CUBIC_TO))
     {
        ctrl_x0 = 2 * current_x - current_ctrl_x;
        ctrl_y0 = 2 * current_y - current_ctrl_y;
     }
   else
     {
        // use currnt point as the 1st control point
        ctrl_x0 = current_x;
        ctrl_y0 = current_y;
     }

   _efl_gfx_path_append_cubic_to(obj, pd, ctrl_x0, ctrl_y0, ctrl_x, ctrl_y,
                                 x, y);
}

EOLIAN static void
_efl_gfx_path_append_quadratic_to(Eo *obj, Efl_Gfx_Path_Data *pd,
                                   double x, double y,
                                   double ctrl_x, double ctrl_y)
{
   double current_x = 0, current_y = 0;
   double ctrl_x0, ctrl_y0, ctrl_x1, ctrl_y1;

   current_x = pd->current.x;
   current_y = pd->current.y;

   // Convert quadratic bezier to cubic
   ctrl_x0 = (current_x + 2 * ctrl_x) * (1.0 / 3.0);
   ctrl_y0 = (current_y + 2 * ctrl_y) * (1.0 / 3.0);
   ctrl_x1 = (x + 2 * ctrl_x) * (1.0 / 3.0);
   ctrl_y1 = (y + 2 * ctrl_y) * (1.0 / 3.0);

   _efl_gfx_path_append_cubic_to(obj, pd, ctrl_x0, ctrl_y0, ctrl_x1, ctrl_y1,
                                 x, y);
}

EOLIAN static void
_efl_gfx_path_append_squadratic_to(Eo *obj, Efl_Gfx_Path_Data *pd,
                                    double x, double y)
{
   double xc, yc; /* quadratic control point */
   double ctrl_x0, ctrl_y0, ctrl_x1, ctrl_y1;
   double current_x = 0, current_y = 0;
   double current_ctrl_x = 0, current_ctrl_y = 0;

   current_x = pd->current.x;
   current_y = pd->current.y;
   current_ctrl_x = pd->current_ctrl.x;
   current_ctrl_y = pd->current_ctrl.y;

   xc = 2 * current_x - current_ctrl_x;
   yc = 2 * current_y - current_ctrl_y;

   /* generate a quadratic bezier with control point = xc, yc */
   ctrl_x0 = (current_x + 2 * xc) * (1.0 / 3.0);
   ctrl_y0 = (current_y + 2 * yc) * (1.0 / 3.0);
   ctrl_x1 = (x + 2 * xc) * (1.0 / 3.0);
   ctrl_y1 = (y + 2 * yc) * (1.0 / 3.0);

   _efl_gfx_path_append_cubic_to(obj, pd, ctrl_x0, ctrl_y0,
                                  ctrl_x1, ctrl_y1,
                                   x, y);
}

/*
 * code adapted from enesim which was adapted from moonlight sources
 */
EOLIAN static void
_efl_gfx_path_append_arc_to(Eo *obj, Efl_Gfx_Path_Data *pd,
                             double x, double y,
                             double rx, double ry,
                             double angle,
                             Eina_Bool large_arc, Eina_Bool sweep)
{
   double cxp, cyp, cx, cy;
   double sx, sy;
   double cos_phi, sin_phi;
   double dx2, dy2;
   double x1p, y1p;
   double x1p2, y1p2;
   double rx2, ry2;
   double lambda;
   double c;
   double at;
   double theta1, delta_theta;
   double nat;
   double delta, bcp;
   double cos_phi_rx, cos_phi_ry;
   double sin_phi_rx, sin_phi_ry;
   double cos_theta1, sin_theta1;
   int segments, i;

   // some helpful stuff is available here:
   // http://www.w3.org/TR/SVG/implnote.html#ArcImplementationNotes
   sx = pd->current.x;
   sy = pd->current.y;

   // if start and end points are identical, then no arc is drawn
   if ((fabs(x - sx) < (1 / 256.0)) && (fabs(y - sy) < (1 / 256.0)))
     return;

   // Correction of out-of-range radii, see F6.6.1 (step 2)
   rx = fabs(rx);
   ry = fabs(ry);
   if ((rx < 0.5) || (ry < 0.5))
     {
        _efl_gfx_path_append_line_to(obj, pd, x, y);
        return;
     }

   angle = angle * M_PI / 180.0;
   cos_phi = cos(angle);
   sin_phi = sin(angle);
   dx2 = (sx - x) / 2.0;
   dy2 = (sy - y) / 2.0;
   x1p = cos_phi * dx2 + sin_phi * dy2;
   y1p = cos_phi * dy2 - sin_phi * dx2;
   x1p2 = x1p * x1p;
   y1p2 = y1p * y1p;
   rx2 = rx * rx;
   ry2 = ry * ry;
   lambda = (x1p2 / rx2) + (y1p2 / ry2);

   // Correction of out-of-range radii, see F6.6.2 (step 4)
   if (lambda > 1.0)
     {
        // see F6.6.3
        double lambda_root = sqrt(lambda);

        rx *= lambda_root;
        ry *= lambda_root;
        // update rx2 and ry2
        rx2 = rx * rx;
        ry2 = ry * ry;
     }

   c = (rx2 * ry2) - (rx2 * y1p2) - (ry2 * x1p2);

   // check if there is no possible solution
   // (i.e. we can't do a square root of a negative value)
   if (c < 0.0)
     {
        // scale uniformly until we have a single solution
        // (see F6.2) i.e. when c == 0.0
        double scale = sqrt(1.0 - c / (rx2 * ry2));
        rx *= scale;
        ry *= scale;
        // update rx2 and ry2
        rx2 = rx * rx;
        ry2 = ry * ry;

        // step 2 (F6.5.2) - simplified since c == 0.0
        cxp = 0.0;
        cyp = 0.0;
        // step 3 (F6.5.3 first part) - simplified since cxp and cyp == 0.0
        cx = 0.0;
        cy = 0.0;
     }
   else
     {
        // complete c calculation
        c = sqrt(c / ((rx2 * y1p2) + (ry2 * x1p2)));
        // inverse sign if Fa == Fs
        if (large_arc == sweep)
          c = -c;

        // step 2 (F6.5.2)
        cxp = c * ( rx * y1p / ry);
        cyp = c * (-ry * x1p / rx);

        // step 3 (F6.5.3 first part)
        cx = cos_phi * cxp - sin_phi * cyp;
        cy = sin_phi * cxp + cos_phi * cyp;
     }

   // step 3 (F6.5.3 second part) we now have the center point of the ellipse
   cx += (sx + x) / 2.0;
   cy += (sy + y) / 2.0;

   // step 4 (F6.5.4)
   // we dont' use arccos (as per w3c doc), see
   // http://www.euclideanspace.com/maths/algebra/vectors/angleBetween/index.htm
   // note: atan2 (0.0, 1.0) == 0.0
   at = atan2(((y1p - cyp) / ry), ((x1p - cxp) / rx));
   theta1 = (at < 0.0) ? 2.0 * M_PI + at : at;

   nat = atan2(((-y1p - cyp) / ry), ((-x1p - cxp) / rx));
   delta_theta = (nat < at) ? 2.0 * M_PI - at + nat : nat - at;

   if (sweep)
     {
        // ensure delta theta < 0 or else add 360 degrees
        if (delta_theta < 0.0)
          delta_theta += 2.0 * M_PI;
     }
   else
     {
        // ensure delta theta > 0 or else substract 360 degrees
        if (delta_theta > 0.0)
          delta_theta -= 2.0 * M_PI;
     }

   // add several cubic bezier to approximate the arc
   // (smaller than 90 degrees)
   // we add one extra segment because we want something
   // smaller than 90deg (i.e. not 90 itself)
   segments = (int) (fabs(delta_theta / M_PI_2)) + 1;
   delta = delta_theta / segments;

   // http://www.stillhq.com/ctpfaq/2001/comp.text.pdf-faq-2001-04.txt (section 2.13)
   bcp = 4.0 / 3 * (1 - cos(delta / 2)) / sin(delta / 2);

   cos_phi_rx = cos_phi * rx;
   cos_phi_ry = cos_phi * ry;
   sin_phi_rx = sin_phi * rx;
   sin_phi_ry = sin_phi * ry;

   cos_theta1 = cos(theta1);
   sin_theta1 = sin(theta1);

   for (i = 0; i < segments; ++i)
     {
        // end angle (for this segment) = current + delta
        double c1x, c1y, ex, ey, c2x, c2y;
        double theta2 = theta1 + delta;
        double cos_theta2 = cos(theta2);
        double sin_theta2 = sin(theta2);

        // first control point (based on start point sx,sy)
        c1x = sx - bcp * (cos_phi_rx * sin_theta1 + sin_phi_ry * cos_theta1);
        c1y = sy + bcp * (cos_phi_ry * cos_theta1 - sin_phi_rx * sin_theta1);

        // end point (for this segment)
        ex = cx + (cos_phi_rx * cos_theta2 - sin_phi_ry * sin_theta2);
        ey = cy + (sin_phi_rx * cos_theta2 + cos_phi_ry * sin_theta2);

        // second control point (based on end point ex,ey)
        c2x = ex + bcp * (cos_phi_rx * sin_theta2 + sin_phi_ry * cos_theta2);
        c2y = ey + bcp * (sin_phi_rx * sin_theta2 - cos_phi_ry * cos_theta2);

        _efl_gfx_path_append_cubic_to(obj, pd, c1x, c1y, c2x, c2y, ex, ey);

        // next start point is the current end point (same for angle)
        sx = ex;
        sy = ey;
        theta1 = theta2;
        // avoid recomputations
        cos_theta1 = cos_theta2;
        sin_theta1 = sin_theta2;
     }
}

// append arc implementation
typedef struct _Point
{
   double x;
   double y;
} Point;

inline static void
_bezier_coefficients(double t, double *ap, double *bp, double *cp, double *dp)
{
   double a,b,c,d;
   double m_t = 1.0 - t;

   b = m_t * m_t;
   c = t * t;
   d = c * t;
   a = b * m_t;
   b *= 3.0 * t;
   c *= 3.0 * m_t;
   *ap = a;
   *bp = b;
   *cp = c;
   *dp = d;
}

#define PATH_KAPPA 0.5522847498

static double
_efl_gfx_t_for_arc_angle(double angle)
{
   double radians, cos_angle, sin_angle, tc, ts, t;

   if (angle < 0.00001) return 0;
   if (EINA_FLT_EQ(angle, 90.0)) return 1;

   radians = (angle/180) * M_PI;

   cos_angle = cos(radians);
   sin_angle = sin(radians);

   // initial guess
   tc = angle / 90;

   // do some iterations of newton's method to approximate cos_angle
   // finds the zero of the function b.pointAt(tc).x() - cos_angle
   tc -= ((((2-3*PATH_KAPPA) * tc + 3*(PATH_KAPPA-1)) * tc) * tc + 1 - cos_angle) // value
   / (((6-9*PATH_KAPPA) * tc + 6*(PATH_KAPPA-1)) * tc); // derivative
   tc -= ((((2-3*PATH_KAPPA) * tc + 3*(PATH_KAPPA-1)) * tc) * tc + 1 - cos_angle) // value
   / (((6-9*PATH_KAPPA) * tc + 6*(PATH_KAPPA-1)) * tc); // derivative

   // initial guess
   ts = tc;
   // do some iterations of newton's method to approximate sin_angle
   // finds the zero of the function b.pointAt(tc).y() - sin_angle
   ts -= ((((3*PATH_KAPPA-2) * ts -  6*PATH_KAPPA + 3) * ts + 3*PATH_KAPPA) * ts - sin_angle)
   / (((9*PATH_KAPPA-6) * ts + 12*PATH_KAPPA - 6) * ts + 3*PATH_KAPPA);
   ts -= ((((3*PATH_KAPPA-2) * ts -  6*PATH_KAPPA + 3) * ts + 3*PATH_KAPPA) * ts - sin_angle)
   / (((9*PATH_KAPPA-6) * ts + 12*PATH_KAPPA - 6) * ts + 3*PATH_KAPPA);

   // use the average of the t that best approximates cos_angle
   // and the t that best approximates sin_angle
   t = 0.5 * (tc + ts);
   return t;
}

static void
_find_ellipse_coords(double x, double y, double w, double h, double angle,
                     double length, Point* start_point, Point *end_point)
{
   int i, quadrant;
   double theta, t, a, b, c, d, px, py, cx, cy;
   double w2 = w / 2;
   double h2 = h / 2;
   double angles[2] = { angle, angle + length };
   Point *points[2];

   if (EINA_FLT_EQ(w, 0.0) || EINA_FLT_EQ(h, 0.0))
     {
        if (start_point)
          {
            start_point->x = 0;
            start_point->y = 0;
          }
        if (end_point)
          {
             end_point->x = 0;
             end_point->y = 0;
          }
        return;
     }

   points[0] = start_point;
   points[1] = end_point;

   for (i = 0; i < 2; ++i)
     {
        if (!points[i])
          continue;

        theta = angles[i] - 360 * floor(angles[i] / 360);
        t = theta / 90;
        // truncate
        quadrant = (int)t;
        t -= quadrant;

        t = _efl_gfx_t_for_arc_angle(90 * t);

        // swap x and y?
        if (quadrant & 1)
          t = 1 - t;

        _bezier_coefficients(t, &a, &b, &c, &d);
        px = a + b + c*PATH_KAPPA;
        py = d + c + b*PATH_KAPPA;

        // left quadrants
        if (quadrant == 1 || quadrant == 2)
          px = -px;

        // top quadrants
        if (quadrant == 0 || quadrant == 1)
          py = -py;
        cx = x+w/2;
        cy = y+h/2;
        points[i]->x = cx + w2 * px;
        points[i]->y = cy + h2 * py;
     }
}

// The return value is the starting point of the arc
static Point
_curves_for_arc(double x, double y, double w, double h,
                double start_angle, double sweep_length,
                Point *curves, int *point_count)
{
   int start_segment, end_segment, delta, i, j, end, quadrant;
   double start_t, end_t;
   Eina_Bool split_at_start, split_at_end;
   Eina_Bezier b, res;
   Point start_point, end_point;
   double w2 = w / 2;
   double w2k = w2 * PATH_KAPPA;
   double h2 = h / 2;
   double h2k = h2 * PATH_KAPPA;

   Point points[16] =
     {
        // start point
          { x + w, y + h2 },

          // 0 -> 270 degrees
          { x + w, y + h2 + h2k },
          { x + w2 + w2k, y + h },
          { x + w2, y + h },

          // 270 -> 180 degrees
          { x + w2 - w2k, y + h },
          { x, y + h2 + h2k },
          { x, y + h2 },

          // 180 -> 90 degrees
          { x, y + h2 - h2k },
          { x + w2 - w2k, y },
          { x + w2, y },

          // 90 -> 0 degrees
          { x + w2 + w2k, y },
          { x + w, y + h2 - h2k },
          { x + w, y + h2 }
     };

   *point_count = 0;

   if (sweep_length > 360) sweep_length = 360;
   else if (sweep_length < -360) sweep_length = -360;

   // Special case fast paths
   if (EINA_FLT_EQ(start_angle, 0))
     {
        if (EINA_FLT_EQ(sweep_length, 360))
          {
             for (i = 11; i >= 0; --i)
               curves[(*point_count)++] = points[i];
             return points[12];
          }
        else if (EINA_FLT_EQ(sweep_length, -360))
          {
             for (i = 1; i <= 12; ++i)
               curves[(*point_count)++] = points[i];
             return points[0];
          }
     }

   start_segment = (int)(floor(start_angle / 90));
   end_segment = (int)(floor((start_angle + sweep_length) / 90));

   start_t = (start_angle - start_segment * 90) / 90;
   end_t = (start_angle + sweep_length - end_segment * 90) / 90;

   delta = sweep_length > 0 ? 1 : -1;
   if (delta < 0)
     {
        start_t = 1 - start_t;
        end_t = 1 - end_t;
     }

   // avoid empty start segment
   if (EINA_FLT_EQ(start_t, 1.0))
     {
        start_t = 0;
        start_segment += delta;
     }

   // avoid empty end segment
   if (EINA_FLT_EQ(end_t, 0.0))
     {
        end_t = 1;
        end_segment -= delta;
     }

   start_t = _efl_gfx_t_for_arc_angle(start_t * 90);
   end_t = _efl_gfx_t_for_arc_angle(end_t * 90);

   split_at_start = !(fabs(start_t) <= 0.00001f);
   split_at_end = !(fabs(end_t - 1.0) <= 0.00001f);

   end = end_segment + delta;

   // empty arc?
   if (start_segment == end)
     {
        quadrant = 3 - ((start_segment % 4) + 4) % 4;
        j = 3 * quadrant;
        return delta > 0 ? points[j + 3] : points[j];
     }

   _find_ellipse_coords(x, y, w, h, start_angle, sweep_length,
                        &start_point, &end_point);

   for (i = start_segment; i != end; i += delta)
     {
        quadrant = 3 - ((i % 4) + 4) % 4;
        j = 3 * quadrant;

        if (delta > 0)
          eina_bezier_values_set(&b, points[j + 3].x, points[j + 3].y,
                                 points[j + 2].x, points[j + 2].y,
                                 points[j + 1].x, points[j + 1].y,
                                 points[j].x, points[j].y);
        else
          eina_bezier_values_set(&b, points[j].x, points[j].y,
                                 points[j + 1].x, points[j + 1].y,
                                 points[j + 2].x, points[j + 2].y,
                                 points[j + 3].x, points[j + 3].y);

        // empty arc?
        if (start_segment == end_segment && (EINA_FLT_EQ(start_t, end_t)))
            return start_point;

        res = b;
        if (i == start_segment)
          {
             if (i == end_segment && split_at_end)
               eina_bezier_on_interval(&b, start_t, end_t, &res);
             else if (split_at_start)
               eina_bezier_on_interval(&b, start_t, 1, &res);
          }
        else if (i == end_segment && split_at_end)
          {
             eina_bezier_on_interval(&b, 0, end_t, &res);
          }

        // push control points
        curves[(*point_count)].x = res.ctrl_start.x;
        curves[(*point_count)++].y = res.ctrl_start.y;
        curves[(*point_count)].x = res.ctrl_end.x;
        curves[(*point_count)++].y = res.ctrl_end.y;
        curves[(*point_count)].x = res.end.x;
        curves[(*point_count)++].y = res.end.y;
     }

   curves[*(point_count)-1] = end_point;

   return start_point;
}

EOLIAN static void
_efl_gfx_path_append_arc(Eo *obj, Efl_Gfx_Path_Data *pd,
                          double x, double y, double w, double h,
                          double start_angle, double sweep_length)
{
   int i, point_count;
   Point pts[15];
   Point curve_start =
      _curves_for_arc(x, y, w, h, start_angle, sweep_length, pts, &point_count);

   if (pd->commands_count &&
       (pd->commands[pd->commands_count-2] != EFL_GFX_PATH_COMMAND_TYPE_CLOSE))
     _efl_gfx_path_append_line_to(obj, pd, curve_start.x, curve_start.y);
   else
     _efl_gfx_path_append_move_to(obj, pd, curve_start.x, curve_start.y);

   for (i = 0; i < point_count; i += 3)
     {
        _efl_gfx_path_append_cubic_to(obj, pd, pts[i].x, pts[i].y,
                                       pts[i+1].x, pts[i+1].y,
                                       pts[i+2].x, pts[i+2].y);
     }
}

EOLIAN static void
_efl_gfx_path_append_close(Eo *obj, Efl_Gfx_Path_Data *pd)
{
   Efl_Gfx_Path_Change_Event ev = { EFL_GFX_CHANGE_FLAG_PATH };
   double *offset_point;

   efl_gfx_path_grow(EFL_GFX_PATH_COMMAND_TYPE_CLOSE, pd, &offset_point);

   efl_event_callback_legacy_call(obj, EFL_GFX_PATH_EVENT_CHANGED, &ev);
}

static void
_efl_gfx_path_append_circle(Eo *obj, Efl_Gfx_Path_Data *pd,
                             double xc, double yc, double radius)
{
   Eina_Bool first = (pd->commands_count <= 0);

   _efl_gfx_path_append_arc(obj, pd, (xc - radius), (yc - radius),
                           (2 * radius), (2 * radius), 0, 360);
   _efl_gfx_path_append_close(obj, pd);

   //update convex flag
   pd->convex = first;
}

EOLIAN static void
_efl_gfx_path_append_rect(Eo *obj, Efl_Gfx_Path_Data *pd,
                           double x, double y, double w, double h,
                           double rx, double ry)
{
   Eina_Bool first = (pd->commands_count <= 0);

   // check for invalid rectangle
   if (w <=0 || h<= 0) return;

   if (rx <=0 || ry<=0)
     {
         // add a normal rect.
         _efl_gfx_path_append_move_to(obj, pd, x, y);
         _efl_gfx_path_append_line_to(obj, pd, x, y + h);
         _efl_gfx_path_append_line_to(obj, pd, x + w, y + h);
         _efl_gfx_path_append_line_to(obj, pd, x + w, y);
         _efl_gfx_path_append_close(obj, pd);
         return;
     }

   // clamp the rx and ry radius value.
   rx = 2*rx;
   ry = 2*ry;
   if (rx > w) rx = w;
   if (ry > h) ry = h;

   _efl_gfx_path_append_move_to(obj, pd, x, y + h/2);
   _efl_gfx_path_append_arc(obj, pd, x, y + h - ry, rx, ry, 180, 90);
   _efl_gfx_path_append_arc(obj, pd, x + w - rx, y + h - ry, rx, ry, 270, 90);
   _efl_gfx_path_append_arc(obj, pd, x + w - rx, y, rx, ry, 0, 90);
   _efl_gfx_path_append_arc(obj, pd, x, y, rx, ry, 90, 90);
   _efl_gfx_path_append_close(obj, pd);

   //update convex flag
   pd->convex = first;
}

EOLIAN static void
_efl_gfx_path_append_horizontal_to(Eo *obj, Efl_Gfx_Path_Data *pd, double d,
                                   double current_x EINA_UNUSED,
                                   double current_y)
{
   _efl_gfx_path_append_line_to(obj, pd, d, current_y);
}

EOLIAN static void
_efl_gfx_path_append_vertical_to(Eo *obj, Efl_Gfx_Path_Data *pd, double d,
                                 double current_x,
                                 double current_y EINA_UNUSED)
{
   _efl_gfx_path_append_line_to(obj, pd, current_x, d);
}

static char *
_skipcomma(const char *content)
{
   while (*content && isspace(*content)) content++;
   if (*content == ',') return (char*) content + 1;
   return (char*) content;
}

#if 0
static inline Eina_Bool
_next_isnumber(const char *content)
{
   char *tmp = NULL;

   (void) strtod(content, &tmp);
   return content != tmp;
}
#endif

static inline Eina_Bool
_parse_number(char **content, double *number)
{
   char *end = NULL;
   *number = strtod(*content, &end);
   // if the start of string is not number
   if ((*content) == end) return EINA_FALSE;
   //skip comma if any
   *content = _skipcomma(end);
   return EINA_TRUE;
}

static inline Eina_Bool
_parse_long(char **content, int *number)
{
   char *end = NULL;
   *number = strtol(*content, &end, 10) ? 1 : 0;
   // if the start of string is not number
   if ((*content) == end) return EINA_FALSE;
   *content = _skipcomma(end);
   return EINA_TRUE;
}

static int
_number_count(char cmd)
{
   int count = 0;
   switch (cmd)
     {
      case 'M':
      case 'm':
      case 'L':
      case 'l':
        {
           count = 2;
           break;
        }
      case 'C':
      case 'c':
      case 'E':
      case 'e':
        {
           count = 6;
           break;
        }
      case 'H':
      case 'h':
      case 'V':
      case 'v':
        {
           count = 1;
           break;
        }
      case 'S':
      case 's':
      case 'Q':
      case 'q':
      case 'T':
      case 't':
        {
           count = 4;
           break;
        }
      case 'A':
      case 'a':
        {
           count = 7;
           break;
        }
      default:
         break;
      }
   return count;
}

static void
process_command(Eo *obj, Efl_Gfx_Path_Data *pd, char cmd, double *arr, int count, double *cur_x, double *cur_y)
{
   int i;
   switch (cmd)
     {
      case 'm':
      case 'l':
      case 'c':
      case 's':
      case 'q':
      case 't':
        {
           for(i = 0; i < count - 1; i += 2)
             {
                arr[i] = arr[i] + *cur_x;
                arr[i+1] = arr[i+1] + *cur_y;
             }
           break;
        }
      case 'h':
        {
           arr[0] = arr[0] + *cur_x;
           break;
        }
      case 'v':
        {
           arr[0] = arr[0] + *cur_y;
           break;
        }
      case 'a':
        {
           arr[5] = arr[5] + *cur_x;
           arr[6] = arr[6] + *cur_y;
           break;
        }
      default:
         break;
      }

   switch (cmd)
     {
      case 'm':
      case 'M':
        {
           _efl_gfx_path_append_move_to(obj, pd, arr[0], arr[1]);
           *cur_x = arr[0];
           *cur_y = arr[1];
           break;
        }
      case 'l':
      case 'L':
        {
           _efl_gfx_path_append_line_to(obj, pd, arr[0], arr[1]);
           *cur_x = arr[0];
           *cur_y = arr[1];
           break;
        }
      case 'c':
      case 'C':
        {
           _efl_gfx_path_append_cubic_to(obj, pd, arr[0], arr[1], arr[2], arr[3], arr[4], arr[5]);
           *cur_x = arr[4];
           *cur_y = arr[5];
           break;
        }
      case 's':
      case 'S':
        {
           _efl_gfx_path_append_scubic_to(obj, pd, arr[2], arr[3], arr[0], arr[1]);
           *cur_x = arr[2];
           *cur_y = arr[3];
           break;
        }
      case 'q':
      case 'Q':
        {
           _efl_gfx_path_append_quadratic_to(obj, pd, arr[2], arr[3], arr[0], arr[1]);
           *cur_x = arr[2];
           *cur_y = arr[3];
           break;
        }
      case 't':
      case 'T':
        {
           _efl_gfx_path_append_move_to(obj, pd, arr[0], arr[1]);
           *cur_x = arr[0];
           *cur_y = arr[1];
           break;
        }
      case 'h':
      case 'H':
        {
           _efl_gfx_path_append_horizontal_to(obj, pd, arr[0], *cur_x, *cur_y);
           *cur_x = arr[0];
           break;
        }
      case 'v':
      case 'V':
        {
           _efl_gfx_path_append_vertical_to(obj, pd, arr[0], *cur_x, *cur_y);
           *cur_y = arr[0];
           break;
        }
      case 'z':
      case 'Z':
        {
           _efl_gfx_path_append_close(obj, pd);
           break;
        }
      case 'a':
      case 'A':
        {
           _efl_gfx_path_append_arc_to(obj, pd, arr[5], arr[6], arr[0], arr[1], arr[2], arr[3], arr[4]);
           *cur_x = arr[5];
           *cur_y = arr[6];
           break;
        }
      case 'E':
      case 'e':
        {
           _efl_gfx_path_append_arc(obj, pd, arr[0], arr[1], arr[2], arr[3], arr[4], arr[5]);
           break;
        }
      default:
         break;
      }
}

static char *
_next_command(char *path, char *cmd, double *arr, int *count)
{
   int i=0, large, sweep;

   path = _skipcomma(path);
   if (isalpha(*path))
     {
        *cmd = *path;
        path++;
        *count = _number_count(*cmd);
     }
   if ( *count == 7)
     {
        // special case for arc command
        if(_parse_number(&path, &arr[0]))
          if(_parse_number(&path, &arr[1]))
            if(_parse_number(&path, &arr[2]))
               if(_parse_long(&path, &large))
                  if(_parse_long(&path, &sweep))
                     if(_parse_number(&path, &arr[5]))
                        if(_parse_number(&path, &arr[6]))
                          {
                             arr[3] = large;
                             arr[4] = sweep;
                             return path;
                          }
         *count = 0;
         return NULL;
     }
   for (i = 0; i < *count; i++)
     {
        if (!_parse_number(&path, &arr[i]))
          {
             *count = 0;
             return NULL;
          }
        path = _skipcomma(path);
     }
   return path;
}

static void
_path_interpolation(Eo *obj, Efl_Gfx_Path_Data *pd,
                     char *from, char *to, double pos)
{
   int i;
   double from_arr[7], to_arr[7];
   int from_count=0, to_count=0;
   double cur_x=0, cur_y=0;
   char from_cmd= 0, to_cmd = 0;
   char *cur_locale;

   if (!from || !to)
     return;

   cur_locale = setlocale(LC_NUMERIC, NULL);
   if (cur_locale)
     cur_locale = strdup(cur_locale);
   setlocale(LC_NUMERIC, "POSIX");

   while ((from[0] != '\0') && (to[0] != '\0'))
     {
        from = _next_command(from, &from_cmd, from_arr, &from_count);
        to = _next_command(to, &to_cmd, to_arr, &to_count);
        if (from_cmd == to_cmd)
          {
             if (from_count == 7)
               {
                  //special case for arc command
                  i=0;
                  from_arr[i] = interpolate(from_arr[i], to_arr[i], pos);
                  i=1;
                  from_arr[i] = interpolate(from_arr[i], to_arr[i], pos);
                  i=2;
                  from_arr[i] = interpolate(from_arr[i], to_arr[i], pos);
                  i=5;
                  from_arr[i] = interpolate(from_arr[i], to_arr[i], pos);
                  i=6;
                  from_arr[i] = interpolate(from_arr[i], to_arr[i], pos);
               }
             else
               {
                  for(i=0; i < from_count; i++)
                    {
                       from_arr[i] = interpolate(from_arr[i], to_arr[i], pos);
                    }
               }
            process_command(obj, pd, from_cmd, from_arr, from_count, &cur_x, &cur_y);
          }
        else
          {
             goto error;
          }
     }

error:
   setlocale(LC_NUMERIC, cur_locale);
   if (cur_locale)
     free(cur_locale);
}

EOLIAN static void
_efl_gfx_path_append_svg_path(Eo *obj, Efl_Gfx_Path_Data *pd,
                              const char *svg_path_data)
{
   double number_array[7];
   int number_count = 0;
   double cur_x=0, cur_y=0;
   char cmd= 0;
   char *path = (char *) svg_path_data;
   Eina_Bool arc = EINA_FALSE;
   char *cur_locale;

   if (!path)
     return;

   cur_locale = setlocale(LC_NUMERIC, NULL);
   if (cur_locale)
     cur_locale = strdup(cur_locale);
   setlocale(LC_NUMERIC, "POSIX");

   while ((path[0] != '\0'))
     {
        path = _next_command(path, &cmd, number_array, &number_count);
        if (!path)
          {
             //printf("Error parsing command\n");
             goto error;
          }
        process_command(obj, pd, cmd, number_array, number_count, &cur_x, &cur_y);
        if ((!arc) && ((cmd == 'a') || (cmd == 'A') ||
            (cmd == 'e') || (cmd == 'E')))
          arc = EINA_TRUE;
     }
   if (arc)
     {
        // need to keep the path for interpolation
        if (pd->path_data)
          free(pd->path_data);
        pd->path_data = malloc(strlen(svg_path_data) + 1);
        strcpy(pd->path_data, svg_path_data);
     }

error:
   setlocale(LC_NUMERIC, cur_locale);
   if (cur_locale)
     free(cur_locale);
}

EOLIAN static void
_efl_gfx_path_copy_from(Eo *obj, Efl_Gfx_Path_Data *pd, const Eo *dup_from)
{
   Efl_Gfx_Path_Change_Event ev = { EFL_GFX_CHANGE_FLAG_PATH };
   Efl_Gfx_Path_Data *from;

   if (obj == dup_from) return;
   from = efl_data_scope_get(dup_from, EFL_GFX_PATH_MIXIN);
   if (!from) return;

   pd->convex = from->convex;

   _efl_gfx_path_path_set(obj, pd, from->commands, from->points);

   efl_event_callback_legacy_call(obj, EFL_GFX_PATH_EVENT_CHANGED, &ev);
}

#include "interfaces/efl_gfx_path.eo.c"
