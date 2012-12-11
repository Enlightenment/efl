#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

typedef struct _Collision_Data Collision_Data;

struct _Collision_Data {
     Test_Data base;
     EPhysics_Body *sphere;
     EPhysics_Body *sphere2;
};

static void
_del_cb(void *data, EPhysics_Body *body, void *event_info __UNUSED__)
{
   Collision_Data *collision_data = data;
   Evas_Object *obj, *shadow;

   obj = ephysics_body_evas_object_get(body);
   shadow = evas_object_data_get(obj, "shadow");

   collision_data->base.evas_objs = eina_list_remove(
      collision_data->base.evas_objs, shadow);
   collision_data->base.evas_objs = eina_list_remove(
      collision_data->base.evas_objs, obj);

   evas_object_del(shadow);
   evas_object_del(obj);

   collision_data->sphere = NULL;
}

static void
_collision_cb(void *data, EPhysics_Body *body, void *event_info)
{
   EPhysics_Body_Collision *collision = event_info;
   Collision_Data *collision_data = data;
   EPhysics_Body *contact_body;

   contact_body = ephysics_body_collision_contact_body_get(collision);
   if (contact_body != collision_data->sphere2) return;

   collision_data->base.bodies = eina_list_remove(collision_data->base.bodies,
                                                  body);

   INF("Collision Detected");
   ephysics_body_del(body);
}

static void
_world_populate(Collision_Data *collision_data)
{
   Evas_Object *sphere1, *sphere2, *sh1, *sh2;
   EPhysics_Body *sphere_body1, *sphere_body2;

   sh1 = elm_layout_add(collision_data->base.win);
   elm_layout_file_set(
      sh1, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "shadow-ball");
   evas_object_move(sh1, WIDTH / 2, FLOOR_Y);
   evas_object_resize(sh1, 70, 3);
   evas_object_show(sh1);
   collision_data->base.evas_objs = eina_list_append(
      collision_data->base.evas_objs, sh1);

   sphere1 = elm_image_add(collision_data->base.win);
   elm_image_file_set(
      sphere1, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj",
      "big-red-ball");
   evas_object_move(sphere1, WIDTH / 2, FLOOR_Y - 70 + 1);
   evas_object_resize(sphere1, 70, 70);
   evas_object_show(sphere1);
   evas_object_data_set(sphere1, "shadow", sh1);
   collision_data->base.evas_objs = eina_list_append(
      collision_data->base.evas_objs, sphere1);

   sphere_body1 = ephysics_body_sphere_add(collision_data->base.world);
   ephysics_body_evas_object_set(sphere_body1, sphere1, EINA_TRUE);
   ephysics_body_event_callback_add(sphere_body1,
                                    EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, sh1);
   ephysics_body_event_callback_add(sphere_body1,
                                    EPHYSICS_CALLBACK_BODY_DEL,
                                    _del_cb, collision_data);
   ephysics_body_restitution_set(sphere_body1, 0.8);
   ephysics_body_friction_set(sphere_body1, 0.4);
   collision_data->base.bodies = eina_list_append(
      collision_data->base.bodies, sphere_body1);
   collision_data->sphere = sphere_body1;

   sh2 = elm_layout_add(collision_data->base.win);
   elm_layout_file_set(
      sh2, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "shadow-ball");
   evas_object_move(sh2, WIDTH / 8, FLOOR_Y);
   evas_object_resize(sh2, 70, 3);
   evas_object_show(sh2);
   collision_data->base.evas_objs = eina_list_append(
      collision_data->base.evas_objs, sh2);

   sphere2 = elm_image_add(collision_data->base.win);
   elm_image_file_set(
      sphere2, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj",
      "big-green-ball");
   evas_object_move(sphere2, WIDTH / 8, FLOOR_Y - 70 + 1);
   evas_object_resize(sphere2, 70, 70);
   evas_object_show(sphere2);
   collision_data->base.evas_objs = eina_list_append(
      collision_data->base.evas_objs, sphere2);

   sphere_body2 = ephysics_body_sphere_add(collision_data->base.world);
   ephysics_body_evas_object_set(sphere_body2, sphere2, EINA_TRUE);
   ephysics_body_event_callback_add(sphere_body2,
                                    EPHYSICS_CALLBACK_BODY_UPDATE,
                                    update_object_cb, sh2);
   ephysics_body_restitution_set(sphere_body2, 1);
   ephysics_body_friction_set(sphere_body2, 0.4);
   ephysics_body_central_impulse_apply(sphere_body2, 360, 0, 0);
   collision_data->sphere2 = sphere_body2;
   collision_data->base.bodies = eina_list_append(
      collision_data->base.bodies, sphere_body2);

   ephysics_body_event_callback_add(collision_data->sphere,
                                    EPHYSICS_CALLBACK_BODY_COLLISION,
                                    _collision_cb, collision_data);
}

static void
_restart(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Collision_Data *collision_data = data;

   DBG("Restart pressed");
   test_clean((Test_Data *)collision_data);
   _world_populate(collision_data);
}

static void
_win_del(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Collision_Data *collision_data = data;

   test_clean((Test_Data *)collision_data);
   evas_object_del(collision_data->base.layout);
   ephysics_world_del(collision_data->base.world);
   free(collision_data);
   ephysics_shutdown();
}

void
test_delete(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Collision_Data *collision_data;
   EPhysics_Body *boundary;
   EPhysics_World *world;

   if (!ephysics_init())
     return;

   collision_data = calloc(1, sizeof(Collision_Data));
   if (!collision_data)
     {
        ERR("Failed to create test data");
        ephysics_shutdown();
        return;
     }

   test_win_add((Test_Data *)collision_data, "Delete Body", EINA_FALSE);
   evas_object_event_callback_add(collision_data->base.win, EVAS_CALLBACK_DEL,
                                  _win_del, collision_data);
   elm_object_signal_emit(collision_data->base.layout, "borders,show",
                          "ephysics_test");
   elm_layout_signal_callback_add(collision_data->base.layout, "restart",
                                  "test-theme", _restart, collision_data);

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 50, 40, -50,
                                      WIDTH - 100, FLOOR_Y - 40, DEPTH);
   collision_data->base.world = world;

   boundary = ephysics_body_bottom_boundary_add(collision_data->base.world);
   ephysics_body_restitution_set(boundary, 0);
   ephysics_body_friction_set(boundary, 20);

   ephysics_body_left_boundary_add(collision_data->base.world);
   ephysics_body_right_boundary_add(collision_data->base.world);

   _world_populate(collision_data);

   ephysics_body_event_callback_add(collision_data->sphere,
                                    EPHYSICS_CALLBACK_BODY_COLLISION,
                                    _collision_cb, collision_data);
}
