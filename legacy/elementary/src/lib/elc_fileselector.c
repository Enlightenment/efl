/*
 * TODO:
 *  - child elements focusing support
 *  - user defined icon/label cb
 *  - show/hide/add buttons ???
 *  - show/hide hidden files
 *  - double click to choose a file
 *  - multi-selection
 *  - make variable/function names that are sensible
 *  - Filter support
 */
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_fileselector.h"

EAPI Eo_Op ELM_OBJ_FILESELECTOR_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_FILESELECTOR_CLASS

#define MY_CLASS_NAME "elm_fileselector"

static Elm_Genlist_Item_Class *list_itc[ELM_FILE_LAST];
static Elm_Gengrid_Item_Class *grid_itc[ELM_FILE_LAST];

static const char SIG_DIRECTORY_OPEN[] = "directory,open";
static const char SIG_DONE[] = "done";
static const char SIG_SELECTED[] = "selected";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_DIRECTORY_OPEN, "s"},
   {SIG_DONE, "s"},
   {SIG_SELECTED, "s"},
   {NULL, NULL}
};

/* final routine on deletion */
static void
_elm_fileselector_smart_del_do(Elm_Fileselector_Smart_Data *sd)
{
   if (sd->path) eina_stringshare_del(sd->path);
   if (sd->selection) eina_stringshare_del(sd->selection);
   if (sd->sel_idler) free(ecore_idler_del(sd->sel_idler));

   eo_do_super(sd->obj, MY_CLASS, evas_obj_smart_del());
}

static void
_elm_fileselector_smart_sizing_eval(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Evas_Coord minw = -1, minh = -1;

   Elm_Widget_Smart_Data *wd = eo_data_scope_get(obj, ELM_OBJ_WIDGET_CLASS);

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   ELM_FILESELECTOR_DATA_GET(obj, sd);

   elm_widget_mirrored_set(sd->cancel_button, rtl);
   elm_widget_mirrored_set(sd->ok_button, rtl);
   elm_widget_mirrored_set(sd->files_list, rtl);
   elm_widget_mirrored_set(sd->up_button, rtl);
   elm_widget_mirrored_set(sd->home_button, rtl);
}

