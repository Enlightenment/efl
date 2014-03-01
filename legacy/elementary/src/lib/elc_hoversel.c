#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_hoversel.h"

EAPI Eo_Op ELM_OBJ_HOVERSEL_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_HOVERSEL_CLASS

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

static void
_elm_hoversel_smart_translate(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Hoversel_Smart_Data *sd = _pd;
   Elm_Hoversel_Item *it;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->items, l, it)
     elm_widget_item_translate(it);

   eo_do_super(obj, MY_CLASS, elm_wdg_translate(NULL));

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_hoversel_smart_theme(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;

   Elm_Hoversel_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   char buf[4096];
   const char *style;

   style = eina_stringshare_add(elm_widget_style_get(obj));

   if (sd->horizontal)
     snprintf(buf, sizeof(buf), "hoversel_horizontal/%s", style);
   else
     snprintf(buf, sizeof(buf), "hoversel_vertical/%s", style);

   /* hoversel's style has an extra bit: orientation */
   eina_stringshare_replace(&(wd->style), buf);

   eo_do_super(obj, MY_CLASS, elm_wdg_theme_apply(&int_ret));
   if (!int_ret) return;

   eina_stringshare_replace(&(wd->style), style);

   eina_stringshare_del(style);

   if (sd->hover)
     elm_widget_mirrored_set(sd->hover, elm_widget_mirrored_get(obj));

   elm_hoversel_hover_end(obj);

   if (ret) *ret = EINA_TRUE;
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
             ic = elm_icon_add(obj);
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

static void
_elm_hoversel_smart_add(Eo *obj, void *_pd EINA_UNUSED,
                        va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   elm_widget_mirrored_automatic_set(obj, EINA_FALSE);

   evas_object_smart_callback_add(obj, "clicked", _on_clicked, obj);

   //What are you doing here?
   eo_do(obj, elm_wdg_theme_apply(NULL));
}

static void
_elm_hoversel_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Hoversel_Item *item;

   Elm_Hoversel_Smart_Data *sd = _pd;

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

static void
_elm_hoversel_smart_show(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Elm_Hoversel_Smart_Data *sd = _pd;
   eo_do_super(obj, MY_CLASS, evas_obj_smart_show());
   evas_object_show(sd->hover);
}

static void
_elm_hoversel_smart_hide(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Hoversel_Smart_Data *sd = _pd;
   eo_do_super(obj, MY_CLASS, evas_obj_smart_hide());
   evas_object_hide(sd->hover);
}

static void
_elm_hoversel_smart_parent_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *parent = va_arg(*list, Evas_Object *);
   elm_hoversel_hover_parent_set(obj, parent);
}

static void
_elm_hoversel_smart_admits_autorepeat_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
}

EAPI Evas_Object *
elm_hoversel_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));
}

EAPI void
elm_hoversel_hover_parent_set(Evas_Object *obj,
                              Evas_Object *parent)
{
   ELM_HOVERSEL_CHECK(obj);
   eo_do(obj, elm_obj_hoversel_hover_parent_set(parent));
}

static void
_hover_parent_set(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *parent = va_arg(*list, Evas_Object *);
   Elm_Hoversel_Smart_Data *sd = _pd;

   if (sd->hover_parent)
     evas_object_event_callback_del_full
       (sd->hover_parent, EVAS_CALLBACK_DEL, _on_parent_del, obj);

   sd->hover_parent = parent;
   if (sd->hover_parent)
     evas_object_event_callback_add
       (sd->hover_parent, EVAS_CALLBACK_DEL, _on_parent_del, obj);
}

EAPI Evas_Object *
elm_hoversel_hover_parent_get(const Evas_Object *obj)
{
   ELM_HOVERSEL_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do((Eo *) obj, elm_obj_hoversel_hover_parent_get(&ret));
   return ret;
}

static void
_hover_parent_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   Elm_Hoversel_Smart_Data *sd = _pd;

   if (ret) *ret = sd->hover_parent;
}

