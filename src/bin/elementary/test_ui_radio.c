#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>

const char *countries[] =
{
  "Germany",
  "USA",
  "France",
  "Korea",
  "UK",
  "Romania",
  "Italy",
  NULL,
};
#define MAX_INDEX 8

static void
_check_button_selection_changed_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   if (efl_ui_selectable_selected_get(ev->object))
     printf("Object %p is now selected\n", ev->object);
   else
     printf("Object %p is now unselected\n", ev->object);
}

static Eina_Array*
create_radios(Efl_Ui_Win *win)
{
   Eina_Array *arr = eina_array_new(5);

   for (unsigned int i = 0; countries[i]; ++i)
     {
        Efl_Ui_Radio *rbtn = efl_add(EFL_UI_RADIO_CLASS, win);
        efl_ui_radio_state_value_set(rbtn, i);
        efl_text_set(rbtn, countries[i]);
        efl_event_callback_add(rbtn, EFL_UI_EVENT_SELECTED_CHANGED, _check_button_selection_changed_cb, NULL);
        eina_array_push(arr, rbtn);
     }

   return arr;
}

static void
_value_changed_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Efl_Ui_Radio_Group *g = ev->object;
   int index = efl_ui_radio_group_selected_value_get(g);
   if (index == -1)
     {
        printf("Nothing is selected anymore\n");
     }
   else
     {
        EINA_SAFETY_ON_FALSE_RETURN((index >= 0) && index < MAX_INDEX);
        printf("Now selected value %s\n", countries[index]);
     }

}

static void
_select_btn_clicked(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Ui_Check *c = data;

   efl_ui_selectable_selected_set(c, EINA_TRUE);
}

static void
_set_selected_btn_clicked(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Ui_Radio_Group *group = data;

   efl_ui_radio_group_selected_value_set(group, 0);
}

static void
_set_fallback_radio_btn_clicked(void *data, const Efl_Event *ev EINA_UNUSED)
{
   if (!efl_ui_selectable_fallback_selection_get(data))
     efl_ui_selectable_fallback_selection_set(data, efl_pack_content_get(data, 4));
   else
     efl_ui_selectable_fallback_selection_set(data, NULL);
}

void test_efl_ui_radio(void *data EINA_UNUSED,
                                   Eo *obj EINA_UNUSED,
                                   void *event_info EINA_UNUSED)
{
   Efl_Ui_Win *win;
   Efl_Ui_Table *table;
   Efl_Ui_Box *bx;
   Eina_Array *arr;
   Efl_Ui_Button *o;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                                  efl_text_set(efl_added, "Efl.Ui.Radio_Box"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));
   table = efl_add(EFL_UI_TABLE_CLASS, win);
   efl_content_set(win, table);

   bx = efl_add(EFL_UI_RADIO_BOX_CLASS, table);
   efl_pack_table(table, bx, 0, 0, 1, 3);
   efl_event_callback_add(bx, EFL_UI_RADIO_GROUP_EVENT_VALUE_CHANGED, _value_changed_cb, NULL);

   arr = create_radios(win);
   for (unsigned int i = 0; i < eina_array_count(arr); ++i)
     {
        Efl_Ui_Radio *r = eina_array_data_get(arr, i);
        efl_pack_end(bx, r);
     }

   o = efl_add(EFL_UI_BUTTON_CLASS, table);
   efl_pack_table(table, o, 1, 0, 1, 1);
   efl_text_set(o, "Selected France check");
   efl_event_callback_add(o, EFL_INPUT_EVENT_CLICKED, _select_btn_clicked, eina_array_data_get(arr, 2));

   o = efl_add(EFL_UI_BUTTON_CLASS, table);
   efl_pack_table(table, o, 1, 1, 1, 1);
   efl_text_set(o, "Set value for Germany");
   efl_event_callback_add(o, EFL_INPUT_EVENT_CLICKED, _set_selected_btn_clicked, bx);

   o = efl_add(EFL_UI_BUTTON_CLASS, table);
   efl_pack_table(table, o, 1, 2, 1, 1);
   efl_text_set(o, "Fallback set to UK");
   efl_event_callback_add(o, EFL_INPUT_EVENT_CLICKED, _set_fallback_radio_btn_clicked, bx);

   eina_array_free(arr);
}