static void
_elm_fileselector_smart_theme(Eo *obj, void *_pd, va_list *list)
{
   const char *style;
   const char *data;
   char buf[1024];

   Elm_Fileselector_Smart_Data *sd = _pd;
   Elm_Widget_Smart_Data *wd = eo_data_scope_get(obj, ELM_OBJ_WIDGET_CLASS);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;

   eo_do_super(obj, MY_CLASS, elm_wdg_theme(&int_ret));
   if (!int_ret) return;

   style = elm_widget_style_get(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   data = edje_object_data_get
       (wd->resize_obj, "path_separator");
   if (data) sd->path_separator = data;
   else sd->path_separator = "/";

   snprintf(buf, sizeof(buf), "fileselector/%s", style);

#define SWALLOW(part_name, object_ptn)                          \
  if (object_ptn)                                               \
    {                                                           \
       elm_widget_style_set(object_ptn, buf);                   \
       if (!elm_layout_content_set(obj, part_name, object_ptn)) \
         evas_object_hide(object_ptn);                          \
    }

   SWALLOW("elm.swallow.up", sd->up_button);
   SWALLOW("elm.swallow.home", sd->home_button);
   SWALLOW("elm.swallow.spinner", sd->spinner);
   elm_object_style_set(sd->spinner, "wheel");

   if (sd->mode == ELM_FILESELECTOR_LIST)
     {
        if (elm_layout_content_set(obj, "elm.swallow.files", sd->files_list))
          evas_object_hide(sd->files_grid);
        else evas_object_hide(sd->files_list);
     }
   else
     {
        if (elm_layout_content_set(obj, "elm.swallow.files", sd->files_grid))
          evas_object_hide(sd->files_list);
        else evas_object_hide(sd->files_grid);
     }

   SWALLOW("elm.swallow.path", sd->path_entry);

   snprintf(buf, sizeof(buf), "fileselector/actions/%s", style);
   SWALLOW("elm.swallow.cancel", sd->cancel_button);
   SWALLOW("elm.swallow.ok", sd->ok_button);

#undef SWALLOW

   edje_object_message_signal_process(wd->resize_obj);

   elm_layout_sizing_eval(obj);

   if (ret) *ret = EINA_TRUE;
}

/***  GENLIST/GENGRID "MODEL"  ***/
static char *
_itc_text_get(void *data,
              Evas_Object *obj   __UNUSED__,
              const char *source __UNUSED__)
{
   return elm_entry_utf8_to_markup
            (ecore_file_file_get(data)); /* NOTE this will be free()'d by
                                          * the caller */
}

static Evas_Object *
_itc_icon_folder_get(void *data __UNUSED__,
                     Evas_Object *obj,
                     const char *source)
{
   Evas_Object *ic;

   if (strcmp(source, "elm.swallow.icon")) return NULL;

   ic = elm_icon_add(obj);
   elm_icon_standard_set(ic, "folder");

   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   return ic;
}

static Evas_Object *
_itc_icon_image_get(void *data,
                    Evas_Object *obj,
                    const char *source)
{
   const char *filename = data;
   Evas_Object *ic;

   if (strcmp(source, "elm.swallow.icon")) return NULL;

   ic = elm_icon_add(obj);
   elm_icon_standard_set(ic, "image");
   elm_icon_thumb_set(ic, filename, NULL);

   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   return ic;
}

static Evas_Object *
_itc_icon_file_get(void *data __UNUSED__,
                   Evas_Object *obj,
                   const char *source)
{
   Evas_Object *ic;

   if (strcmp(source, "elm.swallow.icon")) return NULL;

   ic = elm_icon_add(obj);
   elm_icon_standard_set(ic, "file");

   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   return ic;
}

static Eina_Bool
_itc_state_get(void *data         __UNUSED__,
               Evas_Object *obj   __UNUSED__,
               const char *source __UNUSED__)
{
   return EINA_FALSE;
}

static void
_itc_del(void *data,
         Evas_Object *obj __UNUSED__)
{
   eina_stringshare_del(data);
}

static void
_anchors_do(Evas_Object *obj,
            const char *path)
{
   char **tok, buf[PATH_MAX * 3], *s;
   int i, j;

   ELM_FILESELECTOR_DATA_GET(obj, sd);

   s = elm_entry_utf8_to_markup(path);
   if (!s) return;

   buf[0] = '\0';
   tok = eina_str_split(s, "/", 0);
   free(s);

   eina_strlcat(buf, "<a href=/>root</a>", sizeof(buf));
   for (i = 0; tok[i]; i++)
     {
        if ((!tok[i]) || (!tok[i][0])) continue;
        eina_strlcat(buf, sd->path_separator, sizeof(buf));
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

   elm_object_text_set(sd->path_entry, buf);
}

#ifdef HAVE_EIO
static Eina_Bool
_ls_filter_cb(void *data,
              Eio_File *handler __UNUSED__,
              const Eina_File_Direct_Info *info)
{
   Listing_Request *lreq = data;

   if (info->path[info->name_start] == '.')
     return EINA_FALSE;

   if (lreq->sd->only_folder && info->type != EINA_FILE_DIR)
     return EINA_FALSE;

   return EINA_TRUE;
}

static int
_file_grid_cmp(const void *a,
               const void *b)
{
   const Elm_Object_Item *ga = a;
   const Elm_Object_Item *gb = b;
   const Elm_Gengrid_Item_Class *ca = elm_gengrid_item_item_class_get(ga);
   const Elm_Gengrid_Item_Class *cb = elm_gengrid_item_item_class_get(gb);

   if (ca == grid_itc[ELM_DIRECTORY])
     {
        if (cb != grid_itc[ELM_DIRECTORY])
          return -1;
     }
   else if (cb == grid_itc[ELM_DIRECTORY])
     {
        return 1;
     }

   return strcoll(elm_object_item_data_get(ga), elm_object_item_data_get(gb));
}

static int
_file_list_cmp(const void *a,
               const void *b)
{
   const Elm_Object_Item *la = a;
   const Elm_Object_Item *lb = b;
   const Elm_Genlist_Item_Class *ca = elm_genlist_item_item_class_get(la);
   const Elm_Genlist_Item_Class *cb = elm_genlist_item_item_class_get(lb);

   if (ca == list_itc[ELM_DIRECTORY])
     {
        if (cb != list_itc[ELM_DIRECTORY])
          return -1;
     }
   else if (cb == list_itc[ELM_DIRECTORY])
     {
        return 1;
     }

   return strcoll(elm_object_item_data_get(la), elm_object_item_data_get(lb));
}

static void
_signal_first(Listing_Request *lreq)
{
   if (!lreq->first) return;

   evas_object_smart_callback_call
     (lreq->obj, SIG_DIRECTORY_OPEN, (void *)lreq->path);

   if (!lreq->parent_it)
     {
        elm_genlist_clear(lreq->sd->files_list);
        elm_gengrid_clear(lreq->sd->files_grid);
        eina_stringshare_replace(&lreq->sd->path, lreq->path);
        _anchors_do(lreq->obj, lreq->path);
     }

   lreq->first = EINA_FALSE;
}

static void
_ls_main_cb(void *data,
            Eio_File *handler,
            const Eina_File_Direct_Info *info)
{
   Listing_Request *lreq = data;
   int itcn = ELM_FILE_UNKNOW;

   if (eio_file_check(handler)) return;

   if (!lreq->sd->files_list || !lreq->sd->files_grid
       || lreq->sd->current != handler)
     {
        eio_file_cancel(handler);
        return;
     }

   _signal_first(lreq);

   if (info->type == EINA_FILE_DIR)
     itcn = ELM_DIRECTORY;
   else
     {
        if (evas_object_image_extension_can_load_get
              (info->path + info->name_start))
          itcn = ELM_FILE_IMAGE;
     }

   if (lreq->sd->mode == ELM_FILESELECTOR_LIST)
     elm_genlist_item_sorted_insert(lreq->sd->files_list, list_itc[itcn],
                                    eina_stringshare_add(info->path),
                                    lreq->parent_it,
                                    ((lreq->sd->expand) && (itcn == ELM_DIRECTORY))
                                    ? ELM_GENLIST_ITEM_TREE : ELM_GENLIST_ITEM_NONE,
                                    _file_list_cmp, NULL, NULL);
   else if (lreq->sd->mode == ELM_FILESELECTOR_GRID)
     elm_gengrid_item_sorted_insert(lreq->sd->files_grid, grid_itc[itcn],
                                    eina_stringshare_add(info->path),
                                    _file_grid_cmp, NULL, NULL);
}

static void
_listing_request_cleanup(Listing_Request *lreq)
{
   EINA_REFCOUNT_UNREF(lreq->sd) _elm_fileselector_smart_del_do(lreq->sd);

   eina_stringshare_del(lreq->path);
   free(lreq);
}

static void
_ls_done_cb(void *data, Eio_File *handler __UNUSED__)
{
   Listing_Request *lreq = data;

   _signal_first(lreq);
   elm_progressbar_pulse(lreq->sd->spinner, EINA_FALSE);
   elm_layout_signal_emit(lreq->obj, "elm,action,spinner,hide", "elm");

   lreq->sd->current = NULL;
   _listing_request_cleanup(lreq);
}

static void
_ls_error_cb(void *data, Eio_File *handler, int error __UNUSED__)
{
   Listing_Request *lreq = data;

   elm_progressbar_pulse(lreq->sd->spinner, EINA_FALSE);
   elm_layout_signal_emit(lreq->obj, "elm,action,spinner,hide", "elm");

   if (lreq->sd->current == handler)
     lreq->sd->current = NULL;
   _listing_request_cleanup(lreq);
}

#endif

static void
_populate(Evas_Object *obj,
          const char *path,
          Elm_Object_Item *parent_it)
{
   ELM_FILESELECTOR_DATA_GET(obj, sd);

#ifdef HAVE_EIO
   Listing_Request *lreq;
#else
   Eina_List *files = NULL, *dirs = NULL;
   Eina_File_Direct_Info *file;
   Eina_Iterator *it;
   const char *entry;
#endif

#ifndef HAVE_EIO /* synchronous listing path */
   if (!ecore_file_is_dir(path)) return;

   it = eina_file_stat_ls(path);
   if (!it) return;

   evas_object_smart_callback_call(obj, SIG_DIRECTORY_OPEN, (void *)path);
   if (!parent_it)
     {
        elm_genlist_clear(sd->files_list);
        elm_gengrid_clear(sd->files_grid);
        eina_stringshare_replace(&sd->path, path);
        _anchors_do(obj, path);
     }

   EINA_ITERATOR_FOREACH(it, file)
     {
        const char *filename;

        if (file->path[file->name_start] == '.') continue;

        filename = eina_stringshare_add(file->path);
        if (file->type == EINA_FILE_DIR)
          dirs = eina_list_append(dirs, filename);
        else if (!sd->only_folder)
          files = eina_list_append(files, filename);
     }
   eina_iterator_free(it);

   files = eina_list_sort
       (files, eina_list_count(files), EINA_COMPARE_CB(strcoll));

   dirs = eina_list_sort
       (dirs, eina_list_count(dirs), EINA_COMPARE_CB(strcoll));
   EINA_LIST_FREE(dirs, entry)
     {
        if (sd->mode == ELM_FILESELECTOR_LIST)
          elm_genlist_item_append(sd->files_list, list_itc[ELM_DIRECTORY],
                                  entry, /* item data */
                                  parent_it,
                                  (sd->expand)
                                  ? ELM_GENLIST_ITEM_TREE : ELM_GENLIST_ITEM_NONE,
                                  NULL, NULL);
        else if (sd->mode == ELM_FILESELECTOR_GRID)
          elm_gengrid_item_append(sd->files_grid, grid_itc[ELM_DIRECTORY],
                                  entry, /* item data */
                                  NULL, NULL);
     }

   EINA_LIST_FREE(files, entry)
     {
        Elm_Fileselector_Type type =
          evas_object_image_extension_can_load_fast_get(entry) ?
          ELM_FILE_IMAGE : ELM_FILE_UNKNOW;

        if (sd->mode == ELM_FILESELECTOR_LIST)
          elm_genlist_item_append(sd->files_list, list_itc[type],
                                  entry, /* item data */
                                  parent_it, ELM_GENLIST_ITEM_NONE,
                                  NULL, NULL);
        else if (sd->mode == ELM_FILESELECTOR_GRID)
          elm_gengrid_item_append(sd->files_grid, grid_itc[type],
                                  entry, /* item data */
                                  NULL, NULL);
     }

#else /* asynchronous listing path */
   if (sd->expand && sd->current) return;

   if (sd->current) eio_file_cancel(sd->current);

   lreq = malloc(sizeof (Listing_Request));
   if (!lreq) return;

   lreq->sd = sd;
   EINA_REFCOUNT_REF(lreq->sd);

   lreq->parent_it = parent_it; /* FIXME: should we refcount the parent_it ? */
   lreq->obj = obj;
   lreq->path = eina_stringshare_add(path);
   lreq->first = EINA_TRUE;

   sd->current = eio_file_stat_ls(path, _ls_filter_cb, _ls_main_cb,
                                  _ls_done_cb, _ls_error_cb, lreq);
   elm_progressbar_pulse(sd->spinner, EINA_TRUE);
   elm_layout_signal_emit(lreq->obj, "elm,action,spinner,show", "elm");
#endif
}

static void
_on_list_expanded(void *data,
                  Evas_Object *obj __UNUSED__,
                  void *event_info)
{
   Elm_Object_Item *it = event_info;
   const char *path = elm_object_item_data_get(it);

   _populate(data, path, it);
}

static void
_on_list_contracted(void *data       __UNUSED__,
                    Evas_Object *obj __UNUSED__,
                    void *event_info)
{
   Elm_Object_Item *it = event_info;

   elm_genlist_item_subitems_clear(it);
}

static void
_on_list_expand_req(void *data       __UNUSED__,
                    Evas_Object *obj __UNUSED__,
                    void *event_info)
{
   Elm_Object_Item *it = event_info;

   elm_genlist_item_expanded_set(it, EINA_TRUE);
}

static void
_on_list_contract_req(void *data       __UNUSED__,
                      Evas_Object *obj __UNUSED__,
                      void *event_info)
{
   Elm_Object_Item *it = event_info;

   elm_genlist_item_expanded_set(it, EINA_FALSE);
}

static Eina_Bool
_sel_do(void *data)
{
   struct sel_data *sdata = data;
   const char *path;
   const char *p;

   ELM_FILESELECTOR_DATA_GET(sdata->fs, sd);
   path = sdata->path;

   if ((!sd->only_folder) && ecore_file_is_dir(path))
     {
        if (sd->expand && sd->mode == ELM_FILESELECTOR_LIST)
          {
             _anchors_do(sdata->fs, path);
          }
        else
          {
             /* keep a ref to path 'couse it will be destroyed by _populate */
             p = eina_stringshare_add(path);
             _populate(sdata->fs, p, NULL);
             eina_stringshare_del(p);
          }
        goto end;
     }
   else /* navigating through folders only or file is not a dir. */
     {
        if (sd->expand && sd->mode == ELM_FILESELECTOR_LIST)
          _anchors_do(sdata->fs, path);
        else if (sd->only_folder)
          {
             /* keep a ref to path 'couse it will be destroyed by _populate */
             p = eina_stringshare_add(path);
             _populate(sdata->fs, p, NULL);
             eina_stringshare_del(p);
          }
     }

   evas_object_smart_callback_call(sdata->fs, SIG_SELECTED, (void *)path);

end:
   sd->sel_idler = NULL;
   free(sdata);
   return ECORE_CALLBACK_CANCEL;
}

static void
_on_item_selected(void *data,
                  Evas_Object *obj __UNUSED__,
                  void *event_info)
{
   //This event_info could be a list or gengrid item
   Elm_Object_Item *it = event_info;
   struct sel_data *sdata;
   void *old_sd;
   char *dir;

   ELM_FILESELECTOR_DATA_GET(data, sd);

   sdata = malloc(sizeof(*sdata));
   sdata->fs = data;
   sdata->path = elm_object_item_data_get(it);

   if (!sdata->path)
     {
        eina_stringshare_replace(&sd->path, "");
        goto end;
     }

   dir = sd->only_folder ? strdup(sdata->path) :
     ecore_file_dir_get(sdata->path);
   if (dir)
     {
        eina_stringshare_replace(&sd->path, dir);
        free(dir);
     }
   else
     {
        eina_stringshare_replace(&sd->path, "");
     }

end:
   if (sd->sel_idler)
     {
        old_sd = ecore_idler_del(sd->sel_idler);
        free(old_sd);
     }
   sd->sel_idler = ecore_idler_add(_sel_do, sdata);
}

static void
_on_dir_up(void *data,
           Evas_Object *obj __UNUSED__,
           void *event_info __UNUSED__)
{
   Evas_Object *fs = data;
   char *parent;

   ELM_FILESELECTOR_DATA_GET(fs, sd);

   parent = ecore_file_dir_get(sd->path);
   _populate(fs, parent, NULL);
   free(parent);
}

static void
_home(void *data,
      Evas_Object *obj __UNUSED__,
      void *event_info __UNUSED__)
{
   Evas_Object *fs = data;

   _populate(fs, getenv("HOME"), NULL);
}

static void
_ok(void *data,
    Evas_Object *obj __UNUSED__,
    void *event_info __UNUSED__)
{
   Evas_Object *fs = data;

   evas_object_smart_callback_call
     (fs, SIG_DONE, (void *)elm_fileselector_selected_get(fs));
}

static void
_canc(void *data,
      Evas_Object *obj __UNUSED__,
      void *event_info __UNUSED__)
{
   Evas_Object *fs = data;

   evas_object_smart_callback_call(fs, SIG_DONE, NULL);
}

static void
_on_text_clicked(void *data __UNUSED__,
                 Evas_Object *obj,
                 void *event_info __UNUSED__)
{
   ELM_FILESELECTOR_DATA_GET(data, sd);

   elm_entry_entry_set(obj, sd->path);
   elm_entry_cursor_pos_set(obj, strlen(sd->path));
}

static void
_on_text_activated(void *data,
		   Evas_Object *obj,
		   void *event_info __UNUSED__)
{
   Evas_Object *fs = data;
   const char *p;

   // keep a ref to path 'couse it will be destroyed by _populate
   p = eina_stringshare_add(elm_object_text_get(obj));
   if (ecore_file_is_dir(p))
     _populate(fs, p, NULL);
   else
     evas_object_smart_callback_call(data, SIG_SELECTED, (void *)p);

   eina_stringshare_del(p);
}

static void
_on_text_unfocused(void *data __UNUSED__,
		   Evas_Object *obj,
		   void *event_info __UNUSED__)
{
   _anchors_do(obj, elm_object_text_get(obj));
}

static void
_anchor_clicked(void *data,
                Evas_Object *obj __UNUSED__,
                void *event_info)
{
   Elm_Entry_Anchor_Info *info = event_info;
   Evas_Object *fs = data;
   const char *p;

   // keep a ref to path 'couse it will be destroyed by _populate
   p = eina_stringshare_add(info->name);
   _populate(fs, p, NULL);
   evas_object_smart_callback_call(data, SIG_SELECTED, (void *)p);
   eina_stringshare_del(p);
}

static void
_elm_fileselector_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Object *ic, *bt, *li, *en, *grid, *pb;
   unsigned int i;
   int s;

   Elm_Fileselector_Smart_Data *priv = _pd;

   EINA_REFCOUNT_INIT(priv);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());

   elm_widget_can_focus_set(obj, EINA_FALSE);

   priv->expand = !!_elm_config->fileselector_expand_enable;

   if (!elm_layout_theme_set
       (obj, "fileselector", "base", elm_widget_style_get(obj)))
     CRITICAL("Failed to set layout!");

   // up btn
   ic = elm_icon_add(obj);
   elm_icon_standard_set(ic, "arrow_up");
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   bt = elm_button_add(obj);
   elm_widget_mirrored_automatic_set(bt, EINA_FALSE);
   elm_object_part_content_set(bt, "icon", ic);
   elm_object_domain_translatable_text_set(bt, PACKAGE, N_("Up"));
   evas_object_size_hint_align_set(bt, 0.0, 0.0);
   evas_object_smart_callback_add(bt, "clicked", _on_dir_up, obj);

   elm_widget_sub_object_add(obj, bt);

   priv->up_button = bt;

   // home btn
   ic = elm_icon_add(obj);
   elm_icon_standard_set(ic, "home");
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   bt = elm_button_add(obj);
   elm_widget_mirrored_automatic_set(bt, EINA_FALSE);
   elm_object_part_content_set(bt, "icon", ic);
   elm_object_domain_translatable_text_set(bt, PACKAGE, N_("Home"));
   evas_object_size_hint_align_set(bt, 0.0, 0.0);
   evas_object_smart_callback_add(bt, "clicked", _home, obj);

   elm_widget_sub_object_add(obj, bt);
   priv->home_button = bt;

   // spinner
   pb = elm_progressbar_add(obj);
   elm_widget_sub_object_add(obj, pb);
   priv->spinner = pb;

   for (i = 0; i < ELM_FILE_LAST; ++i)
     {
        list_itc[i] = elm_genlist_item_class_new();
        grid_itc[i] = elm_gengrid_item_class_new();

        list_itc[i]->item_style = "default";
        list_itc[i]->func.text_get = grid_itc[i]->func.text_get =
            _itc_text_get;
        list_itc[i]->func.state_get = grid_itc[i]->func.state_get =
            _itc_state_get;
        list_itc[i]->func.del = grid_itc[i]->func.del = _itc_del;
     }

   list_itc[ELM_DIRECTORY]->func.content_get =
     grid_itc[ELM_DIRECTORY]->func.content_get = _itc_icon_folder_get;
   list_itc[ELM_FILE_IMAGE]->func.content_get =
     grid_itc[ELM_FILE_IMAGE]->func.content_get = _itc_icon_image_get;
   list_itc[ELM_FILE_UNKNOW]->func.content_get =
     grid_itc[ELM_FILE_UNKNOW]->func.content_get = _itc_icon_file_get;

   li = elm_genlist_add(obj);
   elm_widget_mirrored_automatic_set(li, EINA_FALSE);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_min_set(li, 100, 100);

   grid = elm_gengrid_add(obj);
   elm_widget_mirrored_automatic_set(grid, EINA_FALSE);
   evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   // XXX: will fail for dynamic finger size changing
   s = elm_config_finger_size_get() * 2;
   elm_gengrid_item_size_set(grid, s, s);
   elm_gengrid_align_set(grid, 0.0, 0.0);

   evas_object_smart_callback_add(li, "selected", _on_item_selected, obj);
   evas_object_smart_callback_add
     (li, "expand,request", _on_list_expand_req, obj);
   evas_object_smart_callback_add
     (li, "contract,request", _on_list_contract_req, obj);
   evas_object_smart_callback_add(li, "expanded", _on_list_expanded, obj);
   evas_object_smart_callback_add(li, "contracted", _on_list_contracted, obj);
   evas_object_smart_callback_add(grid, "selected", _on_item_selected, obj);

   elm_widget_sub_object_add(obj, li);
   elm_widget_sub_object_add(obj, grid);
   priv->files_list = li;
   priv->files_grid = grid;

   // path entry
   en = elm_entry_add(obj);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_widget_mirrored_automatic_set(en, EINA_FALSE);
   elm_entry_single_line_set(en, EINA_TRUE);
   elm_entry_line_wrap_set(en, ELM_WRAP_CHAR);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);

   evas_object_smart_callback_add(en, "anchor,clicked", _anchor_clicked, obj);
   evas_object_smart_callback_add(en, "clicked", _on_text_clicked, obj);
   evas_object_smart_callback_add(en, "unfocused", _on_text_unfocused, obj);
   evas_object_smart_callback_add(en, "activated", _on_text_activated, obj);

   elm_widget_sub_object_add(obj, en);
   priv->path_entry = en;

   elm_fileselector_buttons_ok_cancel_set(obj, EINA_TRUE);
   elm_fileselector_is_save_set(obj, EINA_FALSE);

   eo_do(obj, elm_wdg_theme(NULL));
}

