#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_INTERFACE_ATSPI_COMPONENT_PROTECTED
#define ELM_WIDGET_PROTECTED
#define ELM_WIDGET_ITEM_PROTECTED
#define EFL_CANVAS_OBJECT_BETA
#define EFL_INPUT_EVENT_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_container.h"
#include "elm_interface_scrollable.h"
#include "elm_part_helper.h"

/* FIXME: remove this when we don't rely on evas event structs anymore */
#define EFL_INTERNAL_UNSTABLE
#include "interfaces/efl_common_internal.h"

#define MY_CLASS ELM_WIDGET_CLASS

#define MY_CLASS_NAME "Elm_Widget"
#define MY_CLASS_NAME_LEGACY "elm_widget"

#define ELM_WIDGET_DATA_GET(o, wd)                             \
  Elm_Widget_Smart_Data *wd = efl_data_scope_get(o, MY_CLASS)

#define API_ENTRY                                    \
  Elm_Widget_Smart_Data *sd = NULL;                  \
  if (!_elm_widget_is(obj) ||                        \
      (!(sd = efl_data_scope_get(obj, MY_CLASS))))

#define INTERNAL_ENTRY                               \
  ELM_WIDGET_DATA_GET(obj, sd);                      \
  if (!sd) return

#define ELM_WIDGET_FOCUS_GET(obj)                                          \
  (efl_isa(obj, ELM_WIDGET_CLASS) &&                                    \
   ((_elm_access_auto_highlight_get()) ? (elm_widget_highlight_get(obj)) : \
                                         (elm_widget_focus_get(obj))))

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
   return efl_isa(obj, MY_CLASS);
}

static inline Eina_Bool
_is_focusable(Evas_Object *obj)
{
   API_ENTRY return EINA_FALSE;
   return sd->can_focus || (sd->child_can_focus);
}

static inline Eina_Bool
_is_focused(Evas_Object *obj)
{
   API_ENTRY return EINA_FALSE;
   return sd->focused;
}

static inline Eina_Bool
_elm_scrollable_is(const Evas_Object *obj)
{
   INTERNAL_ENTRY EINA_FALSE;
   return
      efl_isa(obj, ELM_INTERFACE_SCROLLABLE_MIXIN);
}

static void
elm_widget_disabled_internal(Eo *obj, Eina_Bool disabled);
static void
_on_sub_obj_del(void *data, const Efl_Event *event);
static void
_on_sub_obj_hide(void *data, const Efl_Event *event);
static void
_propagate_event(void *data, const Efl_Event *event);

EFL_CALLBACKS_ARRAY_DEFINE(elm_widget_subitems_callbacks,
                          { EFL_EVENT_DEL, _on_sub_obj_del },
                          { EFL_GFX_EVENT_HIDE, _on_sub_obj_hide });
EFL_CALLBACKS_ARRAY_DEFINE(efl_subitems_callbacks,
                          { EFL_EVENT_DEL, _on_sub_obj_del });
EFL_CALLBACKS_ARRAY_DEFINE(focus_callbacks,
                          { EFL_EVENT_KEY_DOWN, _propagate_event },
                          { EFL_EVENT_KEY_UP, _propagate_event },
                          { EFL_EVENT_POINTER_WHEEL, _propagate_event });

static inline void
_callbacks_add(Eo *widget, void *data)
{
   if (_elm_widget_is(widget))
     {
        efl_event_callback_array_add(widget, elm_widget_subitems_callbacks(), data);
     }
   else
     {
        efl_event_callback_array_add(widget, efl_subitems_callbacks(), data);
     }
}

static inline void
_callbacks_del(Eo *widget, void *data)
{
   if (_elm_widget_is(widget))
     {
        efl_event_callback_array_del(widget, elm_widget_subitems_callbacks(), data);
     }
   else
     {
        efl_event_callback_array_del(widget, efl_subitems_callbacks(), data);
     }
}

void
_elm_widget_item_highlight_in_theme(Evas_Object *obj, Elm_Object_Item *eo_it)
{
   const char *str;

   if (!eo_it) return;
   if (efl_isa(eo_it, ELM_WIDGET_ITEM_CLASS))
     {
        Elm_Widget_Item_Data *it = efl_data_scope_get(eo_it, ELM_WIDGET_ITEM_CLASS);

        if (efl_isa(it->view, ELM_LAYOUT_CLASS))
          str = edje_object_data_get(elm_layout_edje_get(it->view), "focus_highlight");
        else
          str = edje_object_data_get(it->view, "focus_highlight");
     }
   else
      str = edje_object_data_get(((Elm_Widget_Item_Data *)eo_it)->view, "focus_highlight");
   if ((str) && (!strcmp(str, "on")))
     elm_widget_highlight_in_theme_set(obj, EINA_TRUE);
   else
     elm_widget_highlight_in_theme_set(obj, EINA_FALSE);
}

void
_elm_widget_focus_highlight_start(const Evas_Object *obj)
{
   Evas_Object *top = elm_widget_top_get(obj);

   if (top && efl_isa(top, EFL_UI_WIN_CLASS))
     _elm_win_focus_highlight_start(top);
}

Evas_Object *
_elm_widget_focus_highlight_object_get(const Evas_Object *obj)
{
   Evas_Object *top = elm_widget_top_get(obj);

   if (top && efl_isa(top, EFL_UI_WIN_CLASS))
     return _elm_win_focus_highlight_object_get(top);
   return NULL;
}

EAPI Eina_Bool
elm_widget_focus_highlight_enabled_get(const Evas_Object *obj)
{
   const Evas_Object *win = elm_widget_top_get(obj);

   if (win && efl_isa(win, EFL_UI_WIN_CLASS))
     return elm_win_focus_highlight_enabled_get(win);
   return EINA_FALSE;
}

static Eina_Bool
_tree_unfocusable(Eo *obj)
{
   Elm_Widget *wid = obj;

   do {
     ELM_WIDGET_DATA_GET(wid, wid_pd);

     if (wid_pd->tree_unfocusable) return EINA_TRUE;
   } while((wid = elm_widget_parent_widget_get(wid)));

   return EINA_FALSE;
}

static Eina_Bool
_tree_disabled(Eo *obj)
{
   Elm_Widget *wid = obj;

   do {
     ELM_WIDGET_DATA_GET(wid, wid_pd);

     if (wid_pd->disabled) return EINA_TRUE;
   } while((wid = elm_widget_parent_widget_get(wid)));

   return EINA_FALSE;
}

static void _full_eval(Eo *obj, Elm_Widget_Smart_Data *pd);

static void
_manager_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET(data, pd);

   _full_eval(data, pd);
}

static Efl_Ui_Focus_Object*
_focus_manager_eval(Eo *obj, Elm_Widget_Smart_Data *pd)
{
   Evas_Object *provider = NULL;
   Evas_Object *parent;
   Efl_Ui_Focus_Manager *new = NULL, *old = NULL;

   parent = elm_widget_parent_get(obj);
   if (efl_isa(parent, EFL_UI_FOCUS_MANAGER_CLASS))
     {
        new = parent;
     }
   else
     {
        new = efl_ui_focus_user_manager_get(parent);
        provider = parent;
     }

   if (new != pd->manager.manager )
     {
        old = pd->manager.manager;

        if (pd->manager.provider)
          efl_event_callback_del(pd->manager.provider, EFL_UI_FOCUS_USER_EVENT_MANAGER_CHANGED, _manager_changed_cb, obj);

        pd->manager.manager = new;
        pd->manager.provider = provider;

        if (pd->manager.provider)
          efl_event_callback_add(pd->manager.provider, EFL_UI_FOCUS_USER_EVENT_MANAGER_CHANGED, _manager_changed_cb, obj);
     }

   return old;
}

EOLIAN static Eina_Bool
_elm_widget_focus_register(Eo *obj, Elm_Widget_Smart_Data *pd EINA_UNUSED,
  Efl_Ui_Focus_Manager *manager,
  Efl_Ui_Focus_Object *logical, Eina_Bool full)
{

   if (full)
     efl_ui_focus_manager_register(manager, obj, logical, NULL);
   else
     efl_ui_focus_manager_register_logical(manager, obj, logical, NULL);

   return full;
}


static void
_focus_state_eval(Eo *obj, Elm_Widget_Smart_Data *pd)
{
   Eina_Bool should = EINA_FALSE;
   Eina_Bool want_full = EINA_FALSE;
   Efl_Ui_Focus_Manager *manager = efl_ui_focus_user_manager_get(obj);

   //there are two reasons to be registered, the child count is bigger than 0, or the widget is flagged to be able to handle focus
   if (pd->can_focus)
     {
        should = EINA_TRUE;
        //can focus can be overridden by the following properties

        if (_tree_unfocusable(obj))
          should = EINA_FALSE;

        if (_tree_disabled(obj))
          should = EINA_FALSE;

        if (!evas_object_visible_get(obj))
          should = EINA_FALSE;

        if (should)
          want_full = EINA_TRUE;
     }

   if (!should && pd->logical.child_count > 0)
     should = EINA_TRUE;

   if ( //check if we have changed the manager
        (pd->focus.manager != manager && should) ||
        //check if we are already registered but in a different state
        (pd->focus.manager && should && want_full == pd->focus.logical)
      )
     {
        efl_ui_focus_manager_unregister(pd->focus.manager, obj);
        pd->focus.manager = NULL;
     }

   //now register in the manager
   if (should && !pd->focus.manager)
     {
        if (manager != obj)
          {
             pd->focus.manager = manager;

             if (!pd->logical.parent) return;

             pd->focus.logical =
                !elm_obj_widget_focus_register(obj, pd->focus.manager, pd->logical.parent, want_full);
          }
     }
   else if (!should && pd->focus.manager)
     {
        efl_ui_focus_manager_unregister(pd->focus.manager, obj);
        pd->focus.manager = NULL;
     }
}

static Efl_Ui_Focus_Object*
_logical_parent_eval(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *pd)
{
   Elm_Widget *parent;
   parent = pd->parent_obj;
   if (pd->logical.parent != parent)
     {
        Efl_Ui_Focus_Object *old = NULL;

        //update old logical parent;
        if (pd->logical.parent)
          {
             ELM_WIDGET_DATA_GET(pd->logical.parent, logical_wd);
             if (!logical_wd)
               {
                  ERR("Widget parent has the wrong type!");
                  return NULL;
               }
             logical_wd->logical.child_count --;
             old = pd->logical.parent;
             if (pd->logical.parent)
               efl_weak_unref(&pd->logical.parent);
             pd->logical.parent = NULL;
          }
        if (parent)
          {
             ELM_WIDGET_DATA_GET(parent, logical_wd);
             if (!logical_wd)
               {
                  ERR("Widget parent has the wrong type!");
                  return NULL;
               }
             logical_wd->logical.child_count ++;
             pd->logical.parent = parent;
             efl_weak_ref(&pd->logical.parent);
          }
        return old;
     }
   return NULL;
}

static void
_full_eval(Eo *obj, Elm_Widget_Smart_Data *pd)
{
   Efl_Ui_Focus_Object *old_parent, *old_manager;

   old_parent = _logical_parent_eval(obj, pd);
   old_manager = _focus_manager_eval(obj, pd);

   if (old_parent)
     {
        //emit signal and focus eval old and new
        ELM_WIDGET_DATA_GET(old_parent, old_pd);
        _focus_state_eval(old_parent, old_pd);

     }

   if (pd->logical.parent)
     {
        ELM_WIDGET_DATA_GET(pd->logical.parent, new_pd);
        _focus_state_eval(pd->logical.parent, new_pd);
     }

   if (old_parent != pd->logical.parent)
     {
        efl_event_callback_call(obj,
             EFL_UI_FOCUS_USER_EVENT_LOGICAL_CHANGED, old_parent);
     }

   if (old_manager != pd->manager.manager)
     {
        //emit signal
        efl_event_callback_call(obj,
             EFL_UI_FOCUS_USER_EVENT_MANAGER_CHANGED, old_manager);
     }

   _focus_state_eval(obj, pd);
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

   if (efl_ui_mirrored_automatic_get(obj) && (sd->is_mirrored != mirrored))
     {
        sd->is_mirrored = mirrored;
     }
}

EOLIAN static Eina_Bool
_elm_widget_on_focus_region(Eo *obj, Elm_Widget_Smart_Data *_pd EINA_UNUSED, Evas_Coord *x EINA_UNUSED, Evas_Coord *y EINA_UNUSED, Evas_Coord *w EINA_UNUSED, Evas_Coord *h EINA_UNUSED)
{
   DBG("The %s widget does not implement the \"on_focus_region\" function.",
       efl_class_name_get(efl_class_get(obj)));

   return EINA_FALSE;
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
_on_sub_obj_hide(void *data EINA_UNUSED, const Efl_Event *event)
{
   elm_widget_focus_hide_handle(event->object);
}

static void
_on_sub_obj_del(void *data, const Efl_Event *event)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(data, sd);

   if (_elm_widget_is(event->object))
     {
        if (_is_focused(event->object)) _parents_unfocus(sd->obj);
     }
   if (event->object == sd->resize_obj)
     {
        /* already dels sub object */
        elm_widget_resize_object_set(sd->obj, NULL, EINA_TRUE);
     }
   else if (event->object == sd->hover_obj)
     {
        sd->hover_obj = NULL;
     }
   else
     {
        if (!elm_widget_sub_object_del(sd->obj, event->object))
          ERR("failed to remove sub object %p from %p\n", event->object, sd->obj);
     }
}

static const Evas_Smart_Cb_Description _smart_callbacks[] =
{
   {SIG_WIDGET_FOCUSED, ""},
   {SIG_WIDGET_UNFOCUSED, ""},
   {SIG_WIDGET_LANG_CHANGED, ""},
   {SIG_WIDGET_ACCESS_CHANGED, ""},
   {NULL, NULL}
};

static void
_obj_mouse_down(void *data,
                Evas *e EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info)
{
   Evas_Object *top;

   ELM_WIDGET_DATA_GET(data, sd);
   Evas_Event_Mouse_Down *ev = event_info;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   top = elm_widget_top_get(data);
   if (top && efl_isa(top, EFL_UI_WIN_CLASS)) _elm_win_focus_auto_hide(top);
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
   if (!sd->still_in) return;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     sd->still_in = EINA_FALSE;
   else
     {
        Evas_Coord x, y, w, h;
        evas_object_geometry_get(obj, &x, &y, &w, &h);
        if (ELM_RECTS_POINT_OUT(x, y, w, h, ev->cur.canvas.x, ev->cur.canvas.y))
          sd->still_in = EINA_FALSE;
     }
}

static void
_obj_mouse_up(void *data,
              Evas *e EINA_UNUSED,
              Evas_Object *obj,
              void *event_info)
{
   ELM_WIDGET_DATA_GET(data, sd);
   Evas_Event_Mouse_Up *ev = event_info;

   if (sd->still_in && (ev->flags == EVAS_BUTTON_NONE) &&
       (sd->focus_move_policy == ELM_FOCUS_MOVE_POLICY_CLICK))
     elm_widget_focus_mouse_up_handle(obj);

   sd->still_in = EINA_FALSE;
}

static void
_obj_mouse_in(void *data,
              Evas *e EINA_UNUSED,
              Evas_Object *obj,
              void *event_info EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET(data, sd);
   if (sd->focus_move_policy == ELM_FOCUS_MOVE_POLICY_IN)
     elm_widget_focus_mouse_up_handle(obj);
}

EOLIAN static void
_elm_widget_efl_canvas_group_group_add(Eo *obj, Elm_Widget_Smart_Data *priv)
{

   priv->obj = obj;
   priv->mirrored_auto_mode = EINA_TRUE; /* will follow system locale
                                          * settings */
   priv->focus_move_policy_auto_mode = EINA_TRUE;
   priv->focus_region_show_mode = ELM_FOCUS_REGION_SHOW_WIDGET;
   elm_widget_can_focus_set(obj, EINA_TRUE);
   priv->is_mirrored = elm_config_mirrored_get();
   priv->focus_move_policy = _elm_config->focus_move_policy;

   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_DOWN,
                                  _obj_mouse_down, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_MOVE,
                                  _obj_mouse_move, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_UP,
                                  _obj_mouse_up, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_IN,
                                  _obj_mouse_in, obj);
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
             if (newest == top)
               {
                  ELM_WIDGET_DATA_GET(newest, sd2);
                  if (!sd2) return;

                  if (!_is_focused(newest))
                    elm_widget_focus_steal(newest, NULL);
                  else
                    {
                       if (sd2->resize_obj && _is_focused(sd2->resize_obj))
                         elm_widget_focused_object_clear(sd2->resize_obj);
                       else
                         {
                            const Eina_List *l;
                            Evas_Object *child;
                            EINA_LIST_FOREACH(sd2->subobjs, l, child)
                              {
                                 if (!_elm_widget_is(child)) continue;
                                 if (_is_focused(child))
                                   {
                                      elm_widget_focused_object_clear(child);
                                      break;
                                   }
                              }
                         }
                    }
                  evas_object_focus_set(newest, EINA_TRUE);
               }
             else
               {
                  elm_object_focus_set(newest, EINA_FALSE);
                  elm_object_focus_set(newest, EINA_TRUE);
               }
          }
     }
}

EOLIAN static void
_elm_widget_efl_canvas_group_group_del(Eo *obj, Elm_Widget_Smart_Data *sd)
{
   Evas_Object *sobj;
   Elm_Translate_String_Data *ts;
   Elm_Event_Cb_Data *ecb;

   if (sd->hover_obj)
     {
        /* detach it from us */
        _callbacks_del(sd->hover_obj, obj);
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
   eina_stringshare_del(sd->accessible_name);
   evas_object_smart_data_set(obj, NULL);
   efl_canvas_group_del(efl_super(obj, MY_CLASS));
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

EOLIAN static void
_elm_widget_efl_gfx_position_set(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd, Evas_Coord x, Evas_Coord y)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, x, y))
     return;

   sd->x = x;
   sd->y = y;
   _smart_reconfigure(sd);

   efl_gfx_position_set(efl_super(obj, MY_CLASS), x, y);
}

