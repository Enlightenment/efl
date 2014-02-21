#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_container.h"
#include "elm_interface_scrollable.h"

EAPI Eo_Op ELM_WIDGET_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_WIDGET_CLASS

#define MY_CLASS_NAME "Elm_Widget"
#define MY_CLASS_NAME_LEGACY "elm_widget"

#define ELM_WIDGET_DATA_GET(o, wd)                             \
  Elm_Widget_Smart_Data *wd = eo_data_scope_get(o, MY_CLASS)

#define API_ENTRY                                    \
  ELM_WIDGET_DATA_GET(obj, sd);                      \
  if ((!sd) || (!_elm_widget_is(obj)))
#define INTERNAL_ENTRY                               \
  ELM_WIDGET_DATA_GET(obj, sd);                      \
  if (!sd) return

#define ELM_WIDGET_FOCUS_GET(obj)                                    \
  ((_elm_access_auto_highlight_get()) ? (elm_widget_highlight_get(obj)) : \
                                        (elm_widget_focus_get(obj)))

const char SIG_WIDGET_FOCUSED[] = "focused";
const char SIG_WIDGET_UNFOCUSED[] = "unfocused";
const char SIG_WIDGET_LANG_CHANGED[] = "language,changed";
const char SIG_WIDGET_ACCESS_CHANGED[] = "access,changed";

typedef struct _Elm_Event_Cb_Data         Elm_Event_Cb_Data;
typedef struct _Elm_Label_Data            Elm_Label_Data;
typedef struct _Elm_Translate_String_Data Elm_Translate_String_Data;

struct _Elm_Event_Cb_Data
{
   Elm_Event_Cb func;
   const void  *data;
};

struct _Elm_Label_Data
{
   const char *part;
   const char *text;
};

struct _Elm_Translate_String_Data
{
   EINA_INLIST;
   Eina_Stringshare *id;
   Eina_Stringshare *domain;
   Eina_Stringshare *string;
   Eina_Bool   preset : 1;
};

/* local subsystem globals */
static unsigned int focus_order = 0;

static inline Eina_Bool
_elm_widget_is(const Evas_Object *obj)
{
   return eo_isa(obj, MY_CLASS);
}

static inline Eina_Bool
_is_focusable(Evas_Object *obj)
{
   API_ENTRY return EINA_FALSE;
   return sd->can_focus || (sd->child_can_focus);
}

static inline Eina_Bool
_elm_scrollable_is(const Evas_Object *obj)
{
   INTERNAL_ENTRY EINA_FALSE;
   return
      eo_isa(obj, ELM_SCROLLABLE_INTERFACE);
}

void
_elm_widget_item_highlight_in_theme(Evas_Object *obj, Elm_Object_Item *it)
{
   const char *str;

   if (!it) return;
   str = edje_object_data_get(VIEW(it), "focus_highlight");
   if ((str) && (!strcmp(str, "on")))
     elm_widget_highlight_in_theme_set(obj, EINA_TRUE);
   else
     elm_widget_highlight_in_theme_set(obj, EINA_FALSE);
}

void
_elm_widget_focus_highlight_start(const Evas_Object *obj)
{
   Evas_Object *top = elm_widget_top_get(obj);

   if (top && eo_isa(top, ELM_OBJ_WIN_CLASS))
     _elm_win_focus_highlight_start(top);
}

EAPI Eina_Bool
elm_widget_focus_highlight_enabled_get(const Evas_Object *obj)
{
   const Evas_Object *win = elm_widget_top_get(obj);

   if (win && eo_isa(win, ELM_OBJ_WIN_CLASS))
     return elm_win_focus_highlight_enabled_get(win);
   return EINA_FALSE;
}

/**
 * @internal
 *
 * Resets the mirrored mode from the system mirror mode for widgets that are in
 * automatic mirroring mode. This function does not call elm_widget_theme.
 *
 * @param obj The widget.
 * @param mirrored EINA_TRUE to set mirrored mode. EINA_FALSE to unset.
 */
static void
_elm_widget_mirrored_reload(Evas_Object *obj)
{
   API_ENTRY return;
   Eina_Bool mirrored = elm_config_mirrored_get();

   if (elm_widget_mirrored_automatic_get(obj) && (sd->is_mirrored != mirrored))
     {
        sd->is_mirrored = mirrored;
     }
}

static void
_elm_widget_on_focus_region(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   va_arg(*list, Evas_Coord *);
   va_arg(*list, Evas_Coord *);
   va_arg(*list, Evas_Coord *);
   va_arg(*list, Evas_Coord *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   WRN("The %s widget does not implement the \"on_focus_region\" function.",
       eo_class_name_get(eo_class_get(obj)));
}

static void
_parents_focus(Evas_Object *obj)
{
   for (; obj; obj = elm_widget_parent_get(obj))
     {
        INTERNAL_ENTRY;
        if (sd->focused) return;
        sd->focused = 1;
     }
}

static void
_parents_unfocus(Evas_Object *obj)
{
   for (; obj; obj = elm_widget_parent_get(obj))
     {
        INTERNAL_ENTRY;
        if (!sd->focused) return;
        sd->focused = 0;
     }
}

static void
_on_sub_obj_hide(void *data EINA_UNUSED,
              Evas *e EINA_UNUSED,
              Evas_Object *obj,
              void *event_info EINA_UNUSED)
{
   elm_widget_focus_hide_handle(obj);
}

static void
_on_sub_obj_del(void *data,
             Evas *e EINA_UNUSED,
             Evas_Object *obj,
             void *event_info EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET(data, sd);

   if (_elm_widget_is(obj))
     {
        if (elm_widget_focus_get(obj)) _parents_unfocus(sd->obj);
     }
   if (obj == sd->resize_obj)
     {
        /* already dels sub object */
        elm_widget_resize_object_set(sd->obj, NULL, EINA_TRUE);
        return;
     }
   else if (obj == sd->hover_obj)
     {
        sd->hover_obj = NULL;
        return;
     }
   else
     {
        if (!elm_widget_sub_object_del(sd->obj, obj))
          ERR("failed to remove sub object %p from %p\n", obj, sd->obj);
     }
}

static const Evas_Smart_Cb_Description _smart_callbacks[] =
{
   /* FIXME: complete later */
   {NULL, NULL}
};

static void
_obj_mouse_down(void *data,
                Evas *e EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info)
{
   ELM_WIDGET_DATA_GET(data, sd);
   Evas_Event_Mouse_Down *ev = event_info;
   if (!(ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD))
     sd->still_in = EINA_TRUE;
}

static void
_obj_mouse_move(void *data,
                Evas *e EINA_UNUSED,
                Evas_Object *obj,
                void *event_info)
{
   ELM_WIDGET_DATA_GET(data, sd);
   Evas_Event_Mouse_Move *ev = event_info;
   if (sd->still_in)
     {
        if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
          sd->still_in = EINA_FALSE;
        else
          {
             Evas_Coord x, y, w, h;
             evas_object_geometry_get(obj, &x, &y, &w, &h);
             if ((ev->cur.canvas.x < x) || (ev->cur.canvas.y < y) ||
                 (ev->cur.canvas.x >= (x + w)) || (ev->cur.canvas.y >= (y + h)))
               sd->still_in = EINA_FALSE;
          }
     }
}

static void
_obj_mouse_up(void *data,
              Evas *e EINA_UNUSED,
              Evas_Object *obj,
              void *event_info EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET(data, sd);
   if (sd->still_in)
     elm_widget_focus_mouse_up_handle(obj);
   sd->still_in = EINA_FALSE;
}

static void
_elm_widget_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{

   Elm_Widget_Smart_Data *priv = _pd;

   priv->obj = obj;
   priv->mirrored_auto_mode = EINA_TRUE; /* will follow system locale
                                          * settings */
   elm_widget_can_focus_set(obj, EINA_TRUE);
   priv->is_mirrored = elm_config_mirrored_get();

   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_DOWN,
                                  _obj_mouse_down, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_MOVE,
                                  _obj_mouse_move, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_UP,
                                  _obj_mouse_up, obj);
   /* just a helper for inheriting classes */
   if (priv->resize_obj)
     {
        Evas_Object *r_obj = priv->resize_obj;
        priv->resize_obj = NULL;

        elm_widget_resize_object_set(obj, r_obj, EINA_TRUE);
     }
}

static void
_if_focused_revert(Evas_Object *obj,
                   Eina_Bool can_focus_only)
{
   Evas_Object *top;
   Evas_Object *newest = NULL;
   unsigned int newest_focus_order = 0;

   INTERNAL_ENTRY;

   if (!sd->focused) return;
   if (!sd->parent_obj) return;

   top = elm_widget_top_get(sd->parent_obj);
   if (top)
     {
        newest = elm_widget_newest_focus_order_get
           (top, &newest_focus_order, can_focus_only);
        if (newest)
          {
             elm_object_focus_set(newest, EINA_FALSE);
             elm_object_focus_set(newest, EINA_TRUE);
          }
     }
}

static void
_elm_widget_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Object *sobj;
   Elm_Translate_String_Data *ts;
   Elm_Event_Cb_Data *ecb;

   Elm_Widget_Smart_Data *sd = _pd;

   if (sd->hover_obj)
     {
        /* detach it from us */
        evas_object_event_callback_del_full
          (sd->hover_obj, EVAS_CALLBACK_DEL, _on_sub_obj_del, obj);
        sd->hover_obj = NULL;
     }

   while (sd->subobjs)
     {
        sobj = eina_list_data_get(sd->subobjs);

        /* let the objects clean-up themselves and get rid of this list */
        if (!elm_widget_sub_object_del(obj, sobj))
          {
             ERR("failed to remove sub object %p from %p\n", sobj, obj);
             sd->subobjs = eina_list_remove_list
                 (sd->subobjs, sd->subobjs);
          }
        evas_object_del(sobj);
     }
   sd->tooltips = eina_list_free(sd->tooltips); /* should be empty anyway */
   sd->cursors = eina_list_free(sd->cursors); /* should be empty anyway */
   while (sd->translate_strings)
     {
        ts = EINA_INLIST_CONTAINER_GET(sd->translate_strings,
                                       Elm_Translate_String_Data);
        eina_stringshare_del(ts->id);
        eina_stringshare_del(ts->domain);
        eina_stringshare_del(ts->string);
        sd->translate_strings = eina_inlist_remove(sd->translate_strings,
                                                   sd->translate_strings);
        free(ts);
     }

   EINA_LIST_FREE(sd->event_cb, ecb)
      free(ecb);

   eina_stringshare_del(sd->style);
   if (sd->theme) elm_theme_free(sd->theme);
   _if_focused_revert(obj, EINA_TRUE);
   elm_widget_focus_custom_chain_unset(obj);
   eina_stringshare_del(sd->access_info);
   evas_object_smart_data_set(obj, NULL);
}

static void
_smart_reconfigure(Elm_Widget_Smart_Data *sd)
{
   if (sd->resize_obj)
     {
        evas_object_move(sd->resize_obj, sd->x, sd->y);
        evas_object_resize(sd->resize_obj, sd->w, sd->h);
     }
   if (sd->hover_obj)
     {
        evas_object_move(sd->hover_obj, sd->x, sd->y);
        evas_object_resize(sd->hover_obj, sd->w, sd->h);
     }
}

static void
_elm_widget_smart_move(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord x = va_arg(*list, Evas_Coord);
   Evas_Coord y = va_arg(*list, Evas_Coord);
   Elm_Widget_Smart_Data *sd = _pd;

   sd->x = x;
   sd->y = y;

   _smart_reconfigure(sd);
}

static void
_elm_widget_smart_resize(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord w = va_arg(*list, Evas_Coord);
   Evas_Coord h = va_arg(*list, Evas_Coord);
   Elm_Widget_Smart_Data *sd = _pd;

   sd->w = w;
   sd->h = h;

   _smart_reconfigure(sd);
}

static void
_elm_widget_smart_show(Eo *obj, void *_pd EINA_UNUSED, va_list *params_list EINA_UNUSED)
{
   Eina_Iterator *it;
   Evas_Object *o;

   it = evas_object_smart_iterator_new(obj);
   EINA_ITERATOR_FOREACH(it, o)
     {
       if (evas_object_data_get(o, "_elm_leaveme")) continue;
       evas_object_show(o);
     }
   eina_iterator_free(it);
}

static void
_elm_widget_smart_hide(Eo *obj, void *_pd EINA_UNUSED, va_list *params_list EINA_UNUSED)
{
   Eina_Iterator *it;
   Evas_Object *o;

   it = evas_object_smart_iterator_new(obj);
   EINA_ITERATOR_FOREACH(it, o)
     {
        if (evas_object_data_get(o, "_elm_leaveme")) continue;
        evas_object_hide(o);
     }
   eina_iterator_free(it);
}

static void
_elm_widget_smart_color_set(Eo *obj, void *_pd EINA_UNUSED, va_list *params_list)
{
   int r = va_arg(*params_list, int);
   int g = va_arg(*params_list, int);
   int b = va_arg(*params_list, int);
   int a = va_arg(*params_list, int);
   Eina_Iterator *it;
   Evas_Object *o;

   it = evas_object_smart_iterator_new(obj);
   EINA_ITERATOR_FOREACH(it, o)
     {
       if (evas_object_data_get(o, "_elm_leaveme")) continue;
       evas_object_color_set(o, r, g, b, a);
     }
   eina_iterator_free(it);
}

static void
_elm_widget_smart_clip_set(Eo *obj, void *_pd EINA_UNUSED, va_list *params_list)
{
   Evas_Object *clip = va_arg(*params_list, Evas_Object *);
   Eina_Iterator *it;
   Evas_Object *o;

   it = evas_object_smart_iterator_new(obj);
   EINA_ITERATOR_FOREACH(it, o)
     {
       if (evas_object_data_get(o, "_elm_leaveme")) continue;
       evas_object_clip_set(o, clip);
     }
   eina_iterator_free(it);
}

static void
_elm_widget_smart_clip_unset(Eo *obj, void *_pd EINA_UNUSED, va_list *params_list EINA_UNUSED)
{
   Eina_Iterator *it;
   Evas_Object *o;

   it = evas_object_smart_iterator_new(obj);
   EINA_ITERATOR_FOREACH(it, o)
     {
       if (evas_object_data_get(o, "_elm_leaveme")) continue;
       evas_object_clip_unset(o);
     }
   eina_iterator_free(it);
}

static void
_elm_widget_smart_calculate(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *params_list EINA_UNUSED)
{
   /* a NO-OP, on the base */
}

static void
_elm_widget_smart_member_add(Eo *obj, void *_pd EINA_UNUSED, va_list *params_list)
{
   int r, g, b, a;
   Evas_Object *child = va_arg(*params_list, Evas_Object *);
   eo_do_super(obj, MY_CLASS, evas_obj_smart_member_add(child));

   if (evas_object_data_get(child, "_elm_leaveme")) return;

   evas_object_color_get(obj, &r, &g, &b, &a);
   evas_object_color_set(child, r, g, b, a);

   evas_object_clip_set(child, evas_object_clip_get(obj));

   if (evas_object_visible_get(obj))
     evas_object_show(child);
   else
     evas_object_hide(child);
}

static void
_elm_widget_smart_member_del(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *child = va_arg(*list, Evas_Object *);
   if (!evas_object_data_get(child, "_elm_leaveme"))
      evas_object_clip_unset(child);
   eo_do_super(obj, MY_CLASS, evas_obj_smart_member_del(child));
}

// internal funcs
/**
 * @internal
 *
 * Check if the widget has its own focus next function.
 *
 * @param obj The widget.
 * @return focus next function is implemented/unimplemented.
 * (@c EINA_TRUE = implemented/@c EINA_FALSE = unimplemented.)
 */
static inline Eina_Bool
_elm_widget_focus_chain_manager_is(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;

   Eina_Bool manager_is = EINA_FALSE;
   eo_do((Eo *)obj, elm_wdg_focus_next_manager_is(&manager_is));
   return manager_is;
}

static inline Eina_Bool
_elm_widget_focus_direction_manager_is(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;

   Eina_Bool manager_is = EINA_FALSE;
   eo_do((Eo *)obj, elm_wdg_focus_direction_manager_is(&manager_is));
   return manager_is;
}

static void
_propagate_x_drag_lock(Evas_Object *obj,
                       int dir)
{
   INTERNAL_ENTRY;
   if (sd->parent_obj)
     {
        ELM_WIDGET_DATA_GET(sd->parent_obj, sd2);
        if (sd2)
          {
             sd2->child_drag_x_locked += dir;
             _propagate_x_drag_lock(sd->parent_obj, dir);
          }
     }
}

static void
_propagate_y_drag_lock(Evas_Object *obj,
                       int dir)
{
   INTERNAL_ENTRY;
   if (sd->parent_obj)
     {
        ELM_WIDGET_DATA_GET(sd->parent_obj, sd2);
        if (sd2)
          {
             sd2->child_drag_y_locked += dir;
             _propagate_y_drag_lock(sd->parent_obj, dir);
          }
     }
}

static void
_propagate_event(void *data,
                 Evas *e EINA_UNUSED,
                 Evas_Object *obj,
                 void *event_info)
{
   INTERNAL_ENTRY;
   Evas_Callback_Type type = (Evas_Callback_Type)(uintptr_t)data;
   Evas_Event_Flags *event_flags = NULL;

   switch (type)
     {
      case EVAS_CALLBACK_KEY_DOWN:
           {
              Evas_Event_Key_Down *ev = event_info;
              event_flags = &(ev->event_flags);
           }
         break;

      case EVAS_CALLBACK_KEY_UP:
           {
              Evas_Event_Key_Up *ev = event_info;
              event_flags = &(ev->event_flags);
           }
         break;

      case EVAS_CALLBACK_MOUSE_WHEEL:
           {
              Evas_Event_Mouse_Wheel *ev = event_info;
              event_flags = &(ev->event_flags);
           }
         break;

      default:
         break;
     }

   elm_widget_event_propagate(obj, type, event_info, event_flags);
}

EAPI void
elm_widget_focus_region_show(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj);
   eo_do((Eo *) obj, elm_wdg_focus_region_show());
}
/**
 * @internal
 *
 * If elm_widget_focus_region_get() returns EINA_FALSE, this function will
 * ignore region show action.
 */
static void
_elm_widget_focus_region_show(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Evas_Coord x, y, w, h, ox, oy;
   Evas_Object *o;

   o = elm_widget_parent_get(obj);
   if (!o) return;

   if (!elm_widget_focus_region_get(obj, &x, &y, &w, &h))
     return;

   evas_object_geometry_get(obj, &ox, &oy, NULL, NULL);

   while (o)
     {
        Evas_Coord px, py;

        if (_elm_scrollable_is(o) && !elm_widget_disabled_get(o))
          {
             eo_do(o, elm_scrollable_interface_content_region_show(x, y, w, h));

             if (!elm_widget_focus_region_get(o, &x, &y, &w, &h))
               {
                  o = elm_widget_parent_get(o);
                  continue;
               }
          }
        else
          {
             evas_object_geometry_get(o, &px, &py, NULL, NULL);
             x += ox - px;
             y += oy - py;
             ox = px;
             oy = py;
          }
        o = elm_widget_parent_get(o);
     }
}

EAPI Eina_Bool
elm_widget_focus_highlight_style_set(Evas_Object *obj, const char *style)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_wdg_focus_highlight_style_set(style, &ret));
   return ret;
}

static void
_elm_widget_focus_highlight_style_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *style = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Elm_Widget_Smart_Data *sd = _pd;
   if (eina_stringshare_replace(&sd->focus_highlight_style, style))
     {
        if (ret) *ret = EINA_TRUE;
        return;
     }
}

EAPI const char *
elm_widget_focus_highlight_style_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_wdg_focus_highlight_style_get(&ret));
   return ret;
}

static void
_elm_widget_focus_highlight_style_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **ret = va_arg(*list, const char **);
   Elm_Widget_Smart_Data *sd = _pd;
   *ret = sd->focus_highlight_style;
}

static void
_parent_focus(Evas_Object *obj)
{
   API_ENTRY return;

   if (sd->focused) return;

   Evas_Object *o = elm_widget_parent_get(obj);
   sd->focus_order_on_calc = EINA_TRUE;

   if (o) _parent_focus(o);

   if (!sd->focus_order_on_calc)
     return;  /* we don't want to override it if by means of any of the
                 callbacks below one gets to calculate our order
                 first. */

   focus_order++;
   sd->focus_order = focus_order;

   if (sd->top_win_focused)
     {
        sd->focused = EINA_TRUE;
        eo_do(obj, elm_wdg_on_focus(NULL));
        elm_widget_focus_region_show(obj);
     }
   sd->focus_order_on_calc = EINA_FALSE;

   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     _elm_access_highlight_set(obj);
}

static void
_elm_object_focus_chain_del_cb(void *data,
                               Evas *e EINA_UNUSED,
                               Evas_Object *obj,
                               void *event_info EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET(data, sd);

   sd->focus_chain = eina_list_remove(sd->focus_chain, obj);
}

EAPI void
elm_widget_parent_set(Evas_Object *obj,
                      Evas_Object *parent)
{
   eo_do(obj, elm_wdg_parent_set(parent));
}

static void
_elm_widget_parent_set(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   va_arg(*list, Evas_Object *);
}

EAPI Eina_Bool
elm_widget_api_check(int ver)
{
   if (ver != ELM_INTERNAL_API_VERSION)
     {
        CRI("Elementary widget api versions do not match");
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

EAPI Eina_Bool
elm_widget_access(Evas_Object *obj,
                  Eina_Bool is_access)
{
   const Eina_List *l;
   Evas_Object *child;
   Eina_Bool ret = EINA_TRUE;

   API_ENTRY return EINA_FALSE;
   EINA_LIST_FOREACH(sd->subobjs, l, child)
     ret &= elm_widget_access(child, is_access);

   eo_do(obj, elm_wdg_access(is_access));
   evas_object_smart_callback_call(obj, SIG_WIDGET_ACCESS_CHANGED, NULL);

   return ret;
}

static void
_elm_widget_access(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   va_arg(*list, int);
}

EAPI Eina_Bool
elm_widget_theme(Evas_Object *obj)
{
   const Eina_List *l;
   Evas_Object *child;
   Elm_Tooltip *tt;
   Elm_Cursor *cur;
   Eina_Bool ret = EINA_TRUE;

   API_ENTRY return EINA_FALSE;

   EINA_LIST_FOREACH(sd->subobjs, l, child)
     if (_elm_widget_is(child)) ret &= elm_widget_theme(child);
   if (sd->hover_obj) ret &= elm_widget_theme(sd->hover_obj);

   EINA_LIST_FOREACH(sd->tooltips, l, tt)
     elm_tooltip_theme(tt);
   EINA_LIST_FOREACH(sd->cursors, l, cur)
     elm_cursor_theme(cur);

   Eina_Bool ret2;
   eo_do(obj, elm_wdg_theme_apply(&ret2));
   ret &= ret2;

   return ret;
}

EAPI void
elm_widget_theme_specific(Evas_Object *obj,
                          Elm_Theme *th,
                          Eina_Bool force)
{
   const Eina_List *l;
   Evas_Object *child;
   Elm_Tooltip *tt;
   Elm_Cursor *cur;
   Elm_Theme *th2, *thdef;

   API_ENTRY return;

   thdef = elm_theme_default_get();
   if (!th) th = thdef;
   if (!force)
     {
        th2 = sd->theme;
        if (!th2) th2 = thdef;
        while (th2)
          {
             if (th2 == th)
               {
                  force = EINA_TRUE;
                  break;
               }
             if (th2 == thdef) break;
             th2 = th2->ref_theme;
             if (!th2) th2 = thdef;
          }
     }
   if (!force) return;
   EINA_LIST_FOREACH(sd->subobjs, l, child)
     elm_widget_theme_specific(child, th, force);
   if (sd->hover_obj) elm_widget_theme(sd->hover_obj);
   EINA_LIST_FOREACH(sd->tooltips, l, tt)
     elm_tooltip_theme(tt);
   EINA_LIST_FOREACH(sd->cursors, l, cur)
     elm_cursor_theme(cur);
   eo_do(obj, elm_wdg_theme_apply(NULL));
}

static void
_elm_widget_theme_func(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   _elm_widget_mirrored_reload(obj);

   elm_widget_disabled_set(obj, elm_widget_disabled_get(obj));

   if (ret) *ret = EINA_TRUE;
}

/**
 * @internal
 *
 * Returns the widget's mirrored mode.
 *
 * @param obj The widget.
 * @return mirrored mode of the object.
 *
 **/
EAPI Eina_Bool
elm_widget_mirrored_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;

   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_wdg_mirrored_get(&ret));
   return ret;
}

