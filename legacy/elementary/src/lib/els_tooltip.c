#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Tooltips Tooltips
 *
 * The Tooltip is an (internal, for now) smart object used to show a
 * content in a frame on mouse hover of objects(or widgets), with
 * tips/information about them.
 */

static const char _tooltip_key[] = "_elm_tooltip";

#define ELM_TOOLTIP_GET_OR_RETURN(tt, obj, ...)         \
  Elm_Tooltip *tt;                                      \
  do                                                    \
    {                                                   \
       if (!(obj))                                      \
         {                                              \
            CRITICAL("Null pointer: " #obj);            \
            return __VA_ARGS__;                         \
         }                                              \
       tt = evas_object_data_get((obj), _tooltip_key);  \
       if (!tt)                                         \
         {                                              \
            ERR("Object does not have tooltip: " #obj); \
            return __VA_ARGS__;                         \
         }                                              \
    }                                                   \
  while (0)

struct _Elm_Tooltip
{
   Elm_Tooltip_Content_Cb   func;
   Evas_Smart_Cb            del_cb;
   const void              *data;
   const char              *style;
   Evas                    *evas;
   Evas_Object             *eventarea, *owner;
   Evas_Object             *tooltip, *content;
   Ecore_Timer             *show_timer;
   Ecore_Timer             *hide_timer;
   Ecore_Job               *reconfigure_job;
   struct {
      Evas_Coord            x, y, bx, by;
   } pad;
   struct {
      double                x, y;
   } rel_pos;
   double                   hide_timeout; /* from theme */
   Eina_Bool                visible_lock:1;
   Eina_Bool                changed_style:1;
};

static void _elm_tooltip_reconfigure(Elm_Tooltip *tt);
static void _elm_tooltip_reconfigure_job_start(Elm_Tooltip *tt);
static void _elm_tooltip_reconfigure_job_stop(Elm_Tooltip *tt);
static void _elm_tooltip_hide_anim_start(Elm_Tooltip *tt);
static void _elm_tooltip_hide_anim_stop(Elm_Tooltip *tt);
static void _elm_tooltip_show_timer_stop(Elm_Tooltip *tt);
static void _elm_tooltip_hide(Elm_Tooltip *tt);
static void _elm_tooltip_data_clean(Elm_Tooltip *tt);


static void
_elm_tooltip_content_changed_hints_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _elm_tooltip_reconfigure_job_start(data);
}

static void
_elm_tooltip_content_del_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Tooltip *tt = data;
   tt->content = NULL;
   tt->visible_lock = EINA_FALSE;
   _elm_tooltip_hide(tt);
}

static void
_elm_tooltip_obj_move_cb(void *data, Evas *e  __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info  __UNUSED__)
{
   Elm_Tooltip *tt = data;
   _elm_tooltip_reconfigure_job_start(tt);
}

static void
_elm_tooltip_obj_resize_cb(void *data, Evas *e  __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info  __UNUSED__)
{
   Elm_Tooltip *tt = data;
   _elm_tooltip_reconfigure_job_start(tt);
}

static void
_elm_tooltip_obj_mouse_move_cb(void *data, Evas *e  __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info  __UNUSED__)
{
   Elm_Tooltip *tt = data;
   _elm_tooltip_reconfigure_job_start(tt);
}

static void
_elm_tooltip_show(Elm_Tooltip *tt)
{
   _elm_tooltip_show_timer_stop(tt);
   _elm_tooltip_hide_anim_stop(tt);

   if (tt->tooltip)
     {
        _elm_tooltip_reconfigure_job_start(tt);
        return;
     }
   tt->tooltip = edje_object_add(tt->evas);
   if (!tt->tooltip) return;

   evas_object_layer_set(tt->tooltip, ELM_OBJECT_LAYER_TOOLTIP);

   evas_object_event_callback_add
     (tt->eventarea, EVAS_CALLBACK_MOVE, _elm_tooltip_obj_move_cb, tt);
   evas_object_event_callback_add
     (tt->eventarea, EVAS_CALLBACK_RESIZE, _elm_tooltip_obj_resize_cb, tt);
   evas_object_event_callback_add
     (tt->eventarea, EVAS_CALLBACK_MOUSE_MOVE, _elm_tooltip_obj_mouse_move_cb, tt);

   evas_object_pass_events_set(tt->tooltip, EINA_TRUE);
   tt->changed_style = EINA_TRUE;
   _elm_tooltip_reconfigure_job_start(tt);
}

