/**
 * @defgroup Fileselector Fileselector
 *
 * A fileselector is a widget that allows a user to navigate through a
 * tree of files.  It contains buttons for Home(~) and Up(..) as well
 * as cancel/ok buttons to confirm/cancel a selection.  This widget is
 * currently very much in progress.
 *
 * TODO
 * child elements focusing support
 * userdefined icon/label cb
 * show/hide/add buttons ???
 * show/Hide hidden files
 * double click to choose a file
 * multiselection
 * make variable/function names that are sensible
 * Filter support
 *
 * Signals that you can add callbacks for are:
 *
 * "selected" - the user clicks on a file
 * "directory,open" - the list is populated with new content.
 *                    event_info is a directory.
 * "done" - the user clicks on the ok or cancel button
 */

#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *edje;
   Evas_Object *filename_entry;
   Evas_Object *path_entry;
   Evas_Object *files_list;
   Evas_Object *files_grid;
   Evas_Object *up_button;
   Evas_Object *home_button;

   Evas_Object *ok_button;
   Evas_Object *cancel_button;

   const char  *path;
   const char  *selection;
   Ecore_Idler *sel_idler;

   const char  *path_separator;

   Elm_Fileselector_Mode mode;

   Eina_Bool    only_folder : 1;
   Eina_Bool    expand : 1;
};

struct sel_data
{
   Evas_Object *fs;
   const char  *path;
};

Elm_Genlist_Item_Class list_itc;
Elm_Gengrid_Item_Class grid_itc;

static const char *widtype = NULL;

static const char SIG_DIRECTORY_OPEN[] = "directory,open";
static const char SIG_DONE[] = "done";
static const char SIG_SELECTED[] = "selected";
static const Evas_Smart_Cb_Description _signals[] = {
   {SIG_DIRECTORY_OPEN, "s"},
   {SIG_DONE, "s"},
   {SIG_SELECTED, "s"},
   {NULL, NULL}
};

static void _populate(Evas_Object      *obj,
                      const char       *path,
                      Elm_Genlist_Item *parent);
static void _do_anchors(Evas_Object *obj,
                        const char  *path);

