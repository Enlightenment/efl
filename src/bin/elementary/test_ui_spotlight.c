#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#define EO_BETA_API
#define EFL_UI_WIDGET_PROTECTED

#include <Efl_Ui.h>
#include <Elementary.h>

typedef enum _View_Type
{
   LAYOUT,
   LIST,
   BUTTON
} View_Type;

typedef enum _Pack_Type
{
   PACK_BEGIN,
   PACK_END,
   PACK_BEFORE,
   PACK_AFTER,
   PACK_AT,
   UNPACK_AT,
   CLEAR
} Pack_Type;

typedef struct _Params
{
   Evas_Object *navi;
   Eo          *spotlight;
   Eo          *indicator;
   int          w, h;
   Eina_Bool    wfill, hfill;
} Params;

typedef struct _Page_Set_Params
{
   Eo *spotlight;
   Eo *spinner;
} Page_Set_Params;

typedef struct _Pack_Params
{
   Pack_Type type;
   Eo       *spotlight;
   Eo       *pack_sp;
   Eo       *unpack_sp;
   Eo       *unpack_btn;
} Pack_Params;

typedef struct _Size_Params
{
   Eo     *spotlight;
   Eo     *slider;
   Params *params;
} Size_Params;

#define PAGE_NUM 3

static void
page_clicked_cb(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   printf("Button Page is clicked!\n");
}

static Eo *
view_add(View_Type p, Eo *parent)
{
   Eo *page;
   char buf[PATH_MAX];
   int i;

   switch (p)
     {
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
          efl_gfx_hint_size_min_set(page, EINA_SIZE2D(200, 200));
          evas_object_size_hint_weight_set(page, EVAS_HINT_EXPAND,
                                           EVAS_HINT_EXPAND);
          evas_object_size_hint_align_set(page, EVAS_HINT_FILL,
                                          EVAS_HINT_FILL);
          break;

        case BUTTON:
          page = efl_add(EFL_UI_BUTTON_CLASS, parent,
                         efl_text_set(efl_added, "Button Page"));
          efl_event_callback_add(page, EFL_INPUT_EVENT_CLICKED, page_clicked_cb, NULL);
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

static void
prev_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *spotlight = data;
   int active_index = efl_pack_index_get(spotlight, efl_ui_spotlight_active_element_get(spotlight));

   if (active_index - 1 > -1)
     efl_ui_spotlight_active_element_set(spotlight, efl_pack_content_get(spotlight, active_index - 1));
}

static void
next_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *spotlight = data;
   int active_index = efl_pack_index_get(spotlight, efl_ui_spotlight_active_element_get(spotlight));

   if (active_index + 1 < efl_content_count(spotlight))
     efl_ui_spotlight_active_element_set(spotlight, efl_pack_content_get(spotlight, active_index + 1));
}

static Eina_Value
future_then_cb(void *data EINA_UNUSED, const Eina_Value value, const Eina_Future *dead_future EINA_UNUSED)
{
   /* If efl_ui_spotlight_pop is called with EINA_FALSE, then the content is not
    * deleted and the value contains the content. */
   Eo *content = eina_value_object_get(&value);
   if (content)
     efl_gfx_entity_visible_set(content, EINA_FALSE);

   return EINA_VALUE_EMPTY;
}

static void
pop_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *spotlight = data;

   if (efl_content_count(spotlight) == 0) return;

   Eina_Future *future = efl_ui_spotlight_pop(spotlight, EINA_TRUE);
   eina_future_then(future, future_then_cb, NULL);
}

static void
push_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *spotlight = data;
   Eo *view;
   int count = efl_content_count(spotlight);

   switch (count % 3)
     {
        case 0:
          view = view_add(BUTTON, spotlight);
          break;

        case 1:
          view = view_add(LIST, spotlight);
          break;

        case 2:
          view = view_add(LAYOUT, spotlight);
          break;

        default:
          view = view_add(LAYOUT, spotlight);
          break;
     }
   efl_ui_spotlight_push(spotlight, view);
}

static void
back_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   elm_naviframe_item_pop(data);
}

static void
list_del_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   free(data);
}

static void
width_slider_cb(void *data, const Efl_Event *ev)
{
   Params *params = data;
   int h;

   if (params->hfill) h = -1;
   else h = params->h;

   params->w = efl_ui_range_value_get(ev->object);
   efl_ui_spotlight_size_set(params->spotlight, EINA_SIZE2D(params->w, h));
}

