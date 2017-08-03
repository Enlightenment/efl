/*
 * TODO:
 *  - child elements focusing support
 *  - user defined icon/label cb
 *  - show/hide/add buttons ???
 *  - Pattern Filter support
 */
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_INTERFACE_ATSPI_WIDGET_ACTION_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_fileselector_button.eo.h"
#include "elm_fileselector_entry.eo.h"
#include "elm_interface_fileselector.h"
#include "elm_widget_fileselector.h"

#include "elm_fileselector_internal_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS ELM_FILESELECTOR_CLASS

#define MY_CLASS_NAME "Elm_Fileselector"
#define MY_CLASS_NAME_LEGACY "elm_fileselector"

/* FIXME: need a way to find a gap between the size of item and thumbnail */
#define ITEM_SIZE_DEFAULT 70

typedef struct _Legacy_Event_Path_Then_Data
{
   Eo                          *eo_obj;
   const Efl_Event_Description *evt_desc;
} Legacy_Event_Path_Then_Data;

static Elm_Genlist_Item_Class *list_itc[ELM_FILE_LAST];
static Elm_Gengrid_Item_Class *grid_itc[ELM_FILE_LAST];

static const char _text_activated_model_key[] = "__fs_text_activated_model";
static const char _text_activated_path_key[] = "__fs_text_activated_path";
static const char _selected_model_set_promise_owner_key[] = "__fs_selected_model_set_promise_owner";
static const char _selected_model_set_model_key[] = "__fs_selected_model_set_model";

EAPI Eina_Error ELM_FILESELECTOR_ERROR_UNKNOWN = 0;
EAPI Eina_Error ELM_FILESELECTOR_ERROR_INVALID_MODEL = 0;

static const char ELM_FILESELECTOR_ERROR_UNKNOWN_STR[]           = "Unknown Error";
static const char ELM_FILESELECTOR_ERROR_INVALID_MODEL_STR[]     = "Model not set";

#define ELM_PRIV_FILESELECTOR_SIGNALS(cmd) \
   cmd(SIG_ACTIVATED, "activated", "s") \
   cmd(SIG_DIRECTORY_OPEN, "directory,open", "s") \
   cmd(SIG_DONE, "done", "s") \
   cmd(SIG_SELECTED, "selected", "s") \
   cmd(SIG_SELECTED_INVALID, "selected,invalid", "s")

ELM_PRIV_FILESELECTOR_SIGNALS(ELM_PRIV_STATIC_VARIABLE_DECLARE);

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   ELM_PRIV_FILESELECTOR_SIGNALS(ELM_PRIV_SMART_CALLBACKS_DESC)
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};
#undef ELM_PRIV_FILESELECTOR_SIGNALS

static Eina_Bool _key_action_select(Evas_Object *obj, const char *params);
static Eina_Bool _key_action_escape(Evas_Object *obj, const char *params);
static Eina_Bool _key_action_backspace(Evas_Object *obj, const char *params);

static const Elm_Action key_actions[] = {
   {"select", _key_action_select},
   {"escape", _key_action_escape},
   {"backspace", _key_action_backspace},
   {NULL, NULL}
};

static void _ok(void *data, const Efl_Event *event);
static void _canc(void *data, const Efl_Event *event);
static void _on_dir_up(void *data, const Efl_Event *event);
static void _populate(Evas_Object *obj, Efl_Model *model, Elm_Object_Item *parent_it, Efl_Model *selected);
static Elm_Fileselector_Item_Data *_selected_item_data_get(Elm_Fileselector_Data *sd);

static void _resource_created(void *, const Efl_Event *);
static void _resource_deleted(void *, const Efl_Event *);
static void _listing_request_cleanup(Listing_Request *);

EFL_CALLBACKS_ARRAY_DEFINE(monitoring_callbacks,
                          { EFL_MODEL_EVENT_CHILD_ADDED, _resource_created },
                          { EFL_MODEL_EVENT_CHILD_REMOVED, _resource_deleted });


static void
_focus_chain_update(Eo *obj, Elm_Fileselector_Data *pd)
{
   Eina_List *tmp = NULL;

#define A(p) tmp = eina_list_append(tmp, p);

   A(pd->up_button)
   A(pd->home_button)
   A(pd->search_entry)
   A(pd->files_view)
   A(pd->path_entry)
   A(pd->name_entry)
   A(pd->filter_hoversel)
   A(pd->ok_button)
   A(pd->cancel_button)


#undef A

   efl_ui_focus_manager_update_order(efl_ui_focus_user_manager_get(obj), obj, tmp);
}

static void
_model_free_eo_cb(void *data)
{
   Eo *eo = data;
   efl_unref(eo);
}


void
_event_to_legacy_call(Eo *obj, const Efl_Event_Description *evt_desc, void *event_info)
{
   const Efl_Event_Description *legacy_desc = efl_object_legacy_only_event_description_get(evt_desc->name);
   efl_event_callback_call(obj, legacy_desc, event_info);
}

void
_model_event_call(Eo *obj, const Efl_Event_Description *evt_desc, Efl_Model *model, const char *path)
{
   _event_to_legacy_call(obj, evt_desc, (void *)path);
   efl_event_callback_call(obj, evt_desc, model);
}

static void
_monitoring_start(Elm_Fileselector *fs, Elm_Fileselector_Data *sd, Efl_Model *model)
{
   sd->monitoring = EINA_TRUE;
   efl_event_callback_array_add(model, monitoring_callbacks(), fs);
}

static void
_monitoring_stop(Elm_Fileselector *fs, Elm_Fileselector_Data *sd, Efl_Model *model)
{
   sd->monitoring = EINA_FALSE;
   efl_event_callback_array_del(model, monitoring_callbacks(), fs);
}

static void
_elm_fileselector_replace_model(Elm_Fileselector *fs, Elm_Fileselector_Data *sd, Efl_Model *model, const char *path)
{
   if (sd->model)
     {
        _monitoring_stop(fs, sd, sd->model);
        efl_unref(sd->model);
     }

   if (model && path)
     {
        sd->model = efl_ref(model);
        eina_stringshare_replace(&sd->path, path);
        _monitoring_start(fs, sd, sd->model);
        /* TODO: sub directory should be monitored for expand mode */
     }
   else
     {
        sd->model = NULL;
        eina_stringshare_replace(&sd->path, NULL);
     }
}

/* final routine on deletion */
static void
_elm_fileselector_smart_del_do(Elm_Fileselector *fs, Elm_Fileselector_Data *sd)
{
   _elm_fileselector_replace_model(fs, sd, NULL, NULL);
   if (sd->prev_model)
     efl_unref(sd->prev_model);
   free(ecore_idler_del(sd->populate_idler));
   ecore_idler_del(sd->path_entry_idler);

   efl_canvas_group_del(efl_super(sd->obj, MY_CLASS));
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   ELM_FILESELECTOR_DATA_GET(obj, sd);

   efl_ui_mirrored_set(sd->cancel_button, rtl);
   efl_ui_mirrored_set(sd->ok_button, rtl);
   efl_ui_mirrored_set(sd->files_view, rtl);
   efl_ui_mirrored_set(sd->up_button, rtl);
   efl_ui_mirrored_set(sd->home_button, rtl);
}

static Eina_Bool
_accessor_value_get(Eina_Accessor *acc, int id, void *res)
{
   Eina_Value *v = NULL;

   if (!eina_accessor_data_get(acc, id, (void **) &v))
     return EINA_FALSE;

   eina_value_get(v, res);
   return EINA_TRUE;
}

static Efl_Future*
_model_str_property_set(Efl_Model *model, const char *property_name, const char *property_value)
{
   Efl_Future* r;
   Eina_Value v;
   eina_value_setup(&v, EINA_VALUE_TYPE_STRING);
   eina_value_set(&v, property_value);

   r = efl_model_property_set(model, property_name, &v);

   eina_value_flush(&v);

   return r;
}

EOLIAN static Elm_Theme_Apply
_elm_fileselector_elm_widget_theme_apply(Eo *obj, Elm_Fileselector_Data *sd)
{
   const char *style;
   const char *data;
   char buf[1024];
   Elm_Theme_Apply int_ret = ELM_THEME_APPLY_FAILED;

   int_ret = elm_obj_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return ELM_THEME_APPLY_FAILED;

   style = elm_widget_style_get(obj);
   _mirrored_set(obj, efl_ui_mirrored_get(obj));

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   data = edje_object_data_get
       (wd->resize_obj, "path_separator");
   if (data) sd->path_separator = data;
   else sd->path_separator = "/";

   snprintf(buf, sizeof(buf), "fileselector/%s", style);

   elm_widget_style_set(sd->up_button, buf);
   elm_widget_style_set(sd->home_button, buf);
   elm_widget_style_set(sd->spinner, buf);

   elm_widget_style_set(sd->files_view, buf);

   elm_widget_style_set(sd->path_entry, buf);
   elm_widget_style_set(sd->name_entry, buf);
   elm_widget_style_set(sd->search_entry, buf);

   snprintf(buf, sizeof(buf), "fileselector/actions/%s", style);

   if (sd->filter_hoversel)
     elm_widget_style_set(sd->filter_hoversel, buf);

   if (sd->ok_button)
     {
        elm_widget_style_set(sd->cancel_button, buf);
        elm_widget_style_set(sd->ok_button, buf);
     }

   return int_ret;
}

static Eina_Bool
_key_action_select(Evas_Object *obj, const char *params EINA_UNUSED)
{
   Efl_Event event = {};
   _ok(obj, &event);
   return EINA_TRUE;
}

static Eina_Bool
_key_action_escape(Evas_Object *obj, const char *params EINA_UNUSED)
{
   Efl_Event event = {};
   _canc(obj, &event);
   return EINA_TRUE;
}

