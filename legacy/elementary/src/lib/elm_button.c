#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_button.h"

EAPI const char ELM_BUTTON_SMART_NAME[] = "elm_button";

static const char SIG_CLICKED[] = "clicked";
static const char SIG_REPEATED[] = "repeated";
static const char SIG_PRESSED[] = "pressed";
static const char SIG_UNPRESSED[] = "unpressed";

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

/* smart callbacks coming from elm button objects (besides the ones
 * coming from elm layout): */
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CLICKED, ""},
   {SIG_REPEATED, ""},
   {SIG_PRESSED, ""},
   {SIG_UNPRESSED, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_BUTTON_SMART_NAME, _elm_button, Elm_Button_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, _smart_callbacks);

static void
_activate(Evas_Object *obj)
{
   ELM_BUTTON_DATA_GET_OR_RETURN(obj, sd);

   if (sd->timer)
     {
        ecore_timer_del(sd->timer);
        sd->timer = NULL;
     }

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
_elm_button_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1;

   ELM_BUTTON_DATA_GET(obj, sd);

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (ELM_WIDGET_DATA(sd)->resize_obj, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
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
   _elm_button_smart_sizing_eval(obj);
}

/* FIXME: replicated from elm_layout just because button's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static Eina_Bool
_elm_button_smart_theme(Evas_Object *obj)
{
   if (!ELM_WIDGET_CLASS(_elm_button_parent_sc)->theme(obj)) return EINA_FALSE;

   _icon_signal_emit(obj);

   return EINA_TRUE;
}

/* FIXME: replicated from elm_layout just because button's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static Eina_Bool
_elm_button_smart_sub_object_del(Evas_Object *obj,
                                 Evas_Object *sobj)
{
   if (!ELM_WIDGET_CLASS(_elm_button_parent_sc)->sub_object_del(obj, sobj))
     return EINA_FALSE;

   _icon_signal_emit(obj);

   return EINA_TRUE;
}

/* FIXME: replicated from elm_layout just because button's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static Eina_Bool
_elm_button_smart_content_set(Evas_Object *obj,
                              const char *part,
                              Evas_Object *content)
{
   if (!ELM_CONTAINER_CLASS(_elm_button_parent_sc)->content_set
         (obj, part, content))
     return EINA_FALSE;

   _icon_signal_emit(obj);

   return EINA_TRUE;
}

static Eina_Bool
_elm_button_smart_event(Evas_Object *obj,
                        Evas_Object *src __UNUSED__,
                        Evas_Callback_Type type,
                        void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;

   if (elm_widget_disabled_get(obj)) return EINA_FALSE;

   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

   if ((strcmp(ev->keyname, "Return")) &&
       (strcmp(ev->keyname, "KP_Enter")) &&
       (strcmp(ev->keyname, "space")))
     return EINA_FALSE;

   _activate(obj);
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   elm_layout_signal_emit(obj, "elm,anim,activate", "elm");

   return EINA_TRUE;
}

static void
_on_clicked_signal(void *data,
                   Evas_Object *obj __UNUSED__,
                   const char *emission __UNUSED__,
                   const char *source __UNUSED__)
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

   if (sd->timer) ecore_timer_del(sd->timer);
   sd->repeating = EINA_TRUE;
   _autorepeat_send(data);
   sd->timer = ecore_timer_add(sd->ar_interval, _autorepeat_send, data);

   return ECORE_CALLBACK_CANCEL;
}

static void
_on_pressed_signal(void *data,
                   Evas_Object *obj __UNUSED__,
                   const char *emission __UNUSED__,
                   const char *source __UNUSED__)
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
                     Evas_Object *obj __UNUSED__,
                     const char *emission __UNUSED__,
                     const char *source __UNUSED__)
{
   ELM_BUTTON_DATA_GET_OR_RETURN(data, sd);

   if (sd->timer)
     {
        ecore_timer_del(sd->timer);
        sd->timer = NULL;
     }
   sd->repeating = EINA_FALSE;
   evas_object_smart_callback_call(data, SIG_UNPRESSED, NULL);
}

static char *
_access_info_cb(void *data __UNUSED__,
                Evas_Object *obj,
                Elm_Widget_Item *item __UNUSED__)
{
   const char *txt = elm_widget_access_info_get(obj);

   if (!txt) txt = elm_layout_text_get(obj, NULL);
   if (txt) return strdup(txt);

   return NULL;
}

static char *
_access_state_cb(void *data __UNUSED__,
                 Evas_Object *obj,
                 Elm_Widget_Item *item __UNUSED__)
{
   if (elm_widget_disabled_get(obj))
     return strdup(E_("State: Disabled"));

   return NULL;
}

static void
_access_activate_cb(Evas_Object *obj, void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_CLICKED, NULL);
   elm_layout_signal_emit(obj, "elm,anim,activate", "elm");
}

static void
_elm_button_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Button_Smart_Data);

   ELM_WIDGET_CLASS(_elm_button_parent_sc)->base.add(obj);

   edje_object_signal_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, "elm,action,click", "",
     _on_clicked_signal, obj);
   edje_object_signal_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, "elm,action,press", "",
     _on_pressed_signal, obj);
   edje_object_signal_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, "elm,action,unpress", "",
     _on_unpressed_signal, obj);

   _elm_access_object_register(obj, ELM_WIDGET_DATA(priv)->resize_obj);
   _elm_access_text_set
     (_elm_access_object_get(obj), ELM_ACCESS_TYPE, E_("Button"));
   _elm_access_callback_set
     (_elm_access_object_get(obj), ELM_ACCESS_INFO, _access_info_cb, NULL);
   _elm_access_callback_set
     (_elm_access_object_get(obj), ELM_ACCESS_STATE, _access_state_cb, priv);
   _elm_access_activate_hook_set
     (_elm_access_object_get(obj), _access_activate_cb, NULL);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   elm_layout_theme_set(obj, "button", "base", elm_widget_style_get(obj));
}

static void
_elm_button_smart_set_user(Elm_Button_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_button_smart_add;

   ELM_WIDGET_CLASS(sc)->event = _elm_button_smart_event;
   ELM_WIDGET_CLASS(sc)->theme = _elm_button_smart_theme;
   ELM_WIDGET_CLASS(sc)->sub_object_del = _elm_button_smart_sub_object_del;

   /* not a 'focus chain manager' */
   ELM_WIDGET_CLASS(sc)->focus_next = NULL;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   ELM_CONTAINER_CLASS(sc)->content_set = _elm_button_smart_content_set;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_button_smart_sizing_eval;

   ELM_LAYOUT_CLASS(sc)->content_aliases = _content_aliases;
   ELM_LAYOUT_CLASS(sc)->text_aliases = _text_aliases;

   sc->admits_autorepeat = EINA_TRUE;
}

