#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH
typedef struct _Testitem
{
   Elm_Gengrid_Item *item;
   const char *path;
   int mode;
   int onoff;
} Testitem;

static const char *img[9] =
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

static Elm_Gengrid_Item_Class gic, ggic;

static void
_horizontal_grid(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *grid = data;
   elm_gengrid_horizontal_set(grid, elm_check_state_get(obj));
}

static void
grid_drag_up(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   printf("Drag up: %p\n", event_info);
}

static void
grid_drag_right(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   printf("Drag right: %p\n", event_info);
}

static void
grid_drag_down(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   printf("Drag down: %p\n", event_info);
}

static void
grid_drag_left(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   printf("Drag left: %p\n", event_info);
}

static void
grid_drag_stop(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   printf("Drag stop: %p\n", event_info);
}

static void
grid_selected(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   printf("Selected: %p\n", event_info);
}

static void
grid_double_clicked(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   printf("Double clicked: %p\n", event_info);
}

static void
grid_longpress(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   printf("longpress %p\n", event_info);
}

static void
grid_moved(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   printf("moved %p\n", event_info);
}

static void
grid_item_check_changed(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Testitem *ti = data;
   ti->onoff = elm_check_state_get(obj);
   printf("item %p onoff = %i\n", ti, ti->onoff);
}

char *
grid_label_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   const Testitem *ti = data;
   char buf[256];
   snprintf(buf, sizeof(buf), "Photo %s", ti->path);
   return strdup(buf);
}

Evas_Object *
grid_icon_get(void *data, Evas_Object *obj, const char *part)
{
   const Testitem *ti = data;
   if (!strcmp(part, "elm.swallow.icon"))
     {

	Evas_Object *icon = elm_bg_add(obj);
	elm_bg_file_set(icon, ti->path, NULL);
	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	evas_object_show(icon);
	return icon;
     }
   else if (!strcmp(part, "elm.swallow.end"))
     {
	Evas_Object *ck;
	ck = elm_check_add(obj);
	evas_object_propagate_events_set(ck, 0);
	elm_check_state_set(ck, ti->onoff);
	evas_object_smart_callback_add(ck, "changed", grid_item_check_changed, data);
	evas_object_show(ck);
	return ck;
     }
   return NULL;
}

Eina_Bool
grid_state_get(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   return EINA_FALSE;
}

void
grid_del(void *data __UNUSED__, Evas_Object *obj __UNUSED__)
{
}

static void
grid_sel(void *data, Evas_Object *obj, void *event_info)
{
   printf("sel item data [%p] on grid obj [%p], pointer [%p]\n", data, obj, event_info);
}

void
test_gengrid(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *grid;
   static Testitem ti[144];
   int i, n;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "gengrid", ELM_WIN_BASIC);
   elm_win_title_set(win, "GenGrid");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   grid = elm_gengrid_add(win);
   elm_gengrid_item_size_set(grid, 150, 150);
   elm_gengrid_horizontal_set(grid, EINA_FALSE);
   elm_gengrid_multi_select_set(grid, EINA_TRUE);
   elm_gengrid_reorder_mode_set(grid, EINA_TRUE);
   evas_object_smart_callback_add(grid, "selected", grid_selected, NULL);
   evas_object_smart_callback_add(grid, "clicked,double", grid_double_clicked, NULL);
   evas_object_smart_callback_add(grid, "longpressed", grid_longpress, NULL);
   evas_object_smart_callback_add(grid, "moved", grid_moved, NULL);
   evas_object_smart_callback_add(grid, "drag,start,up", grid_drag_up, NULL);
   evas_object_smart_callback_add(grid, "drag,start,right", grid_drag_right, NULL);
   evas_object_smart_callback_add(grid, "drag,start,down", grid_drag_down, NULL);
   evas_object_smart_callback_add(grid, "drag,start,left", grid_drag_left, NULL);
   evas_object_smart_callback_add(grid, "drag,stop", grid_drag_stop, NULL);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   gic.item_style = "default";
   gic.func.label_get = grid_label_get;
   gic.func.icon_get = grid_icon_get;
   gic.func.state_get = grid_state_get;
   gic.func.del = grid_del;

   n = 0;
   for (i = 0; i < 12 * 12; i++)
     {
	snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n]);
	n = (n + 1) % 9;
	ti[i].mode = i;
	ti[i].path = eina_stringshare_add(buf);
	ti[i].item = elm_gengrid_item_append(grid, &gic, &(ti[i]), grid_sel, NULL);
	if (!(i % 5))
	  elm_gengrid_item_selected_set(ti[i].item, EINA_TRUE);
     }

   evas_object_show(grid);
   elm_win_resize_object_add(win, grid);

   evas_object_resize(win, 600, 600);
   evas_object_show(win);
}