static void
_elm_fileselector_smart_del(Eo *obj EINA_UNUSED, void *_pd, va_list *list EINA_UNUSED)
{
   int i;

   Elm_Fileselector_Smart_Data *sd = _pd;

   for (i = 0; i < ELM_FILE_LAST; ++i)
     {
        elm_genlist_item_class_free(list_itc[i]);
        elm_gengrid_item_class_free(grid_itc[i]);
     }

#ifdef HAVE_EIO
   if (sd->current) eio_file_cancel(sd->current);
#endif

   sd->files_list = NULL;
   sd->files_grid = NULL;

   /* this one matching EINA_REFCOUNT_INIT() */
   EINA_REFCOUNT_UNREF(sd) _elm_fileselector_smart_del_do(sd);
}

EAPI Evas_Object *
elm_fileselector_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Elm_Fileselector_Smart_Data *sd = _pd;
   sd->obj = obj;
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));

   elm_widget_sub_object_add(eo_parent_get(obj), obj);
}

EAPI void
elm_fileselector_is_save_set(Evas_Object *obj,
                             Eina_Bool is_save)
{
   ELM_FILESELECTOR_CHECK(obj);
   eo_do(obj, elm_obj_fileselector_is_save_set(is_save));
}

static void
_is_save_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool is_save = va_arg(*list, int);
   Elm_Fileselector_Smart_Data *sd = _pd;

   elm_object_disabled_set(sd->path_entry, !is_save);

   if (is_save) elm_layout_signal_emit(obj, "elm,state,save,on", "elm");
   else elm_layout_signal_emit(obj, "elm,state,save,off", "elm");
}

