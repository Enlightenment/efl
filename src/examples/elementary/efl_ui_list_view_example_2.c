// gcc -o efl_ui_list_view_example_2 efl_ui_list_view_example_2.c `pkg-config --cflags --libs elementary`

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

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Efl_Ui_Layout_Factory *factory;
   Efl_Ui_Image_Factory *imgf;
   Evas_Object *win;
   Eo *model, *li;
   char *dirname;

   win = elm_win_util_standard_add("list_view2", "List_View2");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   elm_win_autodel_set(win, EINA_TRUE);

   if (argv[1] != NULL) dirname = argv[1];
   else dirname = EFL_MODEL_TEST_FILENAME_PATH;

   model = efl_add(EFL_IO_MODEL_CLASS, win, efl_io_model_path_set(efl_added, dirname));
   factory = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, win);
   efl_ui_property_bind(factory, "efl.text", "filename");
   efl_ui_layout_factory_theme_config(factory, "list_item", NULL, "default");

   li = efl_add(EFL_UI_LIST_VIEW_CLASS, win);
   efl_ui_list_view_layout_factory_set(li, factory);
   efl_ui_view_model_set(li, model);

   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);


   imgf = efl_add(EFL_UI_IMAGE_FACTORY_CLASS, win);
   efl_ui_property_bind(imgf, "", "path"); //connect to "path" property
   efl_ui_factory_bind(factory, "efl.icon", imgf);

   elm_win_resize_object_add(win, li);

   //showall
   evas_object_show(li);
   evas_object_resize(win, 320, 320);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
