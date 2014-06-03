#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_hoversel.h"

#define MY_CLASS ELM_HOVERSEL_CLASS

#define MY_CLASS_NAME "Elm_Hoversel"
#define MY_CLASS_NAME_LEGACY "elm_hoversel"

static const char SIG_SELECTED[] = "selected";
static const char SIG_DISMISSED[] = "dismissed";
static const char SIG_EXPANDED[] = "expanded";
static const char SIG_ITEM_FOCUSED[] = "item,focused";
static const char SIG_ITEM_UNFOCUSED[] = "item,unfocused";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_SELECTED, ""},
   {SIG_DISMISSED, ""},
   {SIG_EXPANDED, ""},
   {SIG_ITEM_FOCUSED, ""},
   {SIG_ITEM_UNFOCUSED, ""},
   {"clicked", ""}, /**< handled by parent button class */
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {NULL, NULL}
};

static Eina_Bool _key_action_move(Evas_Object *obj, const char *params);

static const Elm_Action key_actions[] = {
   {"move", _key_action_move},
   {NULL, NULL}
};

EOLIAN static Eina_Bool
_elm_hoversel_elm_widget_translate(Eo *obj EINA_UNUSED, Elm_Hoversel_Data *sd)
{
   Elm_Hoversel_Item *it;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->items, l, it)
     elm_widget_item_translate(it);

   eo_do_super(obj, MY_CLASS, elm_obj_widget_translate());

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_hoversel_elm_widget_theme_apply(Eo *obj, Elm_Hoversel_Data *sd)
{
   Eina_Bool int_ret = EINA_FALSE;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   char buf[4096];
   const char *style;

   style = eina_stringshare_add(elm_widget_style_get(obj));

   if (sd->horizontal)
     snprintf(buf, sizeof(buf), "hoversel_horizontal/%s", style);
   else
     snprintf(buf, sizeof(buf), "hoversel_vertical/%s", style);

   /* hoversel's style has an extra bit: orientation */
   eina_stringshare_replace(&(wd->style), buf);

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_theme_apply());
   if (!int_ret) return EINA_FALSE;

   eina_stringshare_replace(&(wd->style), style);

   eina_stringshare_del(style);

   if (sd->hover)
     elm_widget_mirrored_set(sd->hover, elm_widget_mirrored_get(obj));

   elm_hoversel_hover_end(obj);

   return EINA_TRUE;
}

static void
_on_hover_clicked(void *data,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED)
{
   elm_hoversel_hover_end(data);
}

static void
_on_item_clicked(void *data,
                 Evas_Object *obj EINA_UNUSED,
                 void *event_info EINA_UNUSED)
{
   Elm_Hoversel_Item *item = data;
   Evas_Object *obj2 = WIDGET(item);

   if (item->func) item->func((void *)item->base.data, obj2, item);
   evas_object_smart_callback_call(obj2, SIG_SELECTED, item);
   elm_hoversel_hover_end(obj2);
}

static void
_item_focused_cb(void *data,
                 Evas_Object *obj EINA_UNUSED,
                 void *event_info EINA_UNUSED)
{
   Elm_Object_Item *it = data;

   evas_object_smart_callback_call(WIDGET(it), SIG_ITEM_FOCUSED, it);
}

static void
_item_unfocused_cb(void *data,
                   Evas_Object *obj EINA_UNUSED,
                   void *event_info EINA_UNUSED)
{
   Elm_Object_Item *it = data;

   evas_object_smart_callback_call(WIDGET(it), SIG_ITEM_UNFOCUSED, it);
}