EOLIAN static void
_elm_widget_efl_gfx_size_set(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd, Evas_Coord w, Evas_Coord h)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, w, h))
     return;

   sd->w = w;
   sd->h = h;
   _smart_reconfigure(sd);

   efl_gfx_size_set(efl_super(obj, MY_CLASS), w, h);
}

EOLIAN static void
_elm_widget_efl_gfx_visible_set(Eo *obj, Elm_Widget_Smart_Data *pd, Eina_Bool vis)
{
   Eina_Iterator *it;
   Evas_Object *o;

   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_VISIBLE, 0, vis))
     return;

   efl_gfx_visible_set(efl_super(obj, MY_CLASS), vis);

   _focus_state_eval(obj, pd);

   it = evas_object_smart_iterator_new(obj);
   EINA_ITERATOR_FOREACH(it, o)
     {
       if (evas_object_data_get(o, "_elm_leaveme")) continue;
       efl_gfx_visible_set(o, vis);
     }
   eina_iterator_free(it);

   if (!_elm_config->atspi_mode || pd->on_destroy)
     return;

   if (vis)
     {
        elm_interface_atspi_accessible_added(obj);
        if (_elm_widget_onscreen_is(obj))
          elm_interface_atspi_accessible_state_changed_signal_emit(obj, ELM_ATSPI_STATE_SHOWING, EINA_TRUE);
     }
   else
     {
        elm_interface_atspi_accessible_state_changed_signal_emit(obj, ELM_ATSPI_STATE_SHOWING, EINA_FALSE);
     }
}

EOLIAN static void
_elm_widget_efl_gfx_color_set(Eo *obj, Elm_Widget_Smart_Data *_pd EINA_UNUSED, int r, int g, int b, int a)
{
   Eina_Iterator *it;
   Evas_Object *o;

   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_COLOR_SET, 0, r, g, b, a))
     return;

   it = evas_object_smart_iterator_new(obj);
   EINA_ITERATOR_FOREACH(it, o)
     {
       if (evas_object_data_get(o, "_elm_leaveme")) continue;
       evas_object_color_set(o, r, g, b, a);
     }
   eina_iterator_free(it);
}

EOLIAN static void
_elm_widget_efl_canvas_object_no_render_set(Eo *obj, Elm_Widget_Smart_Data *_pd EINA_UNUSED, Eina_Bool hide)
{
   Eina_Iterator *it;
   Evas_Object *o;

   hide = !!hide;
   if (efl_canvas_object_no_render_get(obj) == hide)
     return;

   it = evas_object_smart_iterator_new(obj);
   EINA_ITERATOR_FOREACH(it, o)
     {
       if (evas_object_data_get(o, "_elm_leaveme")) continue;
       efl_canvas_object_no_render_set(o, hide);
     }
   eina_iterator_free(it);

   // bypass implementation in Efl.Canvas.Group
   efl_canvas_object_no_render_set(efl_super(obj, EFL_CANVAS_GROUP_CLASS), hide);
}

EOLIAN static void
_elm_widget_efl_canvas_object_is_frame_object_set(Eo *obj, Elm_Widget_Smart_Data *pd, Eina_Bool frame)
{
   Evas_Object *o;
   Eina_List *li;

   frame = !!frame;
   efl_canvas_object_is_frame_object_set(efl_super(obj, MY_CLASS), frame);
   EINA_LIST_FOREACH(pd->subobjs, li, o)
     {
       if (evas_object_data_get(o, "_elm_leaveme")) continue;
       efl_canvas_object_is_frame_object_set(o, frame);
     }
}

EOLIAN static void
_elm_widget_efl_canvas_object_clip_set(Eo *obj, Elm_Widget_Smart_Data *_pd EINA_UNUSED, Evas_Object *clip)
{
   Eina_Iterator *it;
   Evas_Object *o;

   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_CLIP_SET, 0, clip))
     return;

   efl_canvas_object_clip_set(efl_super(obj, MY_CLASS), clip);

   it = evas_object_smart_iterator_new(obj);
   EINA_ITERATOR_FOREACH(it, o)
     {
       if (evas_object_data_get(o, "_elm_leaveme")) continue;
       evas_object_clip_set(o, clip);
     }
   eina_iterator_free(it);
}

EOLIAN static void
_elm_widget_efl_canvas_group_group_calculate(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *_pd EINA_UNUSED)
{
   /* a NO-OP, on the base */
}

EOLIAN static void
_elm_widget_efl_canvas_group_group_member_add(Eo *obj, Elm_Widget_Smart_Data *_pd EINA_UNUSED, Evas_Object *child)
{
   int r, g, b, a;
   efl_canvas_group_member_add(efl_super(obj, MY_CLASS), child);

   if (evas_object_data_get(child, "_elm_leaveme")) return;

   evas_object_color_get(obj, &r, &g, &b, &a);
   evas_object_color_set(child, r, g, b, a);

   efl_canvas_object_no_render_set(child, efl_canvas_object_no_render_get(obj));
   evas_object_clip_set(child, evas_object_clip_get(obj));

   if (evas_object_visible_get(obj))
     evas_object_show(child);
   else
     evas_object_hide(child);
}

EOLIAN static void
_elm_widget_efl_canvas_group_group_member_del(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *_pd EINA_UNUSED, Evas_Object *child)
{
   if (!evas_object_data_get(child, "_elm_leaveme"))
      evas_object_clip_unset(child);
   efl_canvas_group_member_del(efl_super(obj, MY_CLASS), child);
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
   manager_is = elm_obj_widget_focus_next_manager_is((Eo *)obj);
   return manager_is;
}

static inline Eina_Bool
_internal_elm_widget_focus_direction_manager_is(const Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj) EINA_FALSE;

   Eina_Bool manager_is = EINA_FALSE;
   manager_is = elm_obj_widget_focus_direction_manager_is((Eo *)obj);
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
_propagate_event(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *obj = event->object;
   INTERNAL_ENTRY;
   Evas_Callback_Type type;
   Evas_Event_Flags *event_flags, prev_flags;
   union {
      Evas_Event_Key_Down    *down;
      Evas_Event_Key_Up      *up;
      Evas_Event_Mouse_Wheel *wheel;
      void                   *any;
   } event_info;

   if ((evas_focus_get(evas_object_evas_get(obj)) != elm_widget_top_get(obj)) &&
       efl_isa(obj, EFL_UI_WIN_CLASS))
     return;

   if (event->desc == EFL_EVENT_KEY_DOWN)
     {
        event_info.down = efl_input_legacy_info_get(event->info);
        EINA_SAFETY_ON_NULL_RETURN(event_info.down);
        type = EVAS_CALLBACK_KEY_DOWN;
        event_flags = &event_info.down->event_flags;
     }
   else if (event->desc == EFL_EVENT_KEY_UP)
     {
        event_info.up = efl_input_legacy_info_get(event->info);
        EINA_SAFETY_ON_NULL_RETURN(event_info.up);
        type = EVAS_CALLBACK_KEY_UP;
        event_flags = &event_info.up->event_flags;
     }
   else if (event->desc == EFL_EVENT_POINTER_WHEEL)
     {
        event_info.wheel = efl_input_legacy_info_get(event->info);
        EINA_SAFETY_ON_NULL_RETURN(event_info.wheel);
        type = EVAS_CALLBACK_MOUSE_WHEEL;
        event_flags = &event_info.wheel->event_flags;
     }
   else
     return;

   prev_flags = *event_flags;
   elm_widget_event_propagate(obj, type, event_info.any, event_flags);
   if (prev_flags != *event_flags)
     efl_input_event_flags_set(event->info, *event_flags);
}

/**
 * @internal
 *
 * If elm_widget_focus_region_get() returns EINA_FALSE, this function will
 * ignore region show action.
 */
EOLIAN static void
_elm_widget_focus_region_show(const Eo *obj, Elm_Widget_Smart_Data *_pd EINA_UNUSED)
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
        evas_object_geometry_get(o, &px, &py, NULL, NULL);

        if (_elm_scrollable_is(o) && !elm_widget_disabled_get(o))
          {
             Evas_Coord sx, sy;
             elm_interface_scrollable_content_region_get(o, &sx, &sy, NULL, NULL);

             // Get the object's on_focus_region position relative to the scroller.
             Evas_Coord rx, ry;
             rx = ox + x - px + sx;
             ry = oy + y - py + sy;

             switch (_elm_config->focus_autoscroll_mode)
               {
                case ELM_FOCUS_AUTOSCROLL_MODE_SHOW:
                   elm_interface_scrollable_content_region_show(o, rx, ry, w, h);
                   break;
                case ELM_FOCUS_AUTOSCROLL_MODE_BRING_IN:
                   elm_interface_scrollable_region_bring_in(o, rx, ry, w, h);
                   break;
                default:
                   break;
               }

             elm_widget_focus_region_get(o, &x, &y, &w, &h);
             evas_object_geometry_get(o, &ox, &oy, NULL, NULL);
          }
        else
          {
             x += ox - px;
             y += oy - py;
             ox = px;
             oy = py;
          }
        o = elm_widget_parent_get(o);
     }
}

EOLIAN static Eina_Bool
_elm_widget_focus_highlight_style_set(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd, const char *style)
{
   if (eina_stringshare_replace(&sd->focus_highlight_style, style)) return EINA_TRUE;
   return EINA_FALSE;
}

EOLIAN static const char*
_elm_widget_focus_highlight_style_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   return sd->focus_highlight_style;
}

static void
_parent_focus(Evas_Object *obj, Elm_Object_Item *item)
{
   API_ENTRY return;

   if (sd->focused) return;

   Evas_Object *o = elm_widget_parent_get(obj);
   sd->focus_order_on_calc = EINA_TRUE;

   if (o) _parent_focus(o, item);

   if (!sd->focus_order_on_calc)
     return;  /* we don't want to override it if by means of any of the
                 callbacks below one gets to calculate our order
                 first. */

   focus_order++;
   sd->focus_order = focus_order;
   sd->focused = EINA_TRUE;

   if (sd->top_win_focused)
     elm_obj_widget_on_focus(obj, item);
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

EOLIAN static void
_elm_widget_widget_parent_set(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *_pd EINA_UNUSED, Evas_Object *parent EINA_UNUSED)
{
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
     {
        if (elm_widget_is(child))
          ret &= elm_widget_access(child, is_access);
     }

   elm_obj_widget_access(obj, is_access);
   efl_event_callback_legacy_call(obj, ELM_WIDGET_EVENT_ACCESS_CHANGED, NULL);

   return ret;
}

EOLIAN static void
_elm_widget_access(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *_pd EINA_UNUSED, Eina_Bool is_access EINA_UNUSED)
{
}

EAPI Elm_Theme_Apply
elm_widget_theme(Evas_Object *obj)
{
   const Eina_List *l;
   Evas_Object *child;
   Elm_Tooltip *tt;
   Elm_Cursor *cur;
   Elm_Theme_Apply ret = ELM_THEME_APPLY_SUCCESS;

   API_ENTRY return ELM_THEME_APPLY_FAILED;

   EINA_LIST_FOREACH(sd->subobjs, l, child)
     if (_elm_widget_is(child))
       ret &= elm_widget_theme(child);

   if (sd->hover_obj) ret &= elm_widget_theme(sd->hover_obj);

   EINA_LIST_FOREACH(sd->tooltips, l, tt)
     elm_tooltip_theme(tt);
   EINA_LIST_FOREACH(sd->cursors, l, cur)
     elm_cursor_theme(cur);

   Elm_Theme_Apply ret2 = ELM_THEME_APPLY_FAILED;
   ret2 = elm_obj_widget_theme_apply(obj);
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
     {
        if (elm_widget_is(child))
          elm_widget_theme_specific(child, th, force);
     }
   if (sd->hover_obj) elm_widget_theme(sd->hover_obj);
   EINA_LIST_FOREACH(sd->tooltips, l, tt)
     elm_tooltip_theme(tt);
   EINA_LIST_FOREACH(sd->cursors, l, cur)
     elm_cursor_theme(cur);
   elm_obj_widget_theme_apply(obj);
}

EOLIAN static Elm_Theme_Apply
_elm_widget_theme_apply(Eo *obj, Elm_Widget_Smart_Data *_pd EINA_UNUSED)
{
   _elm_widget_mirrored_reload(obj);
   if (elm_widget_disabled_get(obj))
     elm_widget_disabled_internal(obj, elm_widget_disabled_get(obj));

   return ELM_THEME_APPLY_SUCCESS;
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
EOLIAN static Eina_Bool
_elm_widget_efl_ui_base_mirrored_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   return sd->is_mirrored;
}

/**
 * @internal
 *
 * Sets the widget's mirrored mode.
 *
 * @param obj The widget.
 * @param mirrored EINA_TRUE to set mirrored mode. EINA_FALSE to unset.
 */
EOLIAN static void
_elm_widget_efl_ui_base_mirrored_set(Eo *obj, Elm_Widget_Smart_Data *sd, Eina_Bool mirrored)
{
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
EOLIAN static Eina_Bool
_elm_widget_efl_ui_base_mirrored_automatic_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   return sd->mirrored_auto_mode;
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
EOLIAN static void
_elm_widget_efl_ui_base_mirrored_automatic_set(Eo *obj, Elm_Widget_Smart_Data *sd, Eina_Bool automatic)
{
   if (sd->mirrored_auto_mode != automatic)
     {
        sd->mirrored_auto_mode = automatic;

        if (automatic)
          {
             efl_ui_mirrored_set(obj, elm_config_mirrored_get());
          }
     }
}

EOLIAN static void
_elm_widget_on_show_region_hook_set(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd, region_hook_func_type func, void *data)
{
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
   Eo *parent = NULL;

   parent = efl_parent_get(sobj);
   if (!efl_isa(parent, ELM_WIDGET_CLASS))
     {
        ERR("You passed a wrong parent parameter (%p %s). "
            "Elementary widget's parent should be an elementary widget.", parent, evas_object_type_get(parent));
        return EINA_FALSE;
     }

   return elm_obj_widget_sub_object_add(parent, sobj);
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
EOLIAN static Eina_Bool
_elm_widget_sub_object_add(Eo *obj, Elm_Widget_Smart_Data *sd, Evas_Object *sobj)
{
   Eina_Bool mirrored, pmirrored = efl_ui_mirrored_get(obj);

   EINA_SAFETY_ON_TRUE_RETURN_VAL(obj == sobj, EINA_FALSE);

   if (sobj == sd->parent_obj)
     {
        /* in this case, sobj must be an elm widget, or something
         * very wrong is happening */
        if (!_elm_widget_is(sobj)) return EINA_FALSE;

        if (!elm_widget_sub_object_del(sobj, obj)) return EINA_FALSE;
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
               return EINA_FALSE;
          }
        sdc->parent_obj = obj;

        _full_eval(sobj, sdc);

        if (!sdc->on_create)
          elm_obj_widget_orientation_set(sobj, sd->orient_mode);
        else
          sdc->orient_mode = sd->orient_mode;

        if (!sdc->on_create)
          {
             if (!sdc->disabled && (elm_widget_disabled_get(obj)))
               {
                  elm_widget_focus_disabled_handle(sobj);
                  elm_obj_widget_disable(sobj);
               }
          }

        _elm_widget_top_win_focused_set(sobj, sd->top_win_focused);

        /* update child focusable-ness on self and parents, now that a
         * focusable child got in */
        if (!sd->child_can_focus && (_is_focusable(sobj)))
          {
             Elm_Widget_Smart_Data *sdp = sd;

             sdp->child_can_focus = EINA_TRUE;
             while (sdp->parent_obj)
               {
                  sdp = efl_data_scope_get(sdp->parent_obj, MY_CLASS);

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
             if (!elm_widget_sub_object_del(data, sobj)) return EINA_FALSE;
          }
     }
   sd->subobjs = eina_list_append(sd->subobjs, sobj);
   evas_object_data_set(sobj, "elm-parent", obj);

   _callbacks_add(sobj, obj);
   if (_elm_widget_is(sobj))
     {
        ELM_WIDGET_DATA_GET(sobj, sdc);

        /* NOTE: In the following two lines, 'sobj' is correct. Do not change it.
         * Due to elementary's scale policy, scale and pscale can be different in
         * some cases. This happens when sobj's previous parent and new parent have
         * different scale value.
         * For example, if sobj's previous parent's scale is 5 and new parent's scale
         * is 2 while sobj's scale is 0. Then 'pscale' is 5 and 'scale' is 2. So we
         * need to reset sobj's scale to 5.
         * Note that each widget's scale is 0 by default.
         */
        double scale, pscale = efl_ui_scale_get(sobj);
        Elm_Theme *th, *pth = elm_widget_theme_get(sobj);

        scale = efl_ui_scale_get(sobj);
        th = elm_widget_theme_get(sobj);
        mirrored = efl_ui_mirrored_get(sobj);

        if (!sdc->on_create)
          {
             if ((scale != pscale) || (th != pth) || (pmirrored != mirrored))
               elm_widget_theme(sobj);
          }

        if (_is_focused(sobj)) _parents_focus(obj);

        elm_widget_display_mode_set(sobj,
              evas_object_size_hint_display_mode_get(obj));
        if (_elm_config->atspi_mode && !sd->on_create)
          {
             Elm_Interface_Atspi_Accessible *aparent;
             aparent = elm_interface_atspi_accessible_parent_get(sobj);
             if (aparent)
                elm_interface_atspi_accessible_children_changed_added_signal_emit(aparent, sobj);
          }
     }

end:
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_widget_sub_object_del(Eo *obj, Elm_Widget_Smart_Data *sd, Evas_Object *sobj)
{
   Evas_Object *sobj_parent;

   if (!sobj) return EINA_FALSE;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(obj == sobj, EINA_FALSE);

   sobj_parent = evas_object_data_del(sobj, "elm-parent");
   if (sobj_parent && sobj_parent != obj)
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

        return EINA_FALSE;
     }

   if (_elm_widget_is(sobj))
     {
        if (_is_focused(sobj))
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
        if (_elm_config->atspi_mode && !sd->on_destroy)
          {
             Elm_Interface_Atspi_Accessible *aparent;
             aparent = elm_interface_atspi_accessible_parent_get(sobj);
             if (aparent)
                elm_interface_atspi_accessible_children_changed_del_signal_emit(aparent, sobj);
          }

        ELM_WIDGET_DATA_GET(sobj, sdc);
        sdc->parent_obj = NULL;

        _full_eval(sobj, sdc);
     }

   if (sd->resize_obj == sobj) sd->resize_obj = NULL;

   sd->subobjs = eina_list_remove(sd->subobjs, sobj);

   _callbacks_del(sobj, obj);

   return EINA_TRUE;
}

/*
 * @internal
 *
 * a resize object is added to and deleted from the smart member and the sub object
 * of the parent if the third argument, Eina_Bool sub_obj, is set as EINA_TRUE.
 */
EOLIAN static void
_elm_widget_resize_object_set(Eo *obj, Elm_Widget_Smart_Data *sd,
                             Evas_Object *sobj,
                             Eina_Bool sub_obj)
{
   Evas_Object *parent;

   if (sd->resize_obj == sobj) return;

   // orphan previous resize obj
   if (sd->resize_obj && sub_obj)
     {
        evas_object_clip_unset(sd->resize_obj);
        evas_object_smart_member_del(sd->resize_obj);

        if (_elm_widget_is(sd->resize_obj))
          {
             if (_is_focused(sd->resize_obj)) _parents_unfocus(obj);
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
EOLIAN static void
_elm_widget_hover_object_set(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd, Evas_Object *sobj)
{
   if (sd->hover_obj)
     {
        _callbacks_del(sd->hover_obj, obj);
     }
   sd->hover_obj = sobj;
   if (sd->hover_obj)
     {
        _callbacks_add(sobj, obj);
        _smart_reconfigure(sd);
     }
}

EOLIAN static void
_elm_widget_can_focus_set(Eo *obj, Elm_Widget_Smart_Data *sd, Eina_Bool can_focus)
{
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
             ELM_WIDGET_DATA_GET(o, sdp);
             if (!sdp || sdp->child_can_focus) break;
             sdp->child_can_focus = EINA_TRUE;
          }

        efl_event_callback_array_add(obj, focus_callbacks(), NULL);
     }
   else
     {
        // update child_can_focus of parents */
        Evas_Object *parent = elm_widget_parent_get(obj);
        while (parent)
          {
             const Eina_List *l;
             Evas_Object *subobj;

             ELM_WIDGET_DATA_GET(parent, sdp);

             sdp->child_can_focus = EINA_FALSE;
             EINA_LIST_FOREACH(sdp->subobjs, l, subobj)
               {
                  if (_is_focusable(subobj))
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
        efl_event_callback_array_del(obj, focus_callbacks(), NULL);
     }
     if (efl_finalized_get(obj))
       _focus_state_eval(obj, sd);
}

EOLIAN static Eina_Bool
_elm_widget_can_focus_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   return sd->can_focus;
}

EOLIAN static Eina_Bool
_elm_widget_child_can_focus_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   return sd->child_can_focus;
}

static void
_full_eval_children(Eo *obj, Elm_Widget_Smart_Data *sd)
{
   Eina_List *l;
   Eo *child;

   _full_eval(obj, sd);

   EINA_LIST_FOREACH(sd->subobjs , l, child)
     {
        Elm_Widget_Smart_Data *sd_child;

        if (!efl_isa(child, ELM_WIDGET_CLASS)) continue;

        sd_child = efl_data_scope_get(child, ELM_WIDGET_CLASS);
        _full_eval_children(child, sd_child);
     }
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
EOLIAN static void
_elm_widget_tree_unfocusable_set(Eo *obj, Elm_Widget_Smart_Data *sd, Eina_Bool tree_unfocusable)
{
   tree_unfocusable = !!tree_unfocusable;
   if (sd->tree_unfocusable == tree_unfocusable) return;
   sd->tree_unfocusable = tree_unfocusable;
   elm_widget_focus_tree_unfocusable_handle(obj);

   //focus state eval on all children
   _full_eval_children(obj, sd);
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
EOLIAN static Eina_Bool
_elm_widget_tree_unfocusable_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   return sd->tree_unfocusable;
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
EOLIAN static Eina_List*
_elm_widget_can_focus_child_list_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   const Eina_List *l;
   Eina_List *child_list = NULL;
   Evas_Object *child;

   EINA_LIST_FOREACH(sd->subobjs, l, child)
     {
        if (!_elm_widget_is(child)) continue;
        if ((elm_widget_can_focus_get(child)) &&
            (evas_object_visible_get(child)) &&
            (!elm_widget_disabled_get(child)))
          child_list = eina_list_append(child_list, child);
        else
          {
             Eina_List *can_focus_list;
             can_focus_list = elm_widget_can_focus_child_list_get(child);
             if (can_focus_list)
               child_list = eina_list_merge(child_list, can_focus_list);
          }
     }

   return child_list;
}

EOLIAN static void
_elm_widget_highlight_ignore_set(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd, Eina_Bool ignore)
{
   sd->highlight_ignore = !!ignore;
}

EOLIAN static Eina_Bool
_elm_widget_highlight_ignore_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   return sd->highlight_ignore;
}

EOLIAN static void
_elm_widget_highlight_in_theme_set(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd, Eina_Bool highlight)
{
   sd->highlight_in_theme = !!highlight;
   /* FIXME: if focused, it should switch from one mode to the other */
}

void
_elm_widget_highlight_in_theme_update(Eo *obj)
{
   Evas_Object *top = elm_widget_top_get(obj);

   if (top && efl_isa(top, EFL_UI_WIN_CLASS))
     {
        _elm_win_focus_highlight_in_theme_update(
           top, elm_widget_highlight_in_theme_get(obj));
     }
}

EOLIAN static Eina_Bool
_elm_widget_highlight_in_theme_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   return sd->highlight_in_theme;
}

EOLIAN static void
_elm_widget_access_highlight_in_theme_set(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd, Eina_Bool highlight)
{
   sd->access_highlight_in_theme = !!highlight;
}

EOLIAN static Eina_Bool
_elm_widget_access_highlight_in_theme_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   return sd->access_highlight_in_theme;
}

EOLIAN static Eina_Bool
_elm_widget_focus_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   return (sd->focused && sd->top_win_focused);
}

EOLIAN static Eina_Bool
_elm_widget_highlight_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   return sd->highlighted;
}

