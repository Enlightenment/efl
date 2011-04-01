#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Bubble Bubble
 *
 * The Bubble is an widget used to show a text in a frame as speech is
 * represented in comics.
 *
 * Signals that you can add callbacks for are:
 *
 * clicked - This is called when a user has clicked the bubble.
 */

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *bbl;
   Evas_Object *content, *icon;
   const char *label, *info, *corner;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);

#define SIG_CLICKED "clicked"
static const Evas_Smart_Cb_Description _signals[] =
{
     {SIG_CLICKED, ""},
     {NULL, NULL}
};

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->label) eina_stringshare_del(wd->label);
   if (wd->info) eina_stringshare_del(wd->info);
   if (wd->corner) eina_stringshare_del(wd->corner);
   free(wd);
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_mirrored_set(wd->bbl, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _elm_theme_object_set(obj, wd->bbl, "bubble", wd->corner,
                         elm_widget_style_get(obj));
   edje_object_part_text_set(wd->bbl, "elm.text", wd->label);
   if (wd->label) edje_object_signal_emit(wd->bbl, "elm,state,text,visible", "elm");
   else edje_object_signal_emit(wd->bbl, "elm,state,text,hidden", "elm");
   edje_object_part_text_set(wd->bbl, "elm.info", wd->info);
   if (wd->info) edje_object_signal_emit(wd->bbl, "elm,state,info,visible", "elm");
   else edje_object_signal_emit(wd->bbl, "elm,state,info,hidden", "elm");
   if (wd->content)
     {
        edje_object_part_swallow(wd->bbl, "elm.swallow.content", wd->content);
        edje_object_message_signal_process(wd->bbl);
     }
   if (wd->icon)
     edje_object_signal_emit(wd->bbl, "elm,state,icon,visible", "elm");
   else
     edje_object_signal_emit(wd->bbl, "elm,state,icon,hidden", "elm");
   edje_object_scale_set(wd->bbl,
                         elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static Eina_Bool
_elm_bubble_focus_next_hook(const Evas_Object *obj, Elm_Focus_Direction dir, Evas_Object **next)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *cur;

   if ((!wd) || (!wd->content))
     return EINA_FALSE;

   cur = wd->content;

   /* Try Focus cycle in subitem */
   return elm_widget_focus_next_get(cur, dir, next);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   if (!wd) return;
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(wd->bbl, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   _sizing_eval(data);
}

static void
_sub_del(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   if (!wd) return;
   evas_object_event_callback_del_full(sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
   if (sub == wd->content) wd->content = NULL;
   else if (sub == wd->icon)
     {
        edje_object_signal_emit(wd->bbl, "elm,state,icon,hidden", "elm");
        wd->icon = NULL;
        edje_object_message_signal_process(wd->bbl);
     }
   _sizing_eval(obj);
}

static void
_mouse_up(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     return;
   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

/**
 * Add a new bubble to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * This function adds a text bubble to the given parent evas object.
 *
 * @ingroup Bubble
 */
EAPI Evas_Object *
elm_bubble_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "bubble");
   elm_widget_type_set(obj, "bubble");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_focus_next_hook_set(obj, _elm_bubble_focus_next_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);

   wd->corner = eina_stringshare_add("base");

   wd->bbl = edje_object_add(e);
   elm_widget_resize_object_set(obj, wd->bbl);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   evas_object_event_callback_add(wd->bbl, EVAS_CALLBACK_MOUSE_UP,
                                  _mouse_up, obj);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _elm_theme_object_set(obj, wd->bbl, "bubble", wd->corner,
                         elm_widget_style_get(obj));
   _sizing_eval(obj);
   return obj;
}

/**
 * Set the label of the bubble
 *
 * @param obj The bubble object
 * @param label The string to set in the label
 *
 * This function sets the title of the bubble that is shown on top of
 * the bubble.
 *
 * @ingroup Bubble
 */
EAPI void
elm_bubble_label_set(Evas_Object *obj, const char *label)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   eina_stringshare_replace(&wd->label, label);
   edje_object_part_text_set(wd->bbl, "elm.text", label);
   if (label) edje_object_signal_emit(wd->bbl, "elm,state,text,visible", "elm");
   else edje_object_signal_emit(wd->bbl, "elm,state,text,hidden", "elm");
   _sizing_eval(obj);
}

/**
 * Get the label of the bubble
 *
 * @param obj The bubble object
 * @return The string of set in the label
 *
 * This function gets the title of the bubble that is shown on top of
 * the bubble.
 *
 * @ingroup Bubble
 */
EAPI const char*
elm_bubble_label_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->label;
}

/**
 * Set the info of the bubble
 *
 * @param obj The bubble object
 * @param info The given info about the bubble
 *
 * This function sets the text shown on the top right of bubble.
 * In the Anchorblock example of the Elementary tests application it
 * shows time.
 *
 * @ingroup Bubble
 *
 */
