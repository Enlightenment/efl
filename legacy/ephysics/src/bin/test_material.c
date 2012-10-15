#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

static void
_material_change(Test_Data *test_data)
{
   double density, mass, restitution, friction;
   EPhysics_Body *body;
   Evas_Object *sp;
   int material;

   sp = test_data->data;
   material = (int) elm_spinner_value_get(sp);
   body = evas_object_data_get(sp, "body");

   ephysics_body_material_set(body, material);

   material = ephysics_body_material_get(body);
   density = ephysics_body_density_get(body);
   mass = ephysics_body_mass_get(body);
   restitution = ephysics_body_restitution_get(body);
   friction = ephysics_body_friction_get(body);

   INF("Material: %i, Density: %lf, Mass: %lf, Restitution: %lf, Friction: %lf",
       material, density, mass, restitution, friction);
}

static void
_material_changed_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _material_change(data);
}

static void
_world_populate(Test_Data *test_data)
{
   Evas_Object *sphere, *shadow;
   EPhysics_Body *fall_body;

   shadow = elm_layout_add(test_data->win);
   elm_layout_file_set(
      shadow, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "shadow-ball");
   evas_object_move(shadow, WIDTH / 3, FLOOR_Y);
   evas_object_resize(shadow, 70, 3);
   evas_object_show(shadow);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, shadow);

   sphere = elm_image_add(test_data->win);
   elm_image_file_set(
      sphere, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "big-blue-ball");
   evas_object_move(sphere, WIDTH / 3, HEIGHT / 8);
   evas_object_resize(sphere, 70, 70);
   evas_object_show(sphere);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, sphere);

   fall_body = ephysics_body_circle_add(test_data->world);
   ephysics_body_evas_object_set(fall_body, sphere, EINA_TRUE);
   ephysics_body_event_callback_add(fall_body, EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, shadow);
   test_data->bodies = eina_list_append(test_data->bodies, fall_body);

   evas_object_data_set(test_data->data, "body", fall_body);
}

static void
_restart(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   DBG("Restart pressed");
   test_clean(data);
   _world_populate(data);
   _material_change(data);
}

void
test_material(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_Body *boundary;
   EPhysics_World *world;
   Test_Data *test_data;
   Evas_Object *sp;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Material", EINA_TRUE);

   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);
   elm_object_signal_emit(test_data->layout, "borders,show", "ephysics_test");

   sp = elm_spinner_add(test_data->win);
   elm_spinner_min_max_set(sp, 0, 6);
   elm_spinner_step_set(sp, 1);
   elm_spinner_wrap_set(sp, EINA_TRUE);
   elm_spinner_special_value_add(sp, 0, "Custom");
   elm_spinner_special_value_add(sp, 1, "Concrete");
   elm_spinner_special_value_add(sp, 2, "Iron");
   elm_spinner_special_value_add(sp, 3, "Plastic");
   elm_spinner_special_value_add(sp, 4, "Polystyrene");
   elm_spinner_special_value_add(sp, 5, "Rubber");
   elm_spinner_special_value_add(sp, 6, "Wood");
   elm_spinner_editable_set(sp, EINA_FALSE);
   elm_object_style_set(sp, "ephysics-test");
   evas_object_smart_callback_add(sp, "delay,changed", _material_changed_cb,
                                  test_data);
   elm_layout_content_set(test_data->layout, "extra_input", sp);
   test_data->data = sp;

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 50, 40, WIDTH - 100, FLOOR_Y - 40);
   test_data->world = world;

   boundary = ephysics_body_bottom_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 1);

   _world_populate(test_data);
   _material_change(test_data);
}