EAPI Eina_Bool
elm_fileselector_is_save_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_fileselector_is_save_get(&ret));
   return ret;
}

static void
_is_save_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Fileselector_Smart_Data *sd = _pd;
   *ret = !elm_object_disabled_get(sd->path_entry);
}

EAPI void
elm_fileselector_folder_only_set(Evas_Object *obj,
                                 Eina_Bool only)
{
   ELM_FILESELECTOR_CHECK(obj);
   eo_do(obj, elm_obj_fileselector_folder_only_set(only));
}

static void
_folder_only_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool only = va_arg(*list, int);
   Elm_Fileselector_Smart_Data *sd = _pd;

   if (sd->only_folder == only) return;

   sd->only_folder = !!only;
   if (sd->path) _populate(obj, sd->path, NULL);
}

EAPI Eina_Bool
elm_fileselector_folder_only_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_fileselector_folder_only_get(&ret));
   return ret;
}

static void
_folder_only_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Fileselector_Smart_Data *sd = _pd;
   *ret = sd->only_folder;
}

EAPI void
elm_fileselector_buttons_ok_cancel_set(Evas_Object *obj,
                                       Eina_Bool visible)
{
   ELM_FILESELECTOR_CHECK(obj);
   eo_do(obj, elm_obj_fileselector_buttons_ok_cancel_set(visible));
}