EAPI void
elm_hoversel_horizontal_set(Evas_Object *obj,
                            Eina_Bool horizontal)
{
   ELM_HOVERSEL_CHECK(obj);
   eo_do(obj, elm_obj_hoversel_horizontal_set(horizontal));
}

static void
_horizontal_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool horizontal = va_arg(*list, int);
   Elm_Hoversel_Smart_Data *sd = _pd;

   sd->horizontal = !!horizontal;

   eo_do(obj, elm_wdg_theme_apply(NULL));
}

EAPI Eina_Bool
elm_hoversel_horizontal_get(const Evas_Object *obj)
{
   ELM_HOVERSEL_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_hoversel_horizontal_get(&ret));
   return ret;
}

static void
_horizontal_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Hoversel_Smart_Data *sd = _pd;

   if (ret) *ret = sd->horizontal;
}

EAPI void
elm_hoversel_hover_begin(Evas_Object *obj)
{
   ELM_HOVERSEL_CHECK(obj);
   eo_do(obj, elm_obj_hoversel_hover_begin());
}

static void
_hover_begin(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Hoversel_Smart_Data *sd = _pd;

   if (sd->hover) return;

   _activate(obj);
}

EAPI void
elm_hoversel_hover_end(Evas_Object *obj)
{
   ELM_HOVERSEL_CHECK(obj);
   eo_do(obj, elm_obj_hoversel_hover_end());
}

static void
_hover_end(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Object_Item *it;
   Eina_List *l;
   Elm_Hoversel_Smart_Data *sd = _pd;

   if (!sd->hover) return;

   sd->expanded = EINA_FALSE;

   EINA_LIST_FOREACH(sd->items, l, it)
     {
        VIEW(it) = NULL;
     }
   ELM_SAFE_FREE(sd->hover, evas_object_del);

   evas_object_smart_callback_call(obj, SIG_DISMISSED, NULL);
}

EAPI Eina_Bool
elm_hoversel_expanded_get(const Evas_Object *obj)
{
   ELM_HOVERSEL_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_hoversel_expanded_get(&ret));
   return ret;
}

static void
_expanded_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Hoversel_Smart_Data *sd = _pd;

   *ret = (sd->hover) ? EINA_TRUE : EINA_FALSE;
}

EAPI void
elm_hoversel_clear(Evas_Object *obj)
{
   ELM_HOVERSEL_CHECK(obj);
   eo_do(obj, elm_obj_hoversel_clear());
}

static void
_clear(Eo *obj EINA_UNUSED, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Object_Item *it;
   Eina_List *l, *ll;

   Elm_Hoversel_Smart_Data *sd = _pd;

   EINA_LIST_FOREACH_SAFE(sd->items, l, ll, it)
     {
        elm_widget_item_del(it);
     }
}

EAPI const Eina_List *
elm_hoversel_items_get(const Evas_Object *obj)
{
   ELM_HOVERSEL_CHECK(obj) NULL;
   const Eina_List *ret = NULL;
   eo_do((Eo *) obj, elm_obj_hoversel_items_get(&ret));
   return ret;
}

static void
_items_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const Eina_List **ret = va_arg(*list, const Eina_List **);
   Elm_Hoversel_Smart_Data *sd = _pd;

   if (ret) *ret = sd->items;
}

EAPI Elm_Object_Item *
elm_hoversel_item_add(Evas_Object *obj,
                      const char *label,
                      const char *icon_file,
                      Elm_Icon_Type icon_type,
                      Evas_Smart_Cb func,
                      const void *data)
{
   ELM_HOVERSEL_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do(obj, elm_obj_hoversel_item_add(label, icon_file, icon_type, func, data, &ret));
   return ret;
}

