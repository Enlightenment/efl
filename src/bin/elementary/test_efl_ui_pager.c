#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#define EO_BETA_API
#include <Elementary.h>


#define PAGE_NUM 10

/** --------box--------
  * |--box-- ----box---|
  * ||     | |        ||
  * ||     | |        ||
  * ||navi | |  pager ||
  * ||frame| |        ||
  * ||     | |        ||
  * |------ -----------|
  * --------------------
  *
  */

typedef struct _Params {
   Evas_Object *navi;
   Evas_Object *pager;
   int w, h;
   int padding;
   int side_page_num;
} Params;

static void page_size_cb(void *data, Evas_Object *obj, void *event_info);
static void padding_cb(void *data, Evas_Object *obj, void *event_info);
static void side_page_num_cb(void *data, Evas_Object *obj, void *event_info);
static void transition_cb(void *data, Evas_Object *obj, void *event_info);
static void current_page_cb(void *data, Evas_Object *obj, void *event_info);
static void scroll_block_cb(void *data, Evas_Object *obj, void *event_info);
static void loop_cb(void *data, Evas_Object *obj, void *event_info);

void
test_efl_ui_pager(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *box2, *box3, *navi, *list, *pager, *page;
   Efl_Page_Transition *tran;
   int i;
   char buf[64];

   win = elm_win_util_standard_add("pager", "Pager");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   elm_box_horizontal_set(box, EINA_TRUE);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   box2 = elm_box_add(box);
   evas_object_size_hint_weight_set(box2, 0.3, 0.0);
   evas_object_size_hint_align_set(box2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(box2);
   elm_box_pack_end(box, box2);

   box3 = elm_box_add(box);
   elm_box_horizontal_set(box3, EINA_TRUE);
   evas_object_size_hint_weight_set(box3, 0.7, 0.0);
   evas_object_size_hint_align_set(box3, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(box3);
   elm_box_pack_end(box, box3);

   navi = elm_naviframe_add(box2);
   evas_object_size_hint_weight_set(navi, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(navi, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box2, navi);
   evas_object_show(navi);

   list = elm_list_add(navi);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_list_horizontal_set(list, EINA_FALSE);
   elm_list_select_mode_set(list, ELM_OBJECT_SELECT_MODE_ALWAYS);
   elm_naviframe_item_push(navi, "Properties", NULL, NULL, list, NULL);
   evas_object_show(list);

   pager = efl_add(EFL_UI_PAGER_CLASS, win);
   evas_object_size_hint_weight_set(pager, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(pager, EVAS_HINT_FILL, EVAS_HINT_FILL);
   tran = efl_add(EFL_PAGE_TRANSITION_SCROLL_CLASS, NULL);
   efl_ui_pager_transition_set(pager, tran);
   elm_box_pack_end(box3, pager);
   efl_ui_pager_page_size_set(pager, 200, 300);
   efl_ui_pager_padding_set(pager, 20);

   Params *params = (Params *)malloc(sizeof(Params));
   params->navi = navi;
   params->pager = pager;
   params->w = 200;
   params->h = 300;
   params->padding = 20;
   params->side_page_num = efl_ui_pager_side_page_num_get(pager);

   elm_list_item_append(list, "Page Size", NULL, NULL, page_size_cb, params);
   elm_list_item_append(list, "Padding", NULL, NULL, padding_cb, params);
   elm_list_item_append(list, "Side Page Num", NULL, NULL, side_page_num_cb, params);
   elm_list_item_append(list, "Transition", NULL, NULL, transition_cb, params);
   elm_list_item_append(list, "Current Page", NULL, NULL, current_page_cb, params);
   elm_list_item_append(list, "Scroll Block", NULL, NULL, scroll_block_cb, params);
   elm_list_item_append(list, "Loop", NULL, NULL, loop_cb, params);
   elm_list_go(list);

   for (i = 1; i <= PAGE_NUM; i++) {
      page = efl_add(EFL_UI_LAYOUT_CLASS, pager);
      snprintf(buf, sizeof(buf), "%s/objects/test_pager.edj", elm_app_data_dir_get());
      efl_file_set(page, buf, "page");
      snprintf(buf, 16, "Page %d", i);
      efl_text_set(efl_part(page, "text"), buf);
      efl_pack_end(pager, page);
   }

   evas_object_resize(win, 580, 320);
   evas_object_show(win);
}

static void btn_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_naviframe_item_pop(data);
}

static void width_slider_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Params *params = (Params *) data;

   params->w = (int) elm_slider_value_get(obj);
   efl_ui_pager_page_size_set(params->pager, params->w, params->h);
}

static void height_slider_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Params *params = (Params *) data;

   params->h = (int) elm_slider_value_get(obj);
   efl_ui_pager_page_size_set(params->pager, params->w, params->h);
}

static void padding_slider_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Params *params = (Params *) data;

   params->padding = (int) elm_slider_value_get(obj);
   efl_ui_pager_padding_set(params->pager, params->padding);
}

