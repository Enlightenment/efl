#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Efl_Ui.h>
#include <Elementary.h>

#define TAB_LABEL_COUNT 15
#define TAB_ICON_COUNT 9

typedef struct _App_Data {
   Evas_Object *navi;
   Eo *tab_pager;
} App_Data;

typedef struct _Tab_Set_Data {
   Eo *tab_pager;
   Eo *spinner;
} Tab_Set_Data;

typedef struct _Tab_Change_Data {
   Eo *tab_pager;
   Eo *label_check;
   Eo *icon_check;
} Tab_Change_Data;

static int tab_label_count;
static int tab_icon_count;

static void _current_cb(void *data, Evas_Object *obj, void *event_info);
static void _pack_cb(void *data, Evas_Object *obj, void *event_info);
static void _unpack_cb(void *data, Evas_Object *obj, void *event_info);
static void _tab_cb(void *data, Evas_Object *obj, void *event_info);
static void _transition_cb(void *data, Evas_Object *obj, void *event_info);
static void _win_del_cb(void *data, const Efl_Event *ev);

static char *tab_labels[] = {
   "efl", "elementary", "ecore", "evas", "eina",
   "eo", "eolian", "embryo", "ethumb", "evil",
   "eet", "edje", "ector", "efreet", "eldbus"
};

static char *tab_icons[] = {
   "document-print", "folder-new", "object-rotate-right",
   "mail-send", "edit-cut", "edit-copy",
   "edit-paste", "edit-delete", "clock"
};

static char *tab_label_get()
{
	if (tab_label_count == TAB_LABEL_COUNT) tab_label_count = 0;
	return tab_labels[tab_label_count++];
}

static char *tab_icon_get()
{
	if (tab_icon_count == TAB_ICON_COUNT) tab_icon_count = 0;
	return tab_icons[tab_icon_count++];
}

Eo *
content_add(Eo *parent, char *text)
{
   Eo *page;
   char buf[PATH_MAX];
   page = efl_add(EFL_UI_LAYOUT_CLASS, parent);

   snprintf(buf, sizeof(buf), "%s/objects/test_tab_pager.edj", elm_app_data_dir_get());

   page = efl_add(EFL_UI_LAYOUT_CLASS, parent,
                  efl_file_set(efl_added, buf),
                  efl_file_key_set(efl_added, "page_layout"),
                  efl_file_load(efl_added),
                  efl_text_set(efl_part(efl_added, "text"), text),
                  efl_gfx_hint_weight_set(efl_added, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
                  efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_TRUE));

   return page;
}

Eo *
tab_page_add(Eo *parent)
{
   Eo *tab_page;
   char *label = tab_label_get();
   char *icon = tab_icon_get();
   Eo *content = content_add(parent, label);

   tab_page = efl_add(EFL_UI_TAB_PAGE_CLASS, parent);
   efl_content_set(tab_page, content);
   Eo *item = efl_ui_tab_page_tab_bar_item_get(tab_page);
   efl_text_set(item, label);
   efl_ui_tab_bar_default_item_icon_set(item, icon);

   return tab_page;
}

