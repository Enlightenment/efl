
/*initialization navigation panel*/
Evas_Object *
init_panel_camera_light(Evas_Object *win, Eo* camera_node, Eo* light_node, double posX, double posY);
/*free resources*/
void
panel_camera_light_fini(Evas_Object *navigation);
/*initialization graphical objects*/
Evas_Object *
init_graphical_window(Evas_Object *image);
/*Set step of range*/
Eina_Bool
panel_camera_coord_step_set(Evas_Object *navigation, int step);
Eina_Bool
panel_camera_angle_step_set(Evas_Object *navigation, int step);
Eina_Bool
panel_light_coord_step_set(Evas_Object *navigation, int step);
Eina_Bool
panel_light_angle_step_set(Evas_Object *navigation, int step);
