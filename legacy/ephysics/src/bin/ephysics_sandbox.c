#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include<stdio.h>
#include<string.h>
#include<Eina.h>
#include<Elementary.h>
#include<EPhysics.h>

#define SANDBOX_THEME "ephysics_sandbox"
#define WIDTH (1280)
#define HEIGHT (720)

typedef struct _Sandie_Data {
   Evas_Object *win;
   EPhysics_World *world;
   EPhysics_Body *body1, *body2;
} Sandie_Data;

static void
_body_del(void *data __UNUSED__, EPhysics_Body *body __UNUSED__,
          void *event_info)
{
   if (event_info) evas_object_del(event_info);
}

static void
_world_gravity_x_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   EPhysics_World *world = data;
   double gravity_y;

   ephysics_world_gravity_get(world, NULL, &gravity_y);
   ephysics_world_gravity_set(world, elm_spinner_value_get(obj), gravity_y);
}

static void
_world_gravity_y_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   EPhysics_World *world = data;
   double gravity_x;

   ephysics_world_gravity_get(world, &gravity_x, NULL);
   ephysics_world_gravity_set(world, gravity_x, elm_spinner_value_get(obj));
}

static void
_world_rate_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   EPhysics_World *world = data;
   ephysics_world_rate_set(world, elm_spinner_value_get(obj));
}

static void
_world_max_sleeping_time_cb(void *data, Evas_Object *obj,
                            void *event_info __UNUSED__)
{
   EPhysics_World *world = data;
   ephysics_world_max_sleeping_time_set(world, elm_spinner_value_get(obj));
}

static void
_world_friction_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *win = data;

   ephysics_body_friction_set(evas_object_data_get(win, "top"),
                              elm_spinner_value_get(obj));
   ephysics_body_friction_set(evas_object_data_get(win, "bottom"),
                              elm_spinner_value_get(obj));
   ephysics_body_friction_set(evas_object_data_get(win, "left"),
                              elm_spinner_value_get(obj));
   ephysics_body_friction_set(evas_object_data_get(win, "right"),
                              elm_spinner_value_get(obj));
}

static void
_world_restitution_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *win = data;

   ephysics_body_restitution_set(evas_object_data_get(win, "top"),
                                 elm_spinner_value_get(obj));
   ephysics_body_restitution_set(evas_object_data_get(win, "bottom"),
                                 elm_spinner_value_get(obj));
   ephysics_body_restitution_set(evas_object_data_get(win, "left"),
                                 elm_spinner_value_get(obj));
   ephysics_body_restitution_set(evas_object_data_get(win, "right"),
                                 elm_spinner_value_get(obj));
}
/*
static void
_body_type_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_Body *body = data;
   EPhysics_World *world;
   Evas_Object *body_image;
   double mass, rotation, friction, restitution, lin_damping, ang_damping;
   double lin_sleeping, ang_sleeping;

   mass = ephysics_body_mass_get(body);
   rotation = ephysics_body_rotation_get(body);
   friction = ephysics_body_friction_get(body);
   restitution = ephysics_body_restitution_get(body);
   ephysics_body_damping_get(body, &lin_damping, &ang_damping);
   ephysics_body_sleeping_threshold_get(body, &lin_sleeping, &ang_sleeping);
   body_image = ephysics_body_evas_object_unset(body);
   world = ephysics_body_world_get(body);

   ephysics_body_del(body);

   if (elm_check_state_get(obj))
     {
        body = ephysics_body_soft_circle_add(world);
        ephysics_body_soft_body_hardness_set(body, 10);
        elm_object_disabled_set(evas_object_data_get(obj, "hardness"), EINA_FALSE);
     }
   else
     {
        body = ephysics_body_circle_add(world);
        elm_object_disabled_set(evas_object_data_get(obj, "hardness"), EINA_TRUE);
     }

   body_image = ephysics_body_evas_object_set(body, body_image, EINA_TRUE);
   ephysics_body_mass_set(body, mass);
   ephysics_body_rotation_set(body, rotation);
   ephysics_body_friction_set(body, friction);
}
*/
static void
_body_material_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   EPhysics_Body *body = data;
   Evas_Object *body_image;
   const char *material;
   int x, y, w, h;

   ephysics_body_material_set(body, elm_spinner_value_get(obj));

   elm_spinner_value_set(evas_object_data_get(obj, "density"),
                         ephysics_body_density_get(body));
   elm_spinner_value_set(evas_object_data_get(obj, "friction"),
                         ephysics_body_friction_get(body));
   elm_spinner_value_set(evas_object_data_get(obj, "restitution"),
                         ephysics_body_restitution_get(body));

   body_image = ephysics_body_evas_object_unset(body);
   evas_object_geometry_get(body_image, &x, &y, &w, &h);
   evas_object_del(body_image);

   material = elm_spinner_special_value_get(obj, elm_spinner_value_get(obj));
   body_image = elm_image_add(evas_object_data_get(obj, "win"));
   elm_image_file_set(
      body_image, PACKAGE_DATA_DIR "/" SANDBOX_THEME ".edj", material);
   evas_object_resize(body_image, w, h);
   evas_object_move(body_image, x, y);
   evas_object_show(body_image);
   ephysics_body_evas_object_set(body, body_image, EINA_FALSE);
}

