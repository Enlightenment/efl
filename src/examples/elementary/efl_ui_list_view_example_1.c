// gcc -o efl_ui_list_view_example_1 efl_ui_list_view_example_1.c `pkg-config --cflags --libs elementary`

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

static void
_realized_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Gfx_Entity *layout = event->info;

   elm_object_focus_allow_set(layout, EINA_TRUE);
}

static Efl_Model*
_make_model(Evas_Object *win)
{
   Eina_Value vtext;
   Efl_Generic_Model *model, *child;
   unsigned int i;
   char buf[256];

   model = efl_add(EFL_GENERIC_MODEL_CLASS, win);
   eina_value_setup(&vtext, EINA_VALUE_TYPE_STRING);

   for (i = 0; i < (NUM_ITEMS); i++)
     {
        child = efl_model_child_add(model);

        snprintf(buf, sizeof(buf), "Item # %i", i);
        eina_value_set(&vtext, buf);
        efl_model_property_set(child, "title", &vtext);
     }

   eina_value_flush(&vtext);
   return model;
}

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Efl_Ui_Factory *factory;
   Evas_Object *win, *li;
   Eo *model;
   Efl_Ui_Select_Model *selmodel;

   win = elm_win_util_standard_add("list_view", "List_View");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   elm_win_autodel_set(win, EINA_TRUE);

   model = _make_model(win);
   selmodel = efl_add(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get()
     , efl_ui_view_model_set(efl_added, model)
   );

   factory = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, win);
   efl_ui_widget_factory_item_class_set(factory, EFL_UI_LIST_DEFAULT_ITEM_CLASS);
   efl_ui_property_bind(factory, "efl.text", "title");

   li = efl_add(EFL_UI_LIST_VIEW_CLASS, win,
                efl_ui_collection_view_factory_set(efl_added, factory),
                efl_ui_view_model_set(efl_added, selmodel));

   efl_event_callback_add(li, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_REALIZED, _realized_cb, NULL);

   elm_win_resize_object_add(win, li);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);

   //showall
   evas_object_show(li);
   evas_object_resize(win, 320, 320);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
