#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EO_BASE_BETA
#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_INTERFACE_ATSPI_WIDGET_ACTION_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_combobox.h"

#define MY_CLASS ELM_COMBOBOX_CLASS

#define MY_CLASS_NAME "Elm_Combobox"
#define MY_CLASS_NAME_LEGACY "elm_combobox"

static const char SIG_DISMISSED[] = "dismissed";
static const char SIG_EXPANDED[] = "expanded";
static const char SIG_ITEM_SELECTED[] = "item,selected";
static const char SIG_ITEM_PRESSED[] = "item,pressed";
static const char SIG_FILTER_DONE[] = "filter,done";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_DISMISSED, ""},
   {SIG_EXPANDED, ""},
   {SIG_ITEM_SELECTED, ""},
   {SIG_ITEM_PRESSED, ""},
   {SIG_FILTER_DONE, ""},
   {"clicked", ""}, /**< handled by parent button class */
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {NULL, NULL}
};

static Eina_Bool _key_action_move(Evas_Object *obj, const char *params);
static Eina_Bool _key_action_activate(Evas_Object *obj, const char *params);

static const Elm_Action key_actions[] = {
   {"activate", _key_action_activate},
   {"move", _key_action_move},
   {NULL, NULL}
};

EOLIAN static Eina_Bool
_elm_combobox_elm_widget_translate(Eo *obj EINA_UNUSED, Elm_Combobox_Data *sd)
{
   eo_do_super(obj, MY_CLASS, elm_obj_widget_translate());
   eo_do(sd->genlist, elm_obj_widget_translate());
   eo_do(sd->entry, elm_obj_widget_translate());

   if (sd->hover)
     eo_do(sd->hover, elm_obj_widget_translate());

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_combobox_elm_widget_theme_apply(Eo *obj, Elm_Combobox_Data *sd)
{
   const char *style;
   Eina_Bool int_ret = EINA_FALSE;
   Eina_Bool mirrored;
   char buf[128];

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   style = elm_widget_style_get(obj);

   snprintf(buf, sizeof(buf), "combobox_vertical/%s", style);

   /* combobox's style has no extra bit for orientation but could have... */
   eina_stringshare_replace(&(wd->style), buf);

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_theme_apply());
   if (!int_ret) return EINA_FALSE;

   mirrored = elm_widget_mirrored_get(obj);

   if (sd->hover)
     elm_widget_mirrored_set(sd->hover, mirrored);

   elm_widget_mirrored_set(sd->genlist, mirrored);
   elm_widget_mirrored_set(sd->entry, mirrored);

   elm_combobox_hover_end(obj);

   return EINA_TRUE;
}

static void
_on_hover_clicked(void *data,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED)
{
   elm_combobox_hover_end(data);
}

static void
count_items_genlist(void *data)
{
   ELM_COMBOBOX_DATA_GET(data, sd);
   Eina_Iterator *filter_iter;
   int count = 0;
   Elm_Object_Item *item;

   filter_iter = elm_genlist_filter_iterator_new(sd->genlist);
   if (!filter_iter) return;
   EINA_ITERATOR_FOREACH(filter_iter, item)
     if (item) count++;
   sd->count = count;
   eina_iterator_free(filter_iter);
}

static void
_table_resize(void *data)
{
   ELM_COMBOBOX_DATA_GET(data, sd);
   if (sd->count > 0)
     {
        int hover_parent_w, hover_parent_h, obj_h, obj_w, obj_y, win_y_offset;
        int current_height, h;
        const char *best_location;
        sd->item = elm_genlist_first_item_get(sd->genlist);
        elm_genlist_item_selected_set(sd->item, EINA_TRUE);
        //FIXME:- the height of item is zero, sometimes.
        evas_object_geometry_get(elm_object_item_track(sd->item), NULL, NULL,
                                 NULL, &h);
        if (h) sd->item_height = h;
        evas_object_geometry_get(sd->entry, NULL, NULL, &obj_w, NULL);
        evas_object_geometry_get(data, NULL, &obj_y, NULL, &obj_h);
        evas_object_geometry_get(sd->hover_parent, NULL, NULL, &hover_parent_w,
                                 &hover_parent_h);
        current_height = sd->item_height * sd->count;
        best_location = elm_hover_best_content_location_get(sd->hover,
                                                       ELM_HOVER_AXIS_VERTICAL);
        if (best_location && !strcmp(best_location , "bottom"))
          win_y_offset = hover_parent_h - obj_y - obj_h;
        else win_y_offset = obj_y;

        if (current_height < win_y_offset)
          evas_object_size_hint_min_set(sd->spacer, obj_w * elm_config_scale_get(),
                                        current_height + (2 * elm_config_scale_get()));
        else evas_object_size_hint_min_set(sd->spacer, obj_w * elm_config_scale_get(),
                                           win_y_offset * elm_config_scale_get());
     }
}