static Eina_Bool
_key_action_backspace(Evas_Object *obj, const char *params EINA_UNUSED)
{
   ELM_FILESELECTOR_DATA_GET(obj, sd);
   if (sd->prev_model)
     {
        _populate(obj, sd->prev_model, NULL, NULL);
        efl_unref(sd->prev_model);
        sd->prev_model = NULL;
     }
   else
     _on_dir_up(obj, NULL);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_fileselector_elm_widget_widget_event(Eo *obj, Elm_Fileselector_Data *sd EINA_UNUSED, Evas_Object *src, Evas_Callback_Type type, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   (void) src;

   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

   if (!_elm_config_key_binding_call(obj, MY_CLASS_NAME, ev, key_actions))
     return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

/***  GENLIST/GENGRID "MODEL"  ***/
static char *
_itc_text_get(void *data,
              Evas_Object *obj   EINA_UNUSED,
              const char *source EINA_UNUSED)
{
   Elm_Fileselector_Item_Data *it_data = data;
   return elm_entry_utf8_to_markup
            (it_data->filename);        /* NOTE this will be free()'d by
                                          * the caller */
}

static Evas_Object *
_itc_icon_folder_get(void *data EINA_UNUSED,
                     Evas_Object *obj,
                     const char *source)
{
   Evas_Object *ic, *grid;

   if (strcmp(source, "elm.swallow.icon")) return NULL;

   ic = elm_icon_add(obj);
   elm_icon_standard_set(ic, "folder");
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   evas_object_show(ic);

   grid = elm_grid_add(obj);
   elm_grid_size_set(grid, 1, 1);
   elm_grid_pack(grid, ic, 0, 0, 1, 1);
   evas_object_show(grid);

   return grid;
}

static Evas_Object *
_itc_icon_image_get(void *data,
                    Evas_Object *obj,
                    const char *source)
{
   Elm_Fileselector_Item_Data *it_data = data;
   Evas_Object *ic, *grid;

   if (strcmp(source, "elm.swallow.icon")) return NULL;

   ic = elm_icon_add(obj);
   elm_icon_standard_set(ic, "image");
   // FIXME: maybe use Efl.Model.connect
   elm_icon_thumb_set(ic, it_data->path, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   evas_object_show(ic);

   grid = elm_grid_add(obj);
   elm_grid_size_set(grid, 1, 1);
   elm_grid_pack(grid, ic, 0, 0, 1, 1);
   evas_object_show(grid);

   return grid;
}

static Evas_Object *
_itc_icon_file_get(void *data EINA_UNUSED,
                   Evas_Object *obj,
                   const char *source)
{
   Evas_Object *ic, *grid;

   if (strcmp(source, "elm.swallow.icon")) return NULL;

   ic = elm_icon_add(obj);
   elm_icon_standard_set(ic, "file");
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   evas_object_show(ic);

   grid = elm_grid_add(obj);
   elm_grid_size_set(grid, 1, 1);
   elm_grid_pack(grid, ic, 0, 0, 1, 1);
   evas_object_show(grid);

   return grid;
}

static Eina_Bool
_itc_state_get(void *data         EINA_UNUSED,
               Evas_Object *obj   EINA_UNUSED,
               const char *source EINA_UNUSED)
{
   return EINA_FALSE;
}

static void
_itc_del(void *data,
         Evas_Object *obj EINA_UNUSED)
{
   Elm_Fileselector_Item_Data *it_data = data;
   efl_unref(it_data->model);
   eina_stringshare_del(it_data->path);
   eina_stringshare_del(it_data->filename);
   eina_stringshare_del(it_data->mime_type);
   efl_unref(it_data->parent_model);
   eina_stringshare_del(it_data->parent_path);
   free(it_data);
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

   eina_strlcat(buf, "<a href='/'>root</a>", sizeof(buf));
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

static Eina_Bool
_mime_type_matched(const char *mime_filter, const char *mime_type)
{
   int i = 0;

   while (mime_filter[i] != '\0')
     {
        if (mime_filter[i] != mime_type[i])
          {
             if (mime_filter[i] == '*' && mime_filter[i + 1] == '\0')
               return EINA_TRUE;

             return EINA_FALSE;
          }
        i++;
     }

   if (mime_type[i] != '\0') return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_is_in_filter(const Elm_Fileselector_Filter *filter, const char *mime_type)
{
   int i;
   if (!mime_type) return EINA_FALSE;

   for (i = 0; filter->filter.mime_types[i]; ++i)
     {
        if (_mime_type_matched(filter->filter.mime_types[i], mime_type))
          return EINA_TRUE;
     }
   return EINA_FALSE;
}

static Eina_Bool
_filter_child(Elm_Fileselector_Data* sd,
              const char *path,
              const char *filename,
              Eina_Bool dir,
              const char *mime_type)
{
   Elm_Fileselector_Filter *cf;
   char *pch = NULL, *temp = NULL;
   char temp_path[EINA_PATH_MAX];

   if (!sd) return EINA_FALSE;

   if (!sd->hidden_visible && filename[0] == '.')
     return EINA_FALSE;

   if (sd->only_folder && !dir)
     return EINA_FALSE;

   //Search entry filter
   if ((sd->search_string) && (sd->search_string[0] != '\0'))
     {
        strncpy(temp_path, path, sizeof(temp_path) - 1);
        temp_path[sizeof(temp_path) - 1] = 0;
        pch = strchr(temp_path, EINA_PATH_SEP_C);
        while (pch != NULL)
          {
             temp = pch;
             pch = strchr(pch + 1, EINA_PATH_SEP_C);
          }
        temp++;
        if ((temp) && (sd->search_string) &&
            (!strstr(temp, sd->search_string)))
          return EINA_FALSE;
     }

   cf = sd->current_filter;
   if (!cf)
     return EINA_TRUE;

   switch (cf->filter_type)
     {
      case ELM_FILESELECTOR_MIME_FILTER:
         return dir || _is_in_filter(cf, mime_type);
      case ELM_FILESELECTOR_CUSTOM_FILTER:
         return cf->filter.custom->func(path, dir,
                                        cf->filter.custom->data);
      default:
         return EINA_FALSE;
     }

   return EINA_FALSE;
}

static const char *
_file_type(const char *a)
{
   char *p = strrchr(a, '.');
   if (!p) return "";

   return p;
}

static int
_filename_cmp(const Elm_Fileselector_Item_Data *a, const Elm_Fileselector_Item_Data *b)
{
   return strcoll(a->filename, b->filename);
}

static int
_filename_cmp_rev(const Elm_Fileselector_Item_Data *a, const Elm_Fileselector_Item_Data *b)
{
   return _filename_cmp(b, a);
}

static int
_type_cmp(const Elm_Fileselector_Item_Data *a, const Elm_Fileselector_Item_Data *b)
{
   return strcoll(_file_type(a->filename), _file_type(b->filename));
}

static int
_type_cmp_rev(const Elm_Fileselector_Item_Data *a, const Elm_Fileselector_Item_Data *b)
{
   return _type_cmp(b, a);
}

static int
_size_cmp(const Elm_Fileselector_Item_Data *a, const Elm_Fileselector_Item_Data *b)
{
   return a->size - b->size;
}

static int
_size_cmp_rev(const Elm_Fileselector_Item_Data *a, const Elm_Fileselector_Item_Data *b)
{
   return _size_cmp(b, a);
}

static int
_modified_cmp(const Elm_Fileselector_Item_Data *a, const Elm_Fileselector_Item_Data *b)
{
   if (a->mtime > b->mtime)
     return 1;

   return -1;
}

static int
_modified_cmp_rev(const Elm_Fileselector_Item_Data *a, const Elm_Fileselector_Item_Data *b)
{
   return _modified_cmp(b, a);
}

static int
_file_grid_cmp(const void *a, const void *b)
{
   Evas_Object *f;

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

   f = evas_object_data_get(elm_object_item_widget_get(ga), "parent");
   ELM_FILESELECTOR_DATA_GET(f, sd);
   return sd->sort_method(elm_object_item_data_get(ga),
                          elm_object_item_data_get(gb));
}

static int
_file_list_cmp(const void *a, const void *b)
{
   Evas_Object *f;

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

   f = evas_object_data_get(elm_object_item_widget_get(la), "parent");
   ELM_FILESELECTOR_DATA_GET(f, sd);
   return sd->sort_method(elm_object_item_data_get(la),
                          elm_object_item_data_get(lb));
}

static void
_signal_first(Listing_Request *lreq)
{
   Elm_Fileselector_Data *sd = lreq->sd;
   if (!lreq->first) return;
   if (!sd) return;

   if (sd->multi)
     {
        sd->multi_selection = eina_list_free(sd->multi_selection);
     }

   _model_event_call
     (lreq->obj, ELM_FILESELECTOR_EVENT_DIRECTORY_OPEN, lreq->model, lreq->path);

   if (!lreq->parent_it)
     {
        if (sd->mode == ELM_FILESELECTOR_LIST)
          elm_genlist_clear(sd->files_view);
        else
          elm_gengrid_clear(sd->files_view);
        _elm_fileselector_replace_model(lreq->obj, sd, lreq->model, lreq->path);
        _anchors_do(lreq->obj, lreq->path);
     }

   lreq->first = EINA_FALSE;
}

static void
_process_last(Listing_Request *lreq)
{
   Elm_Fileselector_Data *sd = lreq->sd;
   elm_progressbar_pulse(sd->spinner, EINA_FALSE);
   elm_layout_signal_emit(lreq->obj, "elm,action,spinner,hide", "elm");

   _listing_request_cleanup(lreq);

   sd->current_populate_lreq = NULL;
}

static Eina_Bool
_process_child(Elm_Fileselector_Item_Data *it_data, Eina_Accessor *values_acc)
{
   Elm_Object_Item *item;
   Listing_Request *lreq = it_data->user_data;
   int itcn = ELM_FILE_UNKNOW;
   const char *path = NULL;
   const char *filename = NULL;
   const char *mime_type = NULL;
   int64_t size = 0;
   double mtime = 0;
   Eina_Bool dir = EINA_FALSE;
   Elm_Fileselector_Data *sd = lreq->sd;
   it_data->user_data = NULL;

   if (!sd->files_view)
     return EINA_FALSE;

   if (!_accessor_value_get(values_acc, 0, &path) || !path ||
       !_accessor_value_get(values_acc, 1, &filename) || !filename ||
       !_accessor_value_get(values_acc, 2, &dir) ||
       !_accessor_value_get(values_acc, 3, &size) ||
       !_accessor_value_get(values_acc, 4, &mtime) ||
       !_accessor_value_get(values_acc, 5, &mime_type))
     {
        ERR("missing child Efl.Model data");
        return EINA_FALSE;
     }

   if (!_filter_child(sd, path, filename, dir, mime_type))
     return EINA_FALSE;

   _signal_first(lreq);

   it_data->path = eina_stringshare_add(path);
   it_data->filename = eina_stringshare_add(filename);
   it_data->size = size;
   it_data->mtime = mtime;
   it_data->mime_type = eina_stringshare_add(mime_type);
   it_data->parent_model = efl_ref(lreq->model);
   it_data->parent_path = eina_stringshare_add(lreq->path);
   it_data->is_dir = dir;

   if (it_data->is_dir)
     itcn = ELM_DIRECTORY;
   else
     {
        if (evas_object_image_extension_can_load_get(it_data->filename))
          itcn = ELM_FILE_IMAGE;
     }

   if (sd->mode == ELM_FILESELECTOR_LIST)
     {
        item = elm_genlist_item_sorted_insert(sd->files_view, list_itc[itcn],
                                              it_data,
                                              lreq->parent_it,
                                              ((sd->expand) && (itcn == ELM_DIRECTORY))
                                              ? ELM_GENLIST_ITEM_TREE : ELM_GENLIST_ITEM_NONE,
                                              _file_list_cmp, NULL, NULL);

        if (lreq->selected_path && it_data->path == lreq->selected_path)
          {
             elm_genlist_item_selected_set(item, EINA_TRUE);
             elm_object_text_set(sd->name_entry, it_data->filename);
          }
     }
   else if (sd->mode == ELM_FILESELECTOR_GRID)
     {
        item = elm_gengrid_item_sorted_insert(sd->files_view, grid_itc[itcn],
                                              it_data,
                                              _file_grid_cmp, NULL, NULL);

        if (lreq->selected_path && it_data->path == lreq->selected_path)
          {
             elm_gengrid_item_selected_set(item, EINA_TRUE);
             elm_object_text_set(sd->name_entry, it_data->filename);
          }
     }
   return EINA_TRUE;
}

static void
_process_child_cb(void *data, Efl_Event const*event)
{
   Elm_Fileselector_Item_Data *it_data = data;
   Listing_Request *lreq = it_data->user_data;
   Efl_Future_Event_Success *ev = event->info;
   Eina_Accessor *values_acc = ev->value;

   if (!lreq->valid || !_process_child(it_data, values_acc))
     {
        efl_unref(it_data->model);
        free(it_data);
     }

   ++(lreq->item_processed_count);
   if (lreq->item_processed_count >= lreq->item_total)
     {
        if (!lreq->valid)
          {
             _listing_request_cleanup(lreq);
             return;
          }
        _signal_first(lreq);
        _process_last(lreq);
     }
}

static void
_process_child_error_cb(void *data, Efl_Event const* event EINA_UNUSED)
{
   Elm_Fileselector_Item_Data *it_data = data;
   Listing_Request *lreq = it_data->user_data;

   efl_unref(it_data->model);
   free(it_data);

   ERR("Failed to access to a model property");

   ++(lreq->item_processed_count);
   if (lreq->item_processed_count >= lreq->item_total)
     {
        if (!lreq->valid)
          {
             _listing_request_cleanup(lreq);
             return;
          }
        _signal_first(lreq);
        _process_last(lreq);
     }
}

static void
_listing_request_cleanup(Listing_Request *lreq)
{
   if (lreq->parent_it)
     efl_unref(lreq->parent_it);
   efl_unref(lreq->obj);
   efl_unref(lreq->model);
   if (lreq->selected)
     efl_unref(lreq->selected);
   eina_stringshare_del(lreq->path);
   eina_stringshare_del(lreq->selected_path);
   free(lreq);
}

static void
_process_children_cb(void *data, Efl_Event const *event)
{
   Listing_Request *lreq = data;
   Efl_Future_Event_Success *ev = event->info;
   Eina_Accessor *all_promises = ev->value;
   Eina_Accessor *children_accessor = NULL;
   Elm_Fileselector_Item_Data *it_data = NULL;
   const char *path = NULL;
   const char *selected_path = NULL;
   unsigned int count = 0;
   Elm_Fileselector_Data *sd = lreq->sd;

   if (!lreq->valid)
     {
        _listing_request_cleanup(lreq);
        return;
     }

   if (_accessor_value_get(all_promises, 0, &path) && path &&
       eina_accessor_data_get(all_promises, 1, (void **) &children_accessor))
   {
        if (lreq->selected)
          {
             if (!_accessor_value_get(all_promises, 2, &selected_path) ||
                 !selected_path)
               {
                  ERR("missing selected Efl.Model path information");
                  _listing_request_cleanup(lreq);
                  sd->current_populate_lreq = NULL;
                  return;
               }
             lreq->selected_path = eina_stringshare_add(selected_path);
          }
        lreq->path = eina_stringshare_add(path);
        if (children_accessor)
          {
             Eina_List *children = NULL;
             void *child = NULL;
             EINA_ACCESSOR_FOREACH(children_accessor, count, child)
               {
                  children = eina_list_append(children, child);
               }

             lreq->item_total = count;

             EINA_LIST_FREE(children, child)
               {
                  Efl_Future *futures[7];
                  Efl_Future *future_all = NULL;
                  const char *prop[6] = {
                     "path", "filename", "is_dir", "size", "mtime", "mime_type"
                  };
                  unsigned int i;

                  it_data = calloc(1, sizeof(Elm_Fileselector_Item_Data));
                  if (!it_data)
                    {
                       ERR("insufficient memory");
                       break;
                    }

                  it_data->model = efl_ref(child);
                  it_data->user_data = lreq;

                  for (i = 0; i != 6; i++)
                    {
                       futures[i] = efl_model_property_get(child, prop[i]);
                    }

                  future_all = efl_future_all(futures[0], futures[1], futures[2], futures[3], futures[4], futures[5]);
                  efl_future_then(future_all, _process_child_cb, _process_child_error_cb, NULL, it_data);
               }

             // NOTE: lreq may have been deallocated in the previous loop
             lreq = NULL;
          }
        if (count == 0)
          {
             _signal_first(lreq);
             _process_last(lreq);
          }
     }
   else
     {
        ERR("missing Efl.Model information");
        _listing_request_cleanup(lreq);
        sd->current_populate_lreq = NULL;
     }
}

static void
_process_children_error_cb(void *data, Efl_Event const* event)
{
   Eina_Error error = ((Efl_Future_Event_Failure*)event->info)->error;
   Listing_Request *lreq = data;
   Elm_Fileselector_Data *sd = lreq->sd;

   if (error != EINA_ERROR_FUTURE_CANCEL)
     {
        ERR("failed to get information from Efl.Model");
     }

   if (lreq->valid)
     {
        elm_progressbar_pulse(sd->spinner, EINA_FALSE);
        elm_layout_signal_emit(lreq->obj, "elm,action,spinner,hide", "elm");
        sd->current_populate_lreq = NULL;
     }
   _listing_request_cleanup(lreq);
}

static void
_populate(Evas_Object *obj,
          Efl_Model *model,
          Elm_Object_Item *parent_it,
          Efl_Model *selected)
{
   ELM_FILESELECTOR_DATA_GET(obj, sd);
   if (!model) return;

   Listing_Request *lreq;

   if (sd->expand && sd->current_populate_lreq)
     return;

   if (sd->current_populate_lreq)
     {
        sd->current_populate_lreq->valid = EINA_FALSE;
        sd->current_populate_lreq = NULL;
     }

   if (sd->model)
     _monitoring_stop(obj, sd, sd->model);

   lreq = calloc(1, sizeof (Listing_Request));
   if (!lreq) return;

   lreq->sd = sd;
   lreq->parent_it = (parent_it ? efl_ref(parent_it) : NULL);
   lreq->obj = efl_ref(obj);
   lreq->model = efl_ref(model);
   lreq->selected = (selected ? efl_ref(selected) : NULL);
   lreq->path = NULL;
   lreq->selected_path = NULL;
   lreq->item_total = 0;
   lreq->item_processed_count = 0;
   lreq->first = EINA_TRUE;
   lreq->valid = EINA_TRUE;

   sd->current_populate_lreq = lreq;

   elm_progressbar_pulse(sd->spinner, EINA_TRUE);
   elm_layout_signal_emit(lreq->obj, "elm,action,spinner,show", "elm");

   // Clear name entry not in case of save mode.
   if (elm_object_disabled_get(sd->name_entry))
     elm_object_text_set(sd->name_entry, "");

   Efl_Future *futures[4] = {NULL,};
   Efl_Future *future_all = NULL;
   futures[0] = efl_model_property_get(model, "path");
   futures[1] = efl_model_children_slice_get(model, 0, 0);
   if (selected)
     futures[2] = efl_model_property_get(selected, "path");

   future_all = efl_future_all(futures[0], futures[1], futures[2]);
   efl_future_then(future_all, _process_children_cb, _process_children_error_cb, NULL, lreq);
}

static void
_on_list_expanded(void *data, const Efl_Event *event)
{
   Elm_Object_Item *it = event->info;
   const Elm_Fileselector_Item_Data *it_data = elm_object_item_data_get(it);

   _populate(data, it_data->model, it, NULL);
}

static void
_on_list_contracted(void *data EINA_UNUSED, const Efl_Event *event)
{
   Elm_Object_Item *it = event->info;

   elm_genlist_item_subitems_clear(it);
}

static void
_on_list_expand_req(void *data EINA_UNUSED, const Efl_Event *event)
{
   Elm_Object_Item *it = event->info;

   elm_genlist_item_expanded_set(it, EINA_TRUE);
}

static void
_on_list_contract_req(void *data EINA_UNUSED, const Efl_Event *event)
{
   Elm_Object_Item *it = event->info;

   elm_genlist_item_expanded_set(it, EINA_FALSE);
}

static Eina_Bool
_populate_do(void *data)
{
   struct sel_data *sdata = data;
   ELM_FILESELECTOR_DATA_GET(sdata->fs, sd);
   _populate(sdata->fs, sdata->model, NULL, sdata->selected);
   if (sdata->model)
     efl_unref(sdata->model);
   if (sdata->selected)
     efl_unref(sdata->selected);

   sd->populate_idler = NULL;

   free(sdata);
   return ECORE_CALLBACK_CANCEL;
}

static void
_schedule_populate(Evas_Object *fs,
                   Elm_Fileselector_Data *sd,
                   Efl_Model *model,
                   Efl_Model *selected)
{
   struct sel_data *sdata;
   sdata = calloc(1, sizeof(*sdata));
   if (!sdata) return;

   sdata->fs = fs;
   sdata->model = model;
   sdata->selected = selected;

   if (model) efl_ref(model);
   if (selected) efl_ref(selected);

   if (sd->populate_idler)
     {
        struct sel_data *old_sdata;
        old_sdata = ecore_idler_del(sd->populate_idler);
        if (old_sdata->model)
          efl_unref(old_sdata->model);
        if (old_sdata->selected)
          efl_unref(old_sdata->selected);
        free(old_sdata);
     }
   sd->populate_idler = ecore_idler_add(_populate_do, sdata);
}

static void
_on_item_activated(void *data, const Efl_Event *event)
{
   //This event_info could be a list or gengrid item
   Elm_Object_Item *it = event->info;
   const Elm_Fileselector_Item_Data *it_data;

   ELM_FILESELECTOR_DATA_GET(data, sd);

   it_data = elm_object_item_data_get(it);
   if (!it_data) return;

   if (!it_data->is_dir)
     {
        _model_event_call
          (data, ELM_FILESELECTOR_EVENT_ACTIVATED, it_data->model, it_data->path);
        return;
     }

   if (!sd->double_tap_navigation) return;

   _schedule_populate(data, sd, it_data->model, NULL);
}

static void
_clear_selections(Elm_Fileselector_Data *sd, Elm_Object_Item *last_selected)
{
   Eina_List *items;
   Elm_Object_Item *sel;

   if (sd->mode == ELM_FILESELECTOR_LIST)
     {
        items = eina_list_clone(elm_genlist_selected_items_get(sd->files_view));

        EINA_LIST_FREE(items, sel)
          {
            if (sel == last_selected) continue;
            elm_genlist_item_selected_set(sel, EINA_FALSE);
          }
     }
   else if (sd->mode == ELM_FILESELECTOR_GRID)
     {
        items = eina_list_clone(elm_gengrid_selected_items_get(sd->files_view));

        EINA_LIST_FREE(items, sel)
          {
            if (sel == last_selected) continue;
            elm_gengrid_item_selected_set(sel, EINA_FALSE);
          }
     }
}

static void
_on_item_selected(void *data, const Efl_Event *event)
{
   //This event_info could be a list or gengrid item
   Elm_Object_Item *it = event->info;
   Elm_Fileselector_Item_Data *it_data = NULL;

   ELM_FILESELECTOR_DATA_GET(data, sd);

   it_data = elm_object_item_data_get(it);
   if (!it_data) return;

   /* We need to send callback when:
    * - path is dir and mode is ONLY FOLDER
    * - path is file and mode is NOT ONLY FOLDER */
   if (it_data->is_dir == sd->only_folder)
     {
        if (sd->multi)
          {
             Eina_List *li;
             Elm_Object_Item *it2;
             Eina_Strbuf *buf;

             if (sd->dir_selected)
               {
                  _clear_selections(sd, it);
                  sd->dir_selected = EINA_FALSE;
               }

             buf = eina_strbuf_new();
             EINA_LIST_FOREACH(sd->multi_selection, li, it2)
               {
                  Elm_Fileselector_Item_Data *it2_data = elm_object_item_data_get(it2);
                  eina_strbuf_append(buf, it2_data->filename);
                  eina_strbuf_append_length(buf, ", ", 2);
               }

             sd->multi_selection = eina_list_append(sd->multi_selection, it);
             eina_strbuf_append(buf, it_data->filename);

             elm_object_text_set(sd->name_entry, eina_strbuf_string_get(buf));
             eina_strbuf_free(buf);
          }
        else
          elm_object_text_set(sd->name_entry, it_data->filename);

        _model_event_call
          (data, EFL_UI_EVENT_SELECTED, it_data->model, it_data->path);
     }
   else if (sd->multi && it_data->is_dir && sd->double_tap_navigation)
     {
        _clear_selections(sd, it);
        sd->dir_selected = EINA_TRUE;
     }

   /* We need to populate, if path is directory and:
    * - mode is GRID;
    * - mode is LIST and 'not expand mode';
    *   in other cases update anchors. */

   if (sd->expand && sd->mode == ELM_FILESELECTOR_LIST)
     {
        if (!it_data->is_dir)
          {
             _elm_fileselector_replace_model(data, sd, it_data->parent_model, it_data->parent_path);
             _anchors_do(data, it_data->parent_path);
             return;
          }
        if (sd->only_folder)
          {
             _elm_fileselector_replace_model(data, sd, it_data->parent_model, it_data->parent_path);
             _anchors_do(data, it_data->parent_path);
          }
        else
          {
             _elm_fileselector_replace_model(data, sd, it_data->model, it_data->path);
             _anchors_do(data, it_data->path);
          }
        // Clear name entry not in case of save mode.
        if (elm_object_disabled_get(sd->name_entry))
          elm_object_text_set(sd->name_entry, "");
        return;
     }

   if (!it_data->is_dir) return;

   if (sd->double_tap_navigation) return;

   _schedule_populate(data, sd, it_data->model, NULL);
}

static void
_on_item_unselected(void *data, const Efl_Event *event)
{
   Eina_List *li, *l;
   const Elm_Fileselector_Item_Data *it_data;
   Eina_Strbuf *buf;
   Elm_Object_Item *it = event->info;
   Elm_Object_Item *it2 = NULL;
   Eina_Bool first = EINA_TRUE;

   ELM_FILESELECTOR_DATA_GET(data, sd);

   if (!sd->multi) return;

   it_data = elm_object_item_data_get(it);
   if (!it_data) return;

   buf = eina_strbuf_new();
   EINA_LIST_FOREACH_SAFE(sd->multi_selection, li, l, it2)
     {
        if (it2 == it)
          {
             sd->multi_selection = eina_list_remove_list(sd->multi_selection, li);
          }
        else
          {
             Elm_Fileselector_Item_Data *it2_data = elm_object_item_data_get(it2);
             if (!it2_data)
               continue;
             if (!first)
               eina_strbuf_append_length(buf, ", ", 2);
             else
               first = EINA_FALSE;

             eina_strbuf_append(buf, it2_data->path);
          }
     }

   elm_object_text_set(sd->name_entry, eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
}

static void
_on_dir_up(void *data, const Efl_Event *event EINA_UNUSED)
{
   Evas_Object *fs = data;
   Efl_Model *parent = NULL;

   ELM_FILESELECTOR_DATA_GET(fs, sd);

   parent = efl_parent_get(sd->model);
   if (!parent)
     return;
   _populate(fs, parent, NULL, NULL);
}

static void
_home(void *data, const Efl_Event *event EINA_UNUSED)
{
   Evas_Object *fs = data;
   ELM_FILESELECTOR_DATA_GET(fs, sd);

   // FIXME: maybe use vpath
   if (!sd->model || efl_isa(sd->model, EIO_MODEL_CLASS))
     {
        Eio_Model *model = efl_add(EIO_MODEL_CLASS, NULL,
                                  eio_model_path_set(efl_added, eina_environment_home_get()));
        _populate(fs, model, NULL, NULL);
        efl_unref(model);
     }
}

static void
_current_filter_changed(void *data,
                        Evas_Object *obj,
                        void *event_info EINA_UNUSED)
{
   Elm_Fileselector_Filter *filter = data;

   if (filter->sd->current_filter == filter) return;

   elm_object_text_set(obj, filter->filter_name);
   filter->sd->current_filter = filter;

   _populate(filter->sd->obj, filter->sd->model, NULL, NULL);
}

static void
_ok(void *data, const Efl_Event *event EINA_UNUSED)
{
   const char *name;
   const char *selection = NULL;
   Evas_Object *fs = data;
   ELM_FILESELECTOR_DATA_GET(fs, sd);

   if (!sd->model || !sd->path)
     {
        _model_event_call(fs, ELM_FILESELECTOR_EVENT_DONE, NULL, NULL);
        return;
     }

   name = elm_object_text_get(sd->name_entry);
   if (name && name[0] != '\0')
     {
        Efl_Model *selected_model = NULL;
        int len = eina_stringshare_strlen(sd->path);
        if (sd->path[len - 1] == '/')
          selection = eina_stringshare_printf("%s%s", sd->path, name);
        else
          selection = eina_stringshare_printf("%s/%s", sd->path, name);

        selected_model = efl_add(efl_class_get(sd->model), NULL);
        _model_str_property_set(selected_model, "path", selection);

        _model_event_call
          (fs, ELM_FILESELECTOR_EVENT_DONE, selected_model, selection);

        efl_unref(selected_model);
        eina_stringshare_del(selection);
     }
   else
     {
        Elm_Fileselector_Item_Data *it_data = _selected_item_data_get(sd);
        if (it_data)
          {
             _model_event_call
               (fs, ELM_FILESELECTOR_EVENT_DONE, it_data->model, it_data->path);
          }
        else
          {
             _model_event_call
               (fs, ELM_FILESELECTOR_EVENT_DONE, sd->model, sd->path);
          }
     }
}

static void
_canc(void *data, const Efl_Event *event EINA_UNUSED)
{
   Evas_Object *fs = data;

   _model_event_call(fs, ELM_FILESELECTOR_EVENT_DONE, NULL, NULL);
}

static void
_text_activated_free_fs_data(Elm_Fileselector *fs)
{
   Eina_Stringshare *str = efl_key_data_get(fs, _text_activated_path_key);
   eina_stringshare_del(str);
   efl_key_data_set(fs, _text_activated_path_key, NULL);
   efl_key_ref_set(fs, _text_activated_model_key, NULL);
   efl_unref(fs);
}

static void
_text_activated_is_dir_then(void *data, Efl_Event const *event)
{
   Evas_Object *fs = data;
   Eina_Bool is_dir = EINA_FALSE;
   ELM_FILESELECTOR_DATA_GET(fs, sd);

   Efl_Model *model = efl_key_ref_get(fs, _text_activated_model_key);
   Eina_Stringshare *str = efl_key_data_get(fs, _text_activated_path_key);

   eina_value_get((Eina_Value*)((Efl_Future_Event_Success*)event->info)->value, &is_dir);
   if (is_dir)
     {
        // keep previous path for backspace key action
        if (sd->prev_model)
          efl_unref(sd->prev_model);
        sd->prev_model = efl_ref(sd->model);

        _populate(fs, model, NULL, NULL);

        if (sd->only_folder)
          {
             _model_event_call
               (fs, EFL_UI_EVENT_SELECTED, model, str);
          }
     }
   else
     {
        Efl_Model *parent = efl_parent_get(model);
        if (!parent)
          {
             ERR("Efl.Model allocation error");
          }
        else
          {
             _populate(fs, parent, NULL, model);

             if (sd->only_folder)
               {
                  _model_event_call
                    (fs, EFL_UI_EVENT_SELECTED, model, str);
               }
          }
     }

   _text_activated_free_fs_data(fs);
}

static void
_text_activated_is_dir_then_error(void *data, Efl_Event const* event EINA_UNUSED)
{
   ERR("could not get information from Efl.Model");
   _text_activated_free_fs_data(data);
}

static void
_on_text_activated_set_path_then(void *data, Efl_Event const * event EINA_UNUSED)
{
   Evas_Object *fs = data;
   Efl_Future *future = NULL;
   ELM_FILESELECTOR_DATA_GET(fs, sd);

   if (!sd->model) return ;

   future = efl_model_property_get(sd->model, "is_dir");
   efl_future_then
     (future, _text_activated_is_dir_then, _text_activated_is_dir_then_error, NULL, data);
}

static void
_on_text_activated_set_path_then_error(void *data, Efl_Event const* event EINA_UNUSED)
{
   Evas_Object *fs = data;
   Efl_Model *model = efl_key_data_get(fs, _text_activated_model_key);
   Eina_Stringshare *str = efl_key_data_get(fs, _text_activated_path_key);

   _model_event_call
     (fs, EFL_UI_EVENT_SELECTED, model, str);

   _model_event_call
     (fs, ELM_FILESELECTOR_EVENT_SELECTED_INVALID, model, str);

   _text_activated_free_fs_data(fs);
}

static void
_on_text_activated(void *data, const Efl_Event *event)
{
   Evas_Object *fs = data;
   const char *path;
   Efl_Model *model;
   Efl_Future *future = NULL;

   ELM_FILESELECTOR_DATA_GET(fs, sd);

   if (!sd->model)
     return;

   path = elm_widget_part_text_get(event->object, NULL);
   model = efl_add(efl_class_get(sd->model), NULL);
   if (!model)
     return;

   future = _model_str_property_set(model, "path", path);

   efl_key_data_set(fs, _text_activated_path_key, eina_stringshare_add(path));
   efl_key_ref_set(fs, _text_activated_model_key, model);
   efl_ref(fs);
   efl_future_then(future,
                   _on_text_activated_set_path_then,
                   _on_text_activated_set_path_then_error,
                   NULL,
                   fs);

   efl_unref(model);
   elm_object_focus_set(event->object, EINA_FALSE);
}

static Eina_Bool
_anchors_undo(void *data)
{
   ELM_FILESELECTOR_DATA_GET(data, sd);

   elm_entry_entry_set(sd->path_entry, sd->path);
   elm_entry_cursor_pos_set(sd->path_entry, eina_stringshare_strlen(sd->path));

   sd->path_entry_idler = NULL;

   return ECORE_CALLBACK_CANCEL;
}

static void
_on_text_focused(void *data, const Efl_Event *event EINA_UNUSED)
{
   ELM_FILESELECTOR_DATA_GET(data, sd);

   if (!sd->path_entry_idler)
       sd->path_entry_idler = ecore_idler_add(_anchors_undo, data);
}

static void
_on_text_unfocused(void *data, const Efl_Event *event EINA_UNUSED)
{
   ELM_FILESELECTOR_DATA_GET(data, sd);
   _anchors_do(data, sd->path);
}

static void
_anchor_clicked(void *data, const Efl_Event *event)
{
   Elm_Entry_Anchor_Info *info = event->info;
   Evas_Object *fs = data;
   Efl_Model *model = NULL;

   ELM_FILESELECTOR_DATA_GET(fs, sd);

   if (!sd->model)
     return;

   model = efl_add(efl_class_get(sd->model), NULL);
   if (!model)
     return;
   _model_str_property_set(model, "path", info->name);

   _populate(fs, model, NULL, NULL);
   efl_unref(model);
   /* After anchor was clicked, entry will be focused, and will be editable.
    * It's wrong. So remove focus. */
   elm_object_focus_set(event->object, EINA_FALSE);

   if (sd->path_entry_idler) {
       ecore_idler_del(sd->path_entry_idler);
       sd->path_entry_idler = NULL;
   }
}

static void
_files_key_down(void *data, const Efl_Event *event)
{
     Efl_Input_Key *ev = event->info;
     Evas_Object *par, *searchbar;
     const char *string, *key;

     par = data;
     searchbar = evas_object_data_get(par, "search");

     if (!searchbar) return;

     key = efl_input_key_get(ev);
     string = efl_input_key_string_get(ev);
     if (string && *(string) && (isalpha(*string) || isdigit(*string)))
       {

          elm_entry_entry_append(searchbar, string);
          efl_input_processed_set(ev, EINA_TRUE);
       }
     else if (key && *(key) && !strcmp(key, "BackSpace"))
       {
          char buf[PATH_MAX];
          const char *en;
          en = elm_entry_entry_get(searchbar);
          if (en && strlen(en) > 0)
            {
               memmove(buf, en, strlen(en) -1);
               buf[strlen(en) -1] = '\0';
               elm_entry_entry_set(searchbar, buf);
               efl_input_processed_set(ev, EINA_TRUE);
            }
       }
}

static Evas_Object *
_files_list_add(Evas_Object *obj)
{
   Evas_Object *li;

   li = elm_genlist_add(obj);
   evas_object_data_set(li, "parent", obj);
   efl_ui_mirrored_automatic_set(li, EINA_FALSE);

   efl_event_callback_add
     (li, EFL_UI_EVENT_SELECTED, _on_item_selected, obj);
   efl_event_callback_add
     (li, EFL_UI_EVENT_UNSELECTED, _on_item_unselected, obj);
   efl_event_callback_add
     (li, ELM_GENLIST_EVENT_ACTIVATED, _on_item_activated, obj);
   efl_event_callback_add
     (li, ELM_GENLIST_EVENT_EXPAND_REQUEST, _on_list_expand_req, obj);
   efl_event_callback_add
     (li, ELM_GENLIST_EVENT_CONTRACT_REQUEST, _on_list_contract_req, obj);
   efl_event_callback_add
     (li, ELM_GENLIST_EVENT_EXPANDED, _on_list_expanded, obj);
   efl_event_callback_add
     (li, ELM_GENLIST_EVENT_CONTRACTED, _on_list_contracted, obj);
   efl_event_callback_add
     (li, EFL_EVENT_KEY_DOWN, _files_key_down, obj);

   return li;
}

static Evas_Object *
_files_grid_add(Evas_Object *obj)
{
   Evas_Object *grid;

   ELM_FILESELECTOR_DATA_GET(obj, sd);

   grid = elm_gengrid_add(obj);
   evas_object_data_set(grid, "parent", obj);
   efl_ui_mirrored_automatic_set(grid, EINA_FALSE);
   evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   elm_gengrid_item_size_set(grid, sd->thumbnail_size.w, sd->thumbnail_size.h);

   elm_gengrid_align_set(grid, 0.0, 0.0);

   efl_event_callback_add
     (grid, EFL_UI_EVENT_SELECTED, _on_item_selected, obj);
   efl_event_callback_add
     (grid, EFL_UI_EVENT_UNSELECTED, _on_item_unselected, obj);
   efl_event_callback_add
     (grid, ELM_GENGRID_EVENT_ACTIVATED, _on_item_activated, obj);
   efl_event_callback_add
     (grid, EFL_EVENT_KEY_DOWN, _files_key_down, obj);

   return grid;
}

static void
_resource_then_error(void *data, Efl_Event const* event EINA_UNUSED)
{
   Elm_Fileselector_Item_Data *it_data = data;
   WRN("could not get information from Efl.Model");
   efl_unref(it_data->user_data);
   efl_unref(it_data->model);
   free(it_data);
}

static void
_resource_created_then(void *data, Efl_Event const*event)
{
   Elm_Fileselector_Item_Data *it_data = data;
   Evas_Object *obj = it_data->user_data;
   Efl_Future_Event_Success *ev = event->info;
   Eina_Accessor *values_acc = ev->value;
   int itcn = ELM_FILE_UNKNOW;
   const char *path = NULL;
   const char *filename = NULL;
   const char *mime_type = NULL;
   int64_t size = 0;
   double mtime = 0;
   Eina_Bool dir = EINA_FALSE;
   it_data->user_data = NULL;

   ELM_FILESELECTOR_DATA_GET(obj, sd);

   if (!sd || !sd->monitoring || sd->model != it_data->parent_model)
     goto cancel;

   if (!_accessor_value_get(values_acc, 0, &path) || !path ||
       !_accessor_value_get(values_acc, 1, &filename) || !filename ||
       !_accessor_value_get(values_acc, 2, &dir) ||
       !_accessor_value_get(values_acc, 3, &size) ||
       !_accessor_value_get(values_acc, 4, &mtime) ||
       !_accessor_value_get(values_acc, 5, &mime_type))
     {
        ERR("missing Efl.Model data");
        goto cancel;
     }

   if (!_filter_child(sd, path, filename, dir, mime_type))
     goto cancel;

   it_data->path = eina_stringshare_add(path);
   it_data->filename = eina_stringshare_add(filename);
   it_data->size = size;
   it_data->mtime = mtime;
   it_data->mime_type = eina_stringshare_add(mime_type);
   it_data->is_dir = dir;

   if (dir)
     itcn = ELM_DIRECTORY;
   else
     {
        if (evas_object_image_extension_can_load_get(filename))
          itcn = ELM_FILE_IMAGE;
     }

   if (sd->mode == ELM_FILESELECTOR_LIST)
     elm_genlist_item_sorted_insert(sd->files_view, list_itc[itcn],
                                    it_data,
                                    NULL,
                                    (sd->expand && itcn == ELM_DIRECTORY)
                                    ? ELM_GENLIST_ITEM_TREE : ELM_GENLIST_ITEM_NONE,
                                    _file_list_cmp, NULL, NULL);
   else
     elm_gengrid_item_sorted_insert(sd->files_view, grid_itc[itcn],
                                    it_data,
                                    _file_grid_cmp, NULL, NULL);

   efl_unref(obj);
   return;

cancel:
   efl_unref(obj);
   efl_unref(it_data->model);
   efl_unref(it_data->parent_model);
   eina_stringshare_del(it_data->parent_path);
   free(it_data);
}

static void
_resource_created(void *data, const Efl_Event *event)
{
   Elm_Fileselector *fs = data;
   Efl_Model_Children_Event* evt = event->info;
   Efl_Model *child = evt->child;
   Efl_Future *futures[7] = {NULL,};
   Efl_Future *future_all = NULL;
   Elm_Fileselector_Item_Data *it_data = NULL;

   ELM_FILESELECTOR_DATA_GET(fs, sd);

   if (!sd || !sd->monitoring || sd->model != event->object)
     return;

   it_data = calloc(1, sizeof(Elm_Fileselector_Item_Data));
   if (!it_data)
     return;

   it_data->model = efl_ref(child);
   it_data->user_data = efl_ref(fs);
   it_data->parent_model = efl_ref(sd->model);
   it_data->parent_path = eina_stringshare_add(sd->path);

   future_all = efl_future_all
     (
      futures[0] = efl_model_property_get(child, "path"),
      futures[1] = efl_model_property_get(child, "filename"),
      futures[2] = efl_model_property_get(child, "is_dir"),
      futures[3] = efl_model_property_get(child, "size"),
      futures[4] = efl_model_property_get(child, "mtime"),
      futures[5] = efl_model_property_get(child, "mime_type")
     );

   efl_future_then(future_all, _resource_created_then, _resource_then_error, NULL, it_data);

   return;
}

static void
_resource_deleted(void *data, const Efl_Event *event)
{
   Evas_Object *obj = data;
   Efl_Model_Children_Event* evt = event->info;
   Efl_Model *child = evt->child;
   Elm_Object_Item *it = NULL;
   Eina_Bool selected = EINA_FALSE;

   ELM_FILESELECTOR_DATA_GET(obj, sd);

   if (!sd || !sd->monitoring || sd->model != event->object)
     return;

   if (sd->mode == ELM_FILESELECTOR_LIST)
     {
        it = elm_genlist_first_item_get(sd->files_view);
        while (it)
          {
             Elm_Fileselector_Item_Data *it_data = elm_object_item_data_get(it);
             if (child == it_data->model)
               {
                  selected = elm_genlist_item_selected_get(it);
                  break;
               }
             it = elm_genlist_item_next_get(it);
          }
     }
   else
     {
        it = elm_gengrid_first_item_get(sd->files_view);
        while (it)
          {
             Elm_Fileselector_Item_Data *it_data = elm_object_item_data_get(it);
             if (child == it_data->model)
               {
                  selected = elm_genlist_item_selected_get(it);
                  break;
               }
             it = elm_gengrid_item_next_get(it);
          }
     }

   if (it) elm_wdg_item_del(it);

   if (selected)
     {
        if (sd->multi)
          {
             Eina_List *li, *l;
             Elm_Object_Item *item;
             Eina_Strbuf *buf;
             Eina_Bool first = EINA_TRUE;

             buf = eina_strbuf_new();
             EINA_LIST_FOREACH_SAFE(sd->multi_selection, li, l, item)
               {
                  Elm_Fileselector_Item_Data *it_data = elm_object_item_data_get(item);
                  if (child == it_data->model)
                    {
                       sd->multi_selection = eina_list_remove_list(sd->multi_selection, li);
                    }
                  else
                    {
                       if (!first)
                         eina_strbuf_append_length(buf, ", ", 2);
                       else
                         first = EINA_FALSE;

                       eina_strbuf_append(buf, it_data->filename);
                    }
               }

             elm_object_text_set(sd->name_entry, eina_strbuf_string_get(buf));
             eina_strbuf_free(buf);
          }
        else
          elm_object_text_set(sd->name_entry, "");
     }

   return;
}

static void
_preedit_cb(void *data, const Efl_Event *event)
{
   ELM_FILESELECTOR_DATA_GET(data, sd);

   sd->search_string = elm_entry_entry_get(event->object);

   if (sd->search_string && sd->model)
     _populate(data, sd->model, NULL, NULL);
}

EOLIAN static void
_elm_fileselector_efl_canvas_group_group_add(Eo *obj, Elm_Fileselector_Data *priv)
{
   Evas_Object *ic, *bt, *en, *pb;
   const char *data;
   char buf[1024];

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);
   elm_widget_can_focus_set(obj, EINA_FALSE);

   priv->expand = !!_elm_config->fileselector_expand_enable;
   priv->double_tap_navigation = !!_elm_config->fileselector_double_tap_navigation_enable;

   if (!elm_layout_theme_set
       (obj, "fileselector", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   data = edje_object_data_get
       (wd->resize_obj, "path_separator");
   if (data) priv->path_separator = data;
   else priv->path_separator = "/";

   snprintf(buf, sizeof(buf), "fileselector/%s", elm_widget_style_get(obj));

   // up btn
   ic = elm_icon_add(obj);
   elm_icon_standard_set(ic, "go-up");
   bt = elm_button_add(obj);
   efl_ui_mirrored_automatic_set(bt, EINA_FALSE);
   elm_object_part_content_set(bt, "icon", ic);
   elm_object_domain_translatable_text_set(bt, PACKAGE, N_("Up"));
   efl_event_callback_add
     (bt, EFL_UI_EVENT_CLICKED, _on_dir_up, obj);

   priv->up_button = bt;
   elm_object_style_set(priv->up_button, buf);
   elm_object_part_content_set(obj, "elm.swallow.up", priv->up_button);

   // home btn
   ic = elm_icon_add(obj);
   elm_icon_standard_set(ic, "go-home");
   bt = elm_button_add(obj);
   efl_ui_mirrored_automatic_set(bt, EINA_FALSE);
   elm_object_part_content_set(bt, "icon", ic);
   elm_object_domain_translatable_text_set(bt, PACKAGE, N_("Home"));
   efl_event_callback_add
     (bt, EFL_UI_EVENT_CLICKED, _home, obj);

   priv->home_button = bt;
   elm_object_style_set(priv->home_button, buf);
   elm_object_part_content_set(obj, "elm.swallow.home", priv->home_button);

   // search entry
   ic = elm_icon_add(obj);
   elm_icon_standard_set(ic, "edit-find");
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   en = elm_entry_add(obj);
   elm_entry_scrollable_set(en, EINA_TRUE);
   efl_ui_mirrored_automatic_set(en, EINA_FALSE);
   elm_entry_editable_set(en, EINA_TRUE);
   elm_entry_single_line_set(en, EINA_TRUE);
   elm_entry_line_wrap_set(en, ELM_WRAP_CHAR);
   elm_object_domain_translatable_part_text_set(en, "guide",
                                                PACKAGE, N_("Search"));
   elm_object_part_content_set(en, "icon", ic);
   elm_entry_icon_visible_set(en, EINA_TRUE);
   efl_event_callback_add
     (en, ELM_ENTRY_EVENT_CHANGED, _preedit_cb, obj);
   evas_object_data_set(obj, "search", en);

   priv->search_entry = en;
   elm_object_style_set(priv->search_entry, buf);
   elm_object_part_content_set(obj, "elm.swallow.search", priv->search_entry);

   // spinner
   pb = elm_progressbar_add(obj);
   priv->spinner = pb;
   elm_progressbar_pulse_set(priv->spinner, EINA_TRUE);
   elm_object_style_set(priv->spinner, "wheel");
   elm_object_part_content_set(obj, "elm.swallow.spinner", priv->spinner);

   priv->thumbnail_size.w = ITEM_SIZE_DEFAULT;
   priv->thumbnail_size.h = ITEM_SIZE_DEFAULT;

   priv->sort_type = ELM_FILESELECTOR_SORT_BY_FILENAME_ASC;
   priv->sort_method = _filename_cmp;

   // path entry
   en = elm_entry_add(obj);
   elm_entry_scrollable_set(en, EINA_TRUE);
   efl_ui_mirrored_automatic_set(en, EINA_FALSE);
   elm_entry_single_line_set(en, EINA_TRUE);
   elm_entry_line_wrap_set(en, ELM_WRAP_CHAR);

   efl_event_callback_add
     (en, ELM_ENTRY_EVENT_ANCHOR_CLICKED, _anchor_clicked, obj);
   efl_event_callback_add
     (en, ELM_WIDGET_EVENT_FOCUSED, _on_text_focused, obj);
   efl_event_callback_add
     (en, ELM_WIDGET_EVENT_UNFOCUSED, _on_text_unfocused, obj);
   efl_event_callback_add
     (en, ELM_ENTRY_EVENT_ACTIVATED, _on_text_activated, obj);

   priv->path_entry = en;
   elm_object_style_set(priv->path_entry, buf);
   elm_object_part_content_set(obj, "elm.swallow.path", priv->path_entry);

   // name entry
   en = elm_entry_add(obj);
   elm_entry_scrollable_set(en, EINA_TRUE);
   efl_ui_mirrored_automatic_set(en, EINA_FALSE);
   elm_entry_editable_set(en, EINA_TRUE);
   elm_entry_single_line_set(en, EINA_TRUE);
   elm_entry_line_wrap_set(en, ELM_WRAP_CHAR);

   priv->name_entry = en;
   elm_object_style_set(priv->name_entry, buf);
   elm_object_part_content_set(obj, "elm.swallow.filename", priv->name_entry);

   // ok cancel button
   elm_fileselector_buttons_ok_cancel_set(obj, EINA_TRUE);
   elm_fileselector_is_save_set(obj, EINA_FALSE);

   // files_view
   priv->files_view = _files_list_add(obj);
   elm_object_part_content_set(obj, "elm.swallow.files", priv->files_view);

   elm_layout_sizing_eval(obj);

   _focus_chain_update(obj, priv);
}

EOLIAN static void
_elm_fileselector_efl_canvas_group_group_del(Eo *obj, Elm_Fileselector_Data *sd)
{
   Elm_Fileselector_Filter *filter;

   if (sd->current_populate_lreq)
     sd->current_populate_lreq->valid = EINA_FALSE;
   sd->current_populate_lreq = NULL;

   if (sd->model)
     _monitoring_stop(obj, sd, sd->model);

   EINA_LIST_FREE(sd->filter_list, filter)
     {
        eina_stringshare_del(filter->filter_name);

        if (filter->filter_type == ELM_FILESELECTOR_MIME_FILTER)
          {
             free(filter->filter.mime_types[0]);
             free(filter->filter.mime_types);
          }
        else
          free(filter->filter.custom);

        free(filter);
     }

   sd->multi_selection = eina_list_free(sd->multi_selection);
   sd->multi_selection_tmp = eina_list_free(sd->multi_selection_tmp);
   sd->files_view = NULL;
   _elm_fileselector_smart_del_do(obj, sd);
}

EAPI Evas_Object *
elm_fileselector_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return efl_add(MY_CLASS, parent, efl_canvas_object_legacy_ctor(efl_added));
}

EOLIAN static Eo *
_elm_fileselector_efl_object_constructor(Eo *obj, Elm_Fileselector_Data *sd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   sd->obj = obj;
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_FILE_CHOOSER);

   return obj;
}

static void
_legacy_smart_callback_caller_path_then(void *data, Efl_Event const *event)
{
   Legacy_Event_Path_Then_Data *evt_data = data;
   _event_to_legacy_call(evt_data->eo_obj, evt_data->evt_desc, ((Efl_Future_Event_Success*)event->info)->value);
   free(data);
}

static void
_legacy_smart_callback_caller_path_then_error(void *data, Efl_Event const* event)
{
   Eina_Error err = ((Efl_Future_Event_Failure*)event->info)->error;
   ERR("Efl.Model property \"path\" error: %s", eina_error_msg_get(err));
   free(data);
}

static Eina_Bool
_from_efl_event_call(Elm_Fileselector *fs, const Efl_Event_Description *evt_desc, Efl_Model *model)
{
   Efl_Future *future;
   Legacy_Event_Path_Then_Data *evt_data;

   evt_data = calloc(1, sizeof(Legacy_Event_Path_Then_Data));
   evt_data->eo_obj = fs;
   evt_data->evt_desc = evt_desc;

   // Call legacy smart callback with path
   future = efl_model_property_get(model, "path");
   efl_future_then(future,
                   _legacy_smart_callback_caller_path_then,
                   _legacy_smart_callback_caller_path_then_error,
                   NULL,
                   evt_data);

   // Call Eo event with model
   return efl_event_callback_call(fs, evt_desc, model);
}

static Eina_Bool
_from_legacy_event_call(Elm_Fileselector *fs, Elm_Fileselector_Data *sd, const Efl_Event_Description *legacy_desc, const Efl_Event_Description *evt_desc, const char *path)
{
   const Efl_Class *model_cls = NULL;
   if (!sd->model)
     model_cls = EIO_MODEL_CLASS;
   else
     model_cls = efl_class_get(sd->model);

   Efl_Model *model = efl_add(model_cls, NULL);
   _model_str_property_set(model, "path", path);

   // Call Eo event with model
   efl_event_callback_call(fs, evt_desc, model);

   efl_unref(model);

   // Call legacy smart callback with path
   return efl_event_callback_call(fs, legacy_desc, (void *)path);
}

EOLIAN static Eina_Bool
_elm_fileselector_efl_object_event_callback_legacy_call(Eo *obj, Elm_Fileselector_Data *sd,
   const Efl_Event_Description *desc, void *event_info)
{
   if (desc->legacy_is)
     {
        const Efl_Event_Description *evt_desc = NULL;
        if (strcmp(desc->name, "selected") == 0)
          evt_desc = EFL_UI_EVENT_SELECTED;
        else if (strcmp(desc->name, "activated") == 0)
          evt_desc = ELM_FILESELECTOR_EVENT_ACTIVATED;
        else if (strcmp(desc->name, "directory,open") == 0)
          evt_desc = ELM_FILESELECTOR_EVENT_DIRECTORY_OPEN;
        else if (strcmp(desc->name, "done") == 0)
          evt_desc = ELM_FILESELECTOR_EVENT_DONE;
        else if (strcmp(desc->name, "selected,invalid") == 0)
          evt_desc = ELM_FILESELECTOR_EVENT_SELECTED_INVALID;
        else
          return efl_event_callback_legacy_call(efl_super(obj, MY_CLASS), desc, event_info);

        return _from_legacy_event_call(obj, sd, desc, evt_desc, event_info);
     }

   if (desc == EFL_UI_EVENT_SELECTED ||
       desc == ELM_FILESELECTOR_EVENT_ACTIVATED ||
       desc == ELM_FILESELECTOR_EVENT_DIRECTORY_OPEN ||
       desc == ELM_FILESELECTOR_EVENT_DONE ||
       desc == ELM_FILESELECTOR_EVENT_SELECTED_INVALID)
     {
        return _from_efl_event_call(obj, desc, event_info);
     }

   return efl_event_callback_legacy_call(efl_super(obj, MY_CLASS), desc, event_info);
}

EAPI void
elm_fileselector_is_save_set(Evas_Object *obj,
                             Eina_Bool is_save)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   elm_interface_fileselector_is_save_set(obj, is_save);
}

EOLIAN static void
_elm_fileselector_elm_interface_fileselector_is_save_set(Eo *obj, Elm_Fileselector_Data *sd, Eina_Bool is_save)
{
   elm_object_disabled_set(sd->name_entry, !is_save);

   if (is_save) elm_layout_signal_emit(obj, "elm,state,save,on", "elm");
   else elm_layout_signal_emit(obj, "elm,state,save,off", "elm");

   _focus_chain_update(obj, sd);
}

EAPI Eina_Bool
elm_fileselector_is_save_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, EINA_FALSE);
   return elm_interface_fileselector_is_save_get((Eo *) obj);
}

EOLIAN static Eina_Bool
_elm_fileselector_elm_interface_fileselector_is_save_get(Eo *obj EINA_UNUSED, Elm_Fileselector_Data *sd)
{
   return !elm_object_disabled_get(sd->name_entry);
}

EAPI void
elm_fileselector_folder_only_set(Evas_Object *obj,
                                 Eina_Bool only)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   elm_interface_fileselector_folder_only_set(obj, only);
}

EOLIAN static void
_elm_fileselector_elm_interface_fileselector_folder_only_set(Eo *obj, Elm_Fileselector_Data *sd, Eina_Bool only)
{
   if (sd->only_folder == only) return;

   sd->only_folder = !!only;
   if (sd->model)
     {
        _schedule_populate(obj, sd, sd->model, NULL);
     }
}

EAPI Eina_Bool
elm_fileselector_folder_only_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, EINA_FALSE);
   return elm_interface_fileselector_folder_only_get((Eo *) obj);
}

