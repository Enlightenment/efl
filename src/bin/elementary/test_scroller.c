#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

// FIXME: Some internal APIs are used here...
#include "elm_interface_scrollable.h"

#define PSIZE 318

static void
_my_bt_go_300_300(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_scroller_region_bring_in((Evas_Object *)data, 300, 300, PSIZE, PSIZE);
}

static void
_my_bt_go_900_300(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_scroller_region_bring_in((Evas_Object *)data, 900, 300, PSIZE, PSIZE);
}

static void
_my_bt_go_300_900(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_scroller_region_bring_in((Evas_Object *)data, 300, 900, PSIZE, PSIZE);
}

static void
_my_bt_go_900_900(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_scroller_region_bring_in((Evas_Object *)data, 900, 900, PSIZE, PSIZE);
}

static void
_my_bt_y_minus_one(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   int x, y, w, h;
   elm_scroller_region_get((Evas_Object *)data, &x, &y, &w, &h);
   printf("Current region: %d %d %d %d\n", x, y, w, h);
   elm_scroller_region_show((Evas_Object *)data, x, y - 1, w, h);
}

static void
_my_bt_y_plus_one(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   int x, y, w, h;
   elm_scroller_region_get((Evas_Object *)data, &x, &y, &w, &h);
   printf("Current region: %d %d %d %d\n", x, y, w, h);
   elm_scroller_region_show((Evas_Object *)data, x, y + 1, w, h);
}

static void
_my_bt_prev_page(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
	Evas_Object *sc = (Evas_Object *)data;
	int page_x = 0, page_y = 0;
	elm_scroller_current_page_get(sc, &page_x, &page_y);
	elm_scroller_page_bring_in(sc, --page_x, page_y);
}

static void
_my_bt_next_page(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
	Evas_Object *sc = (Evas_Object *)data;
	int page_x = 0, page_y = 0;
	elm_scroller_current_page_get(sc, &page_x, &page_y);
	elm_scroller_page_bring_in(sc, ++page_x, page_y);
}

static void
_my_bt_freeze_toggle(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   if (elm_check_state_get(obj))
     elm_object_scroll_freeze_push((Evas_Object *)data);
   else
     elm_object_scroll_freeze_pop((Evas_Object *)data);
}

static void
_my_bt_hold_toggle(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   if (elm_check_state_get(obj))
     elm_object_scroll_hold_push((Evas_Object *)data);
   else
     elm_object_scroll_hold_pop((Evas_Object *)data);
}

static void
_my_bt_block_movements_x_axis(void *data, Evas_Object *obj,
                             void *event_info EINA_UNUSED)
{
   Elm_Scroller_Movement_Block block;

   block = elm_scroller_movement_block_get((Evas_Object *)data);

   if (elm_check_state_get(obj))
     {
        elm_scroller_movement_block_set((Evas_Object *)data,
                                        ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL |
                                        block);
     }
   else if (block & ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL)
     {
        elm_scroller_movement_block_set((Evas_Object *)data,
                                        ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL);
     }
   else
     {
        elm_scroller_movement_block_set((Evas_Object *)data,
                                        ELM_SCROLLER_MOVEMENT_NO_BLOCK);
     }
}

static void
_my_bt_snap_to_pages(void *data,
                    Evas_Object *obj,
                    void *event_info EINA_UNUSED)
{
   if (elm_check_state_get(obj))
     {
        elm_scroller_page_snap_set(data, EINA_TRUE, EINA_TRUE);
     }
   else
     {
        elm_scroller_page_snap_set(data, EINA_FALSE, EINA_FALSE);
     }
}

static void
_my_bt_block_movements_y_axis(void *data, Evas_Object *obj,
                             void *event_info EINA_UNUSED)
{
   Elm_Scroller_Movement_Block block;

   block = elm_scroller_movement_block_get((Evas_Object *)data);

   if (elm_check_state_get(obj))
     {
        elm_scroller_movement_block_set((Evas_Object *)data,
                                        ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL |
                                        block);
     }
   else if (block & ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL)
     {
        elm_scroller_movement_block_set((Evas_Object *)data,
                                        ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL);
     }
   else
     {
        elm_scroller_movement_block_set((Evas_Object *)data,
                                        ELM_SCROLLER_MOVEMENT_NO_BLOCK);
     }
}