static void
_elm_tooltip_content_del(Elm_Tooltip *tt)
{
   if (!tt->content) return;

   evas_object_event_callback_del_full
     (tt->content, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
      _elm_tooltip_content_changed_hints_cb, tt);
   evas_object_event_callback_del_full
     (tt->content, EVAS_CALLBACK_DEL,
      _elm_tooltip_content_del_cb, tt);
   evas_object_hide(tt->content);
   evas_object_del(tt->content);
   tt->content = NULL;
}


static void
_elm_tooltip_hide(Elm_Tooltip *tt)
{
   _elm_tooltip_show_timer_stop(tt);
   _elm_tooltip_hide_anim_stop(tt);
   _elm_tooltip_reconfigure_job_stop(tt);

   if (!tt->tooltip) return;
   if (tt->visible_lock) return;

   _elm_tooltip_content_del(tt);

   evas_object_event_callback_del_full
     (tt->eventarea, EVAS_CALLBACK_MOVE, _elm_tooltip_obj_move_cb, tt);
   evas_object_event_callback_del_full
     (tt->eventarea, EVAS_CALLBACK_RESIZE, _elm_tooltip_obj_resize_cb, tt);
   evas_object_event_callback_del_full
     (tt->eventarea, EVAS_CALLBACK_MOUSE_MOVE, _elm_tooltip_obj_mouse_move_cb, tt);

   evas_object_del(tt->tooltip);
   tt->tooltip = NULL;
}

static void
_elm_tooltip_reconfigure_job(void *data)
{
   Elm_Tooltip *tt = data;
   tt->reconfigure_job = NULL;
   _elm_tooltip_reconfigure(data);
}

static void
_elm_tooltip_reconfigure_job_stop(Elm_Tooltip *tt)
{
   if (!tt->reconfigure_job) return;
   ecore_job_del(tt->reconfigure_job);
   tt->reconfigure_job = NULL;
}

static void
_elm_tooltip_reconfigure_job_start(Elm_Tooltip *tt)
{
   if (tt->reconfigure_job) ecore_job_del(tt->reconfigure_job);
   tt->reconfigure_job = ecore_job_add
     (_elm_tooltip_reconfigure_job, tt);
}

static Eina_Bool
_elm_tooltip_hide_anim_cb(void *data)
{
   Elm_Tooltip *tt = data;
   tt->hide_timer = NULL;
   _elm_tooltip_hide(tt);
   return EINA_FALSE;
}

static void
_elm_tooltip_hide_anim_start(Elm_Tooltip *tt)
{
   if (tt->hide_timer) return;
   edje_object_signal_emit(tt->tooltip, "elm,action,hide", "elm");
   tt->hide_timer = ecore_timer_add
     (tt->hide_timeout, _elm_tooltip_hide_anim_cb, tt);
}

static void
_elm_tooltip_hide_anim_stop(Elm_Tooltip *tt)
{
   if (!tt->hide_timer) return;
   if (tt->tooltip)
     edje_object_signal_emit(tt->tooltip, "elm,action,show", "elm");
   ecore_timer_del(tt->hide_timer);
   tt->hide_timer = NULL;
}

