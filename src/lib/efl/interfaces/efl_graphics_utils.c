#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl.h>

#include <math.h>
#include <float.h>
#include <ctype.h>

static inline unsigned int
efl_graphics_path_command_length(Efl_Graphics_Path_Command command)
{
   switch (command)
     {
      case EFL_GRAPHICS_PATH_COMMAND_TYPE_END: return 0;
      case EFL_GRAPHICS_PATH_COMMAND_TYPE_MOVE_TO: return 2;
      case EFL_GRAPHICS_PATH_COMMAND_TYPE_LINE_TO: return 2;
      case EFL_GRAPHICS_PATH_COMMAND_TYPE_CUBIC_TO: return 6;
      case EFL_GRAPHICS_PATH_COMMAND_TYPE_CLOSE: return 0;
      case EFL_GRAPHICS_PATH_COMMAND_TYPE_LAST: return 0;
     }
   return 0;
}

static inline void
_efl_graphics_path_length(const Efl_Graphics_Path_Command *commands,
                          unsigned int *cmd_length,
                          unsigned int *pts_length)
{
   if (commands)
     while (commands[*cmd_length] != EFL_GRAPHICS_PATH_COMMAND_TYPE_END)
       {
          *pts_length += efl_graphics_path_command_length(commands[*cmd_length]);
          (*cmd_length)++;
       }

   // Accounting for END command and handle gracefully the NULL case at the same time
   cmd_length++;
}

static inline Eina_Bool
efl_graphics_path_grow(Efl_Graphics_Path_Command command,
                       Efl_Graphics_Path_Command **commands, double **points,
                       double **offset_point)
{
   Efl_Graphics_Path_Command *cmd_tmp;
   double *pts_tmp;
   unsigned int cmd_length = 0, pts_length = 0;

   _efl_graphics_path_length(*commands, &cmd_length, &pts_length);

   if (efl_graphics_path_command_length(command))
     {
        pts_length += efl_graphics_path_command_length(command);
        pts_tmp = realloc(*points, pts_length * sizeof (double));
        if (!pts_tmp) return EINA_FALSE;

        *points = pts_tmp;
        *offset_point = *points + pts_length - efl_graphics_path_command_length(command);
     }

   cmd_tmp = realloc(*commands,
                     (cmd_length + 1) * sizeof (Efl_Graphics_Path_Command));
   if (!cmd_tmp) return EINA_FALSE;
   *commands = cmd_tmp;

   // Append the command
   cmd_tmp[cmd_length - 1] = command;
   // NULL terminate the stream
   cmd_tmp[cmd_length] = EFL_GRAPHICS_PATH_COMMAND_TYPE_END;

   return EINA_TRUE;
}

EAPI Eina_Bool
efl_graphics_path_dup(Efl_Graphics_Path_Command **out_cmd, double **out_pts,
                      const Efl_Graphics_Path_Command *in_cmd, const double *in_pts)
{
   unsigned int cmd_length = 0, pts_length = 0;

   _efl_graphics_path_length(in_cmd, &cmd_length, &pts_length);

   *out_pts = malloc(pts_length * sizeof (double));
   *out_cmd = malloc(cmd_length * sizeof (Efl_Graphics_Path_Command));
   if (!(*out_pts) || !(*out_cmd))
     {
        free(*out_pts);
        free(*out_cmd);
        return EINA_FALSE;
     }

   memcpy(*out_pts, in_pts, pts_length * sizeof (double));
   memcpy(*out_cmd, in_cmd, cmd_length * sizeof (Efl_Graphics_Path_Command));
   return EINA_TRUE;
}

EAPI Eina_Bool
efl_graphics_path_equal_commands(const Efl_Graphics_Path_Command *a,
                                 const Efl_Graphics_Path_Command *b)
{
   unsigned int i;

   if (!a && !b) return EINA_TRUE;
   if (!a || !b) return EINA_FALSE;

   for (i = 0; a[i] == b[i] && a[i] != EFL_GRAPHICS_PATH_COMMAND_TYPE_END; i++)
     ;

   return a[i] == b[i];
}

