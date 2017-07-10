// gcc -o efl_ui_list_example_2 efl_ui_list_example_2.c `pkg-config --cflags --libs elementary`

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

struct _Priv_Data
{
   Eo *model;
   Evas_Object *li;
   Eo *imf;
};
typedef struct _Priv_Data Priv_Data;

static void
_cleanup_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Priv_Data *pd = data;
   efl_unref(pd->li);
   efl_unref(pd->model);
}

static void
_realized_cb(void *data, const Efl_Event *event)
{
   Efl_Ui_List_Item_Event *ie = event->info;
   Priv_Data *pd = data;
   elm_layout_theme_set(ie->layout, "list", "item", "default");
   printf("realize %d\n", ie->index);

   evas_object_size_hint_weight_set(ie->layout, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(ie->layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

   efl_ui_model_connect(ie->layout, "elm.text", "filename");
   efl_ui_model_factory_connect(ie->layout, "elm.swallow.icon", pd->imf);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Priv_Data *pd;
   Evas_Object *win;
   char *dirname;

   pd = alloca(sizeof(Priv_Data));

   win = elm_win_util_standard_add("viewlist", "Viewlist");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   elm_win_autodel_set(win, EINA_TRUE);

   if (argv[1] != NULL) dirname = argv[1];
   else dirname = EFL_MODEL_TEST_FILENAME_PATH;

   pd->model = efl_add(EIO_MODEL_CLASS, NULL, eio_model_path_set(efl_added, dirname));

   pd->li = efl_add(EFL_UI_LIST_CLASS, win, efl_ui_view_model_set(efl_added, pd->model));
   efl_event_callback_add(pd->li, EFL_UI_LIST_EVENT_ITEM_REALIZED, _realized_cb, pd);

   evas_object_size_hint_weight_set(pd->li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(pd->li, EVAS_HINT_FILL, EVAS_HINT_FILL);


   pd->imf = efl_add(EFL_UI_IMAGE_FACTORY_CLASS, win);
   efl_ui_model_connect(pd->imf, "", "path"); //connect to "path" property

   evas_object_event_callback_add(win, EVAS_CALLBACK_DEL, _cleanup_cb, pd);

   elm_win_resize_object_add(win, pd->li);


   //showall
   evas_object_show(pd->li);
   evas_object_resize(win, 320, 320);
   evas_object_show(win);

   elm_run();
   elm_shutdown();
   ecore_shutdown();

   return 0;
}
ELM_MAIN()
