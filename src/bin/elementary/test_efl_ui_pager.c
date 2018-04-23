#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#define EO_BETA_API
#define EFL_UI_WIDGET_PROTECTED

#include <Elementary.h>


#define PAGE_NUM 10

/** -------panes--------
  * |-left-- ---right--|
  * ||     | |        ||
  * ||     | |        ||
  * ||navi | |  pager ||
  * ||frame| |        ||
  * ||     | |        ||
  * |------ -----------|
  * --------------------
  *
  */

typedef enum _Page_Type {
   LAYOUT = 1,
   GENLIST,
   BUTTON
} Page_Type;

typedef enum _Pack_Type {
   PACK_BEGIN,
   PACK_END,
   PACK_BEFORE,
   PACK_AFTER,
   PACK_AT
} Pack_Type;

typedef struct _Params {
   Evas_Object *navi;
   Eo *pager;
   Eo *transition;
   Eo *indicator;
   int w, h;
   int padding;
   int side_page_num;
   int curr_page;
   Eina_Bool prev_block, next_block;
} Params;

typedef struct _Page_Set_Params {
   Eo *pager;
   Eo *spinner;
} Page_Set_Params;

typedef struct _Pack_Params {
   Pack_Type type;
   Eo *pager;
   Eo *spinner;
} Pack_Params;

static void page_size_cb(void *data, Evas_Object *obj, void *event_info);
static void padding_cb(void *data, Evas_Object *obj, void *event_info);
static void side_page_num_cb(void *data, Evas_Object *obj, void *event_info);
static void pack_cb(void *data, Evas_Object *obj, void *event_info);
static void loop_cb(void *data, Evas_Object *obj, void *event_info);
static void current_page_cb(void *data, Evas_Object *obj, void *event_info);
static void scroll_block_cb(void *data, Evas_Object *obj, void *event_info);
static void indicator_cb(void *data, Evas_Object *obj, void *event_info);
static void transition_cb(void *data, Evas_Object *obj, void *event_info);
static void win_del_cb(void *data, const Efl_Event *ev);

static char *text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   char buf[PATH_MAX];
   int num = (int)(uintptr_t)data;

   snprintf(buf, sizeof(buf), "Item # %i", num);

   return strdup(buf);
}

