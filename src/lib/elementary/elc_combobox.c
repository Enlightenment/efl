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
   elm_obj_widget_translate(eo_super(obj, MY_CLASS));
   elm_obj_widget_translate(sd->genlist);
   elm_obj_widget_translate(sd->entry);

   if (sd->hover)
     elm_obj_widget_translate(sd->hover);

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

   style = eina_stringshare_add(elm_widget_style_get(obj));

   snprintf(buf, sizeof(buf), "combobox_vertical/%s", style);

   /* combobox's style has no extra bit for orientation but could have... */
   eina_stringshare_replace(&(wd->style), buf);

   int_ret = elm_obj_widget_theme_apply(eo_super(obj, MY_CLASS));
   if (!int_ret) return EINA_FALSE;

   eina_stringshare_replace(&(wd->style), style);

   mirrored = elm_widget_mirrored_get(obj);

   if (sd->hover)
     elm_widget_mirrored_set(sd->hover, mirrored);

   elm_widget_mirrored_set(sd->genlist, mirrored);
   elm_widget_mirrored_set(sd->entry, mirrored);
   eina_stringshare_del(style);

   elm_combobox_hover_end(obj);

   return EINA_TRUE;
}

static Eina_Bool
_on_hover_clicked(void *data, const Eo_Event *event)
{
   const char *dismissstr;

   dismissstr = elm_layout_data_get(event->obj, "dismiss");

   if (!dismissstr || strcmp(dismissstr, "on"))
     elm_combobox_hover_end(data); // for backward compatibility

   return EINA_TRUE;
}

static void
_hover_end_finished(void *data,
                    Evas_Object *obj EINA_UNUSED,
                    const char *emission EINA_UNUSED,
                    const char *source EINA_UNUSED)
{
   const char *dismissstr;
   ELM_COMBOBOX_DATA_GET(data, sd);
   dismissstr = elm_layout_data_get(sd->hover, "dismiss");
   if (dismissstr && !strcmp(dismissstr, "on"))
     {
        sd->expanded = EINA_FALSE;
        evas_object_hide(sd->hover);
        eo_event_callback_call(data, ELM_COMBOBOX_EVENT_DISMISSED, NULL);
     }
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
        sd->item = elm_genlist_first_item_get(sd->genlist);
        //FIXME:- the height of item is zero, sometimes.
        evas_object_geometry_get(elm_object_item_track(sd->item), NULL, NULL,
                                 NULL, &h);
        if (h) sd->item_height = h;
        evas_object_geometry_get(elm_object_part_content_get(data, "elm.swallow.content"),
                                 NULL, NULL, &obj_w, NULL);
        evas_object_geometry_get(data, NULL, &obj_y, NULL, &obj_h);
        evas_object_geometry_get(sd->hover_parent, NULL, NULL, &hover_parent_w,
                                 &hover_parent_h);
        current_height = sd->item_height * sd->count;
        sd->best_location = elm_hover_best_content_location_get(sd->hover,
                                                       ELM_HOVER_AXIS_VERTICAL);
        if (sd->best_location && !strcmp(sd->best_location , "bottom"))
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

   if (sd->expanded)
     {
        elm_combobox_hover_end(obj);
        return;
     }

   sd->expanded = EINA_TRUE;

   if (sd->count <= 0) return;

   _table_resize(obj);
   elm_object_part_content_set(sd->hover, sd->best_location, sd->tbl);
   evas_object_show(sd->genlist);
   elm_genlist_item_selected_set(sd->item, EINA_TRUE);
   evas_object_show(sd->hover);
   eo_event_callback_call(obj, ELM_COMBOBOX_EVENT_EXPANDED, NULL);
}

static void
_on_item_selected(void *data , Evas_Object *obj EINA_UNUSED, void *event)
{
   ELM_COMBOBOX_DATA_GET(data, sd);

   if (!sd->multiple_selection) elm_object_focus_set(sd->entry, EINA_TRUE);
   else
     {
        elm_genlist_item_bring_in(sd->item, ELM_GENLIST_ITEM_SCROLLTO_TOP);
        elm_object_focus_set(sd->mbe, EINA_TRUE);
     }
   eo_event_callback_call(data, ELM_COMBOBOX_EVENT_ITEM_SELECTED, event);
}