/***  ELEMENTARY WIDGET  ***/
static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd;
   void *sd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (wd->path) eina_stringshare_del(wd->path);
   if (wd->selection) eina_stringshare_del(wd->selection);
   if (wd->sel_idler)
     {
        sd = ecore_idler_del(wd->sel_idler);
        free(sd);
     }
   free(wd);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;
   if (!wd) return;
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(wd->edje, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_widget_mirrored_set(wd->cancel_button, rtl);
   elm_widget_mirrored_set(wd->ok_button, rtl);
   elm_widget_mirrored_set(wd->files_list, rtl);
   elm_widget_mirrored_set(wd->up_button, rtl);
   elm_widget_mirrored_set(wd->home_button, rtl);
   edje_object_mirrored_set(wd->edje, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const char *style = elm_widget_style_get(obj);
   const char *data;
   char buf[1024];

   if (!wd) return;
   _elm_widget_mirrored_reload(obj);

   _elm_theme_object_set(obj, wd->edje, "fileselector", "base", style);

   if (elm_object_disabled_get(obj))
     edje_object_signal_emit(wd->edje, "elm,state,disabled", "elm");

   data = edje_object_data_get(wd->edje, "path_separator");
   if (data)
     wd->path_separator = data;
   else
     wd->path_separator = "/";

   if (!style) style = "default";
   snprintf(buf, sizeof(buf), "fileselector/%s", style);

#define SWALLOW(part_name, object_ptn)                                \
  if (object_ptn)                                                     \
    {                                                                 \
       elm_widget_style_set(object_ptn, buf);                         \
       if (edje_object_part_swallow(wd->edje, part_name, object_ptn)) \
         evas_object_show(object_ptn);                                \
       else                                                           \
         evas_object_hide(object_ptn);                                \
    }
   SWALLOW("elm.swallow.up", wd->up_button);
   SWALLOW("elm.swallow.home", wd->home_button);

   if (wd->mode == ELM_FILESELECTOR_LIST)
     {
        if (edje_object_part_swallow(wd->edje, "elm.swallow.files",
                                     wd->files_list))
          {
             evas_object_show(wd->files_list);
             evas_object_hide(wd->files_grid);
          }
        else
          evas_object_hide(wd->files_list);
     }
   else
     {
        if (edje_object_part_swallow(wd->edje, "elm.swallow.files",
                                     wd->files_grid))
          {
             evas_object_show(wd->files_grid);
             evas_object_hide(wd->files_list);
          }
        else
          evas_object_hide(wd->files_grid);
     }

   SWALLOW("elm.swallow.filename", wd->filename_entry);
   SWALLOW("elm.swallow.path", wd->path_entry);

   snprintf(buf, sizeof(buf), "fileselector/actions/%s", style);
   SWALLOW("elm.swallow.cancel", wd->cancel_button);
   SWALLOW("elm.swallow.ok", wd->ok_button);
#undef SWALLOW

   edje_object_message_signal_process(wd->edje);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   edje_object_scale_set
     (wd->edje, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

/***  GENLIST "MODEL"  ***/
static char *
_itc_label_get(void              *data,
               Evas_Object *obj   __UNUSED__,
               const char *source __UNUSED__)
{
   return strdup(ecore_file_file_get(data)); /* NOTE this will be
                                              * free() by the
                                              * caller */
}

static Evas_Object *
_itc_icon_get(void        *data,
              Evas_Object *obj,
              const char  *source)
{
   Evas_Object *ic;

   if (!strcmp(source, "elm.swallow.icon"))
     {
        const char *filename = data;

        ic = elm_icon_add(obj);
        if (ecore_file_is_dir((char *)data))
          elm_icon_standard_set(ic, "folder");
        else
          {
             if (evas_object_image_extension_can_load_fast_get(filename))
               {
                  elm_icon_standard_set(ic, "image");
                  elm_icon_thumb_set(ic, filename, NULL);
               }
             else
               {
                  elm_icon_standard_set(ic, "file");
               }
          }

        evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL,
                                         1, 1);
        evas_object_show(ic);
        return ic;
     }
   return NULL;
}

static Eina_Bool
_itc_state_get(void *data         __UNUSED__,
               Evas_Object *obj   __UNUSED__,
               const char *source __UNUSED__)
{
   return EINA_FALSE;
}

static void
_itc_del(void            *data,
         Evas_Object *obj __UNUSED__)
{
   eina_stringshare_del(data);
}

static void
_expand_done(void            *data,
             Evas_Object *obj __UNUSED__,
             void            *event_info)
{
   Elm_Genlist_Item *it = event_info;
   const char *path = elm_genlist_item_data_get(it);
   _populate(data, path, it);
}

static void
_contract_done(void *data       __UNUSED__,
               Evas_Object *obj __UNUSED__,
               void            *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_subitems_clear(it);
}

static void
_expand_req(void *data       __UNUSED__,
            Evas_Object *obj __UNUSED__,
            void            *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_expanded_set(it, 1);
}

static void
_contract_req(void *data       __UNUSED__,
              Evas_Object *obj __UNUSED__,
              void            *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_expanded_set(it, 0);
}

/***  PRIVATES  ***/
static Eina_Bool
_sel_do(void *data)
{
   struct sel_data *sd;
   const char *path;
   Widget_Data *wd;
   const char *p;

   sd = data;
   wd = elm_widget_data_get(sd->fs);
   path = sd->path;

   if ((!wd->only_folder) && ecore_file_is_dir(path))
     {
        if (wd->expand && wd->mode == ELM_FILESELECTOR_LIST)
          {
             _do_anchors(sd->fs, path);
             elm_entry_entry_set(wd->filename_entry, "");
          }
        else
          {
             /* keep a ref to path 'couse it will be destroyed by _populate */
             p = eina_stringshare_add(path);
             _populate(sd->fs, p, NULL);
             eina_stringshare_del(p);
          }
        goto end;
     }
   else /* navigating through folders only or file is not a dir. */
     {
        if (wd->expand && wd->mode == ELM_FILESELECTOR_LIST)
          _do_anchors(sd->fs, path);
        else if (wd->only_folder)
          {
             /* keep a ref to path 'couse it will be destroyed by _populate */
             p = eina_stringshare_add(path);
             _populate(sd->fs, p, NULL);
             eina_stringshare_del(p);
          }
        elm_entry_entry_set(wd->filename_entry,
                                     ecore_file_file_get(path));
     }

   evas_object_smart_callback_call(sd->fs, SIG_SELECTED, (void *)path);

end:
   wd->sel_idler = NULL;
   free(sd);
   return ECORE_CALLBACK_CANCEL;
}

static void
_sel(void            *data,
     Evas_Object *obj __UNUSED__,
     void            *event_info)
{
   struct sel_data *sd;
   Widget_Data *wd;
   void *old_sd;
   char *dir;

   wd = elm_widget_data_get(data);
   if (!wd) return;

   sd = malloc(sizeof(*sd));
   sd->fs = data;
   sd->path = wd->mode == ELM_FILESELECTOR_LIST ?
       elm_genlist_item_data_get(event_info) :
       elm_gengrid_item_data_get(event_info);

   if (!sd->path)
     {
        eina_stringshare_replace(&wd->path, "");
        goto end;
     }

   dir = wd->only_folder ? strdup(sd->path) : ecore_file_dir_get(sd->path);
   if (dir)
     {
        eina_stringshare_replace(&wd->path, dir);
        free(dir);
     }
   else
     {
        eina_stringshare_replace(&wd->path, "");
     }

end:
   if (wd->sel_idler)
     {
        old_sd = ecore_idler_del(wd->sel_idler);
        free(old_sd);
     }
   wd->sel_idler = ecore_idler_add(_sel_do, sd);
}

static void
_up(void            *data,
    Evas_Object *obj __UNUSED__,
    void *event_info __UNUSED__)
{
   Evas_Object *fs = data;
   char *parent;

   Widget_Data *wd = elm_widget_data_get(fs);
   if (!wd) return;
   parent = ecore_file_dir_get(wd->path);
   _populate(fs, parent, NULL);
   free(parent);
}

static void
_home(void            *data,
      Evas_Object *obj __UNUSED__,
      void *event_info __UNUSED__)
{
   Evas_Object *fs = data;
   _populate(fs, getenv("HOME"), NULL);
}

static void
_ok(void            *data,
    Evas_Object *obj __UNUSED__,
    void *event_info __UNUSED__)
{
   Evas_Object *fs = data;
   evas_object_smart_callback_call(fs, SIG_DONE,
                                   (void *)elm_fileselector_selected_get(fs));
}

static void
_canc(void            *data,
      Evas_Object *obj __UNUSED__,
      void *event_info __UNUSED__)
{
   Evas_Object *fs = data;
   evas_object_smart_callback_call(fs, SIG_DONE, NULL);
}

static void
_anchor_clicked(void            *data,
                Evas_Object *obj __UNUSED__,
                void            *event_info)
{
   Evas_Object *fs = data;
   Widget_Data *wd = elm_widget_data_get(fs);
   Elm_Entry_Anchor_Info *info = event_info;
   const char *p;
   if (!wd) return;
   // keep a ref to path 'couse it will be destroyed by _populate
   p = eina_stringshare_add(info->name);
   _populate(fs, p, NULL);
   evas_object_smart_callback_call(data, SIG_SELECTED, (void *)p);
   eina_stringshare_del(p);
}

static void
_do_anchors(Evas_Object *obj,
            const char  *path)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   char **tok, buf[PATH_MAX * 3];
   int i, j;
   if (!wd) return;
   buf[0] = '\0';
   tok = eina_str_split(path, "/", 0);
   eina_strlcat(buf, "<a href=/>root</a>", sizeof(buf));
   for (i = 0; tok[i]; i++)
     {
        if ((!tok[i]) || (!tok[i][0])) continue;
        eina_strlcat(buf, wd->path_separator, sizeof(buf));
        eina_strlcat(buf, "<a href=", sizeof(buf));
        for (j = 0; j <= i; j++)
          {
             if (strlen(tok[j]) < 1) continue;
             eina_strlcat(buf, "/", sizeof(buf));
             eina_strlcat(buf, tok[j], sizeof(buf));
          }
        eina_strlcat(buf, ">", sizeof(buf));
        eina_strlcat(buf, tok[i], sizeof(buf));
        eina_strlcat(buf, "</a>", sizeof(buf));
     }
   free(tok[0]);
   free(tok);

   elm_entry_entry_set(wd->path_entry, buf);
}

static void
_populate(Evas_Object      *obj,
          const char       *path,
          Elm_Genlist_Item *parent)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_File_Direct_Info *file;
   Eina_Iterator *it;
   const char *real;
   Eina_List *files = NULL, *dirs = NULL;

   if ((!wd) || (!ecore_file_is_dir(path))) return;
   it = eina_file_stat_ls(path);
   if (!it) return ;
   evas_object_smart_callback_call(obj, SIG_DIRECTORY_OPEN, (void *)path);
   if (!parent)
     {
        elm_genlist_clear(wd->files_list);
        elm_gengrid_clear(wd->files_grid);
        eina_stringshare_replace(&wd->path, path);
        _do_anchors(obj, path);
     }

   if (wd->filename_entry) elm_entry_entry_set(wd->filename_entry, "");
   EINA_ITERATOR_FOREACH(it, file)
     {
        const char *filename;

        if (file->path[file->name_start] == '.')
          continue ;

        filename = eina_stringshare_add(file->path);
        if (file->type == EINA_FILE_DIR)
          dirs = eina_list_append(dirs, filename);
        else if (!wd->only_folder)
          files = eina_list_append(files, filename);
     }
   eina_iterator_free(it);

   files = eina_list_sort(files, eina_list_count(files),
                          EINA_COMPARE_CB(strcoll));
   dirs = eina_list_sort(dirs, eina_list_count(dirs), EINA_COMPARE_CB(strcoll));
   EINA_LIST_FREE(dirs, real)
     {
        if (wd->mode == ELM_FILESELECTOR_LIST)
          elm_genlist_item_append(wd->files_list, &list_itc,
                                  real, /* item data */
                                  parent,
                                  wd->expand ? ELM_GENLIST_ITEM_SUBITEMS :
                                  ELM_GENLIST_ITEM_NONE,
                                  NULL, NULL);
        else if (wd->mode == ELM_FILESELECTOR_GRID)
          elm_gengrid_item_append(wd->files_grid, &grid_itc,
                                  real, /* item data */
                                  NULL, NULL);
     }

   EINA_LIST_FREE(files, real)
     {
        if (wd->mode == ELM_FILESELECTOR_LIST)
          elm_genlist_item_append(wd->files_list, &list_itc,
                                  real, /* item data */
                                  parent, ELM_GENLIST_ITEM_NONE,
                                  NULL, NULL);
        else if (wd->mode == ELM_FILESELECTOR_GRID)
          elm_gengrid_item_append(wd->files_grid, &grid_itc,
                                  real, /* item data */
                                  NULL, NULL);
     }
}

