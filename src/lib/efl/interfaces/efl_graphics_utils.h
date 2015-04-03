#ifndef EFL_GRAPHICS_UTILS_H_
# define EFL_GRAPHICS_UTILS_H_

EAPI Eina_Bool
efl_graphics_path_dup(Efl_Graphics_Path_Command **out_cmd, double **out_pts,
                      const Efl_Graphics_Path_Command *in_cmd, const double *in_pts);

EAPI void
efl_graphics_path_append_move_to(Efl_Graphics_Path_Command **commands, double **points,
                                 double x, double y);

EAPI void
efl_graphics_path_append_line_to(Efl_Graphics_Path_Command **commands, double **points,
                                 double x, double y);

EAPI void
efl_graphics_path_append_quadratic_to(Efl_Graphics_Path_Command **commands, double **points,
                                      double x, double y, double ctrl_x, double ctrl_y);

EAPI void
efl_graphics_path_append_squadratic_to(Efl_Graphics_Path_Command **commands, double **points,
                                       double x, double y);

EAPI void
efl_graphics_path_append_cubic_to(Efl_Graphics_Path_Command **commands, double **points,
                                  double x, double y,
                                  double ctrl_x0, double ctrl_y0,
                                  double ctrl_x1, double ctrl_y1);

EAPI void
efl_graphics_path_append_scubic_to(Efl_Graphics_Path_Command **commands, double **points,
                                   double x, double y,
                                   double ctrl_x, double ctrl_y);

EAPI void
efl_graphics_path_append_arc_to(Efl_Graphics_Path_Command **commands, double **points,
                                double x, double y,
                                double rx, double ry,
                                double angle);

EAPI void
efl_graphics_path_append_close(Efl_Graphics_Path_Command **commands, double **points);

EAPI void
efl_graphics_path_append_circle(Efl_Graphics_Path_Command **commands, double **points,
                                double x, double y, double radius);


#endif
