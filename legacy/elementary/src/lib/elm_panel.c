#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_panel.h"

#include "els_box.h"

#define MY_CLASS ELM_OBJ_PANEL_CLASS

#define MY_CLASS_NAME "Elm_Panel"
#define MY_CLASS_NAME_LEGACY "elm_panel"

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};

static Eina_Bool _key_action_toggle(Evas_Object *obj, const char *params);

static const Elm_Action key_actions[] = {
   {"toggle", _key_action_toggle},
   {NULL, NULL}
};

static void
_mirrored_set(Evas_Object *obj,
              Eina_Bool rtl)
{
   ELM_PANEL_DATA_GET(obj, sd);

   if ((sd->content) && (eo_isa(sd->content, ELM_OBJ_WIDGET_CLASS)))
     elm_widget_mirrored_set(sd->content, rtl);
   elm_panel_orient_set(obj, elm_panel_orient_get(obj));
}

EOLIAN static void
_elm_panel_elm_layout_sizing_eval(Eo *obj, Elm_Panel_Data *sd)
{
   Evas_Coord mw = -1, mh = -1;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (sd->delete_me) return;

   evas_object_smart_calculate(sd->bx);
   edje_object_size_min_calc(wd->resize_obj, &mw, &mh);
   evas_object_size_hint_min_set(obj, mw, mh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static char *
_access_state_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   ELM_PANEL_DATA_GET(data, sd);

   if (!sd->hidden) return strdup(E_("state: opened"));
   else return strdup(E_("state: closed"));

   return NULL;
}

static void
_orient_set_do(Evas_Object *obj)
{
   ELM_PANEL_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   switch (sd->orient)
     {
      case ELM_PANEL_ORIENT_TOP:
        if (!elm_layout_theme_set
              (obj, "panel", "top", elm_widget_style_get(obj)))
          CRI("Failed to set layout!");
        break;

      case ELM_PANEL_ORIENT_BOTTOM:
        if (!elm_layout_theme_set
              (obj, "panel", "bottom", elm_widget_style_get(obj)))
          CRI("Failed to set layout!");
        break;

      case ELM_PANEL_ORIENT_LEFT:
        if (!elm_widget_mirrored_get(obj))
          {
             if (!elm_layout_theme_set
                   (obj, "panel", "left", elm_widget_style_get(obj)))
               CRI("Failed to set layout!");
          }
        else
          {
             if (!elm_layout_theme_set
                   (obj, "panel", "right", elm_widget_style_get(obj)))
               CRI("Failed to set layout!");
          }
        break;

      case ELM_PANEL_ORIENT_RIGHT:
        if (!elm_widget_mirrored_get(obj))
          {
             if (!elm_layout_theme_set
                   (obj, "panel", "right", elm_widget_style_get(obj)))
               CRI("Failed to set layout!");
          }
        else
          {
             if (!elm_layout_theme_set
                   (obj, "panel", "left", elm_widget_style_get(obj)))
               CRI("Failed to set layout!");
          }
        break;
     }

   /* access */
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     {
        Evas_Object *ao;
        ao = _elm_access_edje_object_part_object_register
            (obj, wd->resize_obj, "btn_icon");
        _elm_access_text_set(_elm_access_info_get(ao),
                             ELM_ACCESS_TYPE, E_("panel button"));
        _elm_access_callback_set
          (_elm_access_info_get(ao), ELM_ACCESS_STATE, _access_state_cb, obj);
     }
}

EOLIAN static Eina_Bool
_elm_panel_elm_widget_theme_apply(Eo *obj, Elm_Panel_Data *sd)
{
   const char *str;
   Evas_Coord minw = 0, minh = 0;

   Eina_Bool int_ret = EINA_FALSE;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_theme_apply());
   if (!int_ret) return EINA_FALSE;

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   str = edje_object_data_get
       (wd->resize_obj, "focus_highlight");
   if ((str) && (!strcmp(str, "on")))
     elm_widget_highlight_in_theme_set(obj, EINA_TRUE);
   else
     elm_widget_highlight_in_theme_set(obj, EINA_FALSE);

   _orient_set_do(obj);

   evas_object_hide(sd->event);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(sd->event, minw, minh);

   if (edje_object_part_exists
       (wd->resize_obj, "elm.swallow.event"))
     elm_layout_content_set(obj, "elm.swallow.event", sd->event);

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_panel_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Elm_Panel_Data *_pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_panel_elm_widget_focus_next(Eo *obj, Elm_Panel_Data *sd, Elm_Focus_Direction dir, Evas_Object **next)
{
   Evas_Object *cur;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   if (!sd->content) return EINA_FALSE;

   cur = sd->content;

   /* Try to Focus cycle in subitem */
   if (!sd->hidden) return elm_widget_focus_next_get(cur, dir, next);

   /* access */
   if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
     {
        Evas_Object *ao, *po;
        po = (Evas_Object *)edje_object_part_object_get
               (wd->resize_obj, "btn_icon");
        ao = evas_object_data_get(po, "_part_access_obj");
        _elm_access_highlight_set(ao);
     }

   /* Return */
   *next = (Evas_Object *)obj;
   return !elm_widget_focus_get(obj);
}

static void
_box_layout_cb(Evas_Object *o,
               Evas_Object_Box_Data *priv,
               void *data EINA_UNUSED)
{
   _els_box_layout(o, priv, EINA_TRUE, EINA_FALSE, EINA_FALSE);
}

static void
_panel_toggle(void *data EINA_UNUSED,
              Evas_Object *obj,
              const char *emission EINA_UNUSED,
              const char *source EINA_UNUSED)
{
   ELM_PANEL_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (sd->hidden)
     {
        elm_layout_signal_emit(obj, "elm,action,show", "elm");
        sd->hidden = EINA_FALSE;
        evas_object_repeat_events_set(obj, EINA_FALSE);
     }
   else
     {
        elm_layout_signal_emit(obj, "elm,action,hide", "elm");
        sd->hidden = EINA_TRUE;
        evas_object_repeat_events_set(obj, EINA_TRUE);
        if (sd->content && elm_widget_focus_get(sd->content))
          {
             elm_widget_focused_object_clear(obj);
             elm_widget_focus_steal(obj);
          }
     }

   edje_object_message_signal_process(wd->resize_obj);
}

static Eina_Bool
_key_action_toggle(Evas_Object *obj, const char *params EINA_UNUSED)
{
   _panel_toggle(NULL, obj, NULL, NULL);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_panel_elm_widget_event(Eo *obj, Elm_Panel_Data *_pd EINA_UNUSED, Evas_Object *src, Evas_Callback_Type type, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (src != obj) return EINA_FALSE;

   if (!_elm_config_key_binding_call(obj, ev, key_actions)) return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_panel_elm_container_content_set(Eo *obj, Elm_Panel_Data *sd, const char *part, Evas_Object *content)
{
   if (part && strcmp(part, "default"))
     {
        Eina_Bool int_ret = EINA_TRUE;
        eo_do_super(obj, MY_CLASS, int_ret = elm_obj_container_content_set(part, content));
        return int_ret;
     }

   if (sd->content == content) return EINA_TRUE;
   if (sd->content)
     evas_object_box_remove_all(sd->bx, EINA_TRUE);
   sd->content = content;
   if (content)
     {
        evas_object_box_append(sd->bx, sd->content);
        evas_object_show(sd->content);
     }

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

EOLIAN static Evas_Object*
_elm_panel_elm_container_content_get(Eo *obj, Elm_Panel_Data *sd, const char *part)
{
   if (part && strcmp(part, "default"))
     {
        Evas_Object *ret = NULL;
        eo_do_super(obj, MY_CLASS, ret = elm_obj_container_content_get(part));
        return ret;
     }

   return sd->content;
}

EOLIAN static Evas_Object*
_elm_panel_elm_container_content_unset(Eo *obj, Elm_Panel_Data *sd, const char *part)
{
   Evas_Object *ret = NULL;

   if (part && strcmp(part, "default"))
     {
        eo_do_super(obj, MY_CLASS, ret = elm_obj_container_content_unset(part));
        return ret;
     }

   if (!sd->content) return NULL;
   ret = sd->content;

   evas_object_box_remove_all(sd->bx, EINA_FALSE);
   sd->content = NULL;

   return ret;
}

EOLIAN static void
_elm_panel_evas_smart_add(Eo *obj, Elm_Panel_Data *priv)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);
   elm_widget_can_focus_set(obj, EINA_TRUE);

   eo_do(obj, elm_obj_widget_theme_apply());

   priv->bx = evas_object_box_add(evas_object_evas_get(obj));
   evas_object_size_hint_align_set(priv->bx, 0.5, 0.5);
   evas_object_box_layout_set(priv->bx, _box_layout_cb, priv, NULL);
   evas_object_show(priv->bx);

   elm_layout_signal_callback_add
     (obj, "elm,action,panel,toggle", "*", _panel_toggle, obj);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   priv->event = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_color_set(priv->event, 0, 0, 0, 0);
   evas_object_pass_events_set(priv->event, EINA_TRUE);
   elm_widget_sub_object_add(obj, priv->event);

   /* just to bootstrap and have theme hook to work */
   if (!elm_layout_theme_set(obj, "panel", "top", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");
   else
     {
        elm_layout_content_set(obj, "elm.swallow.content", priv->bx);

        if (edje_object_part_exists
            (wd->resize_obj, "elm.swallow.event"))
          {
             Evas_Coord minw = 0, minh = 0;

             elm_coords_finger_size_adjust(1, &minw, 1, &minh);
             evas_object_size_hint_min_set(priv->event, minw, minh);
             elm_layout_content_set(obj, "elm.swallow.event", priv->event);
          }
     }

   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_elm_panel_evas_smart_del(Eo *obj, Elm_Panel_Data *sd)
{
   Evas_Object *child;
   Eina_List *l;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   sd->delete_me = EINA_TRUE;

   /* let's make our box object the *last* to be processed, since it
    * may (smart) parent other sub objects here */
   EINA_LIST_FOREACH(wd->subobjs, l, child)
     {
        if (child == sd->bx)
          {
             wd->subobjs =
               eina_list_demote_list(wd->subobjs, l);
             break;
          }
     }

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EOLIAN static void
_elm_panel_elm_widget_access(Eo *obj, Elm_Panel_Data *_pd EINA_UNUSED, Eina_Bool is_access)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (is_access)
     _elm_access_edje_object_part_object_register
       (obj, wd->resize_obj, "btn_icon");
   else
     _elm_access_edje_object_part_object_unregister
       (obj, wd->resize_obj, "btn_icon");
}

EAPI Evas_Object *
elm_panel_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

EOLIAN static void
_elm_panel_eo_base_constructor(Eo *obj, Elm_Panel_Data *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks));
}

EOLIAN static void
_elm_panel_orient_set(Eo *obj, Elm_Panel_Data *sd, Elm_Panel_Orient orient)
{
   if (sd->orient == orient) return;
   sd->orient = orient;

   _orient_set_do(obj);

   elm_layout_sizing_eval(obj);
}

EOLIAN static Elm_Panel_Orient
_elm_panel_orient_get(Eo *obj EINA_UNUSED, Elm_Panel_Data *sd)
{
   return sd->orient;
}

EOLIAN static void
_elm_panel_hidden_set(Eo *obj, Elm_Panel_Data *sd, Eina_Bool hidden)
{
   if (sd->hidden == !!hidden) return;

   _panel_toggle(NULL, obj, NULL, NULL);
}

EOLIAN static Eina_Bool
_elm_panel_hidden_get(Eo *obj EINA_UNUSED, Elm_Panel_Data *sd)
{
   return sd->hidden;
}

EOLIAN static void
_elm_panel_toggle(Eo *obj, Elm_Panel_Data *_pd EINA_UNUSED)
{
   _panel_toggle(NULL, obj, NULL, NULL);
}

static void
_elm_panel_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#include "elm_panel.eo.c"
