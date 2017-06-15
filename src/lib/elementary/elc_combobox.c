#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

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
static const char SIG_CLICKED[] = "clicked";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_DISMISSED, ""},
   {SIG_EXPANDED, ""},
   {SIG_ITEM_SELECTED, ""},
   {SIG_ITEM_PRESSED, ""},
   {SIG_FILTER_DONE, ""},
   {SIG_CLICKED, ""}, /**< handled by parent button class */
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {NULL, NULL}
};

static void _table_resize(void *data);
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
   elm_obj_widget_translate(efl_super(obj, MY_CLASS));
   elm_obj_widget_translate(sd->genlist);
   elm_obj_widget_translate(sd->entry);

   if (sd->hover)
     elm_obj_widget_translate(sd->hover);

   return EINA_TRUE;
}

EOLIAN static Elm_Theme_Apply
_elm_combobox_elm_widget_theme_apply(Eo *obj, Elm_Combobox_Data *sd)
{
   const char *style;
   Elm_Theme_Apply int_ret = ELM_THEME_APPLY_FAILED;
   Eina_Bool mirrored;
   char buf[128];

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, ELM_THEME_APPLY_FAILED);

   style = eina_stringshare_add(elm_widget_style_get(obj));

   snprintf(buf, sizeof(buf), "combobox_vertical/%s", style);

   /* combobox's style has no extra bit for orientation but could have... */
   eina_stringshare_replace(&(wd->style), buf);

   int_ret = elm_obj_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return ELM_THEME_APPLY_FAILED;

   eina_stringshare_replace(&(wd->style), style);

   mirrored = efl_ui_mirrored_get(obj);

   if (sd->hover)
     {
        efl_ui_mirrored_set(sd->hover, mirrored);
        elm_widget_style_set(sd->hover, buf);
     }

   efl_ui_mirrored_set(sd->genlist, mirrored);
   efl_ui_mirrored_set(sd->entry, mirrored);

   elm_widget_style_set(sd->genlist, buf);
   elm_widget_style_set(sd->entry, buf);

   eina_stringshare_del(style);

   return int_ret;
}

static void
_on_hover_clicked(void *data, const Efl_Event *event)
{
   const char *dismissstr;

   dismissstr = elm_layout_data_get(event->object, "dismiss");

   if (!dismissstr || strcmp(dismissstr, "on"))
     elm_combobox_hover_end(data); // for backward compatibility
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
        efl_event_callback_legacy_call(data, ELM_COMBOBOX_EVENT_DISMISSED, NULL);
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
_item_realized(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _table_resize(data);
}

static void
_table_resize(void *data)
{
   ELM_COMBOBOX_DATA_GET(data, sd);
   if (sd->count > 0)
     {
        int hover_parent_w, hover_parent_h, obj_h, obj_w, obj_y, win_y_offset;
        int current_height, h;
        Eina_List *realized;

        sd->item = elm_genlist_first_item_get(sd->genlist);

        if (!(realized = elm_genlist_realized_items_get(sd->genlist)))
          {
             //nothing realized and wait until at least one item is realized
             h = 1;
             evas_object_smart_callback_add(sd->genlist, "realized", _item_realized, data);
          }
        else
          {
             // take the first, and update according to that
             evas_object_geometry_get(elm_object_item_track(eina_list_data_get(realized)), NULL, NULL,
                                      NULL, &h);
          }

        sd->item_height = h;
        evas_object_geometry_get(sd->entry, NULL, NULL, &obj_w, NULL);
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

   count_items_genlist(obj);

   if (sd->count <= 0) return;

   _table_resize(obj);
   elm_object_part_content_set(sd->hover, sd->best_location, sd->tbl);
   evas_object_show(sd->genlist);
   elm_genlist_item_selected_set(sd->item, EINA_TRUE);
   evas_object_show(sd->hover);
   efl_event_callback_legacy_call(obj, ELM_COMBOBOX_EVENT_EXPANDED, NULL);
}

static void
_on_item_selected(void *data , Evas_Object *obj EINA_UNUSED, void *event)
{
   ELM_COMBOBOX_DATA_GET(data, sd);
   elm_object_focus_set(sd->entry, EINA_TRUE);

   efl_event_callback_legacy_call(data, ELM_COMBOBOX_EVENT_ITEM_SELECTED, event);
}

static void
_on_item_pressed(void *data , Evas_Object *obj EINA_UNUSED, void *event)
{
   efl_event_callback_legacy_call(data, ELM_COMBOBOX_EVENT_ITEM_PRESSED, event);
}

static void
_gl_filter_finished_cb(void *data, const Efl_Event *event)
{
   char buf[1024];
   ELM_COMBOBOX_DATA_GET(data, sd);

   count_items_genlist(data);

   if (sd->first_filter)
     {
        sd->first_filter = EINA_FALSE;
        return;
     }

   efl_event_callback_legacy_call(data, ELM_COMBOBOX_EVENT_FILTER_DONE, event->info);

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
}

static void
_on_aborted(void *data, const Efl_Event *event EINA_UNUSED)
{
   ELM_COMBOBOX_DATA_GET(data, sd);
   if (sd->expanded) elm_combobox_hover_end(data);
}

static void
_on_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   efl_event_callback_legacy_call(data, ELM_ENTRY_EVENT_CHANGED, NULL);
}

static void
_on_clicked(void *data, const Efl_Event *event EINA_UNUSED)
{
   elm_combobox_hover_begin(data);
}

EOLIAN static void
_elm_combobox_efl_canvas_group_group_add(Eo *obj, Elm_Combobox_Data *sd EINA_UNUSED)
{
   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   efl_ui_mirrored_automatic_set(obj, EINA_FALSE);

   efl_event_callback_add(obj, EFL_UI_EVENT_CLICKED, _on_clicked, obj);

   //What are you doing here?
   elm_obj_widget_theme_apply(obj);
}

EOLIAN static void
_elm_combobox_efl_canvas_group_group_del(Eo *obj, Elm_Combobox_Data *sd)
{
   sd->hover_parent = NULL;
   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_elm_combobox_efl_gfx_visible_set(Eo *obj, Elm_Combobox_Data *sd, Eina_Bool vis)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_VISIBLE, 0, vis))
     return;

   efl_gfx_visible_set(efl_super(obj, MY_CLASS), vis);
   if (vis)
     {
        if (sd->expanded) evas_object_show(sd->hover);
     }
   else
     {
        if (sd->hover) evas_object_hide(sd->hover);
     }
}

