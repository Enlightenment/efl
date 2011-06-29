#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Toggle
 *
 * A toggle is a slider which can be used to toggle between
 * two values.  It has two states: on and off.
 *
 * Signals that you can add callbacks for are:
 *
 * "changed" - Whenever the toggle value has been changed.  Is not called until
 *             the toggle is released by the cursor (assuming it has been
 *             triggered by the cursor in the first place).
 */

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *tgl;
   Evas_Object *icon;
   Eina_Bool state;
   Eina_Bool *statep;
   const char *label;
   const char *ontext, *offtext;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _disable_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);
static void _signal_toggle_off(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_toggle_on(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _on_focus_hook(void *data, Evas_Object *obj);
static Eina_Bool _event_hook(Evas_Object *obj, Evas_Object *src,
                             Evas_Callback_Type type, void *event_info);

static const char SIG_CHANGED[] = "changed";
static const Evas_Smart_Cb_Description _signals[] = {
       {SIG_CHANGED, ""},
       {NULL, NULL}
};

static Eina_Bool
_event_hook(Evas_Object *obj, Evas_Object *src __UNUSED__, Evas_Callback_Type type, void *event_info)
{
   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   Evas_Event_Key_Down *ev = event_info;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if ((strcmp(ev->keyname, "Return")) &&
       (strcmp(ev->keyname, "KP_Enter")) &&
       (strcmp(ev->keyname, "space")))
     return EINA_FALSE;
   elm_toggle_state_set(obj, !wd->state);
   evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->label) eina_stringshare_del(wd->label);
   if (wd->ontext) eina_stringshare_del(wd->ontext);
   if (wd->offtext) eina_stringshare_del(wd->offtext);
   free(wd);
}

static void
_disable_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_disabled_get(obj))
     edje_object_signal_emit(wd->tgl, "elm,state,disabled", "elm");
   else
     edje_object_signal_emit(wd->tgl, "elm,state,enabled", "elm");
}

