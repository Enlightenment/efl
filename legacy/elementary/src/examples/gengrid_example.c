/**
 * Simple Elementary's <b>gengrid widget</b> example, illustrating its
 * usage and API.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g `pkg-config --cflags --libs elementary` gengrid_example.c -o gengrid_example
 * @endverbatim
 */

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
# define PACKAGE_DATA_DIR "../../data"
#endif

typedef struct _Example_Item
{
   const char *path;
} Example_Item;

static const char *imgs[9] =
{
   "panel_01.jpg",
   "plant_01.jpg",
   "rock_01.jpg",
   "rock_02.jpg",
   "sky_01.jpg",
   "sky_02.jpg",
   "sky_03.jpg",
   "sky_04.jpg",
   "wood_01.jpg",
};

static Elm_Gengrid_Item_Class gic;
Evas_Object *before_bt, *after_bt;

static void
_on_done(void        *data __UNUSED__,
         Evas_Object *obj __UNUSED__,
         void        *event_info __UNUSED__)
{
   elm_exit();
}

/* change layouting mode */
static void
_horizontal_grid(void        *data,
                 Evas_Object *obj,
                 void        *event_info __UNUSED__)
{
   Evas_Object *grid = data;

   elm_gengrid_horizontal_set(grid, elm_check_state_get(obj));
}

/* "always select" callback */
static void
_always_select_change(void        *data,
                      Evas_Object *obj,
                      void        *event_info __UNUSED__)
{
   Evas_Object *grid = data;
   Eina_Bool always = elm_check_state_get(obj);

   elm_gengrid_always_select_mode_set(grid, always);

   fprintf(stdout, "\"Always select\" mode for gengrid items is now %s\n",
           always ? "on" : "off");
}

/* "bouncing mode" callback */
static void
_bouncing_change(void        *data,
                 Evas_Object *obj,
                 void        *event_info __UNUSED__)
{
   Evas_Object *grid = data;
   Eina_Bool bounce = elm_check_state_get(obj);

   elm_gengrid_bounce_set(grid, bounce, bounce);

   fprintf(stdout, "Bouncing effect for gengrid is now %s\n",
           bounce ? "on" : "off");
}

/* multi-selection callback */
static void
_multi_change(void        *data,
              Evas_Object *obj,
              void        *event_info __UNUSED__)
{
   Evas_Object *grid = data;
   Eina_Bool multi = elm_check_state_get(obj);

   elm_gengrid_multi_select_set(grid, multi);

   fprintf(stdout, "Multi-selection for gengrid is now %s\n",
           multi ? "on" : "off");

   elm_object_disabled_set(before_bt, multi);
   elm_object_disabled_set(after_bt, multi);

   if (!multi)
     {
        Elm_Gengrid_Item *it;
        const Eina_List *selected = elm_gengrid_selected_items_get(grid), *l;
        EINA_LIST_FOREACH(selected, l, it)
          elm_gengrid_item_selected_set(it, EINA_FALSE);
     }
}

/* no selection callback */
static void
_no_sel_change(void        *data,
               Evas_Object *obj,
               void        *event_info __UNUSED__)
{
   Evas_Object *grid = data;
   Eina_Bool no_sel = elm_check_state_get(obj);

   elm_gengrid_no_select_mode_set(grid, no_sel);

   fprintf(stdout, "Selection for gengrid items is now %s\n",
           no_sel ? "disabled" : "enabled");
}

/* item selection callback */
static void
_grid_sel(void        *data,
          Evas_Object *obj __UNUSED__,
          void        *event_info)
{
   unsigned int x, y;
   Example_Item *it = elm_gengrid_item_data_get(event_info);

   elm_gengrid_item_pos_get(event_info, &x, &y);

   fprintf(stdout, "Item [%p], with data [%p], path %s, at position (%d, %d),"
                   " has been selected\n", event_info, data, it->path, x, y);
}

/* new item with random path */
static Example_Item *
_item_new(void)
{
   Example_Item *it;

   it = malloc(sizeof(*it));
   it->path = eina_stringshare_add(imgs[rand() % (sizeof(imgs) /
                                                  sizeof(imgs[0]))]);
   return it;
}

/* "insert before" callback */
static void
_before_bt_clicked(void        *data,
                   Evas_Object *obj __UNUSED__,
                   void        *event_info __UNUSED__)
{
   Example_Item *it;
   Evas_Object *grid = data;
   Elm_Gengrid_Item *sel;

   sel = elm_gengrid_selected_item_get(grid);
   if (!sel)
     return;

   it = _item_new();
   elm_gengrid_item_insert_before(grid, &gic, it, sel, _grid_sel, NULL);
}

