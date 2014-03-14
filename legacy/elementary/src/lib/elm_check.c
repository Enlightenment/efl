#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_check.h"
#include "elm_widget_layout.h"

EAPI Eo_Op ELM_OBJ_CHECK_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_CHECK_CLASS

#define MY_CLASS_NAME "Elm_Check"
#define MY_CLASS_NAME_LEGACY "elm_check"

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
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};

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
             _elm_access_say(E_("State: On"));
     }
   else
     {
        elm_layout_signal_emit(obj, "elm,state,check,off", "elm");
        if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
             _elm_access_say(E_("State: Off"));
     }

   evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);
}

/* FIXME: replicated from elm_layout just because check's icon spot
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

/* FIXME: replicated from elm_layout just because check's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static void
_elm_check_smart_sub_object_del(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *sobj = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_sub_object_del(sobj, &int_ret));
   if (!int_ret) return;

   _icon_signal_emit(obj);

   if (ret) *ret = EINA_TRUE;
   eo_do(obj, elm_obj_layout_sizing_eval());
}

static void
_elm_check_smart_activate(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Activate act = va_arg(*list, Elm_Activate);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   if (elm_widget_disabled_get(obj)) return;
   if (act != ELM_ACTIVATE_DEFAULT) return;

   _activate(obj);

   if (ret) *ret = EINA_TRUE;
}

/* FIXME: replicated from elm_layout just because check's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static void
_elm_check_smart_content_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
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
   eo_do(obj, elm_obj_layout_sizing_eval());
}

static void
_elm_check_smart_sizing_eval(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
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
_elm_check_smart_event(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
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
_elm_check_smart_theme(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   Elm_Check_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, elm_obj_widget_theme_apply(&int_ret));
   if (!int_ret) return;

   if (!sd->state) elm_layout_signal_emit(obj, "elm,state,check,off", "elm");
   else elm_layout_signal_emit(obj, "elm,state,check,on", "elm");

   edje_object_message_signal_process(wd->resize_obj);

   /* FIXME: replicated from elm_layout just because check's icon spot
    * is elm.swallow.content, not elm.swallow.icon. Fix that whenever
    * we can changed the theme API */
   _icon_signal_emit(obj);

   eo_do(obj, elm_obj_layout_sizing_eval());

   if (ret) *ret = EINA_TRUE;
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
_access_state_cb(void *data, Evas_Object *obj)
{
   Elm_Check_Smart_Data *sd = eo_data_scope_get(data, MY_CLASS);
   const char *on_text, *off_text;

   if (elm_widget_disabled_get(obj))
     return strdup(E_("State: Disabled"));
   if (sd->state)
     {
        on_text = elm_layout_text_get(data, "on");

        if (on_text)
          {
             char buf[1024];

             snprintf(buf, sizeof(buf), "%s: %s", E_("State"), on_text);
             return strdup(buf);
          }
        else
          return strdup(E_("State: On"));
     }

   off_text = elm_layout_text_get(data, "off");

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
              Evas_Object *o EINA_UNUSED,
              const char *emission EINA_UNUSED,
              const char *source EINA_UNUSED)
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
             Evas_Object *o EINA_UNUSED,
             const char *emission EINA_UNUSED,
             const char *source EINA_UNUSED)
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
                 Evas_Object *o EINA_UNUSED,
                 const char *emission EINA_UNUSED,
                 const char *source EINA_UNUSED)
{
   _activate(data);
}

static void
_elm_check_smart_add(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,check,on", "*",
     _on_check_on, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,check,off", "*",
     _on_check_off, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,check,toggle", "*",
     _on_check_toggle, obj);

   _elm_access_object_register(obj, wd->resize_obj);
   _elm_access_text_set
     (_elm_access_info_get(obj), ELM_ACCESS_TYPE, E_("Check"));
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_INFO, _access_info_cb, obj);
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_STATE, _access_state_cb, obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   if (!elm_layout_theme_set(obj, "check", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   elm_layout_sizing_eval(obj);
}

static void
_elm_check_smart_content_aliases_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   const Elm_Layout_Part_Alias_Description **aliases = va_arg(*list, const Elm_Layout_Part_Alias_Description **);
   *aliases = _content_aliases;
}

static void
_elm_check_smart_text_aliases_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   const Elm_Layout_Part_Alias_Description **aliases = va_arg(*list, const Elm_Layout_Part_Alias_Description **);
   *aliases = _text_aliases;
}

EAPI Evas_Object *
elm_check_add(Evas_Object *parent)
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
elm_check_state_set(Evas_Object *obj,
                    Eina_Bool state)
{
   ELM_CHECK_CHECK(obj);
   eo_do(obj, elm_obj_check_state_set(state));
}

static void
_elm_check_smart_state_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool state = va_arg(*list, int);
   Elm_Check_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (state != sd->state)
     {
        sd->state = state;
        if (sd->statep) *sd->statep = sd->state;
        if (sd->state)
          elm_layout_signal_emit(obj, "elm,state,check,on", "elm");
        else
          elm_layout_signal_emit(obj, "elm,state,check,off", "elm");
     }

   edje_object_message_signal_process(wd->resize_obj);
}

EAPI Eina_Bool
elm_check_state_get(const Evas_Object *obj)
{
   ELM_CHECK_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_check_state_get(&ret));
   return ret;
}

static void
_elm_check_smart_state_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Check_Smart_Data *sd = _pd;
   *ret = sd->state;
}

EAPI void
elm_check_state_pointer_set(Evas_Object *obj,
                            Eina_Bool *statep)
{
   ELM_CHECK_CHECK(obj);
   eo_do(obj, elm_obj_check_state_pointer_set(statep));
}

static void
_elm_check_smart_state_pointer_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool *statep = va_arg(*list, Eina_Bool *);
   Elm_Check_Smart_Data *sd = _pd;

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

static void
_elm_check_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_elm_check_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_check_smart_add),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_THEME_APPLY), _elm_check_smart_theme),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_EVENT), _elm_check_smart_event),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_SUB_OBJECT_DEL), _elm_check_smart_sub_object_del),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_ACTIVATE), _elm_check_smart_activate),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_check_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_check_smart_focus_direction_manager_is),

        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_SET), _elm_check_smart_content_set),

        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_check_smart_sizing_eval),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_CONTENT_ALIASES_GET), _elm_check_smart_content_aliases_get),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TEXT_ALIASES_GET), _elm_check_smart_text_aliases_get),

        EO_OP_FUNC(ELM_OBJ_CHECK_ID(ELM_OBJ_CHECK_SUB_ID_STATE_SET), _elm_check_smart_state_set),
        EO_OP_FUNC(ELM_OBJ_CHECK_ID(ELM_OBJ_CHECK_SUB_ID_STATE_GET), _elm_check_smart_state_get),
        EO_OP_FUNC(ELM_OBJ_CHECK_ID(ELM_OBJ_CHECK_SUB_ID_STATE_POINTER_SET), _elm_check_smart_state_pointer_set),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_CHECK_SUB_ID_STATE_SET, "Set the on/off state of the check object."),
     EO_OP_DESCRIPTION(ELM_OBJ_CHECK_SUB_ID_STATE_GET, "Get the state of the check object."),
     EO_OP_DESCRIPTION(ELM_OBJ_CHECK_SUB_ID_STATE_POINTER_SET, "Set a convenience pointer to a boolean to change."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_CHECK_BASE_ID, op_desc, ELM_OBJ_CHECK_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Check_Smart_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_obj_check_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, NULL);
