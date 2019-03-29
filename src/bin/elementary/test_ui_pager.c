#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#define EO_BETA_API
#define EFL_UI_WIDGET_PROTECTED

#include <Efl_Ui.h>
#include <Elementary.h>


#define PAGE_NUM 3


/** |----------panes----------|
  * ||--left---| |---right---||
  * ||         | |  layout   ||
  * ||         | ||---------|||
  * ||naviframe| ||  pager  |||
  * ||         | ||         |||
  * ||         | ||---------|||
  * ||---------| |-----------||
  * |-------------------------|
  *
  */


typedef enum _Page_Type {
   LAYOUT,
   LIST,
   BUTTON
} Page_Type;

typedef enum _Pack_Type {
   PACK_BEGIN,
   PACK_END,
   PACK_BEFORE,
   PACK_AFTER,
   PACK_AT,
   UNPACK_AT,
   CLEAR
} Pack_Type;

typedef struct _Params {
   Evas_Object *navi;
   Eo *pager;
   Eo *indicator;
   int w, h;
   Eina_Bool wfill, hfill;
} Params;

typedef struct _Page_Set_Params {
   Eo *pager;
   Eo *spinner;
} Page_Set_Params;

typedef struct _Pack_Params {
   Pack_Type type;
   Eo *pager;
   Eo *pack_sp;
   Eo *unpack_sp;
   Eo *unpack_btn;
} Pack_Params;

typedef struct _Size_Params {
   Eo *pager;
   Eo *slider;
   Params *params;
} Size_Params;



static Eo *page_add(Page_Type p, Eo *parent)
{
   Eo *page;
   char buf[PATH_MAX];
   int i;

   switch (p) {
      case LAYOUT:
         snprintf(buf, sizeof(buf), "%s/objects/test_pager.edj",
                  elm_app_data_dir_get());
         page = efl_add(EFL_UI_LAYOUT_CLASS, parent,
                        efl_file_set(efl_added, buf),
                        efl_file_key_set(efl_added, "page"),
                        efl_file_load(efl_added),
                        efl_text_set(efl_part(efl_added, "text"), "Layout Page"));
         efl_gfx_hint_fill_set(page, EINA_TRUE, EINA_TRUE);
         break;
      case LIST:
         page = elm_list_add(parent);
         elm_list_select_mode_set(page, ELM_OBJECT_SELECT_MODE_ALWAYS);
         evas_object_show(page);
         for (i = 0; i < 20; i++) {
            snprintf(buf, sizeof(buf), "List Page - Item #%d", i);
            elm_list_item_append(page, buf, NULL, NULL, NULL, NULL);
         }
         evas_object_size_hint_weight_set(page, EVAS_HINT_EXPAND,
                                          EVAS_HINT_EXPAND);
         evas_object_size_hint_align_set(page, EVAS_HINT_FILL,
                                         EVAS_HINT_FILL);
         break;
      case BUTTON:
         page = efl_add(EFL_UI_BUTTON_CLASS, parent,
                        efl_text_set(efl_added, "Button Page"));
         efl_gfx_hint_fill_set(page, EINA_TRUE, EINA_TRUE);
         break;
      default:
         snprintf(buf, sizeof(buf), "%s/objects/test_pager.edj",
                  elm_app_data_dir_get());
         page = efl_add(EFL_UI_LAYOUT_CLASS, parent,
                        efl_file_set(efl_added, buf),
                        efl_file_key_set(efl_added, "page"),
                        efl_file_load(efl_added),
                        efl_text_set(efl_part(efl_added, "text"), "Layout Page"));
         efl_gfx_hint_fill_set(page, EINA_TRUE, EINA_TRUE);
         break;
   }

   return page;
}

static void prev_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *pager = data;
   int curr_page = efl_ui_pager_current_page_get(pager);

   efl_ui_pager_current_page_set(pager, (curr_page - 1));
}

static void next_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *pager = data;
   int curr_page = efl_ui_pager_current_page_get(pager);

   efl_ui_pager_current_page_set(pager, (curr_page + 1));
}

static void back_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   elm_naviframe_item_pop(data);
}

