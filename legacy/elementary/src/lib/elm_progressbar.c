#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Progressbar Progressbar
 *
 * The progressbar adds a widget for representing current progress
 * of a job status
 *
 * A progressbar can be horizontal or vertical. It can contain an Icon and has a
 * primary label as well as a units label (that is formatted with floating
 * point values and thus accepts a printf-style format string, like
 * “%1.2f units”.
 *
 *  Label, Icon and Unit strings/objects are optional.
 *
 * A progressbar may be inverted which means values invert, with high vales being
 * on the left or top and low values on the right or bottom (as opposed to
 * normally being low on the left or top and high on the bottom and right).
 *
 * The span of the progressbar is its length (horizontally or vertically).
 * This will be scaled by the object or applications scaling factor. At any point
 * code can query the progressbar for its value with elm_progressbar_value_get().
 */

#define MIN_RATIO_LVL 0.0
#define MAX_RATIO_LVL 1.0

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *progressbar;
   Evas_Object *spacer;
   Evas_Object *icon;
   Evas_Coord size;
   Eina_Bool horizontal : 1;
   Eina_Bool inverted : 1;
   Eina_Bool pulse : 1;
   Eina_Bool pulse_state : 1;
   const char *units;
   const char *label;
   double val;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);
static void _units_set(Evas_Object *obj);
static void _val_set(Evas_Object *obj);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->label) eina_stringshare_del(wd->label);
   if (wd->units) eina_stringshare_del(wd->units);
   free(wd);
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_mirrored_set(wd->progressbar, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   if (wd->horizontal)
     _elm_theme_object_set(obj, wd->progressbar, "progressbar", "horizontal", elm_widget_style_get(obj));
   else
     _elm_theme_object_set(obj, wd->progressbar, "progressbar", "vertical", elm_widget_style_get(obj));

   if (wd->icon)
     {
        edje_object_part_swallow(wd->progressbar, "elm.swallow.content", wd->icon);
        edje_object_signal_emit(wd->progressbar, "elm,state,icon,visible", "elm");
     }
   if (wd->label)
     {
        edje_object_part_text_set(wd->progressbar, "elm.text", wd->label);
        edje_object_signal_emit(wd->progressbar, "elm,state,text,visible", "elm");
     }
   if (wd->pulse)
     edje_object_signal_emit(wd->progressbar, "elm,state,pulse", "elm");
   else
     edje_object_signal_emit(wd->progressbar, "elm,state,fraction", "elm");
   if (wd->pulse_state)
     edje_object_signal_emit(wd->progressbar, "elm,state,pulse,start", "elm");

   if ((wd->units) && (!wd->pulse))
     edje_object_signal_emit(wd->progressbar, "elm,state,units,visible", "elm");

   if (wd->horizontal)
     evas_object_size_hint_min_set(wd->spacer, (double)wd->size * elm_widget_scale_get(obj) * _elm_config->scale, 1);
   else
     evas_object_size_hint_min_set(wd->spacer, 1, (double)wd->size * elm_widget_scale_get(obj) * _elm_config->scale);

   edje_object_part_swallow(wd->progressbar, "elm.swallow.bar", wd->spacer);

   if (wd->inverted)
     edje_object_signal_emit(wd->progressbar, "elm,state,inverted,on", "elm");

   _units_set(obj);
   edje_object_message_signal_process(wd->progressbar);
   edje_object_scale_set(wd->progressbar, elm_widget_scale_get(obj) * _elm_config->scale);
   _val_set(obj);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;
   if (!wd) return;
   edje_object_size_min_restricted_calc(wd->progressbar, &minw, &minh, minw, minh);
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
        edje_object_signal_emit(wd->progressbar, "elm,state,icon,hidden", "elm");
        evas_object_event_callback_del_full
           (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints, obj);
        wd->icon = NULL;
        edje_object_message_signal_process(wd->progressbar);
        _sizing_eval(obj);
     }
}