static void
_activate(Evas_Object *obj)
{
   ELM_COMBOBOX_DATA_GET(obj, sd);
   if (elm_widget_disabled_get(obj)) return;
   sd->expanded = EINA_TRUE;
   eo_do(obj, eo_event_callback_call(ELM_COMBOBOX_EVENT_EXPANDED, NULL));
   _table_resize(obj);
   elm_object_part_content_set(sd->hover, elm_hover_best_content_location_get
                               (sd->hover, ELM_HOVER_AXIS_VERTICAL), sd->tbl);
   evas_object_show(sd->genlist);
   evas_object_show(sd->hover);
}

static void
_on_item_selected(void *data , Evas_Object *obj EINA_UNUSED, void *event)
{
   ELM_COMBOBOX_DATA_GET(data, sd);
   elm_object_focus_set(sd->entry, EINA_TRUE);
   eo_do(data, eo_event_callback_call(ELM_COMBOBOX_EVENT_ITEM_SELECTED, event));
}

static void
_on_item_pressed(void *data , Evas_Object *obj EINA_UNUSED, void *event)
{
   eo_do(data, eo_event_callback_call(ELM_COMBOBOX_EVENT_ITEM_PRESSED, event));
}

static Eina_Bool
_gl_filter_finished_cb(void *data, Eo *obj EINA_UNUSED,
                       const Eo_Event_Description *desc EINA_UNUSED, void *event)
{
   ELM_COMBOBOX_DATA_GET(data, sd);

   count_items_genlist(data);

   if (sd->first_filter)
     {
        sd->first_filter = EINA_FALSE;
        elm_combobox_hover_end(data);
        return EINA_TRUE;
     }

   eo_do(data, eo_event_callback_call(ELM_COMBOBOX_EVENT_FILTER_DONE, event));

   if (sd->count > 0)
     {
        if (sd->expanded == EINA_TRUE)
          elm_combobox_hover_end(data);
        _activate(data);
     }
   else elm_combobox_hover_end(data);
   return EINA_TRUE;
}

static Eina_Bool
_on_aborted(void *data, Eo *obj EINA_UNUSED,
            const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   ELM_COMBOBOX_DATA_GET(data, sd);
   if (sd->expanded == EINA_TRUE) elm_combobox_hover_end(data);
   return EINA_TRUE;
}

static Eina_Bool
_on_changed(void *data, Eo *obj EINA_UNUSED,
            const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   eo_do(data, eo_event_callback_call(ELM_ENTRY_EVENT_CHANGED, NULL));
   return EINA_TRUE;
}

static void
_on_clicked(void *data EINA_UNUSED,
            Evas_Object *obj,
            void *event_info EINA_UNUSED)
{
   elm_combobox_hover_begin(obj);
}

EOLIAN static void
_elm_combobox_evas_object_smart_add(Eo *obj, Elm_Combobox_Data *sd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   elm_widget_mirrored_automatic_set(obj, EINA_FALSE);

   evas_object_smart_callback_add(obj, "clicked", _on_clicked, obj);

   //What are you doing here?
   eo_do(obj, elm_obj_widget_theme_apply());
}

EOLIAN static void
_elm_combobox_evas_object_smart_del(Eo *obj, Elm_Combobox_Data *sd)
{
   sd->hover_parent = NULL;
   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EOLIAN static void
_elm_combobox_evas_object_smart_show(Eo *obj, Elm_Combobox_Data *sd)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_show());
   if (sd->expanded) evas_object_show(sd->hover);
}