static void list_del_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   free(data);
}

static void width_slider_cb(void *data, const Efl_Event *ev)
{
   Params *params = data;
   int h;

   if (params->hfill) h = -1;
   else h = params->h;

   params->w = efl_ui_range_value_get(ev->object);
   efl_ui_pager_page_size_set(params->pager, EINA_SIZE2D(params->w, h));
}

static void height_slider_cb(void *data, const Efl_Event *ev)
{
   Params *params = data;
   int w;

   if (params->wfill) w = -1;
   else w = params->w;

   params->h = efl_ui_range_value_get(ev->object);
   efl_ui_pager_page_size_set(params->pager, EINA_SIZE2D(w, params->h));
}

static void width_check_cb(void *data, const Efl_Event *ev)
{
   Size_Params *params = data;
   Eina_Bool ck = elm_check_selected_get(ev->object);
   int w, h;

   elm_object_disabled_set(params->slider, ck);

   params->params->wfill = ck;

   if (params->params->wfill) w = -1;
   else w = params->params->w;

   if (params->params->hfill) h = -1;
   else h = params->params->h;

   efl_ui_pager_page_size_set(params->pager, EINA_SIZE2D(w, h));
}

static void height_check_cb(void *data, const Efl_Event *ev)
{
   Size_Params *params = data;
   Eina_Bool ck = elm_check_selected_get(ev->object);
   int w, h;

   elm_object_disabled_set(params->slider, ck);

   params->params->hfill = ck;

   if (params->params->wfill) w = -1;
   else w = params->params->w;

   if (params->params->hfill) h = -1;
   else h = params->params->h;

   efl_ui_pager_page_size_set(params->pager, EINA_SIZE2D(w, h));
}

static void check_del_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   free(data);
}

static void pack_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Pack_Params *param = data;
   Eo *pager = param->pager;
   Eo *page, *curr_page;
   int index, cnt;

   if ((param->type != UNPACK_AT) && (param->type != CLEAR)) {
      index  = efl_content_count(pager);

      switch (index % 3) {
         case 0:
            page = page_add(LAYOUT, pager);
            break;
         case 1:
            page = page_add(LIST, pager);
            break;
         case 2:
            page = page_add(BUTTON, pager);
            break;
         default:
            page = page_add(LAYOUT, pager);
            break;
      }
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
         index = efl_ui_range_value_get(param->pack_sp);
         efl_pack_at(pager, page, index);
         break;
      case UNPACK_AT:
         index = efl_ui_range_value_get(param->unpack_sp);
         page = efl_pack_unpack_at(pager, index);
         efl_del(page);
         break;
      case CLEAR:
         efl_pack_clear(pager);
         break;
   }

   cnt = efl_content_count(pager);

   index = efl_ui_range_value_get(param->pack_sp);
   if (index > cnt)
     efl_ui_range_value_set(param->pack_sp, cnt);
   efl_ui_range_min_max_set(param->pack_sp, 0, cnt);

   if (cnt > 0)
     {
        elm_object_disabled_set(param->unpack_btn, EINA_FALSE);
        elm_object_disabled_set(param->unpack_sp, EINA_FALSE);

        cnt -= 1;
        index = efl_ui_range_value_get(param->unpack_sp);
        if (index > cnt)
          efl_ui_range_value_set(param->unpack_sp, cnt);
        efl_ui_range_min_max_set(param->unpack_sp, 0, cnt);
     }
   else
     {
        elm_object_disabled_set(param->unpack_btn, EINA_TRUE);
        elm_object_disabled_set(param->unpack_sp, EINA_TRUE);
     }
}

static void pack_btn_del_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   free(data);
}

static void page_set_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Page_Set_Params *psp = data;

   efl_ui_pager_current_page_set(psp->pager,
                                 efl_ui_range_value_get(psp->spinner));
}

static void page_set_btn_del_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   free(data);
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

