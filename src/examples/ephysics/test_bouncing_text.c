#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

static void
_world_populate(Test_Data *test_data)
{
   EPhysics_Body *text_body;
   Evas_Object *text;
   int x, y, w;

   text = evas_object_text_add(evas_object_evas_get(test_data->win));
   evas_object_text_text_set(text, "EPHYSICS");
   evas_object_text_font_set(text, "Sans", 54);
   evas_object_color_set(text, 95, 56, 19, 255);
   evas_object_move(text, WIDTH / 4, HEIGHT / 8);
   evas_object_show(text);
   evas_object_geometry_get(text, &x, &y, &w, NULL);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, text);

   text_body = ephysics_body_box_add(test_data->world);
   ephysics_body_evas_object_set(text_body, text, EINA_FALSE);
   ephysics_body_geometry_set(text_body, x, y, -15, w * 5 / 6, 46, 30);
   ephysics_body_restitution_set(text_body, 0.95);
   ephysics_body_friction_set(text_body, 0.1);
   test_data->bodies = eina_list_append(test_data->bodies, text_body);
}

static void
_restart(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Test_Data *test_data = data;

   DBG("Restart pressed");
   test_clean(test_data);
   _world_populate(test_data);
}

void
test_bouncing_text(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_Body *boundary;
   EPhysics_World *world;
   Test_Data *test_data;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Bouncing Text", EINA_TRUE);
   elm_object_signal_emit(test_data->layout, "borders,show", "ephysics_test");
   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 50, 40, -50,
                                      WIDTH - 100, FLOOR_Y - 40, 100);
   test_data->world = world;

   boundary = ephysics_body_bottom_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 0.65);
   ephysics_body_friction_set(boundary, 2);

   _world_populate(test_data);
}