/***  API  ***/

/**
 * Add a new Fileselector object
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Fileselector
 */
EAPI Evas_Object *
elm_fileselector_add(Evas_Object *parent)
{
   Evas *e;
   Evas_Object *obj, *ic, *bt, *li, *en, *grid;
   Widget_Data *wd;
   int s;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "fileselector");
   elm_widget_type_set(obj, "fileselector");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);

   wd->expand = !!_elm_config->fileselector_expand_enable;

   wd->edje = edje_object_add(e);
   _elm_theme_object_set(obj, wd->edje, "fileselector", "base", "default");
   elm_widget_resize_object_set(obj, wd->edje);

   // up btn
   ic = elm_icon_add(parent);
   elm_icon_standard_set(ic, "arrow_up");
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   bt = elm_button_add(parent);
   elm_widget_mirrored_automatic_set(bt, EINA_FALSE);
   elm_button_icon_set(bt, ic);
   elm_button_label_set(bt, E_("Up"));
   evas_object_size_hint_align_set(bt, 0.0, 0.0);

   evas_object_smart_callback_add(bt, "clicked", _up, obj);

   elm_widget_sub_object_add(obj, bt);
   wd->up_button = bt;

   // home btn
   ic = elm_icon_add(parent);
   elm_icon_standard_set(ic, "home");
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   bt = elm_button_add(parent);
   elm_widget_mirrored_automatic_set(bt, EINA_FALSE);
   elm_button_icon_set(bt, ic);
   elm_button_label_set(bt, E_("Home"));
   evas_object_size_hint_align_set(bt, 0.0, 0.0);

   evas_object_smart_callback_add(bt, "clicked", _home, obj);

   elm_widget_sub_object_add(obj, bt);
   wd->home_button = bt;

   list_itc.item_style = grid_itc.item_style = "default";
   list_itc.func.label_get = grid_itc.func.label_get = _itc_label_get;
   list_itc.func.icon_get = grid_itc.func.icon_get = _itc_icon_get;
   list_itc.func.state_get = grid_itc.func.state_get = _itc_state_get;
   list_itc.func.del = grid_itc.func.del = _itc_del;

   li = elm_genlist_add(parent);
   elm_widget_mirrored_automatic_set(li, EINA_FALSE);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_min_set(li, 100, 100);

   grid = elm_gengrid_add(parent);
   elm_widget_mirrored_automatic_set(grid, EINA_FALSE);
   evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   s = elm_finger_size_get() * 2;
   elm_gengrid_item_size_set(grid, s, s);
   elm_gengrid_align_set(grid, 0.0, 0.0);

   evas_object_smart_callback_add(li, "selected", _sel, obj);
   evas_object_smart_callback_add(li, "expand,request", _expand_req, obj);
   evas_object_smart_callback_add(li, "contract,request", _contract_req, obj);
   evas_object_smart_callback_add(li, "expanded", _expand_done, obj);
   evas_object_smart_callback_add(li, "contracted", _contract_done, obj);

   evas_object_smart_callback_add(grid, "selected", _sel, obj);

   elm_widget_sub_object_add(obj, li);
   elm_widget_sub_object_add(obj, grid);
   wd->files_list = li;
   wd->files_grid = grid;

   // path entry
   en = elm_entry_add(parent);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_widget_mirrored_automatic_set(en, EINA_FALSE);
   elm_entry_editable_set(en, EINA_FALSE);
   elm_entry_single_line_set(en, EINA_TRUE);
   elm_entry_line_wrap_set(en, ELM_WRAP_CHAR);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);

   evas_object_smart_callback_add(en, "anchor,clicked", _anchor_clicked, obj);

   elm_widget_sub_object_add(obj, en);
   wd->path_entry = en;

   // filename entry
   en = elm_entry_add(parent);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_widget_mirrored_automatic_set(en, EINA_FALSE);
   elm_entry_editable_set(en, EINA_TRUE);
   elm_entry_single_line_set(en, EINA_TRUE);
   elm_entry_line_wrap_set(en, ELM_WRAP_CHAR);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);

   elm_widget_sub_object_add(obj, en);
   wd->filename_entry = en;

   elm_fileselector_buttons_ok_cancel_set(obj, EINA_TRUE);
   elm_fileselector_is_save_set(obj, EINA_FALSE);

   _theme_hook(obj);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   return obj;
}