static void page_size_cb(void *data,
                         Evas_Object *obj EINA_UNUSED,
                         void *event_info EINA_UNUSED)
{
   Params *params = data;
   Size_Params *size_params;
   Evas_Object *navi = params->navi;
   Eo *btn, *box, *fr, *inbox, *ck, *sl;

   btn = efl_add(EFL_UI_BUTTON_CLASS, navi,
                 efl_text_set(efl_added, "Back"),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED,
                                        back_btn_cb, navi));

   box = efl_add(EFL_UI_BOX_CLASS, navi,
                 elm_naviframe_item_push(navi, "Page Size", btn, NULL,
                                         efl_added, NULL));

   // Width
   fr = elm_frame_add(box);
   elm_object_text_set(fr, "Width");
   efl_gfx_hint_align_set(fr, -1, -1);
   efl_gfx_hint_weight_set(fr, 1, 1);
   efl_pack(box, fr);
   efl_gfx_entity_visible_set(fr, 1);

   inbox = efl_add(EFL_UI_BOX_CLASS, fr,
                  efl_content_set(fr, efl_added));

   ck = elm_check_add(inbox);
   elm_object_text_set(ck, "Fill");
   efl_pack_end(inbox, ck);
   efl_gfx_entity_visible_set(ck, 1);

   sl = efl_add(EFL_UI_SLIDER_CLASS, inbox,
                efl_ui_range_min_max_set(efl_added, 100, 200),
                efl_ui_range_value_set(efl_added, params->w),
                efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(100, 0)),
                efl_event_callback_add(efl_added, EFL_UI_SLIDER_EVENT_CHANGED,
                                       width_slider_cb, params),
                efl_pack_end(inbox, efl_added));

   size_params = calloc(1, sizeof(Size_Params));
   if (!size_params) return;

   size_params->slider = sl;
   size_params->pager = params->pager;
   size_params->params = params;

   efl_event_callback_add(ck, EFL_UI_CHECK_EVENT_CHANGED, width_check_cb,
                          size_params);
   efl_event_callback_add(ck, EFL_EVENT_DEL, check_del_cb, size_params);

   if (params->wfill)
     {
        elm_check_state_set(ck, EINA_TRUE);
        elm_object_disabled_set(sl, EINA_TRUE);
     }

   // Height
   fr = elm_frame_add(box);
   elm_object_text_set(fr, "Height");
   efl_gfx_hint_align_set(fr, -1, -1);
   efl_gfx_hint_weight_set(fr, 1, 1);
   efl_pack(box, fr);
   efl_gfx_entity_visible_set(fr, 1);

   inbox = efl_add(EFL_UI_BOX_CLASS, fr,
                  efl_content_set(fr, efl_added));

   ck = elm_check_add(inbox);
   elm_object_text_set(ck, "Fill");
   efl_pack_end(inbox, ck);
   efl_gfx_entity_visible_set(ck, 1);

   sl = efl_add(EFL_UI_SLIDER_CLASS, inbox,
                efl_ui_range_min_max_set(efl_added, 100, 300),
                efl_ui_range_value_set(efl_added, params->h),
                efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(100, 0)),
                efl_event_callback_add(efl_added, EFL_UI_SLIDER_EVENT_CHANGED,
                                       height_slider_cb, params),
                efl_pack_end(inbox, efl_added));

   size_params = calloc(1, sizeof(Size_Params));
   if (!size_params) return;

   size_params->slider = sl;
   size_params->pager = params->pager;
   size_params->params = params;

   efl_event_callback_add(ck, EFL_UI_CHECK_EVENT_CHANGED, height_check_cb,
                          size_params);
   efl_event_callback_add(ck, EFL_EVENT_DEL, check_del_cb, size_params);

   if (params->hfill)
     {
        elm_check_state_set(ck, EINA_TRUE);
        elm_object_disabled_set(sl, EINA_TRUE);
     }
}