static void
_elm_widget_mirrored_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Widget_Smart_Data *sd = _pd;
   *ret = sd->is_mirrored;
}

/**
 * @internal
 *
 * Sets the widget's mirrored mode.
 *
 * @param obj The widget.
 * @param mirrored EINA_TRUE to set mirrored mode. EINA_FALSE to unset.
 */
EAPI void
elm_widget_mirrored_set(Evas_Object *obj,
                        Eina_Bool mirrored)
{
   ELM_WIDGET_CHECK(obj);

   eo_do(obj, elm_wdg_mirrored_set(mirrored));
}

static void
_elm_widget_mirrored_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool mirrored = va_arg(*list, int);

   Elm_Widget_Smart_Data *sd = _pd;

   mirrored = !!mirrored;

   if (sd->is_mirrored == mirrored) return;

   sd->is_mirrored = mirrored;
   elm_widget_theme(obj);
}

/**
 * Returns the widget's mirrored mode setting.
 *
 * @param obj The widget.
 * @return mirrored mode setting of the object.
 *
 **/
EAPI Eina_Bool
elm_widget_mirrored_automatic_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_wdg_mirrored_automatic_get(&ret));
   return ret;
}

static void
_elm_widget_mirrored_automatic_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Widget_Smart_Data *sd = _pd;
   *ret = sd->mirrored_auto_mode;
}

/**
 * @internal
 *
 * Sets the widget's mirrored mode setting.
 * When widget in automatic mode, it follows the system mirrored mode set by
 * elm_mirrored_set().
 * @param obj The widget.
 * @param automatic EINA_TRUE for auto mirrored mode. EINA_FALSE for manual.
 */
EAPI void
elm_widget_mirrored_automatic_set(Evas_Object *obj,
                                  Eina_Bool automatic)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_mirrored_automatic_set(automatic));
}

static void
_elm_widget_mirrored_automatic_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool automatic = va_arg(*list, int);
   Elm_Widget_Smart_Data *sd = _pd;
   if (sd->mirrored_auto_mode != automatic)
     {
        sd->mirrored_auto_mode = automatic;

        if (automatic)
          {
             elm_widget_mirrored_set(obj, elm_config_mirrored_get());
          }
     }
}

EAPI void
elm_widget_on_show_region_hook_set(Evas_Object *obj,
                                   void (*func)(void *data,
                                                Evas_Object *obj),
                                   void *data)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_on_show_region_hook_set(func, data));
}

static void
_elm_widget_on_show_region_hook_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   region_hook_func_type func = va_arg(*list, region_hook_func_type);
   void *data = va_arg(*list, void *);

   Elm_Widget_Smart_Data *sd = _pd;
   sd->on_show_region = func;
   sd->on_show_region_data = data;
}

/*
 * @internal
 *
 * Add myself as a sub object of parent object
 *
 * @see elm_widget_sub_object_add()
 */
EAPI Eina_Bool
elm_widget_sub_object_parent_add(Evas_Object *sobj)
{
   Eina_Bool ret = EINA_FALSE;
   Eo *parent;

   eo_do(sobj, eo_parent_get(&parent));
   eo_do(parent, elm_wdg_sub_object_add(sobj, &ret));

   return ret;
}

/*
 * @internal
 *
 * Add sobj to obj's sub object.
 *
 * What does elementary sub object mean? This is unique in elementary, it
 * handles overall elementary policies between parent and sub objects.
 *   focus, access, deletion, theme, scale, mirror, scrollable child get,
 *   translate, name find, display mode set, orientation set, tree dump
 *   AUTOMATICALLY.
 *
 * @see elm_widget_sub_object_parent_add()
 */
EAPI Eina_Bool
elm_widget_sub_object_add(Evas_Object *obj,
                          Evas_Object *sobj)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;
   EINA_SAFETY_ON_TRUE_RETURN_VAL(obj == sobj, EINA_FALSE);

   if (!sobj) goto err;

   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_wdg_sub_object_add(sobj, &ret));
   // FIXME: better handle the error case in the eo called function than here.
   if (ret) return EINA_TRUE;

err:
   ERR("could not add %p as sub object of %p", obj, sobj);
   return EINA_FALSE;
}

static void
_elm_widget_sub_object_add(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *sobj = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   /* NOTE: In the following two lines, 'sobj' is correct. Do not change it.
    * Due to elementary's scale policy, scale and pscale can be different in
    * some cases. This happens when sobj's previous parent and new parent have
    * different scale value.
    * For example, if sobj's previous parent's scale is 5 and new parent's scale
    * is 2 while sobj's scale is 0. Then 'pscale' is 5 and 'scale' is 2. So we
    * need to reset sobj's scale to 5.
    * Note that each widget's scale is 0 by default.
    */
   double scale, pscale = elm_widget_scale_get(sobj);
   Elm_Theme *th, *pth = elm_widget_theme_get(sobj);
   Eina_Bool mirrored, pmirrored = elm_widget_mirrored_get(obj);

   Elm_Widget_Smart_Data *sd = _pd;
   EINA_SAFETY_ON_TRUE_RETURN(obj == sobj);

   if (sobj == sd->parent_obj)
     {
        /* in this case, sobj must be an elm widget, or something
         * very wrong is happening */
        if (!_elm_widget_is(sobj)) return;

        if (!elm_widget_sub_object_del(sobj, obj)) return;
        WRN("You passed a parent object of obj = %p as the sub object = %p!",
            obj, sobj);
     }

   if (_elm_widget_is(sobj))
     {
        ELM_WIDGET_DATA_GET(sobj, sdc);

        if (sdc->parent_obj == obj) goto end;
        if (sdc->parent_obj)
          {
             if (!elm_widget_sub_object_del(sdc->parent_obj, sobj))
               return;
          }
        sdc->parent_obj = obj;
        sdc->orient_mode = sd->orient_mode;
        _elm_widget_top_win_focused_set(sobj, sd->top_win_focused);

        /* update child focusable-ness on self and parents, now that a
         * focusable child got in */
        if (!sd->child_can_focus && (_is_focusable(sobj)))
          {
             Elm_Widget_Smart_Data *sdp = sd;

             sdp->child_can_focus = EINA_TRUE;
             while (sdp->parent_obj)
               {
                  sdp = eo_data_scope_get(sdp->parent_obj, MY_CLASS);

                  if (sdp->child_can_focus) break;

                  sdp->child_can_focus = EINA_TRUE;
               }
          }
     }
   else
     {
        void *data = evas_object_data_get(sobj, "elm-parent");

        if (data)
          {
             if (data == obj) goto end;
             if (!elm_widget_sub_object_del(data, sobj)) return;
          }
     }
   sd->subobjs = eina_list_append(sd->subobjs, sobj);
   evas_object_data_set(sobj, "elm-parent", obj);
   evas_object_event_callback_add
     (sobj, EVAS_CALLBACK_DEL, _on_sub_obj_del, obj);
   if (_elm_widget_is(sobj))
     {
        ELM_WIDGET_DATA_GET(sobj, sdc);

        evas_object_event_callback_add
          (sobj, EVAS_CALLBACK_HIDE, _on_sub_obj_hide, NULL);

        scale = elm_widget_scale_get(sobj);
        th = elm_widget_theme_get(sobj);
        mirrored = elm_widget_mirrored_get(sobj);

        if (!sdc->on_create)
          {
             if ((scale != pscale) || (th != pth) || (pmirrored != mirrored))
               elm_widget_theme(sobj);
          }

        if (elm_widget_focus_get(sobj)) _parents_focus(obj);
     }

   elm_widget_display_mode_set(sobj,
                               evas_object_size_hint_display_mode_get(obj));
end:
   if (ret) *ret = EINA_TRUE;
}

EAPI Eina_Bool
elm_widget_sub_object_del(Evas_Object *obj,
                          Evas_Object *sobj)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;
   EINA_SAFETY_ON_TRUE_RETURN_VAL(obj == sobj, EINA_FALSE);

   if (!sobj) return EINA_FALSE;

   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_wdg_sub_object_del(sobj, &ret));
   return ret;
}

static void
_elm_widget_sub_object_del(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *sobj = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Evas_Object *sobj_parent;

   if (!sobj) return;

   Elm_Widget_Smart_Data *sd = _pd;
   EINA_SAFETY_ON_TRUE_RETURN(obj == sobj);

   sobj_parent = evas_object_data_del(sobj, "elm-parent");
   if (sobj_parent != obj)
     {
        static int abort_on_warn = -1;

        ERR("removing sub object %p (%s) from parent %p (%s), "
            "but elm-parent is different %p (%s)!",
            sobj, elm_widget_type_get(sobj), obj, elm_widget_type_get(obj),
            sobj_parent, elm_widget_type_get(sobj_parent));

        if (EINA_UNLIKELY(abort_on_warn == -1))
          {
             if (getenv("ELM_ERROR_ABORT")) abort_on_warn = 1;
             else abort_on_warn = 0;
          }
        if (abort_on_warn == 1) abort();

        return;
     }

   if (_elm_widget_is(sobj))
     {
        if (elm_widget_focus_get(sobj))
          {
             elm_widget_tree_unfocusable_set(sobj, EINA_TRUE);
             elm_widget_tree_unfocusable_set(sobj, EINA_FALSE);
          }
        if ((sd->child_can_focus) && (_is_focusable(sobj)))
          {
             Evas_Object *parent = obj;

             /* update child focusable-ness on self and parents, now that a
              * focusable child is gone */
             while (parent)
               {
                  const Eina_List *l;
                  Evas_Object *subobj;

                  ELM_WIDGET_DATA_GET(parent, sdp);

                  sdp->child_can_focus = EINA_FALSE;
                  EINA_LIST_FOREACH(sdp->subobjs, l, subobj)
                    {
                       if ((subobj != sobj) && (_is_focusable(subobj)))
                         {
                            sdp->child_can_focus = EINA_TRUE;
                            break;
                         }
                    }

                  /* break again, child_can_focus went back to
                   * original value */
                  if (sdp->child_can_focus) break;
                  parent = sdp->parent_obj;
               }
          }

        ELM_WIDGET_DATA_GET(sobj, sdc);
        sdc->parent_obj = NULL;
     }

   if (sd->resize_obj == sobj) sd->resize_obj = NULL;

   sd->subobjs = eina_list_remove(sd->subobjs, sobj);

   evas_object_event_callback_del_full
     (sobj, EVAS_CALLBACK_DEL, _on_sub_obj_del, obj);
   if (_elm_widget_is(sobj))
     evas_object_event_callback_del_full
       (sobj, EVAS_CALLBACK_HIDE, _on_sub_obj_hide, NULL);

   if (ret) *ret = EINA_TRUE;
}

/*
 * @internal
 *
 * a resize object is added to and deleted from the smart member and the sub object
 * of the parent if the third argument, Eina_Bool sub_obj, is set as EINA_TRUE.
 */
EAPI void
elm_widget_resize_object_set(Evas_Object *obj,
                             Evas_Object *sobj,
                             Eina_Bool sub_obj)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_resize_object_set(sobj, sub_obj));
}

static void
_elm_widget_resize_object_set(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *sobj = va_arg(*list, Evas_Object *);
   Eina_Bool sub_obj = va_arg(*list, int);
   Evas_Object *parent;

   Elm_Widget_Smart_Data *sd = _pd;

   if (sd->resize_obj == sobj) return;

   // orphan previous resize obj
   if (sd->resize_obj && sub_obj)
     {
        evas_object_clip_unset(sd->resize_obj);
        evas_object_smart_member_del(sd->resize_obj);

        if (_elm_widget_is(sd->resize_obj))
          {
             if (elm_widget_focus_get(sd->resize_obj)) _parents_unfocus(obj);
          }
        elm_widget_sub_object_del(obj, sd->resize_obj);
     }

   sd->resize_obj = sobj;
   if (!sobj) return;

   // orphan new resize obj
   parent = evas_object_data_get(sobj, "elm-parent");
   if (parent && parent != obj)
     {
        ELM_WIDGET_DATA_GET(parent, sdp);

        /* should be there, just being paranoid */
        if (sdp)
          {
             if (sdp->resize_obj == sobj)
               elm_widget_resize_object_set(parent, NULL, sub_obj);
             else if (sub_obj)
               elm_widget_sub_object_del(parent, sobj);
          }
     }
   if (sub_obj)
     {
        elm_widget_sub_object_add(obj, sobj);
        evas_object_smart_member_add(sobj, obj);
     }

   _smart_reconfigure(sd);
}

/*
 * @internal
 *
 * WARNING: the programmer is responsible, in the scenario of
 * exchanging a hover object, of cleaning the old hover "target"
 * before
 */
EAPI void
elm_widget_hover_object_set(Evas_Object *obj,
                            Evas_Object *sobj)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_hover_object_set(sobj));
}

static void
_elm_widget_hover_object_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object *sobj = va_arg(*list, Evas_Object *);
   Elm_Widget_Smart_Data *sd = _pd;

   if (sd->hover_obj)
     {
        evas_object_event_callback_del_full(sd->hover_obj, EVAS_CALLBACK_DEL,
                                            _on_sub_obj_del, obj);
     }
   sd->hover_obj = sobj;
   if (sd->hover_obj)
     {
        evas_object_event_callback_add(sobj, EVAS_CALLBACK_DEL,
                                       _on_sub_obj_del, obj);
        _smart_reconfigure(sd);
     }
}

EAPI void
elm_widget_can_focus_set(Evas_Object *obj,
                         Eina_Bool can_focus)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_can_focus_set(can_focus));
}

static void
_elm_widget_can_focus_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool can_focus = va_arg(*list, int);

   Elm_Widget_Smart_Data *sd = _pd;
   can_focus = !!can_focus;

   if (sd->can_focus == can_focus) return;
   sd->can_focus = can_focus;
   if (sd->can_focus)
     {
        /* update child_can_focus of parents */
        Evas_Object *o = obj;

        for (;;)
          {
             o = elm_widget_parent_get(o);
             if (!o) break;
             sd = eo_data_scope_get(o, MY_CLASS);
             if (!sd || sd->child_can_focus) break;
             sd->child_can_focus = EINA_TRUE;
          }

        evas_object_event_callback_add(obj, EVAS_CALLBACK_KEY_DOWN,
                                       _propagate_event,
                                       (void *)(uintptr_t)EVAS_CALLBACK_KEY_DOWN);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_KEY_UP,
                                       _propagate_event,
                                       (void *)(uintptr_t)EVAS_CALLBACK_KEY_UP);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_WHEEL,
                                       _propagate_event,
                                       (void *)(uintptr_t)EVAS_CALLBACK_MOUSE_WHEEL);
     }
   else
     {
        evas_object_event_callback_del(obj, EVAS_CALLBACK_KEY_DOWN,
                                       _propagate_event);
        evas_object_event_callback_del(obj, EVAS_CALLBACK_KEY_UP,
                                       _propagate_event);
        evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_WHEEL,
                                       _propagate_event);
     }
}

EAPI Eina_Bool
elm_widget_can_focus_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_wdg_can_focus_get(&ret));
   return ret;
}

static void
_elm_widget_can_focus_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Widget_Smart_Data *sd = _pd;
   *ret = sd->can_focus;
}

EAPI Eina_Bool
elm_widget_child_can_focus_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;
   Eina_Bool ret =  EINA_FALSE;
   eo_do((Eo *) obj, elm_wdg_child_can_focus_get(&ret));
   return ret;
}

static void
_elm_widget_child_can_focus_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Widget_Smart_Data *sd = _pd;
   *ret = sd->child_can_focus;
}

/**
 * @internal
 *
 * This API makes the widget object and its children to be unfocusable.
 *
 * This API can be helpful for an object to be deleted.
 * When an object will be deleted soon, it and its children may not
 * want to get focus (by focus reverting or by other focus controls).
 * Then, just use this API before deleting.
 *
 * @param obj The widget root of sub-tree
 * @param tree_unfocusable If true, set the object sub-tree as unfocusable
 *
 * @ingroup Widget
 */
EAPI void
elm_widget_tree_unfocusable_set(Evas_Object *obj,
                                Eina_Bool tree_unfocusable)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_tree_unfocusable_set(tree_unfocusable));
}

static void
_elm_widget_tree_unfocusable_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool tree_unfocusable = va_arg(*list, int);
   Elm_Widget_Smart_Data *sd = _pd;

   tree_unfocusable = !!tree_unfocusable;
   if (sd->tree_unfocusable == tree_unfocusable) return;
   sd->tree_unfocusable = tree_unfocusable;
   elm_widget_focus_tree_unfocusable_handle(obj);
}

/**
 * @internal
 *
 * This returns true, if the object sub-tree is unfocusable.
 *
 * @param obj The widget root of sub-tree
 * @return EINA_TRUE if the object sub-tree is unfocusable
 *
 * @ingroup Widget
 */
EAPI Eina_Bool
elm_widget_tree_unfocusable_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_wdg_tree_unfocusable_get(&ret));
   return ret;
}

static void
_elm_widget_tree_unfocusable_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Widget_Smart_Data *sd = _pd;
   *ret = sd->tree_unfocusable;
}

/**
 * @internal
 *
 * Get the list of focusable child objects.
 *
 * This function retruns list of child objects which can get focus.
 *
 * @param obj The parent widget
 * @retrun list of focusable child objects.
 *
 * @ingroup Widget
 */
EAPI Eina_List *
elm_widget_can_focus_child_list_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) NULL;
   Eina_List *ret = NULL;
   eo_do((Eo *) obj, elm_wdg_can_focus_child_list_get(&ret));
   return ret;
}

static void
_elm_widget_can_focus_child_list_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_List **ret = va_arg(*list, Eina_List **);

   const Eina_List *l;
   Eina_List *child_list = NULL;
   Evas_Object *child;
   Elm_Widget_Smart_Data *sd = _pd;

   if (sd->subobjs)
     {
        EINA_LIST_FOREACH(sd->subobjs, l, child)
          {
             if ((elm_widget_can_focus_get(child)) &&
                 (evas_object_visible_get(child)) &&
                 (!elm_widget_disabled_get(child)))
               child_list = eina_list_append(child_list, child);
             else if (elm_widget_is(child))
               {
                  Eina_List *can_focus_list;
                  can_focus_list = elm_widget_can_focus_child_list_get(child);
                  if (can_focus_list)
                    child_list = eina_list_merge(child_list, can_focus_list);
               }
          }
     }
   *ret = child_list;
}

EAPI void
elm_widget_highlight_ignore_set(Evas_Object *obj,
                                Eina_Bool ignore)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_highlight_ignore_set(ignore));
}

static void
_elm_widget_highlight_ignore_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool ignore = va_arg(*list, int);
   Elm_Widget_Smart_Data *sd = _pd;
   sd->highlight_ignore = !!ignore;
}

EAPI Eina_Bool
elm_widget_highlight_ignore_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_wdg_highlight_ignore_get(&ret));
   return ret;
}

static void
_elm_widget_highlight_ignore_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Widget_Smart_Data *sd = _pd;
   *ret = sd->highlight_ignore;
}

EAPI void
elm_widget_highlight_in_theme_set(Evas_Object *obj,
                                  Eina_Bool highlight)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_highlight_in_theme_set(highlight));
}

static void
_elm_widget_highlight_in_theme_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool highlight = va_arg(*list, int);
   Elm_Widget_Smart_Data *sd = _pd;
   sd->highlight_in_theme = !!highlight;
   /* FIXME: if focused, it should switch from one mode to the other */
}

EAPI Eina_Bool
elm_widget_highlight_in_theme_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_wdg_highlight_in_theme_get(&ret));
   return ret;
}

static void
_elm_widget_highlight_in_theme_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Widget_Smart_Data *sd = _pd;
   *ret = sd->highlight_in_theme;
}

EAPI void
elm_widget_access_highlight_in_theme_set(Evas_Object *obj,
                                         Eina_Bool highlight)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_access_highlight_in_theme_set(highlight));
}

static void
_elm_widget_access_highlight_in_theme_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool highlight = va_arg(*list, int);
   Elm_Widget_Smart_Data *sd = _pd;
   sd->access_highlight_in_theme = !!highlight;
}

EAPI Eina_Bool
elm_widget_access_highlight_in_theme_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_wdg_access_highlight_in_theme_get(&ret));
   return ret;
}

static void
_elm_widget_access_highlight_in_theme_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Widget_Smart_Data *sd = _pd;
   *ret = sd->access_highlight_in_theme;
}

EAPI Eina_Bool
elm_widget_focus_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_wdg_focus_get(&ret));
   return ret;
}

static void
_elm_widget_focus_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Widget_Smart_Data *sd = _pd;
   *ret = sd->focused;
}

EAPI Eina_Bool
elm_widget_highlight_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_wdg_highlight_get(&ret));
   return ret;
}

static void
_elm_widget_highlight_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Widget_Smart_Data *sd = _pd;
   *ret = sd->highlighted;
}

EAPI Evas_Object *
elm_widget_focused_object_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do((Eo *) obj, elm_wdg_focused_object_get(&ret));
   return ret;
}

static void
_elm_widget_focused_object_get(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   *ret = NULL;
   Elm_Widget_Smart_Data *sd = _pd;

   const Evas_Object *subobj;
   const Eina_List *l;

   if (!sd->focused) return;
   EINA_LIST_FOREACH(sd->subobjs, l, subobj)
     {
        Evas_Object *fobj;
        fobj = elm_widget_focused_object_get(subobj);
        if (fobj)
          {
             *ret = fobj;
             return;
          }
     }
   *ret = (Evas_Object *)obj;
}

EAPI Evas_Object *
elm_widget_top_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do((Eo *) obj, elm_wdg_top_get(&ret));
   return ret;
}

static void
_elm_widget_top_get(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   Elm_Widget_Smart_Data *sd = _pd;
   Evas_Object *int_ret = NULL;

   if (sd->parent_obj)
     {
        eo_do((Eo *) sd->parent_obj, elm_wdg_top_get(&int_ret));
        *ret = int_ret;
        return;
     }
   *ret = (Evas_Object *)obj;
}

EAPI Eina_Bool
elm_widget_is(const Evas_Object *obj)
{
   return _elm_widget_is(obj);
}

EAPI Evas_Object *
elm_widget_parent_widget_get(const Evas_Object *obj)
{
   Evas_Object *parent;

   if (_elm_widget_is(obj))
     {
        ELM_WIDGET_DATA_GET(obj, sd);
        if (!sd) return NULL;
        parent = sd->parent_obj;
     }
   else
     {
        parent = evas_object_data_get(obj, "elm-parent");
        if (!parent) parent = evas_object_smart_parent_get(obj);
     }

   while (parent)
     {
        Evas_Object *elm_parent;
        if (_elm_widget_is(parent)) break;
        elm_parent = evas_object_data_get(parent, "elm-parent");
        if (elm_parent) parent = elm_parent;
        else parent = evas_object_smart_parent_get(parent);
     }
   return parent;
}

EAPI Evas_Object *
elm_widget_parent2_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do((Eo *) obj, elm_wdg_parent2_get(&ret));
   return ret;
}

static void
_elm_widget_parent2_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   *ret = NULL;
   Elm_Widget_Smart_Data *sd = _pd;
   *ret = sd->parent2;
}

EAPI void
elm_widget_parent2_set(Evas_Object *obj, Evas_Object *parent)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_parent2_set(parent));
}

static void
_elm_widget_parent2_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object *parent = va_arg(*list, Evas_Object *);
   Elm_Widget_Smart_Data *sd = _pd;
   sd->parent2 = parent;
}

EAPI void
elm_widget_event_callback_add(Evas_Object *obj,
                              Elm_Event_Cb func,
                              const void *data)
{
   ELM_WIDGET_CHECK(obj);
   EINA_SAFETY_ON_NULL_RETURN(func);
   eo_do(obj, elm_wdg_event_callback_add(func, data));
}