static void side_page_num_slider_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Params *params = (Params *) data;

   params->side_page_num = (int) elm_slider_value_get(obj);
   efl_ui_pager_side_page_num_set(params->pager, params->side_page_num);
}

static void page_size_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Params *params = (Params *)data;
   Evas_Object *navi = params->navi;
   Evas_Object *btn, *box, *slider;

   btn = elm_button_add(navi);
   elm_object_text_set(btn, "Back");
   evas_object_smart_callback_add(btn, "clicked", btn_cb, navi);

   box = elm_box_add(navi);
   elm_box_padding_set(box, 10, 10);
   evas_object_show(box);
   elm_naviframe_item_push(navi, "Page Size", btn, NULL, box, NULL);

   slider = elm_slider_add(box);
   elm_slider_indicator_format_set(slider, "%1.0f");
   elm_slider_min_max_set(slider, 100, 200);
   elm_object_text_set(slider, "width");
   elm_slider_horizontal_set(slider, EINA_FALSE);
   elm_slider_value_set(slider, params->w);
   evas_object_size_hint_weight_set(slider, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(slider, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(slider, "changed", width_slider_cb, params);
   elm_box_pack_end(box, slider);
   evas_object_show(slider);

   slider = elm_slider_add(box);
   elm_slider_indicator_format_set(slider, "%1.0f");
   elm_slider_min_max_set(slider, 100, 300);
   elm_object_text_set(slider, "height");
   elm_slider_horizontal_set(slider, EINA_FALSE);
   elm_slider_value_set(slider, params->h);
   evas_object_size_hint_weight_set(slider, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(slider, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(slider, "changed", height_slider_cb, params);
   elm_box_pack_end(box, slider);
   evas_object_show(slider);
}

static void padding_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Params *params = (Params *)data;
   Evas_Object *navi = params->navi;
   Evas_Object *btn, *box, *slider;

   btn = elm_button_add(navi);
   elm_object_text_set(btn, "Back");
   evas_object_smart_callback_add(btn, "clicked", btn_cb, navi);

   box = elm_box_add(navi);
   elm_box_padding_set(box, 10, 10);
   evas_object_show(box);
   elm_naviframe_item_push(navi, "Page Size", btn, NULL, box, NULL);

   slider = elm_slider_add(box);
   elm_slider_indicator_format_set(slider, "%1.0f");
   elm_slider_min_max_set(slider, 0, 50);
   elm_object_text_set(slider, "padding");
   elm_slider_horizontal_set(slider, EINA_FALSE);
   elm_slider_value_set(slider, params->padding);
   evas_object_size_hint_weight_set(slider, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(slider, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(slider, "changed", padding_slider_cb, params);
   elm_box_pack_end(box, slider);
   evas_object_show(slider);
}

static void side_page_num_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Params *params = (Params *)data;
   Evas_Object *navi = params->navi;
   Evas_Object *btn, *box, *slider;

   btn = elm_button_add(navi);
   elm_object_text_set(btn, "Back");
   evas_object_smart_callback_add(btn, "clicked", btn_cb, navi);

   box = elm_box_add(navi);
   elm_box_padding_set(box, 10, 10);
   evas_object_show(box);
   elm_naviframe_item_push(navi, "Side Page", btn, NULL, box, NULL);

   slider = elm_slider_add(box);
   elm_slider_indicator_format_set(slider, "%1.0f");
   elm_slider_min_max_set(slider, 0, 3);
   elm_object_text_set(slider, "side page num");
   elm_slider_horizontal_set(slider, EINA_FALSE);
   elm_slider_value_set(slider, params->side_page_num);
   evas_object_size_hint_weight_set(slider, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(slider, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(slider, "changed", side_page_num_slider_cb, params);
   elm_box_pack_end(box, slider);
   evas_object_show(slider);

}

static void transition_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{

}

static void current_page_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{

}

static void scroll_block_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{

}

static void loop_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{

}