EOLIAN static Eina_Bool
_elm_fileselector_elm_interface_fileselector_folder_only_get(Eo *obj EINA_UNUSED, Elm_Fileselector_Data *sd)
{
   return sd->only_folder;
}

EOLIAN static void
_elm_fileselector_buttons_ok_cancel_set(Eo *obj, Elm_Fileselector_Data *sd, Eina_Bool visible)
{
   Evas_Object *bt;

   visible = !!visible;

   if (!visible == !sd->ok_button) return;

   if (visible)
     {
        // ok btn
        bt = elm_button_add(obj);
        efl_ui_mirrored_automatic_set(bt, EINA_FALSE);
        elm_object_domain_translatable_text_set(bt, PACKAGE, N_("OK"));

        efl_event_callback_add(bt, EFL_UI_EVENT_CLICKED, _ok, obj);

        sd->ok_button = bt;
        elm_object_part_content_set(obj, "elm.swallow.ok", sd->ok_button);

        // cancel btn
        bt = elm_button_add(obj);
        efl_ui_mirrored_automatic_set(bt, EINA_FALSE);
        elm_object_domain_translatable_text_set(bt, PACKAGE, N_("Cancel"));

        efl_event_callback_add(bt, EFL_UI_EVENT_CLICKED, _canc, obj);

        sd->cancel_button = bt;
        elm_object_part_content_set(obj, "elm.swallow.cancel", sd->cancel_button);
     }
   else
     {
        ELM_SAFE_FREE(sd->cancel_button, evas_object_del);
        ELM_SAFE_FREE(sd->ok_button, evas_object_del);
     }

   _focus_chain_update(obj, sd);
}