static void
height_slider_cb(void *data, const Efl_Event *ev)
{
   Params *params = data;
   int w;

   if (params->wfill) w = -1;
   else w = params->w;

   params->h = efl_ui_range_value_get(ev->object);
   efl_ui_spotlight_size_set(params->spotlight, EINA_SIZE2D(w, params->h));
}

static void
width_check_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Size_Params *params = data;
   Eina_Bool ck = elm_check_selected_get(obj);
   int w, h;

   elm_object_disabled_set(params->slider, ck);

   params->params->wfill = ck;

   if (params->params->wfill) w = -1;
   else w = params->params->w;

   if (params->params->hfill) h = -1;
   else h = params->params->h;

   efl_ui_spotlight_size_set(params->spotlight, EINA_SIZE2D(w, h));
}

static void
height_check_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Size_Params *params = data;
   Eina_Bool ck = elm_check_selected_get(obj);
   int w, h;

   elm_object_disabled_set(params->slider, ck);

   params->params->hfill = ck;

   if (params->params->wfill) w = -1;
   else w = params->params->w;

   if (params->params->hfill) h = -1;
   else h = params->params->h;

   efl_ui_spotlight_size_set(params->spotlight, EINA_SIZE2D(w, h));
}

static void
check_del_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   free(data);
}

static void
pack_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Pack_Params *param = data;
   Eo *spotlight = param->spotlight;
   Eo *page = NULL;
   int index, cnt;

   if ((param->type != UNPACK_AT) && (param->type != CLEAR))
     {
        index = efl_content_count(spotlight);

        switch (index % 3)
          {
             case 0:
               page = view_add(LAYOUT, spotlight);
               break;

             case 1:
               page = view_add(LIST, spotlight);
               break;

             case 2:
               page = view_add(BUTTON, spotlight);
               break;

             default:
               page = view_add(LAYOUT, spotlight);
               break;
          }
     }

   switch (param->type)
     {
        case PACK_BEGIN:
          efl_pack_begin(spotlight, page);
          break;

        case PACK_END:
          efl_pack_end(spotlight, page);
          break;

        case PACK_BEFORE:
          efl_pack_before(spotlight, page, efl_ui_spotlight_active_element_get(spotlight));
          break;

        case PACK_AFTER:
          efl_pack_after(spotlight, page, efl_ui_spotlight_active_element_get(spotlight));
          break;

        case PACK_AT:
          index = efl_ui_range_value_get(param->pack_sp);
          efl_pack_at(spotlight, page, index);
          break;

        case UNPACK_AT:
          index = efl_ui_range_value_get(param->unpack_sp);
          page = efl_pack_unpack_at(spotlight, index);
          efl_del(page);
          break;

        case CLEAR:
          efl_pack_clear(spotlight);
          break;
     }

   cnt = efl_content_count(spotlight);

   index = efl_ui_range_value_get(param->pack_sp);
   if (index > cnt)
     efl_ui_range_value_set(param->pack_sp, cnt);
   if (cnt > 0)
     efl_ui_range_limits_set(param->pack_sp, 0, cnt);

   if (cnt > 0)
     {
        elm_object_disabled_set(param->unpack_btn, EINA_FALSE);
        elm_object_disabled_set(param->unpack_sp, EINA_FALSE);

        cnt -= 1;
        index = efl_ui_range_value_get(param->unpack_sp);
        if (index > cnt)
          efl_ui_range_value_set(param->unpack_sp, cnt);
        if (cnt > 0)
          efl_ui_range_limits_set(param->unpack_sp, 0, cnt);
     }
   else
     {
        elm_object_disabled_set(param->unpack_btn, EINA_TRUE);
        elm_object_disabled_set(param->unpack_sp, EINA_TRUE);
     }
}

static void
pack_btn_del_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   free(data);
}

static void
page_set_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Page_Set_Params *psp = data;

   efl_ui_spotlight_active_element_set(psp->spotlight,
    efl_pack_content_get(psp->spotlight,
      efl_ui_range_value_get(psp->spinner)));
}

static void
page_set_btn_del_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   free(data);
}

static void
indicator_icon_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Params *params = data;

   params->indicator = efl_new(EFL_UI_SPOTLIGHT_ICON_INDICATOR_CLASS);
   efl_ui_spotlight_indicator_set(params->spotlight, params->indicator);
}

