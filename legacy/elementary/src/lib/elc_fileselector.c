/*
 * TODO
 *  userdefined icon/label cb
 *  show/hide/add buttons ???
 *  need a background ???
 *  show/Hide hidden files
 *  double click to choose a file
 *  multiselection
 *  write docs
 */

#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *vbox, *entry, *entry2, *list, *scr2;
   const char *path;
   const char *selection;
   Eina_Bool only_folder;
   Eina_Bool expand;

   struct 
   {
    Evas_Object *bx;
    Evas_Object *ok;
    Evas_Object *cancel;
   } buttons;
};

Elm_Genlist_Item_Class itc;

static const char *widtype = NULL;
static void _populate(Evas_Object *obj, const char *path, Elm_Genlist_Item *parent);
static void _do_anchors(Evas_Object *obj, const char *path);

/***  ELEMENTARY WIDGET  ***/
static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->path) eina_stringshare_del(wd->path);
   if (wd->selection) eina_stringshare_del(wd->selection);
   free(wd);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;

   evas_object_size_hint_min_get(wd->vbox, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);

//   printf("***** SIZING EVAL [min %d %d] *************\n", minw, minh);
}

/***  GENLIST "MODEL"  ***/
static char*
_itc_label_get(const void *data, Evas_Object *obj, const char *source)
{
   //~ printf("LABEL_GET: %s\n", (char*) data);
   return strdup(ecore_file_file_get(data)); // NOTE this will be free() by the caller
}