static void
_activate(Evas_Object *obj)
{
   Elm_Hoversel_Item *item;
   Evas_Object *bt, *bx, *ic;
   const Eina_List *l;
   char buf[4096];

   ELM_HOVERSEL_DATA_GET(obj, sd);

   if (sd->expanded)
     {
        elm_hoversel_hover_end(obj);
        return;
     }
   sd->expanded = EINA_TRUE;

   if (elm_widget_disabled_get(obj)) return;

   sd->hover = elm_hover_add(sd->hover_parent);
   elm_widget_sub_object_add(obj, sd->hover);
   elm_widget_mirrored_automatic_set(sd->hover, EINA_FALSE);

   if (sd->horizontal)
     snprintf(buf, sizeof(buf), "hoversel_horizontal/%s",
              elm_widget_style_get(obj));
   else
     snprintf(buf, sizeof(buf), "hoversel_vertical/%s",
              elm_widget_style_get(obj));

   elm_object_style_set(sd->hover, buf);

   evas_object_smart_callback_add
     (sd->hover, "clicked", _on_hover_clicked, obj);
   elm_hover_target_set(sd->hover, obj);

   /* hover's content */
   bx = elm_box_add(sd->hover);
   elm_widget_mirrored_automatic_set(bx, EINA_FALSE);
   elm_box_homogeneous_set(bx, EINA_TRUE);
   elm_box_horizontal_set(bx, sd->horizontal);

   if (sd->horizontal)
     snprintf(buf, sizeof(buf), "hoversel_horizontal_entry/%s",
              elm_widget_style_get(obj));
   else
     snprintf(buf, sizeof(buf), "hoversel_vertical_entry/%s",
              elm_widget_style_get(obj));

   EINA_LIST_FOREACH(sd->items, l, item)
     {
        VIEW(item) = bt = elm_button_add(bx);
        elm_widget_mirrored_automatic_set(bt, EINA_FALSE);
        elm_widget_mirrored_set(bt, elm_widget_mirrored_get(obj));
        elm_object_style_set(bt, buf);
        elm_object_text_set(bt, item->label);

        if (item->icon_file)
          {
             ic = elm_icon_add(bt);
             elm_image_resizable_set(ic, EINA_FALSE, EINA_TRUE);
             if (item->icon_type == ELM_ICON_FILE)
               elm_image_file_set(ic, item->icon_file, item->icon_group);
             else if (item->icon_type == ELM_ICON_STANDARD)
               elm_icon_standard_set(ic, item->icon_file);
             elm_object_part_content_set(bt, "icon", ic);
          }

        evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
        evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_box_pack_end(bx, bt);
        evas_object_smart_callback_add(bt, "clicked", _on_item_clicked, item);
        evas_object_show(bt);
        evas_object_smart_callback_add(bt, SIG_LAYOUT_FOCUSED, _item_focused_cb, item);
        evas_object_smart_callback_add(bt, SIG_LAYOUT_UNFOCUSED, _item_unfocused_cb, item);
     }

   if (sd->horizontal)
     elm_object_part_content_set(sd->hover, elm_hover_best_content_location_get
                                   (sd->hover, ELM_HOVER_AXIS_HORIZONTAL), bx);
   else
     elm_object_part_content_set(sd->hover, elm_hover_best_content_location_get
                                   (sd->hover, ELM_HOVER_AXIS_VERTICAL), bx);

   evas_object_smart_callback_call(obj, SIG_EXPANDED, NULL);
   evas_object_show(sd->hover);
}

static void
_on_clicked(void *data,
            Evas_Object *obj EINA_UNUSED,
            void *event_info EINA_UNUSED)
{
   _activate(data);
}

static void
_on_parent_del(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info EINA_UNUSED)
{
   elm_hoversel_hover_parent_set(data, NULL);
}

static const char *
_item_text_get_hook(const Elm_Object_Item *it,
                    const char *part)
{
   if (part && strcmp(part, "default")) return NULL;
   return ((Elm_Hoversel_Item *)it)->label;
}

static void
_item_signal_emit_hook(Elm_Object_Item *it,
                       const char *emission,
                       const char *source)
{
   elm_object_signal_emit(VIEW(it), emission, source);
}

static void
_item_style_set_hook(Elm_Object_Item *it,
                     const char *style)
{
   elm_object_style_set(VIEW(it), style);
}

static const char *
_item_style_get_hook(Elm_Object_Item *it)
{
   return elm_object_style_get(VIEW(it));
}

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   Elm_Hoversel_Item *item = (Elm_Hoversel_Item *)it;

   ELM_HOVERSEL_DATA_GET_OR_RETURN_VAL(WIDGET(item), sd, EINA_FALSE);

   elm_hoversel_hover_end(WIDGET(item));
   sd->items = eina_list_remove(sd->items, item);
   eina_stringshare_del(item->label);
   eina_stringshare_del(item->icon_file);
   eina_stringshare_del(item->icon_group);

   return EINA_TRUE;
}

EOLIAN static void
_elm_hoversel_evas_object_smart_add(Eo *obj, Elm_Hoversel_Data *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   elm_widget_mirrored_automatic_set(obj, EINA_FALSE);

   evas_object_smart_callback_add(obj, "clicked", _on_clicked, obj);

   //What are you doing here?
   eo_do(obj, elm_obj_widget_theme_apply());
}

