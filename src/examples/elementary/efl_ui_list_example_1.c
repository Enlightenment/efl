// gcc -o efl_ui_list_example_1 efl_ui_list_example_1.c `pkg-config --cflags --libs elementary`

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

const char *styles[] = {
        "odd",
        "default"
   };

const char *texts[] = {
        "Morpheus",
        "Princess Leia",
        "Chuck Norris",
        "Nyota Uhura",
        "Darth Vader",
        "Elvis Presley",
        "Chun-Li"
   };

char edj_path[PATH_MAX];

struct _Layout_Model_Data
{
   Eo *model;
   Evas_Object *ui_list;
};
typedef struct _Layout_Model_Data Layout_Model_Data;

static void
_cleanup_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Layout_Model_Data *priv = (Layout_Model_Data*)data;
   efl_unref(priv->model);
   efl_unref(priv->ui_list);
}

static void
_realized_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Ui_List_Item_Event *ie = event->info;

   efl_ui_model_connect(ie->layout, "elm.text", "name");
   efl_ui_model_connect(ie->layout, "signal/elm,state,%v", "odd_style");
}

static Efl_Model*
_make_model()
{
   Eina_Value vtext, vstyle;
   Efl_Model_Item *model, *child;
   unsigned int i, s, len;

   model = efl_add(EFL_MODEL_ITEM_CLASS, NULL);
   eina_value_setup(&vtext, EINA_VALUE_TYPE_STRING);
   eina_value_setup(&vstyle, EINA_VALUE_TYPE_STRING);

   len = sizeof(texts)/sizeof(const char*);
   for (i = 0; i < (len*4); i++)
     {
        s = i%2;
        child = efl_model_child_add(model);
        eina_value_set(&vstyle, styles[s]);
        efl_model_property_set(child, "odd_style", &vstyle);

        eina_value_set(&vtext, texts[(i % len)]);
        efl_model_property_set(child, "name", &vtext);
     }

   return model;
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Layout_Model_Data *priv;
   Evas_Object *win;

   priv = alloca(sizeof(Layout_Model_Data));
   memset(priv, 0, sizeof(Layout_Model_Data));

   win = elm_win_util_standard_add("viewlist", "Viewlist");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   elm_win_autodel_set(win, EINA_TRUE);

   priv->model = _make_model();
   priv->ui_list = efl_add(EFL_UI_LIST_CLASS, win, efl_ui_view_model_set(efl_added, priv->model));
   efl_event_callback_add(priv->ui_list, EFL_UI_LIST_EVENT_ITEM_REALIZED, _realized_cb, priv);

   elm_win_resize_object_add(win, priv->ui_list);
   evas_object_size_hint_weight_set(priv->ui_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   evas_object_event_callback_add(win, EVAS_CALLBACK_DEL, _cleanup_cb, priv);
   //showall
   evas_object_show(priv->ui_list);
   evas_object_resize(win, 320, 320);
   evas_object_show(win);

   elm_run();
   elm_shutdown();
   ecore_shutdown();

   return 0;
}
ELM_MAIN()