static void
_before_bt_clicked(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Testitem *ti;
   Evas_Object *grid = data;
   Elm_Gengrid_Item *sel;
   char buf[PATH_MAX];

   sel = elm_gengrid_selected_item_get(grid);
   if (!sel)
       return;
   snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[rand() % 9]);
   ti = malloc(sizeof(*ti));
   ti->mode = 0;
   ti->path = eina_stringshare_add(buf);
   ti->item = elm_gengrid_item_insert_before(grid, &gic, ti, sel, grid_sel,
                                             NULL);
}

static void
_after_bt_clicked(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Testitem *ti;
   Evas_Object *grid = data;
   Elm_Gengrid_Item *sel;
   char buf[PATH_MAX];

   sel = elm_gengrid_selected_item_get(grid);
   if (!sel)
       return;
   snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[rand() % 9]);
   ti = malloc(sizeof(*ti));
   ti->mode = 0;
   ti->path = eina_stringshare_add(buf);
   ti->item = elm_gengrid_item_insert_after(grid, &gic, ti, sel, grid_sel,
                                            NULL);
}

static void
_delete_bt_clicked(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *grid = data;
   Eina_List *l, *l2, *l3;
   Elm_Gengrid_Item *it;

   l = (Eina_List*)elm_gengrid_selected_items_get(grid);
   if (!l) return;
   EINA_LIST_FOREACH_SAFE(l, l2, l3, it)
     elm_gengrid_item_del(it);
}

static void
_prepend_bt_clicked(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Testitem *ti;
   Evas_Object *grid = data;
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[rand() % 9]);
   ti = malloc(sizeof(*ti));
   ti->mode = 0;
   ti->path = eina_stringshare_add(buf);
   ti->item = elm_gengrid_item_prepend(grid, &gic, ti, grid_sel, NULL);
}

static void
_append_bt_clicked(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Testitem *ti;
   Evas_Object *grid = data;
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[rand() % 9]);
   ti = malloc(sizeof(*ti));
   ti->mode = 0;
   ti->path = eina_stringshare_add(buf);
   ti->item = elm_gengrid_item_append(grid, &gic, ti, grid_sel, NULL);
}

static void
_size_changed(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *grid = data;
   int size = elm_spinner_value_get(obj);
   elm_gengrid_item_size_set(grid, size, size);
}

