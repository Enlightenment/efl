// gcc -o efl_ui_list_view_example_3 efl_ui_list_view_example_3.c `pkg-config --cflags --libs elementary`

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define EFL_BETA_API_SUPPORT 1
#endif

#define ELM_INTERFACE_ATSPI_SELECTION_PROTECTED

#include <Eo.h>
#include <Efl_Ui.h>
#include <Elementary.h>
#include <Efl.h>
#include <Eio.h>
#include <stdio.h>

const char *subtexts[] = {
        "Captain",
        "Princess ",
        "Actor",
        "Translator",
        "Jedi",
        "Singer",
        "Fighter"
   };

const char *texts[] = {
        "Morpheus",
        "Leia",
        "Chuck Norris",
        "Nyota Uhura",
        "Darth Vader",
        "Elvis Presley",
        "Chun-Li"
   };

char edj_path[PATH_MAX];

struct _Priv_Data
{
   Eo *model;
   Evas_Object *list1;
   Evas_Object *list2;
   Evas_Object *e_name;
   Evas_Object *e_occ;
   Evas_Object *selected;
};
typedef struct _Priv_Data Priv_Data;

static void
_cleanup_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Priv_Data *priv = (Priv_Data*)data;
   efl_del(priv->model);
}

static void
_selected(void *data, const Efl_Event *event)
{
   Priv_Data *priv = (Priv_Data*)data;

   priv->selected = efl_ui_selectable_last_selected_get(event->object);
}

static void
_bt_add_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Priv_Data *priv = (Priv_Data*)data;
   Eina_Value vtext, value;
   Efl_Generic_Model *child;

   eina_value_setup(&vtext, EINA_VALUE_TYPE_STRING);
   eina_value_setup(&value, EINA_VALUE_TYPE_UCHAR);
   child = efl_model_child_add(priv->model);

   eina_value_set(&vtext, elm_object_text_get(priv->e_name));
   efl_model_property_set(child, "filename", &vtext);

   eina_value_set(&vtext, elm_object_text_get(priv->e_occ));
   efl_model_property_set(child, "occupation", &vtext);

   eina_value_set(&value, EINA_FALSE);
   efl_model_property_set(child, "selected", &vtext);

   eina_value_flush(&vtext);
   eina_value_flush(&value);
}

static void
_bt_del_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Priv_Data *priv = (Priv_Data*)data;
   Eo *child = NULL;

   if(priv->selected)
     {
        printf("focused %p\n", priv->selected);
        child = efl_ui_view_model_get(priv->selected);
        efl_model_child_del(priv->model, child);
        priv->selected = NULL;
     }
   else
     {
        printf("no focused\n");
     }
}

static void
_bt_none_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *li = data;
   efl_ui_multi_selectable_select_mode_set(li, EFL_UI_SELECT_MODE_NONE);
}

static void
_bt_donly_clicked(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   /* Evas_Object *li = data; */
   /* efl_ui_multi_selectable_async_select_mode_set(li, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY); */
}

static void
_bt_default_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *li = data;
   efl_ui_multi_selectable_select_mode_set(li, EFL_UI_SELECT_MODE_SINGLE);
}

static void
_bt_set_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Priv_Data *priv = data;
   efl_ui_view_model_set(priv->list2, priv->model);
}

static void
_bt_unset_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *li = data;
   efl_ui_view_model_set(li, NULL);
}
static void
_realized_1_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Gfx_Entity *layout = event->info;

   efl_ui_property_bind(layout, "efl.text", "filename");
}

static void
_realized_2_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Gfx_Entity *layout = event->info;

   efl_ui_property_bind(layout, "efl.text", "occupation");
}