static Evas_Object*
_itc_icon_get(const void *data, Evas_Object *obj, const char *source)
{
   Evas_Object *ic;

   //~ printf("ICON GET for %s (source: %s)\n", (char*)data, source);

   if (!strcmp(source, "elm.swallow.icon"))
     {
	ic = elm_icon_add(obj);
	if (ecore_file_is_dir((char*)data))
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
_itc_state_get(const void *data, Evas_Object *obj, const char *source)
{
   return EINA_FALSE;
}

static void
_itc_del(const void *data, Evas_Object *obj)
{
   //~ printf("DEL DATA [%s]\n", (char*)data);
   eina_stringshare_del(data);
}

static void
_expand_done(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   const char *path = elm_genlist_item_data_get(it);

//   printf("EXPAND %s\n", path);
   _populate(data, path, it);
}

static void
_contract_done(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   const char *path = elm_genlist_item_data_get(it);

//   printf("CONTRACT %s\n", path);
   elm_genlist_item_subitems_clear(it);
}

static void
_expand_req(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_expanded_set(it, 1);
}

static void
_contract_req(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_expanded_set(it, 0);
}

/***  PRIVATES  ***/
static void
_sel(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *fs = data;
   Widget_Data *wd = elm_widget_data_get(fs);
   const char *path = elm_genlist_item_data_get(event_info);
   const char *p;

   if (ecore_file_is_dir(path))
     {
//	printf("SELECTED DIR: %s\n", path);
	if (wd->expand)
	  {
	     _do_anchors(fs, path);
	     if (wd->entry2) elm_entry_entry_set(wd->entry2, "");
	  }
	else
	  {
	     // keep a ref to path 'couse it will be destroyed by _populate
	     p = eina_stringshare_add(path);
	     _populate(fs, p, NULL);
	     eina_stringshare_del(p);
	  }
	return;
     }
   else
     {
//	printf("SELECTED FILE: %s\n", path);
	if (wd->entry2)
	  elm_entry_entry_set(wd->entry2, ecore_file_file_get(path));
     }

   evas_object_smart_callback_call(fs, "selected", (void*)path);
}

static void
_up(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *fs = data;
   Widget_Data *wd = elm_widget_data_get(fs);
   char *parent = ecore_file_dir_get(wd->path);

   _populate(fs, parent, NULL);
   free(parent);
}

static void
_home(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *fs = data;
   _populate(fs, getenv("HOME"), NULL);
}

static void
_ok(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *fs = data;
   evas_object_smart_callback_call(fs, "done",
				   (void*)elm_fileselector_selected_get(fs));
}

static void
_canc(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *fs = data;
   evas_object_smart_callback_call(fs, "done", NULL);
}

static void
_anchor_clicked(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *fs = data;
   Widget_Data *wd = elm_widget_data_get(fs);
   Elm_Entry_Anchor_Info *info = event_info;
   const char *p;

   //~ printf("ANCHOR CLICKED %s\n", info->name);

   // keep a ref to path 'couse it will be destroyed by _populate
   p = eina_stringshare_add(info->name);
   _populate(fs, p, NULL);

   evas_object_smart_callback_call(data, "selected", (void*)p);
   eina_stringshare_del(p);
}

static void
_do_anchors(Evas_Object *obj, const char *path)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   char **tok, buf[PATH_MAX*3];
   int i, j;

   //~ printf("ANCHORIZE...\n");

   buf[0] = '\0';
   tok = eina_str_split(path, "/", 0);
   for (i = 0; tok[i]; i++)
     {
	if (strlen(tok[i]) < 1) continue;
	//~ printf("TOK: %s\n", tok[i]);
	eina_strlcat(buf, "/<a href=", sizeof(buf));
	for (j = 0; j <= i; j++)
	  {
	     if (strlen(tok[j]) < 1) continue;
	     //~ printf("REV: %s\n",tok[j]);
	     eina_strlcat(buf, "/", sizeof(buf));
	     eina_strlcat(buf, tok[j], sizeof(buf));
	  }
	eina_strlcat(buf, ">", sizeof(buf));
	eina_strlcat(buf, tok[i], sizeof(buf));
	eina_strlcat(buf, "</a>", sizeof(buf));
     }
   free(tok[0]);
   free(tok);

   //~ printf("ANCHOR: %s\n", buf);
   elm_entry_entry_set(wd->entry, buf);
}

static void
_populate(Evas_Object *obj, const char *path, Elm_Genlist_Item *parent)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   DIR *dir;
   struct dirent *dp;
   char buf[PATH_MAX];
   char *real;
   Eina_List *files = NULL, *dirs = NULL, *l;

   if ((!wd) || (!ecore_file_is_dir(path))) return;

   dir = opendir(path);
   if (!dir) return;

   evas_object_smart_callback_call(obj, "directory,open", (void*)path);

   if (!parent)
     {
	elm_genlist_clear(wd->list);
	if (wd->path) eina_stringshare_del(wd->path);
	wd->path = eina_stringshare_add(path);
	_do_anchors(obj, path);
     }

   if (wd->entry2)
     elm_entry_entry_set(wd->entry2, "");

   while ((dp = readdir(dir)) != NULL)
     {
	if (dp->d_name[0] == '.') continue; // TODO make this configurable

	snprintf(buf, sizeof(buf), "%s/%s", path, dp->d_name);
	real = ecore_file_realpath(buf); //TODO this will resolv symlinks...I dont like it
	if (ecore_file_is_dir(real))
	  dirs = eina_list_append(dirs, real);
	else if(!wd->only_folder)
	  files = eina_list_append(files, real);
     }
   closedir(dir);

   files = eina_list_sort(files, eina_list_count(files), EINA_COMPARE_CB(strcoll));
   dirs = eina_list_sort(dirs, eina_list_count(dirs), EINA_COMPARE_CB(strcoll));

   EINA_LIST_FOREACH(dirs, l, real)
     {
	//~ printf("DIR: %s\n", real);
	elm_genlist_item_append(wd->list, &itc,
				eina_stringshare_add(real), /* item data */
				parent,
				wd->expand ? ELM_GENLIST_ITEM_SUBITEMS :
					     ELM_GENLIST_ITEM_NONE,
				NULL, NULL);
	free(real);
     }
   eina_list_free(dirs);

   EINA_LIST_FOREACH(files, l, real)
     {
	//~ printf("FILE: %s [%p]\n", real, wd->list);
	elm_genlist_item_append(wd->list, &itc,
				eina_stringshare_add(real), /* item data */
				parent, ELM_GENLIST_ITEM_NONE,
				NULL, NULL);
	free(real);
     }
   eina_list_free(files);
}