static void pack_cb(void *data,
                    Evas_Object *obj EINA_UNUSED,
                    void *event_info EINA_UNUSED)
{
   Params *params = (Params *)data;
   Evas_Object *navi = params->navi;
   Eo *pager = params->pager;
   Eo *btn, *box, *in_box1, *in_box2, *sp1, *sp2;
   Pack_Params *pack_param;

   btn = efl_add(EFL_UI_BUTTON_CLASS, navi,
                 efl_text_set(efl_added, "Back"),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED,
                                        back_btn_cb, navi));

   box = efl_add(EFL_UI_BOX_CLASS, navi,
                 efl_pack_padding_set(efl_added, 10, 10, EINA_TRUE),
                 elm_naviframe_item_push(navi, "Pack", btn, NULL,
                                         efl_added, NULL));

   in_box1 = efl_add(EFL_UI_BOX_CLASS, box,
                     efl_pack_padding_set(efl_added, 10, 10, EINA_TRUE),
                     efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL));

   sp1 = efl_add(EFL_UI_SPIN_BUTTON_CLASS, in_box1,
                 efl_ui_range_min_max_set(efl_added, 0,
                                          efl_content_count(pager)),
                 efl_ui_range_value_set(efl_added,
                                        efl_ui_pager_current_page_get(pager)));

   in_box2 = efl_add(EFL_UI_BOX_CLASS, box,
                     efl_pack_padding_set(efl_added, 10, 10, EINA_TRUE),
                     efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL));

   sp2 = efl_add(EFL_UI_SPIN_BUTTON_CLASS, in_box2);

   btn = efl_add(EFL_UI_BUTTON_CLASS, in_box2,
                 efl_text_set(efl_added, "Unpack At"));

   // Pack Begin
   pack_param = calloc(1, sizeof(Pack_Params));
   if (!pack_param) return;

   pack_param->pager = pager;
   pack_param->pack_sp = sp1;
   pack_param->unpack_sp = sp2;
   pack_param->unpack_btn = btn;
   pack_param->type = PACK_BEGIN;

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Pack Begin"),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED,
                                  pack_btn_cb, pack_param),
           efl_event_callback_add(efl_added, EFL_EVENT_DEL,
                                  pack_btn_del_cb, pack_param),
           efl_pack_end(box, efl_added));

   // Pack End
   pack_param = calloc(1, sizeof(Pack_Params));
   if (!pack_param) return;

   pack_param->pager = pager;
   pack_param->pack_sp = sp1;
   pack_param->unpack_sp = sp2;
   pack_param->unpack_btn = btn;
   pack_param->type = PACK_END;

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Pack End"),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED,
                                  pack_btn_cb, pack_param),
           efl_event_callback_add(efl_added, EFL_EVENT_DEL,
                                  pack_btn_del_cb, pack_param),
           efl_pack_end(box, efl_added));

   // Pack Before
   pack_param = calloc(1, sizeof(Pack_Params));
   if (!pack_param) return;

   pack_param->pager = pager;
   pack_param->pack_sp = sp1;
   pack_param->unpack_sp = sp2;
   pack_param->unpack_btn = btn;
   pack_param->type = PACK_BEFORE;

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Pack Before Current Page"),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED,
                                  pack_btn_cb, pack_param),
           efl_event_callback_add(efl_added, EFL_EVENT_DEL,
                                  pack_btn_del_cb, pack_param),
           efl_pack_end(box, efl_added));

   // Pack After
   pack_param = calloc(1, sizeof(Pack_Params));
   if (!pack_param) return;

   pack_param->pager = pager;
   pack_param->pack_sp = sp1;
   pack_param->unpack_sp = sp2;
   pack_param->unpack_btn = btn;
   pack_param->type = PACK_AFTER;

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Pack After Current Page"),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED,
                                  pack_btn_cb, pack_param),
           efl_event_callback_add(efl_added, EFL_EVENT_DEL,
                                  pack_btn_del_cb, pack_param),
           efl_pack_end(box, efl_added));

   // Pack At
   pack_param = calloc(1, sizeof(Pack_Params));
   if (!pack_param) return;

   pack_param->pager = pager;
   pack_param->pack_sp = sp1;
   pack_param->unpack_sp = sp2;
   pack_param->unpack_btn = btn;
   pack_param->type = PACK_AT;

   efl_add(EFL_UI_BUTTON_CLASS, in_box1,
           efl_text_set(efl_added, "Pack At"),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED,
                                  pack_btn_cb, pack_param),
           efl_event_callback_add(efl_added, EFL_EVENT_DEL,
                                  pack_btn_del_cb, pack_param),
           efl_pack_end(in_box1, efl_added));

   efl_pack_end(box, in_box1);
   efl_pack_end(in_box1, sp1);

   // Unpack At
   pack_param = calloc(1, sizeof(Pack_Params));
   if (!pack_param) return;

   pack_param->pager = pager;
   pack_param->pack_sp = sp1;
   pack_param->unpack_sp = sp2;
   pack_param->unpack_btn = btn;
   pack_param->type = UNPACK_AT;

   efl_event_callback_add(btn, EFL_UI_EVENT_CLICKED,
                          pack_btn_cb, pack_param);
   efl_event_callback_add(btn, EFL_EVENT_DEL,
                          pack_btn_del_cb, pack_param);

   if (efl_content_count(pager) > 0)
     {
        efl_ui_range_min_max_set(sp2, 0,
                                 (efl_content_count(pager) - 1));
        efl_ui_range_value_set(sp2,
                               efl_ui_pager_current_page_get(pager));
     }
   else
     {
        elm_object_disabled_set(btn, EINA_TRUE);
        elm_object_disabled_set(sp2, EINA_TRUE);
     }

   efl_pack_end(box, in_box2);
   efl_pack_end(in_box2, btn);
   efl_pack_end(in_box2, sp2);

   // Clear
   pack_param = calloc(1, sizeof(Pack_Params));
   if (!pack_param) return;

   pack_param->pager = pager;
   pack_param->pack_sp = sp1;
   pack_param->unpack_sp = sp2;
   pack_param->unpack_btn = btn;
   pack_param->type = CLEAR;

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Clear"),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED,
                                  pack_btn_cb, pack_param),
           efl_event_callback_add(efl_added, EFL_EVENT_DEL,
                                  pack_btn_del_cb, pack_param),
           efl_pack_end(box, efl_added));

}

