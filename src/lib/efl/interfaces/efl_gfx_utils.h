#ifndef EFL_GRAPHICS_UTILS_H_
# define EFL_GRAPHICS_UTILS_H_

EAPI Eina_Bool
efl_gfx_path_dup(Efl_Gfx_Path_Command **out_cmd, double **out_pts,
                 const Efl_Gfx_Path_Command *in_cmd, const double *in_pts);

EAPI void
efl_gfx_path_append_move_to(Efl_Gfx_Path_Command **commands, double **points,
                            double x, double y);

EAPI void
efl_gfx_path_append_line_to(Efl_Gfx_Path_Command **commands, double **points,
                            double x, double y);

EAPI void
efl_gfx_path_append_quadratic_to(Efl_Gfx_Path_Command **commands, double **points,
                                 double x, double y, double ctrl_x, double ctrl_y);

EAPI void
efl_gfx_path_append_squadratic_to(Efl_Gfx_Path_Command **commands, double **points,
                                  double x, double y);

EAPI void
efl_gfx_path_append_cubic_to(Efl_Gfx_Path_Command **commands, double **points,
                             double x, double y,
                             double ctrl_x0, double ctrl_y0,
                             double ctrl_x1, double ctrl_y1);

EAPI void
efl_gfx_path_append_scubic_to(Efl_Gfx_Path_Command **commands, double **points,
                              double x, double y,
                              double ctrl_x, double ctrl_y);

EAPI void
efl_gfx_path_append_arc_to(Efl_Gfx_Path_Command **commands, double **points,
                           double x, double y,
                           double rx, double ry,
                           double angle,
                           Eina_Bool large_arc, Eina_Bool sweep);

EAPI void
efl_gfx_path_append_close(Efl_Gfx_Path_Command **commands, double **points);

EAPI void
efl_gfx_path_append_circle(Efl_Gfx_Path_Command **commands, double **points,
                           double x, double y, double radius);

EAPI Eina_Bool
efl_gfx_path_append_svg_path(Efl_Gfx_Path_Command **commands, double **points, const char *svg_path_data);

EAPI void
efl_gfx_path_interpolate(const Efl_Gfx_Path_Command *cmd,
                         double pos_map,
                         const double *from, const double *to, double *r);

EAPI Eina_Bool
efl_gfx_path_equal_commands(const Efl_Gfx_Path_Command *a,
                            const Efl_Gfx_Path_Command *b);

EAPI Eina_Bool
efl_gfx_path_current_get(const Efl_Gfx_Path_Command *cmd,
                         const double *points,
                         double *current_x, double *current_y,
                         double *current_ctrl_x, double *current_ctrl_y);

#endif