static void
_my_bt_loop_x_axis(void *data, Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED)
{
   Eina_Bool loop_h, loop_v;
   Evas_Object *scroller = (Evas_Object *)data;

   elm_scroller_loop_get(scroller, &loop_h, &loop_v);
   elm_scroller_loop_set(scroller, !loop_h, loop_v);
}

static void
_my_bt_loop_y_axis(void *data, Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED)
{
   Eina_Bool loop_h, loop_v;
   Evas_Object *scroller = (Evas_Object *)data;

   elm_scroller_loop_get(scroller, &loop_h, &loop_v);
   elm_scroller_loop_set(scroller, loop_h, !loop_v);
}

static void
_my_bt_wheel_disable_cb(void *data, Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED)
{
   Evas_Object *scroller = (Evas_Object *)data;

   elm_scroller_wheel_disabled_set(scroller, elm_check_state_get(obj));
}

static void
_page_change_cb(void *data EINA_UNUSED,
               Evas_Object *obj,
               void *event_info EINA_UNUSED)
{
   int page_x = 0, page_y = 0;

   elm_scroller_current_page_get(obj, &page_x, &page_y);

   printf("Page changed to %d, %d\n", page_x, page_y);
}

static void
_sc_move_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Coord x = 0, y = 0;
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   evas_object_move(data, x, y);
}

static void
_sc_resize_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Coord w = 0, h = 0;
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   evas_object_resize(data, w, h);
}

static void
_step_size_changed(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *sc = data;
   int size = elm_spinner_value_get(obj);
   elm_scroller_step_size_set(sc, ELM_SCALE_SIZE(size), ELM_SCALE_SIZE(size));
}

static void
_page_size_changed(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *sc = data;
   int size = elm_spinner_value_get(obj);
   elm_scroller_page_size_set(sc, size, size);
}