static void current_page_cb(void *data,
                            Evas_Object *obj EINA_UNUSED,
                            void *event_info EINA_UNUSED)
{
   Params *params = (Params *)data;
   Evas_Object *navi = params->navi;
   Eo *pager = params->pager;
   Eo *btn, *box, *sp;
   Page_Set_Params *psp = calloc(1, sizeof(Page_Set_Params));
   if (!psp) return;

   btn = efl_add(EFL_UI_BUTTON_CLASS, navi,
                 efl_text_set(efl_added, "Back"),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED,
                                        back_btn_cb, navi));

   box = efl_add(EFL_UI_BOX_CLASS, navi,
                 efl_pack_padding_set(efl_added, 10, 10, EINA_TRUE),
		 elm_naviframe_item_push(navi, "Current Page", btn, NULL,
                                         efl_added, NULL));

   btn = efl_add(EFL_UI_BUTTON_CLASS, box,
                 efl_text_set(efl_added, "Set Current Page As"),
                 efl_pack_end(box, efl_added));

   sp = efl_add(EFL_UI_SPIN_BUTTON_CLASS, box,
                efl_gfx_hint_align_set(efl_added, -1, -1),
                efl_pack_end(box, efl_added));

   if (efl_content_count(pager) > 0)
     {
        efl_ui_range_min_max_set(sp, 0,
                                 (efl_content_count(pager) - 1));
        efl_ui_range_value_set(sp,
                               efl_ui_pager_current_page_get(pager));
     }
   else
     {
        elm_object_disabled_set(btn, EINA_TRUE);
        elm_object_disabled_set(sp, EINA_TRUE);
     }

   psp->pager = pager;
   psp->spinner = sp;

   efl_event_callback_add(btn, EFL_UI_EVENT_CLICKED, page_set_btn_cb, psp);
   efl_event_callback_add(btn, EFL_EVENT_DEL, page_set_btn_del_cb, psp);
}