EOLIAN static Evas_Object*
_elm_widget_focused_object_get(Eo *obj, Elm_Widget_Smart_Data *sd)
{
   const Evas_Object *subobj;
   const Eina_List *l;

   if (!sd->focused || !sd->top_win_focused) return NULL;
   EINA_LIST_FOREACH(sd->subobjs, l, subobj)
     {
        Evas_Object *fobj;
        if (!_elm_widget_is(subobj)) continue;
        fobj = elm_widget_focused_object_get(subobj);
        if (fobj) return fobj;
     }
   return (Evas_Object *)obj;
}

EOLIAN static Evas_Object*
_elm_widget_top_get(Eo *obj, Elm_Widget_Smart_Data *sd)
{
   if (sd->parent_obj)
     {
        Evas_Object *ret = NULL;
        if (!efl_isa(sd->parent_obj, ELM_WIDGET_CLASS)) return NULL;
        ret = elm_obj_widget_top_get((Eo *) sd->parent_obj);
        return ret;
     }
   return (Evas_Object *)obj;
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

EOLIAN static Evas_Object *
_elm_widget_parent2_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   return sd->parent2;
}

EOLIAN static void
_elm_widget_parent2_set(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd, Evas_Object *parent)
{
   sd->parent2 = parent;
}

EAPI void
elm_widget_event_callback_add(Eo *obj, Elm_Event_Cb func, const void *data)
{
   API_ENTRY return;
   EINA_SAFETY_ON_NULL_RETURN(func);

   Elm_Event_Cb_Data *ecb = ELM_NEW(Elm_Event_Cb_Data);
   if (!ecb)
     {
        ERR("Failed to allocate memory");
        return;
     }
   ecb->func = func;
   ecb->data = data;
   sd->event_cb = eina_list_append(sd->event_cb, ecb);
}

EAPI void *
elm_widget_event_callback_del(Eo *obj, Elm_Event_Cb func, const void *data)
{
   API_ENTRY return NULL;
   EINA_SAFETY_ON_NULL_RETURN_VAL(func, NULL);
   Eina_List *l;
   Elm_Event_Cb_Data *ecd;

   EINA_LIST_FOREACH(sd->event_cb, l, ecd)
     if ((ecd->func == func) && (ecd->data == data))
       {
          free(ecd);
          sd->event_cb = eina_list_remove_list(sd->event_cb, l);
          return (void *)data;
       }

   return NULL;
}