/* "insert after" callback */
static void
_after_bt_clicked(void        *data,
                  Evas_Object *obj __UNUSED__,
                  void        *event_info __UNUSED__)
{
   Example_Item *it;
   Evas_Object *grid = data;
   Elm_Gengrid_Item *sel;

   sel = elm_gengrid_selected_item_get(grid);
   if (!sel)
     return;

   it = _item_new();
   elm_gengrid_item_insert_after(grid, &gic, it, sel, _grid_sel, NULL);
}

/* prepend an item */
static void
_prepend_bt_clicked(void        *data,
                    Evas_Object *obj __UNUSED__,
                    void        *event_info __UNUSED__)
{
   Example_Item *it;
   Evas_Object *grid = data;

   it = _item_new();
   elm_gengrid_item_prepend(grid, &gic, it, _grid_sel, NULL);
}

/* append an item */
static void
_append_bt_clicked(void        *data,
                   Evas_Object *obj __UNUSED__,
                   void        *event_info __UNUSED__)
{
   Evas_Object *grid = data;
   Example_Item *it = _item_new();

   elm_gengrid_item_append(grid, &gic, it, _grid_sel, NULL);
}

/* delete items */
static void
_clear_cb(void        *data,
          Evas_Object *obj __UNUSED__,
          void        *event_info __UNUSED__)
{
   elm_gengrid_clear(data);

   fprintf(stdout, "Clearing the grid!\n");
}

/* bring in 1st item */
static void
_bring_1st_clicked(void        *data,
                   Evas_Object *obj __UNUSED__,
                   void        *event_info __UNUSED__)
{
   Elm_Gengrid_Item *it = elm_gengrid_first_item_get(data);

   if (!it) return;

   elm_gengrid_item_bring_in(it);
}

/* show last item */
static void
_show_last_clicked(void        *data,
                   Evas_Object *obj __UNUSED__,
                   void        *event_info __UNUSED__)
{
   Elm_Gengrid_Item *it = elm_gengrid_last_item_get(data);

   if (!it) return;

   elm_gengrid_item_show(it);
}

/* disable selected item */
static void
_toggle_disabled_cb(void        *data,
                    Evas_Object *obj __UNUSED__,
                    void        *event_info __UNUSED__)
{
   Elm_Gengrid_Item *it = elm_gengrid_selected_item_get(data);

   if (!it) return;

   elm_gengrid_item_selected_set(it, EINA_FALSE);
   elm_gengrid_item_disabled_set(it, EINA_TRUE);
}

/* change items' size */
static void
_size_changed(void        *data,
              Evas_Object *obj,
              void        *event_info __UNUSED__)
{
   Evas_Object *grid = data;
   int size = elm_spinner_value_get(obj);

   elm_gengrid_item_size_set(grid, size, size);
}

/* item double click callback */
static void
_double_click(void        *data __UNUSED__,
              Evas_Object *obj __UNUSED__,
              void        *event_info)
{
   fprintf(stdout, "Double click on item with handle %p\n", event_info);
}

/* item long press callback */
static void
_long_pressed(void        *data __UNUSED__,
              Evas_Object *obj __UNUSED__,
              void        *event_info)
{
   fprintf(stdout, "Long press on item with handle %p\n", event_info);
}


/* label fetching callback */
static char *
_grid_label_get(void        *data,
                Evas_Object *obj __UNUSED__,
                const char  *part __UNUSED__)
{
   const Example_Item *it = data;
   char buf[256];

   snprintf(buf, sizeof(buf), "Photo %s", it->path);
   return strdup(buf);
}

/* icon fetching callback */
static Evas_Object *
_grid_content_get(void        *data,
               Evas_Object *obj,
               const char  *part)
{
   const Example_Item *it = data;

   if (!strcmp(part, "elm.swallow.icon"))
     {
        Evas_Object *icon = elm_bg_add(obj);
        char buf[PATH_MAX];

        snprintf(buf, sizeof(buf), "%s/images/%s", PACKAGE_DATA_DIR,
                 it->path);

        elm_bg_file_set(icon, buf, NULL);
        evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1,
                                         1);
        evas_object_show(icon);
        return icon;
     }
   else if (!strcmp(part, "elm.swallow.end"))
     {
        Evas_Object *ck;
        ck = elm_check_add(obj);
        evas_object_propagate_events_set(ck, EINA_FALSE);
        evas_object_show(ck);
        return ck;
     }

   return NULL;
}

