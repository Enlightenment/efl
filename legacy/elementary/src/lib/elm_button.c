#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_button.h"
#include "elm_widget_layout.h"

EAPI Eo_Op ELM_OBJ_BUTTON_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_BUTTON_CLASS

#define MY_CLASS_NAME "Elm_Button"
#define MY_CLASS_NAME_LEGACY "elm_button"

static const char SIG_CLICKED[] = "clicked";
static const char SIG_REPEATED[] = "repeated";
static const char SIG_PRESSED[] = "pressed";
static const char SIG_UNPRESSED[] = "unpressed";

/* smart callbacks coming from elm button objects (besides the ones
 * coming from elm layout): */
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CLICKED, ""},
   {SIG_REPEATED, ""},
   {SIG_PRESSED, ""},
   {SIG_UNPRESSED, ""},
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"icon", "elm.swallow.content"},
   {NULL, NULL}
};

static const Elm_Layout_Part_Alias_Description _text_aliases[] =
{
   {"default", "elm.text"},
   {NULL, NULL}
};

static void
_activate(Evas_Object *obj)
{
   ELM_BUTTON_DATA_GET_OR_RETURN(obj, sd);

   ELM_SAFE_FREE(sd->timer, ecore_timer_del);
   sd->repeating = EINA_FALSE;

   if ((_elm_config->access_mode == ELM_ACCESS_MODE_OFF) ||
       (_elm_access_2nd_click_timeout(obj)))
     {
        if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
          _elm_access_say(E_("Clicked"));
        if (!elm_widget_disabled_get(obj) &&
            !evas_object_freeze_events_get(obj))
          evas_object_smart_callback_call(obj, SIG_CLICKED, NULL);
     }
}

static void
_elm_button_smart_sizing_eval(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Evas_Coord minw = -1, minh = -1;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
}

static void
_elm_button_smart_activate(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Activate act = va_arg(*list, Elm_Activate);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   if (elm_widget_disabled_get(obj)) return;
   if (act != ELM_ACTIVATE_DEFAULT) return;
   if (evas_object_freeze_events_get(obj)) return;

   evas_object_smart_callback_call(obj, SIG_CLICKED, NULL);
   elm_layout_signal_emit(obj, "elm,anim,activate", "elm");

   if (ret) *ret = EINA_TRUE;
}