static void
indicator_none_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Params *params = data;
   efl_ui_spotlight_indicator_set(params->spotlight, NULL);
}

static void
spotlight_size(void *data,
                 Evas_Object *obj EINA_UNUSED,
                 void *event_info EINA_UNUSED)
{
   Params *params = data;
   Size_Params *size_params;
   Evas_Object *navi = params->navi;
   Eo *btn, *box, *fr, *inbox, *ck, *sl;

   btn = efl_add(EFL_UI_BUTTON_CLASS, navi,
                 efl_text_set(efl_added, "Back"),
                 efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                        back_btn_cb, navi));

   box = efl_add(EFL_UI_BOX_CLASS, navi,
                 elm_naviframe_item_push(navi, "View Size", btn, NULL,
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
                efl_ui_range_limits_set(efl_added, 100, 200),
                efl_ui_range_value_set(efl_added, params->w),
                efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(100, 0)),
                efl_event_callback_add(efl_added, EFL_UI_RANGE_EVENT_CHANGED,
                                       width_slider_cb, params),
                efl_pack_end(inbox, efl_added));

   size_params = calloc(1, sizeof(Size_Params));
   if (!size_params) return;

   size_params->slider = sl;
   size_params->spotlight = params->spotlight;
   size_params->params = params;

   evas_object_smart_callback_add(ck, "changed", width_check_cb, size_params);
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
                efl_ui_range_limits_set(efl_added, 100, 300),
                efl_ui_range_value_set(efl_added, params->h),
                efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(100, 0)),
                efl_event_callback_add(efl_added, EFL_UI_RANGE_EVENT_CHANGED,
                                       height_slider_cb, params),
                efl_pack_end(inbox, efl_added));

   size_params = calloc(1, sizeof(Size_Params));
   if (!size_params) return;

   size_params->slider = sl;
   size_params->spotlight = params->spotlight;
   size_params->params = params;

   evas_object_smart_callback_add(ck, "changed", height_check_cb, size_params);
   efl_event_callback_add(ck, EFL_EVENT_DEL, check_del_cb, size_params);

   if (params->hfill)
     {
        elm_check_state_set(ck, EINA_TRUE);
        elm_object_disabled_set(sl, EINA_TRUE);
     }
}

static void
_animation_cb(void *data, const Efl_Event *ev)
{
   Params *params = data;

   efl_ui_spotlight_manager_animated_transition_set(efl_ui_spotlight_manager_get(params->spotlight), efl_ui_selectable_selected_get(ev->object));
}

static void
view_animation_cb(void *data,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED)
{
   Params *params = data;
   Evas_Object *navi = params->navi;
   Eo *btn, *box, *ck;

   btn = efl_add(EFL_UI_BUTTON_CLASS, navi,
                 efl_text_set(efl_added, "Back"),
                 efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                        back_btn_cb, navi));

   box = efl_add(EFL_UI_BOX_CLASS, navi,
                 elm_naviframe_item_push(navi, "View Manager animation", btn, NULL,
                                         efl_added, NULL));

   ck = efl_add(EFL_UI_CHECK_CLASS, box);
   efl_event_callback_add(ck, EFL_UI_EVENT_SELECTED_CHANGED, _animation_cb, params);
   efl_ui_selectable_selected_set(ck, efl_ui_spotlight_manager_animated_transition_get(efl_ui_spotlight_manager_get(params->spotlight)));
   efl_text_set(ck, "Animation");
   efl_pack_end(box, ck);
   efl_gfx_entity_visible_set(ck, 1);
}

static void
_scroll_block_check_cb(void *data, const Efl_Event *ev)
{
   Params *params = data;

   efl_ui_spotlight_manager_scroll_block_set(efl_ui_spotlight_manager_get(params->spotlight), efl_ui_selectable_selected_get(ev->object));
}

static void
scroll_block_cb(void *data,
                Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   Params *params = data;
   Evas_Object *navi = params->navi;
   Eo *btn, *box, *ck;

   btn = efl_add(EFL_UI_BUTTON_CLASS, navi,
                 efl_text_set(efl_added, "Back"),
                 efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                        back_btn_cb, navi));

   box = efl_add(EFL_UI_BOX_CLASS, navi,
                 elm_naviframe_item_push(navi, "Scroll Block", btn, NULL,
                                         efl_added, NULL));

   ck = efl_add(EFL_UI_CHECK_CLASS, box);
   efl_event_callback_add(ck, EFL_UI_EVENT_SELECTED_CHANGED, _scroll_block_check_cb, params);
   efl_ui_selectable_selected_set(ck, efl_ui_spotlight_manager_scroll_block_get(efl_ui_spotlight_manager_get(params->spotlight)));
   efl_text_set(ck, "Scroll Block");
   efl_pack_end(box, ck);
   efl_gfx_entity_visible_set(ck, 1);
}

