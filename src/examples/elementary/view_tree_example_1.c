// gcc -o tree_view_example_1 tree_view_example_1.c `pkg-config --cflags --libs elementary`

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
#include <time.h>

#define NUM_ITEMS 20

static void
_realized_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   char *depth, *name;
   Eina_Value *vd, *vn;
   Efl_Ui_Tree_View_Item_Event *ie = event->info;
   if (!ie->child) return;

   vd = efl_model_property_get(ie->child, "depth");
   vn = efl_model_property_get(ie->child, "name");
   depth = eina_value_to_string(vd);
   name = eina_value_to_string(vn);
   eina_value_free(vd);
   eina_value_free(vn);

   printf("realize item=%s depth=%s\n", name, depth);
   free(name);
   free(depth);
}

static Efl_Model*
_child_add(Efl_Model_Item *parent, Eina_Value *value, const char *msg)
{
   Efl_Model_Item *model;

   model = efl_model_child_add(parent);
   if (model)
     {
       eina_value_set(value, msg);
       efl_model_property_set(model, "name", value);
     }

   return model;
}

static void
_children_add(Efl_Model_Item *parent, Eina_Value *value, int n, const char *msg)
{
   char msgbuf[256];
   char buff[256];
   int i;

   if (!parent) return;
   snprintf(msgbuf, sizeof(msgbuf), "     %s - %%i", msg);

   for (i = 0; i < n; ++i)
     {
        snprintf(buff, sizeof(buff), msgbuf, i);
        _children_add(_child_add(parent, value, buff), value, i, buff);
     }
}

static Efl_Model*
_make_model(Evas_Object *win)
{
   Efl_Model_Item *root, *child;
   Eina_Value vtext;
   unsigned int i;

   srand(time(NULL));
   eina_value_setup(&vtext, EINA_VALUE_TYPE_STRING);
   root = efl_add(EFL_MODEL_ITEM_CLASS, win);
   char buf[256];

   for (i = 0; i < (NUM_ITEMS); ++i)
     {
        snprintf(buf, sizeof(buf), "Item # %i", i);
        child = _child_add(root, &vtext, buf);
        snprintf(buf, sizeof(buf), "|-> Item # %i", i);
        _children_add(child, &vtext, 5, buf);
     }

   eina_value_flush(&vtext);
   return root;
}

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Efl_Ui_Layout_Factory *factory;
   Evas_Object *win;
   Eo *model, *tree;

   win = elm_win_util_standard_add("viewtree", "ViewTree");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   elm_win_autodel_set(win, EINA_TRUE);

   model = _make_model(win);
   factory = efl_add(EFL_UI_TREE_FACTORY_CLASS, win,
                    efl_ui_layout_factory_theme_config(efl_added, "list_item", NULL, "default"));
   efl_ui_model_connect(factory, "efl.text", "name");

   tree = efl_add(EFL_UI_TREE_VIEW_CLASS, win);
   efl_ui_tree_view_layout_factory_set(tree, factory);
   efl_ui_view_model_set(tree, model);

   evas_object_size_hint_weight_set(tree, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tree, EVAS_HINT_FILL, EVAS_HINT_FILL);
   efl_event_callback_add(tree, EFL_UI_TREE_VIEW_EVENT_ITEM_REALIZED, _realized_cb, NULL);

   elm_win_resize_object_add(win, tree);

   //showall
   evas_object_show(tree);
   evas_object_resize(win, 320, 320);
   evas_object_show(win);

   elm_run();
   return 0;
}
ELM_MAIN()