EAPI void
efl_graphics_path_interpolate(const Efl_Graphics_Path_Command *cmd,
                              double pos_map,
                              const double *from, const double *to, double *r)
{
   unsigned int i;
   unsigned int j;

   if (!cmd) return ;

   for (i = 0; cmd[i] != EFL_GRAPHICS_PATH_COMMAND_TYPE_END; i++)
     for (j = 0; j < efl_graphics_path_command_length(cmd[i]); j++)
       *r = (*from) * pos_map + ((*to) * (1.0 - pos_map));
}

EAPI Eina_Bool
efl_graphics_path_current_get(const Efl_Graphics_Path_Command *cmd,
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

   for (i = 0; cmd[i] != EFL_GRAPHICS_PATH_COMMAND_TYPE_END; i++)
     {
        switch (cmd[i])
          {
           case EFL_GRAPHICS_PATH_COMMAND_TYPE_END:
              break;
           case EFL_GRAPHICS_PATH_COMMAND_TYPE_MOVE_TO:
           case EFL_GRAPHICS_PATH_COMMAND_TYPE_LINE_TO:
              if (current_x) *current_x = points[0];
              if (current_y) *current_y = points[1];

              points += 2;
              break;
           case EFL_GRAPHICS_PATH_COMMAND_TYPE_CUBIC_TO:
              if (current_x) *current_x = points[0];
              if (current_y) *current_y = points[1];
              if (current_ctrl_x) *current_ctrl_x = points[4];
              if (current_ctrl_y) *current_ctrl_y = points[5];

              points += 6;
              break;
           case EFL_GRAPHICS_PATH_COMMAND_TYPE_CLOSE:
              break;
           case EFL_GRAPHICS_PATH_COMMAND_TYPE_LAST:
           default:
              return EINA_FALSE;
          }
     }

   return EINA_TRUE;
}

EAPI void
efl_graphics_path_append_move_to(Efl_Graphics_Path_Command **commands, double **points,
                                 double x, double y)
{
   double *offset_point;

   if (!efl_graphics_path_grow(EFL_GRAPHICS_PATH_COMMAND_TYPE_MOVE_TO,
                          commands, points, &offset_point))
     return ;

   offset_point[0] = x;
   offset_point[1] = y;
}

EAPI void
efl_graphics_path_append_line_to(Efl_Graphics_Path_Command **commands, double **points,
                                 double x, double y)
{
   double *offset_point;

   if (!efl_graphics_path_grow(EFL_GRAPHICS_PATH_COMMAND_TYPE_LINE_TO,
                          commands, points, &offset_point))
     return ;

   offset_point[0] = x;
   offset_point[1] = y;
}

EAPI void
efl_graphics_path_append_quadratic_to(Efl_Graphics_Path_Command **commands, double **points,
                                      double x, double y, double ctrl_x, double ctrl_y)
{
   double current_x = 0, current_y = 0;
   double ctrl_x0, ctrl_y0, ctrl_x1, ctrl_y1;

   if (!efl_graphics_path_current_get(*commands, *points,
                                      &current_x, &current_y,
                                      NULL, NULL))
     return ;

   // Convert quadratic bezier to cubic
   ctrl_x0 = (current_x + 2 * ctrl_x) * (1.0 / 3.0);
   ctrl_y0 = (current_y + 2 * ctrl_y) * (1.0 / 3.0);
   ctrl_x1 = (x + 2 * ctrl_x) * (1.0 / 3.0);
   ctrl_y1 = (y + 2 * ctrl_y) * (1.0 / 3.0);

   efl_graphics_path_append_cubic_to(commands, points, x, y,
                                     ctrl_x0, ctrl_y0, ctrl_x1, ctrl_y1);
}