EAPI Eina_Bool
elm_widget_event_propagate(Eo *obj, Evas_Callback_Type type, void *event_info,
                           Evas_Event_Flags *event_flags)
{
   Evas_Object *parent = obj;
   Elm_Event_Cb_Data *ecd;
   Eina_List *l, *l_prev;

   while (parent &&
          (!(event_flags && ((*event_flags) & EVAS_EVENT_FLAG_ON_HOLD))))
     {
        ELM_WIDGET_CHECK(parent) EINA_FALSE;
        Elm_Widget_Smart_Data *sd = efl_data_scope_get(parent, MY_CLASS);

        Eina_Bool int_ret = EINA_FALSE;

        if (elm_widget_disabled_get(obj))
          {
             parent = sd->parent_obj;
             continue;
          }

        int_ret = elm_obj_widget_event(parent, obj, type, event_info);
        if (int_ret) return EINA_TRUE;

        EINA_LIST_FOREACH_SAFE(sd->event_cb, l, l_prev, ecd)
          {
             if (ecd->func((void *)ecd->data, parent, obj, type, event_info) ||
                 (event_flags && ((*event_flags) & EVAS_EVENT_FLAG_ON_HOLD)))
                return EINA_TRUE;
          }
        parent = sd->parent_obj;
     }

   return EINA_FALSE;
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
EOLIAN static void
_elm_widget_focus_custom_chain_set(Eo *obj, Elm_Widget_Smart_Data *sd, Eina_List *objs)
{
   if (!_elm_widget_focus_chain_manager_is(obj)) return;

   elm_widget_focus_custom_chain_unset(obj);

   Eina_List *l;
   Evas_Object *o;

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
EOLIAN static const Eina_List*
_elm_widget_focus_custom_chain_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   return (const Eina_List *)sd->focus_chain;
}

/**
 * @internal
 *
 * Unset custom focus chain
 *
 * @param obj The container widget
 * @ingroup Widget
 */
EOLIAN static void
_elm_widget_focus_custom_chain_unset(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   Eina_List *l, *l_next;
   Evas_Object *o;

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
EOLIAN static void
_elm_widget_focus_custom_chain_append(Eo *obj, Elm_Widget_Smart_Data *sd, Evas_Object *child, Evas_Object *relative_child)
{
   EINA_SAFETY_ON_NULL_RETURN(child);

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
EOLIAN static void
_elm_widget_focus_custom_chain_prepend(Eo *obj, Elm_Widget_Smart_Data *sd, Evas_Object *child, Evas_Object *relative_child)
{
   EINA_SAFETY_ON_NULL_RETURN(child);

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
EOLIAN static void
_elm_widget_focus_cycle(Eo *obj, Elm_Widget_Smart_Data *_pd EINA_UNUSED, Elm_Focus_Direction dir)
{
   Evas_Object *target = NULL;
   Elm_Object_Item *target_item = NULL;
   if (!_elm_widget_is(obj))
     return;
   elm_widget_focus_next_get(obj, dir, &target, &target_item);
   if (target)
     {
        /* access */
        if (_elm_config->access_mode)
          {
             /* highlight cycle does not steal a focus, only after window gets
                the ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_ACTIVATE message,
                target will steal focus, or focus its own job. */
             if (!_elm_access_auto_highlight_get())
               elm_widget_focus_steal(target, target_item);

             _elm_access_highlight_set(target);
             elm_widget_focus_region_show(target);
          }
        else elm_widget_focus_steal(target, target_item);
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
EOLIAN static Eina_Bool
_elm_widget_focus_direction_go(Eo *obj, Elm_Widget_Smart_Data *_pd EINA_UNUSED, double degree)
{
   Evas_Object *target = NULL;
   Elm_Object_Item *target_item = NULL;
   Evas_Object *current_focused = NULL;
   double weight = 0.0;

   if (!_elm_widget_is(obj)) return EINA_FALSE;
   if (!_is_focused(obj)) return EINA_FALSE;

   current_focused = elm_widget_focused_object_get(obj);

   if (elm_widget_focus_direction_get
         (obj, current_focused, degree, &target, &target_item, &weight))
     {
        elm_widget_focus_steal(target, NULL);
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

double
_elm_widget_focus_direction_weight_get(const Evas_Object *obj1,
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

EOLIAN static Eina_Bool
_elm_widget_focus_direction_get(const Eo *obj, Elm_Widget_Smart_Data *sd, const Evas_Object *base, double degree, Evas_Object **direction, Elm_Object_Item **direction_item, double *weight)
{
   double c_weight;

   /* -1 means the best was already decided. Don't need any more searching. */
   if (!direction || !weight || !base || (obj == base))
     return EINA_FALSE;

   /* Ignore if disabled */
   if ((!evas_object_visible_get(obj))
       || (elm_widget_disabled_get(obj))
       || (elm_widget_tree_unfocusable_get(obj)))
     return EINA_FALSE;

   /* Try use hook */
   if (_internal_elm_widget_focus_direction_manager_is(obj))
     {
        Eina_Bool int_ret = EINA_FALSE;
        int_ret = elm_obj_widget_focus_direction((Eo *)obj, base, degree, direction, direction_item, weight);
        return int_ret;
     }

   if (!elm_widget_can_focus_get(obj) || _is_focused((Eo *)obj))
     return EINA_FALSE;

   c_weight = _elm_widget_focus_direction_weight_get(base, obj, degree);
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
                    return EINA_FALSE;
               }
          }
        *direction = (Evas_Object *)obj;
        *weight = c_weight;
        return EINA_TRUE;
     }

   return EINA_FALSE;
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
EOLIAN static Eina_Bool
_elm_widget_focus_list_direction_get(const Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *_pd EINA_UNUSED, const Evas_Object *base, const Eina_List *items, list_data_get_func_type list_data_get, double degree, Evas_Object **direction, Elm_Object_Item **direction_item, double *weight)
{
   if (!direction || !weight || !base || !items)
     return EINA_FALSE;

   const Eina_List *l = items;
   Evas_Object *current_best = *direction;

   for (; l; l = eina_list_next(l))
     {
        Evas_Object *cur = list_data_get(l);
        if (cur && _elm_widget_is(cur))
          elm_widget_focus_direction_get(cur, base, degree, direction, direction_item, weight);
     }
   if (current_best != *direction) return EINA_TRUE;

   return EINA_FALSE;
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
EOLIAN static Eina_Bool
_elm_widget_focus_next_get(const Eo *obj, Elm_Widget_Smart_Data *sd, Elm_Focus_Direction dir, Evas_Object **next, Elm_Object_Item **next_item)
{
   Elm_Access_Info *ac;

   if (!next)
     return EINA_FALSE;
   *next = NULL;

   /* Ignore if disabled */
   if (_elm_config->access_mode && _elm_access_auto_highlight_get())
     {
        if (!evas_object_visible_get(obj)
            || (elm_widget_tree_unfocusable_get(obj)))
          return EINA_FALSE;
     }
   else
     {
        if ((!evas_object_visible_get(obj))
            || (elm_widget_disabled_get(obj))
            || (elm_widget_tree_unfocusable_get(obj)))
          return EINA_FALSE;
     }

   /* Try use hook */
   if (_elm_widget_focus_chain_manager_is(obj))
     {
        Eina_Bool int_ret = EINA_FALSE;
        int_ret = elm_obj_widget_focus_next((Eo *)obj, dir, next, next_item);
        if (!int_ret && _is_focused((Eo *)obj))
          {
             Evas_Object *o = NULL;
             if (dir == ELM_FOCUS_PREVIOUS)
               *next_item = sd->item_focus_previous;
             else if (dir == ELM_FOCUS_NEXT)
               *next_item = sd->item_focus_next;
             else if (dir == ELM_FOCUS_UP)
               *next_item = sd->item_focus_up;
             else if (dir == ELM_FOCUS_DOWN)
               *next_item = sd->item_focus_down;
             else if (dir == ELM_FOCUS_RIGHT)
               *next_item = sd->item_focus_right;
             else if (dir == ELM_FOCUS_LEFT)
               *next_item = sd->item_focus_left;
             if (*next_item)
               o = elm_object_item_widget_get(*next_item);

             if (!o)
               {
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
               }

             if (o)
               {
                  *next = o;
                  return EINA_TRUE;
               }
          }
        return int_ret;
     }

   /* access object does not check sd->can_focus, because an object could
      have highlight even though the object is not focusable. */
   if (_elm_config->access_mode && _elm_access_auto_highlight_get())
     {
        ac = _elm_access_info_get(obj);
        if (!ac) return EINA_FALSE;

        /* check whether the hover object is visible or not */
        if (!evas_object_visible_get(ac->hoverobj))
          return EINA_FALSE;
     }
   else if (!elm_widget_can_focus_get(obj))
     return EINA_FALSE;

   if (_is_focused((Eo *)obj))
     {
        if (dir == ELM_FOCUS_PREVIOUS)
          *next_item = sd->item_focus_previous;
        else if (dir == ELM_FOCUS_NEXT)
          *next_item = sd->item_focus_next;
        else if (dir == ELM_FOCUS_UP)
          *next_item = sd->item_focus_up;
        else if (dir == ELM_FOCUS_DOWN)
          *next_item = sd->item_focus_down;
        else if (dir == ELM_FOCUS_RIGHT)
          *next_item = sd->item_focus_right;
        else if (dir == ELM_FOCUS_LEFT)
          *next_item = sd->item_focus_left;
        if (*next_item) *next = elm_object_item_widget_get(*next_item);

        if (!(*next))
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
          }

        if (*next) return EINA_TRUE;
     }

   /* Return */
   *next = (Evas_Object *)obj;
   return !ELM_WIDGET_FOCUS_GET(obj);
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
EOLIAN static Eina_Bool
_elm_widget_focus_list_next_get(const Eo *obj, Elm_Widget_Smart_Data *_pd EINA_UNUSED, const Eina_List *items, list_data_get_func_type list_data_get, Elm_Focus_Direction dir, Evas_Object **next, Elm_Object_Item **next_item)
{
   Eina_List *(*list_next)(const Eina_List *list) = NULL;
   Evas_Object *focused_object = NULL;

   if (!next)
     return EINA_FALSE;
   *next = NULL;

   if (!_elm_widget_is(obj))
     return EINA_FALSE;

   if (!items)
     return EINA_FALSE;

   /* When Up, Down, Right, or Left, try direction_get first. */
   focused_object = elm_widget_focused_object_get(obj);
   if (focused_object)
     {
        if ((dir == ELM_FOCUS_UP)
           || (dir == ELM_FOCUS_DOWN)
           || (dir == ELM_FOCUS_RIGHT)
           || (dir == ELM_FOCUS_LEFT))
          {
             *next_item = elm_widget_focus_next_item_get(focused_object, dir);
             if (*next_item)
               *next = elm_object_item_widget_get(*next_item);
             else
               *next = elm_widget_focus_next_object_get(focused_object, dir);
             if (*next) return EINA_TRUE;
             else
               {
                  Evas_Object *n = NULL;
                  Elm_Object_Item *n_item = NULL;
                  double degree = 0;
                  double weight = 0.0;

                  if (dir == ELM_FOCUS_UP) degree = 0.0;
                  else if (dir == ELM_FOCUS_DOWN) degree = 180.0;
                  else if (dir == ELM_FOCUS_RIGHT) degree = 90.0;
                  else if (dir == ELM_FOCUS_LEFT) degree = 270.0;

                  if (elm_widget_focus_list_direction_get(obj, focused_object,
                                                          items, list_data_get,
                                                          degree, &n, &n_item,
                                                          &weight))
                    {
                       *next_item = n_item;
                       *next = n;
                       return EINA_TRUE;
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
     return EINA_FALSE;

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
   Elm_Object_Item *to_focus_item = NULL;

   /* Iterate sub items */
   /* Go to the end of list */
   for (; l; l = list_next(l))
     {
        Evas_Object *tmp = NULL;
        Elm_Object_Item *tmp_item = NULL;
        Evas_Object *cur = list_data_get(l);

        if (!cur) continue;
        if (!_elm_widget_is(cur)) continue;
        if (elm_widget_parent_get(cur) != obj)
          continue;

        /* Try Focus cycle in subitem */
        if (elm_widget_focus_next_get(cur, dir, &tmp, &tmp_item))
          {
             *next = tmp;
             *next_item = tmp_item;
             return EINA_TRUE;
          }
        else if ((dir == ELM_FOCUS_UP)
                 || (dir == ELM_FOCUS_DOWN)
                 || (dir == ELM_FOCUS_RIGHT)
                 || (dir == ELM_FOCUS_LEFT))
          {
             if (tmp && _is_focused(cur))
               {
                  *next = tmp;
                  *next_item = tmp_item;
                  return EINA_FALSE;
               }
          }
        else if ((tmp) && (!to_focus))
          {
             to_focus = tmp;
             to_focus_item = tmp_item;
          }
     }

   l = items;

   /* Get First possible */
   for (; l != start; l = list_next(l))
     {
        Evas_Object *tmp = NULL;
        Elm_Object_Item *tmp_item = NULL;
        Evas_Object *cur = list_data_get(l);

        if (elm_widget_parent_get(cur) != obj)
          continue;

        /* Try Focus cycle in subitem */
        elm_widget_focus_next_get(cur, dir, &tmp, &tmp_item);
        if (tmp)
          {
             *next = tmp;
             *next_item = tmp_item;
             return EINA_FALSE;
          }
     }

   *next = to_focus;
   *next_item = to_focus_item;
   return EINA_FALSE;
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
EOLIAN static Evas_Object*
_elm_widget_focus_next_object_get(const Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd, Elm_Focus_Direction dir)
{
   Evas_Object *ret = NULL;

   if (dir == ELM_FOCUS_PREVIOUS)
     ret = sd->focus_previous;
   else if (dir == ELM_FOCUS_NEXT)
     ret = sd->focus_next;
   else if (dir == ELM_FOCUS_UP)
     ret = sd->focus_up;
   else if (dir == ELM_FOCUS_DOWN)
     ret = sd->focus_down;
   else if (dir == ELM_FOCUS_RIGHT)
     ret = sd->focus_right;
   else if (dir == ELM_FOCUS_LEFT)
     ret = sd->focus_left;

   return ret;
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
EOLIAN static void
_elm_widget_focus_next_object_set(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd, Evas_Object *next, Elm_Focus_Direction dir)
{

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

EOLIAN static Elm_Object_Item*
_elm_widget_focus_next_item_get(const Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd, Elm_Focus_Direction dir)
{
   Elm_Object_Item *ret = NULL;

   if (dir == ELM_FOCUS_PREVIOUS)
     ret = sd->item_focus_previous;
   else if (dir == ELM_FOCUS_NEXT)
     ret = sd->item_focus_next;
   else if (dir == ELM_FOCUS_UP)
     ret = sd->item_focus_up;
   else if (dir == ELM_FOCUS_DOWN)
     ret = sd->item_focus_down;
   else if (dir == ELM_FOCUS_RIGHT)
     ret = sd->item_focus_right;
   else if (dir == ELM_FOCUS_LEFT)
     ret = sd->item_focus_left;

   return ret;
}

EOLIAN static void
_elm_widget_focus_next_item_set(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd, Elm_Object_Item *next_item, Elm_Focus_Direction dir)
{
   if (dir == ELM_FOCUS_PREVIOUS)
     sd->item_focus_previous = next_item;
   else if (dir == ELM_FOCUS_NEXT)
     sd->item_focus_next = next_item;
   else if (dir == ELM_FOCUS_UP)
     sd->item_focus_up = next_item;
   else if (dir == ELM_FOCUS_DOWN)
     sd->item_focus_down = next_item;
   else if (dir == ELM_FOCUS_RIGHT)
     sd->item_focus_right = next_item;
   else if (dir == ELM_FOCUS_LEFT)
     sd->item_focus_left = next_item;
}

EOLIAN static void
_elm_widget_parent_highlight_set(Eo *obj, Elm_Widget_Smart_Data *sd, Eina_Bool highlighted)
{
   highlighted = !!highlighted;

   Evas_Object *o = elm_widget_parent_get(obj);

   if (o) elm_widget_parent_highlight_set(o, highlighted);

   sd->highlighted = highlighted;
}

EOLIAN static void
_elm_widget_signal_emit(Eo *obj, Elm_Widget_Smart_Data *_pd EINA_UNUSED, const char *emission, const char *source)
{
   if (evas_object_smart_type_check(obj, "elm_layout"))
     elm_layout_signal_emit(obj, emission, source);
   else if (evas_object_smart_type_check(obj, "elm_icon"))
     {
        WRN("Deprecated function. This functionality on icon objects"
            " will be dropped on a next release.");
        _elm_icon_signal_emit(obj, emission, source);
     }
}

EOLIAN static void
_elm_widget_signal_callback_add(Eo *obj, Elm_Widget_Smart_Data *_pd EINA_UNUSED, const char *emission, const char *source, Edje_Signal_Cb func, void *data)
{
   EINA_SAFETY_ON_NULL_RETURN(func);

   if (evas_object_smart_type_check(obj, "elm_layout"))
     elm_layout_signal_callback_add(obj, emission, source, func, data);
   else if (evas_object_smart_type_check(obj, "elm_icon"))
     {
        WRN("Deprecated function. This functionality on icon objects"
            " will be dropped on a next release.");

        _elm_icon_signal_callback_add(obj, emission, source, func, data);
     }
}

EOLIAN static void*
_elm_widget_signal_callback_del(Eo *obj, Elm_Widget_Smart_Data *_pd EINA_UNUSED, const char *emission, const char *source, Edje_Signal_Cb func)
{
   void *data = NULL;

   if (evas_object_smart_type_check(obj, "elm_layout"))
     data = elm_layout_signal_callback_del(obj, emission, source, func);
   else if (evas_object_smart_type_check(obj, "elm_icon"))
     {
        WRN("Deprecated function. This functionality on icon objects"
            " will be dropped on a next release.");

        data = _elm_icon_signal_callback_del(obj, emission, source, func);
     }

   return data;
}

EOLIAN static void
_elm_widget_focus_set(Eo *obj, Elm_Widget_Smart_Data *sd, Eina_Bool focus)
{
   if (!sd->focused)
     {
        focus_order++;
        sd->focus_order = focus_order;
        sd->focused = EINA_TRUE;
        elm_obj_widget_on_focus(obj, NULL);
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
                  if (!_elm_widget_is(child)) continue;
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
             if (!_elm_widget_is(child)) continue;
             if ((_is_focusable(child)) &&
                 (!elm_widget_disabled_get(child)))
               {
                  elm_widget_focus_set(child, focus);
                  break;
               }
          }
     }
}

EOLIAN static Evas_Object*
_elm_widget_widget_parent_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   return sd->parent_obj;
}

static void
_focused_object_clear(Elm_Widget_Smart_Data *sd)
{
   if (sd->resize_obj && elm_widget_is(sd->resize_obj) &&
       _is_focused(sd->resize_obj))
     {
        elm_obj_widget_focused_object_clear(sd->resize_obj);
     }
   else
     {
        const Eina_List *l;
        Evas_Object *child;
        EINA_LIST_FOREACH(sd->subobjs, l, child)
          {
             if (_elm_widget_is(child) && _is_focused(child))
               {
                  elm_obj_widget_focused_object_clear(child);
                  break;
               }
          }
     }
}

EOLIAN static void
_elm_widget_focused_object_clear(Eo *obj, Elm_Widget_Smart_Data *sd)
{
   if (!sd->focused) return;
   _focused_object_clear(sd);
   sd->focused = EINA_FALSE;
   if (sd->top_win_focused)
     elm_obj_widget_on_focus(obj, NULL);
}

EOLIAN static void
_elm_widget_focus_steal(Eo *obj, Elm_Widget_Smart_Data *sd, Elm_Object_Item *item)
{
   Evas_Object *parent, *parent2, *o;

   if (sd->focused) return;
   if (sd->disabled) return;
   if (!sd->can_focus) return;
   if (sd->tree_unfocusable) return;
   parent = obj;
   for (;; )
     {
        o = elm_widget_parent_get(parent);
        if (!o) break;
        sd = efl_data_scope_get(o, MY_CLASS);
        if (sd->disabled || sd->tree_unfocusable) return;
        if (sd->focused) break;
        parent = o;
     }
   if ((!elm_widget_parent_get(parent)) &&
       (!elm_widget_parent2_get(parent)))
     elm_obj_widget_focused_object_clear(parent);
   else
     {
        parent2 = elm_widget_parent_get(parent);
        if (!parent2) parent2 = elm_widget_parent2_get(parent);
        parent = parent2;
        sd = efl_data_scope_get(parent, MY_CLASS);
        if (sd) _focused_object_clear(sd);
     }
   _parent_focus(obj, item);
   elm_widget_focus_region_show(obj);
   return;
}

static void
_parents_on_focus(Evas_Object *obj)
{
   API_ENTRY return;
   if (!sd->focused || !sd->top_win_focused) return;

   Evas_Object *o = elm_widget_parent_get(obj);
   if (o) _parents_on_focus(o);
   elm_obj_widget_on_focus(obj, NULL);
}

EOLIAN static void
_elm_widget_focus_restore(Eo *obj, Elm_Widget_Smart_Data *_pd EINA_UNUSED)
{
   Evas_Object *newest = NULL;
   unsigned int newest_focus_order = 0;

   newest = elm_widget_newest_focus_order_get(obj, &newest_focus_order, EINA_TRUE);
   if (newest)
     _parents_on_focus(newest);
}

void
_elm_widget_focus_auto_show(Evas_Object *obj)
{
   Evas_Object *top = elm_widget_top_get(obj);
   if (top && efl_isa(top, EFL_UI_WIN_CLASS)) _elm_win_focus_auto_show(top);
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
     {
        if (elm_widget_is(child))
          _elm_widget_top_win_focused_set(child, top_win_focused);
     }
   sd->top_win_focused = top_win_focused;

   if (sd->focused && !sd->top_win_focused)
     elm_obj_widget_on_focus(obj, NULL);
}

Eina_Bool
_elm_widget_top_win_focused_get(const Evas_Object *obj)
{
   API_ENTRY return EINA_FALSE;
   return sd->top_win_focused;
}

static void
_elm_widget_disabled_eval(const Evas_Object *obj, Eina_Bool disabled)
{
   const Eina_List *l;
   Evas_Object *child;
   ELM_WIDGET_DATA_GET(obj, sd);

   if (disabled)
     {
        EINA_LIST_FOREACH(sd->subobjs, l, child)
          {
              if (elm_widget_is(child))
                {
                   elm_widget_focus_disabled_handle(child);
                   elm_obj_widget_disable(child);
                   _elm_widget_disabled_eval(child, EINA_TRUE);
                }
          }
     }
   else
     {
        EINA_LIST_FOREACH(sd->subobjs, l, child)
          if (elm_widget_is(child))
            {
               ELM_WIDGET_DATA_GET(child, sdc);
               if (!sdc->disabled)
                 {
                    elm_widget_focus_disabled_handle(child);
                    elm_obj_widget_disable(child);
                    _elm_widget_disabled_eval(child, EINA_FALSE);
                 }
            }
     }
}

static void
elm_widget_disabled_internal(Eo *obj, Eina_Bool disabled)
{
   Eina_Bool parent_state = EINA_FALSE;

   if (disabled)
     {
        elm_widget_focus_disabled_handle(obj);
        elm_obj_widget_disable(obj);
        _elm_widget_disabled_eval(obj, EINA_TRUE);
     }
   else
     {
        parent_state = elm_widget_disabled_get(elm_widget_parent_get(obj));
        if (parent_state) return;
        elm_widget_focus_disabled_handle(obj);
        elm_obj_widget_disable(obj);
        _elm_widget_disabled_eval(obj, EINA_FALSE);
     }
}

EOLIAN static void
_elm_widget_disabled_set(Eo *obj, Elm_Widget_Smart_Data *sd, Eina_Bool disabled)
{
   if (sd->disabled == disabled) return;
   sd->disabled = !!disabled;

   elm_widget_disabled_internal(obj, disabled);

   if (efl_finalized_get(obj))
     _full_eval_children(obj, sd);
}

EOLIAN static Eina_Bool
_elm_widget_disabled_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   Eina_Bool disabled = EINA_FALSE;

   if (sd->disabled) disabled = EINA_TRUE;
   else disabled = elm_widget_disabled_get(elm_widget_parent_get(obj));
   return disabled;
}

EOLIAN static void
_elm_widget_show_region_set(Eo *obj, Elm_Widget_Smart_Data *sd, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Eina_Bool forceshow)
{

   Evas_Object *parent_obj, *child_obj;
   Evas_Coord px, py, cx, cy, nx = 0, ny = 0;


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
             elm_interface_scrollable_content_pos_get(obj, &nx, &ny);
             x -= nx;
             y -= ny;
          }
     }

   do
     {
        parent_obj = sd->parent_obj;
        child_obj = sd->obj;
        if ((!parent_obj) || (!_elm_widget_is(parent_obj))) break;
        sd = efl_data_scope_get(parent_obj, MY_CLASS);
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

EOLIAN static void
_elm_widget_show_region_get(const Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
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
EOLIAN static Eina_Bool
_elm_widget_focus_region_get(Eo *obj, Elm_Widget_Smart_Data *_pd EINA_UNUSED, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Eina_Bool int_ret = EINA_FALSE;
   int_ret = elm_obj_widget_on_focus_region((Eo *)obj, x, y, w, h);
   if (!int_ret)
     {
        evas_object_geometry_get(obj, NULL, NULL, w, h);
        if (x) *x = 0;
        if (y) *y = 0;
     }
   if ((*w <= 0) || (*h <= 0)) return EINA_FALSE;
   return EINA_TRUE;
}

EOLIAN static void
_elm_widget_parents_bounce_get(Eo *obj, Elm_Widget_Smart_Data *_pd EINA_UNUSED, Eina_Bool *horiz, Eina_Bool *vert)
{
   Evas_Object *parent_obj = obj;
   Eina_Bool h = EINA_FALSE, v = EINA_FALSE;

   *horiz = EINA_FALSE;
   *vert = EINA_FALSE;

   do
     {
        parent_obj = elm_widget_parent_get(parent_obj);
        if ((!parent_obj) || (!_elm_widget_is(parent_obj))) break;

        if (_elm_scrollable_is(parent_obj))
          {
             elm_interface_scrollable_bounce_allow_get(parent_obj, &h, &v);
             if (h) *horiz = EINA_TRUE;
             if (v) *vert = EINA_TRUE;
          }
     }
   while (parent_obj);
}

EOLIAN static Eina_List*
_elm_widget_scrollable_children_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   Eina_List *ret;
   ret = NULL;

   Eina_List *l;
   Evas_Object *child;


   EINA_LIST_FOREACH(sd->subobjs, l, child)
     {
        if (elm_widget_is(child) && _elm_scrollable_is(child))
          ret = eina_list_append(ret, child);
     }

   return ret;
}

EOLIAN static void
_elm_widget_scroll_hold_push(Eo *obj, Elm_Widget_Smart_Data *sd)
{
   sd->scroll_hold++;
   if (sd->scroll_hold == 1)
     {
        if (_elm_scrollable_is(obj))
           elm_interface_scrollable_hold_set(obj, EINA_TRUE);
        else
          {
             Eina_List *scr_children, *l;
             Evas_Object *child;

             scr_children = elm_widget_scrollable_children_get(obj);
             EINA_LIST_FOREACH(scr_children, l, child)
               {
                  elm_interface_scrollable_hold_set(child, EINA_TRUE);
               }
             eina_list_free(scr_children);
          }
     }
   if (sd->parent_obj) elm_obj_widget_scroll_hold_push(sd->parent_obj);
   // FIXME: on delete/reparent hold pop
}

EOLIAN static void
_elm_widget_scroll_hold_pop(Eo *obj, Elm_Widget_Smart_Data *sd)
{
   sd->scroll_hold--;
   if (!sd->scroll_hold)
     {
        if (_elm_scrollable_is(obj))
           elm_interface_scrollable_hold_set(obj, EINA_FALSE);
        else
          {
             Eina_List *scr_children, *l;
             Evas_Object *child;

             scr_children = elm_widget_scrollable_children_get(obj);
             EINA_LIST_FOREACH(scr_children, l, child)
               {
                  elm_interface_scrollable_hold_set(child, EINA_FALSE);
               }
             eina_list_free(scr_children);
          }
     }
   if (sd->parent_obj) elm_obj_widget_scroll_hold_pop(sd->parent_obj);
   if (sd->scroll_hold < 0) sd->scroll_hold = 0;
}

EOLIAN static int
_elm_widget_scroll_hold_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   return sd->scroll_hold;
}

EOLIAN static void
_elm_widget_scroll_freeze_push(Eo *obj, Elm_Widget_Smart_Data *sd)
{
   sd->scroll_freeze++;
   if (sd->scroll_freeze == 1)
     {
        if (_elm_scrollable_is(obj))
           elm_interface_scrollable_freeze_set(obj, EINA_TRUE);
        else
          {
             Eina_List *scr_children, *l;
             Evas_Object *child;

             scr_children = elm_widget_scrollable_children_get(obj);
             EINA_LIST_FOREACH(scr_children, l, child)
               {
                  elm_interface_scrollable_freeze_set(child, EINA_TRUE);
               }
             eina_list_free(scr_children);
          }
     }
   if (sd->parent_obj) elm_obj_widget_scroll_freeze_push(sd->parent_obj);
   // FIXME: on delete/reparent freeze pop
}

EOLIAN static void
_elm_widget_scroll_freeze_pop(Eo *obj, Elm_Widget_Smart_Data *sd)
{
   sd->scroll_freeze--;
   if (!sd->scroll_freeze)
     {
        if (_elm_scrollable_is(obj))
           elm_interface_scrollable_freeze_set(obj, EINA_FALSE);
        else
          {
             Eina_List *scr_children, *l;
             Evas_Object *child;

             scr_children = elm_widget_scrollable_children_get(obj);
             EINA_LIST_FOREACH(scr_children, l, child)
               {
                  elm_interface_scrollable_freeze_set(child, EINA_FALSE);
               }
             eina_list_free(scr_children);
          }
     }
   if (sd->parent_obj) elm_obj_widget_scroll_freeze_pop(sd->parent_obj);
   if (sd->scroll_freeze < 0) sd->scroll_freeze = 0;
}

EOLIAN static int
_elm_widget_scroll_freeze_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   return sd->scroll_freeze;
}

EOLIAN static void
_elm_widget_efl_ui_base_scale_set(Eo *obj, Elm_Widget_Smart_Data *sd, double scale)
{
   if (scale < 0.0) scale = 0.0;
   if (sd->scale != scale)
     {
        sd->scale = scale;
        elm_widget_theme(obj);
     }
}

EOLIAN static double
_elm_widget_efl_ui_base_scale_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   // FIXME: save walking up the tree by storing/caching parent scale
   if (sd->scale == 0.0)
     {
        if (sd->parent_obj && elm_widget_is(sd->parent_obj))
          {
             return efl_ui_scale_get(sd->parent_obj);
          }
        else
          {
             return 1.0;
          }
     }
   return sd->scale;
}

EOLIAN static void
_elm_widget_theme_set(Eo *obj, Elm_Widget_Smart_Data *sd, Elm_Theme *th)
{
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

EOLIAN static void
_elm_widget_part_text_set(Eo *obj, Elm_Widget_Smart_Data *_pd EINA_UNUSED, const char *part, const char *label)
{
   if (evas_object_smart_type_check(obj, "elm_layout"))
     elm_layout_text_set(obj, part, label);
}

EOLIAN static const char*
_elm_widget_part_text_get(Eo *obj, Elm_Widget_Smart_Data *_pd EINA_UNUSED, const char *part)
{
   if (evas_object_smart_type_check(obj, "elm_layout"))
     return elm_layout_text_get(obj, part);

   return NULL;
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
   else if (ts && ((preset) || (!ts->preset)))
     {
        t = eina_inlist_remove(t, EINA_INLIST_GET(ts));
        eina_stringshare_del(ts->id);
        eina_stringshare_del(ts->domain);
        eina_stringshare_del(ts->string);
        ELM_SAFE_FREE(ts, free);
     }

   *translate_strings = t;

   return ts;
}

EOLIAN static void
_elm_widget_domain_translatable_part_text_set(Eo *obj, Elm_Widget_Smart_Data *sd, const char *part, const char *domain, const char *label)
{

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
   elm_obj_widget_part_text_set(obj, part, label);
   sd->on_translate = EINA_FALSE;
}

EOLIAN static const char*
_elm_widget_translatable_part_text_get(const Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd, const char *part)
{
   Elm_Translate_String_Data *ts;

   ts = _translate_string_data_get(sd->translate_strings, part);
   if (ts) return ts->string;
   else return NULL;
}

EOLIAN static void
_elm_widget_domain_part_text_translatable_set(Eo *obj, Elm_Widget_Smart_Data *sd, const char *part, const char *domain, Eina_Bool translatable)
{
   Elm_Translate_String_Data *ts;
   const char *text = NULL;

   ts = _part_text_translatable_set(&sd->translate_strings, part,
                                    translatable, EINA_TRUE);
   if (!ts) return;
   if (!ts->domain) ts->domain = eina_stringshare_add(domain);
   else eina_stringshare_replace(&ts->domain, domain);

   text = elm_obj_widget_part_text_get(obj, part);
   if (!text || !text[0]) return;

   if (!ts->string) ts->string = eina_stringshare_add(text);

//Try to translate text since we don't know the text is already translated.
#ifdef HAVE_GETTEXT
   text = dgettext(domain, text);
#endif
   sd->on_translate = EINA_TRUE;
   elm_obj_widget_part_text_set(obj, part, text);
   sd->on_translate = EINA_FALSE;
}

EAPI void
elm_widget_translate(Evas_Object *obj)
{
   ELM_WIDGET_CHECK(obj);
   elm_obj_widget_translate(obj);
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

EOLIAN static const char*
_elm_widget_part_text_translate(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd, const char *part, const char *text)
{


   if (!sd->translate_strings || sd->on_translate) return text;
   return _part_text_translate(sd->translate_strings, part, text);
}

EOLIAN static Eina_Bool
_elm_widget_translate(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *_pd EINA_UNUSED)
{
   const Eina_List *l;
   Evas_Object *child;
   API_ENTRY return EINA_FALSE;

   EINA_LIST_FOREACH(sd->subobjs, l, child)
     {
        if (elm_widget_is(child))
          elm_widget_translate(child);
     }

   if (sd->hover_obj) elm_widget_translate(sd->hover_obj);

#ifdef HAVE_GETTEXT
   Elm_Translate_String_Data *ts;
   EINA_INLIST_FOREACH(sd->translate_strings, ts)
     {
        if (!ts->string) continue;
        const char *s = dgettext(ts->domain, ts->string);
        sd->on_translate = EINA_TRUE;
        elm_obj_widget_part_text_set(obj, ts->id, s);
        sd->on_translate = EINA_FALSE;
     }
#endif
   efl_event_callback_legacy_call(obj, ELM_WIDGET_EVENT_LANGUAGE_CHANGED, NULL);
   return EINA_TRUE;
}

/**
 * @internal
 *
 * Resets the focus_move_policy mode from the system one
 * for widgets that are in automatic mode.
 *
 * @param obj The widget.
 *
 */
static void
_elm_widget_focus_move_policy_reload(Evas_Object *obj)
{
   API_ENTRY return;
   Elm_Focus_Move_Policy focus_move_policy = elm_config_focus_move_policy_get();

   if (elm_widget_focus_move_policy_automatic_get(obj) &&
       (sd->focus_move_policy != focus_move_policy))
     {
        sd->focus_move_policy = focus_move_policy;
     }
}

EOLIAN static void
_elm_widget_focus_reconfigure(Eo *obj, Elm_Widget_Smart_Data *_pd EINA_UNUSED)
{
   const Eina_List *l;
   Evas_Object *child;
   API_ENTRY return;

   EINA_LIST_FOREACH(sd->subobjs, l, child)
     {
        if (elm_widget_is(child))
          elm_widget_focus_reconfigure(child);
     }

   if (sd->hover_obj) elm_widget_focus_reconfigure(sd->hover_obj);

   _elm_widget_focus_move_policy_reload(obj);
}

EOLIAN static void
_elm_widget_access_info_set(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd, const char *txt)
{
   eina_stringshare_del(sd->access_info);
   if (!txt) sd->access_info = NULL;
   else sd->access_info = eina_stringshare_add(txt);
}

EOLIAN static const char*
_elm_widget_access_info_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   return sd->access_info;
}

EOLIAN static Elm_Theme*
_elm_widget_theme_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   if (!sd->theme)
     {
        if (sd->parent_obj && elm_widget_is(sd->parent_obj))
           return elm_widget_theme_get(sd->parent_obj);
        else return NULL;
     }
   return sd->theme;
}

EOLIAN static Elm_Theme_Apply
_elm_widget_style_set(Eo *obj, Elm_Widget_Smart_Data *sd, const char *style)
{
   if (eina_stringshare_replace(&sd->style, style))
      return elm_widget_theme(obj);

   return ELM_THEME_APPLY_SUCCESS;
}

EOLIAN static const char*
_elm_widget_style_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   const char *ret;
   ret = "default";
   if (sd->style) ret = sd->style;

   return ret;
}

EOLIAN static void
_elm_widget_tooltip_add(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd, Elm_Tooltip *tt)
{
   sd->tooltips = eina_list_append(sd->tooltips, tt);
}

EOLIAN static void
_elm_widget_tooltip_del(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd, Elm_Tooltip *tt)
{
   sd->tooltips = eina_list_remove(sd->tooltips, tt);
}

EAPI void
elm_widget_cursor_add(Eo *obj, Elm_Cursor *cur)
{
   Elm_Widget_Smart_Data *sd = efl_data_scope_safe_get(obj, MY_CLASS);
   if (!sd) return;

   sd->cursors = eina_list_append(sd->cursors, cur);
}

EAPI void
elm_widget_cursor_del(Eo *obj, Elm_Cursor *cur)
{
   Elm_Widget_Smart_Data *sd = efl_data_scope_safe_get(obj, MY_CLASS);
   if (!sd) return;

   sd->cursors = eina_list_remove(sd->cursors, cur);
}

EOLIAN static void
_elm_widget_drag_lock_x_set(Eo *obj, Elm_Widget_Smart_Data *sd, Eina_Bool lock)
{
   if (sd->drag_x_locked == lock) return;
   sd->drag_x_locked = lock;
   if (sd->drag_x_locked) _propagate_x_drag_lock(obj, 1);
   else _propagate_x_drag_lock(obj, -1);
}

EOLIAN static void
_elm_widget_drag_lock_y_set(Eo *obj, Elm_Widget_Smart_Data *sd, Eina_Bool lock)
{
   if (sd->drag_y_locked == lock) return;
   sd->drag_y_locked = lock;
   if (sd->drag_y_locked) _propagate_y_drag_lock(obj, 1);
   else _propagate_y_drag_lock(obj, -1);
}

EOLIAN static Eina_Bool
_elm_widget_drag_lock_x_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   return sd->drag_x_locked;
}

EOLIAN static Eina_Bool
_elm_widget_drag_lock_y_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   return sd->drag_y_locked;
}

EOLIAN static int
_elm_widget_drag_child_locked_x_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   return sd->child_drag_x_locked;
}

EOLIAN static int
_elm_widget_drag_child_locked_y_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   return sd->child_drag_y_locked;
}

EOLIAN static void
_elm_widget_item_loop_enabled_set(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd EINA_UNUSED, Eina_Bool enable EINA_UNUSED)
{
	return;
}

EOLIAN static Eina_Bool
_elm_widget_item_loop_enabled_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd EINA_UNUSED)
{
	return EINA_FALSE;
}

EOLIAN static Elm_Theme_Apply
_elm_widget_theme_object_set(Eo *obj, Elm_Widget_Smart_Data *sd, Evas_Object *edj, const char *wname, const char *welement, const char *wstyle)
{
   Elm_Theme_Apply ret = _elm_theme_object_set(obj, edj, wname, welement, wstyle);
   if (!ret)
     {
        return ELM_THEME_APPLY_FAILED;
     }

   if (sd->orient_mode != -1)
     {
        char buf[128];
        snprintf(buf, sizeof(buf), "elm,state,orient,%d", sd->orient_mode);
        elm_obj_widget_signal_emit(obj, buf, "elm");
     }

   return ret;
}

static void
_convert(Efl_Dbg_Info *info, Eina_Iterator *ptr_list)
{
   void *p;
   int i = 0;

   EINA_ITERATOR_FOREACH(ptr_list, p)
     {
        char name[100];

        snprintf(name, sizeof(name), "Candidate %d", i);

        EFL_DBG_INFO_APPEND(info, name, EINA_VALUE_TYPE_UINT64, p);
        i++;
     }

   eina_iterator_free(ptr_list);
}

EOLIAN static void
_elm_widget_efl_object_dbg_info_get(Eo *eo_obj, Elm_Widget_Smart_Data *_pd EINA_UNUSED, Efl_Dbg_Info *root)
{
   efl_dbg_info_get(efl_super(eo_obj, MY_CLASS), root);
   Efl_Ui_Focus_Relations *rel = NULL;
   Efl_Dbg_Info *focus, *group = EFL_DBG_INFO_LIST_APPEND(root, MY_CLASS_NAME);

   EFL_DBG_INFO_APPEND(group, "Wid-Type", EINA_VALUE_TYPE_STRING, elm_widget_type_get(eo_obj));
   EFL_DBG_INFO_APPEND(group, "Style", EINA_VALUE_TYPE_STRING, elm_widget_style_get(eo_obj));
   EFL_DBG_INFO_APPEND(group, "Layer", EINA_VALUE_TYPE_INT,
         (int) evas_object_layer_get(eo_obj));
   EFL_DBG_INFO_APPEND(group, "Scale", EINA_VALUE_TYPE_DOUBLE,
         evas_object_scale_get(eo_obj));
   EFL_DBG_INFO_APPEND(group, "Has focus", EINA_VALUE_TYPE_CHAR,
         elm_object_focus_get(eo_obj));
   EFL_DBG_INFO_APPEND(group, "Can focus", EINA_VALUE_TYPE_CHAR,
         elm_widget_can_focus_get(eo_obj));
   EFL_DBG_INFO_APPEND(group, "Disabled", EINA_VALUE_TYPE_CHAR,
         elm_widget_disabled_get(eo_obj));
   EFL_DBG_INFO_APPEND(group, "Mirrored", EINA_VALUE_TYPE_CHAR,
         efl_ui_mirrored_get(eo_obj));
   EFL_DBG_INFO_APPEND(group, "Tree Unfocusable", EINA_VALUE_TYPE_CHAR,
         elm_widget_tree_unfocusable_get(eo_obj));
   EFL_DBG_INFO_APPEND(group, "Automatic mirroring", EINA_VALUE_TYPE_CHAR,
         efl_ui_mirrored_automatic_get(eo_obj));

   rel = efl_ui_focus_manager_fetch(_pd->focus.manager, eo_obj);
   if (rel)
     {
        focus = EFL_DBG_INFO_LIST_APPEND(group, "Focus");

        EFL_DBG_INFO_APPEND(focus, "type", EINA_VALUE_TYPE_STRING, rel->type);
        EFL_DBG_INFO_APPEND(focus, "manager", EINA_VALUE_TYPE_UINT64, _pd->focus.manager);
        EFL_DBG_INFO_APPEND(focus, "parent", EINA_VALUE_TYPE_UINT64, rel->parent);
        EFL_DBG_INFO_APPEND(focus, "next", EINA_VALUE_TYPE_UINT64 , rel->next);
        EFL_DBG_INFO_APPEND(focus, "prev", EINA_VALUE_TYPE_UINT64 , rel->prev);

        EFL_DBG_INFO_APPEND(focus, "redirect", EINA_VALUE_TYPE_UINT64 , rel->redirect);

#define ADD_PTR_LIST(name) \
        Efl_Dbg_Info* name = EFL_DBG_INFO_LIST_APPEND(focus, ""#name""); \
        _convert(name, eina_list_iterator_new(rel->name));

        ADD_PTR_LIST(top)
        ADD_PTR_LIST(down)
        ADD_PTR_LIST(right)
        ADD_PTR_LIST(left)

#undef ADD_PTR_LIST

     }

   //if thats a focus manager, give useful information like the border elements
   if (efl_isa(eo_obj, EFL_UI_FOCUS_MANAGER_CLASS))
     {
        Efl_Dbg_Info *border;

        focus = EFL_DBG_INFO_LIST_APPEND(group, "Focus Manager");
        border = EFL_DBG_INFO_LIST_APPEND(focus, "Border Elements");

        _convert(border,
          efl_ui_focus_manager_border_elements_get(eo_obj)
        );

        EFL_DBG_INFO_APPEND(focus, "redirect", EINA_VALUE_TYPE_UINT64,
          efl_ui_focus_manager_redirect_get(eo_obj));
     }
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

   return efl_class_name_get(efl_class_get(obj));
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

EOLIAN static Evas_Object*
_elm_widget_name_find(const Eo *obj, Elm_Widget_Smart_Data *_pd EINA_UNUSED, const char *name, int recurse)
{
   if (!name) return NULL;
   return _widget_name_find(obj, name, recurse);
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

EOLIAN static void
_elm_widget_focus_hide_handle(Eo *obj, Elm_Widget_Smart_Data *_pd EINA_UNUSED)
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

   elm_obj_widget_focus_mouse_up_handle(o);
}

EOLIAN static void
_elm_widget_focus_mouse_up_handle(Eo *obj, Elm_Widget_Smart_Data *pd)
{
   if (!obj) return;
   if (!_is_focusable(obj)) return;

   elm_widget_focus_steal(obj, NULL);

   if (pd->focus.manager && !pd->focus.logical)
     {
        Efl_Ui_Focus_Manager *m, *m2 = obj, *old = NULL;

        /*
         * The object we have clicked could be registered in a submanager.
         * This means we need to look as long as possible to higher redirect managers.
         * And set them to the redirect manager.
         */

        m = elm_widget_top_get(obj);
        m2 = efl_ui_focus_user_manager_get(obj);

        if (m2 != m)
          {
            //first unset all redirect properties
            old = m;
            do
              {
                 Efl_Ui_Focus_Manager *tmp;
                 tmp = efl_ui_focus_manager_redirect_get(old);
                 if (tmp)
                   efl_ui_focus_manager_redirect_set(old, NULL);
                 old = tmp;
              }
            while(old);
            //now set the redirect path to the new object
            do
              {
                Efl_Ui_Focus_Manager *new_manager;;

                new_manager = efl_ui_focus_user_manager_get(m2);
                //new manager is in a higher hirarchy than m2
                //so we set m2 as redirect in new_manager
                efl_ui_focus_manager_redirect_set(new_manager, m2);
                m2 = new_manager;
              }
            while(m && m2 && m != m2);
          }


        efl_ui_focus_manager_focus(pd->focus.manager, obj);
     }
}

EOLIAN static void
_elm_widget_focus_tree_unfocusable_handle(Eo *obj, Elm_Widget_Smart_Data *_pd EINA_UNUSED)
{
   if (!elm_widget_parent_get(obj))
     elm_widget_focused_object_clear(obj);
   else
     _if_focused_revert(obj, EINA_TRUE);
}

EOLIAN static void
_elm_widget_focus_disabled_handle(Eo *obj, Elm_Widget_Smart_Data *_pd EINA_UNUSED)
{
   elm_widget_focus_tree_unfocusable_handle(obj);
}

EOLIAN static unsigned int
_elm_widget_focus_order_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   return sd->focus_order;
}

EOLIAN static Evas_Object*
_elm_widget_newest_focus_order_get(const Eo *obj, Elm_Widget_Smart_Data *sd, unsigned int *newest_focus_order, Eina_Bool can_focus_only)
{
   const Eina_List *l;
   Evas_Object *child, *cur, *best;

   if (!evas_object_visible_get(obj)
       || (elm_widget_disabled_get(obj))
       || (elm_widget_tree_unfocusable_get(obj)))
     return NULL;

   best = NULL;
   if (*newest_focus_order < sd->focus_order)
     {
        if (!can_focus_only || elm_widget_can_focus_get(obj))
          {
             *newest_focus_order = sd->focus_order;
             best = (Evas_Object *)obj;
          }
     }
   EINA_LIST_FOREACH(sd->subobjs, l, child)
     {
        if (!_elm_widget_is(child)) continue;

        cur = elm_widget_newest_focus_order_get
           (child, newest_focus_order, can_focus_only);
        if (!cur) continue;
        best = cur;
     }
   return best;
}

/*
 * @internal
 *
 * Get the focus highlight geometry of a widget.
 *
 * @param obj Widget object for the focus highlight
 * @param x Focus highlight x coordinate
 * @param y Focus highlight y coordinate
 * @param w Focus highlight object width
 * @param h Focus highlight object height
 * @param is_next @c EINA_TRUE if this request is for the new focused object,
 * @c EINA_FALSE if this request is for the previously focused object. This
 * information becomes important when the focus highlight is changed inside one
 * widget.
 *
 * @ingroup Widget
 */
/*
 * @internal
 *
 * Get the 'focus_part' geometry if there is any
 *
 * This queries if there is a 'focus_part' request from the edc style. If edc
 * style offers 'focus_part' edje data item, this function requests for the
 * geometry of a specific part which is described in 'focus_part' edje data.
 *
 * @param obj Widget object for the focus highlight
 * @param x Focus highlight x coordinate
 * @param y Focus highlight y coordinate
 * @param w Focus highlight object width
 * @param h Focus highlight object height
 *
 * x, y, w, h already contain the object's geometry. If there is a 'focus_part'
 * support, these values will be updated accordingly or the values will be
 * remained as they were.
 *
 * @ingroup Widget
 */
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

   if (obj && efl_isa(obj, EDJE_OBJECT_CLASS))
     {
        edje_obj = obj;
        if (!(target_hl_part = edje_object_data_get(edje_obj, "focus_part")))
          return;
     }
   else if (obj && efl_isa(obj, ELM_LAYOUT_CLASS))
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

EOLIAN static void
_elm_widget_focus_highlight_geometry_get(const Eo *obj, Elm_Widget_Smart_Data *sd, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Evas_Coord ox = 0, oy = 0, ow = 0, oh = 0;
   Evas_Object *scroller = (Evas_Object *)obj;

   evas_object_geometry_get(obj, x, y, w, h);
   elm_widget_focus_highlight_focus_part_geometry_get(sd->resize_obj, x, y, w, h);

   if (_elm_config->focus_autoscroll_mode != ELM_FOCUS_AUTOSCROLL_MODE_BRING_IN)
     return;

   while (scroller)
     {
        if (_elm_scrollable_is(scroller))
          {
             elm_interface_scrollable_content_viewport_geometry_get(scroller, &ox, &oy, &ow, &oh);

             if (*y < oy)
               *y = oy;
             else if ((oy + oh) < (*y + *h))
               *y = (oy + oh - *h);
             else if (*x < ox)
               *x = ox;
             else if ((ox + ow) < (*x + *w))
               *x = (ox + ow - *w);

             break;
          }
        scroller = elm_widget_parent_get(scroller);
     }
}

EOLIAN static Elm_Object_Item*
_elm_widget_focused_item_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *_pd EINA_UNUSED)
{
   return NULL;
}

EOLIAN static void
_elm_widget_focus_region_show_mode_set(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *_pd, Elm_Focus_Region_Show_Mode mode)
{
   _pd->focus_region_show_mode = mode;
}

EOLIAN static Elm_Focus_Region_Show_Mode
_elm_widget_focus_region_show_mode_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *_pd)
{
   return _pd->focus_region_show_mode;
}

EAPI void
elm_widget_activate(Evas_Object *obj, Elm_Activate act)
{
   Evas_Object *parent;
   Eina_Bool ret;

   ELM_WIDGET_CHECK(obj);

   ret = EINA_FALSE;

   ret = elm_obj_widget_activate(obj, act);

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
/* Legacy only */
EAPI void
elm_widget_display_mode_set(Evas_Object *obj, Evas_Display_Mode dispmode)
{
   Evas_Display_Mode prev_dispmode;
   Evas_Object *child;
   Eina_List *l;

   API_ENTRY return;
   prev_dispmode = evas_object_size_hint_display_mode_get(obj);

   if ((prev_dispmode == dispmode) ||
       (prev_dispmode == EVAS_DISPLAY_MODE_DONT_CHANGE)) return;

   evas_object_size_hint_display_mode_set(obj, dispmode);

   EINA_LIST_FOREACH (sd->subobjs, l, child)
     {
        if (elm_widget_is(child))
          elm_widget_display_mode_set(child, dispmode);
     }
}

EOLIAN static void
_elm_widget_orientation_mode_disabled_set(Eo *obj, Elm_Widget_Smart_Data *sd, Eina_Bool disabled)
{
   int orient_mode = -1;

   if (!disabled)
     {
        //Get current orient mode from it's parent otherwise, 0.
        sd->orient_mode = 0;
        ELM_WIDGET_DATA_GET(sd->parent_obj, sd_parent);
        if (!sd_parent) orient_mode = 0;
        else orient_mode = sd_parent->orient_mode;
     }
   elm_obj_widget_orientation_set(obj, orient_mode);
}

EOLIAN static Eina_Bool
_elm_widget_orientation_mode_disabled_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   if (sd->orient_mode == -1) return EINA_TRUE;
   else return EINA_FALSE;
}

EOLIAN static void
_elm_widget_orientation_set(Eo *obj, Elm_Widget_Smart_Data *sd, int orient_mode)
{
   Evas_Object *child;
   Eina_List *l;

   sd->orient_mode = orient_mode;

   EINA_LIST_FOREACH (sd->subobjs, l, child)
     {
        if (elm_widget_is(child))
          elm_obj_widget_orientation_set(child, orient_mode);
     }

   if (orient_mode != -1)
     {
        char buf[128];
        snprintf(buf, sizeof(buf), "elm,state,orient,%d", orient_mode);
        elm_obj_widget_signal_emit(obj, buf, "elm");
     }
}

/**
 * @internal
 *
 * Returns the widget's focus move policy.
 *
 * @param obj The widget.
 * @return focus move policy of the object.
 *
 **/
EOLIAN static Elm_Focus_Move_Policy
_elm_widget_focus_move_policy_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   return sd->focus_move_policy;
}

/**
 * @internal
 *
 * Sets the widget's focus move policy.
 *
 * @param obj The widget.
 * @param policy Elm_Focus_Move_Policy to set object's focus move policy.
 */

EOLIAN static void
_elm_widget_focus_move_policy_set(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd, Elm_Focus_Move_Policy policy)
{
   if (sd->focus_move_policy == policy) return;
   sd->focus_move_policy = policy;
}

/**
 * Returns the widget's focus_move_policy mode setting.
 *
 * @param obj The widget.
 * @return focus_move_policy mode setting of the object.
 *
 **/
EOLIAN static Eina_Bool
_elm_widget_focus_move_policy_automatic_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *sd)
{
   return sd->focus_move_policy_auto_mode;
}