static void
_elm_tooltip_reconfigure(Elm_Tooltip *tt)
{
   Evas_Coord ox, oy, ow, oh, px, py, tx, ty, tw, th, cw, ch;
   Evas_Coord eminw, eminh, ominw, ominh;
   double rel_x, rel_y;
   Eina_Bool inside_eventarea;

   _elm_tooltip_reconfigure_job_stop(tt);

   if (tt->hide_timer) return;
   if (!tt->tooltip) return;
   if (tt->changed_style)
     {
        const char *style = tt->style ? tt->style : "default";
        const char *str;
        if (!_elm_theme_object_set
            (tt->owner, tt->tooltip, "tooltip", "base", style))
          {
             ERR("Could not apply the theme to the tooltip! style=%s", style);
             evas_object_del(tt->tooltip);
             tt->tooltip = NULL;
             return;
          }

        tt->rel_pos.x = 0;
        tt->rel_pos.y = 0;

        tt->pad.x = 0;
        tt->pad.y = 0;
        tt->pad.bx = 0;
        tt->pad.by = 0;
        tt->hide_timeout = 0.0;

        str = edje_object_data_get(tt->tooltip, "pad_x");
        if (str) tt->pad.x = atoi(str);
        str = edje_object_data_get(tt->tooltip, "pad_y");
        if (str) tt->pad.y = atoi(str);

        str = edje_object_data_get(tt->tooltip, "pad_border_x");
        if (str) tt->pad.bx = atoi(str);
        str = edje_object_data_get(tt->tooltip, "pad_border_y");
        if (str) tt->pad.by = atoi(str);

        str = edje_object_data_get(tt->tooltip, "hide_timeout");
        if (str)
          {
             tt->hide_timeout = atof(str);
             if (tt->hide_timeout < 0.0) tt->hide_timeout = 0.0;
          }

        evas_object_pass_events_set(tt->tooltip, EINA_TRUE);
        tt->changed_style = EINA_FALSE;
        if (tt->tooltip)
          edje_object_part_swallow
            (tt->tooltip, "elm.swallow.content", tt->content);

        edje_object_signal_emit(tt->tooltip, "elm,action,show", "elm");
     }

   if (!tt->content)
     {
        tt->content = tt->func((void *)tt->data, tt->owner);
        if (!tt->content)
          {
             WRN("could not create tooltip content!");
             evas_object_del(tt->tooltip);
             tt->tooltip = NULL;
             return;
          }
        evas_object_layer_set(tt->content, ELM_OBJECT_LAYER_TOOLTIP);
        evas_object_pass_events_set(tt->content, EINA_TRUE);
        edje_object_part_swallow
          (tt->tooltip, "elm.swallow.content", tt->content);
        evas_object_event_callback_add
          (tt->content, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
           _elm_tooltip_content_changed_hints_cb, tt);
        evas_object_event_callback_add
          (tt->content, EVAS_CALLBACK_DEL,
           _elm_tooltip_content_del_cb, tt);

     }

   evas_object_size_hint_min_get(tt->content, &ominw, &ominh);
   edje_object_size_min_get(tt->tooltip, &eminw, &eminh);

   if (ominw < eminw) ominw = eminw;
   if (ominh < eminh) ominh = eminh;

   if (ominw < 1) ominw = 10; /* at least it is noticeable */
   if (ominh < 1) ominh = 10; /* at least it is noticeable */

   edje_object_size_min_restricted_calc
     (tt->tooltip, &tw, &th, ominw, ominh);

   evas_output_size_get(tt->evas, &cw, &ch);
   evas_pointer_canvas_xy_get(tt->evas, &px, &py);

   evas_object_geometry_get(tt->eventarea, &ox, &oy, &ow, &oh);

   inside_eventarea = ((px >= ox) && (py >= oy) &&
                       (px <= ox + ow) && (py <= oy + oh));
   if (inside_eventarea)
     {
        tx = px;
        ty = py;

        if (tx + tw + tt->pad.x < cw) tx += tt->pad.x;
        if (ty + th + tt->pad.y < ch) ty += tt->pad.y;
     }
   else
     {
        tx = ox + (ow / 2) - (tw / 2);
        if (ch < (th + oy + oh)) ty = oy - th;
        else ty = oy + oh;
     }

   if (tt->pad.bx * 2 + tw < cw)
     {
        if (tx < tt->pad.bx) tx = tt->pad.bx;
        else if (tx + tw >= cw - tt->pad.bx) tx = cw - tw - tt->pad.bx;
     }

   if (tt->pad.by * 2 + th < ch)
     {
        if (ty < tt->pad.by) ty = tt->pad.by;
        else if (ty + th >= ch - tt->pad.by) ty = ch - th - tt->pad.by;
     }

   evas_object_move(tt->tooltip, tx, ty);
   evas_object_resize(tt->tooltip, tw, th);
   evas_object_show(tt->tooltip);

   if (inside_eventarea)
     {
        rel_x = (px - tx) / (double)tw;
        rel_y = (py - ty) / (double)th;
     }
   else
     {
        rel_x = (ox + (ow / 2) - tx) / (double)tw;
        rel_y = (oy + (oh / 2) - ty) / (double)th;
     }

#define FDIF(a, b) (fabs((a) - (b)) > 0.0001)
   if ((FDIF(rel_x, tt->rel_pos.x)) || (FDIF(rel_y, tt->rel_pos.y)))
     {
        Edje_Message_Float_Set *msg;

        msg = alloca(sizeof(Edje_Message_Float_Set) + sizeof(double));
        msg->count = 2;
        msg->val[0] = rel_x;
        msg->val[1] = rel_y;
        tt->rel_pos.x = rel_x;
        tt->rel_pos.y = rel_y;

        edje_object_message_send(tt->tooltip, EDJE_MESSAGE_FLOAT_SET, 1, msg);
     }
#undef FDIF
}

