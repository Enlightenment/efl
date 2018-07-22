#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

#define LIST_ITEM_MAX 20

static void _bstatus(void *data, Evas_Object *obj, void *event_info);
static void _tstatus(void *data, Evas_Object *obj, void *event_info);
static char *_text_get(void *data, Evas_Object *obj, const char *source);
static Evas_Object *_content_get(void *data, Evas_Object *obj, const char *source);
static Eina_Bool _state_get(void *data, Evas_Object *obj, const char *source);
static void _item_del(void *data, Evas_Object *obj);
static void _fill_list(Evas_Object *obj, Elm_Genlist_Item_Class *itc);
static Eina_Bool _dir_has_subs(const char *path);

static Eina_List *dirs = NULL;

static void
_tstatus(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Elm_Object_Item *tb_it;
   const char *status;

   if (elm_panel_hidden_get((Evas_Object*)data))
     status = "hidden";
   else
     status = "shown";
   printf("The top panel is currently %s\n", status);
   tb_it = elm_toolbar_selected_item_get(obj);
   elm_toolbar_item_selected_set(tb_it, EINA_FALSE);
}

static void
_bstatus(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Elm_Object_Item *tb_it;
   const char *status;

   if (elm_panel_hidden_get((Evas_Object*)data))
     status = "hidden";
   else
     status = "shown";
   printf("The bottom panel is currently %s\n", status);
   tb_it = elm_toolbar_selected_item_get(obj);
   elm_toolbar_item_selected_set(tb_it, EINA_FALSE);
}

static char *
_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *source EINA_UNUSED)
{
   return strdup(ecore_file_file_get(data));
}

static Evas_Object *
_content_get(void *data, Evas_Object *obj, const char *source)
{
   if (!strcmp(source, "elm.swallow.icon"))
     {
        Evas_Object *ic;

        ic = elm_icon_add(obj);
        if (ecore_file_is_dir((char *)data))
          elm_icon_standard_set(ic, "folder");
        else
          elm_icon_standard_set(ic, "file");
        evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
        evas_object_show(ic);
        return ic;
     }
   return NULL;
}

static Eina_Bool
_state_get(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *source EINA_UNUSED)
{
   return EINA_FALSE;
}

static void
_item_del(void *data, Evas_Object *obj EINA_UNUSED)
{
   eina_stringshare_del(data);
}

