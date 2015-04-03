#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <math.h>
#include <float.h>
#include <ctype.h>

#include <Efl.h>

typedef struct _Efl_Gfx_Shape_Data Efl_Gfx_Shape_Data;
struct _Efl_Gfx_Shape_Data
{
   struct {
      double x;
      double y;
   } current, current_ctrl;

   Efl_Gfx_Path_Command *commands;
   double *points;

   unsigned int commands_count;
   unsigned int points_count;
};

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
     while (commands[*cmd_length] != EFL_GFX_PATH_COMMAND_TYPE_END)
       {
          *pts_length += _efl_gfx_path_command_length(commands[*cmd_length]);
          (*cmd_length)++;
       }

   // Accounting for END command and handle gracefully the NULL case at the same time
   (*cmd_length)++;
}

static inline Eina_Bool
efl_gfx_path_grow(Efl_Gfx_Path_Command command,
                  Efl_Gfx_Path_Command **commands, double **points,
                  double **offset_point)
{
   Efl_Gfx_Path_Command *cmd_tmp;
   double *pts_tmp;
   unsigned int cmd_length = 0, pts_length = 0;

   _efl_gfx_path_length(*commands, &cmd_length, &pts_length);

   if (_efl_gfx_path_command_length(command))
     {
        pts_length += _efl_gfx_path_command_length(command);
        pts_tmp = realloc(*points, pts_length * sizeof (double));
        if (!pts_tmp) return EINA_FALSE;

        *points = pts_tmp;
        *offset_point = *points + pts_length - _efl_gfx_path_command_length(command);
     }

   cmd_tmp = realloc(*commands,
                     (cmd_length + 1) * sizeof (Efl_Gfx_Path_Command));
   if (!cmd_tmp) return EINA_FALSE;
   *commands = cmd_tmp;

   // Append the command
   cmd_tmp[cmd_length - 1] = command;
   // NULL terminate the stream
   cmd_tmp[cmd_length] = EFL_GFX_PATH_COMMAND_TYPE_END;

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

void
_efl_gfx_shape_path_set(Eo *obj, Efl_Gfx_Shape_Data *pd,
                        const Efl_Gfx_Path_Command *commands,
                        const double *points)
{
   Efl_Gfx_Path_Command *cmds;
   double *pts;
   unsigned int cmds_length = 0, pts_length = 0;

   _efl_gfx_path_length(commands, &cmds_length, &pts_length);

   cmds = realloc(pd->commands,
                  sizeof (Efl_Gfx_Path_Command) * cmds_length);
   if (!cmds) return ;
   pd->commands = cmds;

   pts = realloc(pd->points,
                 sizeof (double) * pts_length);
   if (!pts) return ;
   pd->points = pts;

   pd->commands_count = cmds_length;
   pd->points_count = pts_length;

   memcpy(pd->commands, commands, sizeof (Efl_Gfx_Path_Command) * cmds_length);
   memcpy(pd->points, points, sizeof (double) * pts_length);

   _efl_gfx_path_current_search(pd->commands, pd->points,
                                &pd->current.x, &pd->current.y,
                                &pd->current_ctrl.x, &pd->current_ctrl.y);

   eo_do(obj, eo_event_callback_call(EFL_GFX_CHANGED, NULL));
}

void
_efl_gfx_shape_path_get(Eo *obj EINA_UNUSED, Efl_Gfx_Shape_Data *pd,
                        const Efl_Gfx_Path_Command **commands,
                        const double **points)
{
   if (commands) *commands = pd->commands;
   if (points) *points = pd->points;
}

void
_efl_gfx_shape_path_length_get(Eo *obj EINA_UNUSED, Efl_Gfx_Shape_Data *pd,
                               unsigned int *commands, unsigned int *points)
{
   if (commands) *commands = pd->commands_count;
   if (points) *points = pd->points_count;
}

void
_efl_gfx_shape_current_get(Eo *obj EINA_UNUSED, Efl_Gfx_Shape_Data *pd,
                           double *x, double *y)
{
   if (x) *x = pd->current.x;
   if (y) *y = pd->current.y;
}

void
_efl_gfx_shape_current_ctrl_get(Eo *obj EINA_UNUSED, Efl_Gfx_Shape_Data *pd,
                                double *x, double *y)
{
   if (x) *x = pd->current_ctrl.x;
   if (y) *y = pd->current_ctrl.y;
}

static Eina_Bool
_efl_gfx_shape_equal_commands_internal(Efl_Gfx_Shape_Data *a,
                                       Efl_Gfx_Shape_Data *b)
{
   unsigned int i;

   if (a->commands_count != b->commands_count) return EINA_FALSE;

   for (i = 0; a->commands[i] == b->commands[i] &&
          a->commands[i] != EFL_GFX_PATH_COMMAND_TYPE_END; i++)
     ;

   return (a->commands[i] == b->commands[i]);
}

static inline double
interpolate(double from, double to, double pos_map)
{
   return (from * pos_map) + (to * (1.0 - pos_map));
}

Eina_Bool
_efl_gfx_shape_interpolate(Eo *obj, Efl_Gfx_Shape_Data *pd,
                           const Eo *from, const Eo *to, double pos_map)
{
   Efl_Gfx_Shape_Data *from_pd, *to_pd;
   Efl_Gfx_Path_Command *cmds;
   double *pts, *from_pts, *to_pts;
   unsigned int i, j;

   from_pd = eo_data_scope_get(from, EFL_GFX_SHAPE_MIXIN);
   to_pd = eo_data_scope_get(to, EFL_GFX_SHAPE_MIXIN);
   if (!from_pd && !to_pd) return EINA_FALSE;
   if (!_efl_gfx_shape_equal_commands_internal(from_pd, to_pd))
     return EINA_FALSE;

   cmds = realloc(pd->commands,
                  sizeof (Efl_Gfx_Path_Command) * from_pd->commands_count);
   if (!cmds) return EINA_FALSE;
   pd->commands = cmds;

   pts = realloc(pd->points,
                 sizeof (double) * from_pd->points_count);
   if (!pts) return EINA_FALSE;
   pd->points = pts;

   memcpy(cmds, from_pd->commands,
          sizeof (Efl_Gfx_Path_Command) * from_pd->commands_count);

   to_pts = to_pd->points;
   from_pts = from_pd->points;

   for (i = 0; cmds[i] != EFL_GFX_PATH_COMMAND_TYPE_END; i++)
     for (j = 0; j < _efl_gfx_path_command_length(cmds[i]); j++)
       {
          *pts = interpolate(*from_pts, *to_pts, pos_map);

          pts++;
          from_pts++;
          to_pts++;
       }

   pd->current.x = interpolate(from_pd->current.x,
                               to_pd->current.x,
                               pos_map);
   pd->current.y = interpolate(from_pd->current.y,
                               to_pd->current.y,
                               pos_map);
   pd->current_ctrl.x = interpolate(from_pd->current_ctrl.x,
                                    to_pd->current_ctrl.x,
                                    pos_map);
   pd->current_ctrl.y = interpolate(from_pd->current_ctrl.y,
                                    to_pd->current_ctrl.y,
                                    pos_map);

   eo_do(obj, eo_event_callback_call(EFL_GFX_CHANGED, NULL));

   return EINA_TRUE;
}

Eina_Bool
_efl_gfx_shape_equal_commands(Eo *obj EINA_UNUSED,
                              Efl_Gfx_Shape_Data *pd,
                              const Eo *with)
{
   Efl_Gfx_Shape_Data *with_pd;

   with_pd = eo_data_scope_get(with, EFL_GFX_SHAPE_MIXIN);
   if (!with_pd) return EINA_FALSE;

   return _efl_gfx_shape_equal_commands_internal(with_pd, pd);
}

void
_efl_gfx_shape_dup(Eo *obj, Efl_Gfx_Shape_Data *pd, Eo *dup_from)
{
   const Efl_Gfx_Dash *dash = NULL;
   Efl_Gfx_Shape_Data *from;
   unsigned int dash_length = 0;
   Efl_Gfx_Cap cap;
   Efl_Gfx_Join j;
   int sr, sg, sb, sa;
   double scale, location;
   double sw;

   if (obj == dup_from) return ;
   from = eo_data_scope_get(dup_from, EFL_GFX_SHAPE_MIXIN);
   if (!from) return ;

   eo_do(dup_from,
         scale = efl_gfx_shape_stroke_scale_get(),
         efl_gfx_shape_stroke_color_get(&sr, &sg, &sb, &sa),
         sw = efl_gfx_shape_stroke_width_get(),
         location = efl_gfx_shape_stroke_location_get(),
         efl_gfx_shape_stroke_dash_get(&dash, &dash_length),
         cap = efl_gfx_shape_stroke_cap_get(),
         j = efl_gfx_shape_stroke_join_get());
   eo_do(obj,
         efl_gfx_shape_stroke_scale_set(scale),
         efl_gfx_shape_stroke_color_set(sr, sg, sb, sa),
         efl_gfx_shape_stroke_width_set(sw),
         efl_gfx_shape_stroke_location_set(location),
         efl_gfx_shape_stroke_dash_set(dash, dash_length),
         efl_gfx_shape_stroke_cap_set(cap),
         efl_gfx_shape_stroke_join_set(j));

   _efl_gfx_shape_path_set(obj, pd, from->commands, from->points);

   eo_do(obj, eo_event_callback_call(EFL_GFX_CHANGED, NULL));
}

void
_efl_gfx_shape_reset(Eo *obj, Efl_Gfx_Shape_Data *pd)
{
   free(pd->commands);
   pd->commands = NULL;
   pd->commands_count = 0;

   free(pd->points);
   pd->points = NULL;
   pd->points_count = 0;

   pd->current.x = 0;
   pd->current.y = 0;
   pd->current_ctrl.x = 0;
   pd->current_ctrl.y = 0;

   eo_do(obj, eo_event_callback_call(EFL_GFX_CHANGED, NULL));
}

void
_efl_gfx_shape_append_move_to(Eo *obj, Efl_Gfx_Shape_Data *pd,
                              double x, double y)
{
   double *offset_point;

   if (!efl_gfx_path_grow(EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO,
                          &pd->commands, &pd->points, &offset_point))
     return ;

   offset_point[0] = x;
   offset_point[1] = y;

   pd->current.x = x;
   pd->current.y = y;

   eo_do(obj, eo_event_callback_call(EFL_GFX_CHANGED, NULL));
}

void
_efl_gfx_shape_append_line_to(Eo *obj, Efl_Gfx_Shape_Data *pd,
                              double x, double y)
{
   double *offset_point;

   if (!efl_gfx_path_grow(EFL_GFX_PATH_COMMAND_TYPE_LINE_TO,
                          &pd->commands, &pd->points, &offset_point))
     return ;

   offset_point[0] = x;
   offset_point[1] = y;

   pd->current.x = x;
   pd->current.y = y;

   eo_do(obj, eo_event_callback_call(EFL_GFX_CHANGED, NULL));
}

void
_efl_gfx_shape_append_cubic_to(Eo *obj, Efl_Gfx_Shape_Data *pd,
                               double x, double y,
                               double ctrl_x0, double ctrl_y0,
                               double ctrl_x1, double ctrl_y1)
{
   double *offset_point;

   if (!efl_gfx_path_grow(EFL_GFX_PATH_COMMAND_TYPE_CUBIC_TO,
                          &pd->commands, &pd->points, &offset_point))
     return ;

   offset_point[0] = x;
   offset_point[1] = y;
   offset_point[2] = ctrl_x0;
   offset_point[3] = ctrl_y0;
   offset_point[4] = ctrl_x1;
   offset_point[5] = ctrl_y1;

   pd->current.x = x;
   pd->current.y = y;
   pd->current_ctrl.x = ctrl_x1;
   pd->current_ctrl.y = ctrl_y1;

   eo_do(obj, eo_event_callback_call(EFL_GFX_CHANGED, NULL));
}

void
_efl_gfx_shape_append_scubic_to(Eo *obj, Efl_Gfx_Shape_Data *pd,
                                double x, double y,
                                double ctrl_x, double ctrl_y)
{
   double ctrl_x0, ctrl_y0;
   double current_x = 0, current_y = 0;
   double current_ctrl_x = 0, current_ctrl_y = 0;

   current_x = pd->current.x;
   current_y = pd->current.x;
   current_ctrl_x = pd->current_ctrl.x;
   current_ctrl_y = pd->current_ctrl.y;

   ctrl_x0 = 2 * current_x - current_ctrl_x;
   ctrl_y0 = 2 * current_y - current_ctrl_y;

   _efl_gfx_shape_append_cubic_to(obj, pd, x, y,
                                  ctrl_x0, ctrl_y0, ctrl_x, ctrl_y);
}

void
_efl_gfx_shape_append_quadratic_to(Eo *obj, Efl_Gfx_Shape_Data *pd,
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

   _efl_gfx_shape_append_cubic_to(obj, pd, x, y,
                                  ctrl_x0, ctrl_y0, ctrl_x1, ctrl_y1);
}

void
_efl_gfx_shape_append_squadratic_to(Eo *obj, Efl_Gfx_Shape_Data *pd,
                                    double x, double y)
{
   double xc, yc; /* quadratic control point */
   double ctrl_x0, ctrl_y0, ctrl_x1, ctrl_y1;
   double current_x = 0, current_y = 0;
   double current_ctrl_x = 0, current_ctrl_y = 0;

   current_x = pd->current.x;
   current_y = pd->current.x;
   current_ctrl_x = pd->current_ctrl.x;
   current_ctrl_y = pd->current_ctrl.y;

   xc = 2 * current_x - current_ctrl_x;
   yc = 2 * current_y - current_ctrl_y;
   /* generate a quadratic bezier with control point = xc, yc */
   ctrl_x0 = (current_x + 2 * xc) * (1.0 / 3.0);
   ctrl_y0 = (current_y + 2 * yc) * (1.0 / 3.0);
   ctrl_x1 = (x + 2 * xc) * (1.0 / 3.0);
   ctrl_y1 = (y + 2 * yc) * (1.0 / 3.0);

   _efl_gfx_shape_append_cubic_to(obj, pd, x, y,
                                  ctrl_x0, ctrl_y0,
                                  ctrl_x1, ctrl_y1);
}

// This function come from librsvg rsvg-path.c
static void
_efl_gfx_shape_append_arc_segment(Eo *eo, Efl_Gfx_Shape_Data *pd,
                                  double xc, double yc,
                                  double th0, double th1, double rx, double ry,
                                  double angle)
{
   double x1, y1, x2, y2, x3, y3;
   double t;
   double th_half;
   double f, sinf, cosf;

   f = angle * M_PI / 180.0;
   sinf = sin(f);
   cosf = cos(f);

   th_half = 0.5 * (th1 - th0);
   t = (8.0 / 3.0) * sin(th_half * 0.5) * sin(th_half * 0.5) / sin(th_half);
   x1 = rx * (cos(th0) - t * sin(th0));
   y1 = ry * (sin(th0) + t * cos(th0));
   x3 = rx* cos(th1);
   y3 = ry* sin(th1);
   x2 = x3 + rx * (t * sin(th1));
   y2 = y3 + ry * (-t * cos(th1));

   _efl_gfx_shape_append_cubic_to(eo, pd,
                                  xc + cosf * x3 - sinf * y3,
                                  yc + sinf * x3 + cosf * y3,
                                  xc + cosf * x1 - sinf * y1,
                                  yc + sinf * x1 + cosf * y1,
                                  xc + cosf * x2 - sinf * y2,
                                  yc + sinf * x2 + cosf * y2);
}

// This function come from librsvg rsvg-path.c
void
_efl_gfx_shape_append_arc_to(Eo *obj, Efl_Gfx_Shape_Data *pd,
                             double x, double y,
                             double rx, double ry,
                             double angle,
                             Eina_Bool large_arc, Eina_Bool sweep)
{
   /* See Appendix F.6 Elliptical arc implementation notes
      http://www.w3.org/TR/SVG/implnote.html#ArcImplementationNotes */
   double f, sinf, cosf;
   double x1, y1, x2, y2;
   double x1_, y1_;
   double cx_, cy_, cx, cy;
   double gamma;
   double theta1, delta_theta;
   double k1, k2, k3, k4, k5;
   int i, n_segs;

   x1 = pd->current.x;
   y1 = pd->current.x;

   /* Start and end of path segment */
   x2 = x;
   y2 = y;

   if (x1 == x2 && y1 == y2)
     return;

   /* X-axis */
   f = angle * M_PI / 180.0;
   sinf = sin(f);
   cosf = cos(f);

   /* Check the radius against floading point underflow.
      See http://bugs.debian.org/508443 */
   if ((fabs(rx) < DBL_EPSILON) || (fabs(ry) < DBL_EPSILON))
     {
        _efl_gfx_shape_append_line_to(obj, pd, x, y);
        return;
     }

   if (rx < 0) rx = -rx;
   if (ry < 0) ry = -ry;

   k1 = (x1 - x2) / 2;
   k2 = (y1 - y2) / 2;

   x1_ = cosf * k1 + sinf * k2;
   y1_ = -sinf * k1 + cosf * k2;

   gamma = (x1_ * x1_) / (rx * rx) + (y1_ * y1_) / (ry * ry);
   if (gamma > 1)
     {
        rx *= sqrt(gamma);
        ry *= sqrt(gamma);
     }

   /* Compute the center */
   k1 = rx * rx * y1_ * y1_ + ry * ry * x1_ * x1_;
   if (k1 == 0) return;

   k1 = sqrt(fabs((rx * rx * ry * ry) / k1 - 1));
   if (sweep == large_arc)
     k1 = -k1;

   cx_ = k1 * rx * y1_ / ry;
   cy_ = -k1 * ry * x1_ / rx;

   cx = cosf * cx_ - sinf * cy_ + (x1 + x2) / 2;
   cy = sinf * cx_ + cosf * cy_ + (y1 + y2) / 2;

   /* Compute start angle */
   k1 = (x1_ - cx_) / rx;
   k2 = (y1_ - cy_) / ry;
   k3 = (-x1_ - cx_) / rx;
   k4 = (-y1_ - cy_) / ry;

   k5 = sqrt(fabs(k1 * k1 + k2 * k2));
   if (k5 == 0) return;

   k5 = k1 / k5;
   if (k5 < -1) k5 = -1;
   else if(k5 > 1) k5 = 1;

   theta1 = acos(k5);
   if(k2 < 0) theta1 = -theta1;

   /* Compute delta_theta */
   k5 = sqrt(fabs((k1 * k1 + k2 * k2) * (k3 * k3 + k4 * k4)));
   if (k5 == 0) return;

   k5 = (k1 * k3 + k2 * k4) / k5;
   if (k5 < -1) k5 = -1;
   else if (k5 > 1) k5 = 1;
   delta_theta = acos(k5);
   if(k1 * k4 - k3 * k2 < 0) delta_theta = -delta_theta;

   if (sweep && delta_theta < 0)
     delta_theta += M_PI*2;
   else if (!sweep && delta_theta > 0)
     delta_theta -= M_PI*2;

   /* Now draw the arc */
   n_segs = ceil(fabs(delta_theta / (M_PI * 0.5 + 0.001)));

   for (i = 0; i < n_segs; i++)
     _efl_gfx_shape_append_arc_segment(obj, pd,
                                       cx, cy,
                                       theta1 + i * delta_theta / n_segs,
                                       theta1 + (i + 1) * delta_theta / n_segs,
                                       rx, ry, angle);
}

void
_efl_gfx_shape_append_close(Eo *obj, Efl_Gfx_Shape_Data *pd)
{
   double *offset_point;

   efl_gfx_path_grow(EFL_GFX_PATH_COMMAND_TYPE_CLOSE,
                     &pd->commands, &pd->points, &offset_point);

   eo_do(obj, eo_event_callback_call(EFL_GFX_CHANGED, NULL));
}

void
_efl_gfx_shape_append_circle(Eo *obj, Efl_Gfx_Shape_Data *pd,
                             double x, double y, double radius)
{
   _efl_gfx_shape_append_move_to(obj, pd, x, y - radius);
   _efl_gfx_shape_append_arc_to(obj, pd, x - radius, y, radius, radius, 0, EINA_FALSE, EINA_FALSE);
   _efl_gfx_shape_append_arc_to(obj, pd, x, y + radius, radius, radius, 0, EINA_FALSE, EINA_FALSE);
   _efl_gfx_shape_append_arc_to(obj, pd, x + radius, y, radius, radius, 0, EINA_FALSE, EINA_FALSE);
   _efl_gfx_shape_append_arc_to(obj, pd, x, y - radius, radius, radius, 0, EINA_FALSE, EINA_FALSE);
}

static void
_efl_gfx_path_append_horizontal_to(Eo *obj, Efl_Gfx_Shape_Data *pd,
                                   double d, double current_x EINA_UNUSED, double current_y)
{
   _efl_gfx_shape_append_line_to(obj, pd, d, current_y);
}

static void
_efl_gfx_path_append_vertical_to(Eo *obj, Efl_Gfx_Shape_Data *pd,
                                 double d, double current_x, double current_y EINA_UNUSED)
{
   _efl_gfx_shape_append_line_to(obj, pd, current_x, d);
}

static char *
_strcomma(const char *content)
{
   while (*content && isspace(*content)) content++;
   if (*content != ',') return NULL;
   return (char*) content + 1;
}

static inline Eina_Bool
_next_isnumber(const char *content)
{
   char *tmp = NULL;

   (void) strtod(content, &tmp);
   return content != tmp;
}

static Eina_Bool
_efl_gfx_path_parse_pair(const char *content, char **end, double *x, double *y)
{
   /* "x,y" */
   char *end1 = NULL;
   char *end2 = NULL;

   *x = strtod(content, &end1);
   end1 = _strcomma(end1);
   if (!end1) return EINA_FALSE;
   *y = strtod(end1, &end2);
   if (end1 == end2) return EINA_FALSE;

   *end = end2;
   return EINA_TRUE;
}

static Eina_Bool
_efl_gfx_path_parse_pair_to(const char *content, char **end,
                            Eo *obj, Efl_Gfx_Shape_Data *pd,
                            double *current_x, double *current_y,
                            void (*func)(Eo *obj, Efl_Gfx_Shape_Data *pd, double x, double y),
                            Eina_Bool rel)
{
   double x, y;

   *end = (char*) content;
   do
     {
        Eina_Bool r;

        r = _efl_gfx_path_parse_pair(content, end, &x, &y);
        if (!r) return EINA_FALSE;

        if (rel)
          {
             x += *current_x;
             y += *current_y;
          }

        func(obj, pd, x, y);
        content = *end;

        *current_x = x;
        *current_y = y;
     }
   while (_next_isnumber(content));

   return EINA_TRUE;
}

static Eina_Bool
_efl_gfx_path_parse_double_to(const char *content, char **end,
                              Eo *obj, Efl_Gfx_Shape_Data *pd,
                              double *current, double current_x, double current_y,
                              void (*func)(Eo *obj, Efl_Gfx_Shape_Data *pd, double d, double current_x, double current_y),
                              Eina_Bool rel)
{
   double d;
   Eina_Bool first = EINA_FALSE;

   *end = (char*) content;
   do
     {
        d = strtod(content, end);
        if (content == *end)
          return first;
        first = EINA_TRUE;

        if (rel)
          {
             d += *current;
          }

        func(obj, pd, d, current_x, current_y);
        content = *end;

        *current = d;
     }
   while (1); // This is an optimisation as we have only one parameter.

   return EINA_TRUE;
}

static Eina_Bool
_efl_gfx_path_parse_six(const char *content, char **end,
                        double *x, double *y,
                        double *ctrl_x0, double *ctrl_y0,
                        double *ctrl_x1, double *ctrl_y1)
{
   /* "x,y ctrl_x0,ctrl_y0 ctrl_x1,ctrl_y1" */
   char *end1 = NULL;
   char *end2 = NULL;

   *x = strtod(content, &end1);
   end1 = _strcomma(end1);
   if (!end1) return EINA_FALSE;
   *y = strtod(end1, &end2);
   if (end1 == end2) return EINA_FALSE;

   *ctrl_x0 = strtod(end2, &end2);
   end2 = _strcomma(end2);
   if (!end2) return EINA_FALSE;
   *ctrl_y0 = strtod(end2, &end1);
   if (end1 == end2) return EINA_FALSE;

   *ctrl_x1 = strtod(end1, &end2);
   end2 = _strcomma(end2);
   if (!end2) return EINA_FALSE;
   *ctrl_y1 = strtod(end2, &end1);
   if (end1 == end2) return EINA_FALSE;

   *end = end1;

   return EINA_TRUE;
}

static Eina_Bool
_efl_gfx_path_parse_six_to(const char *content, char **end,
                           Eo *obj, Efl_Gfx_Shape_Data *pd,
                           double *current_x, double *current_y,
                           void (*func)(Eo *obj, Efl_Gfx_Shape_Data *pd, double x, double y, double ctrl_x0, double ctrl_y0, double ctrl_x1, double ctrl_y1),
                           Eina_Bool rel)
{
   double x, y, ctrl_x0, ctrl_y0, ctrl_x1, ctrl_y1;

   *end = (char*) content;
   do
     {
        Eina_Bool r;

        r = _efl_gfx_path_parse_six(content, end,
                                    &x, &y,
                                    &ctrl_x0, &ctrl_y0,
                                    &ctrl_x1, &ctrl_y1);
        if (!r) return EINA_FALSE;

        if (rel)
          {
             x += *current_x;
             y += *current_y;
          }

        func(obj, pd, x, y, ctrl_x0, ctrl_y0, ctrl_x1, ctrl_y1);
        content = *end;

        *current_x = x;
        *current_y = y;
     }
   while (_next_isnumber(content));

   return EINA_TRUE;
}

static Eina_Bool
_efl_gfx_path_parse_quad(const char *content, char **end,
                         double *x, double *y,
                         double *ctrl_x0, double *ctrl_y0)
{
   /* "x,y ctrl_x0,ctrl_y0" */
   char *end1 = NULL;
   char *end2 = NULL;

   *x = strtod(content, &end1);
   end1 = _strcomma(end1);
   if (!end1) return EINA_FALSE;
   *y = strtod(end1, &end2);
   if (end1 == end2) return EINA_FALSE;

   *ctrl_x0 = strtod(end2, &end1);
   end1 = _strcomma(end2);
   if (!end1) return EINA_FALSE;
   *ctrl_y0 = strtod(end1, &end2);
   if (end1 == end2) return EINA_FALSE;

   *end = end2;

   return EINA_TRUE;
}

static Eina_Bool
_efl_gfx_path_parse_quad_to(const char *content, char **end,
                            Eo *obj, Efl_Gfx_Shape_Data *pd,
                            double *current_x, double *current_y,
                            void (*func)(Eo *obj, Efl_Gfx_Shape_Data *pd,
                                         double x, double y, double ctrl_x0, double ctrl_y0),
                            Eina_Bool rel)
{
   double x, y, ctrl_x0, ctrl_y0;

   *end = (char*) content;
   do
     {
        Eina_Bool r;

        r = _efl_gfx_path_parse_quad(content, end,
                                     &x, &y,
                                     &ctrl_x0, &ctrl_y0);
        if (!r) return EINA_FALSE;

        if (rel)
          {
             x += *current_x;
             y += *current_y;
          }

        func(obj, pd, x, y, ctrl_x0, ctrl_y0);
        content = *end;

        *current_x = x;
        *current_y = y;
     }
   while (_next_isnumber(content));

   return EINA_TRUE;
}

static Eina_Bool
_efl_gfx_path_parse_arc(const char *content, char **end,
                        double *x, double *y,
                        double *rx, double *ry,
                        double *radius,
                        Eina_Bool *large_arc, Eina_Bool *sweep)
{
   /* "rx,ry r large-arc-flag,sweep-flag x,y" */
   char *end1 = NULL;
   char *end2 = NULL;

   *rx = strtod(content, &end1);
   end1 = _strcomma(end1);
   if (!end1) return EINA_FALSE;
   *ry = strtod(end1, &end2);
   if (end1 == end2) return EINA_FALSE;

   *radius = strtod(end2, &end1);
   if (end1 == end2) return EINA_FALSE;

   *large_arc = strtol(end1, &end2, 10) ? EINA_TRUE : EINA_FALSE;
   end1 = _strcomma(end2);
   if (!end1) return EINA_FALSE;
   *sweep = strtol(end1, &end2, 10) ? EINA_TRUE : EINA_FALSE;
   if (end1 == end2) return EINA_FALSE;

   *x = strtod(end2, &end1);
   end1 = _strcomma(end2);
   if (!end1) return EINA_FALSE;
   *y = strtod(end1, &end2);
   if (end1 == end2) return EINA_FALSE;

   *end = end2;

   return EINA_TRUE;
}

static Eina_Bool
_efl_gfx_path_parse_arc_to(const char *content, char **end,
                           Eo *obj, Efl_Gfx_Shape_Data *pd,
                           double *current_x, double *current_y,
                           void (*func)(Eo *obj, Efl_Gfx_Shape_Data *pd,
                                        double x, double y, double rx, double ry, double angle,
                                        Eina_Bool large_arc, Eina_Bool sweep),
                           Eina_Bool rel)
{
   double x, y, rx, ry, angle;
   Eina_Bool large_arc, sweep; // FIXME: handle those flag

   *end = (char*) content;
   do
     {
        Eina_Bool r;

        r = _efl_gfx_path_parse_arc(content, end,
                                    &x, &y,
                                    &rx, &ry,
                                    &angle,
                                    &large_arc, &sweep);
        if (!r) return EINA_FALSE;

        if (rel)
          {
             x += *current_x;
             y += *current_y;
          }

        func(obj, pd, x, y, rx, ry, angle, large_arc, sweep);
        content = *end;

        *current_x = x;
        *current_y = y;
     }
   while (_next_isnumber(content));

   return EINA_TRUE;
}

void
_efl_gfx_shape_append_svg_path(Eo *obj, Efl_Gfx_Shape_Data *pd,
                               const char *svg_path_data)
{
   double current_x = 0, current_y = 0;
   char *content = (char*) svg_path_data;

   if (!content) return ;

   while (content[0] != '\0')
     {
        while (isspace(content[0])) content++;

        switch (content[0])
          {
           case 'M':
              if (!_efl_gfx_path_parse_pair_to(&content[1],
                                               &content,
                                               obj, pd,
                                               &current_x, &current_y,
                                               _efl_gfx_shape_append_move_to,
                                               EINA_FALSE))
                return ;
              break;
           case 'm':
              if (!_efl_gfx_path_parse_pair_to(&content[1],
                                               &content,
                                               obj, pd,
                                               &current_x, &current_y,
                                               _efl_gfx_shape_append_move_to,
                                               EINA_TRUE))
                return ;
              break;
           case 'z':
              _efl_gfx_shape_append_close(obj, pd);
              content++;
              break;
           case 'L':
              if (!_efl_gfx_path_parse_pair_to(&content[1],
                                               &content,
                                               obj, pd,
                                               &current_x, &current_y,
                                               _efl_gfx_shape_append_line_to,
                                               EINA_FALSE))
                return ;
              break;
           case 'l':
              if (!_efl_gfx_path_parse_pair_to(&content[1],
                                               &content,
                                               obj, pd,
                                               &current_x, &current_y,
                                               _efl_gfx_shape_append_line_to,
                                               EINA_TRUE))
                return ;
              break;
           case 'H':
              if (!_efl_gfx_path_parse_double_to(&content[1],
                                                 &content,
                                                 obj, pd,
                                                 &current_x, current_x, current_y,
                                                 _efl_gfx_path_append_horizontal_to,
                                                 EINA_FALSE))
                return ;
              break;
           case 'h':
              if (!_efl_gfx_path_parse_double_to(&content[1],
                                                 &content,
                                                 obj, pd,
                                                 &current_x, current_x, current_y,
                                                 _efl_gfx_path_append_horizontal_to,
                                                 EINA_TRUE))
                return ;
              break;
           case 'V':
              if (!_efl_gfx_path_parse_double_to(&content[1],
                                                 &content,
                                                 obj, pd,
                                                 &current_y, current_x, current_y,
                                                 _efl_gfx_path_append_vertical_to,
                                                 EINA_FALSE))
                return ;
              break;
           case 'v':
              if (!_efl_gfx_path_parse_double_to(&content[1],
                                                 &content,
                                                 obj, pd,
                                                 &current_y, current_x, current_y,
                                                 _efl_gfx_path_append_vertical_to,
                                                 EINA_TRUE))
                return ;
              break;
           case 'C':
              if (!_efl_gfx_path_parse_six_to(&content[1],
                                              &content,
                                              obj, pd,
                                              &current_x, &current_y,
                                              _efl_gfx_shape_append_cubic_to,
                                              EINA_FALSE))
                return ;
              break;
           case 'c':
              if (!_efl_gfx_path_parse_six_to(&content[1],
                                              &content,
                                              obj, pd,
                                              &current_x, &current_y,
                                              _efl_gfx_shape_append_cubic_to,
                                              EINA_TRUE))
                return ;
              break;
           case 'S':
              if (!_efl_gfx_path_parse_quad_to(&content[1],
                                               &content,
                                               obj, pd,
                                               &current_x, &current_y,
                                               _efl_gfx_shape_append_scubic_to,
                                               EINA_FALSE))
                return ;
              break;
           case 's':
              if (!_efl_gfx_path_parse_quad_to(&content[1],
                                               &content,
                                               obj, pd,
                                               &current_x, &current_y,
                                               _efl_gfx_shape_append_scubic_to,
                                               EINA_TRUE))
                return ;
              break;
           case 'Q':
              if (!_efl_gfx_path_parse_quad_to(&content[1],
                                               &content,
                                               obj, pd,
                                               &current_x, &current_y,
                                               _efl_gfx_shape_append_quadratic_to,
                                               EINA_FALSE))
                return ;
              break;
           case 'q':
              if (!_efl_gfx_path_parse_quad_to(&content[1],
                                               &content,
                                               obj, pd,
                                               &current_x, &current_y,
                                               _efl_gfx_shape_append_quadratic_to,
                                               EINA_TRUE))
                return ;
              break;
           case 'T':
              if (!_efl_gfx_path_parse_pair_to(&content[1],
                                               &content,
                                               obj, pd,
                                               &current_x, &current_y,
                                               _efl_gfx_shape_append_squadratic_to,
                                               EINA_FALSE))
                return ;
              break;
           case 't':
              if (!_efl_gfx_path_parse_pair_to(&content[1],
                                               &content,
                                               obj, pd,
                                               &current_x, &current_y,
                                               _efl_gfx_shape_append_squadratic_to,
                                               EINA_TRUE))
                return ;
              break;
           case 'A':
              if (!_efl_gfx_path_parse_arc_to(&content[1],
                                              &content,
                                              obj, pd,
                                              &current_x, &current_y,
                                              _efl_gfx_shape_append_arc_to,
                                              EINA_FALSE))
                return ;
              break;
           case 'a':
              if (!_efl_gfx_path_parse_arc_to(&content[1],
                                              &content,
                                              obj, pd,
                                              &current_x, &current_y,
                                              _efl_gfx_shape_append_arc_to,
                                              EINA_TRUE))
                return ;
              break;
           default:
              return ;
          }
     }
}

#include "interfaces/efl_gfx_shape.eo.c"