static void
_elm_widget_event_callback_add(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Event_Cb func = va_arg(*list, Elm_Event_Cb);
   EINA_SAFETY_ON_NULL_RETURN(func);
   const void *data = va_arg(*list, const void *);
   Elm_Widget_Smart_Data *sd = _pd;

   Elm_Event_Cb_Data *ecb = ELM_NEW(Elm_Event_Cb_Data);
   ecb->func = func;
   ecb->data = data;
   sd->event_cb = eina_list_append(sd->event_cb, ecb);
}

EAPI void *
elm_widget_event_callback_del(Evas_Object *obj,
                              Elm_Event_Cb func,
                              const void *data)
{
   ELM_WIDGET_CHECK(obj) NULL;
   EINA_SAFETY_ON_NULL_RETURN_VAL(func, NULL);
   void *ret = NULL;
   eo_do(obj, elm_wdg_event_callback_del(func, data, &ret));
   return ret;
}

static void
_elm_widget_event_callback_del(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Event_Cb func = va_arg(*list, Elm_Event_Cb);
   const void *data = va_arg(*list, const void *);
   void **ret = va_arg(*list, void **);
   if (ret) *ret = NULL;

   EINA_SAFETY_ON_NULL_RETURN(func);
   Eina_List *l;
   Elm_Event_Cb_Data *ecd;
   Elm_Widget_Smart_Data *sd = _pd;
   EINA_LIST_FOREACH(sd->event_cb, l, ecd)
     if ((ecd->func == func) && (ecd->data == data))
       {
          free(ecd);
          sd->event_cb = eina_list_remove_list(sd->event_cb, l);
          if (ret) *ret = (void *)data;
          return;
       }
}

EAPI Eina_Bool
elm_widget_event_propagate(Evas_Object *obj,
                           Evas_Callback_Type type,
                           void *event_info,
                           Evas_Event_Flags *event_flags)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_wdg_event_propagate(type, event_info, event_flags, &ret));
   return ret;
}

static void
_elm_widget_event_propagate(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Callback_Type type = va_arg(*list, Evas_Callback_Type);
   void *event_info = va_arg(*list, void *);
   Evas_Event_Flags *event_flags = va_arg(*list, Evas_Event_Flags *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Evas_Object *parent = obj;
   Elm_Event_Cb_Data *ecd;
   Eina_List *l, *l_prev;

   while (parent &&
          (!(event_flags && ((*event_flags) & EVAS_EVENT_FLAG_ON_HOLD))))
     {
        ELM_WIDGET_CHECK(parent);
        Elm_Widget_Smart_Data *sd = eo_data_scope_get(parent, MY_CLASS);

        Eina_Bool int_ret = EINA_FALSE;
        eo_do(parent, elm_wdg_event(obj, type, event_info, &int_ret));
        if (int_ret)
          {
             if (ret) *ret = EINA_TRUE;
             return ;
          }

        EINA_LIST_FOREACH_SAFE(sd->event_cb, l, l_prev, ecd)
          {
             if (ecd->func((void *)ecd->data, parent, obj, type, event_info) ||
                 (event_flags && ((*event_flags) & EVAS_EVENT_FLAG_ON_HOLD)))
               {
                  if (ret) *ret = EINA_TRUE;
                  return ;
               }
          }
        parent = sd->parent_obj;
     }
}

/**
 * @internal
 *
 * Set custom focus chain.
 *
 * This function i set one new and overwrite any previous custom focus chain
 * with the list of objects. The previous list will be deleted and this list
 * will be managed. After setted, don't modity it.
 *
 * @note On focus cycle, only will be evaluated children of this container.
 *
 * @param obj The container widget
 * @param objs Chain of objects to pass focus
 * @ingroup Widget
 */
EAPI void
elm_widget_focus_custom_chain_set(Evas_Object *obj,
                                  Eina_List *objs)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_focus_custom_chain_set(objs));
}

static void
_elm_widget_focus_custom_chain_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_List *objs = va_arg(*list, Eina_List *);
   if (!_elm_widget_focus_chain_manager_is(obj)) return;

   elm_widget_focus_custom_chain_unset(obj);

   Eina_List *l;
   Evas_Object *o;
   Elm_Widget_Smart_Data *sd = _pd;

   EINA_LIST_FOREACH(objs, l, o)
     {
        evas_object_event_callback_add(o, EVAS_CALLBACK_DEL,
                                       _elm_object_focus_chain_del_cb, obj);
     }

   sd->focus_chain = objs;
}

/**
 * @internal
 *
 * Get custom focus chain
 *
 * @param obj The container widget
 * @ingroup Widget
 */
EAPI const Eina_List *
elm_widget_focus_custom_chain_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) NULL;
   const Eina_List *ret = NULL;
   eo_do((Eo *) obj, elm_wdg_focus_custom_chain_get(&ret));
   return ret;
}

static void
_elm_widget_focus_custom_chain_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const Eina_List **ret = va_arg(*list, const Eina_List **);
   Elm_Widget_Smart_Data *sd = _pd;
   *ret = (const Eina_List *)sd->focus_chain;
}

/**
 * @internal
 *
 * Unset custom focus chain
 *
 * @param obj The container widget
 * @ingroup Widget
 */
EAPI void
elm_widget_focus_custom_chain_unset(Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_focus_custom_chain_unset());
}

static void
_elm_widget_focus_custom_chain_unset(Eo *obj EINA_UNUSED, void *_pd, va_list *list EINA_UNUSED)
{
   Eina_List *l, *l_next;
   Evas_Object *o;
   Elm_Widget_Smart_Data *sd = _pd;

   EINA_LIST_FOREACH_SAFE(sd->focus_chain, l, l_next, o)
     {
        evas_object_event_callback_del_full(o, EVAS_CALLBACK_DEL,
                                            _elm_object_focus_chain_del_cb, obj);
        sd->focus_chain = eina_list_remove_list(sd->focus_chain, l);
     }
}

/**
 * @internal
 *
 * Append object to custom focus chain.
 *
 * @note If relative_child equal to NULL or not in custom chain, the object
 * will be added in end.
 *
 * @note On focus cycle, only will be evaluated children of this container.
 *
 * @param obj The container widget
 * @param child The child to be added in custom chain
 * @param relative_child The relative object to position the child
 * @ingroup Widget
 */
EAPI void
elm_widget_focus_custom_chain_append(Evas_Object *obj,
                                     Evas_Object *child,
                                     Evas_Object *relative_child)
{
   ELM_WIDGET_CHECK(obj);
   EINA_SAFETY_ON_NULL_RETURN(child);

   eo_do(obj, elm_wdg_focus_custom_chain_append(child, relative_child));
}

static void
_elm_widget_focus_custom_chain_append(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *child = va_arg(*list, Evas_Object *);
   Evas_Object *relative_child = va_arg(*list, Evas_Object *);

   EINA_SAFETY_ON_NULL_RETURN(child);

   Elm_Widget_Smart_Data *sd = _pd;

   if (!_elm_widget_focus_chain_manager_is(obj)) return;

   evas_object_event_callback_add(child, EVAS_CALLBACK_DEL,
                                  _elm_object_focus_chain_del_cb, obj);

   if (!relative_child)
     sd->focus_chain = eina_list_append(sd->focus_chain, child);
   else
     sd->focus_chain = eina_list_append_relative(sd->focus_chain,
                                                 child, relative_child);
}

/**
 * @internal
 *
 * Prepend object to custom focus chain.
 *
 * @note If relative_child equal to NULL or not in custom chain, the object
 * will be added in begin.
 *
 * @note On focus cycle, only will be evaluated children of this container.
 *
 * @param obj The container widget
 * @param child The child to be added in custom chain
 * @param relative_child The relative object to position the child
 * @ingroup Widget
 */
EAPI void
elm_widget_focus_custom_chain_prepend(Evas_Object *obj,
                                      Evas_Object *child,
                                      Evas_Object *relative_child)
{
   ELM_WIDGET_CHECK(obj);
   EINA_SAFETY_ON_NULL_RETURN(child);
   eo_do(obj, elm_wdg_focus_custom_chain_prepend(child, relative_child));
}

static void
_elm_widget_focus_custom_chain_prepend(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *child = va_arg(*list, Evas_Object *);
   Evas_Object *relative_child = va_arg(*list, Evas_Object *);

   EINA_SAFETY_ON_NULL_RETURN(child);
   Elm_Widget_Smart_Data *sd = _pd;

   if (!_elm_widget_focus_chain_manager_is(obj)) return;

   evas_object_event_callback_add(child, EVAS_CALLBACK_DEL,
                                  _elm_object_focus_chain_del_cb, obj);

   if (!relative_child)
     sd->focus_chain = eina_list_prepend(sd->focus_chain, child);
   else
     sd->focus_chain = eina_list_prepend_relative(sd->focus_chain,
                                                  child, relative_child);
}

/**
 * @internal
 *
 * Give focus to next object in object tree.
 *
 * Give focus to next object in focus chain of one object sub-tree.
 * If the last object of chain already have focus, the focus will go to the
 * first object of chain.
 *
 * @param obj The widget root of sub-tree
 * @param dir Direction to cycle the focus
 *
 * @ingroup Widget
 */
EAPI void
elm_widget_focus_cycle(Evas_Object *obj,
                       Elm_Focus_Direction dir)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_focus_cycle(dir));
}

static void
_elm_widget_focus_cycle(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);

   Evas_Object *target = NULL;
   if (!_elm_widget_is(obj))
     return;
   elm_widget_focus_next_get(obj, dir, &target);
   if (target)
     {
        /* access */
        if (_elm_config->access_mode)
          {
             /* highlight cycle does not steal a focus, only after window gets
                the ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_ACTIVATE message,
                target will steal focus, or focus its own job. */
             if (!_elm_access_auto_highlight_get())
               elm_widget_focus_steal(target);

             _elm_access_highlight_set(target);
             elm_widget_focus_region_show(target);
          }
        else elm_widget_focus_steal(target);
     }
}

/**
 * @internal
 *
 * Give focus to near object(in object tree) in one direction.
 *
 * Give focus to near object(in object tree) in direction of current
 * focused object.  If none focusable object in given direction or
 * none focused object in object tree, the focus will not change.
 *
 * @param obj The reference widget
 * @param degree Degree changes clockwise. i.e. 0-degree: Up,
 *               90-degree: Right, 180-degree: Down, and 270-degree: Left
 * @return EINA_TRUE if focus is moved.
 *
 * @ingroup Widget
 */
EAPI Eina_Bool
elm_widget_focus_direction_go(Evas_Object *obj,
                              double degree)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_wdg_focus_direction_go(degree, &ret));
   return ret;
}

static void
_elm_widget_focus_direction_go(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   double degree = va_arg(*list, double);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Evas_Object *target = NULL;
   Evas_Object *current_focused = NULL;
   double weight = 0.0;

   if (!_elm_widget_is(obj)) return;
   if (!elm_widget_focus_get(obj)) return;

   current_focused = elm_widget_focused_object_get(obj);

   if (elm_widget_focus_direction_get
         (obj, current_focused, degree, &target, &weight))
     {
        elm_widget_focus_steal(target);
        if (ret) *ret = EINA_TRUE;
        return;
     }
}

static double
_direction_weight_get(const Evas_Object *obj1,
                      const Evas_Object *obj2,
                      double degree)
{
   Evas_Coord obj_x1, obj_y1, w1, h1, obj_x2, obj_y2, w2, h2;
   double x1, yy1, x2, yy2, xx1, yyy1, xx2, yyy2;
   double ax, ay, cx, cy;
   double weight = -1.0, g = 0.0;

   if (obj1 == obj2) return 0.0;

   degree -= 90.0;
   while (degree >= 360.0)
     degree -= 360.0;
   while (degree < 0.0)
     degree += 360.0;

   evas_object_geometry_get(obj1, &obj_x1, &obj_y1, &w1, &h1);
   cx = obj_x1 + (w1 / 2.0);
   cy = obj_y1 + (h1 / 2.0);
   evas_object_geometry_get(obj2, &obj_x2, &obj_y2, &w2, &h2);

   /* For overlapping cases. */
   if (ELM_RECTS_INTERSECT(obj_x1, obj_y1, w1, h1, obj_x2, obj_y2, w2, h2))
     return 0.0;

   /* Change all points to relative one. */
   x1 = obj_x1 - cx;
   xx1 = x1 + w1;
   yy1 = obj_y1 - cy;
   yyy1 = yy1 + h1;
   x2 = obj_x2 - cx;
   xx2 = x2 + w2;
   yy2 = obj_y2 - cy;
   yyy2 = yy2 + h2;

   /* Get crossing points (ax, ay) between obj1 and a line extending
    * to the direction of current degree. */
   if (degree == 0.0)
     {
        ax = xx1;
        ay = 0.0;
     }
   else if (degree == 90.0)
     {
        ax = 0.0;
        ay = yyy1;
     }
   else if (degree == 180.0)
     {
        ax = x1;
        ay = 0.0;
     }
   else if (degree == 270.0)
     {
        ax = 0.0;
        ay = yy1;
     }
   else
     {
        g = tan(degree * (M_PI / 180.0));
        if ((degree > 0.0) && (degree < 90.0))
          {
             ay = g * xx1;
             if (ay <= yyy1) ax = xx1;
             else
               {
                  ax = yyy1 / g;
                  ay = yyy1;
               }
          }
        else if ((degree > 90.0) && (degree < 180.0))
          {
             ay = g * x1;
             if (ay <= yyy1) ax = x1;
             else
               {
                  ax = yyy1 / g;
                  ay = yyy1;
               }
          }
        else if ((degree > 180.0) && (degree < 270.0))
          {
             ay = g * x1;
             if (ay >= yy1) ax = x1;
             else
               {
                  ax = yy1 / g;
                  ay = yy1;
               }
          }
        else
          {
             ay = g * xx1;
             if (ay >= yy1) ax = xx1;
             else
               {
                  ax = yy1 / g;
                  ay = yy1;
               }
          }
     }

   /* Filter obj2, if it is not in the specific derection. */
   int i = 0;
   double rx[4] = {0.0, 0.0, 0.0, 0.0}, ry[4] = {0.0, 0.0, 0.0, 0.0};
   double t1, t2, u1, v1, u2, v2;

   if ((degree == 45.0) || (degree == 225.0) || (degree == 135.0) ||
       (degree == 315.0))
     {
        u1 = 1.0;
        v1 = 0.0;
        u2 = 0.0;
        v2 = 1.0;
     }
   else
     {
        double g2 = tan((degree + 45.0) * (M_PI / 180.0));
        u1 = (-1.0 * g2);
        u2 = (1.0 / g2);
        v1 = v2 = 1.0;
     }
   t1 = (u1 * ax) + (v1 * ay);
   t2 = (u2 * ax) + (v2 * ay);

#define _R(x) (int)((x + 0.05) * 10.0)

   if ((_R(t1 * ((u1 * x2) + (v1 * yy2))) > 0) && (_R(t2 * ((u2 * x2) +
                                                            (v2 * yy2))) > 0))
     {
        rx[i] = x2;
        ry[i++] = yy2;
     }
   if ((_R(t1 * ((u1 * x2) + (v1 * yyy2))) > 0) && (_R(t2 * ((u2 * x2) +
                                                             (v2 * yyy2))) > 0))
     {
        rx[i] = x2;
        ry[i++] = yyy2;
     }
   if ((_R(t1 * ((u1 * xx2) + (v1 * yy2))) > 0) && (_R(t2 * ((u2 * xx2) +
                                                             (v2 * yy2))) > 0))
     {
        rx[i] = xx2;
        ry[i++] = yy2;
     }
   if ((_R(t1 * ((u1 * xx2) + (v1 * yyy2))) > 0) &&
       (_R(t2 * ((u2 * xx2) + (v2 * yyy2))) > 0))
     {
        rx[i] = xx2;
        ry[i++] = yyy2;
     }
   if (i == 0)
     {
        if (degree == 0.0)
          {
             if ((_R(xx2) < 0) || (_R(yy2) > 0) || (_R(yyy2) < 0)) return 0.0;
          }
        else if (degree == 90.0)
          {
             if ((_R(yyy2) < 0) || (_R(x2) > 0) || (_R(xx2) < 0)) return 0.0;
          }
        else if (degree == 180.0)
          {
             if ((_R(x2) > 0) || (_R(yy2) > 0) || (_R(yyy2) < 0)) return 0.0;
          }
        else if (degree == 270.0)
          {
             if ((_R(yy2) > 0) || (_R(x2) > 0) || (_R(xx2) < 0)) return 0.0;
          }
        else
          {
             if ((_R(g * x2) >= _R(yy2)) && (_R((g * x2)) <= _R(yyy2)))
               {
                  if (!((_R(ax * x2) > 0) && (_R(ay * (g * x2)) > 0)))
                    return 0.0;
               }
             else if ((_R(g * xx2) >= _R(yy2)) && (_R((g * xx2)) <= _R(yyy2)))
               {
                  if (!((_R(ax * xx2) > 0) && (_R(ay * (g * xx2)) > 0)))
                    return 0.0;
               }
             else if ((_R((1.0 / g) * yy2) >= _R(xx2)) && (_R((1.0 / g) * yy2)
                                                           <= _R(xx2)))
               {
                  if (!((_R(ax * ((1.0 / g) * yy2)) > 0)
                        && (_R(ay * yy2) > 0)))
                    return 0.0;
               }
             else if ((_R((1.0 / g) * yyy2) >= _R(xx2)) &&
                      (_R((1.0 / g) * yyy2) <= _R(xx2)))
               {
                  if (!((_R(ax * ((1.0 / g) * yyy2)) > 0)
                        && (_R(ay * yyy2) > 0))) return 0.0;
               }
             else return 0.0;
          }
     }

   /* Calculate the weight for obj2. */
   if (degree == 0.0)
     {
        if (_R(xx1) > _R(x2)) weight = -1.0;
        else if ((_R(yy2) >= _R(yy1)) && (_R(yyy2) <= _R(yyy1)))
          weight = (x2 - xx1) * (x2 - xx1);
        else if (_R(yy2) > 0)
          weight = ((x2 - xx1) * (x2 - xx1)) + (yy2 * yy2);
        else if (_R(yyy2) < 0)
          weight = ((x2 - xx1) * (x2 - xx1)) + (yyy2 * yyy2);
        else weight = (x2 - xx1) * (x2 - xx1);
     }
   else if (degree == 90.0)
     {
        if (_R(yyy1) > _R(yy2)) weight = -1.0;
        else if ((_R(x2) >= _R(x1)) && (_R(xx2) <= _R(xx1)))
          weight = (yy2 - yyy1) * (yy2 - yyy1);
        else if (_R(x2) > 0)
          weight = (x2 * x2) + ((yy2 - yyy1) * (yy2 - yyy1));
        else if (_R(xx2) < 0)
          weight = (xx2 * xx2) + ((yy2 - yyy1) * (yy2 - yyy1));
        else weight = (yy2 - yyy1) * (yy2 - yyy1);
     }
   else if (degree == 180.0)
     {
        if (_R(x1) < _R(xx2)) weight = -1.0;
        else if ((_R(yy2) >= _R(yy1)) && (_R(yyy2) <= _R(yyy1)))
          weight = (x1 - xx2) * (x1 - xx2);
        else if (_R(yy2) > 0)
          weight = ((x1 - xx2) * (x1 - xx2)) + (yy2 * yy2);
        else if (_R(yyy2) < 0)
          weight = ((x1 - xx2) * (x1 - xx2)) + (yyy2 * yyy2);
        else weight = (x1 - xx2) * (x1 - xx2);
     }
   else if (degree == 270.0)
     {
        if (_R(yy1) < _R(yyy2)) weight = -1.0;
        else if ((_R(x2) >= _R(x1)) && (_R(xx2) <= _R(xx1)))
          weight = (yy1 - yyy2) * (yy1 - yyy2);
        else if (_R(x2) > 0)
          weight = (x2 * x2) + ((yy1 - yyy2) * (yy1 - yyy2));
        else if (_R(xx2) < 0)
          weight = (xx2 * xx2) + ((yy1 - yyy2) * (yy1 - yyy2));
        else weight = (yy1 - yyy2) * (yy1 - yyy2);
     }
   else
     {
        int j = 0, k = 0;
        double sx[4] = {0.0, 0.0, 0.0, 0.0}, sy[4] = {0.0, 0.0, 0.0, 0.0};
        double t_weight[4] = {-1.0, -1.0, -1.0, -1.0};
        if ((_R(g * x2) >= _R(yy2)) && (_R(g * x2) <= _R(yyy2)))
          {
             sx[j] = x2;
             sy[j] = g * x2;
             t_weight[j++] = ((ax - x2) * (ax - x2)) +
               ((ay - (g * x2)) * (ay - (g * x2)));
          }
        if ((_R(g * xx2) >= _R(yy2)) && (_R(g * xx2) <= _R(yyy2)))
          {
             sx[j] = xx2;
             sy[j] = g * xx2;
             t_weight[j++] = ((ax - xx2) * (ax - xx2)) +
               ((ay - (g * xx2)) * (ay - (g * xx2)));
          }
        if ((_R((1.0 / g) * yy2) >= _R(x2)) && (_R((1.0 / g) * yy2) <= _R(xx2)))
          {
             sx[j] = (1.0 / g) * yy2;
             sy[j] = yy2;
             t_weight[j++] =
               ((ax - ((1.0 / g) * yy2)) * (ax - ((1.0 / g) * yy2))) +
               ((ay - yy2) * (ay - yy2));
          }
        if ((_R((1.0 / g) * yyy2) >= _R(x2)) && (_R((1.0 / g) * yyy2)
                                                 <= _R(xx2)))
          {
             sx[j] = (1.0 / g) * yyy2;
             sy[j] = yyy2;
             t_weight[j++] =
               ((ax - ((1.0 / g) * yyy2)) * (ax - ((1.0 / g) * yyy2))) +
               ((ay - yyy2) * (ay - yyy2));
          }

        if ((j > 2) || ((j == 2) && ((_R(sx[0]) != _R(sx[1])) ||
                                     (_R(sy[0]) != _R(sy[1])))))
          {
             for (; k < j; k++)
               {
                  if (_R(t_weight[k]) == 0) return -1.0;
                  if ((1 / weight) < (1 / t_weight[k])) weight = t_weight[k];
               }
          }
        else
          {
             for (; k < i; k++)
               {
                  double ccx, ccy, t1_weight, x_diff, y_diff;
                  ccx = ((1.0 / g) * rx[k] + ry[k]) / (g + (1.0 / g));
                  ccy = g * ccx;
                  x_diff = rx[k] - ccx;
                  if (x_diff < 0) x_diff *= -1.0;
                  y_diff = ry[k] - ccy;
                  if (y_diff < 0) y_diff *= -1.0;
                  t1_weight =
                    (((ax - ccx) * (ax - ccx)) + ((ay - ccy) * (ay - ccy))) +
                    ((x_diff * x_diff * x_diff) + (y_diff * y_diff * y_diff));
                  if ((_R(t1_weight) != 0) && ((1 / weight) < (1 / t1_weight)))
                    weight = t1_weight;
               }
          }
     }
   /* Return the current object's weight. */
   if (weight == -1.0) return 0.0;
   if (_R(weight) == 0) return -1.0;

#undef _R

   return 1.0 / weight;
}

/**
 * @internal
 *
 * Get near object in one direction of base object.
 *
 * Get near object(in the object sub-tree) in one direction of
 * base object. Return the near object by reference.
 * By initializing weight, you can filter objects locating far
 * from base object. If object is in the specific direction,
 * weight is (1/(distance^2)). If object is not exactly in one
 * direction, some penalty will be added.
 *
 * @param obj The widget root of sub-tree
 * @param base The base object of the direction
 * @param degree Degree changes clockwise. i.e. 0-degree: Up,
 *               90-degree: Right, 180-degree: Down, and 270-degree: Left
 * @param direction The near object in one direction
 * @param weight The weight is bigger when the object is located near
 * @return EINA_TRUE if near object is updated.
 *
 * @ingroup Widget
 */

EAPI Eina_Bool
elm_widget_focus_direction_get(const Evas_Object *obj,
                               const Evas_Object *base,
                               double degree,
                               Evas_Object **direction,
                               double *weight)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_wdg_focus_direction_get(base, degree, direction, weight, &ret));
   return ret;
}

