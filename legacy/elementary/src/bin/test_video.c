
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

static void
my_bt_open(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   Evas_Object *video = data;
   const char *file = event_info;

   if (file)
     {
        elm_video_file_set(video, file);
        elm_video_play(video);
     }
}

static void
_notify_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Coord w, h;
   
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   evas_object_resize(data, w, h);
}

static void
_notify_show(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_show(data);
}

static void
_notify_block(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("in block\n");
   elm_notify_timeout_set(data, 0.0);
   evas_object_show(data);
}

static void
_notify_unblock(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("out block\n");
   elm_notify_timeout_set(data, 10.0);
   evas_object_show(data);
}

void
test_video(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *video, *bt, *tb, *notify, *player, *grid, *rect;

   win = elm_win_add(NULL, "video", ELM_WIN_BASIC);
   elm_win_title_set(win, "Video");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   video = elm_video_add(win);
   evas_object_size_hint_weight_set(video, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, video);
   evas_object_show(video);

   notify = elm_notify_add(win);
   elm_notify_align_set(notify, 0.5, 1.0);

   player = elm_player_add(win);
   elm_object_content_set(player, video);
   elm_object_content_set(notify, player);
   evas_object_show(player);

   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, tb);

   bt = elm_fileselector_button_add(win);
   elm_object_text_set(bt, "Select Video");
   evas_object_smart_callback_add(bt, "file,chosen", my_bt_open, video);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.5, 0.1);
   elm_table_pack(tb, bt, 0, 0, 1, 1);
   evas_object_show(bt);

   evas_object_show(tb);

   grid = elm_grid_add(win);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_grid_size_set(grid, 100, 100);
   evas_object_event_callback_add(win, EVAS_CALLBACK_RESIZE, _notify_resize, grid);
   evas_object_layer_set(grid, 32767);
   evas_object_show(grid);
   
   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rect, 0, 0, 0, 0);
   evas_object_repeat_events_set(rect, EINA_TRUE);
   elm_grid_pack(grid, rect, 0, 75, 100, 25);
   evas_object_show(rect);
   evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_MOVE, _notify_show, notify);
   evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_IN, _notify_block, notify);
   evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_OUT, _notify_unblock, notify);

   evas_object_resize(win, 800, 500);
   evas_object_show(win);
}
#endif