static void
_fill_list(Evas_Object *obj, Elm_Genlist_Item_Class *itc)
{
   Eina_List *l;
   char *real;
   unsigned int x = 0;

   if (!dirs)
     {
        Eina_Iterator *it;
        const char *name;

        it = eina_file_ls(eina_environment_home_get());
        EINA_ITERATOR_FOREACH(it, name)
          {
             if (x >= LIST_ITEM_MAX)
               break;
             if ((ecore_file_file_get(name)[0] != '.') &&
                 ecore_file_is_dir(name))
               {
                  x++;
                  real = ecore_file_realpath(name);
                  dirs = eina_list_sorted_insert(dirs,
                                                 EINA_COMPARE_CB(strcoll),
                                                 real);
                  eina_stringshare_del(name);
               }
          }
        eina_iterator_free(it);
     }
   EINA_LIST_FOREACH(dirs, l, real)
     {
        Eina_Bool result = EINA_FALSE;

        result = _dir_has_subs(real);
        if (!result)
          elm_genlist_item_append(obj, itc, eina_stringshare_add(real),
                                  NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
        else
          elm_genlist_item_append(obj, itc, eina_stringshare_add(real),
                                  NULL, ELM_GENLIST_ITEM_TREE,
                                  NULL, NULL);
     }
}

static Eina_Bool
_dir_has_subs(const char *path)
{
   Eina_Iterator *it;
   const char *name;
   Eina_Bool result = EINA_FALSE;

   it = eina_file_ls(path);
   EINA_ITERATOR_FOREACH(it, name)
     {
        Eina_Bool is_dir;

        is_dir = ecore_file_is_dir(name);
        eina_stringshare_del(name);
        if (is_dir)
          {
             result = EINA_TRUE;
             break;
          }
     }
   eina_iterator_free(it);
   return result;
}

static void
_free_dirs(void)
{
   char *dir;
   EINA_LIST_FREE(dirs, dir)
     free(dir);
}

void
test_panel(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *panel, *tb, *vbx, *toolbar, *photo;
   Evas_Object *list;
   Elm_Genlist_Item_Class *itc = NULL;
   char buf[PATH_MAX] = { 0 };

   win = elm_win_util_standard_add("panel", "Panel");
   elm_win_autodel_set(win, EINA_TRUE);

   vbx = elm_box_add(win);
   evas_object_size_hint_weight_set(vbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, vbx);
   evas_object_show(vbx);

   // top panel
   panel = elm_panel_add(vbx);
   elm_panel_orient_set(panel, ELM_PANEL_ORIENT_TOP);
   evas_object_size_hint_weight_set(panel, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(panel, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(vbx, panel);
   evas_object_show(panel);

   toolbar = elm_toolbar_add(panel);
   elm_toolbar_homogeneous_set(toolbar, EINA_FALSE);
   elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_NONE);
   elm_toolbar_item_append(toolbar, "home", "Hello", _tstatus, panel);
   elm_object_content_set(panel, toolbar);
   evas_object_show(toolbar);

   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(vbx, tb);
   evas_object_show(tb);

   photo = elm_photo_add(tb);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", elm_app_data_dir_get());
   elm_photo_file_set(photo, buf);
   elm_photo_fill_inside_set(photo, EINA_TRUE);
   elm_object_style_set(photo, "shadow");
   evas_object_size_hint_weight_set(photo, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(photo, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, photo, 0, 0, 4, 5);
   evas_object_show(photo);

   // left panel
   panel = elm_panel_add(tb);
   elm_panel_orient_set(panel, ELM_PANEL_ORIENT_LEFT);
   evas_object_size_hint_weight_set(panel, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(panel, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, panel, 0, 0, 2, 4);
   evas_object_show(panel);

   itc = elm_genlist_item_class_new();
   itc->item_style = "default";
   itc->func.text_get = _text_get;
   itc->func.content_get = _content_get;
   itc->func.state_get = _state_get;
   itc->func.del = _item_del;

   list = elm_genlist_add(panel);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(panel, list);
   evas_object_show(list);

   _fill_list(list, itc);

   // right panel
   panel = elm_panel_add(tb);
   elm_panel_orient_set(panel, ELM_PANEL_ORIENT_RIGHT);
   evas_object_size_hint_weight_set(panel, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(panel, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_panel_hidden_set(panel, EINA_TRUE);
   elm_table_pack(tb, panel, 2, 0, 2, 4);
   evas_object_show(panel);

   list = elm_genlist_add(panel);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(panel, list);
   evas_object_show(list);

   _fill_list(list, itc);
   elm_genlist_item_class_free(itc);

   _free_dirs();

   // bottom panel
   panel = elm_panel_add(vbx);
   elm_panel_orient_set(panel, ELM_PANEL_ORIENT_BOTTOM);
   evas_object_size_hint_weight_set(panel, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(panel, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_panel_hidden_set(panel, EINA_TRUE);
   elm_table_pack(tb, panel, 0, 4, 4, 1);
   evas_object_show(panel);

   toolbar = elm_toolbar_add(panel);
   elm_toolbar_homogeneous_set(toolbar, EINA_FALSE);
   elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_NONE);
   evas_object_size_hint_weight_set(toolbar, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(toolbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_toolbar_item_append(toolbar, "home", "Hello", _bstatus, panel);
   elm_object_content_set(panel, toolbar);
   evas_object_show(toolbar);

   evas_object_resize(win, 320, 400);
   evas_object_show(win);
}

static void
_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *panel = data;
   elm_panel_toggle(panel);
}

static void
_toggled_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *list;
   int i;

   if (!elm_check_state_get(data)) return;

   list = elm_object_content_get(obj);
   evas_object_del(list);

   list = elm_list_add(obj);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   for (i = 0; i < 7; i++)
     elm_list_item_append(list, "panel list item", NULL, NULL, NULL, NULL);
   elm_object_content_set(obj, list);

   printf("Panel toggled:%s\n", elm_panel_hidden_get(obj) ? "hidden" : "visible");
}

static void
_changed_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   elm_config_scroll_thumbscroll_enabled_set(elm_check_state_get(obj));
}

void
test_panel2(void *data EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *label, *check, *table, *panel, *list, *button;
   int i;

   // Left Panel
   win = elm_win_util_standard_add("panel", "Left Panel");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 320, 400);
   evas_object_show(win);

   // box for button and table
   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(box);
   elm_win_resize_object_add(win, box);

   // label
   label = elm_label_add(box);
   elm_object_text_set(label, "You should enable thumb scroll to use scrollable panel.<br/>"
                              "Check below or enable thumb scroll in elementary_config.");
   evas_object_show(label);
   elm_box_pack_end(box, label);

   // check
   check = elm_check_add(box);
   elm_check_state_set(check, elm_config_scroll_thumbscroll_enabled_get());
   elm_object_text_set(check, "Enable thumb scroll (temporarily)");
   evas_object_show(check);
   elm_box_pack_end(box, check);

   evas_object_smart_callback_add(check, "changed", _changed_cb, NULL);

   check = elm_check_add(box);
   elm_object_text_set(check, "Reset content on toggle");
   evas_object_show(check);
   elm_box_pack_end(box, check);

   // toggle button
   button = elm_button_add(box);
   evas_object_size_hint_weight_set(button, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(button, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(button);
   elm_object_text_set(button, "Toggle");
   elm_box_pack_end(box, button);

   // table for panel and center content
   table = elm_table_add(win);
   evas_object_size_hint_weight_set(table, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(table, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(table);
   elm_box_pack_end(box, table);

   // center content
   list = elm_list_add(table);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(list);
   for (i = 0; i < 20; i++)
     elm_list_item_append(list, "center list item", NULL, NULL, NULL, NULL);
   elm_table_pack(table, list, 0, 0, 1, 1);

   // panel
   panel = elm_panel_add(table);
   evas_object_size_hint_weight_set(panel, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(panel, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(panel);
   elm_table_pack(table, panel, 0, 0, 1, 1);

   elm_panel_scrollable_set(panel, EINA_TRUE);
   elm_panel_orient_set(panel, ELM_PANEL_ORIENT_LEFT);
   elm_panel_hidden_set(panel, EINA_TRUE);
   elm_panel_scrollable_content_size_set(panel, 0.75);

   list = elm_list_add(panel);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   for (i = 0; i < 7; i++)
     elm_list_item_append(list, "panel list item", NULL, NULL, NULL, NULL);
   elm_object_content_set(panel, list);

   evas_object_smart_callback_add(panel, "toggled", _toggled_cb, check);
   evas_object_smart_callback_add(button, "clicked", _clicked_cb, panel);
}