static void
pack_cb(void *data,
        Evas_Object *obj EINA_UNUSED,
        void *event_info EINA_UNUSED)
{
   Params *params = (Params *)data;
   Evas_Object *navi = params->navi;
   Eo *spotlight = params->spotlight;
   Eo *btn, *box, *in_box1, *in_box2, *sp1, *sp2;
   Pack_Params *pack_param;
   int num;

   btn = efl_add(EFL_UI_BUTTON_CLASS, navi,
                 efl_text_set(efl_added, "Back"),
                 efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                        back_btn_cb, navi));

   box = efl_add(EFL_UI_BOX_CLASS, navi,
                 efl_gfx_arrangement_content_padding_set(efl_added, 10, 10),
                 elm_naviframe_item_push(navi, "Pack", btn, NULL,
                                         efl_added, NULL));

   in_box1 = efl_add(EFL_UI_BOX_CLASS, box,
                     efl_gfx_arrangement_content_padding_set(efl_added, 10, 10),
                     efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL));

   sp1 = efl_add(EFL_UI_SPIN_BUTTON_CLASS, in_box1);
   num = efl_content_count(spotlight);
   if (num)
     efl_ui_range_limits_set(sp1, 0, num);
   num = efl_pack_index_get(spotlight, efl_ui_spotlight_active_element_get(spotlight));
   if (num >= 0)
     efl_ui_range_value_set(sp1, num);

   in_box2 = efl_add(EFL_UI_BOX_CLASS, box,
                     efl_gfx_arrangement_content_padding_set(efl_added, 10, 10),
                     efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL));

   sp2 = efl_add(EFL_UI_SPIN_BUTTON_CLASS, in_box2);

   btn = efl_add(EFL_UI_BUTTON_CLASS, in_box2,
                 efl_text_set(efl_added, "Unpack At"));

   // Pack Begin
   pack_param = calloc(1, sizeof(Pack_Params));
   if (!pack_param) return;

   pack_param->spotlight = spotlight;
   pack_param->pack_sp = sp1;
   pack_param->unpack_sp = sp2;
   pack_param->unpack_btn = btn;
   pack_param->type = PACK_BEGIN;

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Pack Begin"),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                  pack_btn_cb, pack_param),
           efl_event_callback_add(efl_added, EFL_EVENT_DEL,
                                  pack_btn_del_cb, pack_param),
           efl_pack_end(box, efl_added));

   // Pack End
   pack_param = calloc(1, sizeof(Pack_Params));
   if (!pack_param) return;

   pack_param->spotlight = spotlight;
   pack_param->pack_sp = sp1;
   pack_param->unpack_sp = sp2;
   pack_param->unpack_btn = btn;
   pack_param->type = PACK_END;

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Pack End"),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                  pack_btn_cb, pack_param),
           efl_event_callback_add(efl_added, EFL_EVENT_DEL,
                                  pack_btn_del_cb, pack_param),
           efl_pack_end(box, efl_added));

   // Pack Before
   pack_param = calloc(1, sizeof(Pack_Params));
   if (!pack_param) return;

   pack_param->spotlight = spotlight;
   pack_param->pack_sp = sp1;
   pack_param->unpack_sp = sp2;
   pack_param->unpack_btn = btn;
   pack_param->type = PACK_BEFORE;

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Pack Before Current Page"),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                  pack_btn_cb, pack_param),
           efl_event_callback_add(efl_added, EFL_EVENT_DEL,
                                  pack_btn_del_cb, pack_param),
           efl_pack_end(box, efl_added));

   // Pack After
   pack_param = calloc(1, sizeof(Pack_Params));
   if (!pack_param) return;

   pack_param->spotlight = spotlight;
   pack_param->pack_sp = sp1;
   pack_param->unpack_sp = sp2;
   pack_param->unpack_btn = btn;
   pack_param->type = PACK_AFTER;

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Pack After Current Page"),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                  pack_btn_cb, pack_param),
           efl_event_callback_add(efl_added, EFL_EVENT_DEL,
                                  pack_btn_del_cb, pack_param),
           efl_pack_end(box, efl_added));

   // Pack At
   pack_param = calloc(1, sizeof(Pack_Params));
   if (!pack_param) return;

   pack_param->spotlight = spotlight;
   pack_param->pack_sp = sp1;
   pack_param->unpack_sp = sp2;
   pack_param->unpack_btn = btn;
   pack_param->type = PACK_AT;

   efl_add(EFL_UI_BUTTON_CLASS, in_box1,
           efl_text_set(efl_added, "Pack At"),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                  pack_btn_cb, pack_param),
           efl_event_callback_add(efl_added, EFL_EVENT_DEL,
                                  pack_btn_del_cb, pack_param),
           efl_pack_end(in_box1, efl_added));

   efl_pack_end(box, in_box1);
   efl_pack_end(in_box1, sp1);

   // Unpack At
   pack_param = calloc(1, sizeof(Pack_Params));
   if (!pack_param) return;

   pack_param->spotlight = spotlight;
   pack_param->pack_sp = sp1;
   pack_param->unpack_sp = sp2;
   pack_param->unpack_btn = btn;
   pack_param->type = UNPACK_AT;

   efl_event_callback_add(btn, EFL_INPUT_EVENT_CLICKED,
                          pack_btn_cb, pack_param);
   efl_event_callback_add(btn, EFL_EVENT_DEL,
                          pack_btn_del_cb, pack_param);

   if (efl_content_count(spotlight) > 0)
     {
        efl_ui_range_limits_set(sp2, 0,
                                 (efl_content_count(spotlight) - 1));
        efl_ui_range_value_set(sp2,
                               efl_pack_index_get(spotlight, efl_ui_spotlight_active_element_get(spotlight)));
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

   pack_param->spotlight = spotlight;
   pack_param->pack_sp = sp1;
   pack_param->unpack_sp = sp2;
   pack_param->unpack_btn = btn;
   pack_param->type = CLEAR;

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Clear"),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                  pack_btn_cb, pack_param),
           efl_event_callback_add(efl_added, EFL_EVENT_DEL,
                                  pack_btn_del_cb, pack_param),
           efl_pack_end(box, efl_added));
}

