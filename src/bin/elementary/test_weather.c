#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

#ifdef HAVE_ELEMENTARY_EWEATHER
# include "EWeather_Smart.h"
#endif

#ifdef HAVE_ELEMENTARY_EWEATHER
static Evas_Object *en, *hv, *fl;
static Evas_Object *weather[2];
static int current = 0;
static Eina_Module *module[2];

static void
_first_city_cb(void *data EINA_UNUSED, Evas_Object *o EINA_UNUSED, void *event_info EINA_UNUSED)
{
   if (!current) return;
   current = 0;
   elm_flip_go(fl, ELM_FLIP_ROTATE_YZ_CENTER_AXIS);
}

static void
_second_city_cb(void *dat EINA_UNUSED, Evas_Object *o EINA_UNUSED, void *event_info EINA_UNUSED)
{
   if (current) return;
   current = 1;
   elm_flip_go(fl, ELM_FLIP_ROTATE_XZ_CENTER_AXIS);
}

static void _apply_cb(void *data EINA_UNUSED, Evas_Object *o EINA_UNUSED, void *event_info EINA_UNUSED)
{
   EWeather *eweather = eweather_object_eweather_get(weather[current]);

   if (module[current])
     eweather_plugin_set(eweather, module[current]);
   eweather_code_set(eweather, elm_object_text_get(en));
   printf("CURRENT %d, module[current] %p, eweather %p, city : %s\n",
          current, module[current], eweather, elm_object_text_get(en));
}

static void
_hover_select_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   EWeather *eweather = eweather_object_eweather_get(weather[0]);
   module[0] = eweather_plugin_search(eweather, elm_object_item_text_get(event_info));

   eweather = eweather_object_eweather_get(weather[1]);
   module[1] = eweather_plugin_search(eweather, elm_object_item_text_get(event_info));
   printf("%p %p\n", module[0], module[1]);
}
#endif

void
test_weather(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win;
#ifdef HAVE_ELEMENTARY_EWEATHER
   Evas_Object *bx, *bx0, *bt;
   EWeather *eweather;
   Eina_Array_Iterator it;
   Eina_Array *array;
   Eina_Module *m;
   unsigned int i;
#endif

   win = elm_win_util_standard_add("weather", "Weather");
   elm_win_autodel_set(win, EINA_TRUE);

#ifdef HAVE_ELEMENTARY_EWEATHER
   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   fl = elm_flip_add(win);
   evas_object_size_hint_align_set(fl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(fl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, fl);

   current = 0;

   module[0] = NULL;
   weather[0] = eweather_object_add(evas_object_evas_get(win));
   evas_object_size_hint_weight_set(weather[0], EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(weather[0], EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_part_content_set(fl, "front", weather[0]);
   evas_object_show(weather[0]);

   module[1] = NULL;
   weather[1] = eweather_object_add(evas_object_evas_get(win));
   eweather = eweather_object_eweather_get(weather[1]);
   evas_object_size_hint_weight_set(weather[1], EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(weather[1], EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_part_content_set(fl, "back", weather[1]);
   evas_object_show(weather[1]);

   evas_object_show(fl);

   //
   bx0 = elm_box_add(win);
   elm_box_horizontal_set(bx0, EINA_TRUE);
   evas_object_size_hint_weight_set(bx0, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx, bx0);
   evas_object_show(bx0);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "First city");
   evas_object_show(bt);
   elm_box_pack_end(bx0, bt);
   evas_object_smart_callback_add(bt, "clicked", _first_city_cb, NULL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Second city");
   evas_object_show(bt);
   elm_box_pack_end(bx0, bt);
   evas_object_smart_callback_add(bt, "clicked", _second_city_cb, NULL);
   //

   //
   bx0 = elm_box_add(win);
   elm_box_horizontal_set(bx0, EINA_TRUE);
   evas_object_size_hint_weight_set(bx0, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx, bx0);
   evas_object_show(bx0);

   hv = elm_hoversel_add(win);
   elm_hoversel_hover_parent_set(hv, win);
   elm_object_text_set(hv, "data source");
   evas_object_size_hint_weight_set(hv, 0.0, 0.0);
   evas_object_size_hint_align_set(hv, 0.5, 0.5);
   elm_box_pack_end(bx0, hv);
   evas_object_show(hv);

   array = eweather_plugins_list_get(eweather);

   EINA_ARRAY_ITER_NEXT(array, i, m, it)
     elm_hoversel_item_add(hv, eweather_plugin_name_get(eweather, i), NULL, ELM_ICON_NONE, _hover_select_cb, NULL);

   en = elm_entry_add(win);
   elm_entry_line_wrap_set(en, ELM_WRAP_NONE);
   elm_entry_single_line_set(en, EINA_TRUE);
   elm_object_text_set(en, "Paris");
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(en, "activated", _apply_cb, NULL);
   elm_box_pack_end(bx0, en);
   evas_object_show(en);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Apply");
   evas_object_show(bt);
   elm_box_pack_end(bx0, bt);
   evas_object_smart_callback_add(bt, "clicked", _apply_cb, NULL);
#else
    Evas_Object *lbl;

    lbl = elm_label_add(win);
    evas_object_size_hint_weight_set(lbl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(win, lbl);
    elm_object_text_set(lbl, "libeweather is required to display the forecast.");
    evas_object_show(lbl);
#endif

    evas_object_resize(win, 244, 388);
    evas_object_show(win);
}