/**
 * This enables/disables the file name entry box where the user can
 * type in a name for the file to be saved as.
 *
 * @param obj The fileselector object
 * @param is_save If true, the fileselector is a save dialog
 *
 * @ingroup Fileselector
 */
EAPI void
elm_fileselector_is_save_set(Evas_Object *obj,
                             Eina_Bool    is_save)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   elm_object_disabled_set(wd->filename_entry, !is_save);

   if (is_save)
     edje_object_signal_emit(wd->edje, "elm,state,save,on", "elm");
   else
     edje_object_signal_emit(wd->edje, "elm,state,save,off", "elm");
}

/**
 * This returns whether the fileselector is a "save" type fileselector
 *
 * @param obj The fileselector object
 * @return If true, the fileselector is a save type.
 *
 * @ingroup Fileselector
 */
EAPI Eina_Bool
elm_fileselector_is_save_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return elm_object_disabled_get(wd->filename_entry);
}

/**
 * This enables/disables folder-only view in the fileselector.
 *
 * @param obj The fileselector object
 * @param only If true, the fileselector will only display directories.
 * If false, files are displayed also.
 *
 * @ingroup Fileselector
 */
EAPI void
elm_fileselector_folder_only_set(Evas_Object *obj,
                                 Eina_Bool    only)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->only_folder == only) return;
   wd->only_folder = !!only;
   if (wd->path) _populate(obj, wd->path, NULL);
}

