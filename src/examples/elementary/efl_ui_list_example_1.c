// gcc -o efl_ui_list_example_1 efl_ui_list_example_1.c `pkg-config --cflags --libs elementary`
// ./efl_ui_list_example_1 item_style_name

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define EFL_BETA_API_SUPPORT 1
#endif

#include <Efl_Ui.h>
#include <Elementary.h>
#include <Efl.h>
#include <Eio.h>
#include <stdio.h>

#define NUM_ITEMS 400

typedef struct _List_Scroll_Data {
  Eo *list;
  Eo *slider;
  Eina_Bool anim;
} List_Scroll_Data;
List_Scroll_Data priv_d;


static void
_list_selected(void *data EINA_UNUSED, const Efl_Event *ev)
{
  Eo *list = ev->object;
  Eo *item = ev->info, *tmp;
  printf("list item [%p:%d] is %s\n", item, efl_ui_item_index_get(item), (efl_ui_item_selected_get(item)? "selected" : "unselected"));

  Eina_Iterator *selects = efl_ui_list_selected_items_get(list);

  EINA_ITERATOR_FOREACH(selects, tmp)
     printf("selected [%p:%d] ", tmp, efl_ui_item_index_get(tmp));
  printf("\n");
  eina_iterator_free(selects);
}

static void
_list_unselected(void *data EINA_UNUSED, const Efl_Event *ev)
{
  Eo *item = ev->info;
  printf("list item [%p : %d] is %s\n", item, efl_ui_item_index_get(item), (efl_ui_item_selected_get(item)? "selected" : "unselected"));
}

static void
_list_pressed(void *data EINA_UNUSED, const Efl_Event *ev)
{
  Eo *item = ev->info;
  printf("list item [%p : %d] is pressed\n", item, efl_ui_item_index_get(item));
}

static void
_list_unpressed(void *data EINA_UNUSED, const Efl_Event *ev)
{
  Eo *item = ev->info;
  printf("list item [%p : %d] is unpressed\n", item, efl_ui_item_index_get(item));
}

static void
_list_longpressed(void *data EINA_UNUSED, const Efl_Event *ev)
{
  Eo *item = ev->info;
  printf("list item [%p : %d] is longpressed\n", item, efl_ui_item_index_get(item));
}

static void
_select_radio_changed(void *data, const Efl_Event *ev)
{
  Eo *list = data;
  efl_ui_select_mode_set(list, efl_ui_radio_state_value_get(ev->object));
}

static void
_anim_radio_changed(void *data EINA_UNUSED, const Efl_Event *ev)
{
  priv_d.anim = efl_ui_radio_state_value_get(ev->object);
}

static void
_scrl_btn_clicked(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
  Efl_Ui_List_Item *item = efl_ui_list_last_selected_item_get(priv_d.list);
  printf("show [%d:%p] [%d]\n", efl_ui_item_index_get(item), item, priv_d.anim);
  efl_ui_list_item_scroll(priv_d.list, item, priv_d.anim);
}