static void
_buttons_ok_cancel_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool visible = va_arg(*list, int);

   Evas_Object *bt;

   Elm_Fileselector_Smart_Data *sd = _pd;

   if (visible)
     {
        // cancel btn
        bt = elm_button_add(obj);
        elm_widget_mirrored_automatic_set(bt, EINA_FALSE);
        elm_object_domain_translatable_text_set(bt, PACKAGE, N_("Cancel"));

        evas_object_smart_callback_add(bt, "clicked", _canc, obj);

        sd->cancel_button = bt;

        // ok btn
        bt = elm_button_add(obj);
        elm_widget_mirrored_automatic_set(bt, EINA_FALSE);
        elm_object_domain_translatable_text_set(bt, PACKAGE, N_("OK"));

        evas_object_smart_callback_add(bt, "clicked", _ok, obj);

        sd->ok_button = bt;

        eo_do(obj, elm_wdg_theme(NULL));
     }
   else
     {
        evas_object_del(sd->cancel_button);
        sd->cancel_button = NULL;
        evas_object_del(sd->ok_button);
        sd->ok_button = NULL;
     }
}

EAPI Eina_Bool
elm_fileselector_buttons_ok_cancel_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_fileselector_buttons_ok_cancel_get(&ret));
   return ret;
}

