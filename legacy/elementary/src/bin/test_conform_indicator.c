#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static int vis = 0;
static int rotate_with_resize = 0;
static Evas_Object *win_port = NULL;
static Evas_Object *win_land = NULL;

static void
_rot_0(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   if (rotate_with_resize)
     elm_win_rotation_with_resize_set(win, 0);
   else
     elm_win_rotation_set(win, 0);
}

static void
_rot_90(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   if (rotate_with_resize)
     elm_win_rotation_with_resize_set(win, 90);
   else
     elm_win_rotation_set(win, 90);
}

static void
_rot_180(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   if (rotate_with_resize)
     elm_win_rotation_with_resize_set(win, 180);
   else
     elm_win_rotation_set(win, 180);
}

static void
_rot_270(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   if (rotate_with_resize)
     elm_win_rotation_with_resize_set(win, 270);
   else
     elm_win_rotation_set(win, 270);
}

static void
_visible_change_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;

   printf("visible change before = %d\n", vis);
   if (vis == 0)
     {
        elm_win_indicator_mode_set(win, ELM_WIN_INDICATOR_SHOW);
        vis = 1;
     }
   else
     {
        elm_win_indicator_mode_set(win, ELM_WIN_INDICATOR_HIDE);
        vis = 0;
     }

   /*Add App code here*/
   printf("visible change after = %d\n", vis);
}