static void
_elm_widget_focus_direction_get(Eo *obj, void *_pd, va_list *list)
{
   const Evas_Object *base = va_arg(*list, const Evas_Object *);
   double degree = va_arg(*list, double);
   Evas_Object **direction = va_arg(*list, Evas_Object **);
   double *weight = va_arg(*list, double *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;

   Elm_Widget_Smart_Data *sd = _pd;
   double c_weight;

   /* -1 means the best was already decided. Don't need any more searching. */
   if (!direction || !weight || !base || (obj == base))
     return;

   /* Ignore if disabled */
   if ((!evas_object_visible_get(obj))
       || (elm_widget_disabled_get(obj))
       || (elm_widget_tree_unfocusable_get(obj)))
     return;

   /* Try use hook */
   if (_elm_widget_focus_direction_manager_is(obj))
     {
        Eina_Bool int_ret = EINA_FALSE;
        eo_do((Eo *)obj, elm_wdg_focus_direction(base, degree, direction, weight, &int_ret));
        *ret = int_ret;
        return;
     }

   if (!elm_widget_can_focus_get(obj) || elm_widget_focus_get(obj))
     return;

   c_weight = _direction_weight_get(base, obj, degree);
   if ((c_weight == -1.0) ||
       ((c_weight != 0.0) && (*weight != -1.0) &&
        ((int)(*weight * 1000000) <= (int)(c_weight * 1000000))))
     {
        if (*direction &&
            ((int)(*weight * 1000000) == (int)(c_weight * 1000000)))
          {
             ELM_WIDGET_DATA_GET(*direction, sd1);
             if (sd1)
               {
                  if (sd->focus_order <= sd1->focus_order)
                    return;
               }
          }
        *direction = (Evas_Object *)obj;
        *weight = c_weight;
        *ret = EINA_TRUE;
     }
}

/**
 * @internal
 *
 * Get near object in one direction of base object in list.
 *
 * Get near object in one direction of base object in the specific
 * object list. Return the near object by reference.
 * By initializing weight, you can filter objects locating far
 * from base object. If object is in the specific direction,
 * weight is (1/(distance^2)). If object is not exactly in one
 * direction, some penalty will be added.
 *
 * @param obj The widget root of sub-tree
 * @param base The base object of the direction
 * @param items list with ordered objects
 * @param list_data_get function to get the object from one item of list
 * @param degree Degree changes clockwise. i.e. 0-degree: Up,
 *               90-degree: Right, 180-degree: Down, and 270-degree: Left
 * @param direction The near object in one direction
 * @param weight The weight is bigger when the object is located near
 * @return EINA_TRUE if near object is updated.
 *
 * @ingroup Widget
 */
EAPI Eina_Bool
elm_widget_focus_list_direction_get(const Evas_Object *obj,
                                    const Evas_Object *base,
                                    const Eina_List *items,
                                    void *(*list_data_get)(const Eina_List *l),
                                    double degree,
                                    Evas_Object **direction,
                                    double *weight)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_wdg_focus_list_direction_get(base, items, list_data_get, degree, direction, weight, &ret));
   return ret;
}

static void
_elm_widget_focus_list_direction_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   const Evas_Object *base = va_arg(*list, const Evas_Object *);
   const Eina_List *items = va_arg(*list, const Eina_List *);
   list_data_get_func_type list_data_get = va_arg(*list, list_data_get_func_type);
   double degree = va_arg(*list, double);
   Evas_Object **direction = va_arg(*list, Evas_Object **);
   double *weight  = va_arg(*list, double *);

   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;

   if (!direction || !weight || !base || !items)
     return;

   const Eina_List *l = items;
   Evas_Object *current_best = *direction;

   for (; l; l = eina_list_next(l))
     {
        Evas_Object *cur = list_data_get(l);
        if (cur)
          elm_widget_focus_direction_get(cur, base, degree, direction, weight);
     }
   if (current_best != *direction)
     *ret = EINA_TRUE;
}

/**
 * @internal
 *
 * Get next object in focus chain of object tree.
 *
 * Get next object in focus chain of one object sub-tree.
 * Return the next object by reference. If don't have any candidate to receive
 * focus before chain end, the first candidate will be returned.
 *
 * @param obj The widget root of sub-tree
 * @param dir Direction of focus chain
 * @param next The next object in focus chain
 * @return EINA_TRUE if don't need focus chain restart/loop back
 *         to use 'next' obj.
 *
 * @ingroup Widget
 */
EAPI Eina_Bool
elm_widget_focus_next_get(const Evas_Object *obj,
                          Elm_Focus_Direction dir,
                          Evas_Object **next)
{
   if (!next)
     return EINA_FALSE;
   *next = NULL;

   ELM_WIDGET_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_wdg_focus_next_get(dir, next, &ret));
   return ret;
}

static void
_elm_widget_focus_next_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Access_Info *ac;
   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Evas_Object **next = va_arg(*list, Evas_Object **);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Widget_Smart_Data *sd = _pd;
   *ret = EINA_FALSE;

   if (!next)
     return;
   *next = NULL;

   /* Ignore if disabled */
   if (_elm_config->access_mode && _elm_access_auto_highlight_get())
     {
        if (!evas_object_visible_get(obj)
            || (elm_widget_tree_unfocusable_get(obj)))
          return;
     }
   else
     {
        if ((!evas_object_visible_get(obj))
            || (elm_widget_disabled_get(obj))
            || (elm_widget_tree_unfocusable_get(obj)))
          return;
     }

   /* Try use hook */
   if (_elm_widget_focus_chain_manager_is(obj))
     {
        Eina_Bool int_ret = EINA_FALSE;
        eo_do((Eo *)obj, elm_wdg_focus_next(dir, next, &int_ret));
        if (!int_ret && elm_widget_focus_get(obj))
          {
             Evas_Object *o = NULL;
             if (dir == ELM_FOCUS_PREVIOUS)
               o = sd->focus_previous;
             else if (dir == ELM_FOCUS_NEXT)
               o = sd->focus_next;
             else if (dir == ELM_FOCUS_UP)
               o = sd->focus_up;
             else if (dir == ELM_FOCUS_DOWN)
               o = sd->focus_down;
             else if (dir == ELM_FOCUS_RIGHT)
               o = sd->focus_right;
             else if (dir == ELM_FOCUS_LEFT)
               o = sd->focus_left;

             if (o)
               {
                  *next = o;
                  *ret = EINA_TRUE;
                  return;
               }
          }
        *ret = int_ret;
        return;
     }

   /* access object does not check sd->can_focus, because an object could
      have highlight even though the object is not focusable. */
   if (_elm_config->access_mode && _elm_access_auto_highlight_get())
     {
        ac = _elm_access_info_get(obj);
        if (!ac) return;

        /* check whether the hover object is visible or not */
        if (!evas_object_visible_get(ac->hoverobj))
          return;
     }
   else if (!elm_widget_can_focus_get(obj))
     return;

   if (elm_widget_focus_get(obj))
     {
        if (dir == ELM_FOCUS_PREVIOUS)
          *next = sd->focus_previous;
        else if (dir == ELM_FOCUS_NEXT)
          *next = sd->focus_next;
        else if (dir == ELM_FOCUS_UP)
          *next = sd->focus_up;
        else if (dir == ELM_FOCUS_DOWN)
          *next = sd->focus_down;
        else if (dir == ELM_FOCUS_RIGHT)
          *next = sd->focus_right;
        else if (dir == ELM_FOCUS_LEFT)
          *next = sd->focus_left;

        if (*next)
          {
             *ret = EINA_TRUE;
             return;
          }
     }

   /* Return */
   *next = (Evas_Object *)obj;
   *ret = !ELM_WIDGET_FOCUS_GET(obj);
}

/**
 * @internal
 *
 * Get next object in focus chain of object tree in list.
 *
 * Get next object in focus chain of one object sub-tree ordered by one list.
 * Return the next object by reference. If don't have any candidate to receive
 * focus before list end, the first candidate will be returned.
 *
 * @param obj The widget root of sub-tree
 * @param items list with ordered objects
 * @param list_data_get function to get the object from one item of list
 * @param dir Direction of focus chain
 * @param next The next object in focus chain
 * @return EINA_TRUE if don't need focus chain restart/loop back
 *         to use 'next' obj.
 *
 * @ingroup Widget
 */
EAPI Eina_Bool
elm_widget_focus_list_next_get(const Evas_Object *obj,
                               const Eina_List *items,
                               void *(*list_data_get)(const Eina_List *list),
                               Elm_Focus_Direction dir,
                               Evas_Object **next)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_wdg_focus_list_next_get(items, list_data_get, dir, next, &ret));
   return ret;
}

static void
_elm_widget_focus_list_next_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const Eina_List *items = va_arg(*list, const Eina_List *);
   list_data_get_func_type list_data_get = va_arg(*list, list_data_get_func_type);
   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Evas_Object **next = va_arg(*list, Evas_Object **);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Eina_List *(*list_next)(const Eina_List *list) = NULL;
   Evas_Object *focused_object = NULL;

   if (!next)
     return;
   *next = NULL;

   if (!_elm_widget_is(obj))
     return;

   if (!items)
     return;

   /* When Up, Down, Right, or Left, try direction_get first. */
   focused_object = elm_widget_focused_object_get(obj);
   if (focused_object)
     {
        if((dir == ELM_FOCUS_UP)
           || (dir == ELM_FOCUS_DOWN)
           || (dir == ELM_FOCUS_RIGHT)
           || (dir == ELM_FOCUS_LEFT))
          {
             *next = elm_widget_focus_next_object_get(focused_object, dir);
             if (*next)
               {
                  if (ret) *ret = EINA_TRUE;
                  return;
               }
             else
               {
                  Evas_Object *n;
                  double degree;
                  double weight;

                  if (dir == ELM_FOCUS_UP) degree = 0.0;
                  else if (dir == ELM_FOCUS_DOWN) degree = 180.0;
                  else if (dir == ELM_FOCUS_RIGHT) degree = 90.0;
                  else if (dir == ELM_FOCUS_LEFT) degree = 270.0;

                  if (elm_widget_focus_list_direction_get(obj, focused_object,
                                                          items, list_data_get,
                                                          degree, &n, &weight))
                    {
                       *next = n;
                       if (ret) *ret = EINA_TRUE;
                       return;
                    }
               }
          }
     }

   /* Direction */
   if (dir == ELM_FOCUS_PREVIOUS)
     {
        items = eina_list_last(items);
        list_next = eina_list_prev;
     }
   else if ((dir == ELM_FOCUS_NEXT)
            || (dir == ELM_FOCUS_UP)
            || (dir == ELM_FOCUS_DOWN)
            || (dir == ELM_FOCUS_RIGHT)
            || (dir == ELM_FOCUS_LEFT))
     list_next = eina_list_next;
   else
     return;

   const Eina_List *l = items;

   /* Recovery last focused sub item */
   if (ELM_WIDGET_FOCUS_GET(obj))
     {
        for (; l; l = list_next(l))
          {
             Evas_Object *cur = list_data_get(l);
             if (ELM_WIDGET_FOCUS_GET(cur)) break;
          }

         /* Focused object, but no focused sub item */
         if (!l) l = items;
     }

   const Eina_List *start = l;
   Evas_Object *to_focus = NULL;

   /* Iterate sub items */
   /* Go to the end of list */
   for (; l; l = list_next(l))
     {
        Evas_Object *tmp = NULL;
        Evas_Object *cur = list_data_get(l);

        if (!cur) continue;
        if (elm_widget_parent_get(cur) != obj)
          continue;

        /* Try Focus cycle in subitem */
        if (elm_widget_focus_next_get(cur, dir, &tmp))
          {
             *next = tmp;
             if (ret) *ret = EINA_TRUE;
             return;
          }
        else if ((dir == ELM_FOCUS_UP)
                 || (dir == ELM_FOCUS_DOWN)
                 || (dir == ELM_FOCUS_RIGHT)
                 || (dir == ELM_FOCUS_LEFT))
          {
             if (tmp && elm_widget_focus_get(cur))
               {
                  *next = tmp;
                  return;
               }
          }
        else if ((tmp) && (!to_focus))
          to_focus = tmp;
     }

   l = items;

   /* Get First possible */
   for (; l != start; l = list_next(l))
     {
        Evas_Object *tmp = NULL;
        Evas_Object *cur = list_data_get(l);

        if (elm_widget_parent_get(cur) != obj)
          continue;

        /* Try Focus cycle in subitem */
        elm_widget_focus_next_get(cur, dir, &tmp);
        if (tmp)
          {
             *next = tmp;
             return;
          }
     }

   *next = to_focus;
   return;
}

/**
 * @internal
 *
 * Get next object which was set with specific focus direction.
 *
 * Get next object which was set by elm_widget_focus_next_object_set
 * with specific focus directioin.
 *
 * @param obj The widget
 * @param dir Direction of focus
 * @return Widget which was registered with sepecific focus direction.
 *
 * @ingroup Widget
 */
EAPI Evas_Object *
elm_widget_focus_next_object_get(const Evas_Object *obj, Elm_Focus_Direction dir)
{
   ELM_WIDGET_CHECK(obj) NULL;

   Evas_Object *ret = NULL;
   eo_do((Eo *) obj, elm_wdg_focus_next_object_get(dir, &ret));
   return ret;
}

static void
_elm_widget_focus_next_object_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   Elm_Widget_Smart_Data *sd = _pd;

   if (dir == ELM_FOCUS_PREVIOUS)
     *ret = sd->focus_previous;
   else if (dir == ELM_FOCUS_NEXT)
     *ret = sd->focus_next;
   else if (dir == ELM_FOCUS_UP)
     *ret = sd->focus_up;
   else if (dir == ELM_FOCUS_DOWN)
     *ret = sd->focus_down;
   else if (dir == ELM_FOCUS_RIGHT)
     *ret = sd->focus_right;
   else if (dir == ELM_FOCUS_LEFT)
     *ret = sd->focus_left;
}

/**
 * @internal
 *
 * Set next object with specific focus direction.
 *
 * When a widget is set with specific focus direction, this widget will be
 * the first candidate when finding the next focus object.
 * Focus next object can be registered with six directions that are previous,
 * next, up, down, right, and left.
 *
 * @param obj The widget
 * @param next Next focus object
 * @param dir Direction of focus
 *
 * @ingroup Widget
 */
EAPI void
elm_widget_focus_next_object_set(Evas_Object *obj, Evas_Object *next, Elm_Focus_Direction dir)
{
   ELM_WIDGET_CHECK(obj);
   if (!next) return;
   eo_do((Eo *) obj, elm_wdg_focus_next_object_set(next, dir));
}

static void
_elm_widget_focus_next_object_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object *next = va_arg(*list, Evas_Object *);
   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Elm_Widget_Smart_Data *sd = _pd;

   if (dir == ELM_FOCUS_PREVIOUS)
     sd->focus_previous = next;
   else if (dir == ELM_FOCUS_NEXT)
     sd->focus_next = next;
   else if (dir == ELM_FOCUS_UP)
     sd->focus_up = next;
   else if (dir == ELM_FOCUS_DOWN)
     sd->focus_down = next;
   else if (dir == ELM_FOCUS_RIGHT)
     sd->focus_right = next;
   else if (dir == ELM_FOCUS_LEFT)
     sd->focus_left = next;
}

EAPI void
elm_widget_parent_highlight_set(Evas_Object *obj,
                                Eina_Bool highlighted)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_parent_highlight_set(highlighted));
}

static void
_elm_widget_parent_highlight_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool highlighted = va_arg(*list, int);
   Elm_Widget_Smart_Data *sd = _pd;
   highlighted = !!highlighted;

   Evas_Object *o = elm_widget_parent_get(obj);

   if (o) elm_widget_parent_highlight_set(o, highlighted);

   sd->highlighted = highlighted;
}

EAPI void
elm_widget_signal_emit(Evas_Object *obj,
                       const char *emission,
                       const char *source)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_signal_emit(emission, source));
}

static void
_elm_widget_signal_emit(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *emission = va_arg(*list, const char *);
   const char *source = va_arg(*list, const char *);
   if (evas_object_smart_type_check(obj, "elm_layout"))
     elm_layout_signal_emit(obj, emission, source);
   else if (evas_object_smart_type_check(obj, "elm_icon"))
     {
        WRN("Deprecated function. This functionality on icon objects"
            " will be dropped on a next release.");
        _elm_icon_signal_emit(obj, emission, source);
     }
}

EAPI void
elm_widget_signal_callback_add(Evas_Object *obj,
                               const char *emission,
                               const char *source,
                               Edje_Signal_Cb func,
                               void *data)
{
   ELM_WIDGET_CHECK(obj);
   EINA_SAFETY_ON_NULL_RETURN(func);
   eo_do(obj, elm_wdg_signal_callback_add(emission, source, func, data));
}

static void
_elm_widget_signal_callback_add(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *emission = va_arg(*list, const char *);
   const char *source = va_arg(*list, const char *);
   Edje_Signal_Cb func = va_arg(*list, Edje_Signal_Cb);
   EINA_SAFETY_ON_NULL_RETURN(func);
   void *data = va_arg(*list, void *);

   if (evas_object_smart_type_check(obj, "elm_layout"))
     elm_layout_signal_callback_add(obj, emission, source, func, data);
   else if (evas_object_smart_type_check(obj, "elm_icon"))
     {
        WRN("Deprecated function. This functionality on icon objects"
            " will be dropped on a next release.");

        _elm_icon_signal_callback_add(obj, emission, source, func, data);
     }
}

EAPI void *
elm_widget_signal_callback_del(Evas_Object *obj,
                               const char *emission,
                               const char *source,
                               Edje_Signal_Cb func)
{
   ELM_WIDGET_CHECK(obj) NULL;
   void *ret = NULL;
   eo_do(obj, elm_wdg_signal_callback_del(emission, source, func, &ret));
   return ret;
}

static void
_elm_widget_signal_callback_del(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *emission = va_arg(*list, const char *);
   const char *source = va_arg(*list, const char *);
   Edje_Signal_Cb func = va_arg(*list, Edje_Signal_Cb);
   void **ret = va_arg(*list, void **);
   if (ret) *ret = NULL;

   void *data = NULL;

   if (evas_object_smart_type_check(obj, "elm_layout"))
     data = elm_layout_signal_callback_del(obj, emission, source, func);
   else if (evas_object_smart_type_check(obj, "elm_icon"))
     {
        WRN("Deprecated function. This functionality on icon objects"
            " will be dropped on a next release.");

        data = _elm_icon_signal_callback_del(obj, emission, source, func);
     }

   if (ret) *ret = data;
}

EAPI void
elm_widget_focus_set(Evas_Object *obj,
                     Eina_Bool focus)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_focus_set(focus));
}

static void
_elm_widget_focus_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool focus = va_arg(*list, int);

   Elm_Widget_Smart_Data *sd = _pd;

   if (!sd->focused)
     {
        focus_order++;
        sd->focus_order = focus_order;
        sd->focused = EINA_TRUE;
        eo_do(obj, elm_wdg_on_focus(NULL));
     }

   if (focus)
     {
        if ((_is_focusable(sd->resize_obj)) &&
            (!elm_widget_disabled_get(sd->resize_obj)))
          {
             elm_widget_focus_set(sd->resize_obj, focus);
          }
        else
          {
             const Eina_List *l;
             Evas_Object *child;

             EINA_LIST_FOREACH(sd->subobjs, l, child)
               {
                  if ((_is_focusable(child)) &&
                      (!elm_widget_disabled_get(child)))
                    {
                       elm_widget_focus_set(child, focus);
                       break;
                    }
               }
          }
     }
   else
     {
        const Eina_List *l;
        Evas_Object *child;

        EINA_LIST_REVERSE_FOREACH(sd->subobjs, l, child)
          {
             if ((_is_focusable(child)) &&
                 (!elm_widget_disabled_get(child)))
               {
                  elm_widget_focus_set(child, focus);
                  break;
               }
          }
     }
}

EAPI Evas_Object *
elm_widget_parent_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do((Eo *) obj, elm_wdg_parent_get(&ret));
   return ret;
}

static void
_elm_widget_parent_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   Elm_Widget_Smart_Data *sd = _pd;
   *ret = sd->parent_obj;
}

EAPI void
elm_widget_focused_object_clear(Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_focused_object_clear());
}

static void
_elm_widget_focused_object_clear(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Widget_Smart_Data *sd = _pd;

   if (!sd->focused) return;
   if (sd->resize_obj && elm_widget_focus_get(sd->resize_obj))
     eo_do(sd->resize_obj, elm_wdg_focused_object_clear());
   else
     {
        const Eina_List *l;
        Evas_Object *child;
        EINA_LIST_FOREACH(sd->subobjs, l, child)
          {
             if (elm_widget_focus_get(child))
               {
                  eo_do(child, elm_wdg_focused_object_clear());
                  break;
               }
          }
     }
   sd->focused = EINA_FALSE;
   eo_do(obj, elm_wdg_on_focus(NULL));
}

EAPI void
elm_widget_focus_steal(Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_focus_steal());
}

static void
_elm_widget_focus_steal(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Object *parent, *parent2, *o;
   Elm_Widget_Smart_Data *sd = _pd;

   if (sd->focused) return;
   if (sd->disabled) return;
   if (!sd->can_focus) return;
   if (sd->tree_unfocusable) return;
   parent = obj;
   for (;; )
     {
        o = elm_widget_parent_get(parent);
        if (!o) break;
        sd = eo_data_scope_get(o, MY_CLASS);
        if (sd->disabled || sd->tree_unfocusable) return;
        if (sd->focused) break;
        parent = o;
     }
   if ((!elm_widget_parent_get(parent)) &&
       (!elm_widget_parent2_get(parent)))
     eo_do(parent, elm_wdg_focused_object_clear());
   else
     {
        parent2 = elm_widget_parent_get(parent);
        if (!parent2) parent2 = elm_widget_parent2_get(parent);
        parent = parent2;
        sd = eo_data_scope_get(parent, MY_CLASS);
        if (sd)
          {
             if ((sd->resize_obj) && (elm_widget_focus_get(sd->resize_obj)))
               eo_do(sd->resize_obj, elm_wdg_focused_object_clear());
             else
               {
                  const Eina_List *l;
                  Evas_Object *child;
                  EINA_LIST_FOREACH(sd->subobjs, l, child)
                    {
                       if (elm_widget_focus_get(child))
                         {
                            eo_do(child, elm_wdg_focused_object_clear());
                            break;
                         }
                    }
               }
          }
     }
   _parent_focus(obj);
   return;
}

EAPI void
elm_widget_focus_restore(Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_focus_restore());
}

static void
_elm_widget_focus_restore(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Evas_Object *newest = NULL;
   unsigned int newest_focus_order = 0;

   newest = elm_widget_newest_focus_order_get(obj, &newest_focus_order, EINA_TRUE);
   if (newest)
     {
        elm_object_focus_set(newest, EINA_FALSE);
        elm_object_focus_set(newest, EINA_TRUE);
     }
}

void
_elm_widget_top_win_focused_set(Evas_Object *obj,
                                Eina_Bool top_win_focused)
{
   const Eina_List *l;
   Evas_Object *child;
   API_ENTRY return;

   if (sd->top_win_focused == top_win_focused) return;
   EINA_LIST_FOREACH(sd->subobjs, l, child)
     _elm_widget_top_win_focused_set(child, top_win_focused);
   sd->top_win_focused = top_win_focused;
}

Eina_Bool
_elm_widget_top_win_focused_get(const Evas_Object *obj)
{
   API_ENTRY return EINA_FALSE;
   return sd->top_win_focused;
}

EAPI void
elm_widget_disabled_set(Evas_Object *obj,
                        Eina_Bool disabled)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_disabled_set(disabled));
}

static void
_elm_widget_disabled_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool disabled = va_arg(*list, int);
   Elm_Widget_Smart_Data *sd = _pd;
   if (sd->disabled == disabled) return;
   sd->disabled = !!disabled;
   elm_widget_focus_disabled_handle(obj);
   eo_do(obj, elm_wdg_disable(NULL));
}

EAPI Eina_Bool
elm_widget_disabled_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_wdg_disabled_get(&ret));
   return ret;
}

static void
_elm_widget_disabled_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Widget_Smart_Data *sd = _pd;
   *ret = sd->disabled;
}

EAPI void
elm_widget_show_region_set(Evas_Object *obj,
                           Evas_Coord x,
                           Evas_Coord y,
                           Evas_Coord w,
                           Evas_Coord h,
                           Eina_Bool forceshow)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_show_region_set(x, y, w, h, forceshow));
}