void
test_ui_tab_pager(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *layout, *navi, *list, *tp, *page;
   //Efl_Page_Transition *tran;
   App_Data *ad = NULL;
   char buf[PATH_MAX];
   int i;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                                  efl_text_set(efl_added, "Efl.Ui.Tab_Pager"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE),
                 efl_event_callback_add(efl_added, EFL_EVENT_DEL, _win_del_cb, ad));

   snprintf(buf, sizeof(buf), "%s/objects/test_tab_pager.edj", elm_app_data_dir_get());
   layout = efl_add(EFL_UI_LAYOUT_CLASS, win,
                    efl_file_set(efl_added, buf),
                    efl_file_key_set(efl_added, "tab_page_layout"),
                    efl_content_set(win, efl_added));

   navi = elm_naviframe_add(layout);
   efl_gfx_hint_weight_set(navi, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   efl_gfx_hint_align_set(navi, EVAS_HINT_FILL, EVAS_HINT_FILL);
   efl_content_set(efl_part(layout, "menu"), navi);

   list = elm_list_add(navi);
   efl_gfx_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   efl_gfx_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_list_select_mode_set(list, ELM_OBJECT_SELECT_MODE_ALWAYS);
   elm_naviframe_item_push(navi, "Properties", NULL, NULL, list, NULL);
   efl_gfx_entity_visible_set(list, EINA_TRUE);

   tp = efl_add(EFL_UI_TAB_PAGER_CLASS, layout,
                efl_gfx_hint_weight_set(efl_added, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND),
                efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_TRUE),
                efl_content_set(efl_part(layout, "tab_pager"), efl_added));

   tab_label_count = 0;
   tab_icon_count = 0;

   for (i = 0 ; i < 3 ; i ++)
   {
      page = tab_page_add(tp);
      efl_pack_end(tp, page);
      if (i == 0)
        efl_ui_spotlight_active_element_set(tp, page);
   }

   ad = (App_Data*)calloc(1, sizeof(App_Data));
   ad->navi = navi;
   ad->tab_pager = tp;

   elm_list_item_append(list, "Current", NULL, NULL, _current_cb, ad);
   elm_list_item_append(list, "Pack", NULL, NULL, _pack_cb, ad);
   elm_list_item_append(list, "Unpack", NULL, NULL, _unpack_cb, ad);
   elm_list_item_append(list, "Tab", NULL, NULL, _tab_cb, ad);
   elm_list_item_append(list, "Transition", NULL, NULL, _transition_cb, ad);
   elm_list_go(list);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(500, 300));
}

/* Menu Callback */

static void
_win_del_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   free(data);
}

static void
_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   elm_naviframe_item_pop(data);
}

static void
_tab_set_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Tab_Set_Data *tsd = data;
   efl_ui_spotlight_active_element_set(tsd->tab_pager, efl_pack_content_get(tsd->tab_pager, elm_spinner_value_get(tsd->spinner)));
}

static void
_tab_set_btn_del_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   free(data);
}

static void
_current_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   App_Data *ad = (App_Data *) data;
   Evas_Object *navi = ad->navi;
   Eo *tab_pager = ad->tab_pager;
   Eo *btn, *box, *sp;
   Tab_Set_Data *tsd = calloc(1, sizeof(Tab_Set_Data));

   Elm_List_Item *it = event_info;
   elm_list_item_selected_set(it, EINA_FALSE);

   btn = efl_add(EFL_UI_BUTTON_CLASS, navi,
                 efl_text_set(efl_added, "Back"),
                 efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _btn_cb, navi));

   box = efl_add(EFL_UI_BOX_CLASS, navi,
                 efl_gfx_arrangement_content_padding_set(efl_added, 10, 10, EINA_TRUE),
                 elm_naviframe_item_push(navi, "Current", btn, NULL, efl_added, NULL));

   sp = efl_add(EFL_UI_SPIN_BUTTON_CLASS, box,
                efl_ui_range_limits_set(efl_added, 0, efl_content_count(tab_pager) - 1),
                efl_ui_range_value_set(efl_added, efl_content_count(tab_pager)),
                efl_pack_end(box, efl_added));

   tsd->tab_pager = tab_pager;
   tsd->spinner = sp;

   btn = efl_add(EFL_UI_BUTTON_CLASS, box,
                 efl_text_set(efl_added, "Current Tab Page Set"),
                 efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _tab_set_btn_cb, tsd),
                 efl_event_callback_add(efl_added, EFL_EVENT_DEL, _tab_set_btn_del_cb, tsd),
                 efl_pack_end(box, efl_added));
}

static void
_pack_begin_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *tab_pager = data;
   Eo *tab_page;

   tab_page = tab_page_add(tab_pager);

   efl_pack_begin(tab_pager, tab_page);
}

static void
_pack_end_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *tab_pager = data;
   Eo *tab_page;

   tab_page = tab_page_add(tab_pager);

   efl_pack_end(tab_pager, tab_page);
}

static void
_pack_before_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *tab_pager = data;
   Eo *tab_page, *cur_tab_page;

   cur_tab_page = efl_ui_spotlight_active_element_get(tab_pager);

   tab_page = tab_page_add(tab_pager);

   efl_pack_before(tab_pager, tab_page, cur_tab_page);
}