static void
_val_set(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_Bool rtl;
   double pos;
   if (!wd) return;
   pos = wd->val;
   rtl = elm_widget_mirrored_get(obj);
   if ((!rtl && wd->inverted) || (rtl &&
                                  ((!wd->horizontal && wd->inverted) ||
                                   (wd->horizontal && !wd->inverted)))) pos = MAX_RATIO_LVL - pos;
   edje_object_part_drag_value_set(wd->progressbar, "elm.cur.progressbar", pos, pos);
}

static void
_units_set(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->units)
     {
        char buf[1024];
        snprintf(buf, sizeof(buf), wd->units, 100 * wd->val);
        edje_object_part_text_set(wd->progressbar, "elm.text.status", buf);
     }
   else
     edje_object_part_text_set(wd->progressbar, "elm.text.status", NULL);
}

static void
_elm_progressbar_label_set(Evas_Object *obj, const char *item, const char *label)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (item) return;
   if (!wd) return;
   eina_stringshare_replace(&wd->label, label);
   if (label)
     {
        edje_object_signal_emit(wd->progressbar, "elm,state,text,visible", "elm");
        edje_object_message_signal_process(wd->progressbar);
     }
   else
     {
        edje_object_signal_emit(wd->progressbar, "elm,state,text,hidden", "elm");
        edje_object_message_signal_process(wd->progressbar);
     }
   edje_object_part_text_set(wd->progressbar, "elm.text", label);
   _sizing_eval(obj);
}

static const char *
_elm_progressbar_label_get(const Evas_Object *obj, const char *item)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (item) return NULL;
   if (!wd) return NULL;
   return wd->label;
}

/**
 * Add a new progressbar to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Progressbar
 */
EAPI Evas_Object *
elm_progressbar_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "progressbar");
   elm_widget_type_set(obj, "progressbar");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_text_set_hook_set(obj, _elm_progressbar_label_set);
   elm_widget_text_get_hook_set(obj, _elm_progressbar_label_get);

   wd->horizontal = EINA_TRUE;
   wd->inverted = EINA_FALSE;
   wd->pulse = EINA_FALSE;
   wd->pulse_state = EINA_FALSE;
   wd->units = eina_stringshare_add("%.0f %%");
   wd->val = MIN_RATIO_LVL;

   wd->progressbar = edje_object_add(e);
   _elm_theme_object_set(obj, wd->progressbar, "progressbar", "horizontal", "default");
   elm_widget_resize_object_set(obj, wd->progressbar);

   wd->spacer = evas_object_rectangle_add(e);
   evas_object_color_set(wd->spacer, 0, 0, 0, 0);
   evas_object_pass_events_set(wd->spacer, EINA_TRUE);
   elm_widget_sub_object_add(obj, wd->spacer);
   edje_object_part_swallow(wd->progressbar, "elm.swallow.bar", wd->spacer);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   _units_set(obj);
   _val_set(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _sizing_eval(obj);
   return obj;
}

/**
 * Normally the progressbar will display and interpret values from low to high.
 * This display a progressbar for jobs with unknow state of progression,
 * (the cursor pulse right to left and left to right, and loop) if pulse is set to 1.
 *
 * @param obj The progressbar object
 * @param pulse The pulse flag. 1 == pulse, 0 == normal
 *
 * @ingroup Progressbar
 */
EAPI void
elm_progressbar_pulse_set(Evas_Object *obj, Eina_Bool pulse)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   pulse = !!pulse;
   if (wd->pulse == pulse) return;
   wd->pulse = pulse;
   _theme_hook(obj);
}

/**
 * Normally the progressbar will display and interpret values from low to high.
 * This display a progressbar for jobs with unknow state of progression,
 * (the cursor pulse right to left and left to right, and loop) if pulse is set to 1.
 *
 * @param obj The progressbar object
 * @return The pulse flag
 * (1 == pulse, 0 == normal)
 *
 * @ingroup Progressbar
 */
EAPI Eina_Bool
elm_progressbar_pulse_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->pulse;
}