EAPI void
elm_bubble_info_set(Evas_Object *obj, const char *info)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   eina_stringshare_replace(&wd->info, info);
   edje_object_part_text_set(wd->bbl, "elm.info", info);
   if (info) edje_object_signal_emit(wd->bbl, "elm,state,info,visible", "elm");
   else edje_object_signal_emit(wd->bbl, "elm,state,info,hidden", "elm");
   _sizing_eval(obj);
}

/**
 * Get the info of the bubble
 *
 * @param obj The bubble object
 *
 * @return The "info" string of the bubble
 *
 * This function gets the text set to be displayed at the top right of
 * the bubble.
 *
 * @ingroup Bubble
 *
 */
EAPI const char *
elm_bubble_info_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->info;
}

/**
 * Set the content to be shown in the bubble
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep the old content object, use the
 * elm_bubble_content_unset() function.
 *
 * @param obj The bubble object
 * @param content The given content of the bubble
 *
 * This function sets the content shown on the middle of the bubble.
 * In the Anchorblock example of the Elementary tests application it
 * shows time.
 *
 * @ingroup Bubble
 */
EAPI void
elm_bubble_content_set(Evas_Object *obj, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->content == content) return;
   if (wd->content) evas_object_del(wd->content);
   wd->content = content;
   if (content)
     {
        elm_widget_sub_object_add(obj, content);
        evas_object_event_callback_add(content, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
        edje_object_part_swallow(wd->bbl, "elm.swallow.content", content);
     }
   _sizing_eval(obj);
}

/**
 * Get the content shown in the bubble
 *
 * Return the content object which is set for this widget.
 *
 * @param obj The bubble object
 * @return The content that is being used
 *
 * @ingroup Bubble
 */
EAPI Evas_Object *
elm_bubble_content_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->content;
}

/**
 * Unset the content shown in the bubble
 *
 * Unparent and return the content object which was set for this widget.
 *
 * @param obj The bubble object
 * @return The content that was being used
 *
 * @ingroup Bubble
 */
EAPI Evas_Object *
elm_bubble_content_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *content;
   if (!wd) return NULL;
   if (!wd->content) return NULL;
   content = wd->content;
   elm_widget_sub_object_del(obj, content);
   edje_object_part_unswallow(wd->bbl, content);
   wd->content = NULL;
   return content;
}

/**
 * Set the icon of the bubble
 *
 * Once the icon object is set, a previously set one will be deleted.
 * If you want to keep the old content object, use the
 * elm_icon_content_unset() function.
 *
 * @param obj The bubble object
 * @param icon The given icon for the bubble
 *
 * @ingroup Bubble
 */
EAPI void
elm_bubble_icon_set(Evas_Object *obj, Evas_Object *icon)
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
        edje_object_part_swallow(wd->bbl, "elm.swallow.icon", icon);
        evas_object_event_callback_add(icon, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
        edje_object_signal_emit(wd->bbl, "elm,state,icon,visible", "elm");
        edje_object_message_signal_process(wd->bbl);
     }
   _sizing_eval(obj);
}

/**
 * Get the icon of the bubble
 *
 * @param obj The bubble object
 * @return The icon for the bubble
 *
 * This function gets the icon shown on the top left of bubble.
 *
 * @ingroup Bubble
 */
EAPI Evas_Object *
elm_bubble_icon_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->icon;
}

/**
 * Unset the icon of the bubble
 *
 * Unparent and return the icon object which was set for this widget.
 *
 * @param obj The bubble object
 * @return The icon that was being used
 *
 * @ingroup Bubble
 */
EAPI Evas_Object *
elm_bubble_icon_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *icon;
   if (!wd) return NULL;
   if (!wd->icon) return NULL;
   icon = wd->icon;
   elm_widget_sub_object_del(obj, icon);
   edje_object_part_unswallow(wd->bbl, icon);
   wd->icon = NULL;
   return icon;
}

/**
 * Set the corner of the bubble
 *
 * @param obj The bubble object.
 * @param corner The given corner for the bubble.
 *
 * This function sets the corner of the bubble.
 * The corner will be used to find the group in the theme
 * For example, if you set the corner to "bottom_right",
 * the following group will be searched:
 * "elm/bubble/bottom_right/default",
 * considering default style.
 *
 * @ingroup Bubble
 */
EAPI void
elm_bubble_corner_set(Evas_Object *obj, const char *corner)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   EINA_SAFETY_ON_NULL_RETURN(corner);
   eina_stringshare_replace(&wd->corner, corner);
   _theme_hook(obj);
}

/**
 * Get the corner of the bubble
 *
 * @param obj The bubble object.
 * @return The given corner for the bubble.
 *
 * This function gets the corner of the bubble.
 *
 * @ingroup Bubble
 */
EAPI const char*
elm_bubble_corner_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->corner;
}