/**
 * @internal
 *
 * Sets the widget's focus_move_policy mode setting.
 * When widget in automatic mode, it follows the system focus_move_policy mode set by
 * elm_config_focus_move_policy_set().
 * @param obj The widget.
 * @param automatic EINA_TRUE for auto focus_move_policy mode. EINA_FALSE for manual.
 */
EOLIAN static void
_elm_widget_focus_move_policy_automatic_set(Eo *obj, Elm_Widget_Smart_Data *sd, Eina_Bool automatic)
{
   if (sd->focus_move_policy_auto_mode != automatic)
     {
        sd->focus_move_policy_auto_mode = automatic;

        if (automatic)
          {
             elm_widget_focus_move_policy_set(obj, elm_config_focus_move_policy_get());
          }
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
_track_obj_view_update(void *data, const Efl_Event *event)
{
   Elm_Widget_Item_Data *item = data;
   _track_obj_update(item->track_obj, event->object);
}

static void
_track_obj_view_del(void *data, const Efl_Event *event);

EFL_CALLBACKS_ARRAY_DEFINE(tracker_callbacks,
                          { EFL_GFX_EVENT_RESIZE, _track_obj_view_update },
                          { EFL_GFX_EVENT_MOVE, _track_obj_view_update },
                          { EFL_GFX_EVENT_SHOW, _track_obj_view_update },
                          { EFL_GFX_EVENT_HIDE, _track_obj_view_update },
                          { EFL_EVENT_DEL, _track_obj_view_del });

static void
_track_obj_view_del(void *data, const Efl_Event *event EINA_UNUSED)
{
   Elm_Widget_Item_Data *item = data;

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
   Elm_Widget_Item_Data *item = data;
   item->track_obj = NULL;

   if (!item->view) return;

   efl_event_callback_array_del(item->view, tracker_callbacks(), item);
}

static void
_elm_widget_item_signal_cb(void *data, Evas_Object *obj EINA_UNUSED, const char *emission,
                           const char *source)
{
   Elm_Widget_Item_Signal_Data *wisd = data;
   wisd->func(wisd->data, wisd->item, emission, source);
}

static void *
_elm_widget_item_signal_callback_list_get(Elm_Widget_Item_Data *item, Eina_List *position)
{
   Elm_Widget_Item_Signal_Data *wisd = eina_list_data_get(position);
   void *data;

   item->signals = eina_list_remove_list(item->signals, position);
   data = wisd->data;

   if (_elm_widget_is(item->view))
     elm_object_signal_callback_del(item->view,
                                    wisd->emission, wisd->source,
                                    _elm_widget_item_signal_cb);
   else if (efl_isa(item->view, EDJE_OBJECT_CLASS))
     edje_object_signal_callback_del_full(item->view,
                                          wisd->emission, wisd->source,
                                          _elm_widget_item_signal_cb, wisd);

   eina_stringshare_del(wisd->emission);
   eina_stringshare_del(wisd->source);
   free(wisd);

   return data;
}

#define ERR_NOT_SUPPORTED(item, method)  ERR("%s does not support %s API.", elm_widget_type_get(item->widget), method);

static void
_efl_del_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Elm_Widget_Item_Data *item = efl_data_scope_get(event->object, ELM_WIDGET_ITEM_CLASS);
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   if (item->del_func)
      item->del_func((void *) WIDGET_ITEM_DATA_GET(event->object), item->widget, item->eo_obj);
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
EOLIAN static Eo *
_elm_widget_item_efl_object_constructor(Eo *eo_item, Elm_Widget_Item_Data *item)
{
   Evas_Object *widget;
   widget = efl_parent_get(eo_item);

   if (!_elm_widget_is(widget))
     {
        ERR("Failed");
        return NULL;
     }

   eo_item = efl_constructor(efl_super(eo_item, ELM_WIDGET_ITEM_CLASS));

   EINA_MAGIC_SET(item, ELM_WIDGET_ITEM_MAGIC);

   item->widget = widget;
   item->eo_obj = eo_item;
   efl_event_callback_add(eo_item, EFL_EVENT_DEL, _efl_del_cb, NULL);

   return eo_item;
}

EOLIAN static void
_elm_widget_item_efl_object_destructor(Eo *eo_item, Elm_Widget_Item_Data *item)
{
   Elm_Translate_String_Data *ts;

   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);

   evas_object_del(item->view);

   eina_stringshare_del(item->access_info);
   eina_stringshare_del(item->accessible_name);

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

   elm_interface_atspi_accessible_removed(eo_item);

   EINA_MAGIC_SET(item, EINA_MAGIC_NONE);

   efl_destructor(efl_super(eo_item, ELM_WIDGET_ITEM_CLASS));
}

/**
 * @internal
 *
 * Releases widget item memory, calling back item_del_pre_hook() and
 * item_del_cb() if they exist.
 *
 * @param item a valid #Elm_Widget_Item to be deleted.
 *
 * If there is an Elm_Widget_Item::del_cb, then it will be called prior
 * to memory release. Note that elm_widget_item_pre_notify_del() calls
 * this function and then unset it, thus being useful for 2 step
 * cleanup whenever the del_cb may use any of the data that must be
 * deleted from item.
 *
 * The Elm_Widget_Item::view will be deleted (evas_object_del()) if it
 * is presented!
 *
 * Note that if item_del_pre_hook() returns @c EINA_TRUE, item free will be
 * deferred, or item will be freed here if it returns @c EINA_FALSE.
 *
 * @see elm_widget_item_del() convenience macro.
 * @ingroup Widget
 */
EOLIAN static void
_elm_widget_item_del(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   item->on_deletion = EINA_TRUE;

   //Widget item delete callback
   Eina_Bool del_ok;
   del_ok = elm_wdg_item_del_pre(item->eo_obj);
   if (del_ok)
      efl_del(item->eo_obj);
   return;
}

EOLIAN static Eina_Bool
_elm_widget_item_del_pre(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item EINA_UNUSED)
{
   return EINA_TRUE;
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
EOLIAN static void
_elm_widget_item_pre_notify_del(Eo *eo_item, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   if (!item->del_func) return;
   item->del_func((void *)WIDGET_ITEM_DATA_GET(eo_item), item->widget, item->eo_obj);
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
EOLIAN static void
_elm_widget_item_del_cb_set(Eo *eo_item EINA_UNUSED,
                            Elm_Widget_Item_Data *item,
                            Evas_Smart_Cb func)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   if ((item->del_func) && (item->del_func != func))
     WRN("You're replacing a previously set del_cb %p of item %p with %p",
         item->del_func, item->eo_obj, func);

   item->del_func = func;
}

/**
 * @internal
 *
 * Get owner widget of this item.
 *
 * @param item a valid #Elm_Widget_Item to get data from.
 * @return owner widget of this item.
 * @ingroup Widget
 */
EOLIAN static Evas_Object *
_elm_widget_item_widget_get(const Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item, NULL);

   return item->widget;
}

EAPI Eina_Bool
_elm_widget_onscreen_is(Evas_Object *widget)
{
   Evas_Object *parent = widget;
   Eina_Rectangle r1, r2;

   Evas *evas = evas_object_evas_get(widget);
   if (!evas) return EINA_FALSE;

   evas_object_geometry_get(widget, &r1.x, &r1.y, &r1.w, &r1.h);
   if (eina_rectangle_is_empty(&r1))
     return EINA_FALSE;

   // check if on canvas
   evas_output_viewport_get(evas, &r2.x, &r2.y, &r2.w, &r2.h);
   if (!eina_rectangles_intersect(&r1, &r2))
     return EINA_FALSE;

   // check if inside scrollable parent viewport
   do {
      parent = elm_widget_parent_get(parent);
      if (parent && !evas_object_visible_get(parent))
        return EINA_FALSE;
      if (parent && efl_isa(parent, ELM_INTERFACE_SCROLLABLE_MIXIN))
        {
           evas_object_geometry_get(parent, &r2.x, &r2.y, &r2.w, &r2.h);
           if (!eina_rectangles_intersect(&r1, &r2))
             return EINA_FALSE;
        }
   } while (parent && (parent != elm_widget_top_get(widget)));

   return EINA_TRUE;
}

EAPI Eina_Bool
_elm_widget_item_onscreen_is(Elm_Object_Item *item)
{
   Eina_Rectangle r1, r2;
   Elm_Widget_Item_Data *id = efl_data_scope_get(item, ELM_WIDGET_ITEM_CLASS);
   if (!id || !id->view) return EINA_FALSE;

   if (!evas_object_visible_get(id->view))
     return EINA_FALSE;

   if (!_elm_widget_onscreen_is(id->widget))
     return EINA_FALSE;

   evas_object_geometry_get(id->view, &r1.x, &r1.y, &r1.w, &r1.h);
   if (eina_rectangle_is_empty(&r1))
     return EINA_FALSE;

   evas_object_geometry_get(id->widget, &r2.x, &r2.y, &r2.w, &r2.h);
   if (!eina_rectangles_intersect(&r1, &r2))
     return EINA_FALSE;

   return EINA_TRUE;
}

EOLIAN static Elm_Atspi_State_Set
_elm_widget_item_elm_interface_atspi_accessible_state_set_get(Eo *eo_item,
                                                              Elm_Widget_Item_Data *item EINA_UNUSED)
{
   Elm_Atspi_State_Set states = 0;

   STATE_TYPE_SET(states, ELM_ATSPI_STATE_FOCUSABLE);

   if (elm_object_item_focus_get(eo_item))
     STATE_TYPE_SET(states, ELM_ATSPI_STATE_FOCUSED);
   if (!elm_object_item_disabled_get(eo_item))
     {
        STATE_TYPE_SET(states, ELM_ATSPI_STATE_ENABLED);
        STATE_TYPE_SET(states, ELM_ATSPI_STATE_SENSITIVE);
        STATE_TYPE_SET(states, ELM_ATSPI_STATE_VISIBLE);
     }
   if (_elm_widget_item_onscreen_is(eo_item))
     STATE_TYPE_SET(states, ELM_ATSPI_STATE_SHOWING);

   return states;
}

EAPI void
elm_object_item_data_set(Elm_Object_Item *it, void *data)
{
   WIDGET_ITEM_DATA_SET(it, data);
}

EAPI void *
elm_object_item_data_get(const Elm_Object_Item *it)
{
   return (void *) WIDGET_ITEM_DATA_GET(it);
}

EOLIAN static void
_elm_widget_item_disabled_set(Eo *eo_item EINA_UNUSED,
                              Elm_Widget_Item_Data *item,
                              Eina_Bool disabled)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   if (item->disabled == disabled) return;
   item->disabled = !!disabled;
   elm_wdg_item_disable(item->eo_obj);
}