static void indicator_cb(void *data,
                         Evas_Object *obj EINA_UNUSED,
                         void *event_info EINA_UNUSED)
{
   Params *params = (Params *)data;
   Evas_Object *navi = params->navi;
   Eo *btn, *box;

   btn = efl_add(EFL_UI_BUTTON_CLASS, navi,
                 efl_text_set(efl_added, "Back"),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED,
                                        back_btn_cb, navi));

   box = efl_add(EFL_UI_BOX_CLASS, navi,
                 efl_pack_padding_set(efl_added, 10, 10, EINA_TRUE),
                 elm_naviframe_item_push(navi, "Indicator", btn, NULL,
                                         efl_added, NULL));

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Icon Type"),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED,
                                  indicator_icon_btn_cb, params),
           efl_pack_end(box, efl_added));

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "None"),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED,
                                  indicator_none_btn_cb, params),
           efl_pack_end(box, efl_added));
}

void test_ui_pager(void *data EINA_UNUSED,
                   Evas_Object *obj EINA_UNUSED,
                   void *event_info EINA_UNUSED)
{
   Eo *win, *panes, *navi, *list, *layout, *pager, *page;
   Params *params = NULL;
   char buf[PATH_MAX];
   int i;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
                 efl_text_set(efl_added, "Pager"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   panes = efl_add(EFL_UI_PANES_CLASS, win,
                   efl_gfx_hint_weight_set(efl_added, 1, 1),
                   efl_ui_panes_split_ratio_set(efl_added, 0.3),
                   efl_content_set(win, efl_added));

   navi = elm_naviframe_add(panes);
   evas_object_show(navi);
   efl_content_set(efl_part(panes, "first"), navi);

   list = elm_list_add(navi);
   elm_list_horizontal_set(list, EINA_FALSE);
   elm_list_select_mode_set(list, ELM_OBJECT_SELECT_MODE_ALWAYS);
   elm_naviframe_item_push(navi, "Properties", NULL, NULL, list, NULL);
   evas_object_show(list);

   snprintf(buf, sizeof(buf), "%s/objects/test_pager.edj",
            elm_app_data_dir_get());
   layout = efl_add(EFL_UI_LAYOUT_CLASS, panes,
                    efl_file_set(efl_added, buf),
                    efl_file_key_set(efl_added, "pager"),
                    efl_file_load(efl_added),
                    efl_content_set(efl_part(panes, "second"), efl_added));

   pager = efl_add(EFL_UI_PAGER_CLASS, layout,
                   efl_content_set(efl_part(layout, "pager"), efl_added),
                   efl_ui_pager_page_size_set(efl_added, EINA_SIZE2D(200, 300)));

   efl_add(EFL_UI_BUTTON_CLASS, layout,
           efl_text_set(efl_added, "Prev"),
           efl_event_callback_add(efl_added,
                                  EFL_UI_EVENT_CLICKED, prev_btn_cb, pager),
           efl_content_set(efl_part(layout, "prev_btn"), efl_added));

   efl_add(EFL_UI_BUTTON_CLASS, layout,
           efl_text_set(efl_added, "Next"),
           efl_event_callback_add(efl_added,
                                  EFL_UI_EVENT_CLICKED, next_btn_cb, pager),
           efl_content_set(efl_part(layout, "next_btn"), efl_added));

   params = calloc(1, sizeof(Params));
   if (!params) return;

   params->navi = navi;
   params->pager = pager;
   params->indicator = NULL;
   params->w = 200;
   params->h = 300;
   params->wfill = EINA_FALSE;
   params->hfill = EINA_FALSE;

   elm_list_item_append(list, "Page Size", NULL, NULL, page_size_cb, params);
   elm_list_item_append(list, "Pack / Unpack", NULL, NULL, pack_cb, params);
   elm_list_item_append(list, "Current Page", NULL, NULL, current_page_cb, params);
   elm_list_item_append(list, "Indicator", NULL, NULL, indicator_cb, params);
   elm_list_go(list);

   efl_event_callback_add(list, EFL_EVENT_DEL, list_del_cb, params);

   for (i = 0; i < PAGE_NUM; i++) {
      switch (i % 3) {
         case 0:
            page = page_add(LAYOUT, pager);
            break;
         case 1:
            page = page_add(LIST, pager);
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

   efl_gfx_entity_size_set(win, EINA_SIZE2D(580, 320));
}