static void
_elm_widget_show_region_set(Eo *obj, void *_pd, va_list *list)
{
   Evas_Coord x = va_arg(*list, Evas_Coord);
   Evas_Coord y = va_arg(*list, Evas_Coord);
   Evas_Coord w = va_arg(*list, Evas_Coord);
   Evas_Coord h = va_arg(*list, Evas_Coord);
   Eina_Bool forceshow = va_arg(*list, int);

   Evas_Object *parent_obj, *child_obj;
   Evas_Coord px, py, cx, cy, nx, ny;

   Elm_Widget_Smart_Data *sd = _pd;

   evas_smart_objects_calculate(evas_object_evas_get(obj));

   if (!forceshow && (x == sd->rx) && (y == sd->ry) &&
       (w == sd->rw) && (h == sd->rh)) return;

   sd->rx = x;
   sd->ry = y;
   sd->rw = w;
   sd->rh = h;
   if (sd->on_show_region)
     {
        sd->on_show_region
           (sd->on_show_region_data, obj);

        if (_elm_scrollable_is(obj))
          {
             eo_do(obj, elm_scrollable_interface_content_pos_get(&nx, &ny));
             x -= nx;
             y -= ny;
          }
     }

   do
     {
        parent_obj = sd->parent_obj;
        child_obj = sd->obj;
        if ((!parent_obj) || (!_elm_widget_is(parent_obj))) break;
        sd = eo_data_scope_get(parent_obj, MY_CLASS);
        if (!sd) break;

        evas_object_geometry_get(parent_obj, &px, &py, NULL, NULL);
        evas_object_geometry_get(child_obj, &cx, &cy, NULL, NULL);

        x += (cx - px);
        y += (cy - py);
        sd->rx = x;
        sd->ry = y;
        sd->rw = w;
        sd->rh = h;

        if (sd->on_show_region)
          {
             sd->on_show_region
               (sd->on_show_region_data, parent_obj);
          }
     }
   while (parent_obj);
}

EAPI void
elm_widget_show_region_get(const Evas_Object *obj,
                           Evas_Coord *x,
                           Evas_Coord *y,
                           Evas_Coord *w,
                           Evas_Coord *h)
{
   ELM_WIDGET_CHECK(obj);
   eo_do((Eo *) obj, elm_wdg_show_region_get(x, y, w, h));
}

static void
_elm_widget_show_region_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord *x = va_arg(*list, Evas_Coord *);
   Evas_Coord *y = va_arg(*list, Evas_Coord *);
   Evas_Coord *w = va_arg(*list, Evas_Coord *);
   Evas_Coord *h = va_arg(*list, Evas_Coord *);

   Elm_Widget_Smart_Data *sd = _pd;

   if (x) *x = sd->rx;
   if (y) *y = sd->ry;
   if (w) *w = sd->rw;
   if (h) *h = sd->rh;
}

/**
 * @internal
 *
 * Get the focus region of the given widget.
 *
 * @return show region or not
 * (@c EINA_TRUE = show region/@c EINA_FALSE = do not show region). Default is @c EINA_FALSE.
 *
 * The focus region is the area of a widget that should brought into the
 * visible area when the widget is focused. Mostly used to show the part of
 * an entry where the cursor is, for example. The area returned is relative
 * to the object @p obj.
 *
 * @param obj The widget object
 * @param x Where to store the x coordinate of the area
 * @param y Where to store the y coordinate of the area
 * @param w Where to store the width of the area
 * @param h Where to store the height of the area
 *
 * @ingroup Widget
 */
EAPI Eina_Bool
elm_widget_focus_region_get(const Evas_Object *obj,
                            Evas_Coord *x,
                            Evas_Coord *y,
                            Evas_Coord *w,
                            Evas_Coord *h)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_wdg_focus_region_get(x, y, w, h, &ret));
   if (!ret)
     {
        evas_object_geometry_get(obj, NULL, NULL, w, h);
        if (x) *x = 0;
        if (y) *y = 0;
     }
   return EINA_TRUE;
}

static void
_elm_widget_focus_region_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Coord *x = va_arg(*list, Evas_Coord *);
   Evas_Coord *y = va_arg(*list, Evas_Coord *);
   Evas_Coord *w = va_arg(*list, Evas_Coord *);
   Evas_Coord *h = va_arg(*list, Evas_Coord *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);

   Eina_Bool int_ret = EINA_FALSE;
   eo_do((Eo *)obj, elm_wdg_on_focus_region(x, y, w, h, &int_ret));
   *ret = int_ret;
}

EAPI void
elm_widget_parents_bounce_get(Evas_Object *obj,
                              Eina_Bool *horiz, Eina_Bool *vert)
{
   ELM_WIDGET_CHECK(obj);
   eo_do((Eo *)obj, elm_wdg_parents_bounce_get(horiz, vert));
}

static void
_elm_widget_parents_bounce_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *horiz = va_arg(*list, Eina_Bool *);
   Eina_Bool *vert = va_arg(*list, Eina_Bool *);

   Evas_Object *parent_obj = obj;
   Eina_Bool h, v;

   *horiz = EINA_FALSE;
   *vert = EINA_FALSE;

   do
     {
        parent_obj = elm_widget_parent_get(parent_obj);
        if ((!parent_obj) || (!_elm_widget_is(parent_obj))) break;

        if (_elm_scrollable_is(parent_obj))
          {
             eo_do(parent_obj, elm_scrollable_interface_bounce_allow_get(&h, &v));
             if (h) *horiz = EINA_TRUE;
             if (v) *vert = EINA_TRUE;
          }
     }
   while (parent_obj);
}

EAPI Eina_List *
elm_widget_scrollable_children_get(Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) NULL;
   Eina_List *ret = NULL;
   eo_do(obj, elm_wdg_scrollable_children_get(&ret));
   return ret;
}

static void
_elm_widget_scrollable_children_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_List **ret = va_arg(*list, Eina_List **);
   *ret = NULL;

   Eina_List *l;
   Evas_Object *child;

   Elm_Widget_Smart_Data *sd = _pd;

   EINA_LIST_FOREACH(sd->subobjs, l, child)
     {
        if (_elm_scrollable_is(child))
          *ret = eina_list_append(*ret, child);
     }
}

EAPI void
elm_widget_scroll_hold_push(Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_scroll_hold_push());
}

static void
_elm_widget_scroll_hold_push(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Widget_Smart_Data *sd = _pd;
   sd->scroll_hold++;
   if (sd->scroll_hold == 1)
     {
        if (_elm_scrollable_is(obj))
           eo_do(obj, elm_scrollable_interface_hold_set(EINA_TRUE));
        else
          {
             Eina_List *scr_children, *l;
             Evas_Object *child;

             scr_children = elm_widget_scrollable_children_get(obj);
             EINA_LIST_FOREACH(scr_children, l, child)
               {
                  eo_do(child, elm_scrollable_interface_hold_set(EINA_TRUE));
               }
             eina_list_free(scr_children);
          }
     }
   if (sd->parent_obj) eo_do(sd->parent_obj, elm_wdg_scroll_hold_push());
   // FIXME: on delete/reparent hold pop
}

EAPI void
elm_widget_scroll_hold_pop(Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_scroll_hold_pop());
}

static void
_elm_widget_scroll_hold_pop(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Widget_Smart_Data *sd = _pd;
   sd->scroll_hold--;
   if (!sd->scroll_hold)
     {
        if (_elm_scrollable_is(obj))
           eo_do(obj, elm_scrollable_interface_hold_set(EINA_FALSE));
        else
          {
             Eina_List *scr_children, *l;
             Evas_Object *child;

             scr_children = elm_widget_scrollable_children_get(obj);
             EINA_LIST_FOREACH(scr_children, l, child)
               {
                  eo_do(child, elm_scrollable_interface_hold_set(EINA_FALSE));
               }
             eina_list_free(scr_children);
          }
     }
   if (sd->parent_obj) eo_do(sd->parent_obj, elm_wdg_scroll_hold_pop());
   if (sd->scroll_hold < 0) sd->scroll_hold = 0;
}

EAPI int
elm_widget_scroll_hold_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) 0;
   int ret = 0;
   eo_do((Eo *) obj, elm_wdg_scroll_hold_get(&ret));
   return ret;
}

static void
_elm_widget_scroll_hold_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);
   Elm_Widget_Smart_Data *sd = _pd;
   *ret = sd->scroll_hold;
}

EAPI void
elm_widget_scroll_freeze_push(Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_scroll_freeze_push());
}

static void
_elm_widget_scroll_freeze_push(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Widget_Smart_Data *sd = _pd;
   sd->scroll_freeze++;
   if (sd->scroll_freeze == 1)
     {
        if (_elm_scrollable_is(obj))
           eo_do(obj, elm_scrollable_interface_freeze_set(EINA_TRUE));
        else
          {
             Eina_List *scr_children, *l;
             Evas_Object *child;

             scr_children = elm_widget_scrollable_children_get(obj);
             EINA_LIST_FOREACH(scr_children, l, child)
               {
                  eo_do(child, elm_scrollable_interface_freeze_set(EINA_TRUE));
               }
             eina_list_free(scr_children);
          }
     }
   if (sd->parent_obj) eo_do(sd->parent_obj, elm_wdg_scroll_freeze_push());
   // FIXME: on delete/reparent freeze pop
}

EAPI void
elm_widget_scroll_freeze_pop(Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_scroll_freeze_pop());
}

static void
_elm_widget_scroll_freeze_pop(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Widget_Smart_Data *sd = _pd;
   sd->scroll_freeze--;
   if (!sd->scroll_freeze)
     {
        if (_elm_scrollable_is(obj))
           eo_do(obj, elm_scrollable_interface_freeze_set(EINA_FALSE));
        else
          {
             Eina_List *scr_children, *l;
             Evas_Object *child;

             scr_children = elm_widget_scrollable_children_get(obj);
             EINA_LIST_FOREACH(scr_children, l, child)
               {
                  eo_do(child, elm_scrollable_interface_freeze_set(EINA_FALSE));
               }
             eina_list_free(scr_children);
          }
     }
   if (sd->parent_obj) eo_do(sd->parent_obj, elm_wdg_scroll_freeze_pop());
   if (sd->scroll_freeze < 0) sd->scroll_freeze = 0;
}

EAPI int
elm_widget_scroll_freeze_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) 0;
   int ret = 0;
   eo_do((Eo *) obj, elm_wdg_scroll_freeze_get(&ret));
   return ret;
}

static void
_elm_widget_scroll_freeze_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);
   Elm_Widget_Smart_Data *sd = _pd;
   *ret = sd->scroll_freeze;
}

EAPI void
elm_widget_scale_set(Evas_Object *obj,
                     double scale)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_scale_set(scale));
}

static void
_elm_widget_scale_set(Eo *obj, void *_pd, va_list *list)
{
   double scale = va_arg(*list, double);
   Elm_Widget_Smart_Data *sd = _pd;

   if (scale <= 0.0) scale = 0.0;
   if (sd->scale != scale)
     {
        sd->scale = scale;
        elm_widget_theme(obj);
     }
}

EAPI double
elm_widget_scale_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) 1.0;
   double ret = 1.0;
   eo_do((Eo *) obj, elm_wdg_scale_get(&ret));
   return ret;
}

static void
_elm_widget_scale_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double *ret = va_arg(*list, double *);
   Elm_Widget_Smart_Data *sd = _pd;

   // FIXME: save walking up the tree by storing/caching parent scale
   if (sd->scale == 0.0)
     {
        if (sd->parent_obj)
          {
             *ret = elm_widget_scale_get(sd->parent_obj);
             return;
          }
        else
          {
             *ret = 1.0;
             return;
          }
     }
   *ret = sd->scale;
}

EAPI void
elm_widget_theme_set(Evas_Object *obj,
                     Elm_Theme *th)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_theme_set(th));
}

static void
_elm_widget_theme_set(Eo *obj, void *_pd, va_list *list)
{
   Elm_Theme *th = va_arg(*list, Elm_Theme *);
   Elm_Widget_Smart_Data *sd = _pd;
   Eina_Bool apply = EINA_FALSE;
   if (sd->theme != th)
     {
        if (elm_widget_theme_get(obj) != th) apply = EINA_TRUE;
        if (sd->theme) elm_theme_free(sd->theme);
        sd->theme = th;
        if (th) th->ref++;
        if (apply) elm_widget_theme(obj);
     }
}

EAPI void
elm_widget_part_text_set(Evas_Object *obj,
                         const char *part,
                         const char *label)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_part_text_set(part, label));
}

static void
_elm_widget_part_text_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const char *label = va_arg(*list, const char *);
   if (evas_object_smart_type_check(obj, "elm_layout"))
     elm_layout_text_set(obj, part, label);
}

EAPI const char *
elm_widget_part_text_get(const Evas_Object *obj,
                         const char *part)
{
   ELM_WIDGET_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_wdg_part_text_get(part, &ret));
   return ret;
}

static void
_elm_widget_part_text_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const char **ret = va_arg(*list, const char **);
   if (evas_object_smart_type_check(obj, "elm_layout"))
     *ret = elm_layout_text_get(obj, part);
}

EAPI void
elm_widget_domain_translatable_part_text_set(Evas_Object *obj,
                                             const char *part,
                                             const char *domain,
                                             const char *label)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_domain_translatable_part_text_set(part, domain, label));
}

static Elm_Translate_String_Data *
_translate_string_data_get(Eina_Inlist *translate_strings, const char *part)
{
   Elm_Translate_String_Data *ts;
   Eina_Stringshare *str;

   if (!translate_strings) return NULL;

   str = eina_stringshare_add(part);
   EINA_INLIST_FOREACH(translate_strings, ts)
     {
        if (ts->id == str) break;
     }

   eina_stringshare_del(str);

   return ts;
}

static Elm_Translate_String_Data *
_part_text_translatable_set(Eina_Inlist **translate_strings, const char *part, Eina_Bool translatable, Eina_Bool preset)
{
   Eina_Inlist *t;
   Elm_Translate_String_Data *ts;
   t = *translate_strings;
   ts = _translate_string_data_get(t, part);

   if (translatable)
     {
        if (!ts)
          {
             ts = ELM_NEW(Elm_Translate_String_Data);
             if (!ts) return NULL;

             ts->id = eina_stringshare_add(part);
             t = eina_inlist_append(t, (Eina_Inlist*) ts);
          }
        if (preset) ts->preset = EINA_TRUE;
     }
   //Delete this exist one if this part has been not preset.
   //see elm_widget_part_text_translatable_set()
   else if ((preset) || (!ts->preset))
     {
        if (ts)
          {
             t = eina_inlist_remove(t, EINA_INLIST_GET(ts));
             eina_stringshare_del(ts->id);
             eina_stringshare_del(ts->domain);
             eina_stringshare_del(ts->string);
             ELM_SAFE_FREE(ts, free);
          }
     }

   *translate_strings = t;

   return ts;
}

static void
_elm_widget_domain_translatable_part_text_set(Eo *obj, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const char *domain = va_arg(*list, const char *);
   const char *label = va_arg(*list, const char *);

   Elm_Widget_Smart_Data *sd = _pd;
   Elm_Translate_String_Data *ts;

   if (!label)
     {
        _part_text_translatable_set(&sd->translate_strings, part, EINA_FALSE,
                                    EINA_FALSE);
     }
   else
     {
        ts = _part_text_translatable_set(&sd->translate_strings, part,
                                         EINA_TRUE, EINA_FALSE);
        if (!ts) return;
        if (!ts->string) ts->string = eina_stringshare_add(label);
        else eina_stringshare_replace(&ts->string, label);
        if (!ts->domain) ts->domain = eina_stringshare_add(domain);
        else eina_stringshare_replace(&ts->domain, domain);
#ifdef HAVE_GETTEXT
        if (label[0]) label = dgettext(domain, label);
#endif
     }

   sd->on_translate = EINA_TRUE;
   eo_do(obj, elm_wdg_part_text_set(part, label));
   sd->on_translate = EINA_FALSE;
}

EAPI const char *
elm_widget_translatable_part_text_get(const Evas_Object *obj,
                                      const char *part)
{
   ELM_WIDGET_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_wdg_translatable_part_text_get(part, &ret));
   return ret;
}

static void
_elm_widget_translatable_part_text_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const char **ret = va_arg(*list, const char **);

   Elm_Widget_Smart_Data *sd = _pd;
   Elm_Translate_String_Data *ts;

   ts = _translate_string_data_get(sd->translate_strings, part);
   if (ts) *ret = ts->string;
   else *ret = NULL;
}

EAPI void
elm_widget_domain_part_text_translatable_set(Evas_Object *obj, const char *part, const char *domain, Eina_Bool translatable)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj,
         elm_wdg_domain_part_text_translatable_set(part, domain, translatable));
}

static void
_elm_widget_domain_part_text_translatable_set(Eo *obj, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const char *domain = va_arg(*list, const char *);
   Eina_Bool translatable = va_arg(*list, int);

   Elm_Widget_Smart_Data *sd = _pd;
   Elm_Translate_String_Data *ts;
   const char *text;

   ts = _part_text_translatable_set(&sd->translate_strings, part,
                                    translatable, EINA_TRUE);
   if (!ts) return;
   if (!ts->domain) ts->domain = eina_stringshare_add(domain);
   else eina_stringshare_replace(&ts->domain, domain);

   eo_do(obj, elm_wdg_part_text_get(part, &text));
   if (!text || !text[0]) return;

   if (!ts->string) ts->string = eina_stringshare_add(text);

//Try to translate text since we don't know the text is already translated.
#ifdef HAVE_GETTEXT
   text = dgettext(domain, text);
#endif
   sd->on_translate = EINA_TRUE;
   eo_do(obj, elm_wdg_part_text_set(part, text));
   sd->on_translate = EINA_FALSE;
}

EAPI void
elm_widget_translate(Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_translate(NULL));
}

static const char*
_part_text_translate(Eina_Inlist *translate_strings,
                     const char *part,
                     const char *text)
{
   Elm_Translate_String_Data *ts;
   ts = _translate_string_data_get(translate_strings, part);
   if (!ts) return text;

   if (!ts->string) ts->string = eina_stringshare_add(text);
   else eina_stringshare_replace(&ts->string, text);
#ifdef HAVE_GETTEXT
   if (text && text[0])
     text = dgettext(ts->domain, text);
#endif
   return text;
}

EAPI const char *
elm_widget_part_text_translate(Evas_Object *obj, const char *part, const char *text)
{
  ELM_WIDGET_CHECK(obj) NULL;
  const char *ret;
  eo_do(obj, elm_wdg_part_text_translate(part, text, &ret));
  return ret;
}

static void
_elm_widget_part_text_translate(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const char *text = va_arg(*list, const char *);
   const char **ret = va_arg(*list, const char **);

   Elm_Widget_Smart_Data *sd = _pd;

   if (!sd->translate_strings || sd->on_translate)
     {
        *ret = text;
        return;
     }
   *ret = _part_text_translate(sd->translate_strings, part, text);
}

static void
_elm_widget_translate(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   const Eina_List *l;
   Evas_Object *child;
   API_ENTRY return;

   EINA_LIST_FOREACH(sd->subobjs, l, child)
     elm_widget_translate(child);
   if (sd->hover_obj) elm_widget_translate(sd->hover_obj);

#ifdef HAVE_GETTEXT
   Elm_Translate_String_Data *ts;
   EINA_INLIST_FOREACH(sd->translate_strings, ts)
     {
        if (!ts->string) continue;
        const char *s = dgettext(ts->domain, ts->string);
        sd->on_translate = EINA_TRUE;
        eo_do(obj, elm_wdg_part_text_set(ts->id, s));
        sd->on_translate = EINA_FALSE;
     }
#endif
   evas_object_smart_callback_call(obj, SIG_WIDGET_LANG_CHANGED, NULL);
}

EAPI void
elm_widget_content_part_set(Evas_Object *obj,
                            const char *part,
                            Evas_Object *content)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_obj_container_content_set(part, content, NULL));
}

EAPI Evas_Object *
elm_widget_content_part_get(const Evas_Object *obj,
                            const char *part)
{
   ELM_WIDGET_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do((Eo *) obj, elm_obj_container_content_get(part, &ret));
   return ret;
}

EAPI Evas_Object *
elm_widget_content_part_unset(Evas_Object *obj,
                              const char *part)
{
   ELM_WIDGET_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do(obj, elm_obj_container_content_unset(part, &ret));
   return ret;
}

EAPI void
elm_widget_access_info_set(Evas_Object *obj,
                           const char *txt)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_access_info_set(txt));
}

static void
_elm_widget_access_info_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *txt = va_arg(*list, const char *);
   Elm_Widget_Smart_Data *sd = _pd;
   eina_stringshare_del(sd->access_info);
   if (!txt) sd->access_info = NULL;
   else sd->access_info = eina_stringshare_add(txt);
}

EAPI const char *
elm_widget_access_info_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_wdg_access_info_get(&ret));
   return ret;
}

static void
_elm_widget_access_info_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **ret = va_arg(*list, const char **);
   Elm_Widget_Smart_Data *sd = _pd;
   *ret = sd->access_info;
}

EAPI Elm_Theme *
elm_widget_theme_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) NULL;
   Elm_Theme *ret = NULL;
   eo_do((Eo *) obj, elm_wdg_theme_get(&ret));
   return ret;
}

static void
_elm_widget_theme_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Theme **ret = va_arg(*list, Elm_Theme **);
   *ret = NULL;
   Elm_Widget_Smart_Data *sd = _pd;

   if (!sd->theme)
     {
        if (sd->parent_obj)
          {
             *ret = elm_widget_theme_get(sd->parent_obj);
             return;
          }
        else
          return;
     }
   *ret = sd->theme;
}

EAPI Eina_Bool
elm_widget_style_set(Evas_Object *obj,
                     const char *style)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_wdg_style_set(style, &ret));
   return ret;
}

static void
_elm_widget_style_set(Eo *obj, void *_pd, va_list *list)
{
   const char *style = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;
   Elm_Widget_Smart_Data *sd = _pd;
   if (eina_stringshare_replace(&sd->style, style))
     {
        int_ret = elm_widget_theme(obj);
        if (ret) *ret = int_ret;
        return;
     }

   if (ret) *ret = EINA_TRUE;
}

EAPI const char *
elm_widget_style_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) NULL;
   const char *ret = "default";
   eo_do((Eo *) obj, elm_wdg_style_get(&ret));
   return ret;
}

static void
_elm_widget_style_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **ret = va_arg(*list, const char **);
   *ret = "default";
   Elm_Widget_Smart_Data *sd = _pd;
   if (sd->style) *ret = sd->style;
}

EAPI void
elm_widget_tooltip_add(Evas_Object *obj,
                       Elm_Tooltip *tt)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_tooltip_add(tt));
}

static void
_elm_widget_tooltip_add(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Tooltip *tt = va_arg(*list, Elm_Tooltip *);
   Elm_Widget_Smart_Data *sd = _pd;
   sd->tooltips = eina_list_append(sd->tooltips, tt);
}

EAPI void
elm_widget_tooltip_del(Evas_Object *obj,
                       Elm_Tooltip *tt)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_tooltip_del(tt));
}

static void
_elm_widget_tooltip_del(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Tooltip *tt = va_arg(*list, Elm_Tooltip *);
   Elm_Widget_Smart_Data *sd = _pd;
   sd->tooltips = eina_list_remove(sd->tooltips, tt);
}

EAPI void
elm_widget_cursor_add(Evas_Object *obj,
                      Elm_Cursor *cur)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_cursor_add(cur));
}

static void
_elm_widget_cursor_add(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Cursor *cur = va_arg(*list, Elm_Cursor *);
   Elm_Widget_Smart_Data *sd = _pd;
   sd->cursors = eina_list_append(sd->cursors, cur);
}

EAPI void
elm_widget_cursor_del(Evas_Object *obj,
                      Elm_Cursor *cur)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_cursor_del(cur));
}

static void
_elm_widget_cursor_del(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Cursor *cur = va_arg(*list, Elm_Cursor *);
   Elm_Widget_Smart_Data *sd = _pd;
   sd->cursors = eina_list_remove(sd->cursors, cur);
}

EAPI void
elm_widget_drag_lock_x_set(Evas_Object *obj,
                           Eina_Bool lock)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_drag_lock_x_set(lock));
}