Eo *page_add(Page_Type p, Eo *parent)
{
   Eo *page;
   Elm_Genlist_Item_Class *itc;
   char buf[PATH_MAX];
   int i;

   switch (p) {

      case LAYOUT:
         page = efl_add(EFL_UI_LAYOUT_CLASS, parent);
         snprintf(buf, sizeof(buf), "%s/objects/test_pager.edj", elm_app_data_dir_get());
         efl_file_set(page, buf, "page");
         efl_text_set(efl_part(page, "text"), "Layout Page");
         break;
      case GENLIST:
         page = elm_genlist_add(parent);
         itc = elm_genlist_item_class_new();
         itc->item_style = "default";
         itc->func.text_get = text_get;
         for (i = 0; i < 20; i++) {
            elm_genlist_item_append(page,
                                    itc,
                                    (void *) (uintptr_t) i,
                                    NULL,
                                    ELM_GENLIST_ITEM_NONE,
                                    NULL,
                                    NULL);
         }
         evas_object_show(page);
         break;
      case BUTTON:
         page = efl_add(EFL_UI_BUTTON_CLASS, parent);
         efl_text_set(efl_part(page, "text"), "Button Page");
         break;
      default:
         page = efl_add(EFL_UI_LAYOUT_CLASS, parent);
         snprintf(buf, sizeof(buf), "%s/objects/test_pager.edj", elm_app_data_dir_get());
         efl_file_set(page, buf, "page");
         efl_text_set(efl_part(page, "text"), "Layout Page");
         break;
   }

   evas_object_size_hint_weight_set(page, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(page, EVAS_HINT_FILL, EVAS_HINT_FILL);

   return page;
}

void test_efl_ui_pager(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *panes, *navi, *list, *pager, *page;
   Params *params = NULL;
   int i;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_BASIC),
                 efl_text_set(efl_added, "Pager"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE),
                 efl_event_callback_add(efl_added, EFL_EVENT_DEL, win_del_cb, params));

   panes = efl_add(EFL_UI_PANES_CLASS, win,
                   efl_gfx_size_hint_weight_set(efl_added, 1, 1),
                   efl_ui_panes_split_ratio_set(efl_added, 0.3),
                   efl_content_set(win, efl_added));

   navi = elm_naviframe_add(panes);
   evas_object_size_hint_weight_set(navi, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(navi, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(navi);
   efl_content_set(efl_part(panes, "first"), navi);

#define EFL_UI_LIST 0 //FIXME
#if EFL_UI_LIST
   Efl_Model_Item *model, *child;
   Efl_Ui_Layout_Factory *factory;
   char buf[256];
   Eina_Value text;
   eina_value_setup(&text, EINA_VALUE_TYPE_STRING);

   model = efl_add(EFL_MODEL_ITEM_CLASS, NULL);
   for (i = 0; i < 10; i++) {
      child = efl_model_child_add(model);
      snprintf(buf, sizeof(buf), "item %i", i);
      eina_value_set(&text, buf);
      efl_model_property_set(child, "name", &text);
   }
   eina_value_flush(&text);

   factory = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, navi);
   efl_ui_layout_factory_theme_config(factory, "list", "item", "default");

   list = efl_add(EFL_UI_LIST_CLASS, navi,
                  efl_gfx_size_hint_weight_set(efl_added, 1, 1),
                  efl_gfx_size_hint_align_set(efl_added, -1, -1),
                  efl_ui_list_select_mode_set(efl_added, ELM_OBJECT_SELECT_MODE_ALWAYS));

   efl_ui_list_layout_factory_set(list, factory);
   efl_ui_model_connect(factory, "elm.text", "name");
   efl_ui_view_model_set(list, model);

   elm_naviframe_item_push(navi, "Properties", NULL, NULL, list, NULL);
#else
   list = elm_list_add(navi);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_list_horizontal_set(list, EINA_FALSE);
   elm_list_select_mode_set(list, ELM_OBJECT_SELECT_MODE_ALWAYS);
   elm_naviframe_item_push(navi, "Properties", NULL, NULL, list, NULL);
   evas_object_show(list);
#endif

   pager = efl_add(EFL_UI_PAGER_CLASS, win);
   evas_object_size_hint_weight_set(pager, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(pager, EVAS_HINT_FILL, EVAS_HINT_FILL);
   efl_ui_pager_page_size_set(pager, EINA_SIZE2D(-1, -1));
   efl_ui_pager_padding_set(pager, 20);
   efl_content_set(efl_part(panes, "second"), pager);

   params = calloc(1, sizeof(Params));
   params->navi = navi;
   params->pager = pager;
   params->transition = NULL;
   params->indicator = NULL;
   params->w = 200;
   params->h = 300;
   params->padding = 20;
   //params->side_page_num = efl_page_transition_scroll_side_page_num_get(tran);
   params->curr_page = 0;
   params->prev_block = EINA_FALSE;
   params->next_block = EINA_FALSE;

#if EFL_UI_LIST
#else
   elm_list_item_append(list, "Page Size", NULL, NULL, page_size_cb, params);
   elm_list_item_append(list, "Padding", NULL, NULL, padding_cb, params);
   //elm_list_item_append(list, "Side Page Num", NULL, NULL, side_page_num_cb, params);
   elm_list_item_append(list, "Pack", NULL, NULL, pack_cb, params);
   elm_list_item_append(list, "Loop", NULL, NULL, loop_cb, params);
   elm_list_item_append(list, "Current Page", NULL, NULL, current_page_cb, params);
   elm_list_item_append(list, "Scroll Block", NULL, NULL, scroll_block_cb, params);
   elm_list_item_append(list, "Indicator", NULL, NULL, indicator_cb, params);
   elm_list_item_append(list, "Transition", NULL, NULL, transition_cb, params);

   elm_list_go(list);
#endif

   for (i = 0; i < PAGE_NUM; i++) {
      switch (i % 3) {
         case 0:
            page = page_add(LAYOUT, pager);
            break;
         case 1:
            page = page_add(GENLIST, pager);
            break;
         case 2:
            page = page_add(BUTTON, pager);
            break;
         default:
            page = page_add(LAYOUT, pager);
            break;
      }
      efl_pack_end(pager, page);
   }

   efl_gfx_size_set(win, EINA_SIZE2D(580, 320));
}

static void win_del_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   free(data);
}

static void btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   elm_naviframe_item_pop(data);
}

static void width_slider_cb(void *data, const Efl_Event *ev)
{
   Params *params = (Params *) data;

   params->w = efl_ui_range_value_get(ev->object);
   efl_ui_pager_page_size_set(params->pager, EINA_SIZE2D(params->w, params->h));
}

