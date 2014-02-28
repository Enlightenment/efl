#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_panel.h"

EAPI Eo_Op ELM_OBJ_PANEL_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_PANEL_CLASS

#define MY_CLASS_NAME "Elm_Panel"
#define MY_CLASS_NAME_LEGACY "elm_panel"

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};

void _key_action_toggle(Evas_Object *obj, const char *params);

static const Elm_Action key_actions[] = {
   {"toggle", _key_action_toggle},
   {NULL, NULL}
};

static void
_mirrored_set(Evas_Object *obj,
              Eina_Bool rtl)
{
   ELM_PANEL_DATA_GET(obj, sd);

   elm_widget_mirrored_set(sd->bx, rtl);
   elm_panel_orient_set(obj, elm_panel_orient_get(obj));
}

static void
_elm_panel_smart_sizing_eval(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Coord mw = -1, mh = -1;

   Elm_Panel_Smart_Data *sd = _pd;
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

static void
_elm_panel_smart_theme(Eo *obj, void *_pd, va_list *list)
{
   const char *str;
   Evas_Coord minw = 0, minh = 0;
   Elm_Panel_Smart_Data *sd = _pd;

   Eina_Bool int_ret = EINA_FALSE;
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, elm_wdg_theme_apply(&int_ret));
   if (!int_ret) return;

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

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_panel_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
}

static void
_elm_panel_smart_focus_next(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *cur;
   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Evas_Object **next =  va_arg(*list, Evas_Object **);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Eina_Bool int_ret = EINA_FALSE;

   Elm_Panel_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (!sd->content) goto end;

   cur = sd->content;

   /* Try to Focus cycle in subitem */
   if (!sd->hidden)
     {
        int_ret = elm_widget_focus_next_get(cur, dir, next);
        goto end;
     }

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
   int_ret = !elm_widget_focus_get(obj);

end:
   if (ret) *ret = int_ret;
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

void _key_action_toggle(Evas_Object *obj, const char *params EINA_UNUSED)
{
   _panel_toggle(NULL, obj, NULL, NULL);
}

static void
_elm_panel_smart_event(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *src = va_arg(*list, Evas_Object *);
   Evas_Callback_Type type = va_arg(*list, Evas_Callback_Type);
   Evas_Event_Key_Down *ev = va_arg(*list, void *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);

   if (ret) *ret = EINA_FALSE;

   if (elm_widget_disabled_get(obj)) return;
   if (type != EVAS_CALLBACK_KEY_DOWN) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (src != obj) return;

   _elm_config_key_binding_call(obj, ev, key_actions);

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   if (ret) *ret = EINA_TRUE;
}

static void
_elm_panel_smart_content_set(Eo *obj, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Evas_Object *content = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Eina_Bool int_ret = EINA_TRUE;

   Elm_Panel_Smart_Data *sd = _pd;

   if (part && strcmp(part, "default"))
     {
        eo_do_super(obj, MY_CLASS, elm_obj_container_content_set(part, content, &int_ret));
        goto end;
     }

   if (sd->content == content) goto end;
   if (sd->content)
     evas_object_box_remove_all(sd->bx, EINA_TRUE);
   sd->content = content;
   if (content)
     {
        evas_object_box_append(sd->bx, sd->content);
        evas_object_show(sd->content);
     }

   elm_layout_sizing_eval(obj);

end:
   if (ret) *ret = int_ret;

}

static void
_elm_panel_smart_content_get(Eo *obj, void *_pd, va_list *list)
{
   Elm_Panel_Smart_Data *sd = _pd;
   const char *part = va_arg(*list, const char *);
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   if (!ret) return;

   *ret = NULL;

   if (part && strcmp(part, "default"))
     {
        eo_do_super(obj, MY_CLASS, elm_obj_container_content_get(part, ret));
        return;
     }

   *ret = sd->content;
}

static void
_elm_panel_smart_content_unset(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *content = NULL;

   Elm_Panel_Smart_Data *sd = _pd;
   const char *part = va_arg(*list, const char *);
   Evas_Object **ret = va_arg(*list, Evas_Object **);

   if (part && strcmp(part, "default"))
     {
        eo_do_super(obj, MY_CLASS, elm_obj_container_content_unset(part, &content));
        goto end;
     }

   if (!sd->content) goto end;
   content = sd->content;

   evas_object_box_remove_all(sd->bx, EINA_FALSE);
   sd->content = NULL;

end:
   if (ret) *ret = content;
}

static void
_elm_panel_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Panel_Smart_Data *priv = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);
   elm_widget_can_focus_set(obj, EINA_TRUE);

   eo_do(obj, elm_wdg_theme_apply(NULL));

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

static void
_elm_panel_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Object *child;
   Eina_List *l;

   Elm_Panel_Smart_Data *sd = _pd;
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