static void
_body_mass_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   EPhysics_Body *body = data;
   ephysics_body_mass_set(body, elm_spinner_value_get(obj));
   elm_spinner_value_set(evas_object_data_get(obj, "density"), 0);
}

static void
_body_density_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   EPhysics_Body *body = data;
   ephysics_body_density_set(body, elm_spinner_value_get(obj));
   elm_spinner_value_set(evas_object_data_get(obj, "mass"),
                         ephysics_body_mass_get(body));
}

static void
_body_rotation_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   EPhysics_Body *body = data;
   ephysics_body_rotation_set(body, elm_spinner_value_get(obj));
}

static void
_body_friction_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   EPhysics_Body *body = data;
   ephysics_body_friction_set(body, elm_spinner_value_get(obj));
}

static void
_body_restitution_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   EPhysics_Body *body = data;
   ephysics_body_restitution_set(body, elm_spinner_value_get(obj));
}

static void
_body_damping_linear_cb(void *data, Evas_Object *obj,
                        void *event_info __UNUSED__)
{
   EPhysics_Body *body = data;
   double angular_damping;

   ephysics_body_damping_get(body, NULL, &angular_damping);
   ephysics_body_damping_set(body, elm_spinner_value_get(obj), angular_damping);
}

static void
_body_damping_angular_cb(void *data, Evas_Object *obj,
                         void *event_info __UNUSED__)
{
   EPhysics_Body *body = data;
   double linear_damping;

   ephysics_body_damping_get(body, &linear_damping, NULL);
   ephysics_body_damping_set(body, linear_damping, elm_spinner_value_get(obj));
}

static void
_body_sleeping_threshold_linear_cb(void *data, Evas_Object *obj,
                                   void *event_info __UNUSED__)
{
   EPhysics_Body *body = data;
   double angular_sleeping;

   ephysics_body_sleeping_threshold_get(body, NULL, &angular_sleeping);
   ephysics_body_sleeping_threshold_set(body, elm_spinner_value_get(obj),
                                        angular_sleeping);
}

static void
_body_sleeping_threshold_angular_cb(void *data, Evas_Object *obj,
                                    void *event_info __UNUSED__)
{
   EPhysics_Body *body = data;
   double linear_sleeping;

   ephysics_body_sleeping_threshold_get(body, &linear_sleeping, NULL);
   ephysics_body_sleeping_threshold_set(body, linear_sleeping,
                                        elm_spinner_value_get(obj));
}

static void
_body_torque_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   EPhysics_Body *body = data;
   ephysics_body_torque_apply(body, elm_spinner_value_get(obj));
}

static void
_body_impulse_x_x_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   EPhysics_Body *body = data;
   Evas_Coord rel_x, rel_y;
   double y;

   y = elm_spinner_value_get(evas_object_data_get(obj, "y"));
   rel_x = elm_spinner_value_get(evas_object_data_get(obj, "relx"));
   rel_y = elm_spinner_value_get(evas_object_data_get(obj, "rely"));

   ephysics_body_impulse_apply(body, elm_spinner_value_get(obj), y, rel_x,
                               rel_y);
}