static void height_slider_cb(void *data, const Efl_Event *ev)
{
   Params *params = (Params *) data;

   params->h = efl_ui_range_value_get(ev->object);
   efl_ui_pager_page_size_set(params->pager, EINA_SIZE2D(params->w, params->h));
}

static void padding_slider_cb(void *data, const Efl_Event *ev)
{
   Params *params = (Params *) data;

   params->padding = efl_ui_range_value_get(ev->object);
   efl_ui_pager_padding_set(params->pager, params->padding);
}

static void side_page_num_slider_cb(void *data, const Efl_Event *ev)
{
   Params *params = (Params *) data;

   params->side_page_num = efl_ui_range_value_get(ev->object);
   efl_page_transition_scroll_side_page_num_set(params->transition, params->side_page_num);
}

static void pack_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Pack_Params *param = data;
   Eo *pager = param->pager;
   Eo *page, *curr_page;
   int index;

   index  = efl_content_count(pager);
   switch (index % 3) {
      case 0:
         page = page_add(LAYOUT, pager);
         break;
      case 1:
         page = page_add(GENLIST, pager);
         break;
      case 2:
         page = page_add(BUTTON, pager);
         break;
      default:
         page = page_add(LAYOUT, pager);
         break;
   }

   switch (param->type) {
      case PACK_BEGIN:
         efl_pack_begin(pager, page);
         break;
      case PACK_END:
         efl_pack_end(pager, page);
         break;
      case PACK_BEFORE:
         index = efl_ui_pager_current_page_get(pager);
         curr_page = efl_pack_content_get(pager, index);
         efl_pack_before(pager, page, curr_page);
         break;
      case PACK_AFTER:
         index = efl_ui_pager_current_page_get(pager);
         curr_page = efl_pack_content_get(pager, index);
         efl_pack_after(pager, page, curr_page);
         break;
      case PACK_AT:
         index = efl_ui_range_value_get(param->spinner);
         efl_pack_at(pager, page, index);
         break;
   }

   efl_ui_range_min_max_set(param->spinner, 0, (efl_content_count(pager) - 1));
}

static void page_set_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Page_Set_Params *psp = data;

   efl_ui_pager_current_page_set(psp->pager, efl_ui_range_value_get(psp->spinner));

}

static void check_cb(void *data, const Efl_Event *ev)
{
   Eo *pager = data;
   int state = efl_ui_nstate_value_get(ev->object);

   efl_ui_pager_loop_mode_set(pager, state);
}

static void page_set_btn_del_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   free(data);
}

static void pack_btn_del_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   free(data);
}

static void prev_block_check_cb(void *data, const Efl_Event *ev)
{
   Eo *pager = data;
   Eina_Bool prev, next;

   prev = efl_ui_nstate_value_get(ev->object);

   efl_ui_pager_scroll_block_get(pager, NULL, &next);
   efl_ui_pager_scroll_block_set(pager, prev, next);
}

static void next_block_check_cb(void *data, const Efl_Event *ev)
{
   Eo *pager = data;
   Eina_Bool prev, next;

   next = efl_ui_nstate_value_get(ev->object);

   efl_ui_pager_scroll_block_get(pager, &prev, NULL);
   efl_ui_pager_scroll_block_set(pager, prev, next);
}

static void indicator_icon_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Params *params = data;

   params->indicator = efl_add(EFL_PAGE_INDICATOR_ICON_CLASS, params->pager);
   efl_ui_pager_indicator_set(params->pager, params->indicator);
}

static void indicator_none_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Params *params = data;
   efl_ui_pager_indicator_set(params->pager, NULL);
}

static void transition_scroll_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Params *params = data;

   params->transition = efl_add(EFL_PAGE_TRANSITION_SCROLL_CLASS, params->pager);
   efl_ui_pager_transition_set(params->pager, params->transition);
}

static void transition_none_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Params *params = data;
   efl_ui_pager_transition_set(params->pager, NULL);
}