static void
_elm_tooltip_show_timer_stop(Elm_Tooltip *tt)
{
   if (!tt->show_timer) return;
   ecore_timer_del(tt->show_timer);
   tt->show_timer = NULL;
}

static Eina_Bool
_elm_tooltip_timer_show_cb(void *data)
{
   Elm_Tooltip *tt = data;
   tt->show_timer = NULL;
   _elm_tooltip_show(tt);
   return ECORE_CALLBACK_CANCEL;
}

static void
_elm_tooltip_obj_mouse_in_cb(void *data, Evas *e  __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info  __UNUSED__)
{
   Elm_Tooltip *tt = data;

   _elm_tooltip_hide_anim_stop(tt);

   if ((tt->show_timer) || (tt->tooltip)) return;

   tt->show_timer = ecore_timer_add
     (_elm_config->tooltip_delay, _elm_tooltip_timer_show_cb, tt);
}

static void
_elm_tooltip_obj_mouse_out_cb(void *data, Evas *e  __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info  __UNUSED__)
{
   Elm_Tooltip *tt = data;

   if (tt->visible_lock) return;

   if (!tt->tooltip)
     {
        _elm_tooltip_show_timer_stop(tt);
        return;
     }

   _elm_tooltip_hide_anim_start(tt);
}

static void _elm_tooltip_obj_free_cb(void *data, Evas *e  __UNUSED__, Evas_Object *obj, void *event_info  __UNUSED__);

static void
_elm_tooltip_unset(Elm_Tooltip *tt)
{
   tt->visible_lock = EINA_FALSE;
   _elm_tooltip_hide(tt);
   _elm_tooltip_data_clean(tt);

   if (tt->eventarea)
     {
        evas_object_event_callback_del_full
          (tt->eventarea, EVAS_CALLBACK_MOUSE_IN,
           _elm_tooltip_obj_mouse_in_cb, tt);
        evas_object_event_callback_del_full
          (tt->eventarea, EVAS_CALLBACK_MOUSE_OUT,
           _elm_tooltip_obj_mouse_out_cb, tt);
        evas_object_event_callback_del_full
          (tt->eventarea, EVAS_CALLBACK_FREE, _elm_tooltip_obj_free_cb, tt);

        evas_object_data_del(tt->eventarea, _tooltip_key);
     }
   if (tt->owner)
     {
        evas_object_event_callback_del_full
          (tt->owner, EVAS_CALLBACK_FREE, _elm_tooltip_obj_free_cb, tt);
        elm_widget_tooltip_del(tt->owner, tt);
     }

   eina_stringshare_del(tt->style);
   free(tt);
}

static void
_elm_tooltip_obj_free_cb(void *data, Evas *e  __UNUSED__, Evas_Object *obj, void *event_info  __UNUSED__)
{
   Elm_Tooltip *tt = data;
   if (tt->eventarea == obj) tt->eventarea = NULL;
   if (tt->owner == obj) tt->owner = NULL;
   _elm_tooltip_unset(tt);
}

static Evas_Object *
_elm_tooltip_label_create(void *data, Evas_Object *obj)
{
   Evas_Object *label = elm_label_add(obj);
   if (!label)
     return NULL;
   elm_object_style_set(label, "tooltip");
   elm_object_text_set(label, data);
   return label;
}

static void
_elm_tooltip_label_del_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   eina_stringshare_del(data);
}

static void
_elm_tooltip_data_clean(Elm_Tooltip *tt)
{
   if (tt->del_cb) tt->del_cb((void *)tt->data, tt->owner, NULL);

   _elm_tooltip_content_del(tt);

   tt->data = NULL;
   tt->del_cb = NULL;
}

/**
 * Notify tooltip should recalculate its theme.
 * @internal
 */
void
elm_tooltip_theme(Elm_Tooltip *tt)
{
   if (!tt->tooltip) return;
   tt->changed_style = EINA_TRUE;
   _elm_tooltip_reconfigure_job_start(tt);
}