EOLIAN static Eina_Bool
_elm_fileselector_buttons_ok_cancel_get(Eo *obj EINA_UNUSED, Elm_Fileselector_Data *sd)
{
   return sd->ok_button ? EINA_TRUE : EINA_FALSE;
}

EAPI void
elm_fileselector_expandable_set(Evas_Object *obj,
                                Eina_Bool expand)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   elm_interface_fileselector_expandable_set(obj, expand);
}

EOLIAN static void
_elm_fileselector_elm_interface_fileselector_expandable_set(Eo *obj, Elm_Fileselector_Data *sd, Eina_Bool expand)
{
   sd->expand = !!expand;

   if (sd->model)
     {
        _schedule_populate(obj, sd, sd->model, NULL);
     }
}

EAPI Eina_Bool
elm_fileselector_expandable_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, EINA_FALSE);
   return elm_interface_fileselector_expandable_get((Eo *) obj);
}

EOLIAN static Eina_Bool
_elm_fileselector_elm_interface_fileselector_expandable_get(Eo *obj EINA_UNUSED, Elm_Fileselector_Data *sd)
{
   return sd->expand;
}

EAPI void
elm_fileselector_path_set(Evas_Object *obj,
                          const char *_path)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   const Efl_Class *cls = efl_class_get(obj);
   if (cls == ELM_FILESELECTOR_CLASS)
     _elm_fileselector_path_set_internal(obj, _path);
   else if (cls == ELM_FILESELECTOR_ENTRY_CLASS)
     _elm_fileselector_entry_path_set_internal(obj, _path);
   else if (cls == ELM_FILESELECTOR_BUTTON_CLASS)
     _elm_fileselector_button_path_set_internal(obj, _path);
   else
     ERR("Unknown Elm.Fileselector class");
}