/* FIXME: replicated from elm_layout just because button's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static void
_icon_signal_emit(Evas_Object *obj)
{
   char buf[64];

   snprintf(buf, sizeof(buf), "elm,state,icon,%s",
            elm_layout_content_get(obj, "icon") ? "visible" : "hidden");

   elm_layout_signal_emit(obj, buf, "elm");
   edje_object_message_signal_process(elm_layout_edje_get(obj));
   eo_do(obj, elm_obj_layout_sizing_eval());
}

/* FIXME: replicated from elm_layout just because button's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static void
_elm_button_smart_theme(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_theme_apply(&int_ret));
   if (!int_ret) return;
   _icon_signal_emit(obj);

   if (ret) *ret = EINA_TRUE;
}

/* FIXME: replicated from elm_layout just because button's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static void
_elm_button_smart_sub_object_del(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *sobj = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_sub_object_del(sobj, &int_ret));
   if (!int_ret) return;

   _icon_signal_emit(obj);

   if (ret) *ret = EINA_TRUE;
}

/* FIXME: replicated from elm_layout just because button's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static void
_elm_button_smart_content_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Evas_Object *content = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, elm_obj_container_content_set(part, content, &int_ret));
   if (!int_ret) return;

   _icon_signal_emit(obj);

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_button_smart_event(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *src = va_arg(*list, Evas_Object *);
   Evas_Callback_Type type = va_arg(*list, Evas_Callback_Type);
   Evas_Event_Key_Down *ev = va_arg(*list, void *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);

   if (ret) *ret = EINA_FALSE;
   (void) src;

   if (elm_widget_disabled_get(obj)) return;
   if (type != EVAS_CALLBACK_KEY_DOWN) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   if ((strcmp(ev->key, "Return")) &&
       (strcmp(ev->key, "KP_Enter")) &&
       (strcmp(ev->key, "space")))
     return;

   elm_layout_signal_emit(obj, "elm,anim,activate", "elm");
   _activate(obj);

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   if (ret) *ret = EINA_TRUE;
}

static void
_on_clicked_signal(void *data,
                   Evas_Object *obj EINA_UNUSED,
                   const char *emission EINA_UNUSED,
                   const char *source EINA_UNUSED)
{
   _activate(data);
}

static Eina_Bool
_autorepeat_send(void *data)
{
   ELM_BUTTON_DATA_GET_OR_RETURN_VAL(data, sd, ECORE_CALLBACK_CANCEL);

   evas_object_smart_callback_call(data, SIG_REPEATED, NULL);
   if (!sd->repeating)
     {
        sd->timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_autorepeat_initial_send(void *data)
{
   ELM_BUTTON_DATA_GET_OR_RETURN_VAL(data, sd, ECORE_CALLBACK_CANCEL);

   ELM_SAFE_FREE(sd->timer, ecore_timer_del);
   sd->repeating = EINA_TRUE;
   _autorepeat_send(data);
   sd->timer = ecore_timer_add(sd->ar_interval, _autorepeat_send, data);

   return ECORE_CALLBACK_CANCEL;
}

static void
_on_pressed_signal(void *data,
                   Evas_Object *obj EINA_UNUSED,
                   const char *emission EINA_UNUSED,
                   const char *source EINA_UNUSED)
{
   ELM_BUTTON_DATA_GET_OR_RETURN(data, sd);

   if ((sd->autorepeat) && (!sd->repeating))
     {
        if (sd->ar_threshold <= 0.0)
          _autorepeat_initial_send(data);  /* call immediately */
        else
          sd->timer = ecore_timer_add
              (sd->ar_threshold, _autorepeat_initial_send, data);
     }

   evas_object_smart_callback_call(data, SIG_PRESSED, NULL);
}

static void
_on_unpressed_signal(void *data,
                     Evas_Object *obj EINA_UNUSED,
                     const char *emission EINA_UNUSED,
                     const char *source EINA_UNUSED)
{
   ELM_BUTTON_DATA_GET_OR_RETURN(data, sd);

   ELM_SAFE_FREE(sd->timer, ecore_timer_del);
   sd->repeating = EINA_FALSE;
   evas_object_smart_callback_call(data, SIG_UNPRESSED, NULL);
}

static char *
_access_info_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
   const char *txt = elm_widget_access_info_get(obj);

   if (!txt) txt = elm_layout_text_get(obj, NULL);
   if (txt) return strdup(txt);

   return NULL;
}

static char *
_access_state_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
   if (elm_widget_disabled_get(obj))
     return strdup(E_("State: Disabled"));

   return NULL;
}

static void
_elm_button_smart_add(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,click", "*",
     _on_clicked_signal, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,press", "*",
     _on_pressed_signal, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,unpress", "*",
     _on_unpressed_signal, obj);

   _elm_access_object_register(obj, wd->resize_obj);
   _elm_access_text_set
     (_elm_access_info_get(obj), ELM_ACCESS_TYPE, E_("Button"));
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_INFO, _access_info_cb, NULL);
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_STATE, _access_state_cb, obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);
   if (!elm_layout_theme_set(obj, "button", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");
}

static void
_elm_button_smart_content_aliases_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   const Elm_Layout_Part_Alias_Description **aliases = va_arg(*list, const Elm_Layout_Part_Alias_Description **);
   *aliases = _content_aliases;
}

static void
_elm_button_smart_text_aliases_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   const Elm_Layout_Part_Alias_Description **aliases = va_arg(*list, const Elm_Layout_Part_Alias_Description **);
   *aliases = _text_aliases;
}