/**
 * Set the content to be shown in the tooltip object for specific event area.
 *
 * Setup the tooltip to object. The object @a eventarea can have only
 * one tooltip, so any previous tooltip data is removed. @p func(with
 * @p data) will be called every time that need show the tooltip and
 * it should return a valid Evas_Object. This object is then managed
 * fully by tooltip system and is deleted when the tooltip is gone.
 *
 * This is an internal function that is used by objects with sub-items
 * that want to provide different tooltips for each of them. The @a
 * owner object should be an elm_widget and will be used to track
 * theme changes and to feed @a func and @a del_cb. The @a eventarea
 * may be any object and is the one that should be used later on with
 * elm_object_tooltip apis, such as elm_object_tooltip_hide(),
 * elm_object_tooltip_show() or elm_object_tooltip_unset().
 *
 * @param eventarea the object being attached a tooltip.
 * @param owner the elm_widget that owns this object, will be used to
 *        track theme changes and to be used in @a func or @a del_cb.
 * @param func the function used to create the tooltip contents. The
 *        @a Evas_Object parameters will receive @a owner as value.
 * @param data what to provide to @a func as callback data/context.
 * @param del_cb called when data is not needed anymore, either when
 *        another callback replaces @func, the tooltip is unset with
 *        elm_object_tooltip_unset() or the owner object @a obj
 *        dies. This callback receives as the first parameter the
 *        given @a data, and @c event_info is NULL.
 *
 * @internal
 * @ingroup Tooltips
 */
void
elm_object_sub_tooltip_content_cb_set(Evas_Object *eventarea, Evas_Object *owner, Elm_Tooltip_Content_Cb func, const void *data, Evas_Smart_Cb del_cb)
{
   Elm_Tooltip *tt = NULL;
   Eina_Bool just_created;

   EINA_SAFETY_ON_NULL_GOTO(owner, error);
   EINA_SAFETY_ON_NULL_GOTO(eventarea, error);

   if (!func)
     {
        elm_object_tooltip_unset(eventarea);
        return;
     }

   tt = evas_object_data_get(eventarea, _tooltip_key);
   if (tt)
     {
        if (tt->owner != owner)
          {
             if (tt->owner != eventarea)
               evas_object_event_callback_del_full
                 (tt->owner, EVAS_CALLBACK_FREE, _elm_tooltip_obj_free_cb, tt);

             elm_widget_tooltip_del(tt->owner, tt);

             if (owner != eventarea)
               evas_object_event_callback_add
                 (owner, EVAS_CALLBACK_FREE, _elm_tooltip_obj_free_cb, tt);

             elm_widget_tooltip_add(tt->owner, tt);
          }

        if ((tt->func == func) && (tt->data == data) &&
            (tt->del_cb == del_cb))
          return;
        _elm_tooltip_data_clean(tt);
        just_created = EINA_FALSE;
     }
   else
     {
        tt = ELM_NEW(Elm_Tooltip);
        if (!tt) goto error;

        tt->owner = owner;
        tt->eventarea = eventarea;
        tt->evas = evas_object_evas_get(eventarea);
        evas_object_data_set(eventarea, _tooltip_key, tt);

        just_created = EINA_TRUE;

        evas_object_event_callback_add
          (eventarea, EVAS_CALLBACK_MOUSE_IN,
           _elm_tooltip_obj_mouse_in_cb, tt);
        evas_object_event_callback_add
          (eventarea, EVAS_CALLBACK_MOUSE_OUT,
           _elm_tooltip_obj_mouse_out_cb, tt);
        evas_object_event_callback_add
          (eventarea, EVAS_CALLBACK_FREE, _elm_tooltip_obj_free_cb, tt);

        if (owner != eventarea)
          evas_object_event_callback_add
            (owner, EVAS_CALLBACK_FREE, _elm_tooltip_obj_free_cb, tt);

        elm_widget_tooltip_add(tt->owner, tt);
     }

   tt->func = func;
   tt->data = data;
   tt->del_cb = del_cb;

   if (!just_created) _elm_tooltip_reconfigure_job_start(tt);
   return;

 error:
   if (del_cb) del_cb((void *)data, owner, NULL);
}

/**
 * Force show tooltip of object
 *
 * @param obj Target object
 *
 * Force show the tooltip and disable hide on mouse_out.
 * If another content is set as tooltip, the visible tooltip will hididen and
 * showed again with new content.
 * This can force show more than one tooltip at a time.
 *
 * @ingroup Tooltips
 */
EAPI void
elm_object_tooltip_show(Evas_Object *obj)
{
   ELM_TOOLTIP_GET_OR_RETURN(tt, obj);
   tt->visible_lock = EINA_TRUE;
   _elm_tooltip_show(tt);
}

/**
 * Force hide tooltip of object
 *
 * @param obj Target object
 *
 * Force hide the tooltip and (re)enable future mouse interations.
 *
 * @ingroup Tooltips
 */