static void
_buttons_ok_cancel_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Fileselector_Smart_Data *sd = _pd;
   *ret = sd->ok_button ? EINA_TRUE : EINA_FALSE;
}

EAPI void
elm_fileselector_expandable_set(Evas_Object *obj,
                                Eina_Bool expand)
{
   ELM_FILESELECTOR_CHECK(obj);
   eo_do(obj, elm_obj_fileselector_expandable_set(expand));
}

static void
_expandable_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool expand = va_arg(*list, int);
   Elm_Fileselector_Smart_Data *sd = _pd;

   sd->expand = !!expand;

   if (sd->path) _populate(obj, sd->path, NULL);
}

EAPI Eina_Bool
elm_fileselector_expandable_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_fileselector_expandable_get(&ret));
   return ret;
}

static void
_expandable_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Fileselector_Smart_Data *sd = _pd;
   *ret = sd->expand;
}

EAPI void
elm_fileselector_path_set(Evas_Object *obj,
                          const char *_path)
{
   ELM_FILESELECTOR_CHECK(obj);
   eo_do(obj, elm_obj_fileselector_path_set(_path));
}

static void
_path_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *_path = va_arg(*list, const char *);
   char *path;

   path = ecore_file_realpath(_path);
   _populate(obj, path, NULL);
   free(path);
}