void
test_scroller(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bg2, *tb, *tb2, *sc, *bt, *ck1, *ck2, *bx, *bx2, *fr,
       *ck3, *ck4, *ck5, *ck6, *ck7, *ck8;
   int i, j, n;
   char buf[PATH_MAX];
   Evas_Coord x = 0, y = 0, w = 0, h = 0;
   const char *img[9] =
     {
        "panel_01.jpg",
        "plant_01.jpg",
        "rock_01.jpg",
        "rock_02.jpg",
        "sky_01.jpg",
        "sky_02.jpg",
        "sky_03.jpg",
        "sky_04.jpg",
        "wood_01.jpg"
     };

   win = elm_win_util_standard_add("scroller", "Scroller");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, 0.0);
   elm_object_text_set(fr, "Options");
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   bx2 = elm_box_add(win);
   elm_object_content_set(fr, bx2);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_show(bx2);

   ck1 = elm_check_add(win);
   elm_object_text_set(ck1, "Freeze");
   elm_box_pack_end(bx2, ck1);
   evas_object_show(ck1);

   ck2 = elm_check_add(win);
   elm_object_text_set(ck2, "Hold");
   elm_box_pack_end(bx2, ck2);
   evas_object_show(ck2);

   ck3 = elm_check_add(win);
   elm_object_text_set(ck3, "Block movements in X axis");
   elm_box_pack_end(bx2, ck3);
   evas_object_show(ck3);

   ck4 = elm_check_add(win);
   elm_object_text_set(ck4, "Block movements in Y axis");
   elm_box_pack_end(bx2, ck4);
   evas_object_show(ck4);

   ck5 = elm_check_add(win);
   elm_object_text_set(ck5, "Snap to pages");
   elm_box_pack_end(bx2, ck5);
   evas_object_show(ck5);

   ck6 = elm_check_add(win);
   elm_object_text_set(ck6, "Loop in X axis");
   elm_box_pack_end(bx2, ck6);
   evas_object_show(ck6);

   ck7 = elm_check_add(win);
   elm_object_text_set(ck7, "Loop in Y axis");
   elm_box_pack_end(bx2, ck7);
   evas_object_show(ck7);

   ck8 = elm_check_add(win);
   elm_object_text_set(ck8, "Wheel Disable");
   elm_box_pack_end(bx2, ck8);
   evas_object_show(ck8);

   sc = elm_scroller_add(win);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, sc);

   tb = elm_table_add(win);

   n = 0;
   for (j = 0; j < 12; j++)
     {
        for (i = 0; i < 12; i++)
          {
             bg2 = elm_bg_add(win);
             snprintf(buf, sizeof(buf), "%s/images/%s",
                      elm_app_data_dir_get(), img[n]);
             n++;
             if (n >= 9) n = 0;
             elm_bg_file_set(bg2, buf, NULL);
             evas_object_size_hint_weight_set(bg2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
             evas_object_size_hint_align_set(bg2, EVAS_HINT_FILL, EVAS_HINT_FILL);
             evas_object_size_hint_min_set(bg2, PSIZE, PSIZE);
             elm_table_pack(tb, bg2, i, j, 1, 1);
             evas_object_show(bg2);
          }
     }

   elm_object_content_set(sc, tb);
   evas_object_show(tb);

   elm_scroller_page_size_set(sc, PSIZE, PSIZE);
   evas_object_show(sc);

   evas_object_smart_callback_add
       (sc, "scroll,page,changed", _page_change_cb, sc);

   evas_object_smart_callback_add(ck1, "changed", _my_bt_freeze_toggle, tb);
   evas_object_smart_callback_add(ck2, "changed", _my_bt_hold_toggle, tb);
   evas_object_smart_callback_add(ck3, "changed", _my_bt_block_movements_x_axis,
                                  sc);
   evas_object_smart_callback_add(ck4, "changed", _my_bt_block_movements_y_axis,
                                  sc);
   evas_object_smart_callback_add(ck5, "changed", _my_bt_snap_to_pages, sc);
   evas_object_smart_callback_add(ck6, "changed", _my_bt_loop_x_axis, sc);
   evas_object_smart_callback_add(ck7, "changed", _my_bt_loop_y_axis, sc);
   evas_object_smart_callback_add(ck8, "changed", _my_bt_wheel_disable_cb, sc);

   bx2 = elm_box_add(win);
   elm_box_pack_end(bx, bx2);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_padding_set(bx2, 10, 0);
   evas_object_show(bx2);

   bt = elm_spinner_add(win);
   elm_spinner_min_max_set(bt, 0, 500);
   elm_scroller_step_size_get(sc, &x, &y);
   elm_spinner_value_set(bt, x);
   elm_spinner_editable_set(bt, EINA_TRUE);
   elm_spinner_label_format_set(bt, "Step size: %.0f");
   evas_object_smart_callback_add(bt, "changed", _step_size_changed, sc);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_spinner_add(win);
   elm_spinner_min_max_set(bt, 0, PSIZE * 2);
   elm_scroller_page_size_get(sc, &x, &y);
   elm_spinner_value_set(bt, x);
   elm_spinner_editable_set(bt, EINA_TRUE);
   elm_spinner_label_format_set(bt, "Page size: %.0f");
   evas_object_smart_callback_add(bt, "changed", _page_size_changed, sc);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   tb2 = elm_table_add(win);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "to 300 300");
   evas_object_smart_callback_add(bt, "clicked", _my_bt_go_300_300, sc);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.1, 0.1);
   elm_table_pack(tb2, bt, 0, 0, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "y -1 px");
   evas_object_smart_callback_add(bt, "clicked", _my_bt_y_minus_one, sc);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.5, 0.1);
   elm_table_pack(tb2, bt, 1, 0, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "to 900 300");
   evas_object_smart_callback_add(bt, "clicked", _my_bt_go_900_300, sc);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.9, 0.1);
   elm_table_pack(tb2, bt, 2, 0, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "to 300 900");
   evas_object_smart_callback_add(bt, "clicked", _my_bt_go_300_900, sc);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.1, 0.9);
   elm_table_pack(tb2, bt, 0, 2, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "y +1 px");
   evas_object_smart_callback_add(bt, "clicked", _my_bt_y_plus_one, sc);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.5, 0.9);
   elm_table_pack(tb2, bt, 1, 2, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "to 900 900");
   evas_object_smart_callback_add(bt, "clicked", _my_bt_go_900_900, sc);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.9, 0.9);
   elm_table_pack(tb2, bt, 2, 2, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "prev page");
   evas_object_smart_callback_add(bt, "clicked", _my_bt_prev_page, sc);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.1, 0.5);
   elm_table_pack(tb2, bt, 0, 1, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "next page");
   evas_object_smart_callback_add(bt, "clicked", _my_bt_next_page, sc);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.9, 0.5);
   elm_table_pack(tb2, bt, 2, 1, 1, 1);
   evas_object_show(bt);

   evas_object_event_callback_add(sc, EVAS_CALLBACK_MOVE, _sc_move_cb, tb2);
   evas_object_event_callback_add(sc, EVAS_CALLBACK_RESIZE, _sc_resize_cb, tb2);
   evas_object_geometry_get(sc, &x, &y, &w, &h);
   evas_object_move(tb2, x, y);
   evas_object_resize(tb2, w, h);
   evas_object_show(tb2);

   evas_object_resize(win, 320, 420);
   evas_object_show(win);
}

