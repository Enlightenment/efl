#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_radio.h"
#include "elm_widget_layout.h"

EAPI Eo_Op ELM_OBJ_RADIO_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_RADIO_CLASS

#define MY_CLASS_NAME "Elm_Radio"
#define MY_CLASS_NAME_LEGACY "elm_radio"

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
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};

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

   EINA_LIST_FOREACH(sd->group->radios, l, child)
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

   if ((!_elm_config->access_mode) ||
       (_elm_access_2nd_click_timeout(obj)))
     {
        sd->group->value = sd->value;
        if (sd->group->valuep) *(sd->group->valuep) = sd->group->value;

        _state_set_all(sd);

        if (_elm_config->access_mode)
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
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   char buf[64];

   snprintf(buf, sizeof(buf), "elm,state,icon,%s",
            elm_layout_content_get(obj, "icon") ? "visible" : "hidden");

   elm_layout_signal_emit(obj, buf, "elm");
   edje_object_message_signal_process(wd->resize_obj);
}

/* FIXME: replicated from elm_layout just because radio's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static void
_elm_radio_smart_sub_object_del(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *sobj = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_sub_object_del(sobj, &int_ret));
   if(!int_ret) return;

   _icon_signal_emit(obj);

   if (ret) *ret =  EINA_TRUE;
   eo_do(obj, elm_obj_layout_sizing_eval());
}

/* FIXME: replicated from elm_layout just because radio's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static void
_elm_radio_smart_content_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Evas_Object *content = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;
   eo_do_super(obj, MY_CLASS, elm_obj_container_content_set(part, content, &int_ret));
   if(!int_ret) return;

   _icon_signal_emit(obj);

   if (ret) *ret = EINA_TRUE;
   eo_do(obj, elm_obj_layout_sizing_eval());
}

static void
_elm_radio_smart_event(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
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

   _activate(obj);

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   if (ret) *ret = EINA_TRUE;
}

static void
_elm_radio_smart_theme(Eo *obj, void *_pd, va_list *list)
{
   Elm_Radio_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_theme_apply(&int_ret));
   if (!int_ret) return;

   if (sd->state) elm_layout_signal_emit(obj, "elm,state,radio,on", "elm");
   else elm_layout_signal_emit(obj, "elm,state,radio,off", "elm");

   if (elm_widget_disabled_get(obj) && sd->state) _state_set(obj, EINA_FALSE);

   edje_object_message_signal_process(wd->resize_obj);

   /* FIXME: replicated from elm_layout just because radio's icon
    * spot is elm.swallow.content, not elm.swallow.icon. Fix that
    * whenever we can changed the theme API */
   _icon_signal_emit(obj);

   eo_do(obj, elm_obj_layout_sizing_eval());

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_radio_smart_disable(Eo *obj, void *_pd, va_list *list)
{
   Elm_Radio_Smart_Data *sd = _pd;
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_disable(&int_ret));
   if (!int_ret) return;

   if (elm_widget_disabled_get(obj) && sd->state) _state_set(obj, EINA_FALSE);

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_radio_smart_sizing_eval(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)

{
   Evas_Coord minw = -1, minh = -1;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_radio_on_cb(void *data,
             Evas_Object *obj EINA_UNUSED,
             const char *emission EINA_UNUSED,
             const char *source EINA_UNUSED)
{
   _activate(data);
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
   ELM_RADIO_DATA_GET(obj, sd);

   if (elm_widget_disabled_get(obj)) return strdup(E_("State: Disabled"));
   if (sd->state) return strdup(E_("State: On"));

   return strdup(E_("State: Off"));
}

static void
_elm_radio_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)

{
   Elm_Radio_Smart_Data *priv = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   if (!elm_layout_theme_set(obj, "radio", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   elm_layout_signal_callback_add
     (obj, "elm,action,radio,toggle", "*", _radio_on_cb, obj);

   priv->group = calloc(1, sizeof(Group));
   priv->group->radios = eina_list_append(priv->group->radios, obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   elm_layout_sizing_eval(obj);

   _elm_access_object_register(obj, wd->resize_obj);
   _elm_access_text_set
     (_elm_access_info_get(obj), ELM_ACCESS_TYPE, E_("Radio"));
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_INFO, _access_info_cb, obj);
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_STATE, _access_state_cb, obj);
}

static void
_elm_radio_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Radio_Smart_Data *sd = _pd;

   sd->group->radios = eina_list_remove(sd->group->radios, obj);
   if (!sd->group->radios) free(sd->group);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

static void
_elm_radio_smart_text_aliases_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   const Elm_Layout_Part_Alias_Description **aliases = va_arg(*list, const Elm_Layout_Part_Alias_Description **);
   *aliases = _text_aliases;
}

static void
_elm_radio_smart_content_aliases_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   const Elm_Layout_Part_Alias_Description **aliases = va_arg(*list, const Elm_Layout_Part_Alias_Description **);
   *aliases = _content_aliases;
}

EAPI Evas_Object *
elm_radio_add(Evas_Object *parent)
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
elm_radio_group_add(Evas_Object *obj,
                    Evas_Object *group)
{
   ELM_RADIO_CHECK(obj);
   eo_do(obj, elm_obj_radio_group_add(group));
}

static void
_group_add(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *group = va_arg(*list, Evas_Object *);
   Elm_Radio_Smart_Data *sd = _pd;
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
   eo_do(obj, elm_obj_radio_state_value_set(value));
}

static void
_state_value_set(Eo *obj, void *_pd, va_list *list)
{
   int value = va_arg(*list, int);
   Elm_Radio_Smart_Data *sd = _pd;

   sd->value = value;
   if (sd->value == sd->group->value) _state_set(obj, EINA_TRUE);
   else _state_set(obj, EINA_FALSE);
}