EOLIAN static void
_elm_combobox_evas_object_smart_hide(Eo *obj, Elm_Combobox_Data *sd)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_hide());
   if (sd->hover) evas_object_hide(sd->hover);
}

EOLIAN static Eina_Bool
_elm_combobox_elm_button_admits_autorepeat_get(Eo *obj EINA_UNUSED,
                                               Elm_Combobox_Data *sd EINA_UNUSED)
{
   return EINA_FALSE;
}

EAPI Evas_Object *
elm_combobox_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   return obj;
}

EOLIAN static Eo *
_elm_combobox_eo_base_constructor(Eo *obj, Elm_Combobox_Data *sd)
{
   Evas_Object *gl;
   Evas_Object *entry;
   char buf[128];

   eo_do_super(obj, MY_CLASS, eo_constructor());

   sd->first_filter = EINA_TRUE;

   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks),
         elm_interface_atspi_accessible_role_set(ELM_ATSPI_ROLE_GLASS_PANE));

   //hover-parent
   sd->hover_parent = elm_object_parent_widget_get(obj);

   //hover
   sd->hover = eo_add(ELM_HOVER_CLASS, sd->hover_parent);
   elm_widget_mirrored_automatic_set(sd->hover, EINA_FALSE);
   elm_hover_target_set(sd->hover, obj);

   //table
   sd->tbl = elm_table_add(obj);
   evas_object_size_hint_weight_set(sd->tbl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sd->tbl, EVAS_HINT_FILL, EVAS_HINT_FILL);

   //spacer
   sd->spacer = evas_object_rectangle_add(evas_object_evas_get(sd->hover_parent));
   evas_object_size_hint_weight_set(sd->spacer, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sd->spacer, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_color_set(sd->spacer, 0, 0, 0, 0);
   elm_table_pack(sd->tbl, sd->spacer, 0, 0, 1, 1);

   // This is the genlist object that will take over the genlist call
   sd->genlist = gl = eo_add(ELM_GENLIST_CLASS, obj);
   elm_genlist_filter_set(gl, NULL);
   elm_widget_mirrored_automatic_set(gl, EINA_FALSE);
   elm_widget_mirrored_set(gl, elm_widget_mirrored_get(obj));
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(gl, "selected", _on_item_selected, obj);
   evas_object_smart_callback_add(gl, "pressed", _on_item_pressed, obj);
   eo_do(gl, eo_event_callback_add(ELM_GENLIST_EVENT_FILTER_DONE,
         _gl_filter_finished_cb, obj));
   elm_genlist_homogeneous_set(gl, EINA_TRUE);
   elm_genlist_mode_set(gl, ELM_LIST_COMPRESS);
   elm_table_pack(sd->tbl, gl, 0, 0, 1, 1);

   // This is the entry object that will take over the entry call
   sd->entry = entry = eo_add(ELM_ENTRY_CLASS, obj);
   elm_widget_mirrored_automatic_set(entry, EINA_FALSE);
   elm_widget_mirrored_set(entry, elm_widget_mirrored_get(obj));
   elm_scroller_policy_set(entry, ELM_SCROLLER_POLICY_OFF,
                           ELM_SCROLLER_POLICY_OFF);
   elm_entry_scrollable_set(entry, EINA_TRUE);
   elm_entry_single_line_set(entry, EINA_TRUE);
   eo_do(entry, eo_event_callback_add(ELM_ENTRY_EVENT_CHANGED_USER,
         _on_changed, obj));
   eo_do(entry, eo_event_callback_add(ELM_ENTRY_EVENT_ABORTED,
         _on_aborted, obj));
   evas_object_show(entry);

   snprintf(buf, sizeof(buf), "combobox_vertical/%s", elm_widget_style_get(obj));
   elm_object_style_set(sd->hover, buf);
   evas_object_smart_callback_add(sd->hover, "clicked", _on_hover_clicked, obj);
   eo_do(obj,
         eo_composite_attach(gl),
         eo_composite_attach(entry));

   elm_object_part_content_set(obj, "elm.swallow.content", entry);
   return obj;
}