static void
_on_focus_hook(void *data __UNUSED__, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_focus_get(obj))
     {
        edje_object_signal_emit(wd->tgl, "elm,action,focus", "elm");
        evas_object_focus_set(wd->tgl, EINA_TRUE);
     }
   else
     {
        edje_object_signal_emit(wd->tgl, "elm,action,unfocus", "elm");
        evas_object_focus_set(wd->tgl, EINA_FALSE);
     }
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_mirrored_set(wd->tgl, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _elm_theme_object_set(obj, wd->tgl, "toggle", "base", elm_widget_style_get(obj));
   if (wd->icon)
     edje_object_signal_emit(wd->tgl, "elm,state,icon,visible", "elm");
   else
     edje_object_signal_emit(wd->tgl, "elm,state,icon,hidden", "elm");
   if (wd->state)
     edje_object_signal_emit(wd->tgl, "elm,state,toggle,on", "elm");
   else
     edje_object_signal_emit(wd->tgl, "elm,state,toggle,off", "elm");
   if (wd->label)
     edje_object_signal_emit(wd->tgl, "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(wd->tgl, "elm,state,text,hidden", "elm");
   edje_object_part_text_set(wd->tgl, "elm.text", wd->label);
   edje_object_part_text_set(wd->tgl, "elm.ontext", wd->ontext);
   edje_object_part_text_set(wd->tgl, "elm.offtext", wd->offtext);
   if (elm_widget_disabled_get(obj))
     edje_object_signal_emit(wd->tgl, "elm,state,disabled", "elm");
   edje_object_message_signal_process(wd->tgl);
   edje_object_scale_set(wd->tgl, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;

   if (!wd) return;
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(wd->tgl, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   if (obj != wd->icon) return;
   _sizing_eval(data);
}

static void
_sub_del(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   if (!wd) return;
   if (sub == wd->icon)
     {
        edje_object_signal_emit(wd->tgl, "elm,state,icon,hidden", "elm");
        evas_object_event_callback_del_full
           (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints, obj);
        wd->icon = NULL;
        edje_object_message_signal_process(wd->tgl);
        _sizing_eval(obj);
     }
}

static void
_signal_toggle_off(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   wd->state = 0;
   if (wd->statep) *wd->statep = wd->state;
   evas_object_smart_callback_call(data, SIG_CHANGED, NULL);
}

static void
_signal_toggle_on(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   wd->state = 1;
   if (wd->statep) *wd->statep = wd->state;
   evas_object_smart_callback_call(data, SIG_CHANGED, NULL);
}

static void
_elm_toggle_label_set(Evas_Object *obj, const char *item, const char *label)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (item) return;
   if (!wd) return;
   eina_stringshare_replace(&wd->label, label);
   if (label)
     edje_object_signal_emit(wd->tgl, "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(wd->tgl, "elm,state,text,hidden", "elm");
   edje_object_message_signal_process(wd->tgl);
   edje_object_part_text_set(wd->tgl, "elm.text", label);
   _sizing_eval(obj);
}

static const char *
_elm_toggle_label_get(const Evas_Object *obj, const char *item)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (item) return NULL;
   if (!wd) return NULL;
   return wd->label;
}

/**
 * Add a toggle to @p parent.
 *
 * @param parent The parent object
 *
 * @return The toggle object
 *
 * @ingroup Toggle
 */
EAPI Evas_Object *
elm_toggle_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "toggle");
   elm_widget_type_set(obj, "toggle");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_disable_hook_set(obj, _disable_hook);
   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_event_hook_set(obj, _event_hook);
   elm_widget_label_set_hook_set(obj, _elm_toggle_label_set);
   elm_widget_label_get_hook_set(obj, _elm_toggle_label_get);

   wd->tgl = edje_object_add(e);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _elm_theme_object_set(obj, wd->tgl, "toggle", "base", "default");
   wd->ontext = eina_stringshare_add("ON");
   wd->offtext = eina_stringshare_add("OFF");
   edje_object_signal_callback_add(wd->tgl, "elm,action,toggle,on", "",
                                   _signal_toggle_on, obj);
   edje_object_signal_callback_add(wd->tgl, "elm,action,toggle,off", "",
                                   _signal_toggle_off, obj);
   elm_widget_resize_object_set(obj, wd->tgl);
   edje_object_part_text_set(wd->tgl, "elm.ontext", wd->ontext);
   edje_object_part_text_set(wd->tgl, "elm.offtext", wd->offtext);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   edje_object_signal_emit(wd->tgl, "elm,state,toggle,off", "elm");

   _sizing_eval(obj);

   // TODO: convert Elementary to subclassing of Evas_Smart_Class
   // TODO: and save some bytes, making descriptions per-class and not instance!
   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   return obj;
}

/**
 * Sets the label to be displayed with the toggle.
 *
 * @param obj The toggle object
 * @param label The label to be displayed
 *
 * @ingroup Toggle
 * @deprecate use elm_object_text_* instead.
 */
EAPI void
elm_toggle_label_set(Evas_Object *obj, const char *label)
{
   _elm_toggle_label_set(obj, NULL, label);
}

/**
 * Gets the label of the toggle
 *
 * @param obj  toggleeee object
 * @return The label of the toggle
 *
 * @ingroup Toggle
 * @deprecate use elm_object_text_* instead.
 */
EAPI const char *
elm_toggle_label_get(const Evas_Object *obj)
{
   return _elm_toggle_label_get(obj, NULL);
}

/**
 * Set the icon used for the toggle
 *
 * Once the icon object is set, a previously set one will be deleted
 * If you want to keep that old content object, use the
 * elm_toggle_icon_unset() function.
 *
 * @param obj The toggle object
 * @param icon The icon object for the button
 *
 * @ingroup Toggle
 */