/**
 * Stat/Stop de pulse action
 *
 * @param obj The progressbar object
 * @param state The pulse flag. 1 == start pulse, 0 == stop pulse
 *
 * @ingroup Progressbar
 */
EAPI void
elm_progressbar_pulse(Evas_Object *obj, Eina_Bool state)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   state = !!state;
   if ((!wd->pulse) && (wd->pulse_state == state)) return;
   wd->pulse_state = state;
   if (wd->pulse_state)
     edje_object_signal_emit(wd->progressbar, "elm,state,pulse,start", "elm");
   else
     edje_object_signal_emit(wd->progressbar, "elm,state,pulse,stop", "elm");
}

/**
 * Set the value the progressbar indicates
 *
 * @param obj The progressbar object
 * @param val The fraction value (must be between 0.0 and 1.0)
 *
 * @ingroup Progressbar
 */
EAPI void
elm_progressbar_value_set(Evas_Object *obj, double val)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->val == val) return;
   wd->val = val;
   if (wd->val < MIN_RATIO_LVL) wd->val = MIN_RATIO_LVL;
   if (wd->val > MAX_RATIO_LVL) wd->val = MAX_RATIO_LVL;
   _val_set(obj);
   _units_set(obj);
}


/**
 * Get the value the progressbar has
 *
 * @param obj The progressbar object
 * @return The value of the progressbar
 *
 * @ingroup Progressbar
 */
EAPI double
elm_progressbar_value_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0.0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0.0;
   return wd->val;
}

/**
 * Set the label of the progressbar
 *
 * @param obj The progressbar object
 * @param label The text label string in UTF-8
 *
 * @ingroup Progressbar
 * @deprecated use elm_object_text_set() instead.
 */
EAPI void
elm_progressbar_label_set(Evas_Object *obj, const char *label)
{
   _elm_progressbar_label_set(obj, NULL, label);
}

/**
 * Get the label of the progressbar
 *
 * @param obj The progressbar object
 * @return The text label string in UTF-8
 *
 * @ingroup Progressbar
 * @deprecated use elm_object_text_set() instead.
 */
EAPI const char *
elm_progressbar_label_get(const Evas_Object *obj)
{
   return _elm_progressbar_label_get(obj, NULL);
}

/**
 * Set the icon object of the progressbar object
 *
 * Once the icon object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_progressbar_icon_unset() function.
 *
 * @param obj The progressbar object
 * @param icon The icon object
 *
 * @ingroup Progressbar
 */
EAPI void
elm_progressbar_icon_set(Evas_Object *obj, Evas_Object *icon)
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
        edje_object_part_swallow(wd->progressbar, "elm.swallow.content", icon);
        edje_object_signal_emit(wd->progressbar, "elm,state,icon,visible", "elm");
        edje_object_message_signal_process(wd->progressbar);
     }
   _sizing_eval(obj);
}

/**
 * Get the icon object of the progressbar object
 *
 * @param obj The progressbar object
 * @return The icon object
 *
 * @ingroup Progressbar
 */
EAPI Evas_Object *
elm_progressbar_icon_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->icon;
}

/**
 * Unset the icon used for the progressbar object
 *
 * Unparent and return the icon object which was set for this widget.
 *
 * @param obj The progressbar object
 * @return The icon object that was being used
 *
 * @ingroup Progressbar
 */
EAPI Evas_Object *
elm_progressbar_icon_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (!wd->icon) return NULL;
   Evas_Object *icon = wd->icon;
   elm_widget_sub_object_del(obj, wd->icon);
   edje_object_part_unswallow(wd->progressbar, wd->icon);
   wd->icon = NULL;
   return icon;
}

/**
 * Set the length of the progression region of the progressbar
 *
 * This sets the minimum width or height (depending on orientation) of the
 * area of the progressbar that allows the progressbar to be dragged around. This in
 * turn affects the objects minimum size (along with icon label and unit
 * text).
 *
 * @param obj The progressbar object
 * @param size The length of the progressbar area
 *
 * @ingroup Progressbar
 */