static void
_body_impulse_x_rel_cb(void *data, Evas_Object *obj,
                       void *event_info __UNUSED__)
{
   EPhysics_Body *body = data;
   Evas_Object *aux;
   Evas_Coord rel_y;
   double x, y;

   aux = evas_object_data_get(obj, "x");

   x = elm_spinner_value_get(aux);
   y = elm_spinner_value_get(evas_object_data_get(aux, "y"));
   rel_y = elm_spinner_value_get(evas_object_data_get(aux, "rely"));

   ephysics_body_impulse_apply(body, x, y, elm_spinner_value_get(obj), rel_y);
}

static void
_body_impulse_y_y_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   EPhysics_Body *body = data;
   Evas_Object *aux;
   Evas_Coord rel_x, rel_y;
   double x;

   aux = evas_object_data_get(obj, "x");

   x = elm_spinner_value_get(aux);
   rel_x = elm_spinner_value_get(evas_object_data_get(aux, "relx"));
   rel_y = elm_spinner_value_get(evas_object_data_get(aux, "rely"));

   ephysics_body_impulse_apply(body, x, elm_spinner_value_get(obj), rel_x,
                               rel_y);
}

static void
_body_impulse_y_rel_cb(void *data, Evas_Object *obj,
                       void *event_info __UNUSED__)
{
   EPhysics_Body *body = data;
   Evas_Object *aux;
   Evas_Coord rel_x;
   double x, y;

   aux = evas_object_data_get(obj, "x");

   x = elm_spinner_value_get(aux);
   y = elm_spinner_value_get(evas_object_data_get(aux, "y"));
   rel_x = elm_spinner_value_get(evas_object_data_get(aux, "relx"));

   ephysics_body_impulse_apply(body, x, y, rel_x, elm_spinner_value_get(obj));
}

static void
_body_force_x_x_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   EPhysics_Body *body = data;
   Evas_Coord rel_x, rel_y;
   double y;

   y = elm_spinner_value_get(evas_object_data_get(obj, "y"));
   rel_x = elm_spinner_value_get(evas_object_data_get(obj, "relx"));
   rel_y = elm_spinner_value_get(evas_object_data_get(obj, "rely"));

   ephysics_body_force_apply(body, elm_spinner_value_get(obj), y, rel_x, rel_y);
}

static void
_body_force_x_rel_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   EPhysics_Body *body = data;
   Evas_Object *aux;
   Evas_Coord rel_y;
   double x, y;

   aux = evas_object_data_get(obj, "x");

   x = elm_spinner_value_get(aux);
   y = elm_spinner_value_get(evas_object_data_get(aux, "y"));
   rel_y = elm_spinner_value_get(evas_object_data_get(aux, "rely"));

   ephysics_body_force_apply(body, x, y, elm_spinner_value_get(obj), rel_y);
}

static void
_body_force_y_y_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   EPhysics_Body *body = data;
   Evas_Object *aux;
   Evas_Coord rel_x, rel_y;
   double x;

   aux = evas_object_data_get(obj, "x");

   x = elm_spinner_value_get(aux);
   rel_x = elm_spinner_value_get(evas_object_data_get(aux, "relx"));
   rel_y = elm_spinner_value_get(evas_object_data_get(aux, "rely"));

   ephysics_body_force_apply(body, x, elm_spinner_value_get(obj), rel_x, rel_y);
}

static void
_body_force_y_rel_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   EPhysics_Body *body = data;
   Evas_Object *aux;
   Evas_Coord rel_x;
   double x, y;

   aux = evas_object_data_get(obj, "x");

   x = elm_spinner_value_get(aux);
   y = elm_spinner_value_get(evas_object_data_get(aux, "y"));
   rel_x = elm_spinner_value_get(evas_object_data_get(aux, "relx"));

   ephysics_body_force_apply(body, x, y, rel_x, elm_spinner_value_get(obj));
}

