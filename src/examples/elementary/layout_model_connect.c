// gcc -o layout_model_connect layout_model_connect.c `pkg-config --cflags --libs elementary`

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

struct _Layout_Model_Data
{
   Eo *fileview;
   Eo *model;
   Evas_Object *label;
   Evas_Object *entry;
   Evas_Object *img;
   Evas_Object *bt;
};
typedef struct _Layout_Model_Data Layout_Model_Data;

static void
_cleanup_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Layout_Model_Data *priv = (Layout_Model_Data*)data;
   efl_unref(priv->fileview);
   efl_unref(priv->model);
}

static void
_list_selected_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Layout_Model_Data *priv = data;
   Eo *child = event->info;

   printf("LIST selected model\n");
   efl_ui_view_model_set(priv->label, child);
   efl_ui_view_model_set(priv->entry, child);
   efl_ui_view_model_set(priv->img, child);
   efl_ui_view_model_set(priv->bt, child);
}


static void
_update_cb(void *data, Evas_Object *obj EINA_UNUSED, void *ev EINA_UNUSED)
{
   Layout_Model_Data *priv = data;

   const char *text = elm_object_text_get(priv->entry);
   elm_layout_text_set(priv->label, "default", text);
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
_signal_cb(void *data EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           const char *emission,
           const char *source)
{
   printf(">> Signal callback emission='%s' source='%s'\n", emission, source);
}

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Layout_Model_Data *priv;
   Evas_Object *win, *panes, *bxr, *genlist;
   Eo *img_factory;
   char *dirname;

   priv = alloca(sizeof(Layout_Model_Data));
   memset(priv, 0, sizeof(Layout_Model_Data));

   //win
   win = elm_win_util_standard_add("viewlist", "Viewlist");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_win_autodel_set(win, EINA_TRUE);

   panes = elm_panes_add(win);
   evas_object_size_hint_weight_set(panes, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, panes);

   if (argv[1] != NULL) dirname = argv[1];
   else dirname = EFL_MODEL_TEST_FILENAME_PATH;

   priv->model = efl_add(EIO_MODEL_CLASS, NULL, eio_model_path_set(efl_added, dirname));

   genlist = elm_genlist_add(win);
   priv->fileview = efl_add(ELM_VIEW_LIST_CLASS, NULL, elm_view_list_genlist_set(efl_added, genlist, ELM_GENLIST_ITEM_NONE, NULL));
   elm_view_list_property_connect(priv->fileview, "filename", "elm.text");
   elm_view_list_model_set(priv->fileview, priv->model);
   _widget_init(genlist);
   elm_object_part_content_set(panes, "left", genlist);
   elm_obj_panes_content_left_size_set(panes, 0.3);
   efl_event_callback_add(priv->fileview, ELM_VIEW_LIST_EVENT_MODEL_SELECTED, _list_selected_cb, priv);

   bxr = elm_box_add(win);
   _widget_init(bxr);
   elm_object_part_content_set(panes, "right", bxr);

   /*Label widget */

    _label_init(win, bxr, "FILENAME:");
   priv->label = _label_init(win, bxr, "");
   efl_ui_model_connect(priv->label, "default", "path"); //connect "default" to "filename" property

   /* Entry widget */
   priv->entry = elm_entry_add(win);
   efl_ui_model_connect(priv->entry, "elm.text", "path"); //connect "elm.text" to "path" property
   elm_entry_single_line_set(priv->entry, EINA_TRUE);
   elm_box_pack_end(bxr, priv->entry);
   evas_object_size_hint_weight_set(priv->entry, EVAS_HINT_FILL, 0);
   evas_object_size_hint_align_set(priv->entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(priv->entry);

   /* Button widget */
   priv->bt = elm_button_add(win);
   elm_box_pack_end(bxr, priv->bt);
   elm_object_text_set(priv->bt, "update model");
   evas_object_smart_callback_add(priv->bt, "clicked", _update_cb, priv);
   edje_object_signal_callback_add(priv->bt, "test*" , "*", _signal_cb, priv);
   efl_ui_model_connect(priv->bt, "signal/test-%v", "size");
   evas_object_show(priv->bt);

   /* Image widget */
   img_factory = efl_add(EFL_UI_IMAGE_FACTORY_CLASS, win);
   efl_ui_model_connect(img_factory, "", "path"); //connect to "path" property
   efl_ui_model_factory_connect(priv->bt, "icon", img_factory);

   priv->img = efl_ui_factory_create(img_factory, NULL, win);
   elm_box_pack_end(bxr, priv->img);
   evas_object_size_hint_weight_set(priv->img, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(priv->img, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(priv->img);

   evas_object_event_callback_add(win, EVAS_CALLBACK_DEL, _cleanup_cb, priv);
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
