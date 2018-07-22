// gcc -o efl_ui_view_list_example_2 efl_ui_view_list_example_2.c `pkg-config --cflags --libs elementary`

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define EFL_BETA_API_SUPPORT 1
# define EFL_EO_API_SUPPORT 1
#endif

#include <Elementary.h>
#include <Efl.h>
#include <Eio.h>
#include <stdio.h>

#define EFL_MODEL_TEST_FILENAME_PATH "/tmp"

static void
_realized_cb(void *data, const Efl_Event *event)
{
   Efl_Ui_View_List_Item_Event *ie = event->info;
   Eo *imf = data;
   printf("realize %d\n", ie->index);

   evas_object_size_hint_weight_set(ie->layout, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(ie->layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

   efl_ui_factory_model_connect(ie->layout, "efl.icon", imf);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Efl_Ui_Layout_Factory *factory;
   Evas_Object *win;
   Eo *imf, *model, *li;
   char *dirname;

   win = elm_win_util_standard_add("viewlist", "Viewlist");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   elm_win_autodel_set(win, EINA_TRUE);

   if (argv[1] != NULL) dirname = argv[1];
   else dirname = EFL_MODEL_TEST_FILENAME_PATH;

   model = efl_add(EIO_MODEL_CLASS, win, eio_model_path_set(efl_added, dirname));
   factory = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, win);
   efl_ui_model_connect(factory, "efl.text", "filename");
   efl_ui_layout_factory_theme_config(factory, "list_item", NULL, "default");

   li = efl_add(EFL_UI_VIEW_LIST_CLASS, win);
   efl_ui_view_list_layout_factory_set(li, factory);
   efl_ui_view_model_set(li, model);

   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);


   imf = efl_add(EFL_UI_IMAGE_FACTORY_CLASS, win);
   efl_ui_model_connect(imf, "", "path"); //connect to "path" property
   efl_event_callback_add(li, EFL_UI_VIEW_LIST_EVENT_ITEM_REALIZED, _realized_cb, imf);

   elm_win_resize_object_add(win, li);

   //showall
   evas_object_show(li);
   evas_object_resize(win, 320, 320);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