static void
_click_through(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   printf("click went through on %p\n", obj);
}

typedef struct
{
   Evas_Object *scroller;
   Evas_Object *it1, *it2;
   Ecore_Timer *timer;
   int autobounce;
   int frames;
   int bounce_max;
   int y1, y2;
   int state;
} Bounce;

#ifdef CLOCK_PROCESS_CPUTIME_ID
static void
_bounce_cb_frame(void *data, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Bounce *bounce = data;
   bounce->frames++;
}
#endif

static Eina_Bool
_bounce_cb(void *data)
{
   Bounce *bounce = data;

   if (!bounce->y1)
     {
        elm_interface_scrollable_bounce_allow_set(bounce->scroller, 0, 1);
        bounce->y1 = efl_gfx_entity_position_get(bounce->it1).y;
        bounce->y2 = efl_gfx_entity_position_get(bounce->it2).y;
     }

   bounce->state++;
   if (bounce->state & 0x1)
     elm_interface_scrollable_region_bring_in(bounce->scroller, 0, bounce->y2, 1, 1);
   else
     elm_interface_scrollable_region_bring_in(bounce->scroller, 0, bounce->y1, 1, 1);

#ifdef CLOCK_PROCESS_CPUTIME_ID
   static struct timespec t0;
   if (bounce->state == 2)
     {
        evas_event_callback_add(evas_object_evas_get(bounce->scroller),
                                EVAS_CALLBACK_RENDER_FLUSH_POST,
                                _bounce_cb_frame, bounce);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t0);
     }
#endif

   if (bounce->state > bounce->bounce_max)
     {
#ifdef CLOCK_PROCESS_CPUTIME_ID
        struct timespec t;
        unsigned long long tll, t0ll, tdll, frames;

        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t);
        t0ll = ((unsigned long long) t0.tv_sec * 1000000000ULL) + t0.tv_nsec;
        tll = ((unsigned long long) t.tv_sec * 1000000000ULL) + t.tv_nsec;
        tdll = tll - t0ll;
        frames = bounce->frames ?: 1;
        printf("NS since frame 2 = %llu , %llu frames = %llu / frame\n",
               tdll, frames, tdll / frames);
#endif
        if (bounce->autobounce) elm_exit();
     }
   return EINA_TRUE;
}

static void
_scroll2_del_cb(void *data, Evas *e EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Bounce *bounce = data;

#ifdef CLOCK_PROCESS_CPUTIME_ID
   evas_event_callback_del_full(evas_object_evas_get(bounce->scroller),
                                EVAS_CALLBACK_RENDER_FLUSH_POST,
                                _bounce_cb_frame, bounce);
#endif

   ecore_timer_del(bounce->timer);
   free(bounce);
}