/***  API  ***/
EAPI Evas_Object *
elm_fileselector_add(Evas_Object *parent)
{
   Evas_Object *obj, *ic, *bt, *box;
   Widget_Data *wd;

   // Elementary Widget
   wd = ELM_NEW(Widget_Data);
   wd->expand = EINA_FALSE;
   obj = elm_widget_add(evas_object_evas_get(parent));
   ELM_SET_WIDTYPE(widtype, "fileselector");
   elm_widget_type_set(obj, "fileselector");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);

   // TODO Do we need a bg object? a frame?
   // vbox
   wd->vbox = elm_box_add(parent);
   evas_object_size_hint_weight_set(wd->vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_widget_resize_object_set(obj, wd->vbox);
   evas_object_show(wd->vbox);

   // buttons box
   box = elm_box_add(parent);
   elm_box_horizontal_set(box, 1);
   elm_widget_sub_object_add(obj, box);
   elm_box_pack_end(wd->vbox, box);
   evas_object_size_hint_align_set(box, 0.0, 0.0);
   evas_object_show(box);

   // up btn
   ic = elm_icon_add(parent);
   elm_icon_standard_set(ic, "arrow_up");
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   bt = elm_button_add(parent);
   elm_button_icon_set(bt, ic);
   elm_button_label_set(bt, "Up");
   evas_object_size_hint_align_set(bt, 0.0, 0.0);
   elm_widget_sub_object_add(obj, bt);
   elm_box_pack_end(box, bt);
   evas_object_smart_callback_add(bt, "clicked", _up, obj);
   evas_object_show(bt);

   // home btn
   ic = elm_icon_add(parent);
   elm_icon_standard_set(ic, "home");
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   bt = elm_button_add(parent);
   elm_button_icon_set(bt, ic);
   elm_button_label_set(bt, "Home");
   evas_object_size_hint_align_set(bt, 0.0, 0.0);
   elm_widget_sub_object_add(obj, bt);
   elm_box_pack_end(box, bt);
   evas_object_smart_callback_add(bt, "clicked", _home, obj);
   evas_object_show(bt);

   // genlist
   itc.item_style = "default";
   itc.func.label_get = _itc_label_get;
   itc.func.icon_get = _itc_icon_get;
   itc.func.state_get = _itc_state_get;
   itc.func.del = _itc_del;

   wd->list = elm_genlist_add(parent);
   evas_object_size_hint_align_set(wd->list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(wd->list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_min_set(wd->list, 100, 100);
   elm_widget_sub_object_add(obj, wd->list);
   elm_box_pack_end(wd->vbox, wd->list);
   evas_object_show(wd->list);

   evas_object_smart_callback_add(wd->list, "selected", _sel, obj);
   evas_object_smart_callback_add(wd->list, "expand,request", _expand_req, obj);
   evas_object_smart_callback_add(wd->list, "contract,request", _contract_req, obj);
   evas_object_smart_callback_add(wd->list, "expanded", _expand_done, obj);
   evas_object_smart_callback_add(wd->list, "contracted", _contract_done, obj);

   // path entry
   wd->entry = elm_entry_add(parent);
   elm_widget_sub_object_add(obj, wd->entry);
   elm_entry_editable_set(wd->entry, 0);
   elm_entry_single_line_set(wd->entry, EINA_FALSE);
   elm_entry_line_char_wrap_set(wd->entry, EINA_TRUE);
   evas_object_size_hint_weight_set(wd->entry, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(wd->entry, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(wd->vbox, wd->entry);
   evas_object_show(wd->entry);
   evas_object_smart_callback_add(wd->entry, "anchor,clicked", _anchor_clicked, obj);

   // name entry scroller
   wd->scr2 = elm_scroller_add(parent);
   elm_scroller_content_min_limit(wd->scr2, 0, 1);
   elm_scroller_policy_set(wd->scr2, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   evas_object_size_hint_weight_set(wd->scr2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(wd->scr2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(wd->vbox, wd->scr2);
   evas_object_show(wd->scr2);

   elm_fileselector_buttons_ok_cancel_set(obj, 1);
   
   // Is this the right way to show sub-objs ?? or use the show/hide cbs ??
   //~ evas_object_event_callback_add(obj, EVAS_CALLBACK_SHOW, _show, obj);
   //~ evas_object_event_callback_add(obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				       //~ _changed_size_hints, obj);

   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_fileselector_is_save_set(Evas_Object *obj, Eina_Bool is_save)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (is_save)
     {
	if (wd->entry2) return;
	wd->entry2 = elm_entry_add(elm_widget_parent_get(obj));
	elm_widget_sub_object_add(obj, wd->entry2);
	elm_entry_editable_set(wd->entry2, 1);
	elm_entry_single_line_set(wd->entry2, EINA_TRUE);
	evas_object_size_hint_weight_set(wd->entry2, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(wd->entry2, EVAS_HINT_FILL, 0.0);
	elm_scroller_content_set(wd->scr2, wd->entry2);
	evas_object_show(wd->entry2);
     }
   else
     {
	evas_object_del(wd->entry2);
	wd->entry2 = NULL;
     }
}

EAPI Eina_Bool
elm_fileselector_is_save_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->entry2 ? EINA_TRUE : EINA_FALSE;
}

EAPI void
elm_fileselector_folder_only_set(Evas_Object *obj, Eina_Bool only)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if(wd->only_folder == only) return ;

   wd->only_folder = only;
}

EAPI Eina_Bool
elm_fileselector_only_folder_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->only_folder;
}

EAPI void
elm_fileselector_buttons_ok_cancel_set(Evas_Object *obj, Eina_Bool only)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *box, *bt;

   if (only)
     {
	if (wd->buttons.bx) return;
   // buttons box
   box = elm_box_add(obj);
   wd->buttons.bx = box;
   elm_box_horizontal_set(box, 1);
   elm_widget_sub_object_add(obj, box);
   elm_box_pack_end(wd->vbox, box);
   evas_object_show(box);

   // cancel btn
   bt = elm_button_add(obj);
   wd->buttons.cancel = bt;
   elm_button_label_set(bt, "Cancel");
   elm_widget_sub_object_add(obj, bt);
   elm_box_pack_end(box, bt);
   evas_object_smart_callback_add(bt, "clicked", _canc, obj);
   evas_object_show(bt);

   // ok btn
   bt = elm_button_add(obj);
   wd->buttons.ok = bt;
   elm_button_label_set(bt, "OK");
   elm_widget_sub_object_add(obj, bt);
   elm_box_pack_end(box, bt);
   evas_object_smart_callback_add(bt, "clicked", _ok, obj);
   evas_object_show(bt);
     }
   else
     {
   evas_object_del(wd->buttons.bx);
   evas_object_del(wd->buttons.ok);
   evas_object_del(wd->buttons.cancel);
	wd->buttons.bx = NULL;
     }
}

EAPI Eina_Bool
elm_fileselector_ok_cancel_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->buttons.bx ? EINA_TRUE : EINA_FALSE;
}

EAPI void
elm_fileselector_expandable_set(Evas_Object *obj, Eina_Bool expand)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   wd->expand = expand;
}

EAPI void
elm_fileselector_path_set(Evas_Object *obj, const char *path)
{
   _populate(obj, path, NULL);
}

EAPI const char *
elm_fileselector_path_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->path;
}

EAPI const char*
elm_fileselector_selected_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Genlist_Item *it;

   if (wd->entry2)
     {
	const char *name;
	char buf[PATH_MAX];

	name = elm_entry_entry_get(wd->entry2);
	//TODO remove <br>
	snprintf(buf, sizeof(buf), "%s/%s", wd->path, name);
	if (wd->selection) eina_stringshare_del(wd->selection);
	wd->selection = eina_stringshare_add(buf);
	return wd->selection;
     }

   it = elm_genlist_selected_item_get(wd->list);
   if (it) return elm_genlist_item_data_get(it);

   return wd->path;
}
