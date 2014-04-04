#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

#define MAX_TRY 40

static int try_num = 0;

static void
_timer_del(void *data       EINA_UNUSED,
           Evas *e          EINA_UNUSED,
           Evas_Object     *obj,
           void *event_info EINA_UNUSED)
{
   Ecore_Timer *timer = evas_object_data_del(obj, "test-timer");
   if (!timer) return;
   ecore_timer_del(timer);
}

static Eina_Bool
cb_plug_connect(void *data)
{
   Evas_Object *obj = data;
   Ecore_Timer *timer;

   if (!obj) return ECORE_CALLBACK_CANCEL;

   try_num++;
   if (try_num > MAX_TRY) return ECORE_CALLBACK_CANCEL;

   timer= evas_object_data_get(obj, "test-timer");
   if (!timer) return ECORE_CALLBACK_CANCEL;

   if (elm_plug_connect(obj, "ello", 0, EINA_FALSE))
     {
        printf("plug connect to server[ello]\n");
        evas_object_data_del(obj, "test-timer");
        return ECORE_CALLBACK_CANCEL;
     }

   ecore_timer_interval_set(timer, 1);
   return ECORE_CALLBACK_RENEW;
}

static void
cb_plug_disconnected(void *data EINA_UNUSED,
                    Evas_Object *obj,
                    void *event_info EINA_UNUSED)
{
   Ecore_Timer *timer = evas_object_data_get(obj, "test-timer");
   if (timer)
     {
        ecore_timer_del(timer);
        evas_object_data_del(obj, "test-timer");
     }

   timer = ecore_timer_add(1, cb_plug_connect, obj);
   evas_object_data_set(obj, "test-timer", timer);
}

static void
cb_plug_resized(void *data EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info)
{
   Evas_Coord_Size *size = event_info;
   printf("server image resized to %dx%d\n", size->w, size->h);
}

static void
cb_mouse_down(void *data EINA_UNUSED, Evas *evas EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;

   if (ev->button == 1) elm_object_focus_set(obj, EINA_TRUE);
}

static void
cb_mouse_move(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Object *orig = data;
   Evas_Coord x, y;
   Evas_Map *p;
   int i, w, h;

   if (!ev->buttons) return;
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   evas_object_move(obj,
                    x + (ev->cur.canvas.x - ev->prev.output.x),
                    y + (ev->cur.canvas.y - ev->prev.output.y));
   evas_object_image_size_get(orig, &w, &h);
   p = evas_map_new(4);
   evas_object_map_enable_set(orig, EINA_TRUE);
   evas_object_raise(orig);
   for (i = 0; i < 4; i++)
     {
        Evas_Object *hand;
        char key[32];

        snprintf(key, sizeof(key), "h-%i\n", i);
        hand = evas_object_data_get(orig, key);
        evas_object_raise(hand);
        evas_object_geometry_get(hand, &x, &y, NULL, NULL);
        x += 15;
        y += 15;
        evas_map_point_coord_set(p, i, x, y, 0);
        if (i == 0) evas_map_point_image_uv_set(p, i, 0, 0);
        else if (i == 1) evas_map_point_image_uv_set(p, i, w, 0);
        else if (i == 2) evas_map_point_image_uv_set(p, i, w, h);
        else if (i == 3) evas_map_point_image_uv_set(p, i, 0, h);
     }
   evas_object_map_set(orig, p);
   evas_map_free(p);
}

static void
create_handles(Evas_Object *obj)
{
   int i;
   Evas_Coord x, y, w, h;

   evas_object_geometry_get(obj, &x, &y, &w, &h);
   for (i = 0; i < 4; i++)
     {
        Evas_Object *hand;
        char buf[PATH_MAX];
        char key[32];

        hand = evas_object_image_filled_add(evas_object_evas_get(obj));
        evas_object_resize(hand, 31, 31);
        snprintf(buf, sizeof(buf), "%s/images/pt.png", elm_app_data_dir_get());
        evas_object_image_file_set(hand, buf, NULL);
        if (i == 0)      evas_object_move(hand, x     - 15, y     - 15);
        else if (i == 1) evas_object_move(hand, x + w - 15, y     - 15);
        else if (i == 2) evas_object_move(hand, x + w - 15, y + h - 15);
        else if (i == 3) evas_object_move(hand, x     - 15, y + h - 15);
        evas_object_event_callback_add(hand, EVAS_CALLBACK_MOUSE_MOVE, cb_mouse_move, obj);
        evas_object_show(hand);
        snprintf(key, sizeof(key), "h-%i\n", i);
        evas_object_data_set(obj, key, hand);
     }
}

void
test_win_plug(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bg, *plug;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "window-plug", ELM_WIN_BASIC);
   elm_win_title_set(win, "Window Plug");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", elm_app_data_dir_get());
   elm_bg_file_set(bg, buf, NULL);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   plug = elm_plug_add(win);
   evas_object_event_callback_add(elm_plug_image_object_get(plug), EVAS_CALLBACK_MOUSE_DOWN, cb_mouse_down, NULL);
   evas_object_event_callback_add(plug, EVAS_CALLBACK_DEL, _timer_del, NULL);
   if (!elm_plug_connect(plug, "ello", 0, EINA_FALSE))
     {
        printf("Cannot connect plug\n");
        return;
     }

   evas_object_smart_callback_add(plug, "image,deleted", cb_plug_disconnected, NULL);
   evas_object_smart_callback_add(plug, "image,resized", cb_plug_resized, NULL);

   evas_object_resize(plug, 380, 500);
   evas_object_move(plug, 10, 10);
   evas_object_show(plug);

   create_handles(elm_plug_image_object_get(plug));

   evas_object_resize(win, 400, 600);
   evas_object_show(win);
}