void
test_gengrid2(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *grid, *bx, *hbx, *bt, *ck;

   win = elm_win_add(NULL, "gengrid2", ELM_WIN_BASIC);
   elm_win_title_set(win, "GenGrid2");
   elm_win_autodel_set(win, EINA_TRUE);

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
   elm_gengrid_horizontal_set(grid, EINA_FALSE);
   elm_gengrid_multi_select_set(grid, EINA_FALSE);
   evas_object_smart_callback_add(grid, "selected", grid_selected, NULL);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_min_set(grid, 600, 500);
   elm_box_pack_end(bx, grid);
   evas_object_show(grid);

   hbx = elm_box_add(win);
   evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_horizontal_set(hbx, EINA_TRUE);
   elm_box_pack_end(bx, hbx);
   evas_object_show(hbx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Append");
   evas_object_smart_callback_add(bt, "clicked", _append_bt_clicked, grid);
   elm_box_pack_end(hbx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Prepend");
   evas_object_smart_callback_add(bt, "clicked", _prepend_bt_clicked, grid);
   elm_box_pack_end(hbx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Insert before");
   evas_object_smart_callback_add(bt, "clicked", _before_bt_clicked, grid);
   elm_box_pack_end(hbx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Insert after");
   evas_object_smart_callback_add(bt, "clicked", _after_bt_clicked, grid);
   elm_box_pack_end(hbx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Delete");
   evas_object_smart_callback_add(bt, "clicked", _delete_bt_clicked, grid);
   elm_box_pack_end(hbx, bt);
   evas_object_show(bt);

   bt = elm_spinner_add(win);
   elm_spinner_min_max_set(bt, 10, 1024);
   elm_spinner_value_set(bt, 150);
   elm_spinner_label_format_set(bt, "Item size: %.0f");
   evas_object_smart_callback_add(bt, "changed", _size_changed, grid);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(hbx, bt);
   evas_object_show(bt);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "Horizontal Mode");
   evas_object_smart_callback_add(ck, "changed", _horizontal_grid, grid);
   elm_box_pack_end(hbx, ck);
   evas_object_show(ck);

   gic.item_style = "default";
   gic.func.label_get = grid_label_get;
   gic.func.icon_get = grid_icon_get;
   gic.func.state_get = grid_state_get;
   gic.func.del = grid_del;

   evas_object_resize(win, 600, 600);
   evas_object_show(win);
}

void
test_gengrid3(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *grid;
   static Testitem ti[144];
   int i, n;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "gengrid_group", ELM_WIN_BASIC);
   elm_win_title_set(win, "GenGrid Group");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   grid = elm_gengrid_add(win);
   elm_gengrid_item_size_set(grid, 150, 150);
   elm_gengrid_group_item_size_set(grid, 31, 31);
   //elm_gengrid_horizontal_set(grid, EINA_TRUE);
   elm_gengrid_horizontal_set(grid, EINA_FALSE);
   elm_gengrid_multi_select_set(grid, EINA_TRUE);
   elm_gengrid_reorder_mode_set(grid, EINA_TRUE);
   evas_object_smart_callback_add(grid, "selected", grid_selected, NULL);
   evas_object_smart_callback_add(grid, "clicked,double", grid_double_clicked, NULL);
   evas_object_smart_callback_add(grid, "longpressed", grid_longpress, NULL);
   evas_object_smart_callback_add(grid, "moved", grid_moved, NULL);
   evas_object_smart_callback_add(grid, "drag,start,up", grid_drag_up, NULL);
   evas_object_smart_callback_add(grid, "drag,start,right", grid_drag_right, NULL);
   evas_object_smart_callback_add(grid, "drag,start,down", grid_drag_down, NULL);
   evas_object_smart_callback_add(grid, "drag,start,left", grid_drag_left, NULL);
   evas_object_smart_callback_add(grid, "drag,stop", grid_drag_stop, NULL);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   gic.item_style = "default";
   gic.func.label_get = grid_label_get;
   gic.func.icon_get = grid_icon_get;
   gic.func.state_get = grid_state_get;
   gic.func.del = grid_del;

   ggic.item_style = "group_index";
   ggic.func.label_get = grid_label_get;
   ggic.func.icon_get = NULL;
   ggic.func.state_get = NULL;
   ggic.func.del = NULL;


   n = 0;
   for (i = 0; i < 12 * 12; i++)
     {
	snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), img[n]);
	n = (n + 1) % 9;
	ti[i].mode = i;
	ti[i].path = eina_stringshare_add(buf);
        if (i == 0 || i == 18 || i == 53 || i == 100)
        //if (i == 0 || i == 18)
          ti[i].item = elm_gengrid_item_append(grid, &ggic, &(ti[i]), grid_sel, NULL);
        else
          ti[i].item = elm_gengrid_item_append(grid, &gic, &(ti[i]), grid_sel, NULL);
	if (!(i % 5))
	  elm_gengrid_item_selected_set(ti[i].item, EINA_TRUE);
     }

   evas_object_show(grid);
   elm_win_resize_object_add(win, grid);

   evas_object_resize(win, 600, 600);
   evas_object_show(win);
}

#endif