static void
_pack_after_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *tab_pager = data;
   Eo *tab_page, *cur_tab_page;

   cur_tab_page = efl_ui_spotlight_active_element_get(tab_pager);

   tab_page = tab_page_add(tab_pager);

   efl_pack_after(tab_pager, tab_page, cur_tab_page);
}

static void
_pack_at_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Tab_Set_Data *tsd = data;
   Eo *tab_page;

   tab_page = tab_page_add(tsd->tab_pager);

   efl_pack_at(tsd->tab_pager, tab_page, elm_spinner_value_get(tsd->spinner));
   efl_ui_range_limits_set(tsd->spinner, 0, efl_content_count(tsd->tab_pager) - 1);
}

static void
_pack_at_btn_del_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   free(data);
}

static void
_pack_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   App_Data *ad = (App_Data *) data;
   Evas_Object *navi = ad->navi;
   Eo *tab_pager = ad->tab_pager;
   Eo *btn, *box, *in_box, *sp;
   Tab_Set_Data *tsd = calloc(1, sizeof(Tab_Set_Data));

   Elm_List_Item *it = event_info;
   elm_list_item_selected_set(it, EINA_FALSE);

   btn = efl_add(EFL_UI_BUTTON_CLASS, navi,
                 efl_text_set(efl_added, "Back"),
                 efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _btn_cb, navi));

   box = efl_add(EFL_UI_BOX_CLASS, navi,
                 efl_gfx_arrangement_content_padding_set(efl_added, 10, 10, EINA_TRUE),
                 elm_naviframe_item_push(navi, "Pack", btn, NULL, efl_added, NULL));

   /* Pack Begin */
   btn = efl_add(EFL_UI_BUTTON_CLASS, box,
                 efl_text_set(efl_added, "Pack Begin"),
                 efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _pack_begin_btn_cb, tab_pager),
                 efl_pack_end(box, efl_added));

   /* Pack End */
   btn = efl_add(EFL_UI_BUTTON_CLASS, box,
                 efl_text_set(efl_added, "Pack End"),
                 efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _pack_end_btn_cb, tab_pager),
                 efl_pack_end(box, efl_added));

   /* Pack Before */
   btn = efl_add(EFL_UI_BUTTON_CLASS, box,
                 efl_text_set(efl_added, "Pack Before Current Tab Page"),
                 efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _pack_before_btn_cb, tab_pager),
                 efl_pack_end(box, efl_added));

   /* Pack After */
   btn = efl_add(EFL_UI_BUTTON_CLASS, box,
                 efl_text_set(efl_added, "Pack After Current Tab Page"),
                 efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _pack_after_btn_cb, tab_pager),
                 efl_pack_end(box, efl_added));

   in_box = efl_add(EFL_UI_BOX_CLASS, box,
                    efl_gfx_arrangement_content_padding_set(efl_added, 10, 10, EINA_TRUE),
                    efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL),
                    efl_pack_end(box, efl_added));

   sp = efl_add(EFL_UI_SPIN_BUTTON_CLASS, in_box,
                efl_ui_range_limits_set(efl_added, 0, efl_content_count(tab_pager) - 1),
                efl_ui_range_value_set(efl_added, efl_content_count(tab_pager)),
                efl_pack_end(in_box, efl_added));

   tsd->tab_pager = tab_pager;
   tsd->spinner = sp;

   /* Pack At */
   btn = efl_add(EFL_UI_BUTTON_CLASS, in_box,
                 efl_text_set(efl_added, "Pack At"),
                 efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _pack_at_btn_cb, tsd),
                 efl_event_callback_add(efl_added, EFL_EVENT_DEL, _pack_at_btn_del_cb, tsd),
                 efl_pack_end(in_box, efl_added));
}

static void
_clear_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *tab_pager = data;
   efl_pack_clear(tab_pager);
}

static void
_unpack_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *tab_pager = data;

   Eo *tab_page = efl_ui_spotlight_active_element_get(tab_pager);

   efl_pack_unpack(tab_pager, tab_page);
   efl_del(tab_page);
}