EOLIAN static Eina_Bool
_elm_widget_item_disabled_get(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, EINA_FALSE);
   return item->disabled;
}

EOLIAN static void
_elm_widget_item_style_set(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item, const char *style EINA_UNUSED)
{
   ERR_NOT_SUPPORTED(item, "elm_object_style_set()");
}

EOLIAN static const char *
_elm_widget_item_style_get(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ERR_NOT_SUPPORTED(item, "elm_object_style_get()");
   return NULL;
}

EOLIAN static void
_elm_widget_item_disable(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item EINA_UNUSED)
{
}

EOLIAN static void
_elm_widget_item_focus_set(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item, Eina_Bool focused EINA_UNUSED)
{
   ERR_NOT_SUPPORTED(item, "elm_object_item_focus_set");
}

EOLIAN static Eina_Bool
_elm_widget_item_focus_get(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ERR_NOT_SUPPORTED(item, "elm_object_item_focus_get");
   return EINA_FALSE;
}

EOLIAN static void
_elm_widget_item_domain_translatable_part_text_set(Eo *eo_item EINA_UNUSED,
                                                   Elm_Widget_Item_Data *item,
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
   elm_wdg_item_part_text_set(item->eo_obj, part, label);
   item->on_translate = EINA_FALSE;
}

EOLIAN static const char *
_elm_widget_item_translatable_part_text_get(const Eo *eo_item EINA_UNUSED,
                                            Elm_Widget_Item_Data *item,
                                            const char *part)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item, NULL);

   Elm_Translate_String_Data *ts;
   ts = _translate_string_data_get(item->translate_strings, part);
   if (ts) return ts->string;
   return NULL;
}