void
_elm_fileselector_path_set_internal(Evas_Object *obj, const char *_path)
{
   Eio_Model *model = efl_add(EIO_MODEL_CLASS, NULL, eio_model_path_set(efl_added, _path));
   if (!model)
     {
        ERR("Efl.Model allocation error");
        return;
     }
   elm_interface_fileselector_model_set(obj, model);
   efl_unref(model);
}

EOLIAN static void
_elm_fileselector_elm_interface_fileselector_model_set(Eo *obj, Elm_Fileselector_Data *sd, Efl_Model *model)
{
   _schedule_populate(obj, sd, model, NULL);
}

EAPI const char *
elm_fileselector_path_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, NULL);
   const Efl_Class *cls = efl_class_get(obj);
   if (cls == ELM_FILESELECTOR_CLASS)
     return _elm_fileselector_path_get_internal(obj);
   else if (cls == ELM_FILESELECTOR_ENTRY_CLASS)
     return _elm_fileselector_entry_path_get_internal(obj);
   else if (cls == ELM_FILESELECTOR_BUTTON_CLASS)
     return _elm_fileselector_button_path_get_internal(obj);
   else
     {
        ERR("Unknown Elm.Fileselector class");
        return NULL;
     }
}

const char *
_elm_fileselector_path_get_internal(const Evas_Object *obj)
{
   ELM_FILESELECTOR_DATA_GET(obj, sd);
   return sd->path;
}

