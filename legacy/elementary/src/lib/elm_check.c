#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_check.h"

EAPI const char ELM_CHECK_SMART_NAME[] = "elm_check";

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"icon", "elm.swallow.content"},
   {NULL, NULL}
};

static const Elm_Layout_Part_Alias_Description _text_aliases[] =
{
   {"default", "elm.text"},
   {"on", "elm.ontext"},
   {"off", "elm.offtext"},
   {NULL, NULL}
};

static const char SIG_CHANGED[] = "changed";

/* smart callbacks coming from elm check objects: */
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_CHECK_SMART_NAME, _elm_check, Elm_Check_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, _smart_callbacks);

static void
_activate(Evas_Object *obj)
{
   ELM_CHECK_DATA_GET(obj, sd);

   sd->state = !sd->state;
   if (sd->statep) *sd->statep = sd->state;
   if (sd->state)
     {
        elm_layout_signal_emit(obj, "elm,state,check,on", "elm");
        if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
          {
             if (!elm_layout_text_get(obj, "on"))
               {
                  _elm_access_say(E_("State: On"));
               }
             else
               _elm_access_say(E_("State: On"));
          }
     }
   else
     {
        elm_layout_signal_emit(obj, "elm,state,check,off", "elm");
        if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
          {
             if (!elm_layout_text_get(obj, "off"))
               {
                  _elm_access_say(E_("State: Off"));
               }
             else
               _elm_access_say(E_("State: Off"));
          }
     }

   evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);
}

/* FIXME: replicated from elm_layout just because check's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static void
_icon_signal_emit(Evas_Object *obj)
{
   char buf[64];

   snprintf(buf, sizeof(buf), "elm,state,icon,%s",
            elm_layout_content_get(obj, "icon") ? "visible" : "hidden");

   elm_layout_signal_emit(obj, buf, "elm");
}

/* FIXME: replicated from elm_layout just because check's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static Eina_Bool
_elm_check_smart_sub_object_del(Evas_Object *obj,
                                 Evas_Object *sobj)
{
   if (!ELM_WIDGET_CLASS(_elm_check_parent_sc)->sub_object_del(obj, sobj))
     return EINA_FALSE;

   _icon_signal_emit(obj);

   return EINA_TRUE;
}

/* FIXME: replicated from elm_layout just because check's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static Eina_Bool
_elm_check_smart_content_set(Evas_Object *obj,
                              const char *part,
                              Evas_Object *content)
{
   if (!ELM_CONTAINER_CLASS(_elm_check_parent_sc)->content_set
         (obj, part, content))
     return EINA_FALSE;

   _icon_signal_emit(obj);

   return EINA_TRUE;
}

static void
_elm_check_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   ELM_CHECK_DATA_GET(obj, sd);

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (ELM_WIDGET_DATA(sd)->resize_obj, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static Eina_Bool
_elm_check_smart_event(Evas_Object *obj,
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

   return EINA_TRUE;
}

static Eina_Bool
_elm_check_smart_theme(Evas_Object *obj)
{
   ELM_CHECK_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_check_parent_sc)->theme(obj)) return EINA_FALSE;

   if (!sd->state) elm_layout_signal_emit(obj, "elm,state,check,off", "elm");
   else elm_layout_signal_emit(obj, "elm,state,check,on", "elm");

   edje_object_message_signal_process(ELM_WIDGET_DATA(sd)->resize_obj);

   /* FIXME: replicated from elm_layout just because check's icon spot
    * is elm.swallow.content, not elm.swallow.icon. Fix that whenever
    * we can changed the theme API */
   _icon_signal_emit(obj);

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
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
_access_state_cb(void *data,
                 Evas_Object *obj,
                 Elm_Widget_Item *item __UNUSED__)
{
   Elm_Check_Smart_Data *sd = data;
   const char *on_text, *off_text;

   if (elm_widget_disabled_get(obj))
     return strdup(E_("State: Disabled"));
   if (sd->state)
     {
        on_text = elm_layout_text_get(ELM_WIDGET_DATA(sd)->obj, "on");

        if (on_text)
          {
             char buf[1024];

             snprintf(buf, sizeof(buf), "%s: %s", E_("State"), on_text);
             return strdup(buf);
          }
        else
          return strdup(E_("State: On"));
     }

   off_text = elm_layout_text_get(ELM_WIDGET_DATA(sd)->obj, "off");

   if (off_text)
     {
        char buf[1024];

        snprintf(buf, sizeof(buf), "%s: %s", E_("State"), off_text);
        return strdup(buf);
     }
   return strdup(E_("State: Off"));
}