EAPI Evas_Object *
elm_button_add(Evas_Object *parent)
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
elm_button_autorepeat_set(Evas_Object *obj,
                          Eina_Bool on)
{
   ELM_BUTTON_CHECK(obj);
   eo_do(obj, elm_obj_button_autorepeat_set(on));
}

static void
_autorepeat_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool on = va_arg(*list, int);
   Elm_Button_Smart_Data *sd = _pd;

   ELM_SAFE_FREE(sd->timer, ecore_timer_del);
   sd->autorepeat = on;
   sd->repeating = EINA_FALSE;
}

#define _AR_CAPABLE(obj) \
  (_elm_button_admits_autorepeat_get(obj))

static Eina_Bool
_elm_button_admits_autorepeat_get(const Evas_Object *obj)
{
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_button_admits_autorepeat_get(&ret));
   return ret;
}

static void
_admits_autorepeat_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
}

EAPI Eina_Bool
elm_button_autorepeat_get(const Evas_Object *obj)
{
   ELM_BUTTON_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_button_autorepeat_get(&ret));
   return ret;
}

static void
_autorepeat_get(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Button_Smart_Data *sd = _pd;
   *ret = _AR_CAPABLE(obj) & sd->autorepeat;
}

EAPI void
elm_button_autorepeat_initial_timeout_set(Evas_Object *obj,
                                          double t)
{
   ELM_BUTTON_CHECK(obj);
   eo_do(obj, elm_obj_button_autorepeat_initial_timeout_set(t));
}

static void
_autorepeat_initial_timeout_set(Eo *obj, void *_pd, va_list *list)
{
   double t = va_arg(*list, double);
   Elm_Button_Smart_Data *sd = _pd;

   if (!_AR_CAPABLE(obj))
     {
        ERR("this widget does not support auto repetition of clicks.");
        return;
     }

   if (sd->ar_threshold == t) return;
   ELM_SAFE_FREE(sd->timer, ecore_timer_del);
   sd->ar_threshold = t;
}

EAPI double
elm_button_autorepeat_initial_timeout_get(const Evas_Object *obj)
{
   ELM_BUTTON_CHECK(obj) 0.0;
    double ret = 0.0;
    eo_do((Eo *) obj, elm_obj_button_autorepeat_initial_timeout_get(&ret));
    return ret;
}

static void
_autorepeat_initial_timeout_get(Eo *obj, void *_pd, va_list *list)
{
   double *ret = va_arg(*list, double *);
   Elm_Button_Smart_Data *sd = _pd;

   if (!_AR_CAPABLE(obj))
      *ret = 0.0;
   else
      *ret = sd->ar_threshold;
}

EAPI void
elm_button_autorepeat_gap_timeout_set(Evas_Object *obj,
                                      double t)
{
   ELM_BUTTON_CHECK(obj);
   eo_do(obj, elm_obj_button_autorepeat_gap_timeout_set(t));
}

static void
_autorepeat_gap_timeout_set(Eo *obj, void *_pd, va_list *list)
{
   double t = va_arg(*list, double);
   Elm_Button_Smart_Data *sd = _pd;

   if (!_AR_CAPABLE(obj))
     {
        ERR("this widget does not support auto repetition of clicks.");
        return;
     }

   if (sd->ar_interval == t) return;

   sd->ar_interval = t;
   if ((sd->repeating) && (sd->timer)) ecore_timer_interval_set(sd->timer, t);
}

EAPI double
elm_button_autorepeat_gap_timeout_get(const Evas_Object *obj)
{
   ELM_BUTTON_CHECK(obj) 0.0;
   double ret = 0.0;
   eo_do((Eo *) obj, elm_obj_button_autorepeat_gap_timeout_get(&ret));
   return ret;
}