void
test_scroller2(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bt, *bx, *bx2, *sc, *tb, *tb2, *rc;
   Bounce *bounce;
   int i, j;

   bounce = calloc(1, sizeof(Bounce));

   win = elm_win_util_standard_add("scroller2", "Scroller 2");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.0);

   /* { */
   for (i = 0; i < 3; i++)
     {
        bt = elm_button_add(win);
        elm_object_text_set(bt, "Vertical");
        evas_object_smart_callback_add(bt, "clicked", _click_through, NULL);
        evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
        evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0.5);
        elm_box_pack_end(bx, bt);
        evas_object_show(bt);

        if (i == 0) bounce->it1 = bt;
     }
   /* } */

   /* { */
   sc = elm_scroller_add(win);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sc, EVAS_HINT_FILL, 0.5);
   elm_scroller_bounce_set(sc, EINA_TRUE, EINA_FALSE);
   elm_scroller_content_min_limit(sc, 0, 1);
   elm_box_pack_end(bx, sc);
   evas_object_show(sc);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);

   for (i = 0; i < 10; i++)
     {
        bt = elm_button_add(win);
        elm_object_text_set(bt, "... Horizontal scrolling ...");
        evas_object_smart_callback_add(bt, "clicked", _click_through, NULL);
        elm_box_pack_end(bx2, bt);
        evas_object_show(bt);
     }

   elm_object_content_set(sc, bx2);
   evas_object_show(bx2);
   /* } */

   /* { */
   for (i = 0; i < 3; i++)
     {
        bt = elm_button_add(win);
        elm_object_text_set(bt, "Vertical");
        evas_object_smart_callback_add(bt, "clicked", _click_through, NULL);
        evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
        evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0.5);
        elm_box_pack_end(bx, bt);
        evas_object_show(bt);
     }
   /* } */

   /* { */
   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   rc = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_min_set(rc, 200, 120);
   elm_table_pack(tb, rc, 0, 0, 1, 1);

   sc = elm_scroller_add(win);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, sc, 0, 0, 1, 1);
   evas_object_show(sc);

   tb2 = elm_table_add(win);

   for (j = 0; j < 16; j++)
     {
        for (i = 0; i < 16; i++)
          {
             bt = elm_button_add(win);
             elm_object_text_set(bt, "Both");
             evas_object_smart_callback_add(bt, "clicked", _click_through, NULL);
             elm_table_pack(tb2, bt, i, j, 1, 1);
             evas_object_show(bt);
          }
     }

   elm_object_content_set(sc, tb2);
   evas_object_show(tb2);
   /* } */

   for (i = 0; i < 24; i++)
     {
        bt = elm_button_add(win);
        elm_object_text_set(bt, "Vertical");
        evas_object_smart_callback_add(bt, "clicked", _click_through, NULL);
        evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
        evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0.5);
        elm_box_pack_end(bx, bt);
        evas_object_show(bt);

        if (i == 23) bounce->it2 = bt;
     }

   sc = elm_scroller_add(win);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_scroller_bounce_set(sc, EINA_FALSE, EINA_TRUE);
   elm_win_resize_object_add(win, sc);
   elm_object_content_set(sc, bx);
   evas_object_show(bx);
   evas_object_show(sc);

   bounce->scroller = sc;

   evas_object_resize(win, 320, 480);
   evas_object_show(win);

   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _scroll2_del_cb, bounce);

   if (getenv("ELM_TEST_AUTOBOUNCE"))
     {
        bounce->autobounce = 1;
        bounce->bounce_max = atoi(getenv("ELM_TEST_AUTOBOUNCE"));
        bounce->timer = ecore_timer_add(0.5, _bounce_cb, bounce);
        _bounce_cb(bounce);
     }
}

static Ecore_Timer *_timer = NULL;
static int _append = 0;
static int _count = 0;

static void
_del_item(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   evas_object_del(obj);
}

static void
_append_item(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *bx = data, *bt;
   char buf[PATH_MAX];

   bt = elm_button_add(bx);
   snprintf(buf, sizeof(buf), "Item %d", ++_count);
   elm_object_text_set(bt, buf);
   elm_box_pack_end(bx, bt);
   evas_object_smart_callback_add(bt, "clicked", _del_item, NULL);
   evas_object_show(bt);
}

static void
_prepend_item(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *bx = data, *bt;
   char buf[PATH_MAX];

   bt = elm_button_add(bx);
   snprintf(buf, sizeof(buf), "Item %d", ++_count);
   elm_object_text_set(bt, buf);
   elm_box_pack_start(bx, bt);
   evas_object_smart_callback_add(bt, "clicked", _del_item, NULL);
   evas_object_show(bt);
}