EOLIAN static Eina_Bool
_elm_combobox_efl_ui_autorepeat_autorepeat_supported_get(Eo *obj EINA_UNUSED,
                                                         Elm_Combobox_Data *sd EINA_UNUSED)
{
   return EINA_FALSE;
}

EAPI Evas_Object *
elm_combobox_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = efl_add(MY_CLASS, parent);
   return obj;
}

EOLIAN static Eo *
_elm_combobox_efl_object_constructor(Eo *obj, Elm_Combobox_Data *sd)
{
   Evas_Object *gl;
   Evas_Object *entry;
   char buf[128];

   obj = efl_constructor(efl_super(obj, MY_CLASS));

   sd->first_filter = EINA_TRUE;

   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_GLASS_PANE);

   //hover-parent
   sd->hover_parent = elm_object_top_widget_get(obj);

   //hover
   sd->hover = efl_add(ELM_HOVER_CLASS, sd->hover_parent);
   evas_object_layer_set(sd->hover, EVAS_LAYER_MAX);
   efl_ui_mirrored_automatic_set(sd->hover, EINA_FALSE);
   elm_hover_target_set(sd->hover, obj);
   elm_widget_sub_object_add(obj, sd->hover);
   snprintf(buf, sizeof(buf), "combobox_vertical/%s",
              elm_widget_style_get(obj));
   elm_object_style_set(sd->hover, buf);

   efl_event_callback_add
     (sd->hover, EFL_UI_EVENT_CLICKED, _on_hover_clicked, obj);
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
   sd->genlist = gl = efl_add(ELM_GENLIST_CLASS, obj);
   elm_genlist_filter_set(gl, NULL);
   efl_ui_mirrored_automatic_set(gl, EINA_FALSE);
   efl_ui_mirrored_set(gl, efl_ui_mirrored_get(obj));
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(gl, "selected", _on_item_selected, obj);
   evas_object_smart_callback_add(gl, "pressed", _on_item_pressed, obj);
   efl_event_callback_add(gl, ELM_GENLIST_EVENT_FILTER_DONE, _gl_filter_finished_cb, obj);
   elm_genlist_homogeneous_set(gl, EINA_TRUE);
   elm_genlist_mode_set(gl, ELM_LIST_COMPRESS);
   elm_table_pack(sd->tbl, gl, 0, 0, 1, 1);
   elm_object_style_set(gl, buf);

   // This is the entry object that will take over the entry call
   sd->entry = entry = efl_add(ELM_ENTRY_CLASS, obj);
   efl_ui_mirrored_automatic_set(entry, EINA_FALSE);
   efl_ui_mirrored_set(entry, efl_ui_mirrored_get(obj));
   elm_scroller_policy_set(entry, ELM_SCROLLER_POLICY_OFF,
                           ELM_SCROLLER_POLICY_OFF);
   elm_entry_scrollable_set(entry, EINA_TRUE);
   elm_entry_single_line_set(entry, EINA_TRUE);
   efl_event_callback_add(entry, ELM_ENTRY_EVENT_CHANGED_USER, _on_changed, obj);
   efl_event_callback_add(entry, ELM_ENTRY_EVENT_ABORTED, _on_aborted, obj);
   evas_object_show(entry);

   elm_object_part_content_set(obj, "elm.swallow.content", entry);
   elm_object_style_set(entry, buf);

   efl_composite_attach(obj, gl);
   efl_composite_attach(obj, entry);

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
   const char *dismissstr;
   if (!sd->hover) return;
   dismissstr = elm_layout_data_get(sd->hover, "dismiss");

   if (dismissstr && !strcmp(dismissstr, "on"))
     elm_hover_dismiss(sd->hover);
   else
     {
        sd->expanded = EINA_FALSE;
        evas_object_hide(sd->hover);
        efl_event_callback_legacy_call(obj, ELM_COMBOBOX_EVENT_DISMISSED, NULL);
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
        efl_event_callback_legacy_call(sd->genlist, EFL_UI_EVENT_PRESSED, sd->item);
        elm_entry_cursor_end_set(sd->entry);
     }
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_combobox_elm_widget_widget_event(Eo *obj, Elm_Combobox_Data *sd,
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
_elm_combobox_class_constructor(Efl_Class *klass)
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
   elm_object_part_text_set(pd->entry, part, label);
}

EOLIAN const char *
_elm_combobox_elm_widget_part_text_get(Eo *obj EINA_UNUSED, Elm_Combobox_Data *pd,
                                       const char * part)
{
   return elm_object_part_text_get(pd->entry, part);
}

EOLIAN static void
_elm_combobox_efl_gfx_size_set(Eo *obj, Elm_Combobox_Data *pd, Evas_Coord w, Evas_Coord h)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, w, h))
     return;

   if (pd->count > 0) _table_resize(obj);
   efl_gfx_size_set(efl_super(obj, MY_CLASS), w, h);
}

/* Internal EO APIs and hidden overrides */

#define ELM_COMBOBOX_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_combobox)

#include "elm_combobox.eo.c"