/**
 * This gets the state of file display in the fileselector.
 *
 * @param obj The fileselector object
 * @return If true, files are not being shown in the fileselector.
 * If false, files are being shown.
 *
 * @ingroup Fileselector
 */
EAPI Eina_Bool
elm_fileselector_folder_only_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->only_folder;
}

/**
 * This enables/disables the ok,cancel buttons.
 *
 * @param obj The fileselector object
 * @param only If true, a box containing ok and cancel buttons is created.
 * If false, the box and the buttons are destroyed.
 *
 * @ingroup Fileselector
 */
EAPI void
elm_fileselector_buttons_ok_cancel_set(Evas_Object *obj,
                                       Eina_Bool    visible)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *bt;
   if (!wd) return;

   if (visible)
     {
        // cancel btn
        bt = elm_button_add(obj);
        elm_widget_mirrored_automatic_set(bt, EINA_FALSE);
        elm_button_label_set(bt, E_("Cancel"));

        evas_object_smart_callback_add(bt, "clicked", _canc, obj);

        elm_widget_sub_object_add(obj, bt);
        wd->cancel_button = bt;

        // ok btn
        bt = elm_button_add(obj);
        elm_widget_mirrored_automatic_set(bt, EINA_FALSE);
        elm_button_label_set(bt, E_("OK"));

        evas_object_smart_callback_add(bt, "clicked", _ok, obj);

        elm_widget_sub_object_add(obj, bt);
        wd->ok_button = bt;

        _theme_hook(obj);
     }
   else
     {
        evas_object_del(wd->cancel_button);
        wd->cancel_button = NULL;
        evas_object_del(wd->ok_button);
        wd->ok_button = NULL;
     }
}