static void page_size_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Params *params = (Params *)data;
   Evas_Object *navi = params->navi;
   Eo *btn, *box;

   btn = efl_add(EFL_UI_BUTTON_CLASS, navi,
                 efl_text_set(efl_added, "Back"),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, btn_cb, navi));

   box = efl_add(EFL_UI_BOX_CLASS, navi,
                 efl_pack_padding_set(efl_added, 10, 10, EINA_TRUE),
		 elm_naviframe_item_push(navi, "Page Size", btn, NULL, efl_added, NULL));

   efl_add(EFL_UI_SLIDER_CLASS, box,
           efl_text_set(efl_added, "width"),
           efl_ui_format_string_set(efl_part(efl_added, "indicator"), "%1.0f"),
           efl_ui_range_min_max_set(efl_added, 100, 200),
           efl_ui_range_value_set(efl_added, params->w),
           efl_ui_direction_set(efl_added, EFL_UI_DIR_VERTICAL),
           efl_gfx_size_hint_weight_set(efl_added, 1, 1),
           efl_gfx_size_hint_align_set(efl_added, -1, -1),
           efl_event_callback_add(efl_added, EFL_UI_SLIDER_EVENT_CHANGED, width_slider_cb, params),
           efl_pack_end(box, efl_added));

   efl_add(EFL_UI_SLIDER_CLASS, box,
           efl_text_set(efl_added, "height"),
           efl_ui_format_string_set(efl_part(efl_added, "indicator"), "%1.0f"),
           efl_ui_range_min_max_set(efl_added, 100, 300),
           efl_ui_range_value_set(efl_added, params->h),
           efl_ui_direction_set(efl_added, EFL_UI_DIR_VERTICAL),
           efl_gfx_size_hint_weight_set(efl_added, 1, 1),
           efl_gfx_size_hint_align_set(efl_added, -1, -1),
           efl_event_callback_add(efl_added, EFL_UI_SLIDER_EVENT_CHANGED, height_slider_cb, params),
           efl_pack_end(box, efl_added));
}

static void padding_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Params *params = (Params *)data;
   Evas_Object *navi = params->navi;
   Eo *btn, *box;

   btn = efl_add(EFL_UI_BUTTON_CLASS, navi,
                 efl_text_set(efl_added, "Back"),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, btn_cb, navi));

   box = efl_add(EFL_UI_BOX_CLASS, navi,
                 efl_pack_padding_set(efl_added, 10, 10, EINA_TRUE),
		 elm_naviframe_item_push(navi, "Padding Size", btn, NULL, efl_added, NULL));

   efl_add(EFL_UI_SLIDER_CLASS, box,
           efl_text_set(efl_added, "padding"),
           efl_ui_format_string_set(efl_part(efl_added, "indicator"), "%1.0f"),
           efl_ui_range_min_max_set(efl_added, 0, 50),
           efl_ui_range_value_set(efl_added, params->padding),
           efl_ui_direction_set(efl_added, EFL_UI_DIR_VERTICAL),
           efl_gfx_size_hint_weight_set(efl_added, 1, 1),
           efl_gfx_size_hint_align_set(efl_added, -1, -1),
           efl_event_callback_add(efl_added, EFL_UI_SLIDER_EVENT_CHANGED, padding_slider_cb, params),
           efl_pack_end(box, efl_added));
}

static void side_page_num_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Params *params = (Params *)data;
   Evas_Object *navi = params->navi;
   Eo *btn, *box;

   btn = efl_add(EFL_UI_BUTTON_CLASS, navi,
                 efl_text_set(efl_added, "Back"),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, btn_cb, navi));

   box = efl_add(EFL_UI_BOX_CLASS, navi,
                 efl_pack_padding_set(efl_added, 10, 10, EINA_TRUE),
		 elm_naviframe_item_push(navi, "Side Page Num", btn, NULL, efl_added, NULL));

   efl_add(EFL_UI_SLIDER_CLASS, box,
           efl_text_set(efl_added, "side page num"),
           efl_ui_format_string_set(efl_part(efl_added, "indicator"), "%1.0f"),
           efl_ui_range_min_max_set(efl_added, 0, 3),
           efl_ui_range_value_set(efl_added, params->side_page_num),
           efl_ui_direction_set(efl_added, EFL_UI_DIR_VERTICAL),
           efl_gfx_size_hint_weight_set(efl_added, 1, 1),
           efl_gfx_size_hint_align_set(efl_added, -1, -1),
           efl_event_callback_add(efl_added, EFL_UI_SLIDER_EVENT_CHANGED, side_page_num_slider_cb, params),
           efl_pack_end(box, efl_added));
}

