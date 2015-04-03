#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

static unsigned int
evas_vg_path_command_length(Evas_VG_Path_Command command)
{
   switch (command)
     {
      case EVAS_VG_PATH_COMMAND_TYPE_END: return 0;
      case EVAS_VG_PATH_COMMAND_TYPE_MOVE_TO: return 2;
      case EVAS_VG_PATH_COMMAND_TYPE_LINE_TO: return 2;
      case EVAS_VG_PATH_COMMAND_TYPE_QUADRATIC_TO: return 4;
      case EVAS_VG_PATH_COMMAND_TYPE_SQUADRATIC_TO: return 2;
      case EVAS_VG_PATH_COMMAND_TYPE_CUBIC_TO: return 6;
      case EVAS_VG_PATH_COMMAND_TYPE_SCUBIC_TO: return 4;
      case EVAS_VG_PATH_COMMAND_TYPE_ARC_TO: return 5;
      case EVAS_VG_PATH_COMMAND_TYPE_CLOSE: return 0;
      case EVAS_VG_PATH_COMMAND_TYPE_LAST: return 0;
     }
   return 0;
}

static inline void
_evas_vg_path_length(Evas_VG_Path_Command *commands,
                     unsigned int *cmd_length,
                     unsigned int *pts_length)
{
   if (commands)
     while (commands[*cmd_length] != EVAS_VG_PATH_COMMAND_TYPE_END)
       {
          *pts_length += evas_vg_path_command_length(commands[*cmd_length]);
          (*cmd_length)++;
       }

   // Accounting for END command and handle gracefully the NULL case at the same time
   cmd_length++;
}

static inline Eina_Bool
evas_vg_path_grow(Evas_VG_Path_Command command,
                  Evas_VG_Path_Command **commands, double **points,
                  double **offset_point)
{
   Evas_VG_Path_Command *cmd_tmp;
   double *pts_tmp;
   unsigned int cmd_length = 0, pts_length = 0;

   _evas_vg_path_length(commands, &cmd_length, &pts_length);

   if (evas_vg_path_command_length(command))
     {
        pts_length += evas_vg_path_command_length(command);
        pts_tmp = realloc(*points, pts_length * sizeof (double));
        if (!pts_tmp) return EINA_FALSE;

        *points = pts_tmp;
        *offset_point = *points + pts_length - evas_vg_path_command_length(command);
     }

   cmd_tmp = realloc(*commands,
                     (cmd_length + 1) * sizeof (Evas_VG_Path_Command));
   if (!cmd_tmp) return EINA_FALSE;
   *commands = cmd_tmp;

   // Append the command
   cmd_tmp[cmd_length - 1] = command;
   // NULL terminate the stream
   cmd_tmp[cmd_length] = EVAS_VG_PATH_COMMAND_TYPE_END;

   return EINA_TRUE;
}

Eina_Bool
evas_vg_path_dup(Evas_VG_Path_Command **out_cmd, double **out_pts,
                 Evas_VG_Path_Command *in_cmd, double *in_pts)
{
   unsigned int cmd_length = 0, pts_length = 0;

   _evas_vg_path_length(in_cmd, &cmd_length, &pts_length);

   *out_pts = malloc(pts_length * sizeof (double));
   *out_cmd = malloc(cmd_length * sizeof (Evas_VG_Path_Command));
   if (!(*out_pts) || !(*out_cmd))
     {
        free(*out_pts);
        free(*out_cmd);
        return EINA_FALSE;
     }

   memcpy(*out_pts, in_pts, pts_length * sizeof (double));
   memcpy(*out_cmd, in_cmd, cmd_length * sizeof (Evas_VG_Path_Command));
   return EINA_TRUE;
}

void
evas_vg_path_append_move_to(Evas_VG_Path_Command **commands, double **points,
                            double x, double y)
{
   double *offset_point;

   if (!evas_vg_path_grow(EVAS_VG_PATH_COMMAND_TYPE_MOVE_TO,
                          commands, points, &offset_point))
     return ;

   offset_point[0] = x;
   offset_point[1] = y;
}