/* state fetching callback */
static Eina_Bool
_grid_state_get(void        *data __UNUSED__,
                Evas_Object *obj __UNUSED__,
                const char  *part __UNUSED__)
{
   return EINA_FALSE;
}

/* deletion callback */
static void
_grid_del(void        *data,
          Evas_Object *obj __UNUSED__)
{
   Example_Item *it = data;

   eina_stringshare_del(it->path);
   free(it);
}

/* scrolling animation stopped callback */
static void
_grid_scroll_stopped_cb(void        *data __UNUSED__,
                        Evas_Object *obj,
                        void        *event_info __UNUSED__)
{
  int h_pagenumber = 0, v_pagenumber = 0;
  elm_gengrid_current_page_get(obj, &h_pagenumber, &v_pagenumber);
  fprintf(stdout, "Grid current horiz page is %d, vert page is %d\n",
          h_pagenumber, v_pagenumber);
}

/* items grid horizontal alignment change */
static void
_h_align_change_cb(void        *data,
                   Evas_Object *obj,
                   void        *event_info __UNUSED__)
{
   double v_align;
   double val = elm_slider_value_get(obj);

   elm_gengrid_align_get(data, NULL, &v_align);

   fprintf(stdout, "Setting horizontal alignment to %f\n", val);
   elm_gengrid_align_set(data, val, v_align);
}

static void
_v_align_change_cb(void        *data,
                   Evas_Object *obj,
                   void        *event_info __UNUSED__)
{
   double h_align;
   double val = elm_slider_value_get(obj);

   elm_gengrid_align_get(data, &h_align, NULL);

   fprintf(stdout, "Setting vertical alignment to %f\n", val);
   elm_gengrid_align_set(data, h_align, val);
}

/* page relative size change */
static void
_page_change_cb(void        *data,
                Evas_Object *obj,
                void        *event_info __UNUSED__)
{
   double val = elm_slider_value_get(obj);

   elm_gengrid_page_relative_set(data, val, val);

   fprintf(stdout, "Setting grid page's relative size to %f\n", val);
}