static void
active_index_cb(void *data,
                Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   Params *params = (Params *)data;
   Evas_Object *navi = params->navi;
   Eo *spotlight = params->spotlight;
   Eo *btn, *box, *sp;
   Page_Set_Params *psp = calloc(1, sizeof(Page_Set_Params));
   if (!psp) return;

   btn = efl_add(EFL_UI_BUTTON_CLASS, navi,
                 efl_text_set(efl_added, "Back"),
                 efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                        back_btn_cb, navi));

   box = efl_add(EFL_UI_BOX_CLASS, navi,
                 efl_gfx_arrangement_content_padding_set(efl_added, 10, 10),
                 elm_naviframe_item_push(navi, "Active Index", btn, NULL,
                                         efl_added, NULL));

   btn = efl_add(EFL_UI_BUTTON_CLASS, box,
                 efl_text_set(efl_added, "Set Active Index"),
                 efl_pack_end(box, efl_added));

   sp = efl_add(EFL_UI_SPIN_BUTTON_CLASS, box,
                efl_gfx_hint_align_set(efl_added, -1, -1),
                efl_pack_end(box, efl_added));

   if (efl_content_count(spotlight) > 0)
     {
        efl_ui_range_limits_set(sp, 0,
                                 (efl_content_count(spotlight) - 1));
        efl_ui_range_value_set(sp,
                               efl_pack_index_get(spotlight, efl_ui_spotlight_active_element_get(spotlight)));
     }
   else
     {
        elm_object_disabled_set(btn, EINA_TRUE);
        elm_object_disabled_set(sp, EINA_TRUE);
     }

   psp->spotlight = spotlight;
   psp->spinner = sp;

   efl_event_callback_add(btn, EFL_INPUT_EVENT_CLICKED, page_set_btn_cb, psp);
   efl_event_callback_add(btn, EFL_EVENT_DEL, page_set_btn_del_cb, psp);
}

