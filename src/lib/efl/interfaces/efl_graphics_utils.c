#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl.h>

static inline unsigned int
efl_graphics_path_command_length(Efl_Graphics_Path_Command command)
{
   switch (command)
     {
      case EFL_GRAPHICS_PATH_COMMAND_TYPE_END: return 0;
      case EFL_GRAPHICS_PATH_COMMAND_TYPE_MOVE_TO: return 2;
      case EFL_GRAPHICS_PATH_COMMAND_TYPE_LINE_TO: return 2;
      case EFL_GRAPHICS_PATH_COMMAND_TYPE_QUADRATIC_TO: return 4;
      case EFL_GRAPHICS_PATH_COMMAND_TYPE_SQUADRATIC_TO: return 2;
      case EFL_GRAPHICS_PATH_COMMAND_TYPE_CUBIC_TO: return 6;
      case EFL_GRAPHICS_PATH_COMMAND_TYPE_SCUBIC_TO: return 4;
      case EFL_GRAPHICS_PATH_COMMAND_TYPE_ARC_TO: return 5;
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
   double *offset_point;

   if (!efl_graphics_path_grow(EFL_GRAPHICS_PATH_COMMAND_TYPE_QUADRATIC_TO,
                          commands, points, &offset_point))
     return ;

   offset_point[0] = x;
   offset_point[1] = y;
   offset_point[2] = ctrl_x;
   offset_point[3] = ctrl_y;
}

EAPI void
efl_graphics_path_append_squadratic_to(Efl_Graphics_Path_Command **commands, double **points,
                                       double x, double y)
{
   double *offset_point;

   if (!efl_graphics_path_grow(EFL_GRAPHICS_PATH_COMMAND_TYPE_SQUADRATIC_TO,
                          commands, points, &offset_point))
     return ;

   offset_point[0] = x;
   offset_point[1] = y;
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
   double *offset_point;

   if (!efl_graphics_path_grow(EFL_GRAPHICS_PATH_COMMAND_TYPE_SCUBIC_TO,
                          commands, points, &offset_point))
     return ;

   offset_point[0] = x;
   offset_point[1] = y;
   offset_point[2] = ctrl_x;
   offset_point[3] = ctrl_y;
}

EAPI void
efl_graphics_path_append_arc_to(Efl_Graphics_Path_Command **commands, double **points,
                                double x, double y,
                                double rx, double ry,
                                double angle)
{
   double *offset_point;

   if (!efl_graphics_path_grow(EFL_GRAPHICS_PATH_COMMAND_TYPE_ARC_TO,
                          commands, points, &offset_point))
     return ;

   offset_point[0] = x;
   offset_point[1] = y;
   offset_point[2] = rx;
   offset_point[3] = ry;
   offset_point[4] = angle;
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
   efl_graphics_path_append_arc_to(commands, points, x + radius, y, radius, radius, 0);
   efl_graphics_path_append_arc_to(commands, points, x, y + radius, radius, radius, 0);
   efl_graphics_path_append_arc_to(commands, points, x - radius, y, radius, radius, 0);
   efl_graphics_path_append_arc_to(commands, points, x, y - radius, radius, radius, 0);
}