EAPI void
elm_progressbar_span_size_set(Evas_Object *obj, Evas_Coord size)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->size == size) return;
   wd->size = size;
   if (wd->horizontal)
     evas_object_size_hint_min_set(wd->spacer, (double)wd->size * elm_widget_scale_get(obj) * _elm_config->scale, 1);
   else
     evas_object_size_hint_min_set(wd->spacer, 1, (double)wd->size * elm_widget_scale_get(obj) * _elm_config->scale);
   edje_object_part_swallow(wd->progressbar, "elm.swallow.bar", wd->spacer);
   _sizing_eval(obj);
}

/**
 * Get the length of the progression region of the progressbar
 *
 * @param obj The progressbar object
 * @return The length of the progressbar area
 *
 * @ingroup Progressbar
 */
EAPI Evas_Coord
elm_progressbar_span_size_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return wd->size;
}

/**
 * Set the format string of the unit area
 *
 * If NULL, this disabls the unit area display. If not it sets the format
 * string for the unit text. The unit text is provided a floating point
 * value, so the unit text can display up to 1 floating point falue. Note that
 * this is optional. Use a format string such as "%1.2f meters" for example.
 *
 * @param obj The progressbar object
 * @param units The format string for the units display
 *
 * @ingroup Progressbar
 */
EAPI void
elm_progressbar_unit_format_set(Evas_Object *obj, const char *units)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   eina_stringshare_replace(&wd->units, units);
   if (units)
     {
        edje_object_signal_emit(wd->progressbar, "elm,state,units,visible", "elm");
        edje_object_message_signal_process(wd->progressbar);
     }
   else
     {
        edje_object_signal_emit(wd->progressbar, "elm,state,units,hidden", "elm");
        edje_object_message_signal_process(wd->progressbar);
     }
   _units_set(obj);
   _sizing_eval(obj);
}

/**
 * Get the format string of the unit area
 *
 * @param obj The progressbar object
 * @return The format string for the units display
 *
 * @ingroup Progressbar
 */
EAPI const char *
elm_progressbar_unit_format_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->units;
}

/**
 * Set orientation of the progressbar
 *
 * @param obj The progressbar object
 * @param horizontal If set, the progressbar will be horizontal
 *
 * @ingroup Progressbar
 */
EAPI void
elm_progressbar_horizontal_set(Evas_Object *obj, Eina_Bool horizontal)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   horizontal = !!horizontal;
   if (wd->horizontal == horizontal) return;
   wd->horizontal = horizontal;
   _theme_hook(obj);
}

/**
 * Gets orientation of the progressbar
 *
 * @param obj The progressbar object
 * @return The orientation
 * (0 = vertical, 1 = horizontal)
 *
 * @ingroup Progressbar
 */
EAPI Eina_Bool
elm_progressbar_horizontal_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->horizontal;
}

/**
 * Invert the progressbar display
 *
 * Normally the progressbar will display and interpret values from low to high
 * and when horizontal that is left to right. When vertical that is top
 * to bottom. This inverts this (so from right to left or bottom to top) if
 * inverted is set to 1.
 *
 * @param obj The progressbar object
 * @param inverted The inverted flag. 1 == inverted, 0 == normal
 *
 * @ingroup Progressbar
 */
EAPI void
elm_progressbar_inverted_set(Evas_Object *obj, Eina_Bool inverted)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   inverted = !!inverted;
   if (wd->inverted == inverted) return;
   wd->inverted = inverted;
   if (wd->inverted)
     edje_object_signal_emit(wd->progressbar, "elm,state,inverted,on", "elm");
   else
     edje_object_signal_emit(wd->progressbar, "elm,state,inverted,off", "elm");
   edje_object_message_signal_process(wd->progressbar);
   _val_set(obj);
   _units_set(obj);
}

/**
 * Gets if the progressbar will displayed inverted
 *
 * @param obj The progressbar object
 * @return The inverted flag
 * (1 == inverted, 0 == normal)
 *
 * @ingroup Progressbar
 */
EAPI Eina_Bool
elm_progressbar_inverted_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->inverted;
}