/**
 * This gets the state of the box containing ok and cancel buttons.
 *
 * @param obj The fileselector object
 * @return If true, the box exists.
 * If false, the box does not exist.
 *
 * @ingroup Fileselector
 */
EAPI Eina_Bool
elm_fileselector_buttons_ok_cancel_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->ok_button ? EINA_TRUE : EINA_FALSE;
}

/**
 * This enables a tree view in the fileselector, <b>if in @c
 * ELM_FILESELECTOR_LIST mode</b>. If it's in other mode, the changes
 * made by this function will only be visible when one switches back
 * to list mode.
 *
 * @param obj The fileselector object
 * @param expand If true, tree view is enabled.
 * If false, tree view is disabled.
 *
 * In a tree view, arrows are created on the sides of directories,
 * allowing them to expand in place.
 *
 * @ingroup Fileselector
 */
EAPI void
elm_fileselector_expandable_set(Evas_Object *obj,
                                Eina_Bool    expand)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   wd->expand = !!expand;

   if (wd->path) _populate(obj, wd->path, NULL);
}

/**
 * This gets the state of tree view in the fileselector.
 *
 * @param obj The fileselector object
 * @return If true, tree view is enabled and folders will be expandable.
 * If false, tree view is disabled.
 *
 * @ingroup Fileselector
 */
EAPI Eina_Bool
elm_fileselector_expandable_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->expand;
}

/**
 * This sets the path that the fileselector will display.
 *
 * @param obj The fileselector object
 * @param path The path of the fileselector
 *
 * @ingroup Fileselector
 */
EAPI void
elm_fileselector_path_set(Evas_Object *obj,
                          const char  *path)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   _populate(obj, path, NULL);
}