static void
_scrl_align_btn_clicked(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
  Efl_Ui_List_Item *item = efl_ui_list_last_selected_item_get(priv_d.list);
  double align = efl_ui_range_value_get(priv_d.slider);
  printf("show [%d:%p] [%.2lf], [%d]\n", efl_ui_item_index_get(item), item, align, priv_d.anim);
  efl_ui_list_item_scroll_align(priv_d.list, item, align, priv_d.anim);
}

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv)
{
   Eo *win, *list;
   Eo *wbox, *ibox, *bbox, *rbox;
   Eo *item, *check, *txt;
   Eo *radio, *select_radio, *anim_radio;
   Eo *slider, *scrl_btn;
   int i;
   char buf[256];
   Eina_Bool emptystyle = EINA_FALSE;

   win = elm_win_util_standard_add("list", "list");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_win_autodel_set(win, EINA_TRUE);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(400, 800));

   wbox = efl_add(EFL_UI_BOX_CLASS, win);
   efl_ui_direction_set(wbox, EFL_UI_DIR_VERTICAL);
   efl_gfx_hint_weight_set(wbox, EFL_GFX_HINT_EXPAND, EFL_GFX_HINT_EXPAND);
   efl_gfx_hint_fill_set(wbox, EINA_TRUE, EINA_TRUE);

   if ((argv[1] != NULL) &&
       (!strcmp(argv[1], "empty")))
     emptystyle = EINA_TRUE;

   priv_d.list = list = efl_add(EFL_UI_LIST_CLASS, wbox);
   efl_gfx_hint_weight_set(list, EFL_GFX_HINT_EXPAND, 0.9);

   efl_event_callback_add(list, EFL_UI_EVENT_ITEM_SELECTED, _list_selected, NULL);
   efl_event_callback_add(list, EFL_UI_EVENT_ITEM_UNSELECTED, _list_unselected, NULL);
   efl_event_callback_add(list, EFL_UI_EVENT_PRESSED, _list_pressed, NULL);
   efl_event_callback_add(list, EFL_UI_EVENT_UNPRESSED, _list_unpressed, NULL);
   efl_event_callback_add(list, EFL_UI_EVENT_LONGPRESSED, _list_longpressed, NULL);

   for (i = 0; i < NUM_ITEMS; i++)
     {
        if (emptystyle) item = efl_add(EFL_UI_LIST_EMPTY_ITEM_CLASS, list);
        else item = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, list);

        /* disable case test */
        if (i == 1) efl_ui_widget_disabled_set(item, EINA_TRUE);

        if (emptystyle)
          {
             ibox = efl_add(EFL_UI_BOX_CLASS, item);
             efl_ui_direction_set(ibox, EFL_UI_DIR_HORIZONTAL);

             txt = efl_add(EFL_UI_TEXT_CLASS, ibox);
             efl_gfx_hint_weight_set(txt, 0.95, EFL_GFX_HINT_EXPAND);
             efl_text_halign_set(txt, 0.2);
             efl_text_interactive_selection_allowed_set(txt, EINA_FALSE);
             snprintf(buf, sizeof(buf), "empty style item [%d]", i);
             efl_text_set(txt, buf);
             efl_pack_end(ibox, txt);

             check = efl_add(EFL_UI_CHECK_CLASS, ibox);
             efl_gfx_hint_weight_set(check, 0.05, EFL_GFX_HINT_EXPAND);
             efl_pack_end(ibox, check);

             if (i % 2)
               efl_content_set(item, ibox);
             else
               efl_content_set(efl_part(item, "content"), ibox);
          }
        else
          {
             snprintf(buf, sizeof(buf), "default style item [%d]", i);
             if (i % 2)
               efl_text_set(efl_part(item, "text"), buf);
             else
               efl_text_set(item, buf);


             check = efl_add(EFL_UI_CHECK_CLASS, item);
             if ((i % 3) == 0)
               efl_content_set(efl_part(item, "end"), check);
             else if ((i % 3) == 1)
               efl_content_set(efl_part(item, "icon"), check);
             else
               efl_content_set(item, check);
          }
        /* Pack APIs test */
        switch (i)
          {
           case 10:
             efl_gfx_color_set(efl_part(item, "background"), 255, 0, 0, 255);
             efl_pack_begin(list, item);
             break;

           case 20:
             efl_gfx_color_set(efl_part(item, "background"), 0, 255, 0, 255);
             efl_pack_before(list, item, efl_pack_content_get(list, 19));
             break;

           case 30:
             efl_gfx_color_set(efl_part(item, "background"), 0, 0, 255, 255);
             efl_pack_after(list, item, efl_pack_content_get(list, 29));
             break;

           case 40:
             efl_file_simple_load(efl_part(item, "background"), "./sky_01.jpg", NULL);
             efl_gfx_image_scale_type_set(efl_part(item, "background"), EFL_GFX_IMAGE_SCALE_TYPE_FIT_OUTSIDE);
             efl_pack_at(list, item, 39);
             break;

           case 50:
             efl_file_simple_load(efl_part(item, "background"), "./sky_01.jpg", NULL);
             efl_gfx_image_scale_type_set(efl_part(item, "background"), EFL_GFX_IMAGE_SCALE_TYPE_TILE);
             efl_pack(list, item);
             break;

           case 60:
             efl_pack(list, item);
             efl_pack_unpack(list, item);
             efl_del(item);
             break;

           default:
             efl_pack_end(list, item);
          }
        }

   efl_pack_end(wbox, list);

   /*select mode */
   txt = efl_add(EFL_UI_TEXT_CLASS, wbox);
   efl_gfx_hint_weight_set(txt, EFL_GFX_HINT_EXPAND, 0.01);
   efl_text_halign_set(txt, 0.02);
   efl_text_interactive_selection_allowed_set(txt, EINA_FALSE);
   efl_text_set(txt, "Select Mode");
   efl_pack_end(wbox, txt);

   bbox  = efl_add(EFL_UI_BOX_CLASS, wbox);
   efl_ui_direction_set(bbox, EFL_UI_DIR_HORIZONTAL);
   efl_gfx_hint_weight_set(bbox, EFL_GFX_HINT_EXPAND, 0.05);

   select_radio  = radio = efl_add(EFL_UI_RADIO_CLASS, wbox);
   efl_text_set(radio, "SINGLE");
   efl_ui_radio_state_value_set(radio, EFL_UI_SELECT_MODE_SINGLE);
   efl_event_callback_add(radio, EFL_UI_RADIO_EVENT_CHANGED, _select_radio_changed, list);
   efl_pack_end(bbox, radio);
   radio = efl_add(EFL_UI_RADIO_CLASS, wbox);
   efl_text_set(radio, "SINGLE_ALWAYS");
   efl_ui_radio_state_value_set(radio, EFL_UI_SELECT_MODE_SINGLE_ALWAYS);
   efl_ui_radio_group_add(radio, select_radio);
   efl_event_callback_add(radio, EFL_UI_RADIO_EVENT_CHANGED, _select_radio_changed, list);
   efl_pack_end(bbox, radio);
   radio = efl_add(EFL_UI_RADIO_CLASS, wbox);
   efl_text_set(radio, "MULTI");
   efl_ui_radio_state_value_set(radio, EFL_UI_SELECT_MODE_MULTI);
   efl_ui_radio_group_add(radio, select_radio);
   efl_event_callback_add(radio, EFL_UI_RADIO_EVENT_CHANGED, _select_radio_changed, list);
   efl_pack_end(bbox, radio);
   radio = efl_add(EFL_UI_RADIO_CLASS, wbox);
   efl_text_set(radio, "NONE");
   efl_ui_radio_state_value_set(radio, EFL_UI_SELECT_MODE_NONE);
   efl_ui_radio_group_add(radio, select_radio);
   efl_event_callback_add(radio, EFL_UI_RADIO_EVENT_CHANGED, _select_radio_changed, list);
   efl_pack_end(bbox, radio);
   efl_pack_end(wbox, bbox);

  /* scroll mode */
   txt = efl_add(EFL_UI_TEXT_CLASS, wbox);
   efl_gfx_hint_weight_set(txt, EFL_GFX_HINT_EXPAND, 0.01);
   efl_text_interactive_selection_allowed_set(txt, EINA_FALSE);
   efl_text_halign_set(txt, 0.02);
   efl_text_set(txt, "Item Scroll");
   efl_pack_end(wbox, txt);

   priv_d.slider = slider  = efl_add(EFL_UI_SLIDER_CLASS, wbox);
   efl_ui_direction_set(slider, EFL_UI_DIR_HORIZONTAL);
   efl_gfx_hint_weight_set(slider, 0.0, 0.05);
   efl_gfx_hint_align_set(slider, 0.5, 0.5);
   efl_gfx_hint_size_min_set(slider, EINA_SIZE2D(380, 20));
   efl_ui_range_min_max_set(slider, 0.0, 1.0);
   efl_pack_end(wbox, slider);

   bbox  = efl_add(EFL_UI_BOX_CLASS, wbox);
   efl_ui_direction_set(bbox, EFL_UI_DIR_HORIZONTAL);
   efl_gfx_hint_weight_set(bbox, EFL_GFX_HINT_EXPAND, 0.05);

   rbox  = efl_add(EFL_UI_BOX_CLASS, bbox);
   efl_ui_direction_set(rbox, EFL_UI_DIR_VERTICAL);

   anim_radio  = radio = efl_add(EFL_UI_RADIO_CLASS, rbox);
   efl_gfx_hint_align_set(radio, 0.5, 0.5);
   efl_text_set(radio, "ANIMATION OFF");
   efl_ui_radio_state_value_set(radio, 0);
   efl_event_callback_add(radio, EFL_UI_RADIO_EVENT_CHANGED, _anim_radio_changed, NULL);
   efl_pack_end(rbox, radio);
   radio = efl_add(EFL_UI_RADIO_CLASS, rbox);
   efl_gfx_hint_align_set(radio, 0.5, 0.5);
   efl_text_set(radio, "ANIMATION ON");
   efl_ui_radio_state_value_set(radio, 1);
   efl_ui_radio_group_add(radio, anim_radio);
   efl_event_callback_add(radio, EFL_UI_RADIO_EVENT_CHANGED, _anim_radio_changed, NULL);
   efl_pack_end(rbox, radio);
   efl_pack_end(bbox, rbox);

   rbox  = efl_add(EFL_UI_BOX_CLASS, bbox);
   efl_ui_direction_set(rbox, EFL_UI_DIR_VERTICAL);

   scrl_btn  = efl_add(EFL_UI_BUTTON_CLASS, rbox);
   efl_text_set(scrl_btn, "Scroll Item");
   efl_gfx_hint_align_set(scrl_btn, 0.5, 0.5);
   efl_gfx_hint_size_min_set(scrl_btn, EINA_SIZE2D(200, 25));
   efl_event_callback_add(scrl_btn, EFL_UI_EVENT_CLICKED, _scrl_btn_clicked, NULL);
   efl_pack_end(rbox, scrl_btn);

   scrl_btn  = efl_add(EFL_UI_BUTTON_CLASS, rbox);
   efl_text_set(scrl_btn, "Scroll Item Align");
   efl_gfx_hint_align_set(scrl_btn, 0.5, 0.5);
   efl_gfx_hint_size_min_set(scrl_btn, EINA_SIZE2D(200, 25));
   efl_event_callback_add(scrl_btn, EFL_UI_EVENT_CLICKED, _scrl_align_btn_clicked, NULL);
   efl_pack_end(rbox, scrl_btn);

   efl_pack_end(bbox, rbox);
   efl_pack_end(wbox, bbox);

   elm_win_resize_object_add(win, wbox);
   //window show
   efl_gfx_entity_visible_set(win, EINA_TRUE);

   elm_run();

   return 0;
}
ELM_MAIN()