static void
_elm_widget_drag_lock_x_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool lock = va_arg(*list, int);
   Elm_Widget_Smart_Data *sd = _pd;
   if (sd->drag_x_locked == lock) return;
   sd->drag_x_locked = lock;
   if (sd->drag_x_locked) _propagate_x_drag_lock(obj, 1);
   else _propagate_x_drag_lock(obj, -1);
}

EAPI void
elm_widget_drag_lock_y_set(Evas_Object *obj,
                           Eina_Bool lock)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_drag_lock_y_set(lock));
}

static void
_elm_widget_drag_lock_y_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool lock = va_arg(*list, int);
   Elm_Widget_Smart_Data *sd = _pd;
   if (sd->drag_y_locked == lock) return;
   sd->drag_y_locked = lock;
   if (sd->drag_y_locked) _propagate_y_drag_lock(obj, 1);
   else _propagate_y_drag_lock(obj, -1);
}

EAPI Eina_Bool
elm_widget_drag_lock_x_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_wdg_drag_lock_x_get(&ret));
   return ret;
}

static void
_elm_widget_drag_lock_x_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Widget_Smart_Data *sd = _pd;
   *ret = sd->drag_x_locked;
}

EAPI Eina_Bool
elm_widget_drag_lock_y_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_wdg_drag_lock_y_get(&ret));
   return ret;
}

static void
_elm_widget_drag_lock_y_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Widget_Smart_Data *sd = _pd;
   *ret = sd->drag_y_locked;
}

EAPI int
elm_widget_drag_child_locked_x_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) 0;
   int ret = 0;
   eo_do((Eo *) obj, elm_wdg_drag_child_locked_x_get(&ret));
   return ret;
}

static void
_elm_widget_drag_child_locked_x_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);
   Elm_Widget_Smart_Data *sd = _pd;
   *ret = sd->child_drag_x_locked;
}

EAPI int
elm_widget_drag_child_locked_y_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) 0;
   int ret = 0;
   eo_do((Eo *) obj, elm_wdg_drag_child_locked_y_get(&ret));
   return ret;
}

static void
_elm_widget_drag_child_locked_y_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);
   Elm_Widget_Smart_Data *sd = _pd;
   *ret = sd->child_drag_y_locked;
}

EAPI Eina_Bool
elm_widget_theme_object_set(Evas_Object *obj,
                            Evas_Object *edj,
                            const char *wname,
                            const char *welement,
                            const char *wstyle)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_wdg_theme_object_set(edj, wname, welement, wstyle, &ret));
   return ret;
}

static void
_elm_widget_theme_object_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *edj = va_arg(*list, Evas_Object *);
   const char *wname = va_arg(*list, const char *);
   const char *welement = va_arg(*list, const char *);
   const char *wstyle = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Widget_Smart_Data *sd = _pd;

   if (!_elm_theme_object_set(obj, edj, wname, welement, wstyle))
     {
        if (ret) *ret = EINA_FALSE;
        return;
     }

   if (sd->orient_mode != -1)
     {
        char buf[128];
        snprintf(buf, sizeof(buf), "elm,state,orient,%d", sd->orient_mode);
        eo_do(obj, elm_wdg_signal_emit(buf, "elm"));
     }

   if (ret) *ret = EINA_TRUE;
}

static void
_dbg_info_get(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eo_Dbg_Info *root = (Eo_Dbg_Info *) va_arg(*list, Eo_Dbg_Info *);
   eo_do_super(eo_obj, MY_CLASS, eo_dbg_info_get(root));
   Eo_Dbg_Info *group = EO_DBG_INFO_LIST_APPEND(root, MY_CLASS_NAME);

   EO_DBG_INFO_APPEND(group, "Wid-Type", EINA_VALUE_TYPE_STRING, elm_widget_type_get(eo_obj));
   EO_DBG_INFO_APPEND(group, "Style", EINA_VALUE_TYPE_STRING, elm_widget_style_get(eo_obj));
   EO_DBG_INFO_APPEND(group, "Layer", EINA_VALUE_TYPE_INT,
         (int) evas_object_layer_get(eo_obj));
   EO_DBG_INFO_APPEND(group, "Scale", EINA_VALUE_TYPE_DOUBLE,
         evas_object_scale_get(eo_obj));
   EO_DBG_INFO_APPEND(group, "Has focus", EINA_VALUE_TYPE_CHAR,
         elm_object_focus_get(eo_obj));
   EO_DBG_INFO_APPEND(group, "Disabled", EINA_VALUE_TYPE_CHAR,
         elm_widget_disabled_get(eo_obj));
   EO_DBG_INFO_APPEND(group, "Mirrored", EINA_VALUE_TYPE_CHAR,
         elm_widget_mirrored_get(eo_obj));
   EO_DBG_INFO_APPEND(group, "Automatic mirroring", EINA_VALUE_TYPE_CHAR,
         elm_widget_mirrored_automatic_get(eo_obj));
}

EAPI Eina_Bool
elm_widget_is_check(const Evas_Object *obj)
{
   static int abort_on_warn = -1;
   if (elm_widget_is(obj))
     return EINA_TRUE;

   ERR("Passing Object: %p.", obj);
   if (abort_on_warn == -1)
     {
        if (getenv("ELM_ERROR_ABORT")) abort_on_warn = 1;
        else abort_on_warn = 0;
     }
   if (abort_on_warn == 1) abort();
   return EINA_FALSE;
}

EAPI const char *
elm_widget_type_get(const Evas_Object *obj)
{
   API_ENTRY return NULL;

   return eo_class_name_get(eo_class_get(obj));
}

EAPI Eina_Bool
elm_widget_type_check(const Evas_Object *obj,
                      const char *type,
                      const char *func)
{
   const char *provided, *expected = "(unknown)";
   static int abort_on_warn = -1;

   provided = elm_widget_type_get(obj);
   /* TODO: eventually migrate to check_ptr version */
   if (evas_object_smart_type_check(obj, type)) return EINA_TRUE;
   if (type) expected = type;
   if ((!provided) || (!provided[0]))
     {
        provided = evas_object_type_get(obj);
        if ((!provided) || (!provided[0]))
          provided = "(unknown)";
     }
   ERR("Passing Object: %p in function: %s, of type: '%s' when expecting"
       " type: '%s'", obj, func, provided, expected);
   if (abort_on_warn == -1)
     {
        if (getenv("ELM_ERROR_ABORT")) abort_on_warn = 1;
        else abort_on_warn = 0;
     }
   if (abort_on_warn == 1) abort();
   return EINA_FALSE;
}

static Evas_Object *
_widget_name_find(const Evas_Object *obj,
                  const char *name,
                  int recurse)
{
   Eina_List *l;
   Evas_Object *child;
   const char *s;
   INTERNAL_ENTRY NULL;

   if (!_elm_widget_is(obj)) return NULL;
   EINA_LIST_FOREACH(sd->subobjs, l, child)
     {
        s = evas_object_name_get(child);
        if ((s) && (!strcmp(s, name))) return child;
        if ((recurse != 0) &&
            ((child = _widget_name_find(child, name, recurse - 1))))
          return child;
     }
   if (sd->hover_obj)
     {
        s = evas_object_name_get(sd->hover_obj);
        if ((s) && (!strcmp(s, name))) return sd->hover_obj;
        if ((recurse != 0) &&
            ((child = _widget_name_find(sd->hover_obj, name, recurse - 1))))
          return child;
     }
   return NULL;
}

EAPI Evas_Object *
elm_widget_name_find(const Evas_Object *obj,
                     const char *name,
                     int recurse)
{
   ELM_WIDGET_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do((Eo *) obj, elm_wdg_name_find(name, recurse, &ret));
   return ret;
}

static void
_elm_widget_name_find(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *name = va_arg(*list, const char *);
   int recurse = va_arg(*list, int);
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   *ret = NULL;
   if (!name) return;
   *ret = _widget_name_find(obj, name, recurse);
}

/**
 * @internal
 *
 * Split string in words
 *
 * @param str Source string
 * @return List of const words
 *
 * @see elm_widget_stringlist_free()
 * @ingroup Widget
 */
EAPI Eina_List *
elm_widget_stringlist_get(const char *str)
{
   Eina_List *list = NULL;
   const char *s, *b;
   if (!str) return NULL;
   for (b = s = str; 1; s++)
     {
        if ((*s == ' ') || (!*s))
          {
             char *t = malloc(s - b + 1);
             if (t)
               {
                  strncpy(t, b, s - b);
                  t[s - b] = 0;
                  list = eina_list_append(list, eina_stringshare_add(t));
                  free(t);
               }
             b = s + 1;
          }
        if (!*s) break;
     }
   return list;
}

EAPI void
elm_widget_stringlist_free(Eina_List *list)
{
   const char *s;
   EINA_LIST_FREE(list, s)
     eina_stringshare_del(s);
}

EAPI void
elm_widget_focus_hide_handle(Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_focus_hide_handle());
}

static void
_elm_widget_focus_hide_handle(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   if (!_elm_widget_is(obj))
     return;
   _if_focused_revert(obj, EINA_TRUE);
}

EAPI void
elm_widget_focus_mouse_up_handle(Evas_Object *obj)
{
   Evas_Object *o = obj;
   do
     {
        if (_elm_widget_is(o)) break;
        o = evas_object_smart_parent_get(o);
     }
   while (o);

   eo_do(o, elm_wdg_focus_mouse_up_handle());
}

static void
_elm_widget_focus_mouse_up_handle(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   if (!obj) return;
   if (!_is_focusable(obj)) return;
   elm_widget_focus_steal(obj);
}

EAPI void
elm_widget_focus_tree_unfocusable_handle(Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_focus_tree_unfocusable_handle());
}

static void
_elm_widget_focus_tree_unfocusable_handle(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   if (!elm_widget_parent_get(obj))
     elm_widget_focused_object_clear(obj);
   else
     _if_focused_revert(obj, EINA_TRUE);
}

EAPI void
elm_widget_focus_disabled_handle(Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_focus_disabled_handle());
}

static void
_elm_widget_focus_disabled_handle(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   elm_widget_focus_tree_unfocusable_handle(obj);
}

EAPI unsigned int
elm_widget_focus_order_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) 0;
   unsigned int ret = 0;
   eo_do((Eo *) obj, elm_wdg_focus_order_get(&ret));
   return ret;
}

static void
_elm_widget_focus_order_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   unsigned int *ret = va_arg(*list, unsigned int *);
   Elm_Widget_Smart_Data *sd = _pd;
   *ret = sd->focus_order;
}

EAPI Evas_Object *
elm_widget_newest_focus_order_get(const Evas_Object *obj,
                                  unsigned int *newest_focus_order,
                                  Eina_Bool can_focus_only)
{
   ELM_WIDGET_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do((Eo *) obj, elm_wdg_newest_focus_order_get(newest_focus_order, can_focus_only, &ret));
   return ret;
}

static void
_elm_widget_newest_focus_order_get(Eo *obj, void *_pd, va_list *list)
{
   unsigned int *newest_focus_order = va_arg(*list, unsigned int *);
   Eina_Bool can_focus_only = va_arg(*list, int);
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   Elm_Widget_Smart_Data *sd = _pd;
   *ret = NULL;

   const Eina_List *l;
   Evas_Object *child, *cur, *best;

   if (!evas_object_visible_get(obj)
       || (elm_widget_disabled_get(obj))
       || (elm_widget_tree_unfocusable_get(obj)))
     return;

   best = NULL;
   if (*newest_focus_order < sd->focus_order)
     {
        if (!can_focus_only || elm_widget_can_focus_get(obj))
          {
             *newest_focus_order = sd->focus_order;
             best = obj;
          }
     }
   EINA_LIST_FOREACH(sd->subobjs, l, child)
     {
        cur = elm_widget_newest_focus_order_get
           (child, newest_focus_order, can_focus_only);
        if (!cur) continue;
        best = cur;
     }
   *ret = best;
   return;
}

EAPI void
elm_widget_focus_highlight_geometry_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h, Eina_Bool is_next)
{
   ELM_WIDGET_CHECK(obj);
   eo_do(obj, elm_wdg_focus_highlight_geometry_get(x, y, w, h, &is_next));
}

EAPI void
elm_widget_focus_highlight_focus_part_geometry_get(const Evas_Object *obj,
                                                   Evas_Coord *x,
                                                   Evas_Coord *y,
                                                   Evas_Coord *w,
                                                   Evas_Coord *h)
{
   Evas_Coord tx = 0, ty = 0, tw = 0, th = 0;
   const char *target_hl_part = NULL;
   const Evas_Object *edje_obj = NULL;

   if (obj && eo_isa(obj, EDJE_OBJ_CLASS))
     {
        edje_obj = obj;
        if (!(target_hl_part = edje_object_data_get(edje_obj, "focus_part")))
          return;
     }
   else if (obj && eo_isa(obj, ELM_OBJ_LAYOUT_CLASS))
     {
        edje_obj = elm_layout_edje_get(obj);
        if (!(target_hl_part = elm_layout_data_get(obj, "focus_part")))
          return;
     }
   else
     return;

  edje_object_part_geometry_get(edje_obj, target_hl_part,
                                &tx, &ty, &tw, &th);
  *x += tx;
  *y += ty;
  if (tw != *w) *w = tw;
  if (th != *h) *h = th;
}

static void
_elm_widget_focus_highlight_geometry_get(Eo *obj, void *_pd, va_list *list)
{
   Evas_Coord *x = va_arg(*list, Evas_Coord *);
   Evas_Coord *y = va_arg(*list, Evas_Coord *);
   Evas_Coord *w = va_arg(*list, Evas_Coord *);
   Evas_Coord *h = va_arg(*list, Evas_Coord *);
   Eina_Bool *is_next = va_arg(*list, Eina_Bool *);
   (void)is_next;

   Elm_Widget_Smart_Data *sd = _pd;

   evas_object_geometry_get(obj, x, y, w, h);
   elm_widget_focus_highlight_focus_part_geometry_get(sd->resize_obj, x, y, w, h);
}

EAPI Elm_Object_Item *
elm_widget_focused_item_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do(obj, elm_wdg_focused_item_get(&ret));

   return ret;
}

static void
_elm_widget_focused_item_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);

   if (ret) *ret = NULL;
}

EAPI void
elm_widget_activate(Evas_Object *obj, Elm_Activate act)
{
   Evas_Object *parent;
   Eina_Bool ret;

   ELM_WIDGET_CHECK(obj);

   ret = EINA_FALSE;

   eo_do(obj, elm_wdg_activate(act, &ret));

   if (ret) return;

   parent = elm_widget_parent_get(obj);
   if (parent)
     elm_widget_activate(parent, act);

   return;
}

/**
 * @internal
 *
 * Sets the widget and child widget's Evas_Display_Mode.
 *
 * @param obj The widget.
 * @param dispmode Evas_Display_Mode to set widget's mode.
 *
 * Widgets are resized by several reasons.
 * Evas_Display_Mode can help for widgets to get one more reason of resize.
 * For example, elm conform widget resizes it's contents when keypad state changed.
 * After keypad showing, conform widget can change child's Evas_Display_Mode.
 * @ingroup Widget
 */
EAPI void
elm_widget_display_mode_set(Evas_Object *obj, Evas_Display_Mode dispmode)
{
   ELM_WIDGET_CHECK(obj);
   eo_do((Eo *) obj, elm_wdg_display_mode_set(dispmode));
}

static void
_elm_widget_display_mode_set(Eo *obj, void *_pd, va_list *list)
{
   Evas_Display_Mode dispmode = va_arg(*list, Evas_Display_Mode);
   Evas_Display_Mode prev_dispmode;
   Evas_Object *child;
   Eina_List *l;

   Elm_Widget_Smart_Data *sd = _pd;

   prev_dispmode = evas_object_size_hint_display_mode_get(obj);

   if ((prev_dispmode == dispmode) ||
       (prev_dispmode == EVAS_DISPLAY_MODE_DONT_CHANGE)) return;

   evas_object_size_hint_display_mode_set(obj, dispmode);

   EINA_LIST_FOREACH (sd->subobjs, l, child)
     elm_widget_display_mode_set(child, dispmode);
}

EAPI void
elm_widget_orientation_mode_disabled_set(Evas_Object *obj, Eina_Bool disabled)
{
   ELM_WIDGET_CHECK(obj);
   ELM_WIDGET_DATA_GET(obj, sd);
   if (disabled && (sd->orient_mode == -1)) return;
   if (!disabled && (sd->orient_mode != -1)) return;
   eo_do((Eo *) obj, elm_wdg_orientation_mode_disabled_set(disabled));
}

static void
_elm_widget_orientation_mode_disabled_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool disabled = va_arg(*list, int);
   int orient_mode = -1;
   Elm_Widget_Smart_Data *sd = _pd;

   if (!disabled)
     {
        //Get current orient mode from it's parent otherwise, 0.
        sd->orient_mode = 0;
        ELM_WIDGET_DATA_GET(sd->parent_obj, sd_parent);
        if (!sd_parent) orient_mode = 0;
        else orient_mode = sd_parent->orient_mode;
     }
   eo_do(obj, elm_wdg_orientation_set(orient_mode));
}

EAPI Eina_Bool
elm_widget_orientation_mode_disabled_get(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;
   Eina_Bool ret;
   eo_do((Eo *) obj, elm_wdg_orientation_mode_disabled_get(&ret));
   return ret;
}

static void
_elm_widget_orientation_mode_disabled_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Widget_Smart_Data *sd = _pd;
   if (sd->orient_mode == -1) *ret = EINA_TRUE;
   else *ret = EINA_FALSE;
}

EAPI void
elm_widget_orientation_set(Evas_Object *obj, int rotation)
{
   ELM_WIDGET_CHECK(obj);
   ELM_WIDGET_DATA_GET(obj, sd);
   if ((sd->orient_mode == rotation) || (sd->orient_mode == -1)) return;
   eo_do((Eo *) obj, elm_wdg_orientation_set(rotation));
}

static void
_elm_widget_orientation_set(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *child;
   Eina_List *l;
   int orient_mode = va_arg(*list, int);
   Elm_Widget_Smart_Data *sd = _pd;

   sd->orient_mode = orient_mode;

   EINA_LIST_FOREACH (sd->subobjs, l, child)
     elm_widget_orientation_set(child, orient_mode);

   if (orient_mode != -1)
     {
        char buf[128];
        snprintf(buf, sizeof(buf), "elm,state,orient,%d", orient_mode);
        eo_do(obj, elm_wdg_signal_emit(buf, "elm"));
     }
}

static void
_track_obj_del(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void
_track_obj_update(Evas_Object *track, Evas_Object *obj)
{
   //Geometry
   Evas_Coord x, y, w, h;
   evas_object_geometry_get(obj, &x, &y, &w, &h);
   evas_object_move(track, x, y);
   evas_object_resize(track, w, h);

   //Visibility
   if (evas_object_visible_get(obj)) evas_object_show(track);
   else evas_object_hide(track);
}

static void
_track_obj_view_update(void *data, Evas *e EINA_UNUSED, Evas_Object *obj,
                       void *event_info EINA_UNUSED)
{
   Evas_Object *track = data;
   _track_obj_update(track, obj);
}

static void
_track_obj_view_del(void *data, Evas *e EINA_UNUSED,
                    Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Widget_Item *item = data;

   while (evas_object_ref_get(item->track_obj) > 0)
     evas_object_unref(item->track_obj);

   evas_object_event_callback_del(item->track_obj, EVAS_CALLBACK_DEL,
                                  _track_obj_del);
   evas_object_del(item->track_obj);
   item->track_obj = NULL;
}

static void
_track_obj_del(void *data, Evas *e EINA_UNUSED,
                    Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Widget_Item *item = data;
   item->track_obj = NULL;

   if (!item->view) return;

   evas_object_event_callback_del(item->view, EVAS_CALLBACK_RESIZE,
                                  _track_obj_view_update);
   evas_object_event_callback_del(item->view, EVAS_CALLBACK_MOVE,
                                  _track_obj_view_update);
   evas_object_event_callback_del(item->view, EVAS_CALLBACK_SHOW,
                                  _track_obj_view_update);
   evas_object_event_callback_del(item->view, EVAS_CALLBACK_HIDE,
                                  _track_obj_view_update);
   evas_object_event_callback_del(item->view, EVAS_CALLBACK_DEL,
                                  _track_obj_view_del);
}

static void
_elm_widget_item_signal_cb(void *data, Evas_Object *obj EINA_UNUSED, const char *emission,
                           const char *source)
{
   Elm_Widget_Item_Signal_Data *wisd = data;
   wisd->func(wisd->data, wisd->item, emission, source);
}

static void *
_elm_widget_item_signal_callback_list_get(Elm_Widget_Item *item, Eina_List *position)
{
   Elm_Widget_Item_Signal_Data *wisd = eina_list_data_get(position);
   void *data;

   item->signals = eina_list_remove_list(item->signals, position);
   data = wisd->data;

   if (_elm_widget_is(item->view))
     elm_object_signal_callback_del(item->view,
                                    wisd->emission, wisd->source,
                                    _elm_widget_item_signal_cb);
   else if (!strcmp(eo_class_name_get(eo_class_get(item->view)),
                    "edje"))
     edje_object_signal_callback_del_full(item->view,
                                          wisd->emission, wisd->source,
                                          _elm_widget_item_signal_cb, wisd);

   eina_stringshare_del(wisd->emission);
   eina_stringshare_del(wisd->source);
   free(wisd);

   return data;
}

/**
 * @internal
 *
 * Allocate a new Elm_Widget_Item-derived structure.
 *
 * The goal of this structure is to provide common ground for actions
 * that a widget item have, such as the owner widget, callback to
 * notify deletion, data pointer and maybe more.
 *
 * @param widget the owner widget that holds this item, must be an elm_widget!
 * @param alloc_size any number greater than sizeof(Elm_Widget_Item) that will
 *        be used to allocate memory.
 *
 * @return allocated memory that is already zeroed out, or NULL on errors.
 *
 * @see elm_widget_item_new() convenience macro.
 * @see elm_widget_item_del() to release memory.
 * @ingroup Widget
 */
EAPI Elm_Widget_Item *
_elm_widget_item_new(Evas_Object *widget,
                     size_t alloc_size)
{
   if (!_elm_widget_is(widget))
     return NULL;

   Elm_Widget_Item *item;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(alloc_size < sizeof(Elm_Widget_Item), NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!_elm_widget_is(widget), NULL);

   item = calloc(1, alloc_size);
   EINA_SAFETY_ON_NULL_RETURN_VAL(item, NULL);

   EINA_MAGIC_SET(item, ELM_WIDGET_ITEM_MAGIC);
   item->widget = widget;
   return item;
}

EAPI void
_elm_widget_item_free(Elm_Widget_Item *item)
{
   Elm_Translate_String_Data *ts;

   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);

   if (item->del_func)
     item->del_func((void *)item->data, item->widget, item);

   evas_object_del(item->view);

   eina_stringshare_del(item->access_info);

   while (item->signals)
     _elm_widget_item_signal_callback_list_get(item, item->signals);

   while (item->translate_strings)
     {
        ts = EINA_INLIST_CONTAINER_GET(item->translate_strings,
                                       Elm_Translate_String_Data);
        eina_stringshare_del(ts->id);
        eina_stringshare_del(ts->domain);
        eina_stringshare_del(ts->string);
        item->translate_strings = eina_inlist_remove(item->translate_strings,
                                                     item->translate_strings);
        free(ts);
     }
   eina_hash_free(item->labels);

   EINA_MAGIC_SET(item, EINA_MAGIC_NONE);
   free(item);
}

/**
 * @internal
 *
 * Releases widget item memory, calling back del_cb() if it exists.
 *
 * If there is a Elm_Widget_Item::del_cb, then it will be called prior
 * to memory release. Note that elm_widget_item_pre_notify_del() calls
 * this function and then unset it, thus being useful for 2 step
 * cleanup whenever the del_cb may use any of the data that must be
 * deleted from item.
 *
 * The Elm_Widget_Item::view will be deleted (evas_object_del()) if it
 * is presented!
 *
 * @param item a valid #Elm_Widget_Item to be deleted.
 * @see elm_widget_item_del() convenience macro.
 * @ingroup Widget
 */
EAPI void
_elm_widget_item_del(Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   item->on_deletion = EINA_TRUE;

   //Widget item delete callback
   if (item->del_pre_func)
     {
        if (item->del_pre_func((Elm_Object_Item *)item))
          _elm_widget_item_free(item);
     }
   else
     _elm_widget_item_free(item);
}