static void
_body_linear_velocity_x_cb(void *data, Evas_Object *obj,
                           void *event_info __UNUSED__)
{
   EPhysics_Body *body = data;
   double ylinear;

   ylinear = elm_spinner_value_get(evas_object_data_get(obj, "y"));
   ephysics_body_linear_velocity_set(body, elm_spinner_value_get(obj), ylinear);
}

static void
_body_linear_velocity_y_cb(void *data, Evas_Object *obj,
                           void *event_info __UNUSED__)
{
   EPhysics_Body *body = data;
   double xlinear;

   xlinear = elm_spinner_value_get(evas_object_data_get(obj, "x"));
   ephysics_body_linear_velocity_set(body, xlinear, elm_spinner_value_get(obj));
}

static void
_body_angular_velocity_cb(void *data, Evas_Object *obj,
                          void *event_info __UNUSED__)
{
   EPhysics_Body *body = data;
   ephysics_body_angular_velocity_set(body, elm_spinner_value_get(obj));
}
/*
static void
_body_hardness_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   EPhysics_Body *body = data;

   ephysics_body_soft_body_hardness_set(body, elm_spinner_value_get(obj));
}
*/
static void
_sandie_label_add(Evas_Object *win, Evas_Object *bxparent,
                  const char *subcategory)
{
   Evas_Object *label;

   label = elm_label_add(win);
   elm_object_text_set(label, subcategory);
   elm_box_pack_end(bxparent, label);
   evas_object_show(label);
}

static Evas_Object *
_sandie_enum_add(Evas_Object *win, Evas_Object *bxparent,
                 const char *subcategory)
{
   Evas_Object *sp;

   _sandie_label_add(win, bxparent, subcategory);

   sp = elm_spinner_add(win);
   elm_spinner_wrap_set(sp, EINA_TRUE);
   elm_spinner_min_max_set(sp, 0, 6);
   elm_spinner_value_set(sp, 0);
   elm_spinner_step_set(sp, 1);
   elm_spinner_label_format_set(sp, "%.0f");
   elm_spinner_editable_set(sp, EINA_FALSE);

   elm_spinner_special_value_add(sp, EPHYSICS_BODY_MATERIAL_CUSTOM, "Custom");
   elm_spinner_special_value_add(sp, EPHYSICS_BODY_MATERIAL_CONCRETE,
                                 "Concrete");
   elm_spinner_special_value_add(sp, EPHYSICS_BODY_MATERIAL_IRON, "Iron");
   elm_spinner_special_value_add(sp, EPHYSICS_BODY_MATERIAL_PLASTIC, "Plastic");
   elm_spinner_special_value_add(sp, EPHYSICS_BODY_MATERIAL_POLYSTYRENE,
                                 "Polystyrene");
   elm_spinner_special_value_add(sp, EPHYSICS_BODY_MATERIAL_RUBBER, "Rubber");
   elm_spinner_special_value_add(sp, EPHYSICS_BODY_MATERIAL_WOOD, "Wood");

   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sp, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bxparent, sp);
   evas_object_show(sp);

   evas_object_data_set(sp, "win", win);

   return sp;
}

static Evas_Object *
_sandie_toggle_add(Evas_Object *win, Evas_Object *bxparent,
                   const char *subcategory, const char *labeloff,
                   const char *labelon)
{
   Evas_Object *tg;

   _sandie_label_add(win, bxparent, subcategory);

   tg = elm_check_add(win);
   elm_object_style_set(tg, "toggle");
   elm_object_part_text_set(tg, "off", labeloff);
   elm_object_part_text_set(tg, "on", labelon);
   evas_object_size_hint_align_set(tg, 0.5, 0.5);
   evas_object_size_hint_weight_set(tg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bxparent, tg);
   evas_object_show(tg);

   return tg;
}

