//  gcc -o efl_ui_collection_view_example_1 efl_ui_collection_view_example_1.c `pkg-config --cflags --libs efl-ui

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

static void
_item_constructing(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Efl_Gfx_Entity *item = ev->info;

   if (!efl_ui_item_calc_locked_get(item))
     efl_gfx_hint_size_min_set(item, EINA_SIZE2D(50, 50));
}

EAPI_MAIN void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Efl_Ui_Factory *factory;
   Evas_Object *win, *li;
   Eo *model;
   Eo *position_manager;
   Efl_App *app = ev->object;
   Eina_Accessor *ac;
   Eina_Bool list = EINA_TRUE, multi = EINA_FALSE, none = EINA_FALSE;
   Efl_Ui_Select_Mode mode = EFL_UI_SELECT_MODE_SINGLE;
   const char *arg;
   unsigned int i;

   ac = efl_core_command_line_command_access(app);
   EINA_ACCESSOR_FOREACH(ac, i, arg)
     {
        if (eina_streq(arg, "grid")) list = EINA_FALSE;
        if (eina_streq(arg, "multi")) multi = EINA_TRUE;
        if (eina_streq(arg, "none")) none = EINA_TRUE;
     }
   eina_accessor_free(ac);

   if (multi) mode = EFL_UI_SELECT_MODE_MULTI;
   if (none) mode = EFL_UI_SELECT_MODE_NONE;

   win = efl_add(EFL_UI_WIN_CLASS, app,
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
                 efl_ui_win_autohide_set(efl_added, EINA_TRUE));
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);
   model = _make_model(win);

   factory = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, win);
   efl_ui_property_bind(factory, "text", "title");

   if (list)
     {
        position_manager = efl_new(EFL_UI_POSITION_MANAGER_LIST_CLASS);
        efl_ui_widget_factory_item_class_set(factory, EFL_UI_LIST_DEFAULT_ITEM_CLASS);
     }
   else
     {
        position_manager = efl_new(EFL_UI_POSITION_MANAGER_GRID_CLASS);
        efl_ui_widget_factory_item_class_set(factory, EFL_UI_GRID_DEFAULT_ITEM_CLASS);
        efl_event_callback_add(factory, EFL_UI_FACTORY_EVENT_ITEM_CONSTRUCTING, _item_constructing, NULL);
     }

   li = efl_add(EFL_UI_COLLECTION_VIEW_CLASS, win,
                efl_ui_collection_view_position_manager_set(efl_added, position_manager),
                efl_ui_view_model_set(efl_added, model),
                efl_ui_multi_selectable_select_mode_set(efl_added, mode),
                efl_ui_collection_view_factory_set(efl_added, factory));

   efl_content_set(win, li);

   //showall
   efl_gfx_entity_size_set(win, EINA_SIZE2D(320, 320));
}
EFL_MAIN()