static void
_unpack_all_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *tab_pager = data;
   efl_pack_unpack_all(tab_pager);
}

static void
_unpack_at_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Tab_Set_Data *tsd = data;

   efl_pack_unpack_at(tsd->tab_pager, elm_spinner_value_get(tsd->spinner));
   efl_ui_range_limits_set(tsd->spinner, 0, efl_content_count(tsd->tab_pager) - 1);
}

static void
_unpack_at_btn_del_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   free(data);
}

static void
_unpack_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   App_Data *ad = (App_Data *) data;
   Evas_Object *navi = ad->navi;
   Eo *tab_pager = ad->tab_pager;
   Eo *btn, *box, *sp, *in_box;
   Tab_Set_Data *tsd = calloc(1, sizeof(Tab_Set_Data));

   Elm_List_Item *it = event_info;
   elm_list_item_selected_set(it, EINA_FALSE);

   btn = efl_add(EFL_UI_BUTTON_CLASS, navi,
                 efl_text_set(efl_added, "Back"),
                 efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _btn_cb, navi));

   box = efl_add(EFL_UI_BOX_CLASS, navi,
                 efl_gfx_arrangement_content_padding_set(efl_added, 10, 10, EINA_TRUE),
                 elm_naviframe_item_push(navi, "Unpack", btn, NULL, efl_added, NULL));

   /* Clear */
   btn = efl_add(EFL_UI_BUTTON_CLASS, box,
                 efl_text_set(efl_added, "Clear"),
                 efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _clear_btn_cb, tab_pager),
                 efl_ui_widget_disabled_set(efl_added, EINA_TRUE), //Soon to be implemented
                 efl_pack_end(box, efl_added));

   /* Unpack */
   btn = efl_add(EFL_UI_BUTTON_CLASS, box,
                 efl_text_set(efl_added, "Unpack"),
                 efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _unpack_btn_cb, tab_pager),
                 efl_pack_end(box, efl_added));

   /* Unpack All */
   btn = efl_add(EFL_UI_BUTTON_CLASS, box,
                 efl_text_set(efl_added, "Unpack All"),
                 efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _unpack_all_btn_cb, tab_pager),
                 efl_ui_widget_disabled_set(efl_added, EINA_TRUE), //Soon to be implemented
                 efl_pack_end(box, efl_added));

   in_box = efl_add(EFL_UI_BOX_CLASS, box,
                    efl_gfx_arrangement_content_padding_set(efl_added, 10, 10, EINA_TRUE),
                    efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL),
                    efl_pack_end(box, efl_added));

   sp = efl_add(EFL_UI_SPIN_BUTTON_CLASS, in_box,
                efl_ui_range_limits_set(efl_added, 0, efl_content_count(tab_pager) - 1),
                efl_ui_range_value_set(efl_added, efl_content_count(tab_pager)),
                efl_pack_end(in_box, efl_added));

   tsd->tab_pager = tab_pager;
   tsd->spinner = sp;

   /* Unpack At */
   btn = efl_add(EFL_UI_BUTTON_CLASS, in_box,
                 efl_text_set(efl_added, "Unpack At"),
                 efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _unpack_at_btn_cb, tsd),
                 efl_event_callback_add(efl_added, EFL_EVENT_DEL, _unpack_at_btn_del_cb, tsd),
                 efl_ui_widget_disabled_set(efl_added, EINA_TRUE), //Soon to be implemented
                 efl_pack_end(in_box, efl_added));
}

static void
_change_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Tab_Change_Data *tcd = data;

   Eo *tab_page, *content;
   char *label = NULL;
   char *icon = NULL;

   tab_page = efl_ui_spotlight_active_element_get(tcd->tab_pager);

   if (efl_ui_selectable_selected_get(tcd->label_check))
   {
      label = tab_label_get();
      efl_text_set(efl_ui_tab_page_tab_bar_item_get(tab_page), label);
      content = content_add(tab_page, label);
      efl_content_set(tab_page, content);
   }

   if (efl_ui_selectable_selected_get(tcd->icon_check))
   {
      icon = tab_icon_get();
      efl_ui_tab_bar_default_item_icon_set(efl_ui_tab_page_tab_bar_item_get(tab_page), icon);
   }
}