static void pack_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Params *params = (Params *)data;
   Evas_Object *navi = params->navi;
   Eo *pager = params->pager;
   Eo *btn, *box, *in_box, *sp;
   Pack_Params *pack_param;

   btn = efl_add(EFL_UI_BUTTON_CLASS, navi,
                 efl_text_set(efl_added, "Back"),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, btn_cb, navi));

   box = efl_add(EFL_UI_BOX_CLASS, navi,
                 efl_pack_padding_set(efl_added, 10, 10, EINA_TRUE),
                 elm_naviframe_item_push(navi, "Pack", btn, NULL, efl_added, NULL));

   in_box = efl_add(EFL_UI_BOX_CLASS, box,
                    efl_pack_padding_set(efl_added, 10, 10, EINA_TRUE),
                    efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL));

   sp = efl_add(EFL_UI_SPIN_BUTTON_CLASS, in_box,
                efl_ui_range_min_max_set(efl_added, 0, (efl_content_count(pager) - 1)),
                efl_ui_range_value_set(efl_added, efl_ui_pager_current_page_get(pager)));

   /* Pack Begin */
   pack_param = calloc(1, sizeof(Pack_Params));
   pack_param->pager = pager;
   pack_param->type = PACK_BEGIN;
   pack_param->spinner = sp;

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Pack Begin"),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, pack_btn_cb, pack_param),
           efl_event_callback_add(efl_added, EFL_EVENT_DEL, pack_btn_del_cb, pack_param),
           efl_pack_end(box, efl_added));

   /* Pack End */
   pack_param = calloc(1, sizeof(Pack_Params));
   pack_param->pager = pager;
   pack_param->type = PACK_END;
   pack_param->spinner = sp;

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Pack End"),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, pack_btn_cb, pack_param),
           efl_event_callback_add(efl_added, EFL_EVENT_DEL, pack_btn_del_cb, pack_param),
           efl_pack_end(box, efl_added));

   /* Pack Before */
   pack_param = calloc(1, sizeof(Pack_Params));
   pack_param->pager = pager;
   pack_param->type = PACK_BEFORE;
   pack_param->spinner = sp;

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Pack Before Current Page"),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, pack_btn_cb, pack_param),
           efl_event_callback_add(efl_added, EFL_EVENT_DEL, pack_btn_del_cb, pack_param),
           efl_pack_end(box, efl_added));

   /* Pack After */
   pack_param = calloc(1, sizeof(Pack_Params));
   pack_param->pager = pager;
   pack_param->type = PACK_AFTER;
   pack_param->spinner = sp;

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Pack After Current Page"),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, pack_btn_cb, pack_param),
           efl_event_callback_add(efl_added, EFL_EVENT_DEL, pack_btn_del_cb, pack_param),
           efl_pack_end(box, efl_added));

   /* Pack At */
   pack_param = calloc(1, sizeof(Pack_Params));
   pack_param->pager = pager;
   pack_param->type = PACK_AT;
   pack_param->spinner = sp;

   efl_add(EFL_UI_BUTTON_CLASS, in_box,
           efl_text_set(efl_added, "Pack At"),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, pack_btn_cb, pack_param),
           efl_event_callback_add(efl_added, EFL_EVENT_DEL, pack_btn_del_cb, pack_param),
           efl_pack_end(in_box, efl_added));

   efl_pack_end(box, in_box);
   efl_pack_end(in_box, sp);

}

static void loop_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Params *params = (Params *)data;
   Evas_Object *navi = params->navi;
   Eo *pager = params->pager;
   Eo *btn, *box;

   btn = efl_add(EFL_UI_BUTTON_CLASS, navi,
                 efl_text_set(efl_added, "Back"),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, btn_cb, navi));

   box = efl_add(EFL_UI_BOX_CLASS, navi,
                 efl_pack_padding_set(efl_added, 10, 10, EINA_TRUE),
		 elm_naviframe_item_push(navi, "Loop", btn, NULL, efl_added, NULL));

   efl_add(EFL_UI_CHECK_CLASS, box,
           efl_ui_widget_style_set(efl_added, "toggle"),
           efl_text_set(efl_added, "Loop"),
           efl_ui_nstate_value_set(efl_added, efl_ui_pager_loop_mode_get(pager)),
           efl_event_callback_add(efl_added, EFL_UI_CHECK_EVENT_CHANGED, check_cb, pager),
           efl_pack_end(box, efl_added));
}