EAPI void
elm_object_tooltip_hide(Evas_Object *obj)
{
   ELM_TOOLTIP_GET_OR_RETURN(tt, obj);
   tt->visible_lock = EINA_FALSE;
   _elm_tooltip_hide_anim_start(tt);
}

/**
 * Set the text to be shown in the tooltip object
 *
 * @param obj Target object
 * @param text The text to set in the content
 *
 * Setup the text as tooltip to object. The object can have only one tooltip,
 * so any previous tooltip data is removed.
 * This method call internaly the elm_tooltip_content_cb_set().
 *
 * @ingroup Tooltips
 */
EAPI void
elm_object_tooltip_text_set(Evas_Object *obj, const char *text)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   EINA_SAFETY_ON_NULL_RETURN(text);

   text = eina_stringshare_add(text);
   elm_object_tooltip_content_cb_set
     (obj, _elm_tooltip_label_create, text, _elm_tooltip_label_del_cb);
}

/**
 * Set the content to be shown in the tooltip object
 *
 * Setup the tooltip to object. The object can have only one tooltip,
 * so any previous tooltip data is removed. @p func(with @p data) will
 * be called every time that need show the tooltip and it should
 * return a valid Evas_Object. This object is then managed fully by
 * tooltip system and is deleted when the tooltip is gone.
 *
 * @param obj the object being attached a tooltip.
 * @param func the function used to create the tooltip contents.
 * @param data what to provide to @a func as callback data/context.
 * @param del_cb called when data is not needed anymore, either when
 *        another callback replaces @func, the tooltip is unset with
 *        elm_object_tooltip_unset() or the owner object @a obj
 *        dies. This callback receives as the first parameter the
 *        given @a data, and @c event_info is NULL.
 *
 * @ingroup Tooltips
 */
EAPI void
elm_object_tooltip_content_cb_set(Evas_Object *obj, Elm_Tooltip_Content_Cb func, const void *data, Evas_Smart_Cb del_cb)
{
   elm_object_sub_tooltip_content_cb_set(obj, obj, func, data, del_cb);
}

/**
 * Unset tooltip from object
 *
 * @param obj Target object
 *
 * Remove tooltip from object. The callback provided as del_cb to
 * elm_object_tooltip_content_cb_set() will be called to notify it is
 * not used anymore.
 *
 * @see elm_object_tooltip_content_cb_set()
 *
 * @ingroup Tooltips
 */
EAPI void
elm_object_tooltip_unset(Evas_Object *obj)
{
   ELM_TOOLTIP_GET_OR_RETURN(tt, obj);
   _elm_tooltip_unset(tt);
}

/**
 * Sets a different style for this object tooltip.
 *
 * @note before you set a style you should define a tooltip with
 *       elm_object_tooltip_content_cb_set() or
 *       elm_object_tooltip_text_set().
 *
 * @param obj an object with tooltip already set.
 * @param style the theme style to use (default, transparent, ...)
 */
EAPI void
elm_object_tooltip_style_set(Evas_Object *obj, const char *style)
{
   ELM_TOOLTIP_GET_OR_RETURN(tt, obj);
   if (!eina_stringshare_replace(&tt->style, style)) return;
   elm_tooltip_theme(tt);
}

/**
 * Get the style for this object tooltip.
 *
 * @param obj an object with tooltip already set.
 * @return style the theme style in use, defaults to "default". If the
 *         object does not have a tooltip set, then NULL is returned.
 */
EAPI const char *
elm_object_tooltip_style_get(const Evas_Object *obj)
{
   ELM_TOOLTIP_GET_OR_RETURN(tt, obj, NULL);
   return tt->style ? tt->style : "default";
}

/**
 * Get the configured tooltip delay
 *
 * This gets the globally configured tooltip delay in seconds
 *
 * @return The tooltip delay
 * @ingroup Tooltips
 */
EAPI double
elm_tooltip_delay_get(void)
{
   return _elm_config->tooltip_delay;
}

/**
 * Set the configured tooltip delay
 *
 * This sets the globally configured delay to tooltip
 *
 * @param delay The delay to show the tooltip
 * @return EINA_TRUE if value is valid and setted
 * @ingroup Tooltips
 */
EAPI Eina_Bool
elm_tooltip_delay_set(double delay)
{
   if (delay < 0.0) return EINA_FALSE;
   _elm_config->tooltip_delay = delay;
   return EINA_TRUE;
}