static Eina_Bool
_append_cb(void *data)
{
   Evas_Object *bx = data, *bt;
   char buf[PATH_MAX];

   bt = elm_button_add(bx);
   snprintf(buf, sizeof(buf), "Item %d", ++_count);
   elm_object_text_set(bt, buf);
   elm_box_pack_end(bx, bt);
   evas_object_smart_callback_add(bt, "clicked", _del_item, NULL);
   evas_object_show(bt);

   _append--;

   if (_append <= 0)
     {
        _timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
   else
     return ECORE_CALLBACK_RENEW;
}

static void
_append_items(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _append += 10;
   _timer = ecore_timer_add(0.3, _append_cb, data);
}

static void
_changed_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
	Evas_Object *sl = data;
	double val = elm_slider_value_get(obj);
	elm_scroller_gravity_set(sl, 0.0, val);
	printf("Gravity change to %lf\n",val);
}

static void
_win_del_cb(void *data EINA_UNUSED,
		Evas *e EINA_UNUSED,
		Evas_Object *obj EINA_UNUSED,
		void *event_info EINA_UNUSED)
{
   ecore_timer_del(_timer);
   _timer = NULL;
}

void
test_scroller3(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bt, *bt2, *bt3, *bx, *bx2, *bx3, *bx4, *sc, *sl;
   _count = 0;

   win = elm_win_util_standard_add("scroller3", "Scroller 3");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_DEL, _win_del_cb, NULL);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   bx2 = elm_box_add(bx);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.1);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bt = elm_button_add(bx2);
   elm_object_text_set(bt, "Append Item");
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt2 = elm_button_add(bx2);
   elm_object_text_set(bt2, "Prepend Item");
   elm_box_pack_end(bx2, bt2);
   evas_object_show(bt2);

   bt3 = elm_button_add(bx2);
   elm_object_text_set(bt3, "Append 10 Items in 3s");
   elm_box_pack_end(bx2, bt3);
   evas_object_show(bt3);

   sl = elm_slider_add(bx);
   elm_object_text_set(sl, "Gravity");
   elm_slider_unit_format_set(sl, "%1.1f");
   elm_slider_indicator_format_set(sl, "%1.1f");
   elm_slider_min_max_set(sl, 0, 1);
   elm_slider_value_set(sl, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.1);
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   bx3 = elm_box_add(bx);
   evas_object_size_hint_align_set(bx3, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bx3, EVAS_HINT_EXPAND, 0.8);
   elm_box_pack_end(bx, bx3);
   evas_object_show(bx3);

   sc = elm_scroller_add(bx3);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx3, sc);
   evas_object_show(sc);

   bx4 = elm_box_add(sc);
   elm_box_padding_set(bx4, 0, 5);
   evas_object_size_hint_weight_set(bx4, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_content_set(sc, bx4);
   evas_object_show(bx4);

   evas_object_smart_callback_add(bt, "clicked", _append_item, bx4);
   evas_object_smart_callback_add(bt2, "clicked", _prepend_item, bx4);
   evas_object_smart_callback_add(bt3, "clicked", _append_items, bx4);
   evas_object_smart_callback_add(sl, "changed", _changed_cb, sc);

   evas_object_resize(win, 500, 500);
   evas_object_show(win);
}