EAPI void
efl_graphics_path_append_squadratic_to(Efl_Graphics_Path_Command **commands, double **points,
                                       double x, double y)
{
   double xc, yc; /* quadratic control point */
   double ctrl_x0, ctrl_y0, ctrl_x1, ctrl_y1;
   double current_x = 0, current_y = 0;
   double current_ctrl_x = 0, current_ctrl_y = 0;

   if (!efl_graphics_path_current_get(*commands, *points,
                                      &current_x, &current_y,
                                      &current_ctrl_x, &current_ctrl_y))
     return ;

   xc = 2 * current_x - current_ctrl_x;
   yc = 2 * current_y - current_ctrl_y;
   /* generate a quadratic bezier with control point = xc, yc */
   ctrl_x0 = (current_x + 2 * xc) * (1.0 / 3.0);
   ctrl_y0 = (current_y + 2 * yc) * (1.0 / 3.0);
   ctrl_x1 = (x + 2 * xc) * (1.0 / 3.0);
   ctrl_y1 = (y + 2 * yc) * (1.0 / 3.0);

   efl_graphics_path_append_cubic_to(commands, points, x, y,
                                     ctrl_x0, ctrl_y0,
                                     ctrl_x1, ctrl_y1);
}

EAPI void
efl_graphics_path_append_cubic_to(Efl_Graphics_Path_Command **commands, double **points,
                                  double x, double y,
                                  double ctrl_x0, double ctrl_y0,
                                  double ctrl_x1, double ctrl_y1)
{
   double *offset_point;

   if (!efl_graphics_path_grow(EFL_GRAPHICS_PATH_COMMAND_TYPE_CUBIC_TO,
                          commands, points, &offset_point))
     return ;

   offset_point[0] = x;
   offset_point[1] = y;
   offset_point[2] = ctrl_x0;
   offset_point[3] = ctrl_y0;
   offset_point[4] = ctrl_x1;
   offset_point[5] = ctrl_y1;
}

EAPI void
efl_graphics_path_append_scubic_to(Efl_Graphics_Path_Command **commands, double **points,
                                   double x, double y,
                                   double ctrl_x, double ctrl_y)
{
   double ctrl_x0, ctrl_y0;
   double current_x = 0, current_y = 0;
   double current_ctrl_x = 0, current_ctrl_y = 0;

   if (!efl_graphics_path_current_get(*commands, *points,
                                      &current_x, &current_y,
                                      &current_ctrl_x, &current_ctrl_y))
     return ;

   ctrl_x0 = 2 * current_x - current_ctrl_x;
   ctrl_y0 = 2 * current_y - current_ctrl_y;

   efl_graphics_path_append_cubic_to(commands, points, x, y,
                                     ctrl_x0, ctrl_y0, ctrl_x, ctrl_y);
}

// This function come from librsvg rsvg-path.c
static void
_efl_graphics_path_append_arc_segment(Efl_Graphics_Path_Command **commands, double **points,
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

   efl_graphics_path_append_cubic_to(commands, points,
                                     xc + cosf * x3 - sinf * y3,
                                     yc + sinf * x3 + cosf * y3,
                                     xc + cosf * x1 - sinf * y1,
                                     yc + sinf * x1 + cosf * y1,
                                     xc + cosf * x2 - sinf * y2,
                                     yc + sinf * x2 + cosf * y2);
}

// This function come from librsvg rsvg-path.c
EAPI void
efl_graphics_path_append_arc_to(Efl_Graphics_Path_Command **commands, double **points,
                                double x, double y,
                                double rx, double ry, double angle,
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

   if (!efl_graphics_path_current_get(*commands, *points,
                                      &x1, &y1,
                                      NULL, NULL))
     return ;

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
        efl_graphics_path_append_line_to(commands, points, x, y);
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
     _efl_graphics_path_append_arc_segment(commands, points,
                                           cx, cy,
                                           theta1 + i * delta_theta / n_segs,
                                           theta1 + (i + 1) * delta_theta / n_segs,
                                           rx, ry, angle);
}