static void
indicator_cb(void *data,
             Evas_Object *obj EINA_UNUSED,
             void *event_info EINA_UNUSED)
{
   Params *params = (Params *)data;
   Evas_Object *navi = params->navi;
   Eo *btn, *box;

   btn = efl_add(EFL_UI_BUTTON_CLASS, navi,
                 efl_text_set(efl_added, "Back"),
                 efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                        back_btn_cb, navi));

   box = efl_add(EFL_UI_BOX_CLASS, navi,
                 efl_gfx_arrangement_content_padding_set(efl_added, 10, 10),
                 elm_naviframe_item_push(navi, "Indicator", btn, NULL,
                                         efl_added, NULL));

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "Icon Type"),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                  indicator_icon_btn_cb, params),
           efl_pack_end(box, efl_added));

   efl_add(EFL_UI_BUTTON_CLASS, box,
           efl_text_set(efl_added, "None"),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED,
                                  indicator_none_btn_cb, params),
           efl_pack_end(box, efl_added));
}

void
test_ui_spotlight_stack(void *data EINA_UNUSED,
                          Evas_Object *obj EINA_UNUSED,
                          void *event_info EINA_UNUSED)
{
   Eo *win, *panes, *navi, *list, *layout, *spotlight, *view = NULL;
   Params *params = NULL;
   char buf[PATH_MAX];
   int i;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                                  efl_text_set(efl_added, "Efl.Ui.Spotlight Stack"),
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

   spotlight = efl_add(EFL_UI_STACK_CLASS, layout,
                         efl_content_set(efl_part(layout, "pager"), efl_added),
                         efl_ui_spotlight_size_set(efl_added, EINA_SIZE2D(200, 300)));

   efl_add(EFL_UI_BUTTON_CLASS, layout,
           efl_text_set(efl_added, "Pop"),
           efl_event_callback_add(efl_added,
                                  EFL_INPUT_EVENT_CLICKED, pop_btn_cb, spotlight),
           efl_content_set(efl_part(layout, "prev_btn"), efl_added));

   efl_add(EFL_UI_BUTTON_CLASS, layout,
           efl_text_set(efl_added, "Push"),
           efl_event_callback_add(efl_added,
                                  EFL_INPUT_EVENT_CLICKED, push_btn_cb, spotlight),
           efl_content_set(efl_part(layout, "next_btn"), efl_added));

   params = calloc(1, sizeof(Params));
   if (!params) return;

   params->navi = navi;
   params->spotlight = spotlight;
   params->indicator = NULL;
   params->w = 200;
   params->h = 300;
   params->wfill = EINA_FALSE;
   params->hfill = EINA_FALSE;

   elm_list_item_append(list, "View Size", NULL, NULL, spotlight_size, params);
   elm_list_item_append(list, "Pack / Unpack", NULL, NULL, pack_cb, params);
   elm_list_item_append(list, "Active Index", NULL, NULL, active_index_cb, params);
   elm_list_item_append(list, "Indicator", NULL, NULL, indicator_cb, params);
   elm_list_item_append(list, "Animation", NULL, NULL, view_animation_cb, params);
   elm_list_item_append(list, "Scroll Block", NULL, NULL, scroll_block_cb, params);
   elm_list_go(list);

   efl_event_callback_add(list, EFL_EVENT_DEL, list_del_cb, params);

   for (i = 0; i < PAGE_NUM; i++)
     {
        switch (i % 3)
          {
           case 0:
             view = view_add(LAYOUT, spotlight);
             break;

           case 1:
             view = view_add(LIST, spotlight);
             break;

           case 2:
             view = view_add(BUTTON, spotlight);
             break;
          }
        efl_pack_end(spotlight, view);
     }

   efl_gfx_entity_size_set(win, EINA_SIZE2D(580, 320));
}

