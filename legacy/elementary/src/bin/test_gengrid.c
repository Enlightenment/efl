#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH
typedef struct _Testitem
{
   Elm_Gengrid_Item *item;
   const char *path;
   int mode;
   int onoff;
} Testitem;

static Elm_Gengrid_Item_Class gic;

static void
grid_drag_up(void *data, Evas_Object *obj, void *event_info)
{
   printf("Drag up: %p\n", event_info);
}

static void
grid_drag_right(void *data, Evas_Object *obj, void *event_info)
{
   printf("Drag right: %p\n", event_info);
}

static void
grid_drag_down(void *data, Evas_Object *obj, void *event_info)
{
   printf("Drag down: %p\n", event_info);
}

static void
grid_drag_left(void *data, Evas_Object *obj, void *event_info)
{
   printf("Drag left: %p\n", event_info);
}

static void
grid_drag_stop(void *data, Evas_Object *obj, void *event_info)
{
   printf("Drag stop: %p\n", event_info);
}

static void
grid_selected(void *data, Evas_Object *obj, void *event_info)
{
   printf("Selected: %p\n", event_info);
}

static void
grid_clicked(void *data, Evas_Object *obj, void *event_info)
{
   printf("Clicked: %p\n", event_info);
}

static void
grid_longpress(void *data, Evas_Object *obj, void *event_info)
{
   printf("longpress %p\n", event_info);
}

static void
grid_item_check_changed(void *data, Evas_Object *obj, void *event_info)
{
   Testitem *ti = data;
   ti->onoff = elm_check_state_get(obj);
   printf("item %p onoff = %i\n", ti, ti->onoff);
}

char *
grid_label_get(const void *data, Evas_Object *obj, const char *part)
{
   const Testitem *ti = data;
   char buf[256];
   snprintf(buf, sizeof(buf), "Photo %s", ti->path);
   return strdup(buf);
}

Evas_Object *
grid_icon_get(const void *data, Evas_Object *obj, const char *part)
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
grid_state_get(const void *data, Evas_Object *obj, const char *part)
{
   return EINA_FALSE;
}

void
grid_del(const void *data, Evas_Object *obj)
{
}

static void
grid_sel(void *data, Evas_Object *obj, void *event_info)
{
   printf("sel item data [%p] on grid obj [%p], pointer [%p]\n", data, obj, event_info);
}

void
test_gengrid(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *grid;
   static Testitem ti[144];
   int i, j, n;
   char buf[PATH_MAX];
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
	"wood_01.jpg",
     };

   win = elm_win_add(NULL, "grid", ELM_WIN_BASIC);
   elm_win_title_set(win, "Grid");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   grid = elm_gengrid_add(win);
   elm_gengrid_item_size_set(grid, 150, 150);
   elm_gengrid_horizontal_set(grid, EINA_FALSE);
   elm_gengrid_multi_select_set(grid, EINA_TRUE);
   evas_object_smart_callback_add(grid, "selected", grid_selected, NULL);
   evas_object_smart_callback_add(grid, "clicked", grid_clicked, NULL);
   evas_object_smart_callback_add(grid, "longpressed", grid_longpress, NULL);
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
	snprintf(buf, sizeof(buf), "%s/images/%s", PACKAGE_DATA_DIR, img[n]);
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
#endif