EOLIAN static void
_elm_hoversel_evas_object_smart_del(Eo *obj, Elm_Hoversel_Data *sd)
{
   Elm_Hoversel_Item *item;

   EINA_LIST_FREE(sd->items, item)
     {
        eina_stringshare_del(item->label);
        eina_stringshare_del(item->icon_file);
        eina_stringshare_del(item->icon_group);
        elm_widget_item_free(item);
     }
   elm_hoversel_hover_parent_set(obj, NULL);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EOLIAN static void
_elm_hoversel_evas_object_smart_show(Eo *obj, Elm_Hoversel_Data *sd)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_show());
   evas_object_show(sd->hover);
}

EOLIAN static void
_elm_hoversel_evas_object_smart_hide(Eo *obj, Elm_Hoversel_Data *sd)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_hide());
   evas_object_hide(sd->hover);
}

EOLIAN static void
_elm_hoversel_elm_widget_parent_set(Eo *obj, Elm_Hoversel_Data *_pd EINA_UNUSED, Evas_Object *parent)
{
   elm_hoversel_hover_parent_set(obj, parent);
}

EOLIAN static Eina_Bool
_elm_hoversel_elm_button_admits_autorepeat_get(Eo *obj EINA_UNUSED, Elm_Hoversel_Data *sd EINA_UNUSED)
{
   return EINA_FALSE;
}

EAPI Evas_Object *
elm_hoversel_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

EOLIAN static void
_elm_hoversel_eo_base_constructor(Eo *obj, Elm_Hoversel_Data *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks));
}

EOLIAN static void
_elm_hoversel_hover_parent_set(Eo *obj, Elm_Hoversel_Data *sd, Evas_Object *parent)
{
   if (sd->hover_parent)
     evas_object_event_callback_del_full
       (sd->hover_parent, EVAS_CALLBACK_DEL, _on_parent_del, obj);

   sd->hover_parent = parent;
   if (sd->hover_parent)
     evas_object_event_callback_add
       (sd->hover_parent, EVAS_CALLBACK_DEL, _on_parent_del, obj);
}

EOLIAN static Evas_Object*
_elm_hoversel_hover_parent_get(Eo *obj EINA_UNUSED, Elm_Hoversel_Data *sd)
{
   return sd->hover_parent;
}

EOLIAN static void
_elm_hoversel_horizontal_set(Eo *obj, Elm_Hoversel_Data *sd, Eina_Bool horizontal)
{
   sd->horizontal = !!horizontal;

   eo_do(obj, elm_obj_widget_theme_apply());
}

EOLIAN static Eina_Bool
_elm_hoversel_horizontal_get(Eo *obj EINA_UNUSED, Elm_Hoversel_Data *sd)
{
   return sd->horizontal;
}

EOLIAN static void
_elm_hoversel_hover_begin(Eo *obj, Elm_Hoversel_Data *sd)
{
   if (sd->hover) return;

   _activate(obj);
}

EOLIAN static void
_elm_hoversel_hover_end(Eo *obj, Elm_Hoversel_Data *sd)
{
   Elm_Object_Item *it;
   Eina_List *l;

   if (!sd->hover) return;

   sd->expanded = EINA_FALSE;

   EINA_LIST_FOREACH(sd->items, l, it)
     {
        VIEW(it) = NULL;
     }
   ELM_SAFE_FREE(sd->hover, evas_object_del);

   evas_object_smart_callback_call(obj, SIG_DISMISSED, NULL);
}

EOLIAN static Eina_Bool
_elm_hoversel_expanded_get(Eo *obj EINA_UNUSED, Elm_Hoversel_Data *sd)
{
   return (sd->hover) ? EINA_TRUE : EINA_FALSE;
}

EOLIAN static void
_elm_hoversel_clear(Eo *obj EINA_UNUSED, Elm_Hoversel_Data *sd)
{
   Elm_Object_Item *it;
   Eina_List *l, *ll;

   EINA_LIST_FOREACH_SAFE(sd->items, l, ll, it)
     {
        elm_widget_item_del(it);
     }
}

EOLIAN static const Eina_List*
_elm_hoversel_items_get(Eo *obj EINA_UNUSED, Elm_Hoversel_Data *sd)
{
   return sd->items;
}