void
test_ui_spotlight_plain(void *data EINA_UNUSED,
                          Evas_Object *obj EINA_UNUSED,
                          void *event_info EINA_UNUSED)
{
   Eo *win, *panes, *navi, *list, *layout, *spotlight, *view = NULL;
   Params *params = NULL;
   char buf[PATH_MAX];
   int i;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                                  efl_text_set(efl_added, "Efl.Ui.Spotlight Plain"),
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

   spotlight = efl_add(EFL_UI_SPOTLIGHT_CONTAINER_CLASS, layout,
                         efl_content_set(efl_part(layout, "pager"), efl_added),
                         efl_ui_spotlight_size_set(efl_added, EINA_SIZE2D(200, 300)));

   efl_add(EFL_UI_BUTTON_CLASS, layout,
           efl_text_set(efl_added, "Prev"),
           efl_event_callback_add(efl_added,
                                  EFL_INPUT_EVENT_CLICKED, prev_btn_cb, spotlight),
           efl_content_set(efl_part(layout, "prev_btn"), efl_added));

   efl_add(EFL_UI_BUTTON_CLASS, layout,
           efl_text_set(efl_added, "Next"),
           efl_event_callback_add(efl_added,
                                  EFL_INPUT_EVENT_CLICKED, next_btn_cb, spotlight),
           efl_content_set(efl_part(layout, "next_btn"), efl_added));

   params = calloc(1, sizeof(Params));
   if (!params) return;

   params->navi = navi;
   params->spotlight = spotlight;
   params->indicator = NULL;
   params->w = 200;
   params->h = 300;
   params->wfill = EINA_FALSE;
   params->hfill = EINA_FALSE;

   elm_list_item_append(list, "View Size", NULL, NULL, spotlight_size, params);
   elm_list_item_append(list, "Pack / Unpack", NULL, NULL, pack_cb, params);
   elm_list_item_append(list, "Active Index", NULL, NULL, active_index_cb, params);
   elm_list_item_append(list, "Indicator", NULL, NULL, indicator_cb, params);
   elm_list_go(list);

   efl_event_callback_add(list, EFL_EVENT_DEL, list_del_cb, params);

   for (i = 0; i < PAGE_NUM; i++)
     {
        switch (i % 3)
          {
           case 0:
             view = view_add(LAYOUT, spotlight);
             break;

           case 1:
             view = view_add(LIST, spotlight);
             break;

           case 2:
             view = view_add(BUTTON, spotlight);
             break;
          }
        efl_pack_end(spotlight, view);
     }

   efl_gfx_entity_size_set(win, EINA_SIZE2D(580, 320));
}

void
test_ui_spotlight_scroll(void *data EINA_UNUSED,
                           Evas_Object *obj EINA_UNUSED,
                           void *event_info EINA_UNUSED)
{
   Eo *win, *panes, *navi, *list, *layout, *spotlight, *view = NULL;
   Params *params = NULL;
   char buf[PATH_MAX];
   int i;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                                  efl_text_set(efl_added, "Efl.Ui.Spotlight Scroll"),
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

   spotlight = efl_add(EFL_UI_PAGER_CLASS, layout,
                         efl_content_set(efl_part(layout, "pager"), efl_added),
                         efl_ui_spotlight_size_set(efl_added, EINA_SIZE2D(200, 300)));

   efl_add(EFL_UI_BUTTON_CLASS, layout,
           efl_text_set(efl_added, "Prev"),
           efl_event_callback_add(efl_added,
                                  EFL_INPUT_EVENT_CLICKED, prev_btn_cb, spotlight),
           efl_content_set(efl_part(layout, "prev_btn"), efl_added));

   efl_add(EFL_UI_BUTTON_CLASS, layout,
           efl_text_set(efl_added, "Next"),
           efl_event_callback_add(efl_added,
                                  EFL_INPUT_EVENT_CLICKED, next_btn_cb, spotlight),
           efl_content_set(efl_part(layout, "next_btn"), efl_added));

   params = calloc(1, sizeof(Params));
   if (!params) return;

   params->navi = navi;
   params->spotlight = spotlight;
   params->indicator = NULL;
   params->w = 200;
   params->h = 300;
   params->wfill = EINA_FALSE;
   params->hfill = EINA_FALSE;

   elm_list_item_append(list, "View Size", NULL, NULL, spotlight_size, params);
   elm_list_item_append(list, "Pack / Unpack", NULL, NULL, pack_cb, params);
   elm_list_item_append(list, "Active Index", NULL, NULL, active_index_cb, params);
   elm_list_item_append(list, "Indicator", NULL, NULL, indicator_cb, params);
   elm_list_item_append(list, "Animation", NULL, NULL, view_animation_cb, params);
   elm_list_item_append(list, "Scroll Block", NULL, NULL, scroll_block_cb, params);
   elm_list_go(list);

   efl_event_callback_add(list, EFL_EVENT_DEL, list_del_cb, params);

   for (i = 0; i < PAGE_NUM; i++) {
        switch (i % 3)
          {
           case 0:
             view = view_add(LAYOUT, spotlight);
             break;

           case 1:
             view = view_add(LIST, spotlight);
             break;

           case 2:
             view = view_add(BUTTON, spotlight);
             break;
          }
        efl_pack_end(spotlight, view);
     }

   efl_gfx_entity_size_set(win, EINA_SIZE2D(580, 320));
}