EOLIAN static void
_elm_widget_item_domain_part_text_translatable_set(Eo *eo_item EINA_UNUSED,
                                                   Elm_Widget_Item_Data *item,
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

   text = elm_wdg_item_part_text_get(item->eo_obj, part);

   if (!text || !text[0]) return;

   if (!ts->string) ts->string = eina_stringshare_add(text);

//Try to translate text since we don't know the text is already translated.
#ifdef HAVE_GETTEXT
   text = dgettext(domain, text);
#endif
   item->on_translate = EINA_TRUE;
   elm_wdg_item_part_text_set(item->eo_obj, part, text);
   item->on_translate = EINA_FALSE;
}

EOLIAN static void
_elm_widget_item_track_cancel(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   if (!item->track_obj) return;

   while (evas_object_ref_get(item->track_obj) > 0)
     evas_object_unref(item->track_obj);

   evas_object_del(item->track_obj);
}

EOLIAN static Evas_Object *
_elm_widget_item_track(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
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

   efl_event_callback_array_add(item->view, tracker_callbacks(), item);

   evas_object_ref(track);

   item->track_obj = track;

   return track;
}

EOLIAN static void
_elm_widget_item_untrack(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   if (!item->track_obj) return;
   evas_object_unref(item->track_obj);

   if (evas_object_ref_get(item->track_obj) == 0)
     evas_object_del(item->track_obj);
}

EOLIAN static int
_elm_widget_item_track_get(const Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, 0);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item, 0);

   if (!item->track_obj) return 0;
   return evas_object_ref_get(item->track_obj);
}

typedef struct _Elm_Widget_Item_Tooltip Elm_Widget_Item_Tooltip;

struct _Elm_Widget_Item_Tooltip
{
   Elm_Widget_Item_Data       *item;
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
EOLIAN static void
_elm_widget_item_tooltip_text_set(Eo *eo_item EINA_UNUSED,
                                  Elm_Widget_Item_Data *item EINA_UNUSED,
                                  const char *text)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   EINA_SAFETY_ON_NULL_RETURN(text);

   text = eina_stringshare_add(text);
   elm_wdg_item_tooltip_content_cb_set(item->eo_obj, _elm_widget_item_tooltip_label_create, text, _elm_widget_item_tooltip_label_del_cb);
}

EOLIAN static void
_elm_widget_item_tooltip_translatable_text_set(Eo *eo_item EINA_UNUSED,
                                               Elm_Widget_Item_Data *item EINA_UNUSED,
                                               const char *text)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   EINA_SAFETY_ON_NULL_RETURN(text);

   text = eina_stringshare_add(text);
   elm_wdg_item_tooltip_content_cb_set(item->eo_obj, _elm_widget_item_tooltip_trans_label_create, text, _elm_widget_item_tooltip_label_del_cb);
}

static Evas_Object *
_elm_widget_item_tooltip_create(void *data,
                                Evas_Object *obj,
                                Evas_Object *tooltip)
{
   Elm_Widget_Item_Tooltip *wit = data;
   return wit->func((void *)wit->data, obj, tooltip, wit->item->eo_obj);
}

static void
_elm_widget_item_tooltip_del_cb(void *data,
                                Evas_Object *obj,
                                void *event_info EINA_UNUSED)
{
   Elm_Widget_Item_Tooltip *wit = data;
   if (wit->del_cb) wit->del_cb((void *)wit->data, obj, wit->item->eo_obj);
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
EOLIAN static void
_elm_widget_item_tooltip_content_cb_set(Eo *eo_item EINA_UNUSED,
                                        Elm_Widget_Item_Data *item,
                                        Elm_Tooltip_Item_Content_Cb func,
                                        const void *data,
                                        Evas_Smart_Cb del_cb)
{
   Elm_Widget_Item_Tooltip *wit;

   ELM_WIDGET_ITEM_CHECK_OR_GOTO(item, error_noitem);
   //ELM_WIDGET_ITEM_RETURN_IF_GOTO(item, error_noitem);

   if (!func)
     {
        elm_wdg_item_tooltip_unset(item->eo_obj);
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
EOLIAN static void
_elm_widget_item_tooltip_unset(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
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
EOLIAN static void
_elm_widget_item_tooltip_style_set(Eo *eo_item EINA_UNUSED,
                                   Elm_Widget_Item_Data *item,
                                   const char *style)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   elm_object_tooltip_style_set(item->view, style);
}

EOLIAN static Eina_Bool
_elm_widget_item_tooltip_window_mode_set(Eo *eo_item EINA_UNUSED,
                                         Elm_Widget_Item_Data *item,
                                         Eina_Bool disable)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, EINA_FALSE);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item, EINA_FALSE);

   return elm_object_tooltip_window_mode_set(item->view, disable);
}

EOLIAN static Eina_Bool
_elm_widget_item_tooltip_window_mode_get(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
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
EOLIAN static const char *
_elm_widget_item_tooltip_style_get(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);

   return elm_object_tooltip_style_get(item->view);
}

EOLIAN static void
_elm_widget_item_cursor_set(Eo *eo_item EINA_UNUSED,
                            Elm_Widget_Item_Data *item,
                            const char *cursor)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   elm_object_sub_cursor_set(item->view, item->widget, cursor);
}

EOLIAN static const char *
_elm_widget_item_cursor_get(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);
   return elm_object_cursor_get(item->view);
}

EOLIAN static void
_elm_widget_item_cursor_unset(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
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
EOLIAN static void
_elm_widget_item_cursor_style_set(Eo *eo_item EINA_UNUSED,
                                  Elm_Widget_Item_Data *item,
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
EOLIAN static const char *
_elm_widget_item_cursor_style_get(Eo *eo_item EINA_UNUSED,
                                  Elm_Widget_Item_Data *item)
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
EOLIAN static void
_elm_widget_item_cursor_engine_only_set(Eo *eo_item EINA_UNUSED,
                                        Elm_Widget_Item_Data *item,
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
EOLIAN static Eina_Bool
_elm_widget_item_cursor_engine_only_get(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, EINA_FALSE);
   return !elm_object_cursor_theme_search_enabled_get(item->view);
}

EOLIAN static void
_elm_widget_item_part_content_set(Eo *eo_item EINA_UNUSED,
                                  Elm_Widget_Item_Data *item,
                                  const char *part EINA_UNUSED,
                                  Evas_Object *content EINA_UNUSED)
{
   ERR_NOT_SUPPORTED(item, "elm_object_part_content_set()");
}

EOLIAN static Evas_Object *
_elm_widget_item_part_content_get(Eo *eo_item EINA_UNUSED,
                                  Elm_Widget_Item_Data *item,
                                  const char *part EINA_UNUSED)
{
   ERR_NOT_SUPPORTED(item, "elm_object_part_content_get()");
   return NULL;
}

EOLIAN static Evas_Object *
_elm_widget_item_part_content_unset(Eo *eo_item EINA_UNUSED,
                                    Elm_Widget_Item_Data *item,
                                    const char *part EINA_UNUSED)
{
   ERR_NOT_SUPPORTED(item, "elm_object_part_content_unset()");
   return NULL;
}

EOLIAN static void
_elm_widget_item_part_text_set(Eo *eo_item EINA_UNUSED,
                               Elm_Widget_Item_Data *item,
                               const char *part EINA_UNUSED,
                               const char *label EINA_UNUSED)
{
   ERR_NOT_SUPPORTED(item, "elm_object_part_text_set()");
}

EOLIAN static const char *
_elm_widget_item_part_text_get(Eo *eo_item EINA_UNUSED,
                               Elm_Widget_Item_Data *item,
                               const char *part EINA_UNUSED)
{
   ERR_NOT_SUPPORTED(item, "elm_object_part_text_get()");
   return NULL;
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

EOLIAN static void
_elm_widget_item_part_text_custom_set(Eo *eo_item EINA_UNUSED,
                                      Elm_Widget_Item_Data *item,
                                      const char *part,
                                      const char *text)
{
   Elm_Label_Data *label;
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   if (!item->labels)
     item->labels =
        eina_hash_stringshared_new(_elm_widget_item_part_text_custom_free);
   label = eina_hash_find(item->labels, part);
   if (!label)
     {
        label = malloc(sizeof(Elm_Label_Data));
        if (!label)
          {
             ERR("Failed to allocate memory");
             return;
          }
        label->part = eina_stringshare_add(part);
        label->text = eina_stringshare_add(text);
        eina_hash_add(item->labels, part, label);
     }
   else
     eina_stringshare_replace(&label->text, text);
}

EOLIAN static const char *
_elm_widget_item_part_text_custom_get(Eo *eo_item EINA_UNUSED,
                                      Elm_Widget_Item_Data *item,
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
   Elm_Widget_Item_Data *item;
   label = data;
   item = func_data;

   elm_wdg_item_part_text_set(item->eo_obj, label->part, label->text);

   return EINA_TRUE;
}

EOLIAN static void
_elm_widget_item_part_text_custom_update(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   if (item->labels)
     eina_hash_foreach(item->labels,
                       _elm_widget_item_part_text_custom_foreach, item);
}

EOLIAN static void
_elm_widget_item_signal_emit(Eo *eo_item EINA_UNUSED,
                             Elm_Widget_Item_Data *item EINA_UNUSED,
                             const char *emission EINA_UNUSED,
                             const char *source EINA_UNUSED)
{

}

EOLIAN static void
_elm_widget_item_signal_callback_add(Eo *eo_item,
                                     Elm_Widget_Item_Data *item,
                                     const char *emission,
                                     const char *source,
                                     Elm_Object_Item_Signal_Cb func,
                                     void *data)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);
   EINA_SAFETY_ON_NULL_RETURN(func);

   Elm_Widget_Item_Signal_Data *wisd;

   wisd = malloc(sizeof(Elm_Widget_Item_Signal_Data));
   if (!wisd) return;

   wisd->item = eo_item;
   wisd->func = (Elm_Widget_Item_Signal_Cb)func;
   wisd->data = data;
   wisd->emission = eina_stringshare_add(emission);
   wisd->source = eina_stringshare_add(source);

   if (_elm_widget_is(item->view))
     elm_object_signal_callback_add(item->view, emission, source, _elm_widget_item_signal_cb, wisd);
   else if (efl_isa(item->view, EDJE_OBJECT_CLASS))
     edje_object_signal_callback_add(item->view, emission, source, _elm_widget_item_signal_cb, wisd);
   else
     {
        WRN("The %s widget item doesn't support signal callback add!",
            efl_class_name_get(efl_class_get(item->widget)));
        free(wisd);
        return;
     }

   item->signals = eina_list_append(item->signals, wisd);
}

EOLIAN static void *
_elm_widget_item_signal_callback_del(Eo *eo_item EINA_UNUSED,
                                     Elm_Widget_Item_Data *item,
                                     const char *emission,
                                     const char *source,
                                     Elm_Object_Item_Signal_Cb func)
{
   Elm_Widget_Item_Signal_Data *wisd;
   Eina_List *l;

   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(func, NULL);

   EINA_LIST_FOREACH(item->signals, l, wisd)
     {
        if ((wisd->func == (Elm_Widget_Item_Signal_Cb)func) &&
            !strcmp(wisd->emission, emission) &&
            !strcmp(wisd->source, source))
          return _elm_widget_item_signal_callback_list_get(item, l);
     }

   return NULL;
}

EOLIAN static void
_elm_widget_item_access_info_set(Eo *eo_item EINA_UNUSED,
                                 Elm_Widget_Item_Data *item,
                                 const char *txt)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   ELM_WIDGET_ITEM_RETURN_IF_ONDEL(item);

   eina_stringshare_del(item->access_info);
   if (!txt) item->access_info = NULL;
   else item->access_info = eina_stringshare_add(txt);
}

EOLIAN static void
_elm_widget_item_translate(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
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
        elm_wdg_item_part_text_set(item->eo_obj, ts->id, s);
        item->on_translate = EINA_FALSE;
     }
#endif
}

