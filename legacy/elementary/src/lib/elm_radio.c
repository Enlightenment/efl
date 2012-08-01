#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_radio.h"

EAPI const char ELM_RADIO_SMART_NAME[] = "elm_radio";

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

static const char SIG_CHANGED[] = "changed";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_RADIO_SMART_NAME, _elm_radio, Elm_Radio_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, _smart_callbacks);

static void
_state_set(Evas_Object *obj, Eina_Bool state)
{
   ELM_RADIO_DATA_GET(obj, sd);

   if ((state != sd->state) && (!elm_widget_disabled_get(obj)))
     {
        sd->state = state;
        if (sd->state)
          elm_layout_signal_emit(obj, "elm,state,radio,on", "elm");
        else
          elm_layout_signal_emit(obj, "elm,state,radio,off", "elm");
     }
}

static void
_state_set_all(Elm_Radio_Smart_Data *sd)
{
   const Eina_List *l;
   Eina_Bool disabled = EINA_FALSE;
   Evas_Object *child, *selected = NULL;

   EINA_LIST_FOREACH (sd->group->radios, l, child)
     {
        ELM_RADIO_DATA_GET(child, sdc);

        if (sdc->state) selected = child;
        if (sdc->value == sd->group->value)
          {
             _state_set(child, EINA_TRUE);
             if (!sdc->state) disabled = EINA_TRUE;
          }
        else _state_set(child, EINA_FALSE);
     }

   if ((disabled) && (selected)) _state_set(selected, 1);
}

static void
_activate(Evas_Object *obj)
{
   ELM_RADIO_DATA_GET(obj, sd);

   if (sd->group->value == sd->value) return;

   if ((_elm_config->access_mode == ELM_ACCESS_MODE_OFF) ||
       (_elm_access_2nd_click_timeout(obj)))
     {
        sd->group->value = sd->value;
        if (sd->group->valuep) *(sd->group->valuep) = sd->group->value;

        _state_set_all(sd);

        if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
          _elm_access_say(E_("State: On"));
        evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);
     }
}

/* FIXME: replicated from elm_layout just because radio's icon spot
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

/* FIXME: replicated from elm_layout just because radio's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static Eina_Bool
_elm_radio_smart_sub_object_del(Evas_Object *obj,
                                 Evas_Object *sobj)
{
   if (!ELM_WIDGET_CLASS(_elm_radio_parent_sc)->sub_object_del(obj, sobj))
     return EINA_FALSE;

   _icon_signal_emit(obj);

   return EINA_TRUE;
}

/* FIXME: replicated from elm_layout just because radio's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static Eina_Bool
_elm_radio_smart_content_set(Evas_Object *obj,
                              const char *part,
                              Evas_Object *content)
{
   if (!ELM_CONTAINER_CLASS(_elm_radio_parent_sc)->content_set
         (obj, part, content))
     return EINA_FALSE;

   _icon_signal_emit(obj);

   return EINA_TRUE;
}

static Eina_Bool
_elm_radio_smart_event(Evas_Object *obj,
                       Evas_Object *src __UNUSED__,
                       Evas_Callback_Type type,
                       void *event_info)
{
   Evas_Event_Key_Down *ev;

   if (elm_widget_disabled_get(obj)) return EINA_FALSE;

   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   ev = event_info;

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
_elm_radio_smart_theme(Evas_Object *obj)
{
   ELM_RADIO_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_radio_parent_sc)->theme(obj)) return EINA_FALSE;

   if (sd->state) elm_layout_signal_emit(obj, "elm,state,radio,on", "elm");
   else elm_layout_signal_emit(obj, "elm,state,radio,off", "elm");

   if (elm_widget_disabled_get(obj) && sd->state) _state_set(obj, EINA_FALSE);

   edje_object_message_signal_process(ELM_WIDGET_DATA(sd)->resize_obj);

   /* FIXME: replicated from elm_layout just because radio's icon
    * spot is elm.swallow.content, not elm.swallow.icon. Fix that
    * whenever we can changed the theme API */
   _icon_signal_emit(obj);

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static Eina_Bool
_elm_radio_smart_disable(Evas_Object *obj)
{
   ELM_RADIO_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_radio_parent_sc)->disable(obj))
     return EINA_FALSE;

   if (elm_widget_disabled_get(obj) && sd->state) _state_set(obj, EINA_FALSE);

   return EINA_TRUE;
}

static void
_elm_radio_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1;

   ELM_RADIO_DATA_GET(obj, sd);

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (ELM_WIDGET_DATA(sd)->resize_obj, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_radio_on_cb(void *data,
             Evas_Object *obj __UNUSED__,
             const char *emission __UNUSED__,
             const char *source __UNUSED__)
{
   _activate(data);
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
   ELM_RADIO_DATA_GET(obj, sd);

   if (elm_widget_disabled_get(obj)) return strdup(E_("State: Disabled"));
   if (sd->state) return strdup(E_("State: On"));

   return strdup(E_("State: Off"));
}

static void
_elm_radio_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Radio_Smart_Data);

   ELM_WIDGET_CLASS(_elm_radio_parent_sc)->base.add(obj);

   elm_layout_theme_set(obj, "radio", "base", elm_widget_style_get(obj));

   elm_layout_signal_callback_add
     (obj, "elm,action,radio,on", "", _radio_on_cb, obj);
   elm_layout_signal_callback_add
     (obj, "elm,action,radio,toggle", "", _radio_on_cb, obj);

   priv->group = calloc(1, sizeof(Group));
   priv->group->radios = eina_list_append(priv->group->radios, obj);
   priv->state = EINA_FALSE;

   elm_widget_can_focus_set(obj, EINA_TRUE);

   elm_layout_sizing_eval(obj);

   _elm_access_object_register(obj, ELM_WIDGET_DATA(priv)->resize_obj);
   _elm_access_text_set
     (_elm_access_object_get(obj), ELM_ACCESS_TYPE, E_("Radio"));
   _elm_access_callback_set
     (_elm_access_object_get(obj), ELM_ACCESS_INFO, _access_info_cb, obj);
   _elm_access_callback_set
     (_elm_access_object_get(obj), ELM_ACCESS_STATE, _access_state_cb, obj);
}