/**
 * @internal
 *
 * Set the function to notify to widgets when item is being deleted by user.
 *
 * @param item a valid #Elm_Widget_Item to be notified
 * @see elm_widget_item_del_pre_hook_set() convenience macro.
 * @ingroup Widget
 */
EAPI void
_elm_widget_item_del_pre_hook_set(Elm_Widget_Item *item,
                                  Elm_Widget_Del_Pre_Cb func)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   item->del_pre_func = func;
}
/**
 * @internal
 *
 * Set the function to set the style of item
 *
 * @param item a valid #Elm_Widget_Item to be notified
 * @see elm_widget_item_style_set_hook_set() convenience macro.
 * @ingroup Widget
 */
EAPI void
_elm_widget_item_style_set_hook_set(Elm_Widget_Item *item,
                                  Elm_Widget_Style_Set_Cb func)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   item->style_set_func = func;
}

/**
 * @internal
 *
 * Set the function to get the style of item
 *
 * @param item a valid #Elm_Widget_Item to be notified
 * @see elm_widget_item_style_get_hook_set() convenience macro.
 * @ingroup Widget
 */
EAPI void
_elm_widget_item_style_get_hook_set(Elm_Widget_Item *item,
                                  Elm_Widget_Style_Get_Cb func)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   item->style_get_func = func;
}

/**
 * @internal
 *
 * Set the function to set the focus on widget item.
 *
 * @param item a valid #Elm_Widget_Item to be notified
 * @see elm_widget_item_focus_set_hook_set() convenience macro.
 * @ingroup Widget
 */
EAPI void
_elm_widget_item_focus_set_hook_set(Elm_Widget_Item *item, Elm_Widget_Focus_Set_Cb func)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   item->focus_set_func = func;
}

/**
 * @internal
 *
 * Set the function to set the focus on widget item.
 *
 * @param item a valid #Elm_Widget_Item to be notified
 * @see elm_widget_item_focus_get_hook_set() convenience macro.
 * @ingroup Widget
 */
EAPI void
_elm_widget_item_focus_get_hook_set(Elm_Widget_Item *item,
                                  Elm_Widget_Focus_Get_Cb func)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   item->focus_get_func = func;
}

/**
 * @internal
 *
 * Notify object will be deleted without actually deleting it.
 *
 * This function will callback Elm_Widget_Item::del_cb if it is set
 * and then unset it so it is not called twice (ie: from
 * elm_widget_item_del()).
 *
 * @param item a valid #Elm_Widget_Item to be notified
 * @see elm_widget_item_pre_notify_del() convenience macro.
 * @ingroup Widget
 */
EAPI void
_elm_widget_item_pre_notify_del(Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   if (!item->del_func) return;
   item->del_func((void *)item->data, item->widget, item);
   item->del_func = NULL;
}

/**
 * @internal
 *
 * Set the function to notify when item is being deleted.
 *
 * This function will complain if there was a callback set already,
 * however it will set the new one.
 *
 * The callback will be called from elm_widget_item_pre_notify_del()
 * or elm_widget_item_del() will be called with:
 *   - data: the Elm_Widget_Item::data value.
 *   - obj: the Elm_Widget_Item::widget evas object.
 *   - event_info: the item being deleted.
 *
 * @param item a valid #Elm_Widget_Item to be notified
 * @see elm_widget_item_del_cb_set() convenience macro.
 * @ingroup Widget
 */
EAPI void
_elm_widget_item_del_cb_set(Elm_Widget_Item *item,
                            Evas_Smart_Cb func)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   if ((item->del_func) && (item->del_func != func))
     WRN("You're replacing a previously set del_cb %p of item %p with %p",
         item->del_func, item, func);

   item->del_func = func;
}

/**
 * @internal
 *
 * Retrieves owner widget of this item.
 *
 * @param item a valid #Elm_Widget_Item to get data from.
 * @return owner widget of this item.
 * @ingroup Widget
 */
EAPI Evas_Object *
_elm_widget_item_widget_get(const Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item, NULL);

   return item->widget;
}

/**
 * @internal
 *
 * Set user-data in this item.
 *
 * User data may be used to identify this item or just store any
 * application data. It is automatically given as the first parameter
 * of the deletion notify callback.
 *
 * @param item a valid #Elm_Widget_Item to store data in.
 * @param data user data to store.
 * @see elm_widget_item_del_cb_set() convenience macro.
 * @ingroup Widget
 */
EAPI void
_elm_widget_item_data_set(Elm_Widget_Item *item,
                          const void *data)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   if ((item->data) && (item->data != data))
     DBG("Replacing item %p data %p with %p", item, item->data, data);
   item->data = data;
}

/**
 * @internal
 *
 * Retrieves user-data of this item.
 *
 * @param item a valid #Elm_Widget_Item to get data from.
 * @see elm_widget_item_data_set()
 * @ingroup Widget
 */
EAPI void *
_elm_widget_item_data_get(const Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);
   return (void *)item->data;
}

EAPI void
_elm_widget_item_disabled_set(Elm_Widget_Item *item,
                              Eina_Bool disabled)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   if (item->disabled == disabled) return;
   item->disabled = !!disabled;
   if (item->disable_func) item->disable_func(item);
}

EAPI Eina_Bool
_elm_widget_item_disabled_get(const Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, EINA_FALSE);
   return item->disabled;
}

EAPI void
_elm_widget_item_style_set(Elm_Widget_Item *item, const char *style)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   item->style_set_func(item, style);
}

EAPI const char *
_elm_widget_item_style_get(Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);
   return item->style_get_func(item);
}

EAPI void
_elm_widget_item_disable_hook_set(Elm_Widget_Item *item,
                                  Elm_Widget_Disable_Cb func)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   item->disable_func = func;
}

EAPI void
_elm_widget_item_focus_set(Elm_Widget_Item *item, Eina_Bool focused)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   item->focus_set_func(item, focused);
}

EAPI Eina_Bool
_elm_widget_item_focus_get(const Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, EINA_FALSE);
   return item->focus_get_func(item);
}

EAPI void
_elm_widget_item_domain_translatable_part_text_set(Elm_Widget_Item *item,
                                                   const char *part,
                                                   const char *domain,
                                                   const char *label)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   Elm_Translate_String_Data *ts;

   if (!label)
     {
        _part_text_translatable_set(&item->translate_strings, part, EINA_FALSE,
                                    EINA_FALSE);
     }
   else
     {
        ts = _part_text_translatable_set(&item->translate_strings, part,
                                         EINA_TRUE, EINA_FALSE);
        if (!ts) return;
        if (!ts->string) ts->string = eina_stringshare_add(label);
        else eina_stringshare_replace(&ts->string, label);
        if (!ts->domain) ts->domain = eina_stringshare_add(domain);
        else eina_stringshare_replace(&ts->domain, domain);
#ifdef HAVE_GETTEXT
        if (label[0]) label = dgettext(domain, label);
#endif
     }
   item->on_translate = EINA_TRUE;
   _elm_widget_item_part_text_set(item, part, label);
   item->on_translate = EINA_FALSE;
}

EAPI const char *
_elm_widget_item_translatable_part_text_get(const Elm_Widget_Item *item,
                                            const char *part)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item, NULL);

   Elm_Translate_String_Data *ts;
   ts = _translate_string_data_get(item->translate_strings, part);
   if (ts) return ts->string;
   return NULL;
}

EAPI void
_elm_widget_item_domain_part_text_translatable_set(Elm_Widget_Item *item,
                                                   const char *part,
                                                   const char *domain,
                                                   Eina_Bool translatable)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   Elm_Translate_String_Data *ts;
   const char *text;

   ts = _part_text_translatable_set(&item->translate_strings, part,
                                    translatable, EINA_TRUE);
   if (!ts) return;
   if (!ts->domain) ts->domain = eina_stringshare_add(domain);
   else eina_stringshare_replace(&ts->domain, domain);

   text = _elm_widget_item_part_text_get(item, part);
   if (!text || !text[0]) return;

   if (!ts->string) ts->string = eina_stringshare_add(text);

//Try to translate text since we don't know the text is already translated.
#ifdef HAVE_GETTEXT
   text = dgettext(domain, text);
#endif
   item->on_translate = EINA_TRUE;
   _elm_widget_item_part_text_set(item, part, text);
   item->on_translate = EINA_FALSE;
}

EAPI void
_elm_widget_item_track_cancel(Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   if (!item->track_obj) return;

   while (evas_object_ref_get(item->track_obj) > 0)
     evas_object_unref(item->track_obj);

   evas_object_del(item->track_obj);
}

EAPI Evas_Object *
elm_widget_item_track(Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item, NULL);

   if (item->track_obj)
     {
        evas_object_ref(item->track_obj);
        return item->track_obj;
     }

   if (!item->view)
     {
        WRN("view obj of the item(%p) is invalid. Please make sure the view obj is created!", item);
        return NULL;
     }

   Evas_Object *track =
      evas_object_rectangle_add(evas_object_evas_get(item->widget));
   evas_object_color_set(track, 0, 0, 0, 0);
   evas_object_pass_events_set(track, EINA_TRUE);
   _track_obj_update(track, item->view);
   evas_object_event_callback_add(track, EVAS_CALLBACK_DEL, _track_obj_del,
                                  item);

   evas_object_event_callback_add(item->view, EVAS_CALLBACK_RESIZE,
                                  _track_obj_view_update, track);
   evas_object_event_callback_add(item->view, EVAS_CALLBACK_MOVE,
                                  _track_obj_view_update, track);
   evas_object_event_callback_add(item->view, EVAS_CALLBACK_SHOW,
                                  _track_obj_view_update, track);
   evas_object_event_callback_add(item->view, EVAS_CALLBACK_HIDE,
                                  _track_obj_view_update, track);
   evas_object_event_callback_add(item->view, EVAS_CALLBACK_DEL,
                                  _track_obj_view_del, item);

   evas_object_ref(track);

   item->track_obj = track;

   return track;
}

void
elm_widget_item_untrack(Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   if (!item->track_obj) return;
   evas_object_unref(item->track_obj);

   if (evas_object_ref_get(item->track_obj) == 0)
     evas_object_del(item->track_obj);
}

int
elm_widget_item_track_get(const Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, 0);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item, 0);

   if (!item->track_obj) return 0;
   return evas_object_ref_get(item->track_obj);
}

typedef struct _Elm_Widget_Item_Tooltip Elm_Widget_Item_Tooltip;

struct _Elm_Widget_Item_Tooltip
{
   Elm_Widget_Item            *item;
   Elm_Tooltip_Item_Content_Cb func;
   Evas_Smart_Cb               del_cb;
   const void                 *data;
};

static Evas_Object *
_elm_widget_item_tooltip_label_create(void *data,
                                      Evas_Object *obj EINA_UNUSED,
                                      Evas_Object *tooltip,
                                      void *item EINA_UNUSED)
{
   Evas_Object *label = elm_label_add(tooltip);
   if (!label)
     return NULL;
   elm_object_style_set(label, "tooltip");
   elm_object_text_set(label, data);
   return label;
}

static Evas_Object *
_elm_widget_item_tooltip_trans_label_create(void *data,
                                            Evas_Object *obj EINA_UNUSED,
                                            Evas_Object *tooltip,
                                            void *item EINA_UNUSED)
{
   Evas_Object *label = elm_label_add(tooltip);
   if (!label)
     return NULL;
   elm_object_style_set(label, "tooltip");
   elm_object_translatable_text_set(label, data);
   return label;
}

static void
_elm_widget_item_tooltip_label_del_cb(void *data,
                                      Evas_Object *obj EINA_UNUSED,
                                      void *event_info EINA_UNUSED)
{
   eina_stringshare_del(data);
}

/**
 * @internal
 *
 * Set the text to be shown in the widget item.
 *
 * @param item Target item
 * @param text The text to set in the content
 *
 * Setup the text as tooltip to object. The item can have only one tooltip,
 * so any previous tooltip data is removed.
 *
 * @ingroup Widget
 */
EAPI void
_elm_widget_item_tooltip_text_set(Elm_Widget_Item *item,
                                  const char *text)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   EINA_SAFETY_ON_NULL_RETURN(text);

   text = eina_stringshare_add(text);
   _elm_widget_item_tooltip_content_cb_set
     (item, _elm_widget_item_tooltip_label_create, text,
     _elm_widget_item_tooltip_label_del_cb);
}

EAPI void
_elm_widget_item_tooltip_translatable_text_set(Elm_Widget_Item *item,
                                               const char *text)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   EINA_SAFETY_ON_NULL_RETURN(text);

   text = eina_stringshare_add(text);
   _elm_widget_item_tooltip_content_cb_set
     (item, _elm_widget_item_tooltip_trans_label_create, text,
     _elm_widget_item_tooltip_label_del_cb);
}

static Evas_Object *
_elm_widget_item_tooltip_create(void *data,
                                Evas_Object *obj,
                                Evas_Object *tooltip)
{
   Elm_Widget_Item_Tooltip *wit = data;
   return wit->func((void *)wit->data, obj, tooltip, wit->item);
}

static void
_elm_widget_item_tooltip_del_cb(void *data,
                                Evas_Object *obj,
                                void *event_info EINA_UNUSED)
{
   Elm_Widget_Item_Tooltip *wit = data;
   if (wit->del_cb) wit->del_cb((void *)wit->data, obj, wit->item);
   free(wit);
}

/**
 * @internal
 *
 * Set the content to be shown in the tooltip item
 *
 * Setup the tooltip to item. The item can have only one tooltip,
 * so any previous tooltip data is removed. @p func(with @p data) will
 * be called every time that need show the tooltip and it should
 * return a valid Evas_Object. This object is then managed fully by
 * tooltip system and is deleted when the tooltip is gone.
 *
 * @param item the widget item being attached a tooltip.
 * @param func the function used to create the tooltip contents.
 * @param data what to provide to @a func as callback data/context.
 * @param del_cb called when data is not needed anymore, either when
 *        another callback replaces @func, the tooltip is unset with
 *        elm_widget_item_tooltip_unset() or the owner @a item
 *        dies. This callback receives as the first parameter the
 *        given @a data, and @c event_info is the item.
 *
 * @ingroup Widget
 */
EAPI void
_elm_widget_item_tooltip_content_cb_set(Elm_Widget_Item *item,
                                        Elm_Tooltip_Item_Content_Cb func,
                                        const void *data,
                                        Evas_Smart_Cb del_cb)
{
   Elm_Widget_Item_Tooltip *wit;

   ELM_WIDGET_ITEM_CHECK_OR_GOTO(item, error_noitem);
   //ELM_WIDGET_ITEM_RETURN_IF_GOTO(item, error_noitem);

   if (!func)
     {
        _elm_widget_item_tooltip_unset(item);
        return;
     }

   wit = ELM_NEW(Elm_Widget_Item_Tooltip);
   if (!wit) goto error;
   wit->item = item;
   wit->func = func;
   wit->data = data;
   wit->del_cb = del_cb;

   elm_object_sub_tooltip_content_cb_set
     (item->view, item->widget, _elm_widget_item_tooltip_create, wit,
     _elm_widget_item_tooltip_del_cb);

   return;

error_noitem:
   if (del_cb) del_cb((void *)data, NULL, item);
   return;
error:
   if (del_cb) del_cb((void *)data, item->widget, item);
}

/**
 * @internal
 *
 * Unset tooltip from item
 *
 * @param item widget item to remove previously set tooltip.
 *
 * Remove tooltip from item. The callback provided as del_cb to
 * elm_widget_item_tooltip_content_cb_set() will be called to notify
 * it is not used anymore.
 *
 * @see elm_widget_item_tooltip_content_cb_set()
 *
 * @ingroup Widget
 */
EAPI void
_elm_widget_item_tooltip_unset(Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   elm_object_tooltip_unset(item->view);
}

/**
 * @internal
 *
 * Sets a different style for this item tooltip.
 *
 * @note before you set a style you should define a tooltip with
 *       elm_widget_item_tooltip_content_cb_set() or
 *       elm_widget_item_tooltip_text_set()
 *
 * @param item widget item with tooltip already set.
 * @param style the theme style to use (default, transparent, ...)
 *
 * @ingroup Widget
 */
EAPI void
_elm_widget_item_tooltip_style_set(Elm_Widget_Item *item,
                                   const char *style)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   elm_object_tooltip_style_set(item->view, style);
}

EAPI Eina_Bool
_elm_widget_item_tooltip_window_mode_set(Elm_Widget_Item *item,
                                         Eina_Bool disable)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, EINA_FALSE);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item, EINA_FALSE);

   return elm_object_tooltip_window_mode_set(item->view, disable);
}

EAPI Eina_Bool
_elm_widget_item_tooltip_window_mode_get(const Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, EINA_FALSE);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item, EINA_FALSE);

   return elm_object_tooltip_window_mode_get(item->view);
}

/**
 * @internal
 *
 * Get the style for this item tooltip.
 *
 * @param item widget item with tooltip already set.
 * @return style the theme style in use, defaults to "default". If the
 *         object does not have a tooltip set, then NULL is returned.
 *
 * @ingroup Widget
 */
EAPI const char *
_elm_widget_item_tooltip_style_get(const Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);

   return elm_object_tooltip_style_get(item->view);
}

EAPI void
_elm_widget_item_cursor_set(Elm_Widget_Item *item,
                            const char *cursor)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   elm_object_sub_cursor_set(item->view, item->widget, cursor);
}

EAPI const char *
_elm_widget_item_cursor_get(const Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);
   return elm_object_cursor_get(item->view);
}

EAPI void
_elm_widget_item_cursor_unset(Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   elm_object_cursor_unset(item->view);
}

/**
 * @internal
 *
 * Sets a different style for this item cursor.
 *
 * @note before you set a style you should define a cursor with
 *       elm_widget_item_cursor_set()
 *
 * @param item widget item with cursor already set.
 * @param style the theme style to use (default, transparent, ...)
 *
 * @ingroup Widget
 */
EAPI void
_elm_widget_item_cursor_style_set(Elm_Widget_Item *item,
                                  const char *style)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   elm_object_cursor_style_set(item->view, style);
}

/**
 * @internal
 *
 * Get the style for this item cursor.
 *
 * @param item widget item with cursor already set.
 * @return style the theme style in use, defaults to "default". If the
 *         object does not have a cursor set, then NULL is returned.
 *
 * @ingroup Widget
 */
EAPI const char *
_elm_widget_item_cursor_style_get(const Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);
   return elm_object_cursor_style_get(item->view);
}

/**
 * @internal
 *
 * Set if the cursor set should be searched on the theme or should use
 * the provided by the engine, only.
 *
 * @note before you set if should look on theme you should define a cursor
 * with elm_object_cursor_set(). By default it will only look for cursors
 * provided by the engine.
 *
 * @param item widget item with cursor already set.
 * @param engine_only boolean to define it cursors should be looked only
 * between the provided by the engine or searched on widget's theme as well.
 *
 * @ingroup Widget
 */
EAPI void
_elm_widget_item_cursor_engine_only_set(Elm_Widget_Item *item,
                                        Eina_Bool engine_only)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   elm_object_cursor_theme_search_enabled_set(item->view, !engine_only);
}

/**
 * @internal
 *
 * Get the cursor engine only usage for this item cursor.
 *
 * @param item widget item with cursor already set.
 * @return engine_only boolean to define it cursors should be looked only
 * between the provided by the engine or searched on widget's theme as well. If
 *         the object does not have a cursor set, then EINA_FALSE is returned.
 *
 * @ingroup Widget
 */
EAPI Eina_Bool
_elm_widget_item_cursor_engine_only_get(const Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, EINA_FALSE);
   return !elm_object_cursor_theme_search_enabled_get(item->view);
}

EAPI void
_elm_widget_item_part_content_set(Elm_Widget_Item *item,
                                  const char *part,
                                  Evas_Object *content)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   if (!item->content_set_func)
     {
        ERR("%s does not support elm_object_item_part_content_set() API.",
            elm_widget_type_get(item->widget));
        return;
     }
   item->content_set_func((Elm_Object_Item *)item, part, content);
}

EAPI Evas_Object *
_elm_widget_item_part_content_get(const Elm_Widget_Item *item,
                                  const char *part)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item, NULL);
   if (!item->content_get_func)
     {
        ERR("%s does not support elm_object_item_part_content_get() API.",
            elm_widget_type_get(item->widget));
        return NULL;
     }
   return item->content_get_func((Elm_Object_Item *)item, part);
}

EAPI Evas_Object *
_elm_widget_item_part_content_unset(Elm_Widget_Item *item,
                                    const char *part)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item, NULL);
   if (!item->content_unset_func)
     {
        ERR("%s does not support elm_object_item_part_content_unset() API.",
            elm_widget_type_get(item->widget));
        return NULL;
     }
   return item->content_unset_func((Elm_Object_Item *)item, part);
}

EAPI void
_elm_widget_item_part_text_set(Elm_Widget_Item *item,
                               const char *part,
                               const char *label)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   if (!item->text_set_func)
     {
        ERR("%s does not support elm_object_item_part_text_set() API.",
            elm_widget_type_get(item->widget));
        return;
     }
   item->text_set_func((Elm_Object_Item *)item, part, label);
}

EAPI const char *
_elm_widget_item_part_text_get(const Elm_Widget_Item *item,
                               const char *part)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item, NULL);
   if (!item->text_get_func)
     {
        ERR("%s does not support elm_object_item_part_text_get() API.",
            elm_widget_type_get(item->widget));
        return NULL;
     }
   return item->text_get_func((Elm_Object_Item *)item, part);
}

static void
_elm_widget_item_part_text_custom_free(void *data)
{
   Elm_Label_Data *label;
   label = data;
   eina_stringshare_del(label->part);
   eina_stringshare_del(label->text);
   free(label);
}

EAPI void
_elm_widget_item_part_text_custom_set(Elm_Widget_Item *item,
                                      const char *part,
                                      const char *text)
{
   Elm_Label_Data *label;
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   if (!item->text_get_func)
     {
        ERR("%s does not support elm_object_item_part_text_get() API.",
            elm_widget_type_get(item->widget));
        return;
     }
   if (!item->labels)
     item->labels =
        eina_hash_stringshared_new(_elm_widget_item_part_text_custom_free);
   label = eina_hash_find(item->labels, part);
   if (!label)
     {
        label = malloc(sizeof(Elm_Label_Data));
        label->part = eina_stringshare_add(part);
        label->text = eina_stringshare_add(text);
        eina_hash_add(item->labels, part, label);
     }
   else
     eina_stringshare_replace(&label->text, text);
}

EAPI const char *
_elm_widget_item_part_text_custom_get(Elm_Widget_Item *item,
                                      const char *part)
{
   Elm_Label_Data *label;
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);
   label = eina_hash_find(item->labels, part);
   return label ? label->text : NULL;
}

static Eina_Bool
_elm_widget_item_part_text_custom_foreach(const Eina_Hash *labels EINA_UNUSED,
                                          const void *key EINA_UNUSED,
                                          void *data,
                                          void *func_data)
{
   Elm_Label_Data *label;
   Elm_Widget_Item *item;
   label = data;
   item = func_data;
   item->text_set_func((Elm_Object_Item *)item, label->part, label->text);
   return EINA_TRUE;
}

EAPI void
_elm_widget_item_part_text_custom_update(Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   if (item->labels)
     eina_hash_foreach(item->labels,
                       _elm_widget_item_part_text_custom_foreach, item);
}

EAPI void
_elm_widget_item_content_set_hook_set(Elm_Widget_Item *item,
                                      Elm_Widget_Content_Set_Cb func)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   item->content_set_func = func;
}

EAPI void
_elm_widget_item_content_get_hook_set(Elm_Widget_Item *item,
                                      Elm_Widget_Content_Get_Cb func)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   item->content_get_func = func;
}

EAPI void
_elm_widget_item_content_unset_hook_set(Elm_Widget_Item *item,
                                        Elm_Widget_Content_Unset_Cb func)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   item->content_unset_func = func;
}

EAPI void
_elm_widget_item_text_set_hook_set(Elm_Widget_Item *item,
                                   Elm_Widget_Text_Set_Cb func)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   item->text_set_func = func;
}

EAPI void
_elm_widget_item_text_get_hook_set(Elm_Widget_Item *item,
                                   Elm_Widget_Text_Get_Cb func)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   item->text_get_func = func;
}