static Evas_Object *
_sandie_spinner_add(Evas_Object *win, Evas_Object *bxparent,
                    const char *subcategory, const char *itemlb, float min,
                    float max, float initial, float step)
{
   Evas_Object *sp;

   _sandie_label_add(win, bxparent, subcategory);

   sp = elm_spinner_add(win);
   elm_spinner_label_format_set(sp, itemlb);
   elm_spinner_min_max_set(sp, min, max);
   elm_spinner_step_set(sp, step);
   elm_spinner_wrap_set(sp, EINA_TRUE);
   elm_spinner_value_set(sp, initial);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sp, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bxparent, sp);
   evas_object_show(sp);

   return sp;
}

static Evas_Object *
_sandie_double_spinner_box_add(Evas_Object *win, Evas_Object *bxparent,
                               const char *subcategory)
{
   Evas_Object *dbx;

   _sandie_label_add(win, bxparent, subcategory);

   dbx = elm_box_add(win);
   elm_box_horizontal_set(dbx, EINA_TRUE);
   evas_object_size_hint_weight_set(dbx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(dbx, EVAS_HINT_FILL, 0.0);
   elm_box_align_set(dbx, 0.0, 0.5);
   elm_box_pack_end(bxparent, dbx);
   evas_object_show(dbx);

   return dbx;
}

static Evas_Object *
_category_add(Evas_Object *win, Evas_Object *bxparent, const char *label)
{
   Evas_Object *cfr, *cbx;

   cfr = elm_frame_add(win);
   elm_frame_autocollapse_set(cfr, EINA_TRUE);
   elm_object_text_set(cfr, label);
   evas_object_size_hint_weight_set(cfr, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(cfr, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(bxparent, cfr);
   evas_object_show(cfr);

   cbx = elm_box_add(win);
   elm_box_horizontal_set(cbx, EINA_FALSE);
   evas_object_size_hint_weight_set(cbx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(cbx, EVAS_HINT_FILL, 0.0);
   elm_box_align_set(cbx, 0.0, 0.5);
   elm_object_content_set(cfr, cbx);
   evas_object_show(cbx);

   return cbx;
}

static void
_menu_body_items_create(Evas_Object *win, Evas_Object *bxparent,
                        const char *category, EPhysics_Body *body)
{
   Evas_Object *bx, *dbx, *widget, *aux_widget, *material_widget, *type_widget;

   bx = _category_add(win, bxparent, category);
   type_widget = _sandie_toggle_add(win, bx, "Body Type", "Solid", "Soft");
   material_widget = _sandie_enum_add(win, bx, "Body Material");
   aux_widget = _sandie_spinner_add(win, bx, "Mass", "%1.3f kg",
                                0, 100, 15, 2);
   widget = _sandie_spinner_add(win, bx, "Density", "%1.3f kg/m³",
                                0, 9999, 0, 2);
   evas_object_data_set(aux_widget, "density", widget);
   evas_object_data_set(material_widget, "density", widget);
   evas_object_smart_callback_add(aux_widget, "delay,changed", _body_mass_cb,
                                  body);
   evas_object_data_set(widget, "mass", aux_widget);
   evas_object_smart_callback_add(widget, "delay,changed", _body_density_cb,
                                  body);
   widget = _sandie_spinner_add(win, bx, "Rotation", "%1.0fº",
                                -360, 360, 0, 2);
   evas_object_smart_callback_add(widget, "delay,changed", _body_rotation_cb,
                                  body);
   widget = _sandie_spinner_add(win, bx, "Friction", "%1.3f",
                                0, 1, 0.5, 0.05);
   evas_object_data_set(material_widget, "friction", widget);
   evas_object_smart_callback_add(widget, "delay,changed", _body_friction_cb,
                                  body);
   widget = _sandie_spinner_add(win, bx, "Restitution", "%1.3f",
                                0, 1, 0, 0.05);
   evas_object_data_set(material_widget, "restitution", widget);
   evas_object_smart_callback_add(material_widget, "delay,changed",
                                  _body_material_cb, body);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _body_restitution_cb, body);
   dbx = _sandie_double_spinner_box_add(win, bx, "Damping");
   widget = _sandie_spinner_add(win, dbx, "Linear:", "%1.3f",
                                0, 1, 0, 0.05);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _body_damping_linear_cb, body);
   widget = _sandie_spinner_add(win, dbx, "Angular:", "%1.3f",
                                0, 1, 0, 0.05);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _body_damping_angular_cb, body);
   dbx = _sandie_double_spinner_box_add(win, bx, "Sleeping Threshold");
   widget = _sandie_spinner_add(win, dbx, "Linear:", "%1.2f p/s",
                                0, 100, 0, 2);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _body_sleeping_threshold_linear_cb, body);
   widget = _sandie_spinner_add(win, dbx, "Angular:", "%1.2f º/s",
                                0, 360, 0, 2);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _body_sleeping_threshold_angular_cb, body);
   widget = _sandie_spinner_add(win, bx, "Torque", "%1.3f",
                                0, 1, 0.5, 0.05);
   evas_object_smart_callback_add(widget, "delay,changed", _body_torque_cb,
                                  body);

   //Impulse needs four values
   dbx = _sandie_double_spinner_box_add(win, bx, "Impulse X");
   aux_widget = _sandie_spinner_add(win, dbx, "X:", "%1.3f kg * p/s",
                                    -360, 360, 0, 2);
   widget = _sandie_spinner_add(win, dbx, "Rel Position X:", "%1.2f",
                                0, 360, 0, 2);
   evas_object_data_set(aux_widget, "relx", widget);
   dbx = _sandie_double_spinner_box_add(win, bx, "Impulse Y");
   widget = _sandie_spinner_add(win, dbx, "Y:", "%1.3f kg * p/s",
                                -360, 360, 0, 2);
   evas_object_data_set(aux_widget, "y", widget);
   widget = _sandie_spinner_add(win, dbx, "Rel Position Y:", "%1.2f",
                                0, 360, 0, 2);
   evas_object_data_set(aux_widget, "rely", widget);
   evas_object_smart_callback_add(aux_widget, "delay,changed",
                                  _body_impulse_x_x_cb, body);
   evas_object_data_set(widget, "x", aux_widget);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _body_impulse_x_rel_cb, body);
   widget = evas_object_data_get(aux_widget, "y");
   evas_object_data_set(widget, "x", aux_widget);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _body_impulse_y_y_cb, body);
   widget = evas_object_data_get(aux_widget, "relx");
   evas_object_data_set(widget, "x", aux_widget);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _body_impulse_y_rel_cb, body);

   //Force needs four values
   dbx = _sandie_double_spinner_box_add(win, bx, "Force X");
   aux_widget = _sandie_spinner_add(win, dbx, "X:", "%1.3f kg * p/s/s",
                                    -360, 360, 0, 2);
   widget = _sandie_spinner_add(win, dbx, "Rel Position X:", "%1.2f",
                                0, 360, 0, 2);
   evas_object_data_set(aux_widget, "relx", widget);
   dbx = _sandie_double_spinner_box_add(win, bx, "Force Y");
   widget = _sandie_spinner_add(win, dbx, "Y:", "%1.3f kg * p/s/s",
                                -360, 360, 0, 2);
   evas_object_data_set(aux_widget, "y", widget);
   widget = _sandie_spinner_add(win, dbx, "Rel Position Y:", "%1.2f",
                                0, 360, 0, 2);
   evas_object_data_set(aux_widget, "rely", widget);
   evas_object_smart_callback_add(aux_widget, "delay,changed",
                                  _body_force_x_x_cb, body);
   evas_object_data_set(widget, "x", aux_widget);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _body_force_x_rel_cb, body);
   widget = evas_object_data_get(aux_widget, "y");
   evas_object_data_set(widget, "x", aux_widget);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _body_force_y_y_cb, body);
   widget = evas_object_data_get(aux_widget, "relx");
   evas_object_data_set(widget, "x", aux_widget);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _body_force_y_rel_cb, body);

   dbx = _sandie_double_spinner_box_add(win, bx, "Linear Velocity");
   aux_widget = _sandie_spinner_add(win, dbx, "X:", "%1.2f p/s",
                                    -800, 800, 0, 2);
   widget = _sandie_spinner_add(win, dbx, "Y:", "%1.2f p/s",
                                -800, 800, 0, 2);
   evas_object_data_set(aux_widget, "y", widget);
   evas_object_smart_callback_add(aux_widget, "delay,changed",
                                  _body_linear_velocity_x_cb, body);
   evas_object_data_set(widget, "x", aux_widget);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _body_linear_velocity_y_cb, body);

   widget = _sandie_spinner_add(win, bx, "Angular Velocity", "%1.2f º/s",
                                0, 360, 0, 2);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _body_angular_velocity_cb, body);