static void
_on_check_off(void *data,
              Evas_Object *o __UNUSED__,
              const char *emission __UNUSED__,
              const char *source __UNUSED__)
{
   Evas_Object *obj = data;

   ELM_CHECK_DATA_GET(obj, sd);

   sd->state = EINA_FALSE;
   if (sd->statep) *sd->statep = sd->state;

   elm_layout_signal_emit(obj, "elm,state,check,off", "elm");
   evas_object_smart_callback_call(data, SIG_CHANGED, NULL);
}

static void
_on_check_on(void *data,
             Evas_Object *o __UNUSED__,
             const char *emission __UNUSED__,
             const char *source __UNUSED__)
{
   Evas_Object *obj = data;

   ELM_CHECK_DATA_GET(obj, sd);

   sd->state = EINA_TRUE;
   if (sd->statep) *sd->statep = sd->state;
   elm_layout_signal_emit(obj, "elm,state,check,on", "elm");
   evas_object_smart_callback_call(data, SIG_CHANGED, NULL);
}

static void
_on_check_toggle(void *data,
                 Evas_Object *o __UNUSED__,
                 const char *emission __UNUSED__,
                 const char *source __UNUSED__)
{
   _activate(data);
}

static void
_elm_check_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Check_Smart_Data);

   ELM_WIDGET_CLASS(_elm_check_parent_sc)->base.add(obj);

   edje_object_signal_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, "elm,action,check,on", "",
     _on_check_on, obj);
   edje_object_signal_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, "elm,action,check,off", "",
     _on_check_off, obj);
   edje_object_signal_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, "elm,action,check,toggle", "",
     _on_check_toggle, obj);

   _elm_access_object_register(obj, ELM_WIDGET_DATA(priv)->resize_obj);
   _elm_access_text_set
     (_elm_access_object_get(obj), ELM_ACCESS_TYPE, E_("Check"));
   _elm_access_callback_set
     (_elm_access_object_get(obj), ELM_ACCESS_INFO, _access_info_cb, priv);
   _elm_access_callback_set
     (_elm_access_object_get(obj), ELM_ACCESS_STATE, _access_state_cb, priv);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   elm_layout_theme_set(obj, "check", "base", elm_widget_style_get(obj));
   elm_layout_sizing_eval(obj);
}

static void
_elm_check_smart_set_user(Elm_Check_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_check_smart_add;

   ELM_WIDGET_CLASS(sc)->theme = _elm_check_smart_theme;
   ELM_WIDGET_CLASS(sc)->event = _elm_check_smart_event;
   ELM_WIDGET_CLASS(sc)->sub_object_del = _elm_check_smart_sub_object_del;

   /* not a 'focus chain manager' */
   ELM_WIDGET_CLASS(sc)->focus_next = NULL;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   ELM_CONTAINER_CLASS(sc)->content_set = _elm_check_smart_content_set;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_check_smart_sizing_eval;

   ELM_LAYOUT_CLASS(sc)->content_aliases = _content_aliases;
   ELM_LAYOUT_CLASS(sc)->text_aliases = _text_aliases;
}

EAPI const Elm_Check_Smart_Class *
elm_check_smart_class_get(void)
{
   static Elm_Check_Smart_Class _sc =
     ELM_CHECK_SMART_CLASS_INIT_NAME_VERSION(ELM_CHECK_SMART_NAME);
   static const Elm_Check_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_check_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_check_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_check_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_check_state_set(Evas_Object *obj,
                    Eina_Bool state)
{
   ELM_CHECK_CHECK(obj);
   ELM_CHECK_DATA_GET(obj, sd);

   if (state != sd->state)
     {
        sd->state = state;
        if (sd->statep) *sd->statep = sd->state;
        if (sd->state)
          elm_layout_signal_emit(obj, "elm,state,check,on", "elm");
        else
          elm_layout_signal_emit(obj, "elm,state,check,off", "elm");
     }

   edje_object_message_signal_process(ELM_WIDGET_DATA(sd)->resize_obj);
}

EAPI Eina_Bool
elm_check_state_get(const Evas_Object *obj)
{
   ELM_CHECK_CHECK(obj) EINA_FALSE;
   ELM_CHECK_DATA_GET(obj, sd);

   return sd->state;
}

EAPI void
elm_check_state_pointer_set(Evas_Object *obj,
                            Eina_Bool *statep)
{
   ELM_CHECK_CHECK(obj);
   ELM_CHECK_DATA_GET(obj, sd);

   if (statep)
     {
        sd->statep = statep;
        if (*sd->statep != sd->state)
          {
             sd->state = *sd->statep;
             if (sd->state)
               elm_layout_signal_emit(obj, "elm,state,check,on", "elm");
             else
               elm_layout_signal_emit(obj, "elm,state,check,off", "elm");
          }
     }
   else
     sd->statep = NULL;
}