static void
_on_item_pressed(void *data , Evas_Object *obj EINA_UNUSED, void *event)
{
   eo_event_callback_call(data, ELM_COMBOBOX_EVENT_ITEM_PRESSED, event);
}

static Eina_Bool
_gl_filter_finished_cb(void *data, const Eo_Event *event)
{
   char buf[1024];
   ELM_COMBOBOX_DATA_GET(data, sd);

   count_items_genlist(data);

   if (sd->first_filter)
     {
        sd->first_filter = EINA_FALSE;
        return EINA_TRUE;
     }

   eo_event_callback_call(data, ELM_COMBOBOX_EVENT_FILTER_DONE, event->info);

   if (sd->count > 0)
     {
        if (!sd->expanded) _activate(data);
        else _table_resize(data);
        elm_genlist_item_selected_set(sd->item, EINA_TRUE);
     }
   else 
     {
        sd->expanded = EINA_FALSE;
        elm_layout_signal_emit(sd->hover, "elm,action,hide,no_animate", "elm");
        snprintf(buf, sizeof(buf), "elm,action,slot,%s,hide", sd->best_location);
        elm_layout_signal_emit(sd->hover, buf, "elm");
        edje_object_message_signal_process(elm_layout_edje_get(sd->hover));
     }

   return EINA_TRUE;
}

static Eina_Bool
_on_aborted(void *data, const Eo_Event *event EINA_UNUSED)
{
   ELM_COMBOBOX_DATA_GET(data, sd);
   if (sd->expanded) elm_combobox_hover_end(data);
   return EINA_TRUE;
}

static Eina_Bool
_on_changed(void *data, const Eo_Event *event EINA_UNUSED)
{
   eo_event_callback_call(data, ELM_ENTRY_EVENT_CHANGED, NULL);
   return EINA_TRUE;
}

static Eina_Bool
_on_clicked(void *data, const Eo_Event *event EINA_UNUSED)
{
   elm_combobox_hover_begin(data);
   return EINA_TRUE;
}