EAPI int
elm_radio_state_value_get(const Evas_Object *obj)
{
   ELM_RADIO_CHECK(obj) 0;
   int ret = 0;
   eo_do((Eo *) obj, elm_obj_radio_state_value_get(&ret));
   return ret;
}

static void
_state_value_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);
   Elm_Radio_Smart_Data *sd = _pd;
   *ret = sd->value;
}

EAPI void
elm_radio_value_set(Evas_Object *obj,
                    int value)
{
   ELM_RADIO_CHECK(obj);
   eo_do(obj, elm_obj_radio_value_set(value));
}

static void
_value_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int value = va_arg(*list, int);
   Elm_Radio_Smart_Data *sd = _pd;

   if (value == sd->group->value) return;
   sd->group->value = value;
   if (sd->group->valuep) *(sd->group->valuep) = sd->group->value;
   _state_set_all(sd);
}

EAPI int
elm_radio_value_get(const Evas_Object *obj)
{
   ELM_RADIO_CHECK(obj) 0;
   int ret = 0;
   eo_do((Eo *) obj, elm_obj_radio_value_get(&ret));
   return ret;
}

static void
_value_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);
   Elm_Radio_Smart_Data *sd = _pd;
   *ret = sd->group->value;
}

EAPI void
elm_radio_value_pointer_set(Evas_Object *obj,
                            int *valuep)
{
   ELM_RADIO_CHECK(obj);
   eo_do(obj, elm_obj_radio_value_pointer_set(valuep));
}

static void
_value_pointer_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *valuep = va_arg(*list, int *);
   Elm_Radio_Smart_Data *sd = _pd;

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
   ELM_RADIO_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do(obj, elm_obj_radio_selected_object_get(&ret));
   return ret;
}

static void
_selected_object_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   Eina_List *l;
   Evas_Object *child;

   Elm_Radio_Smart_Data *sd = _pd;

   EINA_LIST_FOREACH(sd->group->radios, l, child)
     {
        ELM_RADIO_DATA_GET(child, sdc);

        if (sdc->value == sd->group->value)
          {
             *ret = child;
             return;
          }
     }

   *ret = NULL;
}

static void
_elm_radio_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_elm_radio_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_elm_radio_smart_activate(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Activate act = va_arg(*list, Elm_Activate);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   if (elm_widget_disabled_get(obj)) return;
   if (act != ELM_ACTIVATE_DEFAULT) return;

   _activate(obj);

   if (ret) *ret = EINA_TRUE;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_radio_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_radio_smart_del),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_DISABLE), _elm_radio_smart_disable),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_SUB_OBJECT_DEL), _elm_radio_smart_sub_object_del),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_THEME_APPLY), _elm_radio_smart_theme),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_EVENT), _elm_radio_smart_event),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_radio_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_radio_smart_focus_direction_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_ACTIVATE), _elm_radio_smart_activate),

        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_SET), _elm_radio_smart_content_set),

        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_radio_smart_sizing_eval),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TEXT_ALIASES_GET), _elm_radio_smart_text_aliases_get),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_CONTENT_ALIASES_GET), _elm_radio_smart_content_aliases_get),

        EO_OP_FUNC(ELM_OBJ_RADIO_ID(ELM_OBJ_RADIO_SUB_ID_GROUP_ADD), _group_add),
        EO_OP_FUNC(ELM_OBJ_RADIO_ID(ELM_OBJ_RADIO_SUB_ID_STATE_VALUE_SET), _state_value_set),
        EO_OP_FUNC(ELM_OBJ_RADIO_ID(ELM_OBJ_RADIO_SUB_ID_STATE_VALUE_GET), _state_value_get),
        EO_OP_FUNC(ELM_OBJ_RADIO_ID(ELM_OBJ_RADIO_SUB_ID_VALUE_SET), _value_set),
        EO_OP_FUNC(ELM_OBJ_RADIO_ID(ELM_OBJ_RADIO_SUB_ID_VALUE_GET), _value_get),
        EO_OP_FUNC(ELM_OBJ_RADIO_ID(ELM_OBJ_RADIO_SUB_ID_VALUE_POINTER_SET), _value_pointer_set),
        EO_OP_FUNC(ELM_OBJ_RADIO_ID(ELM_OBJ_RADIO_SUB_ID_SELECTED_OBJECT_GET), _selected_object_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}
static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_RADIO_SUB_ID_GROUP_ADD, "Add this radio to a group of other radio objects."),
     EO_OP_DESCRIPTION(ELM_OBJ_RADIO_SUB_ID_STATE_VALUE_SET, "Set the integer value that this radio object represents."),
     EO_OP_DESCRIPTION(ELM_OBJ_RADIO_SUB_ID_STATE_VALUE_GET, "Get the integer value that this radio object represents."),
     EO_OP_DESCRIPTION(ELM_OBJ_RADIO_SUB_ID_VALUE_SET, "Set the value of the radio group."),
     EO_OP_DESCRIPTION(ELM_OBJ_RADIO_SUB_ID_VALUE_GET, "Get the value of the radio group."),
     EO_OP_DESCRIPTION(ELM_OBJ_RADIO_SUB_ID_VALUE_POINTER_SET, "Set a convenience pointer to a integer to change when radio group value changes."),
     EO_OP_DESCRIPTION(ELM_OBJ_RADIO_SUB_ID_SELECTED_OBJECT_GET, "Get the selected radio object."),
     EO_OP_DESCRIPTION_SENTINEL
};
static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_RADIO_BASE_ID, op_desc, ELM_OBJ_RADIO_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Radio_Smart_Data),
     _class_constructor,
     NULL
};
EO_DEFINE_CLASS(elm_obj_radio_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, NULL);