static void
_change_btn_del_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   free(data);
}

static void
_tab_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   App_Data *ad = (App_Data *) data;
   Evas_Object *navi = ad->navi;
   Eo *tab_pager = ad->tab_pager;
   Eo *btn, *box, *label_check, *icon_check;
   Tab_Change_Data *tcd = calloc(1, sizeof(Tab_Change_Data));

   Elm_List_Item *it = event_info;
   elm_list_item_selected_set(it, EINA_FALSE);

   btn = efl_add(EFL_UI_BUTTON_CLASS, navi,
                 efl_text_set(efl_added, "Back"),
                 efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _btn_cb, navi));

   box = efl_add(EFL_UI_BOX_CLASS, navi,
                 efl_gfx_arrangement_content_padding_set(efl_added, 10, 10, EINA_TRUE),
                 elm_naviframe_item_push(navi, "Tab", btn, NULL, efl_added, NULL));

   label_check = efl_add(EFL_UI_CHECK_CLASS, box);
   efl_text_set(label_check, "Tab Label + Page");
   efl_ui_selectable_selected_set(label_check, EINA_FALSE);
   efl_pack_end(box, label_check);

   icon_check = efl_add(EFL_UI_CHECK_CLASS, box);
   efl_text_set(icon_check, "Tab Icon");
   efl_ui_selectable_selected_set(icon_check, EINA_FALSE);
   efl_pack_end(box, icon_check);

   tcd->tab_pager = tab_pager;
   tcd->label_check = label_check;
   tcd->icon_check = icon_check;

   btn = efl_add(EFL_UI_BUTTON_CLASS, box,
                 efl_text_set(efl_added, "Change"),
                 efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _change_btn_cb, tcd),
                 efl_event_callback_add(efl_added, EFL_EVENT_DEL, _change_btn_del_cb, tcd),
                 efl_pack_end(box, efl_added));
}

static void
_tran_set_btn_scroll_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Ui_Spotlight_Manager_Scroll *scroll = efl_new(EFL_UI_SPOTLIGHT_MANAGER_SCROLL_CLASS);
   efl_ui_spotlight_manager_set(data, scroll);
}

static void
_tran_set_btn_stack_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Ui_Spotlight_Manager_Scroll *stack = efl_new(EFL_UI_SPOTLIGHT_MANAGER_STACK_CLASS);
   efl_ui_spotlight_manager_set(data, stack);
}

static void
_tran_unset_btn_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   efl_ui_spotlight_manager_set(data, NULL);
}

static void
_transition_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   App_Data *ad = (App_Data *) data;
   Evas_Object *navi = ad->navi;
   Eo *tab_pager = ad->tab_pager;
   Eo *btn, *box;

   Elm_List_Item *it = event_info;
   elm_list_item_selected_set(it, EINA_FALSE);

   btn = efl_add(EFL_UI_BUTTON_CLASS, navi,
                 efl_text_set(efl_added, "Back"),
                 efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _btn_cb, navi));

   box = efl_add(EFL_UI_BOX_CLASS, navi,
                 efl_gfx_arrangement_content_padding_set(efl_added, 10, 10, EINA_TRUE),
                 elm_naviframe_item_push(navi, "Transition", btn, NULL, efl_added, NULL));

   btn = efl_add(EFL_UI_BUTTON_CLASS, box,
                 efl_text_set(efl_added, "Scroll"),
                 efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _tran_set_btn_scroll_cb, tab_pager),
                 efl_pack_end(box, efl_added));
   btn = efl_add(EFL_UI_BUTTON_CLASS, box,
                 efl_text_set(efl_added, "Stack"),
                 efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _tran_set_btn_stack_cb, tab_pager),
                 efl_pack_end(box, efl_added));

   btn = efl_add(EFL_UI_BUTTON_CLASS, box,
                 efl_text_set(efl_added, "Unset"),
                 efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _tran_unset_btn_cb, tab_pager),
                 efl_pack_end(box, efl_added));
}