EOLIAN static void
_elm_combobox_evas_object_smart_add(Eo *obj, Elm_Combobox_Data *sd EINA_UNUSED)
{
   evas_obj_smart_add(eo_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   elm_widget_mirrored_automatic_set(obj, EINA_FALSE);

   eo_event_callback_add(obj, EVAS_CLICKABLE_INTERFACE_EVENT_CLICKED, _on_clicked, obj);

   //What are you doing here?
   elm_obj_widget_theme_apply(obj);
}

EOLIAN static void
_elm_combobox_evas_object_smart_del(Eo *obj, Elm_Combobox_Data *sd)
{
   sd->hover_parent = NULL;
   evas_obj_smart_del(eo_super(obj, MY_CLASS));
}

EOLIAN static void
_elm_combobox_evas_object_smart_show(Eo *obj, Elm_Combobox_Data *sd)
{
   evas_obj_smart_show(eo_super(obj, MY_CLASS));
   if (sd->expanded) evas_object_show(sd->hover);
}

EOLIAN static void
_elm_combobox_evas_object_smart_hide(Eo *obj, Elm_Combobox_Data *sd)
{
   evas_obj_smart_hide(eo_super(obj, MY_CLASS));
   if (sd->hover) evas_object_hide(sd->hover);
}

EOLIAN static Eina_Bool
_elm_combobox_elm_button_admits_autorepeat_get(Eo *obj EINA_UNUSED,
                                               Elm_Combobox_Data *sd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_combobox_multiple_selection_get(Eo *obj EINA_UNUSED, Elm_Combobox_Data *pd)
{
   return pd->multiple_selection;
}

static Eina_Bool
_mbe_clicked_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   //Unset the multibuttonentry to contracted mode of single line
   elm_multibuttonentry_expanded_set(event->obj, EINA_TRUE);
   return EINA_TRUE;
}

static Eina_Bool
_mbe_focused_cb(void *data EINA_UNUSED, const Eo_Event *event EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_mbe_unfocused_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   //Set the multibuttonentry to contracted mode of single line
   elm_multibuttonentry_expanded_set(event->obj, EINA_FALSE);
   return EINA_TRUE;
}

static Eina_Bool
_mbe_item_added(void *data, const Eo_Event *event EINA_UNUSED)
{
   ELM_COMBOBOX_DATA_GET(data, sd);
   elm_genlist_filter_set(sd->genlist, NULL);
   return EINA_TRUE;
}

EO_CALLBACKS_ARRAY_DEFINE(mbe_callbacks,
       { EVAS_CLICKABLE_INTERFACE_EVENT_CLICKED, _mbe_clicked_cb },
       { ELM_WIDGET_EVENT_FOCUSED, _mbe_focused_cb },
       { ELM_WIDGET_EVENT_UNFOCUSED, _mbe_unfocused_cb },
       { ELM_MULTIBUTTONENTRY_EVENT_ITEM_ADDED , _mbe_item_added });

EO_CALLBACKS_ARRAY_DEFINE(entry_callbacks,
       { ELM_ENTRY_EVENT_CHANGED_USER, _on_changed },
       { ELM_ENTRY_EVENT_ABORTED, _on_aborted });

EOLIAN static void
_elm_combobox_multiple_selection_set(Eo *obj, Elm_Combobox_Data *pd,
                                     Eina_Bool enabled)
{
   Evas_Object* scr;

   pd->multiple_selection = enabled;

   if (enabled)
     {
        // This is multibuttonentry object that will take over the MBE call
        pd->mbe = eo_add(ELM_MULTIBUTTONENTRY_CLASS, obj);
        evas_object_size_hint_weight_set(pd->mbe, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(pd->mbe, EVAS_HINT_FILL, EVAS_HINT_FILL);
        eo_event_callback_array_add(elm_multibuttonentry_entry_get(pd->mbe), entry_callbacks(), obj);
        eo_event_callback_array_add(pd->mbe, mbe_callbacks(), obj);

        pd->entry = elm_object_part_content_unset(obj, "elm.swallow.content");
        elm_object_text_set(pd->mbe, elm_object_part_text_get(pd->entry, NULL));
        elm_object_part_text_set(pd->mbe, "guide", elm_object_part_text_get(pd->entry,
                                 "guide"));
        evas_object_hide(pd->entry);

        scr = elm_scroller_add(obj);
        elm_scroller_bounce_set(scr, EINA_FALSE, EINA_TRUE);
        elm_scroller_policy_set(scr, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
        evas_object_size_hint_weight_set(scr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(scr, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_show(scr);
        elm_object_content_set(scr, pd->mbe);
        elm_object_part_content_set(obj, "elm.swallow.content", scr);
        elm_widget_can_focus_set(pd->genlist, EINA_FALSE);

        eo_composite_attach(obj, pd->mbe);
     }
   else
     {
        scr = elm_object_part_content_unset(obj, "elm.swallow.content");
        elm_object_part_content_set(obj, "elm.swallow.content", pd->entry);
        elm_object_text_set(pd->entry, elm_object_part_text_get(pd->mbe, NULL));
        elm_object_part_text_set(pd->entry, "guide",
                                 elm_object_part_text_get(pd->mbe, "guide"));
        elm_widget_can_focus_set(pd->genlist, EINA_TRUE);
        elm_genlist_item_bring_in(pd->item, ELM_GENLIST_ITEM_SCROLLTO_NONE);
        evas_object_hide(scr);

        eo_composite_attach(obj, pd->entry);
     }
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

   obj = eo_constructor(eo_super(obj, MY_CLASS));

   sd->first_filter = EINA_TRUE;

   evas_obj_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_obj_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_GLASS_PANE);

   //hover-parent
   sd->hover_parent = elm_object_parent_widget_get(obj);

   //hover
   sd->hover = eo_add(ELM_HOVER_CLASS, sd->hover_parent);
   elm_widget_mirrored_automatic_set(sd->hover, EINA_FALSE);
   elm_hover_target_set(sd->hover, obj);
   snprintf(buf, sizeof(buf), "combobox_vertical/%s",
              elm_widget_style_get(obj));
   elm_object_style_set(sd->hover, buf);

   eo_event_callback_add
     (sd->hover, EVAS_CLICKABLE_INTERFACE_EVENT_CLICKED, _on_hover_clicked, obj);
   elm_layout_signal_callback_add
     (sd->hover, "elm,action,hide,finished", "elm", _hover_end_finished, obj);

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
   eo_event_callback_add(gl, ELM_GENLIST_EVENT_FILTER_DONE, _gl_filter_finished_cb, obj);
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
   eo_event_callback_array_add(entry, entry_callbacks(), obj);
   evas_object_show(entry);

   elm_object_part_content_set(obj, "elm.swallow.content", entry);

   eo_composite_attach(obj, gl);
   eo_composite_attach(obj, entry);

   return obj;
}

EOLIAN static void
_elm_combobox_hover_begin(Eo *obj, Elm_Combobox_Data *sd)
{
   if (!sd->hover) return;

   if (sd->multiple_selection)
     elm_object_focus_set(sd->mbe, EINA_TRUE);
   else elm_object_focus_set(sd->entry, EINA_TRUE);

   _activate(obj);
}

EOLIAN static void
_elm_combobox_hover_end(Eo *obj, Elm_Combobox_Data *sd)
{
   const char *dismissstr;
   if (!sd->hover) return;
   dismissstr = elm_layout_data_get(sd->hover, "dismiss");

   if (dismissstr && !strcmp(dismissstr, "on"))
     elm_hover_dismiss(sd->hover);
   else
     {
        sd->expanded = EINA_FALSE;
        evas_object_hide(sd->hover);
        eo_event_callback_call(obj, ELM_COMBOBOX_EVENT_DISMISSED, NULL);
     } // for backward compatibility
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

   if (!strcmp(dir, "up"))
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
   ELM_COMBOBOX_DATA_GET(obj, sd);
   if (!sd->expanded)
     elm_combobox_hover_begin(obj);
   else
     {
        eo_event_callback_call(sd->genlist, EVAS_CLICKABLE_INTERFACE_EVENT_PRESSED, sd->item);
        elm_entry_cursor_end_set(sd->entry);
     }
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
_elm_combobox_elm_genlist_filter_set(Eo *obj EINA_UNUSED, Elm_Combobox_Data *pd, void *key)
{
   pd->first_filter = EINA_FALSE;

   elm_obj_genlist_filter_set(pd->genlist, key);
}

EOLIAN void
_elm_combobox_elm_widget_part_text_set(Eo *obj EINA_UNUSED, Elm_Combobox_Data *pd,
                                       const char * part, const char *label)
{
   if (pd->multiple_selection) elm_object_part_text_set(pd->mbe, part, label);
   else elm_object_part_text_set(pd->entry, part, label);
}

EOLIAN const char *
_elm_combobox_elm_widget_part_text_get(Eo *obj EINA_UNUSED, Elm_Combobox_Data *pd,
                                       const char * part)
{
   if (pd->multiple_selection) return elm_object_part_text_get(pd->mbe, part);
   return elm_object_part_text_get(pd->entry, part);
}

EOLIAN static void
_elm_combobox_evas_object_smart_resize(Eo *obj, Elm_Combobox_Data *pd,
                                       Evas_Coord w, Evas_Coord h)
{
   evas_obj_smart_resize(eo_super(obj, MY_CLASS), w, h);
   if (pd->count > 0) _table_resize(obj);
}
#include "elm_combobox.eo.c"