EAPI void
efl_graphics_path_append_close(Efl_Graphics_Path_Command **commands, double **points)
{
   double *offset_point;

   efl_graphics_path_grow(EFL_GRAPHICS_PATH_COMMAND_TYPE_CLOSE,
                          commands, points, &offset_point);
}

EAPI void
efl_graphics_path_append_circle(Efl_Graphics_Path_Command **commands, double **points,
                                double x, double y, double radius)
{
   efl_graphics_path_append_move_to(commands, points, x, y - radius);
   efl_graphics_path_append_arc_to(commands, points, x - radius, y, radius, radius, 0, EINA_FALSE, EINA_FALSE);
   efl_graphics_path_append_arc_to(commands, points, x, y + radius, radius, radius, 0, EINA_FALSE, EINA_FALSE);
   efl_graphics_path_append_arc_to(commands, points, x + radius, y, radius, radius, 0, EINA_FALSE, EINA_FALSE);
   efl_graphics_path_append_arc_to(commands, points, x, y - radius, radius, radius, 0, EINA_FALSE, EINA_FALSE);
}

static void
_efl_graphics_path_append_horizontal_to(Efl_Graphics_Path_Command **commands, double **points,
                                        double d, double current_x EINA_UNUSED, double current_y)
{
   efl_graphics_path_append_line_to(commands, points, d, current_y);
}

static void
_efl_graphics_path_append_vertical_to(Efl_Graphics_Path_Command **commands, double **points,
                                        double d, double current_x, double current_y EINA_UNUSED)
{
   efl_graphics_path_append_line_to(commands, points, current_x, d);
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
_efl_graphics_path_parse_pair(const char *content, char **end, double *x, double *y)
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
_efl_graphics_path_parse_pair_to(const char *content, char **end,
                                 Efl_Graphics_Path_Command **commands, double **points,
                                 double *current_x, double *current_y,
                                 void (*func)(Efl_Graphics_Path_Command **commands, double **points, double x, double y),
                                 Eina_Bool rel)
{
   double x, y;

   *end = (char*) content;
   do
     {
        Eina_Bool r;

        r = _efl_graphics_path_parse_pair(content, end, &x, &y);
        if (!r) return EINA_FALSE;

        if (rel)
          {
             x += *current_x;
             y += *current_y;
          }

        func(commands, points, x, y);
        content = *end;

        *current_x = x;
        *current_y = y;
     }
   while (_next_isnumber(content));

   return EINA_TRUE;
}

static Eina_Bool
_efl_graphics_path_parse_double_to(const char *content, char **end,
                                   Efl_Graphics_Path_Command **commands, double **points,
                                   double *current, double current_x, double current_y,
                                   void (*func)(Efl_Graphics_Path_Command **commands, double **points, double d, double current_x, double current_y),
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

        func(commands, points, d, current_x, current_y);
        content = *end;

        *current = d;
     }
   while (1); // This is an optimisation as we have only one parameter.

   return EINA_TRUE;
}

