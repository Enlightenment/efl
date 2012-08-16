#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_test.h"

typedef struct _Old_Rate Old_Rate;
struct _Old_Rate {
     EPhysics_Body *body;
     double w;
     double h;
};

static void
_del_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   free(data);
}

static void
_win_resize_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Test_Data *test_data = data;
   EPhysics_Body *sphere_body;
   int ww, wh, x, y, w, h;
   double ratew, rateh;
   Evas_Object *sphere;
   Old_Rate *old_rate;

   evas_object_geometry_get(obj, NULL, NULL, &ww, &wh);
   ratew = (double) ww / WIDTH;
   rateh = (double) wh / HEIGHT;

   /* Since we don't support different rates on ephysics world, we
      should make sure window keeps its aspect.
      Maybe if we decide it should be supported someday,
      this example could be modified just dropping the following check. */
   if (ratew > rateh)
     {
        ww = rateh * WIDTH;
        evas_object_resize(obj, ww, wh);
        return;
     }
   else if (ratew < rateh)
     {
        wh = ratew * HEIGHT;
        evas_object_resize(obj, ww, wh);
        return;
     }

   evas_object_resize(test_data->layout, ww, wh);
   ephysics_world_rate_set(test_data->world, 30 * rateh);
   ephysics_world_render_geometry_set(test_data->world, 50 * ratew, 40 *rateh,
                                      (WIDTH - 100) * ratew,
                                      (FLOOR_Y - 40) * rateh);

   old_rate = test_data->data;
   if (!old_rate) return;
   sphere_body = old_rate->body;

   sphere = ephysics_body_evas_object_get(sphere_body);
   evas_object_geometry_get(sphere, &x, &y, &w, &h);

   /* FIXME: add resize / move to API */
   ephysics_body_geometry_set(sphere_body, x * (ratew / old_rate->w),
                              y * (rateh / old_rate->h), 1, 1);
   evas_object_resize(sphere, 70 * ratew, 70 * rateh);

   old_rate->w = ratew;
   old_rate->h = rateh;
}

static void
_update_object_cb(void *data, EPhysics_Body *body, void *event_info)
{
   Evas_Object *sphere = event_info;
   Evas_Object *shadow = data;
   Edje_Message_Float msg;
   Old_Rate *old_rate;
   Evas_Object *edje;
   int x, y, w, h;

   ephysics_body_evas_object_update(body);
   old_rate = ephysics_body_data_get(body);
   evas_object_geometry_get(sphere, &x, &y, &w, &h);

   if (y > SH_THRESHOLD * old_rate->h)
     {
        int sh_w;

        msg.val = (double) (y - SH_THRESHOLD * old_rate->h) /
           ((FLOOR_Y - SH_THRESHOLD) * old_rate->h - h);
        sh_w = (1 + 2 * msg.val) * w / 3;
        evas_object_resize(shadow, sh_w, 3 * old_rate->h);
        evas_object_move(shadow, x + (w - sh_w) / 2, FLOOR_Y * old_rate->h);
     }
   else
     msg.val = 0;

   edje = elm_layout_edje_get(shadow);
   edje_object_message_send(edje, EDJE_MESSAGE_FLOAT, SHADOW_ALPHA_ID, &msg);
}

static void
_world_populate(Test_Data *test_data)
{
   Evas_Object *sphere, *shadow;
   EPhysics_Body *sphere_body;
   Old_Rate *old_rate;
   int w, h;

   old_rate = test_data->data;
   if (!test_data->data)
     old_rate = calloc(1, sizeof(Old_Rate));
   if (!old_rate) return;
   test_data->data = old_rate;

   evas_object_geometry_get(test_data->win, NULL, NULL, &w, &h);
   old_rate->w = (double) w / WIDTH;
   old_rate->h = (double) h / HEIGHT;

   shadow = elm_layout_add(test_data->win);
   elm_layout_file_set(
      shadow, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "shadow-ball");
   evas_object_move(shadow, WIDTH / 3 * old_rate->w, FLOOR_Y * old_rate->h);
   evas_object_resize(shadow, 70 * old_rate->w, 3 * old_rate->h);
   evas_object_show(shadow);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, shadow);

   sphere = elm_image_add(test_data->win);
   elm_image_file_set(
      sphere, PACKAGE_DATA_DIR "/" EPHYSICS_TEST_THEME ".edj", "big-blue-ball");
   evas_object_move(sphere, WIDTH / 3 * old_rate->w, HEIGHT / 2 * old_rate->h);
   evas_object_resize(sphere, 70 * old_rate->w, 70 * old_rate->h);
   evas_object_show(sphere);
   test_data->evas_objs = eina_list_append(test_data->evas_objs, sphere);

   sphere_body = ephysics_body_circle_add(test_data->world);
   ephysics_body_evas_object_set(sphere_body, sphere, EINA_TRUE);
   ephysics_body_restitution_set(sphere_body, 1.0);
   ephysics_body_event_callback_add(sphere_body, EPHYSICS_CALLBACK_BODY_UPDATE,
                                    _update_object_cb, shadow);
   test_data->bodies = eina_list_append(test_data->bodies, sphere_body);
   old_rate->body = sphere_body;
   ephysics_body_data_set(sphere_body, old_rate);
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
test_win_resize(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_Body *boundary;
   EPhysics_World *world;
   Test_Data *test_data;

   if (!ephysics_init())
     return;

   test_data = test_data_new();
   test_win_add(test_data, "Win Resize", EINA_TRUE);
   evas_object_event_callback_add(test_data->win, EVAS_CALLBACK_RESIZE,
                                  _win_resize_cb, test_data);

   elm_layout_signal_callback_add(test_data->layout, "restart", "test-theme",
                                  _restart, test_data);
   elm_object_signal_emit(test_data->layout, "borders,show", "ephysics_test");
   evas_object_size_hint_weight_set(test_data->layout, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   elm_win_aspect_set(test_data->win, (double) WIDTH / HEIGHT);
   evas_object_smart_callback_add(test_data->win, "delete,request", _del_cb,
                                  test_data->data);

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 50, 40, WIDTH - 100, FLOOR_Y - 40);
   test_data->world = world;

   boundary = ephysics_body_bottom_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 1.0);

   boundary = ephysics_body_right_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 1.0);

   boundary = ephysics_body_left_boundary_add(test_data->world);
   ephysics_body_restitution_set(boundary, 1.0);

   ephysics_body_top_boundary_add(test_data->world);

   _world_populate(test_data);
}
