#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Check Check
 *
 * The check widget allows for toggling a value between true or false (1 or 0).
 *
 * Check objects are a lot like radio objects in layout and functionality
 * except they do not work as a group, but independently and only toggle the
 * value of a boolean from false to true (0 or 1). elm_check_state_set() sets
 * the boolean state (1 for true, 0 for false), and elm_check_state_get()
 * returns the current state. For convenience, like the radio objects, you
 * can set a pointer to a boolean directly with elm_check_state_pointer_set()
 * for it to modify.
 *
 * Signals that you can add callbacks for are:
 *
 * "changed" - This is called whenever the user changes the state of one of the
 *             check object.
 */
typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *chk, *icon;
   Eina_Bool state;
   Eina_Bool *statep;
   const char *label;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);
static void _theme_hook(Evas_Object *obj);
static void _disable_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);
static void _signal_check_off(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_check_on(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_check_toggle(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _on_focus_hook(void *data, Evas_Object *obj);
static void _activate_hook(Evas_Object *obj);
static void _activate(Evas_Object *obj);
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
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if ((strcmp(ev->keyname, "Return")) &&
       (strcmp(ev->keyname, "KP_Enter")) &&
       (strcmp(ev->keyname, "space")))
     return EINA_FALSE;
   _activate(obj);
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}


static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->label) eina_stringshare_del(wd->label);
   free(wd);
}

static void
_on_focus_hook(void *data __UNUSED__, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_focus_get(obj))
     {
        edje_object_signal_emit(wd->chk, "elm,action,focus", "elm");
        evas_object_focus_set(wd->chk, EINA_TRUE);
     }
   else
     {
        edje_object_signal_emit(wd->chk, "elm,action,unfocus", "elm");
        evas_object_focus_set(wd->chk, EINA_FALSE);
     }
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_mirrored_set(wd->chk, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _elm_theme_object_set(obj, wd->chk, "check", "base", elm_widget_style_get(obj));
   if (wd->icon)
     edje_object_signal_emit(wd->chk, "elm,state,icon,visible", "elm");
   else
     edje_object_signal_emit(wd->chk, "elm,state,icon,hidden", "elm");
   if (wd->state)
     edje_object_signal_emit(wd->chk, "elm,state,check,on", "elm");
   else
     edje_object_signal_emit(wd->chk, "elm,state,check,off", "elm");
   if (wd->label)
     edje_object_signal_emit(wd->chk, "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(wd->chk, "elm,state,text,hidden", "elm");
   edje_object_part_text_set(wd->chk, "elm.text", wd->label);
   if (elm_widget_disabled_get(obj))
     edje_object_signal_emit(wd->chk, "elm,state,disabled", "elm");
   edje_object_message_signal_process(wd->chk);
   edje_object_scale_set(wd->chk, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static void
_disable_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_disabled_get(obj))
     edje_object_signal_emit(wd->chk, "elm,state,disabled", "elm");
   else
     edje_object_signal_emit(wd->chk, "elm,state,enabled", "elm");
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   if (!wd) return;
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(wd->chk, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   if (obj != wd->icon) return;
   Evas_Coord mw, mh;
   evas_object_size_hint_min_get(obj, &mw, &mh);
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
        edje_object_signal_emit(wd->chk, "elm,state,icon,hidden", "elm");
        evas_object_event_callback_del_full(sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                            _changed_size_hints, obj);
        wd->icon = NULL;
        _sizing_eval(obj);
        edje_object_message_signal_process(wd->chk);
     }
}

static void
_signal_check_off(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   wd->state = EINA_FALSE;
   if (wd->statep) *wd->statep = wd->state;
   edje_object_signal_emit(wd->chk, "elm,state,check,off", "elm");
   evas_object_smart_callback_call(data, SIG_CHANGED, NULL);
}

static void
_signal_check_on(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   wd->state = EINA_TRUE;
   if (wd->statep) *wd->statep = wd->state;
   edje_object_signal_emit(wd->chk, "elm,state,check,on", "elm");
   evas_object_smart_callback_call(data, SIG_CHANGED, NULL);
}

static void
_signal_check_toggle(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   _activate(data);
}

static void
_activate_hook(Evas_Object *obj)
{
   _activate(obj);
}

static void
_activate(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->state = !wd->state;
   if (wd->statep) *wd->statep = wd->state;
   if (wd->state)
     edje_object_signal_emit(wd->chk, "elm,state,check,on", "elm");
   else
     edje_object_signal_emit(wd->chk, "elm,state,check,off", "elm");
   evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);
}

static void
_elm_check_label_set(Evas_Object *obj, const char *item, const char *label)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (item) return;
   if (!wd) return;
   eina_stringshare_replace(&wd->label, label);
   if (label)
     edje_object_signal_emit(wd->chk, "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(wd->chk, "elm,state,text,hidden", "elm");
   edje_object_message_signal_process(wd->chk);
   edje_object_part_text_set(wd->chk, "elm.text", label);
   _sizing_eval(obj);
}

