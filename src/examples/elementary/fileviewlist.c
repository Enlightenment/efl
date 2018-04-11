//Compile with:
// gcc -o fileviewlist fileviewlist.c `pkg-config --cflags --libs elementary`

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define EFL_BETA_API_SUPPORT 1
# define EFL_EO_API_SUPPORT 1
#endif

#include <Elementary.h>
#include <Efl.h>
#include <Eio.h>
#include <eio_model.eo.h>
#include <stdio.h>

#define EFL_MODEL_TEST_FILENAME_PATH "/tmp"

struct _Efl_Model_Test_Fileview_Data
{
   Eo *filemodel;
   Eo *fileview;
};
typedef struct _Efl_Model_Test_Fileview_Data Efl_Model_Test_Fileview_Data;

static void
_cleanup_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Efl_Model_Test_Fileview_Data *priv = (Efl_Model_Test_Fileview_Data *)data;
   efl_unref(priv->fileview);
   efl_unref(priv->filemodel);
}

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Efl_Model_Test_Fileview_Data priv;
   Evas_Object *win = NULL;
   Evas_Object *genlist = NULL;
   char *dirname;

   memset(&priv, 0, sizeof(Efl_Model_Test_Fileview_Data));

   ecore_init();

   if(argv[1] != NULL) dirname = argv[1];
   else dirname = EFL_MODEL_TEST_FILENAME_PATH;

   win = elm_win_util_standard_add("viewlist", "Viewlist");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_win_autodel_set(win, EINA_TRUE);

   genlist = elm_genlist_add(win);
   evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(genlist);

   priv.filemodel = efl_add_ref(EIO_MODEL_CLASS, NULL, eio_model_path_set(efl_added, dirname));
   priv.fileview = efl_add_ref(ELM_VIEW_LIST_CLASS, NULL, elm_view_list_genlist_set(efl_added, genlist, ELM_GENLIST_ITEM_TREE, "double_label"));
   elm_view_list_model_set(priv.fileview, priv.filemodel);
   evas_object_event_callback_add(win, EVAS_CALLBACK_DEL, _cleanup_cb, &priv);

   elm_view_list_property_connect(priv.fileview, "filename", "elm.text");
   elm_view_list_property_connect(priv.fileview, "mtime", "elm.text.sub");

   evas_object_resize(win, 320, 520);
   elm_win_resize_object_add(win, genlist);
   evas_object_show(win);

   elm_run();

   elm_shutdown();
   ecore_shutdown();

   return 0;
}
ELM_MAIN()