/* widget = _sandie_spinner_add(win, bx, "Soft Body Hardness", "%1.2f%%",
                                0, 100, 100, 2);
   elm_object_disabled_set(widget, EINA_TRUE);
   evas_object_smart_callback_add(widget, "delay,changed", _body_hardness_cb, body);
   evas_object_data_set(type_widget, "hardness", widget);
   evas_object_smart_callback_add(type_widget, "changed", _body_type_cb, body);//FIX CB
*/
}

static EPhysics_Body *
_sandie_body_add(Evas_Object *win, EPhysics_World *world, int x, int y)
{
   Evas_Object *body_image;
   EPhysics_Body *body;

   body_image = elm_image_add(win);
   elm_image_file_set(
      body_image, PACKAGE_DATA_DIR "/" SANDBOX_THEME ".edj", "Custom");
   evas_object_resize(body_image, 70, 70);
   evas_object_move(body_image, x, y);
   evas_object_show(body_image);

   body = ephysics_body_circle_add(world);
   ephysics_body_evas_object_set(body, body_image, EINA_TRUE);
   ephysics_body_mass_set(body, 20);
   ephysics_body_central_impulse_apply(body, 8201, 2110);
   ephysics_body_event_callback_add(body, EPHYSICS_CALLBACK_BODY_DEL,
                                    _body_del, NULL);

   return body;
}
static void
_menu_items_create(Evas_Object *win, Evas_Object *bxparent,
                   EPhysics_World *world)
{
   Evas_Object *bx, *dbx, *widget;

   bx = _category_add(win, bxparent, "EPhysics World");
   dbx = _sandie_double_spinner_box_add(win, bx, "Gravity");
   widget = _sandie_spinner_add(win, dbx, "X:", "%1.2f px/s²",
                                -1000, 1000, 0, 2);
   evas_object_smart_callback_add(widget, "delay,changed", _world_gravity_x_cb,
                                  world);
   widget = _sandie_spinner_add(win, dbx, "Y:", "%1.2f px/s²",
                                -1000, 1000, 294, 2);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _world_gravity_y_cb, world);
   widget = _sandie_spinner_add(win, bx, "Rate", "%1.0f pixel/meter",
                                1, 100, 30, 2);
   evas_object_smart_callback_add(widget, "delay,changed", _world_rate_cb,
                                  world);
   widget = _sandie_spinner_add(win, bx, "Max Sleeping Time", "%1.0f s",
                                0, 100, 2, 2);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _world_max_sleeping_time_cb, world);

   bx = _category_add(win, bxparent, "EPhysics Boundaries");
   widget = _sandie_spinner_add(win, bx, "Friction", "%1.3f",
                                0, 1, 0.5, 0.05);
   evas_object_data_set(widget, "win", win);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _world_friction_cb, win);
   widget = _sandie_spinner_add(win, bx, "Restitution", "%1.3f",
                                0, 1, 0, 0.05);
   evas_object_smart_callback_add(widget, "delay,changed",
                                  _world_restitution_cb, win);
}