void
evas_vg_path_append_line_to(Evas_VG_Path_Command **commands, double **points,
                            double x, double y)
{
   double *offset_point;

   if (!evas_vg_path_grow(EVAS_VG_PATH_COMMAND_TYPE_LINE_TO,
                          commands, points, &offset_point))
     return ;

   offset_point[0] = x;
   offset_point[1] = y;
}

void
evas_vg_path_append_quadratic_to(Evas_VG_Path_Command **commands, double **points,
                                 double x, double y, double ctrl_x, double ctrl_y)
{
   double *offset_point;

   if (!evas_vg_path_grow(EVAS_VG_PATH_COMMAND_TYPE_QUADRATIC_TO,
                          commands, points, &offset_point))
     return ;

   offset_point[0] = x;
   offset_point[1] = y;
   offset_point[2] = ctrl_x;
   offset_point[3] = ctrl_y;
}

void
evas_vg_path_append_squadratic_to(Evas_VG_Path_Command **commands, double **points,
                                  double x, double y)
{
   double *offset_point;

   if (!evas_vg_path_grow(EVAS_VG_PATH_COMMAND_TYPE_SQUADRATIC_TO,
                          commands, points, &offset_point))
     return ;

   offset_point[0] = x;
   offset_point[1] = y;
}

void
evas_vg_path_append_cubic_to(Evas_VG_Path_Command **commands, double **points,
                             double x, double y,
                             double ctrl_x0, double ctrl_y0,
                             double ctrl_x1, double ctrl_y1)
{
   double *offset_point;

   if (!evas_vg_path_grow(EVAS_VG_PATH_COMMAND_TYPE_CUBIC_TO,
                          commands, points, &offset_point))
     return ;

   offset_point[0] = x;
   offset_point[1] = y;
   offset_point[2] = ctrl_x0;
   offset_point[3] = ctrl_y0;
   offset_point[4] = ctrl_x1;
   offset_point[5] = ctrl_y1;
}

void
evas_vg_path_append_scubic_to(Evas_VG_Path_Command **commands, double **points,
                              double x, double y,
                              double ctrl_x, double ctrl_y)
{
   double *offset_point;

   if (!evas_vg_path_grow(EVAS_VG_PATH_COMMAND_TYPE_SCUBIC_TO,
                          commands, points, &offset_point))
     return ;

   offset_point[0] = x;
   offset_point[1] = y;
   offset_point[2] = ctrl_x;
   offset_point[3] = ctrl_y;
}

void
evas_vg_path_append_arc_to(Evas_VG_Path_Command **commands, double **points,
                           double x, double y,
                           double rx, double ry,
                           double angle)
{
   double *offset_point;

   if (!evas_vg_path_grow(EVAS_VG_PATH_COMMAND_TYPE_ARC_TO,
                          commands, points, &offset_point))
     return ;

   offset_point[0] = x;
   offset_point[1] = y;
   offset_point[2] = rx;
   offset_point[3] = ry;
   offset_point[4] = angle;
}

void
evas_vg_path_append_close(Evas_VG_Path_Command **commands, double **points)
{
   double *offset_point;

   evas_vg_path_grow(EVAS_VG_PATH_COMMAND_TYPE_ARC_TO,
                     commands, points, &offset_point);
}

void
evas_vg_path_append_circle(Evas_VG_Path_Command **commands, double **points,
                           double x, double y, double radius)
{
   evas_vg_path_append_move_to(commands, points, x, y - radius);
   evas_vg_path_append_arc_to(commands, points, x + radius, y, radius, radius, 0);
   evas_vg_path_append_arc_to(commands, points, x, y + radius, radius, radius, 0);
   evas_vg_path_append_arc_to(commands, points, x - radius, y, radius, radius, 0);
   evas_vg_path_append_arc_to(commands, points, x, y - radius, radius, radius, 0);
}