static Efl_Model*
_make_model(void)
{
   Eina_Value vtext, value;
   Efl_Generic_Model *model, *child;
   unsigned int i, len;

   model = efl_add(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   eina_value_setup(&vtext, EINA_VALUE_TYPE_STRING);
   eina_value_setup(&value, EINA_VALUE_TYPE_UCHAR);

   len = sizeof(texts)/sizeof(const char*);
   for (i = 0; i < (len); i++)
     {
        child = efl_model_child_add(model);

        eina_value_set(&vtext, texts[(i % len)]);
        efl_model_property_set(child, "filename", &vtext);

        eina_value_set(&vtext, subtexts[(i % len)]);
        efl_model_property_set(child, "occupation", &vtext);

        eina_value_set(&value, EINA_FALSE);
        efl_model_property_set(child, "selected", &value);
     }

   eina_value_flush(&vtext);
   eina_value_flush(&value);
   return model;
}

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Priv_Data *priv;
   Evas_Object *win, *bx, *vbx, *bt;
   Efl_Ui_Layout_Factory *factory;

   priv = alloca(sizeof(Priv_Data));
   memset(priv, 0, sizeof(Priv_Data));

   win = elm_win_util_standard_add("list_view3", "List_View3");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   priv->model = _make_model();
   factory = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, win);
   efl_ui_property_bind(factory, "efl.text", "filename");
   efl_ui_widget_factory_item_class_set(factory, EFL_UI_LIST_DEFAULT_ITEM_CLASS);

   priv->list1 = efl_add(EFL_UI_LIST_VIEW_CLASS, win, efl_ui_view_model_set(efl_added, priv->model));
   efl_event_callback_add(priv->list1, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_REALIZED, _realized_1_cb, priv);
   evas_object_size_hint_weight_set(priv->list1, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(priv->list1, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, priv->list1);
   efl_ui_collection_view_factory_set(priv->list1, factory);

   factory = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, win);
   efl_ui_property_bind(factory, "efl.text", "filename");
   efl_ui_widget_factory_item_class_set(factory, EFL_UI_LIST_DEFAULT_ITEM_CLASS);
   priv->list2 = efl_add(EFL_UI_LIST_VIEW_CLASS, win, efl_ui_view_model_set(efl_added, priv->model));
   efl_event_callback_add(priv->list2, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_REALIZED, _realized_2_cb, priv->list2);
   evas_object_size_hint_weight_set(priv->list2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(priv->list2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   efl_ui_collection_view_factory_set(priv->list2, factory);

   vbx = elm_box_add(win);
   elm_box_pack_end(bx, vbx);

   bt = elm_label_add(win);
   elm_object_text_set(bt, "Name:");
   elm_box_pack_end(vbx, bt);
   evas_object_show(bt);

   priv->e_name = elm_entry_add(win);
   elm_box_pack_end(vbx, priv->e_name);
   elm_object_text_set(priv->e_name, "Neo");
   evas_object_size_hint_weight_set(priv->e_name, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(priv->e_name, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(priv->e_name);

   bt = elm_label_add(win);
   elm_object_text_set(bt, "Occupation:");
   elm_box_pack_end(vbx, bt);
   evas_object_show(bt);

   priv->e_occ = elm_entry_add(win);
   elm_box_pack_end(vbx, priv->e_occ);
   elm_object_text_set(priv->e_occ, "Chosen");
   evas_object_size_hint_weight_set(priv->e_occ, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(priv->e_occ, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(priv->e_occ);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Add Item");
   evas_object_smart_callback_add(bt, "clicked", _bt_add_clicked, priv);
   evas_object_show(bt);
   elm_box_pack_end(vbx, bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Delete Item");
   evas_object_smart_callback_add(bt, "clicked", _bt_del_clicked, priv);
   evas_object_show(bt);
   elm_box_pack_end(vbx, bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Select None");
   evas_object_smart_callback_add(bt, "clicked", _bt_none_clicked, priv->list2);
   evas_object_show(bt);
   elm_box_pack_end(vbx, bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Select DisplayOnly");
   evas_object_smart_callback_add(bt, "clicked", _bt_donly_clicked, priv->list2);
   evas_object_show(bt);
   elm_box_pack_end(vbx, bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Select Default");
   evas_object_smart_callback_add(bt, "clicked", _bt_default_clicked, priv->list2);
   evas_object_show(bt);
   elm_box_pack_end(vbx, bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Model Set");
   evas_object_smart_callback_add(bt, "clicked", _bt_set_clicked, priv);
   evas_object_show(bt);
   elm_box_pack_end(vbx, bt);


   bt = elm_button_add(win);
   elm_object_text_set(bt, "Model Unset");
   evas_object_smart_callback_add(bt, "clicked", _bt_unset_clicked, priv->list2);
   evas_object_show(bt);
   elm_box_pack_end(vbx, bt);

   elm_box_pack_end(bx, priv->list2);
   efl_event_callback_add(priv->list2, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, _selected, priv);

   evas_object_event_callback_add(win, EVAS_CALLBACK_DEL, _cleanup_cb, priv);

   //showall
   evas_object_show(priv->list1);
   evas_object_show(priv->list2);
   evas_object_show(bx);
   evas_object_show(vbx);
   evas_object_resize(win, 400, 320);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