EOLIAN static Efl_Model *
_elm_fileselector_elm_interface_fileselector_model_get(Eo *obj EINA_UNUSED, Elm_Fileselector_Data *sd)
{
   return sd->model;
}

EAPI void
elm_fileselector_mode_set(Evas_Object *obj,
                          Elm_Fileselector_Mode mode)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   elm_interface_fileselector_mode_set(obj, mode);
}

EOLIAN static void
_elm_fileselector_elm_interface_fileselector_mode_set(Eo *obj, Elm_Fileselector_Data *sd, Elm_Fileselector_Mode mode)
{
   Evas_Object *old;

   if (mode == sd->mode) return;

   old = elm_layout_content_unset(obj, "elm.swallow.files");

   if (mode == ELM_FILESELECTOR_LIST)
     {
        sd->files_view = _files_list_add(obj);
        if (sd->multi)
          elm_genlist_multi_select_set(sd->files_view, EINA_TRUE);
     }
   else
     {
        sd->files_view = _files_grid_add(obj);
        if (sd->multi)
          elm_gengrid_multi_select_set(sd->files_view, EINA_TRUE);
     }

   elm_layout_content_set(obj, "elm.swallow.files", sd->files_view);

   evas_object_del(old);

   sd->mode = mode;

   elm_obj_widget_theme_apply(obj);
   if (sd->model)
     {
        _schedule_populate(obj, sd, sd->model, NULL);
     }
}

EAPI Elm_Fileselector_Mode
elm_fileselector_mode_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, ELM_FILESELECTOR_LAST);
   return elm_interface_fileselector_mode_get((Eo *) obj);
}

EOLIAN static Elm_Fileselector_Mode
_elm_fileselector_elm_interface_fileselector_mode_get(Eo *obj EINA_UNUSED, Elm_Fileselector_Data *sd)
{
   return sd->mode;
}

EAPI void
elm_fileselector_multi_select_set(Evas_Object *obj, Eina_Bool multi)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   elm_interface_fileselector_multi_select_set(obj, multi);
}

EOLIAN static void
_elm_fileselector_elm_interface_fileselector_multi_select_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Data *sd, Eina_Bool multi)
{
   multi = !!multi;
   if (sd->multi == multi) return;
   sd->multi = multi;

   if (sd->mode == ELM_FILESELECTOR_LIST)
     elm_genlist_multi_select_set(sd->files_view, multi);
   else
     elm_gengrid_multi_select_set(sd->files_view, multi);

   if (!sd->multi)
     {
        _clear_selections(sd, NULL);

        sd->multi_selection = eina_list_free(sd->multi_selection);
     }
   else
     {
        const Eina_List *selected_items, *li;
        const Elm_Object_Item *it;

        if (sd->mode == ELM_FILESELECTOR_LIST)
          selected_items = elm_genlist_selected_items_get(sd->files_view);
        else
          selected_items = elm_gengrid_selected_items_get(sd->files_view);

        EINA_LIST_FOREACH(selected_items, li, it)
          {
             sd->multi_selection = eina_list_append(sd->multi_selection, it);
          }
     }
}

EAPI Eina_Bool
elm_fileselector_multi_select_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, EINA_FALSE);
   return elm_interface_fileselector_multi_select_get((Eo *) obj);
}

EOLIAN static Eina_Bool
_elm_fileselector_elm_interface_fileselector_multi_select_get(Eo *obj EINA_UNUSED, Elm_Fileselector_Data *sd)
{
   return sd->multi;
}