static void
_autorepeat_gap_timeout_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double *ret = va_arg(*list, double *);
   Elm_Button_Smart_Data *sd = _pd;

   *ret = sd->ar_interval;
}

static void
_elm_button_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_elm_button_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_button_smart_add),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_THEME_APPLY), _elm_button_smart_theme),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_EVENT), _elm_button_smart_event),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_SUB_OBJECT_DEL), _elm_button_smart_sub_object_del),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_ACTIVATE), _elm_button_smart_activate),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_button_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_button_smart_focus_direction_manager_is),

        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_SET), _elm_button_smart_content_set),

        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_button_smart_sizing_eval),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_CONTENT_ALIASES_GET), _elm_button_smart_content_aliases_get),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TEXT_ALIASES_GET), _elm_button_smart_text_aliases_get),

        EO_OP_FUNC(ELM_OBJ_BUTTON_ID(ELM_OBJ_BUTTON_SUB_ID_ADMITS_AUTOREPEAT_GET), _admits_autorepeat_get),
        EO_OP_FUNC(ELM_OBJ_BUTTON_ID(ELM_OBJ_BUTTON_SUB_ID_AUTOREPEAT_SET), _autorepeat_set),
        EO_OP_FUNC(ELM_OBJ_BUTTON_ID(ELM_OBJ_BUTTON_SUB_ID_AUTOREPEAT_GET), _autorepeat_get),
        EO_OP_FUNC(ELM_OBJ_BUTTON_ID(ELM_OBJ_BUTTON_SUB_ID_AUTOREPEAT_INITIAL_TIMEOUT_SET), _autorepeat_initial_timeout_set),
        EO_OP_FUNC(ELM_OBJ_BUTTON_ID(ELM_OBJ_BUTTON_SUB_ID_AUTOREPEAT_INITIAL_TIMEOUT_GET), _autorepeat_initial_timeout_get),
        EO_OP_FUNC(ELM_OBJ_BUTTON_ID(ELM_OBJ_BUTTON_SUB_ID_AUTOREPEAT_GAP_TIMEOUT_SET), _autorepeat_gap_timeout_set),
        EO_OP_FUNC(ELM_OBJ_BUTTON_ID(ELM_OBJ_BUTTON_SUB_ID_AUTOREPEAT_GAP_TIMEOUT_GET), _autorepeat_gap_timeout_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_BUTTON_SUB_ID_ADMITS_AUTOREPEAT_GET, "Get whether auto-repetition is implemented or not"),
     EO_OP_DESCRIPTION(ELM_OBJ_BUTTON_SUB_ID_AUTOREPEAT_SET, "Turn on/off the autorepeat event generated when the button is kept pressed."),
     EO_OP_DESCRIPTION(ELM_OBJ_BUTTON_SUB_ID_AUTOREPEAT_GET, "Get whether the autorepeat feature is enabled."),
     EO_OP_DESCRIPTION(ELM_OBJ_BUTTON_SUB_ID_AUTOREPEAT_INITIAL_TIMEOUT_SET, "Set the initial timeout before the autorepeat event is generated."),
     EO_OP_DESCRIPTION(ELM_OBJ_BUTTON_SUB_ID_AUTOREPEAT_INITIAL_TIMEOUT_GET, "Get the initial timeout before the autorepeat event is generated."),
     EO_OP_DESCRIPTION(ELM_OBJ_BUTTON_SUB_ID_AUTOREPEAT_GAP_TIMEOUT_SET, "Set the interval between each generated autorepeat event."),
     EO_OP_DESCRIPTION(ELM_OBJ_BUTTON_SUB_ID_AUTOREPEAT_GAP_TIMEOUT_GET, "Get the interval between each generated autorepeat event."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_BUTTON_BASE_ID, op_desc, ELM_OBJ_BUTTON_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Button_Smart_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_obj_button_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, EVAS_SMART_CLICKABLE_INTERFACE, NULL);