static void
_world_populate(Sandie_Data *sandie, Evas_Object *bxparent)
{
   sandie->body1 = _sandie_body_add(sandie->win, sandie->world, (int) WIDTH / 4,
                                    (int) HEIGHT / 5);
   sandie->body2 = _sandie_body_add(sandie->win, sandie->world, (int) WIDTH / 4,
                                    (int) HEIGHT / 2);

   _menu_body_items_create(sandie->win, bxparent, "EPhysics Body 1",
                           sandie->body1);
   _menu_body_items_create(sandie->win, bxparent, "EPhysics Body 2",
                           sandie->body2);
}

static void
_restart(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Sandie_Data *sandie = data;
   ephysics_body_del(sandie->body1);
   ephysics_body_del(sandie->body2);
}

static Evas_Object *
_menu_create(Sandie_Data *sandie)
{
   Evas_Object *bx0, *fr, *lb, *bt, *sc, *bxparent, *layout;

   layout = evas_object_data_get(sandie->win, "layout");

   bx0 = elm_box_add(sandie->win);
   evas_object_size_hint_weight_set(bx0, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_part_content_set(layout, "swallow", bx0);
   evas_object_show(bx0);

   fr = elm_frame_add(sandie->win);
   elm_frame_autocollapse_set(fr, EINA_TRUE);
   elm_object_text_set(fr, "Information");
   elm_box_pack_end(bx0, fr);
   evas_object_show(fr);

   lb = elm_label_add(sandie->win);
   elm_object_text_set(lb,
                       "Please modify physical parameters below<br/>"
                       "the test button to show the test window.");
   elm_object_content_set(fr, lb);
   evas_object_show(lb);

   bt = elm_button_add(sandie->win);
   elm_object_text_set(bt, "     Try     ");
   elm_box_pack_end(bx0, bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked", _restart, sandie);

   sc = elm_scroller_add(sandie->win);
   elm_scroller_bounce_set(sc, EINA_FALSE, EINA_TRUE);
   elm_scroller_policy_set(sc, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_ON);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx0, sc);
   evas_object_show(sc);

   bxparent = elm_box_add(sandie->win);
   evas_object_size_hint_weight_set(bxparent, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bxparent, EVAS_HINT_FILL, 0.0);
   elm_object_content_set(sc, bxparent);
   evas_object_show(bxparent);

   _menu_items_create(sandie->win, bxparent, sandie->world);

   return bxparent;
}

static EPhysics_World *
_sandie_world_add(Evas_Object *win)
{
   EPhysics_World *world;

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 80, 80, (int) WIDTH * 0.7 - 160,
                                      HEIGHT - 160);
   evas_object_data_set(win, "top", ephysics_body_top_boundary_add(world));
   evas_object_data_set(win, "bottom",
                        ephysics_body_bottom_boundary_add(world));
   evas_object_data_set(win, "left", ephysics_body_left_boundary_add(world));
   evas_object_data_set(win, "right", ephysics_body_right_boundary_add(world));

   return world;
}