static Elm_Fileselector_Item_Data *
_selected_item_data_get(Elm_Fileselector_Data *sd)
{
   if (sd->mode == ELM_FILESELECTOR_LIST)
     {
        Elm_Object_Item *gl_it = elm_genlist_selected_item_get(sd->files_view);

        if (gl_it) return elm_object_item_data_get(gl_it);
     }
   else
     {
        Elm_Object_Item *gg_it = elm_gengrid_selected_item_get(sd->files_view);

        if (gg_it) return elm_object_item_data_get(gg_it);
     }
   return NULL;
}

EAPI const char *
elm_fileselector_selected_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, NULL);

   const Efl_Class *cls = efl_class_get(obj);
   if (cls == ELM_FILESELECTOR_CLASS)
     return _elm_fileselector_selected_get_internal(obj);
   else if (cls == ELM_FILESELECTOR_ENTRY_CLASS)
     return _elm_fileselector_entry_selected_get_internal(obj);
   else if (cls == ELM_FILESELECTOR_BUTTON_CLASS)
     return _elm_fileselector_button_selected_get_internal(obj);
   else
     {
        ERR("Unknown Elm.Fileselector class");
        return NULL;
     }
}

const char *
_elm_fileselector_selected_get_internal(const Evas_Object *obj)
{
   ELM_FILESELECTOR_DATA_GET(obj, sd);
   if (!sd->path) return NULL;

   Elm_Fileselector_Item_Data *it_data = _selected_item_data_get(sd);
   if (it_data)
     return it_data->path;

   return sd->path;
}

EOLIAN static Efl_Model *
_elm_fileselector_elm_interface_fileselector_selected_model_get(Eo *fs EINA_UNUSED, Elm_Fileselector_Data *sd)
{
   if (!sd->model)
     {
        return NULL;
     }

   Elm_Fileselector_Item_Data *it_data = _selected_item_data_get(sd);
   if (it_data)
     return it_data->model;

   return sd->model;
}

EAPI Eina_Bool
elm_fileselector_selected_set(Evas_Object *obj,
                              const char *_path)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, EINA_FALSE);

   const Efl_Class *cls = efl_class_get(obj);
   if (cls == ELM_FILESELECTOR_CLASS)
     return _elm_fileselector_selected_set_internal(obj, _path);
   else if (cls == ELM_FILESELECTOR_ENTRY_CLASS)
     return _elm_fileselector_entry_selected_set_internal(obj, _path);
   else if (cls == ELM_FILESELECTOR_BUTTON_CLASS)
     return _elm_fileselector_button_selected_set_internal(obj, _path);
   else
     {
        ERR("Unknown Elm.Fileselector class");
        return EINA_FALSE;
     }
}

Eina_Bool
_elm_fileselector_selected_set_internal(Evas_Object *obj, const char *_path)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, EINA_FALSE);
   Eina_Bool ret = EINA_FALSE;
   char *dir;
   char *path;
   Eio_Model *model = NULL;
   Eio_Model *parent = NULL;
   ELM_FILESELECTOR_DATA_GET(obj, sd);

   path = ecore_file_realpath(_path);

   if (ecore_file_is_dir(path))
     {
         model = efl_add(EIO_MODEL_CLASS, NULL, eio_model_path_set(efl_added, path));
         if (!model)
           {
              ERR("Efl.Model allocation error");
              goto clean_up;
           }

        _schedule_populate(obj, sd, model, NULL);
        efl_unref(model);
        ret = EINA_TRUE;
     }
   else
     {
        if (!ecore_file_exists(path))
          {
             goto clean_up;
          }

        model = efl_add(EIO_MODEL_CLASS, NULL, eio_model_path_set(efl_added, path));
        if (!model)
          {
             ERR("Efl.Model allocation error");
             goto clean_up;
          }

        dir = ecore_file_dir_get(path);
        parent = efl_add(EIO_MODEL_CLASS, NULL, eio_model_path_set(efl_added, dir));
        if (parent)
          {
             _schedule_populate(obj, sd, parent, model);
             efl_unref(parent);
             ret = EINA_TRUE;
          }
        free(dir);
        efl_unref(model);
     }

clean_up:
   free(path);

   return ret;
}

static void
_selected_model_set_free_fs_data(Elm_Fileselector *fs)
{
   efl_key_ref_set(fs, _selected_model_set_model_key, NULL);
   efl_key_data_set(fs, _selected_model_set_promise_owner_key, NULL);
   efl_unref(fs);
}

static void
_selected_model_set_then_error(void *data, Efl_Event const* event)
{
   Eina_Error err = ((Efl_Future_Event_Failure*)event->info)->error;
   Efl_Promise *promise_owner = efl_key_data_get(data, _selected_model_set_promise_owner_key);
   if (promise_owner)
     efl_promise_failed_set(promise_owner, err);
   _selected_model_set_free_fs_data(data);
}

static void
_selected_model_set_is_dir_then(void *data, Efl_Event const *event)
{
   Elm_Fileselector *fs = data;
   Eina_Bool is_dir = EINA_FALSE;
   Efl_Model *model = efl_key_ref_get(fs, _selected_model_set_model_key);
   Efl_Promise *promise_owner = efl_key_data_get(fs, _selected_model_set_promise_owner_key);
   ELM_FILESELECTOR_DATA_GET(fs, sd);

   eina_value_get((Eina_Value*)((Efl_Future_Event_Success*)event->info)->value, &is_dir);
   if (is_dir)
     {
        _schedule_populate(fs, sd, model, NULL);
        if (promise_owner)
          {
             efl_ref(model);
             efl_promise_value_set(promise_owner, model, _model_free_eo_cb);
          }
     }
   else
     {
        Efl_Model *parent = efl_parent_get(model);
        if (parent)
          {
             _schedule_populate(fs, sd, parent, model);

             if (promise_owner)
               {
                  efl_ref(model);
                  efl_promise_value_set(promise_owner, model, _model_free_eo_cb);
               }
          }
        else
          {
             if (promise_owner)
               efl_promise_failed_set(promise_owner, ELM_FILESELECTOR_ERROR_UNKNOWN);
          }
     }
   _selected_model_set_free_fs_data(fs);
}

EOLIAN static Efl_Future*
_elm_fileselector_elm_interface_fileselector_selected_model_set(Eo *obj, Elm_Fileselector_Data *sd EINA_UNUSED, Efl_Model *model)
{
   Efl_Future *future = NULL;
   Efl_Promise* promise = efl_add(EFL_PROMISE_CLASS, obj);
   if (!model)
     {
        efl_promise_failed_set(promise, ELM_FILESELECTOR_ERROR_INVALID_MODEL);
        return efl_promise_future_get(promise);
     }
   future = efl_model_property_get(model, "is_dir");

   efl_key_ref_set(obj, _selected_model_set_model_key, model);
   efl_key_data_set(obj, _selected_model_set_promise_owner_key, promise);

   efl_future_then(future, _selected_model_set_is_dir_then, _selected_model_set_then_error, NULL, efl_ref(obj));
   return efl_promise_future_get(promise);
}

EAPI const Eina_List *
elm_fileselector_selected_paths_get(const Evas_Object* obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, NULL);

   const Efl_Class *cls = efl_class_get(obj);
   if (cls == ELM_FILESELECTOR_CLASS)
     return _elm_fileselector_selected_paths_get_internal(obj);
   else if (cls == ELM_FILESELECTOR_BUTTON_CLASS)
     return _elm_fileselector_button_selected_paths_get_internal(obj);
   else
     ERR("Unknown Elm.Fileselector class");
   return NULL;
}

const Eina_List *
_elm_fileselector_selected_paths_get_internal(const Evas_Object* obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, NULL);
   Eina_List *l;
   Elm_Object_Item *item;
   ELM_FILESELECTOR_DATA_GET(obj, sd);

   if (!sd->multi)
     return NULL;

   if (sd->multi_selection_tmp)
     {
        sd->multi_selection_tmp = eina_list_free(sd->multi_selection_tmp);
     }

   EINA_LIST_FOREACH(sd->multi_selection, l, item)
     {
        Elm_Fileselector_Item_Data *it_data = elm_object_item_data_get(item);
        sd->multi_selection_tmp = eina_list_append(sd->multi_selection_tmp, it_data->path);
     }
   return sd->multi_selection_tmp;
}

EOLIAN static const Eina_List*
_elm_fileselector_elm_interface_fileselector_selected_models_get(Eo *obj EINA_UNUSED, Elm_Fileselector_Data *sd)
{
   Eina_List *l;
   Elm_Object_Item *item;
   if (!sd->multi)
     return NULL;

   if (sd->multi_selection_tmp)
     {
        sd->multi_selection_tmp = eina_list_free(sd->multi_selection_tmp);
     }

   EINA_LIST_FOREACH(sd->multi_selection, l, item)
     {
        Elm_Fileselector_Item_Data *it_data = elm_object_item_data_get(item);
        sd->multi_selection_tmp = eina_list_append(sd->multi_selection_tmp, it_data->model);
     }
   return sd->multi_selection_tmp;
}

EAPI const char *
elm_fileselector_current_name_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, NULL);
   const char *ret = NULL;
   ret = elm_interface_fileselector_current_name_get((Eo *) obj);
   return ret;
}

EOLIAN static const char *
_elm_fileselector_elm_interface_fileselector_current_name_get(Eo *obj EINA_UNUSED, Elm_Fileselector_Data *sd)
{
   return elm_object_text_get(sd->name_entry);
}

EAPI void
elm_fileselector_current_name_set(Evas_Object *obj,
                                  const char *name)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   elm_interface_fileselector_current_name_set((Eo *) obj, name);
}

EOLIAN static void
_elm_fileselector_elm_interface_fileselector_current_name_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Data *sd, const char *name)
{
   elm_object_text_set(sd->name_entry, name);
}

static Elm_Fileselector_Filter *
_filter_add(Elm_Fileselector_Data *sd, const char *filter_name)
{
   Elm_Fileselector_Filter *ff;
   ff = calloc(1, sizeof(Elm_Fileselector_Filter));
   if (!ff) return NULL;

   ff->filter_name = eina_stringshare_add(filter_name);
   ff->sd = sd;

   return ff;
}

EAPI Eina_Bool
elm_fileselector_mime_types_filter_append(Evas_Object *obj, const char *mime_type, const char *filter_name)
{
   ELM_FILESELECTOR_CHECK(obj) EINA_FALSE;
   return elm_interface_fileselector_mime_types_filter_append(obj, mime_type, filter_name);
}

EOLIAN static Eina_Bool
_elm_fileselector_elm_interface_fileselector_mime_types_filter_append(Eo *obj, Elm_Fileselector_Data *sd, const char *mime_types, const char *filter_name)
{
   Elm_Fileselector_Filter *ff;
   char buf[1024];

   if (!mime_types) return EINA_FALSE;

   ff = _filter_add(sd, filter_name ? filter_name : mime_types);
   if (!ff) return EINA_FALSE;

   ff->filter_type = ELM_FILESELECTOR_MIME_FILTER;

   ff->filter.mime_types = eina_str_split(mime_types, ",", 0);

   if (!sd->filter_list)
     {
        sd->current_filter = ff;
        sd->filter_hoversel = elm_hoversel_add(obj);
        elm_object_text_set(sd->filter_hoversel, ff->filter_name);
        snprintf(buf, sizeof(buf), "fileselector/actions/%s", elm_widget_style_get(obj));
        elm_widget_style_set(sd->filter_hoversel, buf);
        elm_object_part_content_set(obj, "elm.swallow.filters", sd->filter_hoversel);
     }
   elm_hoversel_item_add(sd->filter_hoversel, ff->filter_name, NULL, ELM_ICON_NONE, _current_filter_changed, ff);

   sd->filter_list = eina_list_append(sd->filter_list, ff);

   if (sd->model)
     {
        _schedule_populate(obj, sd, sd->model, NULL);
     }

   return EINA_TRUE;
}

EAPI Eina_Bool
elm_fileselector_custom_filter_append(Evas_Object *obj, Elm_Fileselector_Filter_Func func, void *data, const char *filter_name)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, EINA_FALSE);
   return elm_interface_fileselector_custom_filter_append(obj, func, data, filter_name);
}