void
test_ui_spotlight_animation(void *data EINA_UNUSED,
                           Evas_Object *obj EINA_UNUSED,
                           void *event_info EINA_UNUSED)
{
   Eo *win, *panes, *navi, *list, *layout, *spotlight, *view, *custom_animation_manager;
   Efl_Canvas_Animation *jump_animation, *push_animation, *pop_animation;
   Params *params = NULL;
   char buf[PATH_MAX];
   int i;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                                  efl_text_set(efl_added, "Efl.Ui.Spotlight Scroll"),
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


   jump_animation = efl_new(EFL_CANVAS_ALPHA_ANIMATION_CLASS);
   efl_animation_alpha_set(jump_animation, 0.0, 1.0);
   efl_animation_duration_set(jump_animation, 0.5);

   push_animation = efl_new(EFL_CANVAS_TRANSLATE_ANIMATION_CLASS);
   efl_animation_translate_set(push_animation, EINA_POSITION2D(0, 100), EINA_POSITION2D(0, 0));
   efl_animation_duration_set(push_animation, 0.5);

   pop_animation = efl_new(EFL_CANVAS_TRANSLATE_ANIMATION_CLASS);
   efl_animation_translate_set(pop_animation, EINA_POSITION2D(0, -100), EINA_POSITION2D(0, 0));
   efl_animation_duration_set(pop_animation, 0.5);

   custom_animation_manager = efl_new(EFL_UI_SPOTLIGHT_ANIMATION_MANAGER_CLASS,
                                      efl_ui_spotlight_manager_animation_push_setup_set(efl_added, push_animation, jump_animation),
                                      efl_ui_spotlight_manager_animation_pop_setup_set(efl_added, jump_animation, pop_animation),
                                      efl_ui_spotlight_manager_animation_jump_setup_set(efl_added, jump_animation, jump_animation));

   spotlight = efl_add(EFL_UI_SPOTLIGHT_CONTAINER_CLASS, layout,
                         efl_ui_spotlight_manager_set(efl_added, custom_animation_manager),
                         efl_content_set(efl_part(layout, "pager"), efl_added),
                         efl_ui_spotlight_size_set(efl_added, EINA_SIZE2D(200, 300)));

   efl_add(EFL_UI_BUTTON_CLASS, layout,
           efl_text_set(efl_added, "Pop"),
           efl_event_callback_add(efl_added,
                                  EFL_INPUT_EVENT_CLICKED, pop_btn_cb, spotlight),
           efl_content_set(efl_part(layout, "prev_btn"), efl_added));

   efl_add(EFL_UI_BUTTON_CLASS, layout,
           efl_text_set(efl_added, "Push"),
           efl_event_callback_add(efl_added,
                                  EFL_INPUT_EVENT_CLICKED, push_btn_cb, spotlight),
           efl_content_set(efl_part(layout, "next_btn"), efl_added));

   params = calloc(1, sizeof(Params));
   if (!params) return;

   params->navi = navi;
   params->spotlight = spotlight;
   params->indicator = NULL;
   params->w = 200;
   params->h = 300;
   params->wfill = EINA_FALSE;
   params->hfill = EINA_FALSE;

   elm_list_item_append(list, "View Size", NULL, NULL, spotlight_size, params);
   elm_list_item_append(list, "Pack / Unpack", NULL, NULL, pack_cb, params);
   elm_list_item_append(list, "Active Index", NULL, NULL, active_index_cb, params);
   elm_list_item_append(list, "Indicator", NULL, NULL, indicator_cb, params);
   elm_list_item_append(list, "Animation", NULL, NULL, view_animation_cb, params);
   elm_list_item_append(list, "Scroll Block", NULL, NULL, scroll_block_cb, params);
   elm_list_go(list);

   efl_event_callback_add(list, EFL_EVENT_DEL, list_del_cb, params);

   for (i = 0; i < PAGE_NUM; i++) {
        switch (i % 3)
          {
           case 0:
             view = view_add(LAYOUT, spotlight);
             break;

           case 1:
             view = view_add(LIST, spotlight);
             break;

           case 2:
             view = view_add(BUTTON, spotlight);
             break;
          }
        efl_pack_end(spotlight, view);
     }

   efl_gfx_entity_size_set(win, EINA_SIZE2D(580, 320));
}