EAPI void
_elm_widget_item_signal_emit(Elm_Widget_Item *item,
                             const char *emission,
                             const char *source)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   if (item->signal_emit_func)
     item->signal_emit_func((Elm_Object_Item *)item, emission, source);
}

EAPI void
_elm_widget_item_signal_emit_hook_set(Elm_Widget_Item *item,
                                      Elm_Widget_Signal_Emit_Cb func)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   item->signal_emit_func = func;
}

EAPI void
_elm_widget_item_signal_callback_add(Elm_Widget_Item *item,
                                     const char *emission,
                                     const char *source,
                                     Elm_Widget_Item_Signal_Cb func,
                                     void *data)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   EINA_SAFETY_ON_NULL_RETURN(func);

   Elm_Widget_Item_Signal_Data *wisd;

   wisd = malloc(sizeof(Elm_Widget_Item_Signal_Data));
   if (!wisd) return;

   wisd->item = item;
   wisd->func = func;
   wisd->data = data;
   wisd->emission = eina_stringshare_add(emission);
   wisd->source = eina_stringshare_add(source);

   if (_elm_widget_is(item->view))
     elm_object_signal_callback_add(item->view, emission, source, _elm_widget_item_signal_cb, wisd);
   else if (!strcmp(eo_class_name_get(eo_class_get(item->view)), "edje"))
     edje_object_signal_callback_add(item->view, emission, source, _elm_widget_item_signal_cb, wisd);
   else
     {
        WRN("The %s widget item doesn't support signal callback add!",
            eo_class_name_get(eo_class_get(item->widget)));
        free(wisd);
        return;
     }

   item->signals = eina_list_append(item->signals, wisd);
}

EAPI void *
_elm_widget_item_signal_callback_del(Elm_Widget_Item *item,
                                    const char *emission,
                                    const char *source,
                                    Elm_Widget_Item_Signal_Cb func)
{
   Elm_Widget_Item_Signal_Data *wisd;
   Eina_List *l;

   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(func, NULL);

   EINA_LIST_FOREACH(item->signals, l, wisd)
     {
        if ((wisd->func == func) &&
            !strcmp(wisd->emission, emission) &&
            !strcmp(wisd->source, source))
          return _elm_widget_item_signal_callback_list_get(item, l);
     }

   return NULL;
}

EAPI void
_elm_widget_item_access_info_set(Elm_Widget_Item *item,
                                 const char *txt)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   eina_stringshare_del(item->access_info);
   if (!txt) item->access_info = NULL;
   else item->access_info = eina_stringshare_add(txt);
}

EAPI void
_elm_widget_item_translate(Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

#ifdef HAVE_GETTEXT
   Elm_Translate_String_Data *ts;
   EINA_INLIST_FOREACH(item->translate_strings, ts)
     {
        if (!ts->string) continue;
        const char *s = dgettext(ts->domain, ts->string);
        item->on_translate = EINA_TRUE;
        _elm_widget_item_part_text_set(item, ts->id, s);
        item->on_translate = EINA_FALSE;
     }
#endif
}

/* happy debug functions */
#ifdef ELM_DEBUG
static void
_sub_obj_tree_dump(const Evas_Object *obj,
                   int lvl)
{
   int i;

   for (i = 0; i < lvl * 3; i++)
     putchar(' ');

   if (_elm_widget_is(obj))
     {
        Eina_List *l;
        INTERNAL_ENTRY;
        DBG("+ %s(%p)\n",
            elm_widget_type_get(obj),
            obj);
        EINA_LIST_FOREACH(sd->subobjs, l, obj)
          _sub_obj_tree_dump(obj, lvl + 1);
     }
   else
     DBG("+ %s(%p)\n", evas_object_type_get(obj), obj);
}

static void
_sub_obj_tree_dot_dump(const Evas_Object *obj,
                       FILE *output)
{
   if (!_elm_widget_is(obj))
     return;
   INTERNAL_ENTRY;

   Eina_Bool visible = evas_object_visible_get(obj);
   Eina_Bool disabled = elm_widget_disabled_get(obj);
   Eina_Bool focused = elm_widget_focus_get(obj);
   Eina_Bool can_focus = elm_widget_can_focus_get(obj);

   if (sd->parent_obj)
     {
        fprintf(output, "\"%p\" -- \"%p\" [ color=black", sd->parent_obj, obj);

        if (focused)
          fprintf(output, ", style=bold");

        if (!visible)
          fprintf(output, ", color=gray28");

        fprintf(output, " ];\n");
     }

   fprintf(output, "\"%p\" [ label = \"{%p|%s|%s|visible: %d|"
                   "disabled: %d|focused: %d/%d|focus order:%d}\"",
           obj, obj, elm_widget_type_get(obj),
           evas_object_name_get(obj), visible, disabled, focused, can_focus,
           sd->focus_order);

   if (focused)
     fprintf(output, ", style=bold");

   if (!visible)
     fprintf(output, ", fontcolor=gray28");

   if ((disabled) || (!visible))
     fprintf(output, ", color=gray");

   fprintf(output, " ];\n");

   Eina_List *l;
   Evas_Object *o;
   EINA_LIST_FOREACH(sd->subobjs, l, o)
     _sub_obj_tree_dot_dump(o, output);
}

#endif

EAPI void
elm_widget_tree_dump(const Evas_Object *top)
{
#ifdef ELM_DEBUG
   if (!_elm_widget_is(top))
     return;
   _sub_obj_tree_dump(top, 0);
#else
   (void)top;
   return;
#endif
}

EAPI void
elm_widget_tree_dot_dump(const Evas_Object *top,
                         FILE *output)
{
#ifdef ELM_DEBUG
   if (!_elm_widget_is(top))
     return;
   fprintf(output, "graph " " { node [shape=record];\n");
   _sub_obj_tree_dot_dump(top, output);
   fprintf(output, "}\n");
#else
   (void)top;
   (void)output;
   return;
#endif
}

static Eina_Bool
_atspi_obj_create(void *data)
{
   Elm_Atspi_Object *parent = NULL;
   Elm_Atspi_Object *obj = _elm_atspi_factory_construct(data);
   if (obj)
     {
       eo_do(obj, elm_atspi_obj_parent_get(&parent));
       eo_do(parent, eo_event_callback_call(EV_ATSPI_OBJ_CHILD_ADD, obj, NULL));
     }
   return EINA_FALSE;
}

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Eo *parent;
   ELM_WIDGET_DATA_GET(obj, sd);

   sd->on_create = EINA_TRUE;
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         eo_parent_get(&parent));
   eo_do(obj, elm_wdg_parent_set(parent));
   sd->on_create = EINA_FALSE;

   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     ecore_idle_enterer_add(_atspi_obj_create, obj);
}

static void
_elm_widget_on_focus(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   ELM_WIDGET_DATA_GET(obj, sd);

   if (elm_widget_can_focus_get(obj))
     {
        if (elm_widget_focus_get(obj))
          {
             if (!sd->resize_obj)
               evas_object_focus_set(obj, EINA_TRUE);
             evas_object_smart_callback_call(obj, SIG_WIDGET_FOCUSED, NULL);
          }
        else
          {
             if (!sd->resize_obj)
               evas_object_focus_set(obj, EINA_FALSE);
             evas_object_smart_callback_call(obj, SIG_WIDGET_UNFOCUSED, NULL);
          }
     }
   else
     return;

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_widget_disable(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
}

static void
_elm_widget_event(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   va_arg(*list, Evas_Object *);
   va_arg(*list, Evas_Callback_Type);
   va_arg(*list, void *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
}

static void
_elm_widget_focus_next_manager_is_unimplemented(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
   WRN("The %s widget does not implement the \"focus_next/focus_next_manager_is\" functions.",
       eo_class_name_get(eo_class_get(obj)));
}

static void
_elm_widget_focus_direction_manager_is_unimplemented(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
   WRN("The %s widget does not implement the \"focus_direction/focus_direction_manager_is\" functions.",
       eo_class_name_get(eo_class_get(obj)));
}

static void
_elm_widget_activate(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   va_arg(*list, Elm_Activate);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
   WRN("The %s widget does not implement the \"activate\" functions.",
       eo_class_name_get(eo_class_get(obj)));
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DBG_INFO_GET), _dbg_info_get),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MEMBER_ADD), _elm_widget_smart_member_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MEMBER_DEL), _elm_widget_smart_member_del),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_widget_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_widget_smart_del),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_RESIZE), _elm_widget_smart_resize),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MOVE), _elm_widget_smart_move),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_SHOW), _elm_widget_smart_show),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_HIDE), _elm_widget_smart_hide),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_COLOR_SET), _elm_widget_smart_color_set),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CLIP_SET), _elm_widget_smart_clip_set),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CLIP_UNSET), _elm_widget_smart_clip_unset),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALCULATE), _elm_widget_smart_calculate),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_ON_FOCUS), _elm_widget_on_focus),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_DISABLE), _elm_widget_disable),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_THEME_APPLY), _elm_widget_theme_func),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_TRANSLATE), _elm_widget_translate),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_EVENT), _elm_widget_event),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_ON_FOCUS_REGION), _elm_widget_on_focus_region),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_widget_focus_next_manager_is_unimplemented),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_widget_focus_direction_manager_is_unimplemented),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_SUB_OBJECT_ADD), _elm_widget_sub_object_add),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_SUB_OBJECT_DEL), _elm_widget_sub_object_del),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_ACCESS), _elm_widget_access),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_PARENT_SET), _elm_widget_parent_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_PARENT_GET), _elm_widget_parent_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_PARENT2_SET), _elm_widget_parent2_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_PARENT2_GET), _elm_widget_parent2_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_ACTIVATE), _elm_widget_activate),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_MIRRORED_GET), _elm_widget_mirrored_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_MIRRORED_SET), _elm_widget_mirrored_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_MIRRORED_AUTOMATIC_GET), _elm_widget_mirrored_automatic_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_MIRRORED_AUTOMATIC_SET), _elm_widget_mirrored_automatic_set),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_HIGHLIGHT_IGNORE_SET), _elm_widget_highlight_ignore_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_HIGHLIGHT_IGNORE_GET), _elm_widget_highlight_ignore_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_HIGHLIGHT_IN_THEME_SET), _elm_widget_highlight_in_theme_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_HIGHLIGHT_IN_THEME_GET), _elm_widget_highlight_in_theme_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_ACCESS_HIGHLIGHT_IN_THEME_SET), _elm_widget_access_highlight_in_theme_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_ACCESS_HIGHLIGHT_IN_THEME_GET), _elm_widget_access_highlight_in_theme_get),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_SCROLL_HOLD_PUSH), _elm_widget_scroll_hold_push),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_SCROLL_HOLD_POP), _elm_widget_scroll_hold_pop),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_SCROLL_HOLD_GET), _elm_widget_scroll_hold_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_SCROLL_FREEZE_PUSH), _elm_widget_scroll_freeze_push),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_SCROLL_FREEZE_POP), _elm_widget_scroll_freeze_pop),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_SCROLL_FREEZE_GET), _elm_widget_scroll_freeze_get),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_ON_SHOW_REGION_HOOK_SET), _elm_widget_on_show_region_hook_set),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_RESIZE_OBJECT_SET), _elm_widget_resize_object_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_HOVER_OBJECT_SET), _elm_widget_hover_object_set),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_CAN_FOCUS_SET), _elm_widget_can_focus_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_CAN_FOCUS_GET), _elm_widget_can_focus_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_CHILD_CAN_FOCUS_GET), _elm_widget_child_can_focus_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_GET), _elm_widget_focus_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_HIGHLIGHT_GET), _elm_widget_highlight_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUSED_OBJECT_GET), _elm_widget_focused_object_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_TOP_GET), _elm_widget_top_get),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_SET), _elm_widget_focus_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUSED_OBJECT_CLEAR), _elm_widget_focused_object_clear),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_STEAL), _elm_widget_focus_steal),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_RESTORE), _elm_widget_focus_restore),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_DISABLED_SET), _elm_widget_disabled_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_DISABLED_GET), _elm_widget_disabled_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_SHOW_REGION_SET), _elm_widget_show_region_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_SHOW_REGION_GET), _elm_widget_show_region_get),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_PARENTS_BOUNCE_GET), _elm_widget_parents_bounce_get),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_SCROLLABLE_CHILDREN_GET), _elm_widget_scrollable_children_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_SCALE_SET), _elm_widget_scale_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_SCALE_GET), _elm_widget_scale_get),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_PART_TEXT_SET), _elm_widget_part_text_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_PART_TEXT_GET), _elm_widget_part_text_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_DOMAIN_TRANSLATABLE_PART_TEXT_SET), _elm_widget_domain_translatable_part_text_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_TRANSLATABLE_PART_TEXT_GET), _elm_widget_translatable_part_text_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_DOMAIN_PART_TEXT_TRANSLATABLE_SET), _elm_widget_domain_part_text_translatable_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_PART_TEXT_TRANSLATE), _elm_widget_part_text_translate),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_ACCESS_INFO_SET), _elm_widget_access_info_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_ACCESS_INFO_GET), _elm_widget_access_info_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_THEME_SET), _elm_widget_theme_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_THEME_GET), _elm_widget_theme_get),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_STYLE_SET), _elm_widget_style_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_STYLE_GET), _elm_widget_style_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_TOOLTIP_ADD), _elm_widget_tooltip_add),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_TOOLTIP_DEL), _elm_widget_tooltip_del),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_CURSOR_ADD), _elm_widget_cursor_add),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_CURSOR_DEL), _elm_widget_cursor_del),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_DRAG_LOCK_X_SET), _elm_widget_drag_lock_x_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_DRAG_LOCK_Y_SET), _elm_widget_drag_lock_y_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_DRAG_LOCK_X_GET), _elm_widget_drag_lock_x_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_DRAG_LOCK_Y_GET), _elm_widget_drag_lock_y_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_DRAG_CHILD_LOCKED_X_GET), _elm_widget_drag_child_locked_x_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_DRAG_CHILD_LOCKED_Y_GET), _elm_widget_drag_child_locked_y_get),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_EVENT_CALLBACK_ADD), _elm_widget_event_callback_add),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_EVENT_CALLBACK_DEL), _elm_widget_event_callback_del),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_EVENT_PROPAGATE), _elm_widget_event_propagate),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_SIGNAL_EMIT), _elm_widget_signal_emit),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_SIGNAL_CALLBACK_ADD), _elm_widget_signal_callback_add),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_SIGNAL_CALLBACK_DEL), _elm_widget_signal_callback_del),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_NAME_FIND), _elm_widget_name_find),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_HIDE_HANDLE), _elm_widget_focus_hide_handle),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_MOUSE_UP_HANDLE), _elm_widget_focus_mouse_up_handle),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_TREE_UNFOCUSABLE_HANDLE), _elm_widget_focus_tree_unfocusable_handle),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_DISABLED_HANDLE), _elm_widget_focus_disabled_handle),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_ORDER_GET), _elm_widget_focus_order_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_REGION_GET), _elm_widget_focus_region_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_REGION_SHOW), _elm_widget_focus_region_show),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_HIGHLIGHT_STYLE_SET), _elm_widget_focus_highlight_style_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_HIGHLIGHT_STYLE_GET), _elm_widget_focus_highlight_style_get),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_THEME_OBJECT_SET), _elm_widget_theme_object_set),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_ORIENTATION_SET), _elm_widget_orientation_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_ORIENTATION_MODE_DISABLED_SET), _elm_widget_orientation_mode_disabled_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_ORIENTATION_MODE_DISABLED_GET), _elm_widget_orientation_mode_disabled_get),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_CUSTOM_CHAIN_SET), _elm_widget_focus_custom_chain_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_CUSTOM_CHAIN_GET), _elm_widget_focus_custom_chain_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_CUSTOM_CHAIN_UNSET), _elm_widget_focus_custom_chain_unset),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_CUSTOM_CHAIN_APPEND), _elm_widget_focus_custom_chain_append),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_CUSTOM_CHAIN_PREPEND), _elm_widget_focus_custom_chain_prepend),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_CYCLE), _elm_widget_focus_cycle),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_DIRECTION_GO), _elm_widget_focus_direction_go),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_DIRECTION_GET), _elm_widget_focus_direction_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_LIST_DIRECTION_GET), _elm_widget_focus_list_direction_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_NEXT_GET), _elm_widget_focus_next_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_LIST_NEXT_GET), _elm_widget_focus_list_next_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_NEXT_OBJECT_GET), _elm_widget_focus_next_object_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_NEXT_OBJECT_SET), _elm_widget_focus_next_object_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_PARENT_HIGHLIGHT_SET), _elm_widget_parent_highlight_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_DISPLAY_MODE_SET), _elm_widget_display_mode_set),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_TREE_UNFOCUSABLE_SET), _elm_widget_tree_unfocusable_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_TREE_UNFOCUSABLE_GET), _elm_widget_tree_unfocusable_get),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_CAN_FOCUS_CHILD_LIST_GET), _elm_widget_can_focus_child_list_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_NEWEST_FOCUS_ORDER_GET), _elm_widget_newest_focus_order_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_HIGHLIGHT_GEOMETRY_GET), _elm_widget_focus_highlight_geometry_get),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUSED_ITEM_GET), _elm_widget_focused_item_get),

        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_ON_FOCUS, "'Virtual' function handling focus in/out events on the widget."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_DISABLE, "'Virtual' function on the widget being disabled."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_THEME_APPLY, "'Virtual' function on the widget being re-themed."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_TRANSLATE, "'Virtual' function handling language changes on Elementary."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_EVENT, "'Virtual' function handling input events on the widget."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_ON_FOCUS_REGION, "'Virtual' function returning an inner area of a widget that should be brought into the visible are of a broader viewport, may this context arise."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS, "'Virtual' function which checks if handling of passing focus to sub-objects is supported by widget."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_NEXT, "'Virtual' function handling passing focus to sub-objects."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS, "'Virtual' function which checks if handling of passing focus to sub-objects in given direction is supported by widget."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_DIRECTION,"'Virtual' function handling passing focus to sub-objects given a direction, in degrees."),
	 EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_SUB_OBJECT_ADD, "'Virtual' function handling sub objects being added."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_SUB_OBJECT_DEL, "'Virtual' function handling sub objects being removed."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_ACCESS, "'Virtual' function on the widget being set access."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_PARENT_SET, "'Virtual' function handling parent widget attachment to new object."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_PARENT_GET, "'Virtual' function handling getting object's parent widget."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_PARENT2_SET, ""),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_PARENT2_GET, ""),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_ACTIVATE, "'Virtual' function to activate widget."),

     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_MIRRORED_GET, "Returns the widget's mirrored mode."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_MIRRORED_SET, "Sets the widget's mirrored mode."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_MIRRORED_AUTOMATIC_GET, "Returns the widget's mirrored mode setting."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_MIRRORED_AUTOMATIC_SET, "Sets the widget's mirrored mode setting."),

     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_HIGHLIGHT_IGNORE_SET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_HIGHLIGHT_IGNORE_GET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_HIGHLIGHT_IN_THEME_SET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_HIGHLIGHT_IN_THEME_GET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_ACCESS_HIGHLIGHT_IN_THEME_SET, "Set the access highlight in widget theme."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_ACCESS_HIGHLIGHT_IN_THEME_GET, "Get the access highlight in widget theme."),

     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_SCROLL_HOLD_PUSH, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_SCROLL_HOLD_POP, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_SCROLL_HOLD_GET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_SCROLL_FREEZE_PUSH, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_SCROLL_FREEZE_POP, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_SCROLL_FREEZE_GET, "description here"),

     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_ON_SHOW_REGION_HOOK_SET, "description here"),

     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_RESIZE_OBJECT_SET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_HOVER_OBJECT_SET, "description here"),

     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_CAN_FOCUS_SET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_CAN_FOCUS_GET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_CHILD_CAN_FOCUS_GET, "description here"),

     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_GET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_HIGHLIGHT_GET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUSED_OBJECT_GET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_TOP_GET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_PARENT_WIDGET_GET, "description here"),

     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_SET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUSED_OBJECT_CLEAR, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_STEAL, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_RESTORE, "description here"),

     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_DISABLED_SET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_DISABLED_GET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_SHOW_REGION_SET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_SHOW_REGION_GET, "description here"),

     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_PARENTS_BOUNCE_GET, "Get the whether parents have a bounce."),

     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_SCROLLABLE_CHILDREN_GET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_SCALE_SET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_SCALE_GET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_PART_TEXT_SET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_PART_TEXT_GET, "description here"),

     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_DOMAIN_TRANSLATABLE_PART_TEXT_SET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_TRANSLATABLE_PART_TEXT_GET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_DOMAIN_PART_TEXT_TRANSLATABLE_SET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_PART_TEXT_TRANSLATE, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_ACCESS_INFO_SET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_ACCESS_INFO_GET, "description here"),

     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_THEME_SET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_THEME_GET, "description here"),

     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_STYLE_SET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_STYLE_GET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_TOOLTIP_ADD, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_TOOLTIP_DEL, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_CURSOR_ADD, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_CURSOR_DEL, "description here"),

     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_DRAG_LOCK_X_SET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_DRAG_LOCK_Y_SET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_DRAG_LOCK_X_GET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_DRAG_LOCK_Y_GET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_DRAG_CHILD_LOCKED_X_GET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_DRAG_CHILD_LOCKED_Y_GET, "description here"),

     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_EVENT_CALLBACK_ADD, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_EVENT_CALLBACK_DEL, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_EVENT_PROPAGATE, "description here"),

     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_SIGNAL_EMIT, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_SIGNAL_CALLBACK_ADD, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_SIGNAL_CALLBACK_DEL, "description here"),

     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_NAME_FIND, "description here"),

     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_HIDE_HANDLE, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_MOUSE_UP_HANDLE, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_TREE_UNFOCUSABLE_HANDLE, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_DISABLED_HANDLE, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_ORDER_GET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_REGION_GET, "Get the focus region of the given widget."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_REGION_SHOW, "Show the focus region of the given widget."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_HIGHLIGHT_STYLE_SET, "Function to set the focus highlight style."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_HIGHLIGHT_STYLE_GET, "Function to get the focus highlight style."),

     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_THEME_OBJECT_SET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_ORIENTATION_SET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_ORIENTATION_MODE_DISABLED_SET, "description here"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_ORIENTATION_MODE_DISABLED_GET, "description here"),

     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_CUSTOM_CHAIN_SET, "Set custom focus chain."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_CUSTOM_CHAIN_GET, "Get custom focus chain."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_CUSTOM_CHAIN_UNSET, "Unset custom focus chain."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_CUSTOM_CHAIN_APPEND, "Append object to custom focus chain."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_CUSTOM_CHAIN_PREPEND, "Prepend object to custom focus chain."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_CYCLE, "Give focus to next object in object tree."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_DIRECTION_GO, "Give focus to near object(in object tree) in one direction."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_DIRECTION_GET, "Get near object in one direction of base object."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_LIST_DIRECTION_GET, "Get near object in one direction of base object in list."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_NEXT_GET, "Get next object in focus chain of object tree."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_LIST_NEXT_GET, "Get next object in focus chain of object tree in list."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_NEXT_OBJECT_GET, "Get next object specified by focus direction."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_NEXT_OBJECT_SET, "Set next object with specific focus direction."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_PARENT_HIGHLIGHT_SET, "Set highlighted value from itself to top parent object."),

     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_DISPLAY_MODE_SET, "Sets the widget and child widget's Evas_Display_Mode."),

     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_TREE_UNFOCUSABLE_SET, "Sets the widget object and its children to be unfocusable"),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_TREE_UNFOCUSABLE_GET, "Returns true, if the object sub-tree is unfocusable"),

     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_CAN_FOCUS_CHILD_LIST_GET, "Get the list of focusable child objects."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_NEWEST_FOCUS_ORDER_GET, "Get the newest focused object and its order."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUS_HIGHLIGHT_GEOMETRY_GET, "Get the focus highlight geometry of widget."),
     EO_OP_DESCRIPTION(ELM_WIDGET_SUB_ID_FOCUSED_ITEM_GET, "Get the focused widget item."),

     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR_NO_INSTANT,
     EO_CLASS_DESCRIPTION_OPS(&ELM_WIDGET_BASE_ID, op_desc, ELM_WIDGET_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Widget_Smart_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_widget_class_get, &class_desc, EVAS_OBJ_SMART_CLASS, NULL);