EAPI const char *
elm_fileselector_path_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_obj_fileselector_path_get(&ret));
   return ret;
}

static void
_path_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **ret = va_arg(*list, const char **);
   Elm_Fileselector_Smart_Data *sd = _pd;
   *ret = sd->path;
}

EAPI void
elm_fileselector_mode_set(Evas_Object *obj,
                          Elm_Fileselector_Mode mode)
{
   ELM_FILESELECTOR_CHECK(obj);
   eo_do(obj, elm_obj_fileselector_mode_set(mode));
}

static void
_mode_set(Eo *obj, void *_pd, va_list *list)
{
   Elm_Fileselector_Mode mode = va_arg(*list, Elm_Fileselector_Mode);

   Elm_Fileselector_Smart_Data *sd = _pd;

   if (mode == sd->mode) return;

   evas_object_hide(elm_layout_content_unset(obj, "elm.swallow.files"));

   if (mode == ELM_FILESELECTOR_LIST)
     elm_layout_content_set(obj, "elm.swallow.files", sd->files_list);
   else elm_layout_content_set(obj, "elm.swallow.files", sd->files_grid);

   sd->mode = mode;

   _populate(obj, sd->path, NULL);
}

EAPI Elm_Fileselector_Mode
elm_fileselector_mode_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_CHECK(obj) ELM_FILESELECTOR_LAST;
   Elm_Fileselector_Mode ret = ELM_FILESELECTOR_LAST;
   eo_do((Eo *) obj, elm_obj_fileselector_mode_get(&ret));
   return ret;
}

static void
_mode_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Fileselector_Mode *ret = va_arg(*list, Elm_Fileselector_Mode *);
   Elm_Fileselector_Smart_Data *sd = _pd;
   *ret = sd->mode;
}

EAPI const char *
elm_fileselector_selected_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_obj_fileselector_selected_get(&ret));
   return ret;
}