static Eina_Bool
_efl_graphics_path_parse_six(const char *content, char **end,
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
_efl_graphics_path_parse_six_to(const char *content, char **end,
                                Efl_Graphics_Path_Command **commands, double **points,
                                double *current_x, double *current_y,
                                void (*func)(Efl_Graphics_Path_Command **commands, double **points, double x, double y, double ctrl_x0, double ctrl_y0, double ctrl_x1, double ctrl_y1),
                                 Eina_Bool rel)
{
   double x, y, ctrl_x0, ctrl_y0, ctrl_x1, ctrl_y1;

   *end = (char*) content;
   do
     {
        Eina_Bool r;

        r = _efl_graphics_path_parse_six(content, end,
                                         &x, &y,
                                         &ctrl_x0, &ctrl_y0,
                                         &ctrl_x1, &ctrl_y1);
        if (!r) return EINA_FALSE;

        if (rel)
          {
             x += *current_x;
             y += *current_y;
          }

        func(commands, points, x, y, ctrl_x0, ctrl_y0, ctrl_x1, ctrl_y1);
        content = *end;

        *current_x = x;
        *current_y = y;
     }
   while (_next_isnumber(content));

   return EINA_TRUE;
}

static Eina_Bool
_efl_graphics_path_parse_quad(const char *content, char **end,
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
_efl_graphics_path_parse_quad_to(const char *content, char **end,
                                 Efl_Graphics_Path_Command **commands, double **points,
                                 double *current_x, double *current_y,
                                 void (*func)(Efl_Graphics_Path_Command **commands, double **points,
                                              double x, double y, double ctrl_x0, double ctrl_y0),
                                 Eina_Bool rel)
{
   double x, y, ctrl_x0, ctrl_y0;

   *end = (char*) content;
   do
     {
        Eina_Bool r;

        r = _efl_graphics_path_parse_quad(content, end,
                                          &x, &y,
                                          &ctrl_x0, &ctrl_y0);
        if (!r) return EINA_FALSE;

        if (rel)
          {
             x += *current_x;
             y += *current_y;
          }

        func(commands, points, x, y, ctrl_x0, ctrl_y0);
        content = *end;

        *current_x = x;
        *current_y = y;
     }
   while (_next_isnumber(content));

   return EINA_TRUE;
}

static Eina_Bool
_efl_graphics_path_parse_arc(const char *content, char **end,
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
_efl_graphics_path_parse_arc_to(const char *content, char **end,
                                Efl_Graphics_Path_Command **commands, double **points,
                                double *current_x, double *current_y,
                                void (*func)(Efl_Graphics_Path_Command **commands, double **points,
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

        r = _efl_graphics_path_parse_arc(content, end,
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

        func(commands, points, x, y, rx, ry, angle, large_arc, sweep);
        content = *end;

        *current_x = x;
        *current_y = y;
     }
   while (_next_isnumber(content));

   return EINA_TRUE;
}

EAPI Eina_Bool
efl_graphics_path_append_svg_path(Efl_Graphics_Path_Command **commands, double **points, const char *svg_path_data)
{
   double current_x = 0, current_y = 0;
   char *content = (char*) svg_path_data;

   if (!content) return EINA_FALSE;

   while (content[0] != '\0')
     {
        while (isspace(content[0])) content++;

        switch (content[0])
          {
           case 'M':
              if (!_efl_graphics_path_parse_pair_to(&content[1],
                                                    &content,
                                                    commands, points,
                                                    &current_x, &current_y,
                                                    efl_graphics_path_append_move_to,
                                                    EINA_FALSE))
                return EINA_FALSE;
              break;
           case 'm':
              if (!_efl_graphics_path_parse_pair_to(&content[1],
                                                    &content,
                                                    commands, points,
                                                    &current_x, &current_y,
                                                    efl_graphics_path_append_move_to,
                                                    EINA_TRUE))
                return EINA_FALSE;
              break;
           case 'z':
              efl_graphics_path_append_close(commands, points);
              content++;
              break;
           case 'L':
              if (!_efl_graphics_path_parse_pair_to(&content[1],
                                                    &content,
                                                    commands, points,
                                                    &current_x, &current_y,
                                                    efl_graphics_path_append_line_to,
                                                    EINA_FALSE))
                return EINA_FALSE;
              break;
           case 'l':
              if (!_efl_graphics_path_parse_pair_to(&content[1],
                                                    &content,
                                                    commands, points,
                                                    &current_x, &current_y,
                                                    efl_graphics_path_append_line_to,
                                                    EINA_TRUE))
                return EINA_FALSE;
              break;
           case 'H':
              if (!_efl_graphics_path_parse_double_to(&content[1],
                                                      &content,
                                                      commands, points,
                                                      &current_x, current_x, current_y,
                                                      _efl_graphics_path_append_horizontal_to,
                                                      EINA_FALSE))
                return EINA_FALSE;
              break;
           case 'h':
              if (!_efl_graphics_path_parse_double_to(&content[1],
                                                      &content,
                                                      commands, points,
                                                      &current_x, current_x, current_y,
                                                      _efl_graphics_path_append_horizontal_to,
                                                      EINA_TRUE))
                return EINA_FALSE;
              break;
           case 'V':
              if (!_efl_graphics_path_parse_double_to(&content[1],
                                                      &content,
                                                      commands, points,
                                                      &current_y, current_x, current_y,
                                                      _efl_graphics_path_append_vertical_to,
                                                      EINA_FALSE))
                return EINA_FALSE;
              break;
           case 'v':
              if (!_efl_graphics_path_parse_double_to(&content[1],
                                                      &content,
                                                      commands, points,
                                                      &current_y, current_x, current_y,
                                                      _efl_graphics_path_append_vertical_to,
                                                      EINA_TRUE))
                return EINA_FALSE;
              break;
           case 'C':
              if (!_efl_graphics_path_parse_six_to(&content[1],
                                                   &content,
                                                   commands, points,
                                                   &current_x, &current_y,
                                                   efl_graphics_path_append_cubic_to,
                                                   EINA_FALSE))
                  return EINA_FALSE;
              break;
           case 'c':
              if (!_efl_graphics_path_parse_six_to(&content[1],
                                                   &content,
                                                   commands, points,
                                                   &current_x, &current_y,
                                                   efl_graphics_path_append_cubic_to,
                                                   EINA_TRUE))
                return EINA_FALSE;
              break;
           case 'S':
              if (!_efl_graphics_path_parse_quad_to(&content[1],
                                                    &content,
                                                    commands, points,
                                                    &current_x, &current_y,
                                                    efl_graphics_path_append_scubic_to,
                                                    EINA_FALSE))
                return EINA_FALSE;
              break;
           case 's':
              if (!_efl_graphics_path_parse_quad_to(&content[1],
                                                    &content,
                                                    commands, points,
                                                    &current_x, &current_y,
                                                    efl_graphics_path_append_scubic_to,
                                                    EINA_TRUE))
                return EINA_FALSE;
              break;
           case 'Q':
              if (!_efl_graphics_path_parse_quad_to(&content[1],
                                                    &content,
                                                    commands, points,
                                                    &current_x, &current_y,
                                                    efl_graphics_path_append_quadratic_to,
                                                    EINA_FALSE))
                return EINA_FALSE;
              break;
           case 'q':
              if (!_efl_graphics_path_parse_quad_to(&content[1],
                                                    &content,
                                                    commands, points,
                                                    &current_x, &current_y,
                                                    efl_graphics_path_append_quadratic_to,
                                                    EINA_TRUE))
                return EINA_FALSE;
              break;
           case 'T':
              if (!_efl_graphics_path_parse_pair_to(&content[1],
                                                    &content,
                                                    commands, points,
                                                    &current_x, &current_y,
                                                    efl_graphics_path_append_squadratic_to,
                                                    EINA_FALSE))
                return EINA_FALSE;
              break;
           case 't':
              if (!_efl_graphics_path_parse_pair_to(&content[1],
                                                    &content,
                                                    commands, points,
                                                    &current_x, &current_y,
                                                    efl_graphics_path_append_squadratic_to,
                                                    EINA_TRUE))
                return EINA_FALSE;
              break;
           case 'A':
                if (!_efl_graphics_path_parse_arc_to(&content[1],
                                                     &content,
                                                     commands, points,
                                                     &current_x, &current_y,
                                                     efl_graphics_path_append_arc_to,
                                                     EINA_FALSE))
                  return EINA_FALSE;
              break;
           case 'a':
              if (!_efl_graphics_path_parse_arc_to(&content[1],
                                                   &content,
                                                   commands, points,
                                                   &current_x, &current_y,
                                                   efl_graphics_path_append_arc_to,
                                                   EINA_TRUE))
                return EINA_FALSE;
              break;
           default:
              return EINA_FALSE;
          }
     }

   return EINA_TRUE;
}