EAPI const Elm_Button_Smart_Class *
elm_button_smart_class_get(void)
{
   static Elm_Button_Smart_Class _sc =
     ELM_BUTTON_SMART_CLASS_INIT_NAME_VERSION(ELM_BUTTON_SMART_NAME);
   static const Elm_Button_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class) return class;

   _elm_button_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_button_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_button_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_button_autorepeat_set(Evas_Object *obj,
                          Eina_Bool on)
{
   ELM_BUTTON_CHECK(obj);
   ELM_BUTTON_DATA_GET_OR_RETURN(obj, sd);

   if (sd->timer)
     {
        ecore_timer_del(sd->timer);
        sd->timer = NULL;
     }
   sd->autorepeat = on;
   sd->repeating = EINA_FALSE;
}

#define _AR_CAPABLE(_sd) \
  (ELM_BUTTON_CLASS(ELM_WIDGET_DATA(_sd)->api)->admits_autorepeat)

EAPI Eina_Bool
elm_button_autorepeat_get(const Evas_Object *obj)
{
   ELM_BUTTON_CHECK(obj) EINA_FALSE;
   ELM_BUTTON_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return _AR_CAPABLE(sd) & sd->autorepeat;
}

EAPI void
elm_button_autorepeat_initial_timeout_set(Evas_Object *obj,
                                          double t)
{
   ELM_BUTTON_CHECK(obj);
   ELM_BUTTON_DATA_GET_OR_RETURN(obj, sd);

   if (!_AR_CAPABLE(sd))
     {
        ERR("this widget does not support auto repetition of clicks.");
        return;
     }

   if (sd->ar_threshold == t) return;
   if (sd->timer)
     {
        ecore_timer_del(sd->timer);
        sd->timer = NULL;
     }
   sd->ar_threshold = t;
}

EAPI double
elm_button_autorepeat_initial_timeout_get(const Evas_Object *obj)
{
   ELM_BUTTON_CHECK(obj) 0.0;
   ELM_BUTTON_DATA_GET_OR_RETURN_VAL(obj, sd, 0.0);

   if (!_AR_CAPABLE(sd)) return 0.0;

   return sd->ar_threshold;
}

EAPI void
elm_button_autorepeat_gap_timeout_set(Evas_Object *obj,
                                      double t)
{
   ELM_BUTTON_CHECK(obj);
   ELM_BUTTON_DATA_GET_OR_RETURN(obj, sd);

   if (!_AR_CAPABLE(sd))
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
   ELM_BUTTON_DATA_GET_OR_RETURN_VAL(obj, sd, 0.0);

   return sd->ar_interval;
}
