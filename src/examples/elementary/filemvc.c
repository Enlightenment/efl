// gcc -o filemvc filemvc.c `pkg-config --cflags --libs elementary`

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
#include <Eio.h>
#include <elm_view_form.h>

#define EFL_MODEL_TEST_FILENAME_PATH "/tmp"

struct _Efl_Model_Test_Filemvc_Data
{
   Eo *fileview;
   Eo *treeview;
   Eo *formview;
   Eo *treemodel;
   Evas_Object *thumb;
   char imagedefault_path[256];
};
typedef struct _Efl_Model_Test_Filemvc_Data Efl_Model_Test_Filemvc_Data;

static void
_cleanup_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Efl_Model_Test_Filemvc_Data *priv = (Efl_Model_Test_Filemvc_Data *)data;
   eo_unref(priv->fileview);
   eo_unref(priv->treeview);
   eo_unref(priv->formview);
   eo_unref(priv->treemodel);
}

static Eina_Bool
_filter_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, const Eina_File_Direct_Info *info)
{
   if (info->type == EINA_FILE_DIR && info->path[info->name_start] != '.')  return EINA_TRUE;

   return EINA_FALSE;
}

static Eina_Bool
_list_selected_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   Efl_Model_Test_Filemvc_Data *priv = data;
   Eo *child = event->info;
//   ethumb_client_file_free(elm_thumb_ethumb_client_get());

   printf("LIST selected model\n");
   elm_view_form_model_set(priv->formview, child);
   return EINA_TRUE;
}

static void
_promise_then(void *data, void *value)
{
   Efl_Model_Test_Filemvc_Data *priv = data;
   char *path;
   Eo *model;

   eina_value_get(value, &path);
   model = eo_add(EIO_MODEL_CLASS, NULL, eio_model_path_set(eo_self, path));
   elm_view_list_model_set(priv->fileview, model);
}

static void
_promise_error(void *data, const Eina_Error err)
{
}

static Eina_Bool
_tree_selected_cb(void *data, const Eo_Event *event)
{
   Efl_Model_Test_Filemvc_Data *priv = data;
   Eo *child = event->info;
   Eina_Promise *promise;

   printf("TREE selected model\n");

   promise= efl_model_property_get(child, "path");
   eina_promise_then(promise, &_promise_then, &_promise_error, priv);
   return EINA_TRUE;
}

static void
_widget_init(Evas_Object *widget)
{
   evas_object_size_hint_weight_set(widget, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(widget, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(widget);
}

Evas_Object *
_label_init(Evas_Object *win, Evas_Object *box, const char *text)
{
   Evas_Object *widget = elm_label_add(win);
   elm_label_line_wrap_set(widget, ELM_WRAP_CHAR);
   elm_object_text_set(widget, text);
   elm_box_pack_end(box, widget);
   evas_object_size_hint_weight_set(widget, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_align_set(widget, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(widget);

   return widget;
}


static void
_thumb_error_cb(void *data, Evas_Object *o EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Efl_Model_Test_Filemvc_Data *priv = data;
   printf("thumbnail generation error.\n");
   elm_thumb_file_set(priv->thumb, priv->imagedefault_path, NULL);
   elm_thumb_reload(priv->thumb);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Efl_Model_Test_Filemvc_Data priv;
   Evas_Object *win, *panes, *bxr, *genlist, *vpanes;
   Evas_Object *entry;
   char *dirname;

   memset(&priv, 0, sizeof(Efl_Model_Test_Filemvc_Data));
   elm_app_info_set(elm_main, "elementary", "images/logo.png");
   sprintf(priv.imagedefault_path, "%s/images/logo.png", elm_app_data_dir_get());

   //win
   win = elm_win_util_standard_add("viewlist", "Viewlist");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_win_autodel_set(win, EINA_TRUE);

   panes = elm_panes_add(win);
   evas_object_size_hint_weight_set(panes, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, panes);

   if(argv[1] != NULL) dirname = argv[1];
   else dirname = EFL_MODEL_TEST_FILENAME_PATH;

   //treemodel
   priv.treemodel = eo_add(EIO_MODEL_CLASS, NULL, eio_model_path_set(eo_self, dirname));
   eio_model_children_filter_set(priv.treemodel, _filter_cb, NULL);

   //treeview
   genlist = elm_genlist_add(win);
   priv.treeview = eo_add(ELM_VIEW_LIST_CLASS, NULL, elm_view_list_genlist_set(eo_self, genlist, ELM_GENLIST_ITEM_TREE, NULL));
   elm_view_list_property_connect(priv.treeview, "filename", "elm.text");
   elm_view_list_property_connect(priv.treeview, "icon", "elm.swallow.icon");
   elm_view_list_model_set(priv.treeview, priv.treemodel);
   _widget_init(genlist);
   elm_object_part_content_set(panes, "left", genlist);
   elm_obj_panes_content_left_size_set(panes, 0.3);

   vpanes = elm_panes_add(win);
   _widget_init(vpanes);
   elm_object_part_content_set(panes, "right", vpanes);
   eo_event_callback_add(priv.treeview, ELM_VIEW_LIST_EVENT_MODEL_SELECTED, _tree_selected_cb, &priv);

   //listview
   genlist = elm_genlist_add(win);
   priv.fileview = eo_add(ELM_VIEW_LIST_CLASS, NULL, elm_view_list_genlist_set(eo_self, genlist, ELM_GENLIST_ITEM_NONE, "double_label"));
   elm_view_list_property_connect(priv.fileview, "filename", "elm.text");
   elm_view_list_property_connect(priv.fileview, "mime_type", "elm.text.sub");

   evas_object_event_callback_add(win, EVAS_CALLBACK_DEL, _cleanup_cb, &priv);
   _widget_init(genlist);
   elm_object_part_content_set(vpanes, "left", genlist);

   eo_event_callback_add(priv.fileview, ELM_VIEW_LIST_EVENT_MODEL_SELECTED, _list_selected_cb, &priv);

   //formview
   bxr = elm_box_add(win);
   _widget_init(bxr);
   elm_object_part_content_set(vpanes, "right", bxr);
   priv.formview = eo_add(ELM_VIEW_FORM_CLASS, NULL);

   /*Label widget */
   elm_view_form_widget_add(priv.formview, "filename", _label_init(win, bxr, "File Name"));

    _label_init(win, bxr, "Size:");
   elm_view_form_widget_add(priv.formview, "size", _label_init(win, bxr, ""));

    _label_init(win, bxr, "Modified:");
   elm_view_form_widget_add(priv.formview, "mtime", _label_init(win, bxr, ""));

   /* Entry widget */
   entry = elm_entry_add(win);
   elm_entry_single_line_set(entry, EINA_TRUE);
   evas_object_size_hint_weight_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bxr, entry);
   evas_object_show(entry);
   elm_view_form_widget_add(priv.formview, "path", entry);

   /* Thumb widget */
   elm_need_ethumb();
   priv.thumb = elm_thumb_add(win);
   _widget_init(priv.thumb);
   elm_box_pack_end(bxr, priv.thumb);
   elm_thumb_editable_set(priv.thumb, EINA_FALSE);
   elm_view_form_widget_add(priv.formview, "path", priv.thumb);
   evas_object_smart_callback_add(priv.thumb, "generate,error", _thumb_error_cb, &priv);
   evas_object_smart_callback_add(priv.thumb, "load,error", _thumb_error_cb, &priv);

   //showall
   evas_object_resize(win, 800, 400);
   evas_object_show(panes);
   evas_object_show(win);

   elm_run();
   elm_shutdown();
   ecore_shutdown();

   return 0;
}
ELM_MAIN()