EOLIAN static void
_elm_combobox_hover_begin(Eo *obj, Elm_Combobox_Data *sd)
{
   if (!sd->hover) return;
   elm_object_focus_set(sd->entry, EINA_TRUE);
   _activate(obj);
}

EOLIAN static void
_elm_combobox_hover_end(Eo *obj, Elm_Combobox_Data *sd)
{
   if (!sd->hover) return;

   sd->expanded = EINA_FALSE;
   evas_object_hide(sd->hover);
   eo_do(obj, eo_event_callback_call(ELM_COMBOBOX_EVENT_DISMISSED, NULL));
}

EOLIAN static Eina_Bool
_elm_combobox_expanded_get(Eo *obj EINA_UNUSED, Elm_Combobox_Data *sd)
{
   return sd->expanded;
}

static Eina_Bool
_key_action_move(Evas_Object *obj, const char *params)
{
   ELM_COMBOBOX_DATA_GET(obj, sd);
   Elm_Object_Item *it = NULL;
   const char *dir = params;
   if (!sd->hover) return EINA_FALSE;

   if (!strcmp(dir, "return"))
     eo_do(obj, eo_event_callback_call(ELM_COMBOBOX_EVENT_CLICKED, NULL));
   else if (!strcmp(dir, "up"))
     {
        it = sd->item;
        it = elm_genlist_item_prev_get(it);
        if (!it) sd->item = elm_genlist_last_item_get(sd->genlist);
        else sd->item = it;
        elm_genlist_item_selected_set(sd->item, EINA_TRUE);
     }
   else if (!strcmp(dir, "down"))
     {
        it = sd->item;
        it = elm_genlist_item_next_get(it);
        if (!it) sd->item = elm_genlist_first_item_get(sd->genlist);
        else sd->item = it;
        elm_genlist_item_selected_set(sd->item, EINA_TRUE);
     }
   else return EINA_FALSE;
   return EINA_TRUE;
}

static Eina_Bool
_key_action_activate(Evas_Object *obj, const char *params EINA_UNUSED)
{
   elm_combobox_hover_begin(obj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_combobox_elm_widget_event(Eo *obj, Elm_Combobox_Data *sd,
                               Evas_Object *src EINA_UNUSED,
                               Evas_Callback_Type type, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   if (!sd || !sd->hover) return EINA_FALSE;
   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (!_elm_config_key_binding_call(obj, MY_CLASS_NAME, ev, key_actions))
     return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

static void
_elm_combobox_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

EOLIAN const Elm_Atspi_Action *
_elm_combobox_elm_interface_atspi_widget_action_elm_actions_get(Eo *obj EINA_UNUSED,
                                                                Elm_Combobox_Data *pd
                                                                EINA_UNUSED)
{
   static Elm_Atspi_Action atspi_actions[] = {
      {"activate", "activate", "return", _key_action_activate},
      {"move,up", "move", "up", _key_action_move},
      {"move,down", "move", "down", _key_action_move},
      {NULL, NULL, NULL, NULL}
   };
   return &atspi_actions[0];
}

EOLIAN void
_elm_combobox_elm_genlist_filter_set(Eo *obj, Elm_Combobox_Data *pd, void *key)
{
   pd->first_filter = EINA_FALSE;

   eo_do(pd->genlist, elm_obj_genlist_filter_set(key));
}

EOLIAN void
_elm_combobox_elm_widget_part_text_set(Eo *obj EINA_UNUSED, Elm_Combobox_Data *pd,
                                       const char * part, const char *label)
{
   elm_object_part_text_set(pd->entry, part, label);
}

EOLIAN const char *
_elm_combobox_elm_widget_part_text_get(Eo *obj EINA_UNUSED, Elm_Combobox_Data *pd,
                                       const char * part)
{
   return elm_object_part_text_get(pd->entry, part);
}

EOLIAN static void
_elm_combobox_evas_object_smart_resize(Eo *obj, Elm_Combobox_Data *pd,
                                       Evas_Coord w, Evas_Coord h)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_resize(w, h));
   if (pd->count > 0) _table_resize(obj);
}
#include "elm_combobox.eo.c"