EOLIAN static Eina_Bool
_elm_fileselector_elm_interface_fileselector_custom_filter_append(Eo *obj, Elm_Fileselector_Data *sd, Elm_Fileselector_Filter_Func func, void *data, const char *filter_name)
{
   Elm_Fileselector_Filter *ff;
   Elm_Fileselector_Custom_Filter *custom_filter;
   char buf[1024];

   if (!func) return EINA_FALSE;

   custom_filter = calloc(1, sizeof(Elm_Fileselector_Custom_Filter));
   if (!custom_filter) return EINA_FALSE;

   ff = _filter_add(sd, filter_name ? filter_name : "custom");
   if (!ff)
     {
        free(custom_filter);
        return EINA_FALSE;
     }

   ff->filter_type = ELM_FILESELECTOR_CUSTOM_FILTER;
   ff->filter.custom = custom_filter;
   ff->filter.custom->func = func;
   ff->filter.custom->data = data;

   if (!sd->filter_list)
     {
        sd->current_filter = ff;
        sd->filter_hoversel = elm_hoversel_add(obj);
        elm_object_text_set(sd->filter_hoversel, ff->filter_name);
        snprintf(buf, sizeof(buf), "fileselector/actions/%s", elm_widget_style_get(obj));
        elm_widget_style_set(sd->filter_hoversel, buf);
        elm_object_part_content_set(obj, "elm.swallow.filters", sd->filter_hoversel);
     }
   elm_hoversel_item_add(sd->filter_hoversel, ff->filter_name, NULL, ELM_ICON_NONE, _current_filter_changed, ff);

   sd->filter_list = eina_list_append(sd->filter_list, ff);

   if (sd->model)
     {
        _schedule_populate(obj, sd, sd->model, NULL);
     }

   return EINA_TRUE;
}

EAPI void
elm_fileselector_filters_clear(Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   elm_interface_fileselector_filters_clear(obj);
}

EOLIAN static void
_elm_fileselector_elm_interface_fileselector_filters_clear(Eo *obj, Elm_Fileselector_Data *sd)
{
   Elm_Fileselector_Filter *filter;

   EINA_LIST_FREE(sd->filter_list, filter)
     {
        eina_stringshare_del(filter->filter_name);

        if (filter->filter_type == ELM_FILESELECTOR_MIME_FILTER)
          {
             free(filter->filter.mime_types[0]);
             free(filter->filter.mime_types);
          }
        else
          free(filter->filter.custom);

        free(filter);
     }

   ELM_SAFE_FREE(sd->filter_hoversel, evas_object_del);

   if (sd->model)
     {
        _schedule_populate(obj, sd, sd->model, NULL);
     }
}

EAPI void
elm_fileselector_hidden_visible_set(Evas_Object *obj, Eina_Bool visible)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   elm_interface_fileselector_hidden_visible_set(obj, visible);
}

EOLIAN static void
_elm_fileselector_elm_interface_fileselector_hidden_visible_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Data *sd, Eina_Bool visible)
{
   visible = !!visible;
   if (sd->hidden_visible == visible) return;
   sd->hidden_visible = visible;

   _clear_selections(sd, NULL);

   if (sd->model)
     {
        _schedule_populate(obj, sd, sd->model, NULL);
     }
}

EAPI Eina_Bool
elm_fileselector_hidden_visible_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, EINA_FALSE);
   return elm_interface_fileselector_hidden_visible_get((Eo *) obj);
}

EOLIAN static Eina_Bool
_elm_fileselector_elm_interface_fileselector_hidden_visible_get(Eo *obj EINA_UNUSED, Elm_Fileselector_Data *sd)
{
   return sd->hidden_visible;
}

EAPI void
elm_fileselector_thumbnail_size_set(Evas_Object *obj,
                                    Evas_Coord w,
                                    Evas_Coord h)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   elm_interface_fileselector_thumbnail_size_set(obj, w, h);
}

EOLIAN static void
_elm_fileselector_elm_interface_fileselector_thumbnail_size_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Data *sd, Evas_Coord w, Evas_Coord h)
{
   if (sd->thumbnail_size.w == w && sd->thumbnail_size.h == h) return;

   if (!w || !h)
     w = h = ITEM_SIZE_DEFAULT;

   sd->thumbnail_size.w = w;
   sd->thumbnail_size.h = h;

   if (sd->mode == ELM_FILESELECTOR_GRID)
     elm_gengrid_item_size_set(sd->files_view, w, h);

   if (sd->model)
     {
        _schedule_populate(obj, sd, sd->model, NULL);
     }
}

EAPI void
elm_fileselector_thumbnail_size_get(const Evas_Object *obj,
                                    Evas_Coord *w,
                                    Evas_Coord *h)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   elm_interface_fileselector_thumbnail_size_get((Eo *) obj, w, h);
}

EOLIAN static void
_elm_fileselector_elm_interface_fileselector_thumbnail_size_get(Eo *obj EINA_UNUSED, Elm_Fileselector_Data *sd, Evas_Coord *w, Evas_Coord *h)
{
   if (w) *w = sd->thumbnail_size.w;
   if (h) *h = sd->thumbnail_size.h;
}

EAPI void
elm_fileselector_sort_method_set(Evas_Object *obj, Elm_Fileselector_Sort sort)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj);
   elm_interface_fileselector_sort_method_set(obj, sort);
}

EOLIAN static void
_elm_fileselector_elm_interface_fileselector_sort_method_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Data *sd, Elm_Fileselector_Sort sort)
{
   if (sd->sort_type == sort) return;
   sd->sort_type = sort;

   switch (sd->sort_type)
     {
      case ELM_FILESELECTOR_SORT_BY_FILENAME_ASC:
         sd->sort_method = _filename_cmp;
         break;
      case ELM_FILESELECTOR_SORT_BY_FILENAME_DESC:
         sd->sort_method = _filename_cmp_rev;
         break;
      case ELM_FILESELECTOR_SORT_BY_TYPE_ASC:
         sd->sort_method = _type_cmp;
         break;
      case ELM_FILESELECTOR_SORT_BY_TYPE_DESC:
         sd->sort_method = _type_cmp_rev;
         break;
      case ELM_FILESELECTOR_SORT_BY_SIZE_ASC:
         sd->sort_method = _size_cmp;
         break;
      case ELM_FILESELECTOR_SORT_BY_SIZE_DESC:
         sd->sort_method = _size_cmp_rev;
         break;
      case ELM_FILESELECTOR_SORT_BY_MODIFIED_ASC:
         sd->sort_method = _modified_cmp;
         break;
      case ELM_FILESELECTOR_SORT_BY_MODIFIED_DESC:
         sd->sort_method = _modified_cmp_rev;
         break;
      case ELM_FILESELECTOR_SORT_LAST:
      default:
         sd->sort_method = _filename_cmp;
     }

   if (sd->model)
     {
        _schedule_populate(obj, sd, sd->model, NULL);
     }
}

EAPI Elm_Fileselector_Sort
elm_fileselector_sort_method_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_INTERFACE_CHECK(obj, ELM_FILESELECTOR_SORT_LAST);
   return elm_interface_fileselector_sort_method_get((Eo *) obj);
}

EOLIAN static Elm_Fileselector_Sort
_elm_fileselector_elm_interface_fileselector_sort_method_get(Eo *obj EINA_UNUSED, Elm_Fileselector_Data *sd)
{
   return sd->sort_type;
}

EOLIAN static Eina_Bool
_elm_fileselector_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Elm_Fileselector_Data *sd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_fileselector_elm_widget_focus_next(Eo *obj EINA_UNUSED, Elm_Fileselector_Data *sd, Elm_Focus_Direction dir, Evas_Object **next, Elm_Object_Item **next_item)
{
   Eina_List *items = NULL;

   if (sd->up_button) items = eina_list_append(items, sd->up_button);
   if (sd->home_button) items = eina_list_append(items, sd->home_button);
   if (sd->files_view) items = eina_list_append(items, sd->files_view);
   if (sd->path_entry) items = eina_list_append(items, sd->path_entry);
   if (sd->name_entry) items = eina_list_append(items, sd->name_entry);
   if (sd->filter_hoversel) items = eina_list_append(items, sd->filter_hoversel);
   if (sd->cancel_button) items = eina_list_append(items, sd->cancel_button);
   if (sd->ok_button) items = eina_list_append(items, sd->ok_button);

   if (_elm_config->access_mode)
     return elm_widget_focus_list_next_get(obj, items, eina_list_data_get, dir, next, next_item);

   if (!elm_widget_focus_list_next_get(obj, items, eina_list_data_get, dir, next, next_item))
     *next = (Evas_Object *)obj;

   eina_list_free(items);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_fileselector_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, Elm_Fileselector_Data *sd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_fileselector_elm_widget_focus_register(Eo *obj, Elm_Fileselector_Data *pd, Efl_Ui_Focus_Manager *manager, Efl_Ui_Focus_Object *logical, Eina_Bool *logical_flag)
{
   Eina_Bool ret = elm_obj_widget_focus_register(efl_super(obj, MY_CLASS), manager, logical, logical_flag);

   _focus_chain_update(obj, pd);

   return ret;
}


EOLIAN static Eina_Bool
_elm_fileselector_elm_widget_focus_direction(Eo *obj EINA_UNUSED, Elm_Fileselector_Data *sd, const Evas_Object *base, double degree, Evas_Object **direction, Elm_Object_Item **direction_item, double *weight)
{
   Eina_List *items = NULL;

   if (sd->up_button) items = eina_list_append(items, sd->up_button);
   if (sd->home_button) items = eina_list_append(items, sd->home_button);
   if (sd->files_view) items = eina_list_append(items, sd->files_view);
   if (sd->path_entry) items = eina_list_append(items, sd->path_entry);
   if (sd->name_entry) items = eina_list_append(items, sd->name_entry);
   if (sd->filter_hoversel) items = eina_list_append(items, sd->filter_hoversel);
   if (sd->cancel_button) items = eina_list_append(items, sd->cancel_button);
   if (sd->ok_button) items = eina_list_append(items, sd->ok_button);

   elm_widget_focus_list_direction_get
     (obj, base, items, eina_list_data_get, degree, direction, direction_item, weight);

   eina_list_free(items);

   return EINA_TRUE;
}

static Eina_Bool
_elm_fileselector_text_set(Eo *obj EINA_UNUSED, Elm_Fileselector_Data *sd, const char *part, const char *label)
{
   if (!part) return EINA_FALSE;

   if (sd->ok_button && !strcmp(part, "ok"))
     {
        elm_object_text_set(sd->ok_button, label);
        return EINA_TRUE;
     }
   else if (sd->cancel_button && !strcmp(part, "cancel"))
     {
        elm_object_text_set(sd->cancel_button, label);
        return EINA_TRUE;
     }
   else
     {
        Eina_Bool int_ret = EINA_TRUE;
        efl_text_set(efl_part(efl_super(obj, MY_CLASS), part), label);
        return int_ret;
     }

   return EINA_FALSE;
}

EOLIAN static void
_elm_fileselector_class_constructor(Efl_Class *klass)
{
   unsigned int i;

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);

   ELM_FILESELECTOR_ERROR_UNKNOWN = eina_error_msg_static_register(ELM_FILESELECTOR_ERROR_UNKNOWN_STR);
   ELM_FILESELECTOR_ERROR_INVALID_MODEL = eina_error_msg_static_register(ELM_FILESELECTOR_ERROR_INVALID_MODEL_STR);

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

}

EOLIAN static void
_elm_fileselector_class_destructor(Efl_Class *klass EINA_UNUSED)
{
   unsigned int i;

   for (i = 0; i < ELM_FILE_LAST; ++i)
     {
        elm_genlist_item_class_free(list_itc[i]);
        elm_gengrid_item_class_free(grid_itc[i]);
     }
}

EOLIAN const Elm_Atspi_Action *
_elm_fileselector_elm_interface_atspi_widget_action_elm_actions_get(Eo *obj EINA_UNUSED, Elm_Fileselector_Data *pd EINA_UNUSED)
{
   static Elm_Atspi_Action atspi_actions[] = {
          { "select", "select", NULL, _key_action_select },
          { "escape", "escape", NULL, _key_action_escape},
          { "backspace", "backspace", NULL, _key_action_backspace},
          { NULL, NULL, NULL, NULL}
   };
   return &atspi_actions[0];
}


/* Efl.Part begin */

ELM_PART_OVERRIDE(elm_fileselector, ELM_FILESELECTOR, ELM_LAYOUT, Elm_Fileselector_Data, Elm_Part_Data)
ELM_PART_OVERRIDE_TEXT_SET(elm_fileselector, ELM_FILESELECTOR, ELM_LAYOUT, Elm_Fileselector_Data, Elm_Part_Data)
#include "elm_fileselector_internal_part.eo.c"

/* Efl.Part end */
/* Internal EO APIs and hidden overrides */

#define ELM_FILESELECTOR_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_fileselector), \
   EFL_OBJECT_OP_FUNC(efl_event_callback_legacy_call, _elm_fileselector_efl_object_event_callback_legacy_call)

#include "elm_fileselector.eo.c"