static void
_selected_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *fp;
   const char **ret = va_arg(*list, const char **);
   *ret = NULL;
   Elm_Fileselector_Smart_Data *sd = _pd;

   if (!sd->path) return;

   fp = elm_object_text_get(sd->path_entry);
   if (ecore_file_exists(fp))
     {
	eina_stringshare_replace(&sd->selection, fp);

        *ret = sd->selection;
        return;
     }

   *ret = sd->path;

   if (sd->mode == ELM_FILESELECTOR_LIST)
     {
        Elm_Object_Item *gl_it = elm_genlist_selected_item_get(sd->files_list);

        if (gl_it) *ret = elm_object_item_data_get(gl_it);
     }
   else
     {
        Elm_Object_Item *gg_it = elm_gengrid_selected_item_get(sd->files_grid);

        if (gg_it) *ret = elm_object_item_data_get(gg_it);
     }

}

EAPI Eina_Bool
elm_fileselector_selected_set(Evas_Object *obj,
                              const char *_path)
{
   ELM_FILESELECTOR_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_fileselector_selected_set(_path, &ret));
   return ret;
}

static void
_selected_set(Eo *obj, void *_pd, va_list *list)
{
   const char *_path = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   char *selected;

   if (ret) *ret = EINA_TRUE;
   char *path;

   Elm_Fileselector_Smart_Data *sd = _pd;

   path = ecore_file_realpath(_path);

   if (ecore_file_is_dir(path)) _populate(obj, path, NULL);
   else
     {
        if (!ecore_file_exists(path))
          {
             if (ret) *ret = EINA_FALSE;
             goto clean_up;
          }

        selected= ecore_file_dir_get(path);
        _populate(obj, selected, NULL);
        eina_stringshare_replace(&sd->selection, path);
        free(selected);
     }

clean_up:
   free(path);
}

static void
_elm_fileselector_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_elm_fileselector_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_fileselector_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_fileselector_smart_del),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_THEME), _elm_fileselector_smart_theme),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_fileselector_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_fileselector_smart_focus_direction_manager_is),

        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_fileselector_smart_sizing_eval),

        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_IS_SAVE_SET), _is_save_set),
        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_IS_SAVE_GET), _is_save_get),
        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_FOLDER_ONLY_SET), _folder_only_set),
        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_FOLDER_ONLY_GET), _folder_only_get),
        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_BUTTONS_OK_CANCEL_SET), _buttons_ok_cancel_set),
        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_BUTTONS_OK_CANCEL_GET), _buttons_ok_cancel_get),
        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_EXPANDABLE_SET), _expandable_set),
        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_EXPANDABLE_GET), _expandable_get),
        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_PATH_SET), _path_set),
        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_PATH_GET), _path_get),
        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_MODE_SET), _mode_set),
        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_MODE_GET), _mode_get),
        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_SELECTED_GET), _selected_get),
        EO_OP_FUNC(ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_SELECTED_SET), _selected_set),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME, klass);
}
static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_SUB_ID_IS_SAVE_SET, "Enable/disable the file name entry box where the user can type in a name for a file, in a given file selector widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_SUB_ID_IS_SAVE_GET, "Get whether the given file selector is in 'saving dialog' mode."),
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_SUB_ID_FOLDER_ONLY_SET, "Enable/disable folder-only view for a given file selector widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_SUB_ID_FOLDER_ONLY_GET, "Get whether folder-only view is set for a given file selector widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_SUB_ID_BUTTONS_OK_CANCEL_SET, "Enable/disable the 'ok' and 'cancel' buttons on a given file selector widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_SUB_ID_BUTTONS_OK_CANCEL_GET, "Get whether the 'ok' and 'cancel' buttons on a given file selector widget are being shown."),
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_SUB_ID_EXPANDABLE_SET, "Enable/disable a tree view in the given file selector widget, if it's in #ELM_FILESELECTOR_LIST mode."),
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_SUB_ID_EXPANDABLE_GET, "Get whether tree view is enabled for the given file selector widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_SUB_ID_PATH_SET, "Set, programmatically, the directory that a given file selector widget will display contents from."),
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_SUB_ID_PATH_GET, "Get the parent directory's path that a given file selector widget is displaying."),
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_SUB_ID_MODE_SET, "Set the mode in which a given file selector widget will display (layout) file system entries in its view."),
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_SUB_ID_MODE_GET, "Get the mode in which a given file selector widget is displaying (layouting) file system entries in its view."),
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_SUB_ID_SELECTED_GET, "Get the currently selected item's (full) path, in the given file selector widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_FILESELECTOR_SUB_ID_SELECTED_SET, "Set, programmatically, the currently selected file/directory in the given file selector widget."),
     EO_OP_DESCRIPTION_SENTINEL
};
static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_FILESELECTOR_BASE_ID, op_desc, ELM_OBJ_FILESELECTOR_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Fileselector_Smart_Data),
     _class_constructor,
     NULL
};
EO_DEFINE_CLASS(elm_obj_fileselector_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, NULL);