static const char *
_elm_check_label_get(const Evas_Object *obj, const char *item)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (item) return NULL;
   if (!wd) return NULL;
   return wd->label;
}

/**
 * Add a new Check object
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Check
 */
EAPI Evas_Object *
elm_check_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "check");
   elm_widget_type_set(obj, "check");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_disable_hook_set(obj, _disable_hook);
   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_activate_hook_set(obj, _activate_hook);
   elm_widget_event_hook_set(obj, _event_hook);
   elm_widget_text_set_hook_set(obj, _elm_check_label_set);
   elm_widget_text_get_hook_set(obj, _elm_check_label_get);

   wd->chk = edje_object_add(e);
   _elm_theme_object_set(obj, wd->chk, "check", "base", "default");
   edje_object_signal_callback_add(wd->chk, "elm,action,check,on", "",
                                   _signal_check_on, obj);
   edje_object_signal_callback_add(wd->chk, "elm,action,check,off", "",
                                   _signal_check_off, obj);
   edje_object_signal_callback_add(wd->chk, "elm,action,check,toggle", "",
                                   _signal_check_toggle, obj);
   elm_widget_resize_object_set(obj, wd->chk);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _sizing_eval(obj);

   // TODO: convert Elementary to subclassing of Evas_Smart_Class
   // TODO: and save some bytes, making descriptions per-class and not instance!
   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   return obj;
}

/**
 * Set the text label of the check object
 *
 * @param obj The check object
 * @param label The text label string in UTF-8
 *
 * @ingroup Check
 * @deprecated use elm_object_text_set() instead.
 */
EAPI void
elm_check_label_set(Evas_Object *obj, const char *label)
{
   _elm_check_label_set(obj, NULL, label);
}

/**
 * Get the text label of the check object
 *
 * @param obj The check object
 * @return The text label string in UTF-8
 *
 * @ingroup Check
 * @deprecated use elm_object_text_set() instead.
 */
EAPI const char *
elm_check_label_get(const Evas_Object *obj)
{
   return _elm_check_label_get(obj, NULL);
}

/**
 * Set the icon object of the check object
 *
 * Once the icon object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_check_icon_unset() function.
 *
 * @param obj The check object
 * @param icon The icon object
 *
 * @ingroup Check
 */
EAPI void
elm_check_icon_set(Evas_Object *obj, Evas_Object *icon)
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
        edje_object_part_swallow(wd->chk, "elm.swallow.content", icon);
        edje_object_signal_emit(wd->chk, "elm,state,icon,visible", "elm");
        edje_object_message_signal_process(wd->chk);
     }
   _sizing_eval(obj);
}

/**
 * Get the icon object of the check object
 *
 * @param obj The check object
 * @return The icon object
 *
 * @ingroup Check
 */
EAPI Evas_Object *
elm_check_icon_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->icon;
}

/**
 * Unset the icon used for the check object
 *
 * Unparent and return the icon object which was set for this widget.
 *
 * @param obj The check object
 * @return The icon object that was being used
 *
 * @ingroup Check
 */
EAPI Evas_Object *
elm_check_icon_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (!wd->icon) return NULL;
   Evas_Object *icon = wd->icon;
   elm_widget_sub_object_del(obj, wd->icon);
   edje_object_part_unswallow(wd->chk, wd->icon);
   wd->icon = NULL;
   return icon;
}

/**
 * Set the on/off state of the check object
 *
 * This sets the state of the check and will also set the value if pointed to
 * to the state supplied, but will not call any callbacks.
 *
 * @param obj The check object
 * @param state The state to use (1 == on, 0 == off)
 *
 * @ingroup Check
 */
EAPI void
elm_check_state_set(Evas_Object *obj, Eina_Bool state)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (state != wd->state)
     {
        wd->state = state;
        if (wd->statep) *wd->statep = wd->state;
        if (wd->state)
          edje_object_signal_emit(wd->chk, "elm,state,check,on", "elm");
        else
          edje_object_signal_emit(wd->chk, "elm,state,check,off", "elm");
     }
}

/**
 * Get the state of the check object
 *
 * @param obj The check object
 * @return The boolean state
 *
 * @ingroup Check
 */
EAPI Eina_Bool
elm_check_state_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->state;
}

/**
 * Set a convenience pointer to a boolean to change
 *
 * This sets a pointer to a boolean, that, in addition to the check objects
 * state will also be modified directly. To stop setting the object pointed
 * to simply use NULL as the statep parameter. If statep is not NULL, then
 * when this is called, the check objects state will also be modified to
 * reflect the value of the boolean statep points to, just like calling
 * elm_check_state_set().
 *
 * @param obj The check object
 * @param statep Pointer to the boolean to modify
 *
 * @ingroup Check
 */
EAPI void
elm_check_state_pointer_set(Evas_Object *obj, Eina_Bool *statep)
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
               edje_object_signal_emit(wd->chk, "elm,state,check,on", "elm");
             else
               edje_object_signal_emit(wd->chk, "elm,state,check,off", "elm");
          }
     }
   else
     wd->statep = NULL;
}