static void
_launch_conformant_indicator_window_btn_cb(void *data EINA_UNUSED,
                                           Evas_Object *obj EINA_UNUSED,
                                           void *event_info EINA_UNUSED)
{
   Evas_Object *win, *conform, *bt, *bx;
   Evas_Object *plug_port = NULL, *plug_land = NULL;

   win = elm_win_util_standard_add("conformant-indicator",
                                   "Conformant Indicator");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_conformant_set(win, EINA_TRUE);

   //Create conformant
   conform = elm_conformant_add(win);
   evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, conform);
   evas_object_show(conform);

   bx = elm_box_add(conform);
   elm_object_content_set(conform, bx);
   evas_object_show(bx);

   bt = elm_button_add(bx);
   elm_object_text_set(bt, "Rot 0");
   evas_object_smart_callback_add(bt, "clicked", _rot_0, win);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(bx);
   elm_object_text_set(bt, "Rot 90");
   evas_object_smart_callback_add(bt, "clicked", _rot_90, win);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(bx);
   elm_object_text_set(bt, "Rot 180");
   evas_object_smart_callback_add(bt, "clicked", _rot_180, win);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(bx);
   elm_object_text_set(bt, "Rot 270");
   evas_object_smart_callback_add(bt, "clicked", _rot_270, win);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   /* portrait plug */
   plug_port = elm_plug_add(bx);
   if (!plug_port)
     {
        printf("fail to create plug to server [elm_indicator_portrait]\n");
        evas_object_del(win);
        return;
     }

   if (!elm_plug_connect(plug_port, "elm_indicator_portrait", 0, EINA_FALSE))
     {
        printf("fail to connect to server [elm_indicator_portraits]\n");
        evas_object_del(win);
        return;
     }
   evas_object_size_hint_weight_set(plug_port, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(plug_port, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(plug_port);
   elm_box_pack_end(bx, plug_port);

   /* landscape plug */
   plug_land = elm_plug_add(bx);
   if (!plug_land)
     {
        printf("fail to create plug to server [elm_indicator_landscape]\n");
        evas_object_del(win);
        return;
     }
   if (!elm_plug_connect(plug_land, "elm_indicator_landscape", 0, EINA_FALSE))
     {
        printf("fail to connect to server [elm_indicator_landscape]\n");
        evas_object_del(win);
        return;
     }
   evas_object_size_hint_weight_set(plug_land, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(plug_land, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(plug_land);
   elm_box_pack_end(bx, plug_land);

   bt = elm_button_add(bx);
   elm_object_text_set(bt, "Show/Hide");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked", _visible_change_cb, win);

   evas_object_resize(win, 400, 600);
   evas_object_show(win);
}

static void
_mouse_down_cb(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Object *sub_obj = data;

   printf("\nAPP mouse down [%d]x[%d] obj=%p type=%s\n", ev->canvas.x, ev->canvas.y,sub_obj,evas_object_type_get(sub_obj));

   evas_object_move(sub_obj, ev->canvas.x, ev->canvas.y);
}

static void
_mouse_move_cb(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Object *sub_obj = data;
   if (!ev->buttons) return;

   printf("APP mouse move [%d]x[%d] obj=%p type=%s\n", ev->cur.canvas.x, ev->cur.canvas.y,sub_obj,evas_object_type_get(sub_obj));
   evas_object_move(sub_obj, ev->cur.canvas.x, ev->cur.canvas.y);
}

static void
_fill_portrait(Evas_Object *win)
{
   Evas_Object *bg, *btn;

   bg = elm_bg_add(win);
   evas_object_resize(bg, 720, 60);
   elm_bg_color_set(bg, 221, 187, 187);
   evas_object_show(bg);

   btn = elm_button_add(win);
   if (!btn)
     {
        printf("fail to elm_button_add() \n");
        return;
     }

   elm_object_text_set(btn, "portrait");

   evas_object_resize(btn, 200, 50);
   evas_object_move(btn, 260, 0);
   evas_object_show(btn);

   // This checks whether the indicator gets mouse event from application correctly
   evas_object_event_callback_add(bg, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, btn);
   evas_object_event_callback_add(bg, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb, btn);
}

static void
_fill_landscape(Evas_Object *win)
{
   Evas_Object *bg, *btn;

   bg = elm_bg_add(win);
   evas_object_resize(bg, 1280, 60);
   elm_bg_color_set(bg, 207, 255, 255);
   evas_object_show(bg);

   btn = elm_button_add(win);
   if (!btn)
     {
        printf("fail to elm_button_add() \n");
        return;
     }

   elm_object_text_set(btn, "landscape");

   evas_object_resize(btn, 200, 50);
   evas_object_move(btn, 580, 0);
   evas_object_show(btn);

   // This checks whether the indicator gets mouse event from application correctly
   evas_object_event_callback_add(bg, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, btn);
   evas_object_event_callback_add(bg, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb, btn);
}

static Evas_Object *
_create_portrait(void)
{
   const char *port_indi_name;

   win_port = elm_win_add(NULL, "portrait_indicator", ELM_WIN_SOCKET_IMAGE);
   if (!win_port)
     {
        printf("fail to create a portrait indicator window\n");
        return NULL;
     }
   elm_win_autodel_set(win_port, EINA_TRUE);

   port_indi_name = elm_config_indicator_service_get(0);
   if (!port_indi_name)
     {
        printf("indicator has no portrait service name: use default name\n");
        port_indi_name = "elm_indicator_portrait";
     }

   if (!elm_win_socket_listen(win_port, port_indi_name, 0, EINA_FALSE))
     {
        printf("failed to listen portrait window socket.\n");
        evas_object_del(win_port);
        return NULL;
     }

   elm_win_title_set(win_port, "win sock test:port");
   elm_win_borderless_set(win_port, EINA_TRUE);

   evas_object_move(win_port, 0, 0);
   evas_object_resize(win_port, 720, 60);

   _fill_portrait(win_port);

   return win_port;
}

static Evas_Object *
_create_landscape(void)
{
   const char *land_indi_name;

   win_land = elm_win_add(NULL, "win_socket_test:land", ELM_WIN_SOCKET_IMAGE);
   if (!win_land)
     {
        printf("fail to create a landscape indicator window\n");
        return NULL;
     }
   elm_win_autodel_set(win_land, EINA_TRUE);

   land_indi_name = elm_config_indicator_service_get(90);
   if (!land_indi_name)
     {
        printf("indicator has no landscape service name: use default name\n");
        land_indi_name = "elm_indicator_landscape";
     }

   if (!elm_win_socket_listen(win_land, land_indi_name, 0, 0))
     {
        printf("fail to listen landscape window socket.\n");
        evas_object_del(win_land);
        return NULL;
     }

   elm_win_title_set(win_land, "win sock test:land");
   elm_win_borderless_set(win_land, EINA_TRUE);

   evas_object_move(win_land, 0, 0);
   evas_object_resize(win_land, 1280, 60);

   _fill_landscape(win_land);

   return win_land;
}

static void
_indicator_service_start_btn_cb(void *data, Evas_Object *obj EINA_UNUSED,
                                void *event EINA_UNUSED)
{
   elm_object_disabled_set(data, EINA_TRUE);

   win_port = _create_portrait();
   if (!win_port) return;

   win_land = _create_landscape();
   if (!win_land) return;

   evas_object_show(win_port);
   evas_object_show(win_land);

}

void
test_conformant_indicator(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                          void *event EINA_UNUSED)
{
   Evas_Object *win, *btn, *bx;

   win = elm_win_util_standard_add("conformant-indicator-service",
                                   "Conformant Indicator Service");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Start Indicator Service");
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "clicked",
                                  _indicator_service_start_btn_cb, btn);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Launch Conformant Indicator Window");
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "clicked",
                                  _launch_conformant_indicator_window_btn_cb, NULL);

   evas_object_resize(win, 200, 200);
   evas_object_show(win);
}