EAPI int
elm_main()
{
   Sandie_Data *sandie;
   Evas_Object *layout, *bxparent;
   short int r = 0;

   if (!ephysics_init())
     {
        fprintf(stderr, "Failed to init ephysics\n");
        r = -1;
        goto ephysics_error;
     }

   sandie = calloc(1, sizeof(Sandie_Data));
   if (!sandie)
     {
        fprintf(stderr, "Failed to alloc sandie data\n");
        r = -1;
        goto data_error;
     }

   elm_theme_extension_add(NULL, PACKAGE_DATA_DIR "/" SANDBOX_THEME ".edj");

   sandie->win = elm_win_add(NULL, "Sandbox", ELM_WIN_BASIC);

   elm_win_title_set(sandie->win, "EPhysics Sandbox");
   evas_object_resize(sandie->win, WIDTH, HEIGHT);
   elm_win_screen_constrain_set(sandie->win, EINA_TRUE);
   elm_win_autodel_set(sandie->win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   layout = elm_layout_add(sandie->win);
   evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(sandie->win, layout);
   elm_layout_file_set(layout, PACKAGE_DATA_DIR "/" SANDBOX_THEME ".edj",
                       "sandbox");
   evas_object_show(layout);
   evas_object_data_set(sandie->win, "layout", layout);

   evas_object_show(sandie->win);

   sandie->world = _sandie_world_add(sandie->win);

   bxparent = _menu_create(sandie);

   _world_populate(sandie, bxparent);

   elm_run();

   free(sandie);
data_error:
   ephysics_shutdown();
ephysics_error:
   elm_shutdown();

   return r;
}
ELM_MAIN()
