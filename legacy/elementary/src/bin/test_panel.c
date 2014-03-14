#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

#define LIST_ITEM_MAX 20

static const char *img1 = PACKAGE_DATA_DIR "/images/plant_01.jpg";

static Elm_Genlist_Item_Class itc;

static void _bstatus(void *data, Evas_Object *obj, void *event_info);
static void _tstatus(void *data, Evas_Object *obj, void *event_info);
static char *_text_get(void *data, Evas_Object *obj, const char *source);
static Evas_Object *_content_get(void *data, Evas_Object *obj, const char *source);
static Eina_Bool _state_get(void *data, Evas_Object *obj, const char *source);
static void _item_del(void *data, Evas_Object *obj);
static void _fill_list(Evas_Object *obj);
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
_fill_list(Evas_Object *obj)
{
   DIR *d;
   struct dirent *de;
   Eina_List *l;
   char *real;
   char *home_env = NULL;
   unsigned int x = 0;

   if (!dirs)
     {
        home_env = getenv("HOME");
        if (!home_env) return;
        if (!(d = opendir(home_env))) return;
        while ((de = readdir(d)) && (x < LIST_ITEM_MAX))
          {
             char buff[PATH_MAX];

             if (de->d_name[0] == '.') continue;
             snprintf(buff, sizeof(buff), "%s/%s", getenv("HOME"), de->d_name);
             if (!ecore_file_is_dir(buff)) continue;
             x++;
             real = ecore_file_realpath(buff);
             dirs = eina_list_sorted_insert(dirs, EINA_COMPARE_CB(strcoll), real);
          }
        closedir(d);
     }
   EINA_LIST_FOREACH(dirs, l, real)
     {
        Eina_Bool result = EINA_FALSE;

        result = _dir_has_subs(real);
        if (!result)
          elm_genlist_item_append(obj, &itc, eina_stringshare_add(real),
                                  NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
        else
          elm_genlist_item_append(obj, &itc, eina_stringshare_add(real),
                                  NULL, ELM_GENLIST_ITEM_TREE,
                                  NULL, NULL);
     }
}

static Eina_Bool
_dir_has_subs(const char *path)
{
   DIR *d;
   struct dirent *de;
   Eina_Bool result = EINA_FALSE;

   if (!path) return result;
   if (!(d = opendir(path))) return result;
   while ((de = readdir(d)))
     {
        char buff[PATH_MAX];

        if (de->d_name[0] == '.') continue;
        snprintf(buff, sizeof(buff), "%s/%s", path, de->d_name);
        if (ecore_file_is_dir(buff))
          {
             result = EINA_TRUE;
             break;
          }
     }
   closedir(d);
   return result;
}

void
test_panel(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *panel, *tb, *vbx, *toolbar, *photo;
   Evas_Object *list;

   win = elm_win_util_standard_add("panel", "Panel");
   elm_win_autodel_set(win, EINA_TRUE);

   vbx = elm_box_add(win);
   evas_object_size_hint_weight_set(vbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, vbx);
   evas_object_show(vbx);

   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(tb);

   panel = elm_panel_add(vbx);
   elm_panel_orient_set(panel, ELM_PANEL_ORIENT_TOP);
   evas_object_size_hint_weight_set(panel, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(panel, EVAS_HINT_FILL, EVAS_HINT_FILL);

   toolbar = elm_toolbar_add(panel);
   elm_toolbar_homogeneous_set(toolbar, EINA_FALSE);
   elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_NONE);
   evas_object_size_hint_weight_set(toolbar, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(toolbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(toolbar);

   elm_toolbar_item_append(toolbar, "home", "Hello", _tstatus, panel);

   elm_object_content_set(panel, toolbar);
   elm_box_pack_end(vbx, panel);
   evas_object_show(panel);

   photo = elm_photo_add(tb);
   elm_photo_file_set(photo, img1);
   elm_photo_fill_inside_set(photo, EINA_TRUE);
   elm_object_style_set(photo, "shadow");
   evas_object_size_hint_weight_set(photo, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(photo, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, photo, 0, 0, 4, 5);
   evas_object_show(photo);

   panel = elm_panel_add(tb);
   elm_panel_orient_set(panel, ELM_PANEL_ORIENT_LEFT);
   evas_object_size_hint_weight_set(panel, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   //   evas_object_size_hint_align_set(panel, 0, EVAS_HINT_FILL);
   evas_object_size_hint_align_set(panel, EVAS_HINT_FILL, EVAS_HINT_FILL);

   itc.item_style = "default";
   itc.func.text_get = _text_get;
   itc.func.content_get = _content_get;
   itc.func.state_get = _state_get;
   itc.func.del = _item_del;

   list = elm_genlist_add(panel);
   evas_object_size_hint_min_set(list, 100, -1);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(list);

   elm_object_content_set(panel, list);
   elm_table_pack(tb, panel, 0, 0, 2, 4);
   evas_object_show(panel);

   _fill_list(list);

   panel = elm_panel_add(tb);
   elm_panel_orient_set(panel, ELM_PANEL_ORIENT_RIGHT);
   evas_object_size_hint_weight_set(panel, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   //   evas_object_size_hint_align_set(panel, 1, EVAS_HINT_FILL);
   evas_object_size_hint_align_set(panel, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_panel_hidden_set(panel, EINA_TRUE);

   list = elm_genlist_add(panel);
   evas_object_size_hint_min_set(list, 100, -1);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(list);

   elm_object_content_set(panel, list);
   elm_table_pack(tb, panel, 2, 0, 2, 4);
   evas_object_show(panel);

   _fill_list(list);
   {
      char *dir;
      EINA_LIST_FREE(dirs, dir)
        free(dir);
   }

   elm_box_pack_end(vbx, tb);

   panel = elm_panel_add(vbx);
   elm_panel_orient_set(panel, ELM_PANEL_ORIENT_BOTTOM);
   evas_object_size_hint_weight_set(panel, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(panel, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_panel_hidden_set(panel, EINA_TRUE);

   toolbar = elm_toolbar_add(panel);
   elm_toolbar_homogeneous_set(toolbar, EINA_FALSE);
   elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_NONE);
   evas_object_size_hint_weight_set(toolbar, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(toolbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(toolbar);

   elm_toolbar_item_append(toolbar, "home", "Hello", _bstatus, panel);

   elm_object_content_set(panel, toolbar);
   elm_table_pack(tb, panel, 0, 4, 4, 1);
   evas_object_show(panel);

   evas_object_resize(win, 320, 400);
   evas_object_show(win);
}