EOLIAN static void
_elm_widget_item_access_order_set(Eo *eo_item EINA_UNUSED,
                                  Elm_Widget_Item_Data *item,
                                  Eina_List *objs)
{
   _elm_access_widget_item_access_order_set(item, objs);
}

EOLIAN static const Eina_List *
_elm_widget_item_access_order_get(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   return _elm_access_widget_item_access_order_get(item);
}

EOLIAN static void
_elm_widget_item_access_order_unset(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   _elm_access_widget_item_access_order_unset(item);
}

EOLIAN static Evas_Object*
_elm_widget_item_access_register(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   _elm_access_widget_item_register(item);
   return item->access_obj;
}

EOLIAN static void
_elm_widget_item_access_unregister(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   _elm_access_widget_item_unregister(item);
}

EOLIAN static Evas_Object*
_elm_widget_item_access_object_get(const Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item)
{
   return item->access_obj;
}

EOLIAN static Evas_Object *
_elm_widget_item_focus_next_object_get(const Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item, Elm_Focus_Direction dir)
{
   Evas_Object *ret = NULL;

   if (dir == ELM_FOCUS_PREVIOUS)
     ret = item->focus_previous;
   else if (dir == ELM_FOCUS_NEXT)
     ret = item->focus_next;
   else if (dir == ELM_FOCUS_UP)
     ret = item->focus_up;
   else if (dir == ELM_FOCUS_DOWN)
     ret = item->focus_down;
   else if (dir == ELM_FOCUS_RIGHT)
     ret = item->focus_right;
   else if (dir == ELM_FOCUS_LEFT)
     ret = item->focus_left;

   return ret;
}

EOLIAN static void
_elm_widget_item_focus_next_object_set(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item, Evas_Object *next, Elm_Focus_Direction dir)
{
   if (dir == ELM_FOCUS_PREVIOUS)
     item->focus_previous = next;
   else if (dir == ELM_FOCUS_NEXT)
     item->focus_next = next;
   else if (dir == ELM_FOCUS_UP)
     item->focus_up = next;
   else if (dir == ELM_FOCUS_DOWN)
     item->focus_down = next;
   else if (dir == ELM_FOCUS_RIGHT)
     item->focus_right = next;
   else if (dir == ELM_FOCUS_LEFT)
     item->focus_left = next;
}

EOLIAN static Elm_Object_Item*
_elm_widget_item_focus_next_item_get(const Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item, Elm_Focus_Direction dir)
{
   Elm_Object_Item *ret = NULL;

   if (dir == ELM_FOCUS_PREVIOUS)
     ret = item->item_focus_previous;
   else if (dir == ELM_FOCUS_NEXT)
     ret = item->item_focus_next;
   else if (dir == ELM_FOCUS_UP)
     ret = item->item_focus_up;
   else if (dir == ELM_FOCUS_DOWN)
     ret = item->item_focus_down;
   else if (dir == ELM_FOCUS_RIGHT)
     ret = item->item_focus_right;
   else if (dir == ELM_FOCUS_LEFT)
     ret = item->item_focus_left;

   return ret;
}

EOLIAN static void
_elm_widget_item_focus_next_item_set(Eo *eo_item EINA_UNUSED, Elm_Widget_Item_Data *item, Elm_Object_Item *next_item, Elm_Focus_Direction dir)
{
   if (dir == ELM_FOCUS_PREVIOUS)
     item->item_focus_previous = next_item;
   else if (dir == ELM_FOCUS_NEXT)
     item->item_focus_next = next_item;
   else if (dir == ELM_FOCUS_UP)
     item->item_focus_up = next_item;
   else if (dir == ELM_FOCUS_DOWN)
     item->item_focus_down = next_item;
   else if (dir == ELM_FOCUS_RIGHT)
     item->item_focus_right = next_item;
   else if (dir == ELM_FOCUS_LEFT)
     item->item_focus_left = next_item;
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

EOLIAN static Eo *
_elm_widget_efl_object_constructor(Eo *obj, Elm_Widget_Smart_Data *sd EINA_UNUSED)
{
   Eo *parent = NULL;

   sd->on_create = EINA_TRUE;
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   parent = efl_parent_get(obj);
   elm_obj_widget_parent_set(obj, parent);
   sd->on_create = EINA_FALSE;

   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_UNKNOWN);

   return obj;
}

EOLIAN static Efl_Object*
_elm_widget_efl_object_finalize(Eo *obj, Elm_Widget_Smart_Data *pd)
{
  Eo *eo;

  eo = efl_finalize(efl_super(obj, MY_CLASS));

  _focus_state_eval(obj, pd);

  return eo;
}


EOLIAN static void
_elm_widget_efl_object_destructor(Eo *obj, Elm_Widget_Smart_Data *sd EINA_UNUSED)
{
   sd->on_destroy = EINA_TRUE;
   efl_destructor(efl_super(obj, ELM_WIDGET_CLASS));
   sd->on_destroy = EINA_FALSE;

   if (sd->manager.provider)
     efl_event_callback_del(sd->manager.provider, EFL_UI_FOCUS_USER_EVENT_MANAGER_CHANGED, _manager_changed_cb, obj);
   sd->manager.provider = NULL;

   elm_interface_atspi_accessible_removed(obj);
   if (sd->logical.parent)
     {
        efl_weak_unref(&sd->logical.parent);
        sd->logical.parent = NULL;
     }
}

EOLIAN static Eina_Bool
_elm_widget_on_focus(Eo *obj, Elm_Widget_Smart_Data *sd, Elm_Object_Item *item EINA_UNUSED)
{
   Eina_Bool focused;
   const Efl_Event_Description *desc;

   if (!elm_widget_can_focus_get(obj))
     return EINA_FALSE;

   focused = elm_widget_focus_get(obj);
   desc = focused ? ELM_WIDGET_EVENT_FOCUSED : ELM_WIDGET_EVENT_UNFOCUSED;

   if (!sd->resize_obj)
     evas_object_focus_set(obj, focused);
   efl_event_callback_legacy_call(obj, desc, NULL);

   if (_elm_config->atspi_mode && !elm_widget_child_can_focus_get(obj))
     elm_interface_atspi_accessible_state_changed_signal_emit(obj, ELM_ATSPI_STATE_FOCUSED, focused);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_widget_disable(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_widget_widget_event(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *_pd EINA_UNUSED, Evas_Object *source EINA_UNUSED, Evas_Callback_Type type EINA_UNUSED, void *event_info EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *_pd EINA_UNUSED)
{
   WRN("The %s widget does not implement the \"focus_next/focus_next_manager_is\" functions.",
       efl_class_name_get(efl_class_get(obj)));
   return EINA_FALSE;
}

static Eina_Bool
_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *_pd EINA_UNUSED)
{
   WRN("The %s widget does not implement the \"focus_direction/focus_direction_manager_is\" functions.",
       efl_class_name_get(efl_class_get(obj)));
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_widget_activate(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *_pd EINA_UNUSED, Elm_Activate act EINA_UNUSED)
{
   WRN("The %s widget does not implement the \"activate\" functions.",
       efl_class_name_get(efl_class_get(obj)));
   return EINA_TRUE;
}

EOLIAN static void
_elm_widget_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

EOLIAN static Eina_Bool
_elm_widget_elm_interface_atspi_component_focus_grab(Eo *obj, Elm_Widget_Smart_Data *pd EINA_UNUSED)
{
   if (elm_object_focus_allow_get(obj))
     {
       Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
       if (!ee) return EINA_FALSE;
       ecore_evas_activate(ee);
       elm_object_focus_set(obj, EINA_TRUE);
       return EINA_TRUE;
     }
   return EINA_FALSE;
}

EOLIAN static const char*
_elm_widget_elm_interface_atspi_accessible_name_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *_pd)
{
   const char *ret, *name;
   char *accessible_name;
   name = elm_interface_atspi_accessible_name_get(efl_super(obj, ELM_WIDGET_CLASS));

   if (name) return name;

   ret = elm_object_text_get(obj);
   if (!ret) return NULL;

   accessible_name = _elm_util_mkup_to_text(ret);
   eina_stringshare_del(_pd->accessible_name);
   _pd->accessible_name =  eina_stringshare_add(accessible_name);
   free(accessible_name);
   return _pd->accessible_name;
}

EOLIAN static Eina_List*
_elm_widget_elm_interface_atspi_accessible_children_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *pd)
{
   Eina_List *l, *accs = NULL;
   Evas_Object *widget;
   Elm_Atspi_Type type;

   EINA_LIST_FOREACH(pd->subobjs, l, widget)
     {
        if (!elm_object_widget_check(widget)) continue;
        if (!efl_isa(widget, ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN)) continue;
        type = elm_interface_atspi_accessible_type_get(widget);
        if (type == ELM_ATSPI_TYPE_DISABLED) continue;
        if (type == ELM_ATSPI_TYPE_SKIPPED)
          {
             Eina_List *children;
             children = elm_interface_atspi_accessible_children_get(widget);
             accs = eina_list_merge(accs, children);
             continue;
          }
        accs = eina_list_append(accs, widget);
     }
   return accs;
}

EOLIAN static Eo*
_elm_widget_elm_interface_atspi_accessible_parent_get(Eo *obj, Elm_Widget_Smart_Data *pd EINA_UNUSED)
{
   Elm_Atspi_Type type;
   Elm_Interface_Atspi_Accessible *parent = obj;

   do {
        ELM_WIDGET_DATA_GET_OR_RETURN(parent, wd, NULL);
        parent = wd->parent_obj;
        type = elm_interface_atspi_accessible_type_get(parent);
   } while (parent && (type == ELM_ATSPI_TYPE_SKIPPED));

   return efl_isa(parent, ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN) ? parent : NULL;
}

EOLIAN static Elm_Atspi_State_Set
_elm_widget_elm_interface_atspi_accessible_state_set_get(Eo *obj, Elm_Widget_Smart_Data *pd EINA_UNUSED)
{
   Elm_Atspi_State_Set states = 0;

   states = elm_interface_atspi_accessible_state_set_get(efl_super(obj, ELM_WIDGET_CLASS));

   if (evas_object_visible_get(obj))
     {
        STATE_TYPE_SET(states, ELM_ATSPI_STATE_VISIBLE);
        if (_elm_widget_onscreen_is(obj))
          STATE_TYPE_SET(states, ELM_ATSPI_STATE_SHOWING);
     }
   if (!elm_widget_child_can_focus_get(obj))
     {
        if (elm_object_focus_allow_get(obj))
          STATE_TYPE_SET(states, ELM_ATSPI_STATE_FOCUSABLE);
        if (elm_object_focus_get(obj))
          STATE_TYPE_SET(states, ELM_ATSPI_STATE_FOCUSED);
     }
   if (!elm_object_disabled_get(obj))
     {
        STATE_TYPE_SET(states, ELM_ATSPI_STATE_ENABLED);
        STATE_TYPE_SET(states, ELM_ATSPI_STATE_SENSITIVE);
     }

   return states;
}

EOLIAN static Eina_List*
_elm_widget_elm_interface_atspi_accessible_attributes_get(Eo *obj, Elm_Widget_Smart_Data *pd EINA_UNUSED)
{
   Eina_List *ret = NULL;
   Elm_Atspi_Attribute *attr = calloc(1, sizeof(Elm_Atspi_Attribute));
   if (!attr) return NULL;

   attr->key = eina_stringshare_add("type");
   attr->value = eina_stringshare_add(evas_object_type_get(obj));

   ret = eina_list_append(ret, attr);
   return ret;
}

EOLIAN static void
_elm_widget_item_elm_interface_atspi_component_extents_get(Eo *obj EINA_UNUSED, Elm_Widget_Item_Data *sd EINA_UNUSED, Eina_Bool screen_coords, int *x, int *y, int *w, int *h)
{
   int ee_x, ee_y;

   if (!sd->view)
     {
        if (x) *x = -1;
        if (y) *y = -1;
        if (w) *w = -1;
        if (h) *h = -1;
        return;
     }

   evas_object_geometry_get(sd->view, x, y, w, h);
   if (screen_coords)
     {
        Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(sd->view));
        if (!ee) return;
        ecore_evas_geometry_get(ee, &ee_x, &ee_y, NULL, NULL);
        if (x) *x += ee_x;
        if (y) *y += ee_y;
     }
}

EOLIAN static Eina_Bool
_elm_widget_item_elm_interface_atspi_component_extents_set(Eo *obj EINA_UNUSED, Elm_Widget_Item_Data *sd EINA_UNUSED, Eina_Bool screen_coords EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static int
_elm_widget_item_elm_interface_atspi_component_layer_get(Eo *obj EINA_UNUSED, Elm_Widget_Item_Data *sd EINA_UNUSED)
{
   if (!sd->view)
     return -1;
   return evas_object_layer_get(sd->view);
}

EOLIAN static Eina_Bool
_elm_widget_item_elm_interface_atspi_component_focus_grab(Eo *obj EINA_UNUSED, Elm_Widget_Item_Data *_pd EINA_UNUSED)
{
   elm_object_item_focus_set(obj, EINA_TRUE);
   return elm_object_item_focus_get(obj);
}

EOLIAN static double
_elm_widget_item_elm_interface_atspi_component_alpha_get(Eo *obj EINA_UNUSED, Elm_Widget_Item_Data *sd EINA_UNUSED)
{
   int alpha;

   if (!sd->view) return -1.0;
   evas_object_color_get(sd->view, NULL, NULL, NULL, &alpha);
   return (double)alpha / 255.0;
}

EOLIAN static Efl_Object *
_elm_widget_efl_object_provider_find(Eo *obj, Elm_Widget_Smart_Data *pd, const Efl_Object *klass)
{
   Efl_Object *lookup = NULL;

   if ((klass == EFL_CONFIG_INTERFACE) || (klass == EFL_CONFIG_GLOBAL_CLASS))
     return _efl_config_obj;

   if (pd->provider_lookup) return NULL;
   pd->provider_lookup = EINA_TRUE;

   lookup = efl_provider_find(pd->parent_obj, klass);
   if (!lookup) lookup = efl_provider_find(efl_super(obj, MY_CLASS), klass);

   pd->provider_lookup = EINA_FALSE;

   return lookup;
}

EOLIAN static Efl_Ui_Focus_Manager*
_elm_widget_efl_ui_focus_user_parent_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *pd EINA_UNUSED)
{
   return pd->logical.parent;
}

EOLIAN static Efl_Ui_Focus_Manager*
_elm_widget_efl_ui_focus_user_manager_get(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *pd EINA_UNUSED)
{
   return pd->manager.manager;
}

EOLIAN static Eina_Rectangle
_elm_widget_efl_ui_focus_object_focus_geometry_get(Eo *obj, Elm_Widget_Smart_Data *pd EINA_UNUSED)
{
   Eina_Rectangle rect;

   efl_gfx_geometry_get(obj, &rect.x , &rect.y, &rect.w, &rect.h);

   return rect;
}

EOLIAN static void
_elm_widget_efl_ui_focus_object_focus_set(Eo *obj, Elm_Widget_Smart_Data *pd, Eina_Bool focus)
{
   pd->focused = focus;
   elm_obj_widget_on_focus(obj, NULL);

   efl_ui_focus_object_focus_set(efl_super(obj, MY_CLASS), focus);
}

EOLIAN static Efl_Ui_Focus_Manager*
_elm_widget_focus_manager_factory(Eo *obj EINA_UNUSED, Elm_Widget_Smart_Data *pd EINA_UNUSED, Efl_Ui_Focus_Object *root EINA_UNUSED)
{
   ERR("No manager presented");
   return NULL;
}


/* Legacy APIs */

/* elm_object_content_xxx APIs are supposed to work on all objects for which
 * elm_object_widget_check() returns true. The below checks avoid printing out
 * undesired ERR messages. */
EAPI void
elm_widget_content_part_set(Evas_Object *obj,
                            const char *part,
                            Evas_Object *content)
{
   ELM_WIDGET_CHECK(obj);
   if (!efl_isa(obj, EFL_PART_INTERFACE)) return;
   if (!part)
     {
        part = elm_widget_default_content_part_get(obj);
        if (!part) return;
     }
   efl_content_set(efl_part(obj, part), content);
}

EAPI Evas_Object *
elm_widget_content_part_get(const Evas_Object *obj,
                            const char *part)
{
   ELM_WIDGET_CHECK(obj) NULL;
   if (!efl_isa(obj, EFL_PART_INTERFACE)) return NULL;
   if (!part)
     {
        part = elm_widget_default_content_part_get(obj);
        if (!part) return NULL;
     }
   return efl_content_get(efl_part(obj, part));
}

EAPI Evas_Object *
elm_widget_content_part_unset(Evas_Object *obj,
                              const char *part)
{
   ELM_WIDGET_CHECK(obj) NULL;
   if (!efl_isa(obj, EFL_PART_INTERFACE)) return NULL;
   if (!part)
     {
        part = elm_widget_default_content_part_get(obj);
        if (!part) return NULL;
     }
   return efl_content_unset(efl_part(obj, part));
}


/* Internal EO APIs and hidden overrides */

EFL_FUNC_BODY_CONST(elm_widget_default_content_part_get, const char *, NULL)
EFL_FUNC_BODY_CONST(elm_widget_default_text_part_get, const char *, NULL)

ELM_PART_CONTENT_DEFAULT_SET(elm_widget, NULL)
ELM_PART_TEXT_DEFAULT_GET(elm_widget, NULL)

#define ELM_WIDGET_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_widget), \
   ELM_PART_CONTENT_DEFAULT_OPS(elm_widget), \
   ELM_PART_TEXT_DEFAULT_OPS(elm_widget), \
   EFL_OBJECT_OP_FUNC(efl_dbg_info_get, _elm_widget_efl_object_dbg_info_get)

#include "elm_widget_item.eo.c"
#include "elm_widget.eo.c"