EAPI void
elm_toggle_icon_set(Evas_Object *obj, Evas_Object *icon)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->icon == icon) return;
   if (wd->icon) evas_object_del(wd->icon);
   wd->icon = icon;
   if (icon)
     {
        elm_widget_sub_object_add(obj, icon);
        evas_object_event_callback_add(icon, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
        edje_object_part_swallow(wd->tgl, "elm.swallow.content", icon);
        edje_object_signal_emit(wd->tgl, "elm,state,icon,visible", "elm");
        edje_object_message_signal_process(wd->tgl);
     }
   _sizing_eval(obj);
}

/**
 * Get the icon used for the toggle
 *
 * Return the icon object which is set for this widget.
 *
 * @param obj The toggle object
 * @return The icon object that is being used
 *
 * @ingroup Toggle
 */
EAPI Evas_Object *
elm_toggle_icon_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->icon;
}

/**
 * Unset the icon used for the toggle
 *
 * Unparent and return the icon object which was set for this widget.
 *
 * @param obj The toggle object
 * @return The icon object that was being used
 *
 * @ingroup Toggle
 */
EAPI Evas_Object *
elm_toggle_icon_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (!wd->icon) return NULL;
   Evas_Object *icon = wd->icon;
   elm_widget_sub_object_del(obj, wd->icon);
   edje_object_part_unswallow(wd->tgl, wd->icon);
   wd->icon = NULL;
   return icon;
}

/**
 * Sets the labels to be associated with the on and off states of the toggle.
 *
 * @param obj The toggle object
 * @param onlabel The label displayed when the toggle is in the "on" state
 * @param offlabel The label displayed when the toggle is in the "off" state
 *
 * @ingroup Toggle
 */
EAPI void
elm_toggle_states_labels_set(Evas_Object *obj, const char *onlabel, const char *offlabel)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   eina_stringshare_replace(&wd->ontext, onlabel);
   eina_stringshare_replace(&wd->offtext, offlabel);
   edje_object_part_text_set(wd->tgl, "elm.ontext", onlabel);
   edje_object_part_text_set(wd->tgl, "elm.offtext", offlabel);
   _sizing_eval(obj);
}


/**
 * Gets the labels associated with the on and off states of the toggle.
 *
 * @param obj The toggle object
 * @param onlabel A char** to place the onlabel of @p obj into
 * @param offlabel A char** to place the offlabel of @p obj into
 *
 * @ingroup Toggle
 */
EAPI void
elm_toggle_states_labels_get(const Evas_Object *obj, const char **onlabel, const char **offlabel)
{
   if (onlabel) *onlabel = NULL;
   if (offlabel) *offlabel = NULL;
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (onlabel) *onlabel = wd->ontext;
   if (offlabel) *offlabel = wd->offtext;
}

/**
 * Sets the state of the toggle to @p state.
 *
 * @param obj The toggle object
 * @param state The state of @p obj
 *
 * @ingroup Toggle
 */
EAPI void
elm_toggle_state_set(Evas_Object *obj, Eina_Bool state)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (state != wd->state)
     {
        wd->state = state;
        if (wd->statep) *wd->statep = wd->state;
        if (wd->state)
          edje_object_signal_emit(wd->tgl, "elm,state,toggle,on", "elm");
        else
          edje_object_signal_emit(wd->tgl, "elm,state,toggle,off", "elm");
     }
}

/**
 * Gets the state of the toggle to @p state.
 *
 * @param obj The toggle object
 * @return The state of @p obj
 *
 * @ingroup Toggle
 */
EAPI Eina_Bool
elm_toggle_state_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->state;
}

/**
 * Sets the state pointer of the toggle to @p statep.
 *
 * @param obj The toggle object
 * @param statep The state pointer of @p obj
 *
 * @ingroup Toggle
 */
EAPI void
elm_toggle_state_pointer_set(Evas_Object *obj, Eina_Bool *statep)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (statep)
     {
        wd->statep = statep;
        if (*wd->statep != wd->state)
          {
             wd->state = *wd->statep;
             if (wd->state)
               edje_object_signal_emit(wd->tgl, "elm,state,toggle,on", "elm");
             else
               edje_object_signal_emit(wd->tgl, "elm,state,toggle,off", "elm");
          }
     }
   else
     wd->statep = NULL;
}