static void
_elm_panel_smart_access(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   Eina_Bool is_access = va_arg(*list, int);
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

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));
}

EAPI void
elm_panel_orient_set(Evas_Object *obj,
                     Elm_Panel_Orient orient)
{
   ELM_PANEL_CHECK(obj);
   eo_do(obj, elm_obj_panel_orient_set(orient));
}

static void
_orient_set(Eo *obj, void *_pd, va_list *list)
{
   Elm_Panel_Orient orient = va_arg(*list, Elm_Panel_Orient);
   Elm_Panel_Smart_Data *sd = _pd;

   if (sd->orient == orient) return;
   sd->orient = orient;

   _orient_set_do(obj);

   elm_layout_sizing_eval(obj);
}

EAPI Elm_Panel_Orient
elm_panel_orient_get(const Evas_Object *obj)
{
   ELM_PANEL_CHECK(obj) ELM_PANEL_ORIENT_LEFT;
   Elm_Panel_Orient ret = ELM_PANEL_ORIENT_LEFT;
   eo_do((Eo *) obj, elm_obj_panel_orient_get(&ret));
   return ret;
}

static void
_orient_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Panel_Orient *ret = va_arg(*list, Elm_Panel_Orient *);
   Elm_Panel_Smart_Data *sd = _pd;

   if (ret) *ret = sd->orient;
}

EAPI void
elm_panel_hidden_set(Evas_Object *obj,
                     Eina_Bool hidden)
{
   ELM_PANEL_CHECK(obj);
   eo_do(obj, elm_obj_panel_hidden_set(hidden));
}

static void
_hidden_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool hidden = va_arg(*list, int);
   Elm_Panel_Smart_Data *sd = _pd;

   if (sd->hidden == !!hidden) return;

   _panel_toggle(NULL, obj, NULL, NULL);
}

EAPI Eina_Bool
elm_panel_hidden_get(const Evas_Object *obj)
{
   ELM_PANEL_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_panel_hidden_get(&ret));
   return ret;
}

static void
_hidden_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Panel_Smart_Data *sd = _pd;

   if (ret) *ret = sd->hidden;
}

EAPI void
elm_panel_toggle(Evas_Object *obj)
{
   ELM_PANEL_CHECK(obj);
   eo_do(obj, elm_obj_panel_toggle());
}

static void
_toggle(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   _panel_toggle(NULL, obj, NULL, NULL);
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_panel_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_panel_smart_del),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_panel_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_NEXT), _elm_panel_smart_focus_next),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_THEME_APPLY), _elm_panel_smart_theme),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_EVENT), _elm_panel_smart_event),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_ACCESS), _elm_panel_smart_access),

        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_SET), _elm_panel_smart_content_set),
        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_GET), _elm_panel_smart_content_get),
        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_UNSET), _elm_panel_smart_content_unset),

        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_panel_smart_sizing_eval),

        EO_OP_FUNC(ELM_OBJ_PANEL_ID(ELM_OBJ_PANEL_SUB_ID_ORIENT_SET), _orient_set),
        EO_OP_FUNC(ELM_OBJ_PANEL_ID(ELM_OBJ_PANEL_SUB_ID_ORIENT_GET), _orient_get),
        EO_OP_FUNC(ELM_OBJ_PANEL_ID(ELM_OBJ_PANEL_SUB_ID_HIDDEN_SET), _hidden_set),
        EO_OP_FUNC(ELM_OBJ_PANEL_ID(ELM_OBJ_PANEL_SUB_ID_HIDDEN_GET), _hidden_get),
        EO_OP_FUNC(ELM_OBJ_PANEL_ID(ELM_OBJ_PANEL_SUB_ID_TOGGLE), _toggle),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}
static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_PANEL_SUB_ID_ORIENT_SET, "Sets the orientation of the panel."),
     EO_OP_DESCRIPTION(ELM_OBJ_PANEL_SUB_ID_ORIENT_GET, "Get the orientation of the panel."),
     EO_OP_DESCRIPTION(ELM_OBJ_PANEL_SUB_ID_HIDDEN_SET, "Set the state of the panel."),
     EO_OP_DESCRIPTION(ELM_OBJ_PANEL_SUB_ID_HIDDEN_GET, "Get the state of the panel."),
     EO_OP_DESCRIPTION(ELM_OBJ_PANEL_SUB_ID_TOGGLE, "Toggle the hidden state of the panel from code."),
     EO_OP_DESCRIPTION_SENTINEL
};
static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_PANEL_BASE_ID, op_desc, ELM_OBJ_PANEL_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Panel_Smart_Data),
     _class_constructor,
     NULL
};
EO_DEFINE_CLASS(elm_obj_panel_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, NULL);