/**
 * This gets the path that the fileselector displays.
 *
 * @param obj The fileselector object
 * @return The path that the fileselector is displaying
 *
 * @ingroup Fileselector
 */
EAPI const char *
elm_fileselector_path_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->path;
}

/**
 * This sets the mode in which the fileselector will display files.
 *
 * @param obj The fileselector object

 * @param mode The mode of the fileselector, being it one of @c
 * ELM_FILESELECTOR_LIST (default) or @c ELM_FILESELECTOR_GRID. The
 * first one, naturally, will display the files in a list. By using
 * elm_fileselector_expandable_set(), the user will trigger a tree
 * view for that list. The latter will make the widget to display its
 * entries in a grid form.
 *
 * @see elm_fileselector_expandable_set().
 *
 * @ingroup Fileselector
 */
EAPI void
elm_fileselector_mode_set(Evas_Object          *obj,
                          Elm_Fileselector_Mode mode)
{
   ELM_CHECK_WIDTYPE(obj, widtype);

   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (mode == wd->mode) return;

   if (mode == ELM_FILESELECTOR_LIST)
     {
        if (edje_object_part_swallow(wd->edje, "elm.swallow.files",
                                     wd->files_list))
          {
             evas_object_show(wd->files_list);
             evas_object_hide(wd->files_grid);
          }
        else
          evas_object_hide(wd->files_list);
     }
   else
     {
        if (edje_object_part_swallow(wd->edje, "elm.swallow.files",
                                     wd->files_grid))
          {
             evas_object_show(wd->files_grid);
             evas_object_hide(wd->files_list);
          }
        else
          evas_object_hide(wd->files_grid);
     }

   wd->mode = mode;

   _populate(obj, wd->path, NULL);
}

/**
 * This gets the mode in which the fileselector is displaying files.
 *
 * @param obj The fileselector object
 * @return The mode in which the fileselector is at
 *
 * @ingroup Fileselector
 */
EAPI Elm_Fileselector_Mode
elm_fileselector_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_FILESELECTOR_LAST;

   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return ELM_FILESELECTOR_LAST;

   return wd->mode;
}

/**
 * This gets the currently selected path in the file selector.
 *
 * @param obj The file selector object
 * @return The absolute path of the selected object in the fileselector
 *
 * @ingroup Fileselector
 */
EAPI const char *
elm_fileselector_selected_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   if (wd->filename_entry)
     {
        const char *name;
        char buf[PATH_MAX];

        name = elm_entry_entry_get(wd->filename_entry);
        snprintf(buf, sizeof(buf), "%s/%s",
                 wd->only_folder ? ecore_file_dir_get(wd->path) : wd->path,
                 name);
        eina_stringshare_replace(&wd->selection, buf);
        return wd->selection;
     }

   if (wd->mode == ELM_FILESELECTOR_LIST)
     {
        Elm_Genlist_Item *it;
        it = elm_genlist_selected_item_get(wd->files_list);
        if (it) return elm_genlist_item_data_get(it);
     }
   else
     {
        Elm_Gengrid_Item *it;
        it = elm_gengrid_selected_item_get(wd->files_grid);
        if (it) return elm_gengrid_item_data_get(it);
     }

   return wd->path;
}

/**
 * This sets the currently selected path in the file selector.
 *
 * @param obj The file selector object
 * @param path The path to a file or directory
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure. The
 * latter case occurs if the directory or file pointed to do not
 * exist.
 *
 * @ingroup Fileselector
 */
EAPI Eina_Bool
elm_fileselector_selected_set(Evas_Object *obj,
                              const char  *path)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;

   if (ecore_file_is_dir(path))
     _populate(obj, path, NULL);
   else
     {
        if (!ecore_file_exists(path))
          return EINA_FALSE;

        _populate(obj, ecore_file_dir_get(path), NULL);
        if (wd->filename_entry)
          {
             elm_entry_entry_set(wd->filename_entry,
                                          ecore_file_file_get(path));
             eina_stringshare_replace(&wd->selection, path);
          }
     }

   return EINA_TRUE;
}