static void current_page_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Params *params = (Params *)data;
   Evas_Object *navi = params->navi;
   Eo *pager = params->pager;
   Eo *btn, *box, *sp;
   Page_Set_Params *psp = calloc(1, sizeof(Page_Set_Params));

   btn = efl_add(EFL_UI_BUTTON_CLASS, navi,
                 efl_text_set(efl_added, "Back"),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, btn_cb, navi));

   box = efl_add(EFL_UI_BOX_CLASS, navi,
                 efl_pack_padding_set(efl_added, 10, 10, EINA_TRUE),
		 elm_naviframe_item_push(navi, "Current Page", btn, NULL, efl_added, NULL));

   btn = efl_add(EFL_UI_BUTTON_CLASS, box,
                 efl_text_set(efl_added, "Set Current Page As"),
                 efl_pack_end(box, efl_added));

   sp = efl_add(EFL_UI_SPIN_BUTTON_CLASS, box,
                efl_ui_range_min_max_set(efl_added, 0, (efl_content_count(pager) - 1)),
                efl_ui_range_value_set(efl_added, efl_ui_pager_current_page_get(pager)),
                efl_gfx_size_hint_align_set(efl_added, -1, -1),
                efl_pack_end(box, efl_added));

   psp->pager = pager;
   psp->spinner = sp;

   efl_event_callback_add(btn, EFL_UI_EVENT_CLICKED, page_set_btn_cb, psp);
   efl_event_callback_add(btn, EFL_EVENT_DEL, page_set_btn_del_cb, psp);
}

static void scroll_block_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Params *params = (Params *)data;
   Evas_Object *navi = params->navi;
   Eo *pager = params->pager;
   Eo *btn, *box;
   Eina_Bool prev, next;

   btn = efl_add(EFL_UI_BUTTON_CLASS, navi,
                 efl_text_set(efl_added, "Back"),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, btn_cb, navi));

   box = efl_add(EFL_UI_BOX_CLASS, navi,
                 efl_pack_padding_set(efl_added, 10, 10, EINA_TRUE),
		 elm_naviframe_item_push(navi, "Scroll Block", btn, NULL, efl_added, NULL));

   efl_ui_pager_scroll_block_get(pager, &prev, &next);

   efl_add(EFL_UI_CHECK_CLASS, box,
           efl_ui_widget_style_set(efl_added, "toggle"),
           efl_text_set(efl_added, "Prev Block"),
           efl_ui_nstate_value_set(efl_added, prev),
           efl_event_callback_add(efl_added, EFL_UI_CHECK_EVENT_CHANGED, prev_block_check_cb, pager),
           efl_pack_end(box, efl_added));

   efl_add(EFL_UI_CHECK_CLASS, box,
           efl_ui_widget_style_set(efl_added, "toggle"),
           efl_text_set(efl_added, "Next Block"),
           efl_ui_nstate_value_set(efl_added, next),
           efl_event_callback_add(efl_added, EFL_UI_CHECK_EVENT_CHANGED, next_block_check_cb, pager),
           efl_pack_end(box, efl_added));
}

static void indicator_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Params *params = (Params *)data;
   Evas_Object *navi = params->navi;
   Eo *btn, *box;

   btn = efl_add(EFL_UI_BUTTON_CLASS, navi,
                 efl_text_set(efl_added, "Back"),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, btn_cb, navi));

   box = efl_add(EFL_UI_BOX_CLASS, navi,
                 efl_pack_padding_set(efl_added, 10, 10, EINA_TRUE),
                 elm_naviframe_item_push(navi, "Indicator", btn, NULL, efl_added, NULL));

   btn = efl_add(EFL_UI_BUTTON_CLASS, box,
                 efl_text_set(efl_added, "Icon Type"),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, indicator_icon_btn_cb, params),
                 efl_pack_end(box, efl_added));

   btn = efl_add(EFL_UI_BUTTON_CLASS, box,
                 efl_text_set(efl_added, "None"),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, indicator_none_btn_cb, params),
                 efl_pack_end(box, efl_added));
}

static void transition_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Params *params = (Params *)data;
   Evas_Object *navi = params->navi;
   Eo *btn, *box;

   btn = efl_add(EFL_UI_BUTTON_CLASS, navi,
                 efl_text_set(efl_added, "Back"),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, btn_cb, navi));

   box = efl_add(EFL_UI_BOX_CLASS, navi,
                 efl_pack_padding_set(efl_added, 10, 10, EINA_TRUE),
                 elm_naviframe_item_push(navi, "Transition", btn, NULL, efl_added, NULL));

   btn = efl_add(EFL_UI_BUTTON_CLASS, box,
                 efl_text_set(efl_added, "Scroll"),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, transition_scroll_btn_cb, params),
                 efl_pack_end(box, efl_added));

   btn = efl_add(EFL_UI_BUTTON_CLASS, box,
                 efl_text_set(efl_added, "None"),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, transition_none_btn_cb, params),
                 efl_pack_end(box, efl_added));

}