int
elm_main(int    argc __UNUSED__,
         char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *grid, *bx, *hbx_1, *hbx_2, *hbx_3, *bt, *ck, *sl,
   *sp;
   Eina_Bool bounce;
   double h, v;

   srand(time(NULL));

   win = elm_win_add(NULL, "gengrid", ELM_WIN_BASIC);
   elm_win_title_set(win, "Generic Grid Example");
   evas_object_smart_callback_add(win, "delete,request", _on_done, NULL);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   grid = elm_gengrid_add(win);
   elm_gengrid_item_size_set(grid, 150, 150);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_min_set(grid, 600, 500);
   elm_box_pack_end(bx, grid);
   evas_object_smart_callback_add(grid, "clicked,double", _double_click, NULL);
   evas_object_smart_callback_add(grid, "longpressed", _long_pressed, NULL);
   evas_object_show(grid);

   hbx_1 = elm_box_add(win);
   evas_object_size_hint_weight_set(hbx_1, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_horizontal_set(hbx_1, EINA_TRUE);
   elm_box_pack_end(bx, hbx_1);
   evas_object_show(hbx_1);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Append");
   evas_object_smart_callback_add(bt, "clicked", _append_bt_clicked, grid);
   elm_box_pack_end(hbx_1, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Prepend");
   evas_object_smart_callback_add(bt, "clicked", _prepend_bt_clicked, grid);
   elm_box_pack_end(hbx_1, bt);
   evas_object_show(bt);

   before_bt = elm_button_add(win);
   elm_object_text_set(before_bt, "Insert before");
   evas_object_smart_callback_add(before_bt, "clicked", _before_bt_clicked,
                                  grid);
   elm_box_pack_end(hbx_1, before_bt);
   evas_object_show(before_bt);

   after_bt = elm_button_add(win);
   elm_object_text_set(after_bt, "Insert after");
   evas_object_smart_callback_add(after_bt, "clicked", _after_bt_clicked, grid);
   elm_box_pack_end(hbx_1, after_bt);
   evas_object_show(after_bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Clear");
   evas_object_smart_callback_add(bt, "clicked", _clear_cb, grid);
   elm_box_pack_end(hbx_1, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Bring in 1st");
   evas_object_smart_callback_add(bt, "clicked", _bring_1st_clicked, grid);
   elm_box_pack_end(hbx_1, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Show last");
   evas_object_smart_callback_add(bt, "clicked", _show_last_clicked, grid);
   elm_box_pack_end(hbx_1, bt);
   evas_object_show(bt);

   sp = elm_spinner_add(win);
   elm_spinner_min_max_set(sp, 10, 1024);
   elm_spinner_value_set(sp, 150);
   elm_spinner_label_format_set(sp, "Item size: %.0f");
   evas_object_smart_callback_add(sp, "changed", _size_changed, grid);
   evas_object_size_hint_weight_set(sp, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(hbx_1, sp);
   evas_object_show(sp);

   hbx_2 = elm_box_add(win);
   evas_object_size_hint_weight_set(hbx_2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_horizontal_set(hbx_2, EINA_TRUE);
   elm_box_pack_end(bx, hbx_2);
   evas_object_show(hbx_2);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Disable item");
   evas_object_smart_callback_add(bt, "clicked", _toggle_disabled_cb, grid);
   elm_box_pack_end(hbx_2, bt);
   evas_object_show(bt);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "Horizontal mode");
   evas_object_smart_callback_add(ck, "changed", _horizontal_grid, grid);
   elm_box_pack_end(hbx_2, ck);
   evas_object_show(ck);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "Always select");
   evas_object_smart_callback_add(ck, "changed", _always_select_change, grid);
   elm_box_pack_end(hbx_2, ck);
   evas_object_show(ck);

   ck = elm_check_add(win);
   elm_gengrid_bounce_get(grid, &bounce, NULL);
   elm_object_text_set(ck, "Bouncing");
   elm_check_state_set(ck, bounce);
   evas_object_smart_callback_add(ck, "changed", _bouncing_change, grid);
   elm_box_pack_end(hbx_2, ck);
   evas_object_show(ck);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "Multi-selection");
   elm_check_state_set(ck, elm_gengrid_multi_select_get(grid));
   evas_object_smart_callback_add(ck, "changed", _multi_change, grid);
   elm_box_pack_end(hbx_2, ck);
   evas_object_show(ck);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "No selection");
   evas_object_smart_callback_add(ck, "changed", _no_sel_change, grid);
   elm_box_pack_end(hbx_2, ck);
   evas_object_show(ck);

   hbx_3 = elm_box_add(win);
   evas_object_size_hint_weight_set(hbx_3, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_horizontal_set(hbx_3, EINA_TRUE);
   elm_box_pack_end(bx, hbx_3);
   evas_object_show(hbx_3);

   elm_gengrid_align_get(grid, &h, &v);
   evas_object_smart_callback_add(grid, "scroll,anim,stop", _grid_scroll_stopped_cb, NULL);

   sl = elm_slider_add(win);
   elm_object_text_set(sl, "Horiz. alignment");
   elm_slider_span_size_set(sl, 100);
   evas_object_size_hint_align_set(sl, 0.5, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(sl, 0.0, EVAS_HINT_EXPAND);
   elm_slider_indicator_format_set(sl, "%1.1f");
   elm_slider_value_set(sl, h);
   elm_box_pack_end(hbx_3, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", _h_align_change_cb, grid);

   sl = elm_slider_add(win);
   elm_object_text_set(sl, "Vert. alignment");
   elm_slider_span_size_set(sl, 100);
   evas_object_size_hint_align_set(sl, 0.5, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(sl, 0.0, EVAS_HINT_EXPAND);
   elm_slider_indicator_format_set(sl, "%1.1f");
   elm_slider_value_set(sl, v);
   elm_box_pack_end(hbx_3, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl, "changed", _v_align_change_cb, grid);

   elm_gengrid_align_get(grid, &h, &v);

   sl = elm_slider_add(win);
   elm_object_text_set(sl, "Page rel. size");
   elm_slider_span_size_set(sl, 100);
   evas_object_size_hint_align_set(sl, 0.5, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(sl, 0.0, EVAS_HINT_EXPAND);
   elm_slider_indicator_format_set(sl, "%1.1f");
   elm_slider_value_set(sl, h);
   elm_box_pack_end(hbx_3, sl);
   evas_object_show(sl);

   _page_change_cb(grid, sl, NULL);
   evas_object_smart_callback_add(sl, "changed", _page_change_cb, grid);

   gic.item_style = "default";
   gic.func.label_get = _grid_label_get;
   gic.func.content_get = _grid_content_get;
   gic.func.state_get = _grid_state_get;
   gic.func.del = _grid_del;

   _append_bt_clicked(grid, NULL, NULL);
   _append_bt_clicked(grid, NULL, NULL);
   _append_bt_clicked(grid, NULL, NULL);

   evas_object_resize(win, 600, 600);
   evas_object_show(win);

   elm_run();
   return 0;
}

ELM_MAIN()
