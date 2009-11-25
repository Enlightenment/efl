#include <Elementary.h>
#include "../../elementary_config.h"
#ifndef ELM_LIB_QUICKLAUNCH

#ifdef HAVE_ELEMENTARY_EWEATHER
# include "EWeather_Smart.h"
#endif

static Evas_Object *en, *hv;
static EWeather *eweather;
static Eina_Module *module;


static void _apply_cb(void *data, Evas_Object *o, void *event_info)
{
    if(module)
        eweather_plugin_set(eweather, module);

    eweather_code_set(eweather, elm_entry_entry_get(en));
}

static void _hover_select_cb(void *data, Evas_Object *obj, void *event_info)
{
    module = data;
}

void
test_weather(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *weather, *bx, *bx0, *bt;
   Eina_Array_Iterator it;
   Eina_Array *array;
   Eina_Module *m;
   int i;

   win = elm_win_add(NULL, "weather", ELM_WIN_BASIC);
   elm_win_title_set(win, "Weather");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

#ifdef HAVE_ELEMENTARY_EWEATHER 
   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   weather = eweather_object_add(evas_object_evas_get(win));
   eweather = eweather_object_eweather_get(weather);
   evas_object_size_hint_weight_set(weather, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(weather, -1.0, -1.0);
   elm_box_pack_end(bx, weather);
   evas_object_show(weather);

   bx0 = elm_box_add(win);
   elm_box_horizontal_set(bx0, EINA_TRUE);
   evas_object_size_hint_weight_set(bx0, 1.0, 0.0);
   elm_box_pack_end(bx, bx0);
   evas_object_show(bx0);

   hv = elm_hoversel_add(win);
   elm_hoversel_hover_parent_set(hv, win);
   elm_hoversel_label_set(hv, "data source");
   evas_object_size_hint_weight_set(hv, 0.0, 0.0);
   evas_object_size_hint_align_set(hv, 0.5, 0.5);
   elm_box_pack_end(bx0, hv);
   evas_object_show(hv);

   array = eweather_plugins_list_get(eweather);

   EINA_ARRAY_ITER_NEXT(array, i, m, it)
   {
       elm_hoversel_item_add(hv, eweather_plugin_name_get(eweather, i), NULL, ELM_ICON_NONE, _hover_select_cb, m);
   }

   en = elm_entry_add(win);
   elm_entry_line_wrap_set(en, 0);
   elm_entry_single_line_set(en, EINA_TRUE);
   elm_entry_entry_set(en, "Paris");
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx0, en);
   evas_object_show(en);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Apply");
   evas_object_show(bt);
   elm_box_pack_end(bx0, bt);
   evas_object_smart_callback_add(bt, "clicked", _apply_cb, NULL);


#else
    Evas_Object *lbl;
    
    lbl = elm_label_add(win);
    elm_win_resize_object_add(win, lbl);
    elm_label_label_set(lbl, "libeweather is required to display the forecast.");
    evas_object_show(lbl);
#endif

    evas_object_resize(win, 244, 388);
    evas_object_show(win);
}

#endif