static void
_elm_radio_smart_del(Evas_Object *obj)
{
   ELM_RADIO_DATA_GET(obj, sd);

   sd->group->radios = eina_list_remove(sd->group->radios, obj);
   if (!sd->group->radios) free(sd->group);

   ELM_WIDGET_CLASS(_elm_radio_parent_sc)->base.del(obj);
}

static void
_elm_radio_smart_set_user(Elm_Radio_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_radio_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_radio_smart_del;

   ELM_WIDGET_CLASS(sc)->disable = _elm_radio_smart_disable;
   ELM_WIDGET_CLASS(sc)->theme = _elm_radio_smart_theme;
   ELM_WIDGET_CLASS(sc)->sub_object_del = _elm_radio_smart_sub_object_del;
   ELM_WIDGET_CLASS(sc)->event = _elm_radio_smart_event;

   /* not a 'focus chain manager' */
   ELM_WIDGET_CLASS(sc)->focus_next = NULL;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   ELM_CONTAINER_CLASS(sc)->content_set = _elm_radio_smart_content_set;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_radio_smart_sizing_eval;

   ELM_LAYOUT_CLASS(sc)->content_aliases = _content_aliases;
   ELM_LAYOUT_CLASS(sc)->text_aliases = _text_aliases;
}

EAPI const Elm_Radio_Smart_Class *
elm_radio_smart_class_get(void)
{
   static Elm_Radio_Smart_Class _sc =
     ELM_RADIO_SMART_CLASS_INIT_NAME_VERSION(ELM_RADIO_SMART_NAME);
   static const Elm_Radio_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_radio_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_radio_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_radio_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_radio_group_add(Evas_Object *obj,
                    Evas_Object *group)
{
   ELM_RADIO_CHECK(obj);
   ELM_RADIO_DATA_GET(obj, sd);
   ELM_RADIO_DATA_GET(group, sdg);

   if (!sdg)
     {
        if (eina_list_count(sd->group->radios) == 1) return;
        sd->group->radios = eina_list_remove(sd->group->radios, obj);
        sd->group = calloc(1, sizeof(Group));
        sd->group->radios = eina_list_append(sd->group->radios, obj);
     }
   else if (sd->group == sdg->group)
     return;
   else
     {
        sd->group->radios = eina_list_remove(sd->group->radios, obj);
        if (!sd->group->radios) free(sd->group);
        sd->group = sdg->group;
        sd->group->radios = eina_list_append(sd->group->radios, obj);
     }
   if (sd->value == sd->group->value) _state_set(obj, EINA_TRUE);
   else _state_set(obj, EINA_FALSE);
}

EAPI void
elm_radio_state_value_set(Evas_Object *obj,
                          int value)
{
   ELM_RADIO_CHECK(obj);
   ELM_RADIO_DATA_GET(obj, sd);

   sd->value = value;
   if (sd->value == sd->group->value) _state_set(obj, EINA_TRUE);
   else _state_set(obj, EINA_FALSE);
}

EAPI int
elm_radio_state_value_get(const Evas_Object *obj)
{
   ELM_RADIO_CHECK(obj) 0;
   ELM_RADIO_DATA_GET(obj, sd);

   return sd->value;
}

EAPI void
elm_radio_value_set(Evas_Object *obj,
                    int value)
{
   ELM_RADIO_CHECK(obj);
   ELM_RADIO_DATA_GET(obj, sd);

   if (value == sd->group->value) return;
   sd->group->value = value;
   if (sd->group->valuep) *(sd->group->valuep) = sd->group->value;
   _state_set_all(sd);
}

EAPI int
elm_radio_value_get(const Evas_Object *obj)
{
   ELM_RADIO_CHECK(obj) 0;
   ELM_RADIO_DATA_GET(obj, sd);

   return sd->group->value;
}

EAPI void
elm_radio_value_pointer_set(Evas_Object *obj,
                            int *valuep)
{
   ELM_RADIO_CHECK(obj);
   ELM_RADIO_DATA_GET(obj, sd);

   if (valuep)
     {
        sd->group->valuep = valuep;
        if (*(sd->group->valuep) != sd->group->value)
          {
             sd->group->value = *(sd->group->valuep);
             _state_set_all(sd);
          }
     }
   else sd->group->valuep = NULL;
}

EAPI Evas_Object *
elm_radio_selected_object_get(Evas_Object *obj)
{
   Eina_List *l;
   Evas_Object *child;

   ELM_RADIO_CHECK(obj) NULL;
   ELM_RADIO_DATA_GET(obj, sd);

   EINA_LIST_FOREACH (sd->group->radios, l, child)
     {
        ELM_RADIO_DATA_GET(child, sdc);

        if (sdc->value == sd->group->value) return child;
     }

   return NULL;
}