static void
_item_add(Eo *obj, void *_pd, va_list *list)
{
   const char *label = va_arg(*list, const char *);
   const char *icon_file = va_arg(*list, const char *);
   Elm_Icon_Type icon_type = va_arg(*list, Elm_Icon_Type);
   Evas_Smart_Cb func = va_arg(*list, Evas_Smart_Cb);
   const void *data = va_arg(*list, const void *);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   Elm_Hoversel_Smart_Data *sd = _pd;
   *ret = NULL;

   Elm_Hoversel_Item *item = elm_widget_item_new(obj, Elm_Hoversel_Item);
   if (!item) return;

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

   *ret = (Elm_Object_Item *)item;
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
item_focused_get(Elm_Hoversel_Smart_Data *sd)
{
   Elm_Hoversel_Item *item;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->items, l, item)
     if (elm_object_focus_get(VIEW(item)))
       return item;
   return NULL;
}

static void
_elm_hoversel_smart_event(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *src = va_arg(*list, Evas_Object *);
   (void) src;
   Evas_Callback_Type type = va_arg(*list, Evas_Callback_Type);
   void *event_info = va_arg(*list, void *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;
   Evas_Event_Key_Down *ev = event_info;
   Elm_Hoversel_Smart_Data *sd = _pd;

   Elm_Hoversel_Item  *litem, *fitem;

   eo_do_super(obj, MY_CLASS, elm_wdg_event(src, type, event_info, &int_ret));
   if (int_ret) return;

   if (!sd || !sd->hover) return;
   if (elm_widget_disabled_get(obj)) return;
   if (type != EVAS_CALLBACK_KEY_DOWN) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   litem = eina_list_last_data_get(sd->items);
   fitem = eina_list_data_get(sd->items);

   if ((!strcmp(ev->key, "Down")) ||
      ((!strcmp(ev->key, "KP_Down")) && (!ev->string)))
     {
        if (item_focused_get(sd) == litem)
          {
            elm_object_focus_set(VIEW(fitem), EINA_TRUE);
            goto success;
          }
        elm_widget_focus_cycle(sd->hover, ELM_FOCUS_DOWN);
        goto success;
     }
   else if ((!strcmp(ev->key, "Up")) ||
           ((!strcmp(ev->key, "KP_Up")) && (!ev->string)))
     {
        if (item_focused_get(sd) == fitem)
          {
            elm_object_focus_set(VIEW(litem), EINA_TRUE);
            goto success;
          }
        elm_widget_focus_cycle(sd->hover, ELM_FOCUS_UP);
        goto success;
     }
   else if ((!strcmp(ev->key, "Left")) ||
           ((!strcmp(ev->key, "KP_Left")) && (!ev->string)))
     {
        if (item_focused_get(sd) == fitem)
          {
            elm_object_focus_set(VIEW(litem), EINA_TRUE);
            goto success;
          }
        elm_widget_focus_cycle(sd->hover, ELM_FOCUS_LEFT);
        goto success;
     }
   else if ((!strcmp(ev->key, "Right")) ||
           ((!strcmp(ev->key, "KP_Right")) && (!ev->string)))
     {
        if (item_focused_get(sd) == litem)
          {
            elm_object_focus_set(VIEW(fitem), EINA_TRUE);
            goto success;
          }
        elm_widget_focus_cycle(sd->hover, ELM_FOCUS_RIGHT);
        goto success;
     }

   return;

   success:
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   if (ret) *ret = EINA_TRUE;
}

static void
_class_constructor(Eo_Class *klass)
{
      const Eo_Op_Func_Description func_desc[] = {
           EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

           EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_hoversel_smart_add),
           EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_hoversel_smart_del),
           EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_SHOW), _elm_hoversel_smart_show),
           EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_HIDE), _elm_hoversel_smart_hide),

           EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_THEME_APPLY), _elm_hoversel_smart_theme),
           EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_TRANSLATE), _elm_hoversel_smart_translate),
           EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_PARENT_SET), _elm_hoversel_smart_parent_set),
           EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_EVENT), _elm_hoversel_smart_event),

           EO_OP_FUNC(ELM_OBJ_BUTTON_ID(ELM_OBJ_BUTTON_SUB_ID_ADMITS_AUTOREPEAT_GET), _elm_hoversel_smart_admits_autorepeat_get),

           EO_OP_FUNC(ELM_OBJ_HOVERSEL_ID(ELM_OBJ_HOVERSEL_SUB_ID_HOVER_PARENT_SET), _hover_parent_set),
           EO_OP_FUNC(ELM_OBJ_HOVERSEL_ID(ELM_OBJ_HOVERSEL_SUB_ID_HOVER_PARENT_GET), _hover_parent_get),
           EO_OP_FUNC(ELM_OBJ_HOVERSEL_ID(ELM_OBJ_HOVERSEL_SUB_ID_HORIZONTAL_SET), _horizontal_set),
           EO_OP_FUNC(ELM_OBJ_HOVERSEL_ID(ELM_OBJ_HOVERSEL_SUB_ID_HORIZONTAL_GET), _horizontal_get),
           EO_OP_FUNC(ELM_OBJ_HOVERSEL_ID(ELM_OBJ_HOVERSEL_SUB_ID_HOVER_BEGIN), _hover_begin),
           EO_OP_FUNC(ELM_OBJ_HOVERSEL_ID(ELM_OBJ_HOVERSEL_SUB_ID_HOVER_END), _hover_end),
           EO_OP_FUNC(ELM_OBJ_HOVERSEL_ID(ELM_OBJ_HOVERSEL_SUB_ID_EXPANDED_GET), _expanded_get),
           EO_OP_FUNC(ELM_OBJ_HOVERSEL_ID(ELM_OBJ_HOVERSEL_SUB_ID_CLEAR), _clear),
           EO_OP_FUNC(ELM_OBJ_HOVERSEL_ID(ELM_OBJ_HOVERSEL_SUB_ID_ITEMS_GET), _items_get),
           EO_OP_FUNC(ELM_OBJ_HOVERSEL_ID(ELM_OBJ_HOVERSEL_SUB_ID_ITEM_ADD), _item_add),
           EO_OP_FUNC_SENTINEL
      };
      eo_class_funcs_set(klass, func_desc);

      evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_HOVERSEL_SUB_ID_HOVER_PARENT_SET, "Set the Hover parent."),
     EO_OP_DESCRIPTION(ELM_OBJ_HOVERSEL_SUB_ID_HOVER_PARENT_GET, "Get the Hover parent."),
     EO_OP_DESCRIPTION(ELM_OBJ_HOVERSEL_SUB_ID_HORIZONTAL_SET, "This sets the hoversel to expand horizontally."),
     EO_OP_DESCRIPTION(ELM_OBJ_HOVERSEL_SUB_ID_HORIZONTAL_GET, "This returns whether the hoversel is set to expand horizontally."),
     EO_OP_DESCRIPTION(ELM_OBJ_HOVERSEL_SUB_ID_HOVER_BEGIN, "This triggers the hoversel popup from code, the same as if the user had clicked the button."),
     EO_OP_DESCRIPTION(ELM_OBJ_HOVERSEL_SUB_ID_HOVER_END, "This dismisses the hoversel popup as if the user had clicked outside the hover."),
     EO_OP_DESCRIPTION(ELM_OBJ_HOVERSEL_SUB_ID_EXPANDED_GET, "Returns whether the hoversel is expanded."),
     EO_OP_DESCRIPTION(ELM_OBJ_HOVERSEL_SUB_ID_CLEAR, "This will remove all the children items from the hoversel."),
     EO_OP_DESCRIPTION(ELM_OBJ_HOVERSEL_SUB_ID_ITEMS_GET, "Get the list of items within the given hoversel."),
     EO_OP_DESCRIPTION(ELM_OBJ_HOVERSEL_SUB_ID_ITEM_ADD, "Add an item to the hoversel button."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_HOVERSEL_BASE_ID, op_desc, ELM_OBJ_HOVERSEL_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Hoversel_Smart_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_obj_hoversel_class_get, &class_desc, ELM_OBJ_BUTTON_CLASS, EVAS_SMART_SELECTABLE_INTERFACE, NULL);
