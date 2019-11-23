// gcc -o layout_model_connect layout_model_connect.c `pkg-config --cflags --libs elementary`

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define EFL_BETA_API_SUPPORT 1
#endif

#include <Efl_Ui.h>
#include <Elementary.h>
#include <Efl.h>
#include <Eio.h>
#include <stdio.h>

#define EFL_MODEL_TEST_FILENAME_PATH "/tmp"

struct _Layout_Model_Data
{
   Eo *fileview;
   Eo *model;
   Eo *provider;
   Evas_Object *label;
   Evas_Object *entry;
   Evas_Object *img;
   Evas_Object *bt;
   Evas_Object *bxr;
};
typedef struct _Layout_Model_Data Layout_Model_Data;

static Evas_Object *win = NULL;

static Eina_Value
_wait_for_image(Eo *o EINA_UNUSED, void *data, const Eina_Value v)
{
   Layout_Model_Data *priv = data;

   priv->img = eina_value_object_get(&v);
   elm_box_pack_end(priv->bxr, priv->img);
   evas_object_size_hint_weight_set(priv->img, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(priv->img, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(priv->img);

   return v;
}

static void
_cleanup_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Layout_Model_Data *priv = (Layout_Model_Data*)data;
   efl_unref(priv->fileview);
   efl_unref(priv->model);
}

static void
_list_pressed_item_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Layout_Model_Data *priv = data;
   Efl_Ui_Item_Clickable_Pressed *pressed = event->info;
   Efl_Ui_Item *item = pressed->item;
   Efl_Model *model = efl_ui_view_model_get(item);

   printf("LIST pressed model `%s` from item `%s`.\n",
          efl_debug_name_get(model),
          efl_debug_name_get(item));
   efl_ui_view_model_set(priv->provider, model);
}

static void
_update_cb(void *data, Evas_Object *obj EINA_UNUSED, void *ev EINA_UNUSED)
{
   Layout_Model_Data *priv = data;
   Efl_Model *newone;

   const char *text = elm_object_text_get(priv->entry);
   newone = efl_add(EFL_IO_MODEL_CLASS, win,
                    efl_io_model_path_set(efl_added, text));
   efl_ui_view_model_set(priv->fileview, newone);
   efl_del(priv->model);
   priv->model = newone;

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
   Evas_Object *panes, *bxr;
   Efl_Ui_Factory *factory;
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

   priv->model = efl_add(EFL_IO_MODEL_CLASS, win, efl_io_model_path_set(efl_added, dirname));

   factory = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, win);
   efl_ui_widget_factory_item_class_set(factory, EFL_UI_LIST_DEFAULT_ITEM_CLASS);
   efl_ui_property_bind(factory, "efl.text", "filename");

   priv->fileview = efl_add(EFL_UI_LIST_VIEW_CLASS, panes,
                            efl_ui_collection_view_factory_set(efl_added, factory),
                            efl_ui_view_model_set(efl_added, priv->model));
   _widget_init(priv->fileview);
   elm_object_part_content_set(panes, "left", priv->fileview);
   elm_panes_content_left_size_set(panes, 0.3);
   efl_event_callback_add(priv->fileview, EFL_UI_EVENT_ITEM_PRESSED, _list_pressed_item_cb, priv);

   bxr = elm_box_add(win);
   priv->bxr = bxr;
   _widget_init(bxr);
   elm_object_part_content_set(panes, "right", bxr);

   priv->provider = efl_add(EFL_MODEL_PROVIDER_CLASS, win);
   efl_provider_register(bxr, EFL_MODEL_PROVIDER_CLASS, priv->provider);

   /*Label widget */
    _label_init(win, bxr, "FILENAME:");
   priv->label = _label_init(win, bxr, "");
   efl_ui_property_bind(priv->label, "default", "path"); //connect "default" to "filename" property

   /* Entry widget */
   priv->entry = elm_entry_add(bxr);
   efl_ui_property_bind(priv->entry, "elm.text", "path"); //connect "elm.text" to "path" property
   elm_entry_single_line_set(priv->entry, EINA_TRUE);
   elm_box_pack_end(bxr, priv->entry);
   evas_object_size_hint_weight_set(priv->entry, EVAS_HINT_FILL, 0);
   evas_object_size_hint_align_set(priv->entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(priv->entry);

   /* Button widget */
   priv->bt = elm_button_add(bxr);
   elm_box_pack_end(bxr, priv->bt);
   elm_object_text_set(priv->bt, "update model");
   evas_object_smart_callback_add(priv->bt, "clicked", _update_cb, priv);
   edje_object_signal_callback_add(priv->bt, "test*" , "*", _signal_cb, priv);
   efl_ui_property_bind(priv->bt, "signal/test-%v", "size");
   evas_object_show(priv->bt);

   /* Image widget */
   img_factory = efl_add(EFL_UI_IMAGE_FACTORY_CLASS, bxr);
   efl_ui_property_bind(img_factory, "", "path"); //connect to "path" property
   efl_ui_factory_bind(priv->bt, "icon", img_factory);

   efl_future_then(win, efl_ui_view_factory_create_with_event(img_factory, NULL),
                   .success = _wait_for_image,
                   .data = priv);

   evas_object_event_callback_add(win, EVAS_CALLBACK_DEL, _cleanup_cb, priv);
   //showall
   evas_object_resize(win, 800, 400);
   evas_object_show(panes);
   evas_object_show(win);

   elm_run();

   elm_shutdown();

   return 0;
}
ELM_MAIN()