void
test_scroller4(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *sc, *bx, *ly, *sc2, *rect, *rect2;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("scroller3", "Scroller 3");
   elm_win_autodel_set(win, EINA_TRUE);

   sc = elm_scroller_add(win);
   elm_scroller_loop_set(sc, EINA_TRUE, EINA_FALSE);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_scroller_page_relative_set(sc, 1.0, 0.0);
   elm_scroller_policy_set(sc, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_scroller_page_scroll_limit_set(sc, 1, 0);
   elm_win_resize_object_add(win, sc);
   evas_object_show(sc);

   bx = elm_box_add(sc);
   elm_box_horizontal_set(bx, EINA_TRUE);
   elm_object_content_set(sc, bx);
   evas_object_show(bx);

   ly = elm_layout_add(bx);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "page_layout");
   elm_object_part_text_set(ly, "text", "Page1");
   elm_box_pack_end(bx, ly);
   evas_object_show(ly);

   sc2 = elm_scroller_add(ly);
   elm_object_part_content_set(ly, "page", sc2);
   evas_object_show(sc2);

   rect2 = evas_object_rectangle_add(evas_object_evas_get(sc2));
   evas_object_color_set(rect2, 50, 0, 0, 50);
   evas_object_size_hint_min_set(rect2, 0, 2000);
   elm_object_content_set(sc2, rect2);
   evas_object_show(rect2);

   ly = elm_layout_add(bx);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "page_layout");
   rect = evas_object_rectangle_add(evas_object_evas_get(ly));
   evas_object_color_set(rect, 0, 50, 0, 50);
   elm_object_part_content_set(ly, "page", rect);
   elm_object_part_text_set(ly, "text", "Page2");
   elm_box_pack_end(bx, ly);
   evas_object_show(ly);

   ly = elm_layout_add(bx);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "page_layout");
   rect = evas_object_rectangle_add(evas_object_evas_get(ly));
   evas_object_color_set(rect, 0, 0, 50, 50);
   elm_object_part_content_set(ly, "page", rect);
   elm_object_part_text_set(ly, "text", "Page3");
   elm_box_pack_end(bx, ly);
   evas_object_show(ly);

   evas_object_resize(win, 400, 550);
   evas_object_show(win);
}

static void
_popup_btn_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   evas_object_del(data);
}

static void
_block_clicked_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   evas_object_del(obj);
}

static void
_btn_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   Evas_Object *popup, *scr, *lbl, *btn, *rect, *tbl;

   popup = elm_popup_add(win);
   elm_object_part_text_set(popup, "title,text", "Scroller on Popup");
   evas_object_smart_callback_add(popup, "block,clicked",
                                  _block_clicked_cb, NULL);

   tbl = elm_table_add(popup);
   elm_object_content_set(popup, tbl);
   evas_object_show(tbl);

   rect = evas_object_rectangle_add(evas_object_evas_get(popup));
   evas_object_size_hint_min_set(rect, 200, 50);
   elm_table_pack(tbl, rect, 0, 0, 1, 1);

   scr = elm_scroller_add(popup);
   elm_object_focus_allow_set(scr, EINA_FALSE);
   evas_object_size_hint_weight_set(scr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(scr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(scr);

   lbl = elm_label_add(scr);
   elm_object_text_set(lbl,
                       "Do you want close the popup?<br>"
                       "Please select cancel button<br><br>"
                       "Do you wnat close the popup?<br>"
                       "Please select cancel button");
   evas_object_size_hint_weight_set(lbl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(lbl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(scr, lbl);
   evas_object_show(lbl);

   elm_table_pack(tbl, scr, 0, 0, 1, 1);

   btn = elm_button_add(popup);
   elm_object_text_set(btn, "Cancel");
   elm_object_part_content_set(popup, "button1", btn);
   evas_object_smart_callback_add(btn, "clicked", _popup_btn_clicked_cb, popup);
   evas_object_show(btn);

   evas_object_show(popup);
   elm_object_focus_set(btn, EINA_TRUE);
}

static void
_focused_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("focused");
}

void
test_scroller5(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *btn;

   win = elm_win_util_standard_add("Scroller on Popup", "Scroller on Popup");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   btn = elm_button_add(bx);
   elm_object_text_set(btn, "Button 1");
   evas_object_smart_callback_add(btn, "clicked", _btn_clicked_cb, win);
   evas_object_smart_callback_add(btn, "focused", _focused_cb, NULL);
   elm_box_pack_end(bx, btn);
   evas_object_show(btn);

   btn = elm_button_add(bx);
   elm_object_text_set(btn, "Button 2");
   evas_object_smart_callback_add(btn, "clicked", _btn_clicked_cb, win);
   evas_object_smart_callback_add(btn, "focused", _focused_cb, NULL);
   elm_box_pack_end(bx, btn);
   evas_object_show(btn);

   elm_object_focus_set(btn, EINA_TRUE);

   evas_object_resize(win, 400, 550);
   evas_object_show(win);
}