EOLIAN static Elm_Object_Item*
_elm_hoversel_item_add(Eo *obj, Elm_Hoversel_Data *sd, const char *label, const char *icon_file, Elm_Icon_Type icon_type, Evas_Smart_Cb func, const void *data)
{
   Elm_Hoversel_Item *item = elm_widget_item_new(obj, Elm_Hoversel_Item);
   if (!item) return NULL;

   elm_widget_item_del_pre_hook_set(item, _item_del_pre_hook);
   elm_widget_item_text_get_hook_set(item, _item_text_get_hook);
   elm_widget_item_signal_emit_hook_set(item, _item_signal_emit_hook);
   elm_widget_item_style_set_hook_set(item, _item_style_set_hook);
   elm_widget_item_style_get_hook_set(item, _item_style_get_hook);

   item->label = eina_stringshare_add(label);
   item->icon_file = eina_stringshare_add(icon_file);
   item->icon_type = icon_type;
   item->func = func;
   item->base.data = data;

   sd->items = eina_list_append(sd->items, item);

   return (Elm_Object_Item *)item;
}

EAPI void
elm_hoversel_item_icon_set(Elm_Object_Item *it,
                           const char *icon_file,
                           const char *icon_group,
                           Elm_Icon_Type icon_type)
{
   ELM_HOVERSEL_ITEM_CHECK_OR_RETURN(it);

   Elm_Hoversel_Item *item = (Elm_Hoversel_Item *)it;

   eina_stringshare_replace(&item->icon_file, icon_file);
   eina_stringshare_replace(&item->icon_group, icon_group);

   item->icon_type = icon_type;
}

EAPI void
elm_hoversel_item_icon_get(const Elm_Object_Item *it,
                           const char **icon_file,
                           const char **icon_group,
                           Elm_Icon_Type *icon_type)
{
   ELM_HOVERSEL_ITEM_CHECK_OR_RETURN(it);

   Elm_Hoversel_Item *item = (Elm_Hoversel_Item *)it;

   if (icon_file) *icon_file = item->icon_file;
   if (icon_group) *icon_group = item->icon_group;
   if (icon_type) *icon_type = item->icon_type;
}

static Elm_Hoversel_Item *
item_focused_get(Elm_Hoversel_Data *sd)
{
   Elm_Hoversel_Item *item;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->items, l, item)
     if (elm_object_focus_get(VIEW(item)))
       return item;
   return NULL;
}

static Eina_Bool
_key_action_move(Evas_Object *obj, const char *params)
{
   ELM_HOVERSEL_DATA_GET(obj, sd);
   const char *dir = params;

   Elm_Hoversel_Item  *litem, *fitem;
   litem = eina_list_last_data_get(sd->items);
   fitem = eina_list_data_get(sd->items);

   if (!strcmp(dir, "down"))
     {
        if ((!sd->horizontal) &&
            (item_focused_get(sd) == litem))
          {
            elm_object_focus_set(VIEW(fitem), EINA_TRUE);
            return EINA_TRUE;
          }
        elm_widget_focus_cycle(sd->hover, ELM_FOCUS_DOWN);
        return EINA_TRUE;
     }
   else if (!strcmp(dir, "up"))
     {
        if ((!sd->horizontal) &&
            (item_focused_get(sd) == fitem))
          {
            elm_object_focus_set(VIEW(litem), EINA_TRUE);
            return EINA_TRUE;
          }
        elm_widget_focus_cycle(sd->hover, ELM_FOCUS_UP);
        return EINA_TRUE;
     }
   else if (!strcmp(dir, "left"))
     {
        if (sd->horizontal &&
            (item_focused_get(sd) == fitem))
          {
            elm_object_focus_set(VIEW(litem), EINA_TRUE);
            return EINA_TRUE;
          }
        elm_widget_focus_cycle(sd->hover, ELM_FOCUS_LEFT);
        return EINA_TRUE;
     }
   else if (!strcmp(dir, "right"))
     {
        if (sd->horizontal &&
            (item_focused_get(sd) == litem))
          {
            elm_object_focus_set(VIEW(fitem), EINA_TRUE);
            return EINA_TRUE;
          }
        elm_widget_focus_cycle(sd->hover, ELM_FOCUS_RIGHT);
        return EINA_TRUE;
     }
   else return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_hoversel_elm_widget_event(Eo *obj, Elm_Hoversel_Data *sd, Evas_Object *src, Evas_Callback_Type type, void *event_info)
{
   (void) src;
   Eina_Bool int_ret = EINA_FALSE;
   Evas_Event_Key_Down *ev = event_info;

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_event(src, type, event_info));
   if (int_ret) return EINA_FALSE;

   if (!sd || !sd->hover) return EINA_FALSE;
   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

   if (!_elm_config_key_binding_call(obj, ev, key_actions))
     return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

static void
_elm_hoversel_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#include "elc_hoversel.eo.c"
