#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#define MY_CLASS ELM_OBJ_ACCESS_CLASS

#define MY_CLASS_NAME "Elm_Access"
#define MY_CLASS_NAME_LEGACY "elm_access"

struct _Func_Data
{
   void                *user_data; /* Holds user data to CB */
   Elm_Access_Action_Cb cb;
};

typedef struct _Func_Data Func_Data;

struct _Action_Info
{
   Evas_Object      *obj;
   Func_Data         fn[ELM_ACCESS_ACTION_LAST + 1]; /* Callback for specific action */
};

typedef struct _Action_Info Action_Info;

static Eina_Bool mouse_event_enable = EINA_TRUE;
static Eina_Bool auto_highlight = EINA_FALSE;
static Elm_Access_Action_Type action_by = ELM_ACCESS_ACTION_FIRST;

static Evas_Object * _elm_access_add(Evas_Object *parent);

static void _access_object_unregister(Evas_Object *obj);

EOLIAN static void
_elm_access_evas_smart_add(Eo *obj, void *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);
}

static Eina_Bool
_access_action_callback_call(Evas_Object *obj,
                             Elm_Access_Action_Type type,
                             Elm_Access_Action_Info *action_info)
{
   Elm_Access_Action_Info *ai = NULL;
   Action_Info *a;
   Eina_Bool ret;

   ret = EINA_FALSE;
   a = evas_object_data_get(obj, "_elm_access_action_info");

   if (!action_info)
     {
        ai = calloc(1, sizeof(Elm_Access_Action_Info));
        action_info = ai;
     }

   action_info->action_type = type;

   if ((type == ELM_ACCESS_ACTION_HIGHLIGHT) &&
       (action_by != ELM_ACCESS_ACTION_FIRST))
     action_info->action_by = action_by;

   if (a && (a->fn[type].cb))
     ret = a->fn[type].cb(a->fn[type].user_data, obj, action_info);

   free(ai);

   return ret;
}

EOLIAN static Eina_Bool
_elm_access_elm_widget_activate(Eo *obj, void *_pd EINA_UNUSED, Elm_Activate act)
{
   int type = ELM_ACCESS_ACTION_FIRST;

   Action_Info *a;
   a = evas_object_data_get(obj, "_elm_access_action_info");

   switch (act)
     {
      case ELM_ACTIVATE_DEFAULT:
        type = ELM_ACCESS_ACTION_ACTIVATE;
        break;

      case ELM_ACTIVATE_UP:
        type = ELM_ACCESS_ACTION_UP;
        break;

      case ELM_ACTIVATE_DOWN:
        type = ELM_ACCESS_ACTION_DOWN;
        break;

      case ELM_ACTIVATE_RIGHT:
        break;

      case ELM_ACTIVATE_LEFT:
        break;

      case ELM_ACTIVATE_BACK:
        type = ELM_ACCESS_ACTION_BACK;
        break;

      default:
        break;
     }

   if (type == ELM_ACCESS_ACTION_FIRST) return EINA_FALSE;

   /* if an access object has a callback, it would have the intention to do
      something. so, check here and return EINA_TRUE. */
   if ((a) && (type > ELM_ACCESS_ACTION_FIRST) &&
              (type < ELM_ACCESS_ACTION_LAST) &&
              (a->fn[type].cb))
     {
        _access_action_callback_call(obj, type, NULL);
        return EINA_TRUE;
     }

   /* TODO: deprecate below? */
   if (act != ELM_ACTIVATE_DEFAULT) return EINA_FALSE;

   Elm_Access_Info *ac = evas_object_data_get(obj, "_elm_access");
   if (!ac) return EINA_FALSE;

   if (ac->activate)
     ac->activate(ac->activate_data, ac->part_object,
                  (Elm_Object_Item *)ac->widget_item);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_access_elm_widget_on_focus(Eo *obj, void *_pd EINA_UNUSED)
{
   evas_object_focus_set(obj, elm_widget_focus_get(obj));

   return EINA_TRUE;
}

typedef struct _Mod_Api Mod_Api;

struct _Mod_Api
{
   void (*out_read) (const char *txt);
   void (*out_read_done) (void);
   void (*out_cancel) (void);
   void (*out_done_callback_set) (void (*func) (void *data), const void *data);
};

static int initted = 0;
static Mod_Api *mapi = NULL;

static void
_access_init(void)
{
   Elm_Module *m;
   initted++;
   if (initted > 1) return;
   if (!(m = _elm_module_find_as("access/api"))) return;
   m->api = malloc(sizeof(Mod_Api));
   if (!m->api) return;
   m->init_func(m);
   ((Mod_Api *)(m->api)      )->out_read = // called to read out some text
      _elm_module_symbol_get(m, "out_read");
   ((Mod_Api *)(m->api)      )->out_read_done = // called to set a done marker so when it is reached the done callback is called
      _elm_module_symbol_get(m, "out_read_done");
   ((Mod_Api *)(m->api)      )->out_cancel = // called to read out some text
      _elm_module_symbol_get(m, "out_cancel");
   ((Mod_Api *)(m->api)      )->out_done_callback_set = // called when last read done
      _elm_module_symbol_get(m, "out_done_callback_set");
   mapi = m->api;
}

static void
_access_shutdown(void)
{
   Elm_Module *m;
   if (initted == 0) return;
   if (!(m = _elm_module_find_as("access/api"))) return;

   m->shutdown_func(m);

   initted = 0;

   /* _elm_module_unload(); could access m->api and try to free(); */
   ELM_SAFE_FREE(m->api, free);
}

static Elm_Access_Item *
_access_add_set(Elm_Access_Info *ac, int type)
{
   Elm_Access_Item *ai;
   Eina_List *l;

   if (!ac) return NULL;
   EINA_LIST_FOREACH(ac->items, l, ai)
     {
        if (ai->type == type)
          {
             if (!ai->func)
               {
                  eina_stringshare_del(ai->data);
               }
             ai->func = NULL;
             ai->data = NULL;
             return ai;
          }
     }
   ai = calloc(1, sizeof(Elm_Access_Item));
   ai->type = type;
   ac->items = eina_list_prepend(ac->items, ai);
   return ai;
}

static Evas_Object *
_access_highlight_object_get(Evas_Object *obj)
{
   Evas_Object *o, *ho;

   o = evas_object_name_find(evas_object_evas_get(obj), "_elm_access_disp");
   if (!o) return NULL;

   ho = evas_object_data_get(o, "_elm_access_target");

   return ho;
}

static void
_access_highlight_read(Elm_Access_Info *ac, Evas_Object *obj)
{
   int type;
   char *txt = NULL;
   Eina_Strbuf *strbuf;

   strbuf = eina_strbuf_new();

   if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
     {
        if (ac->on_highlight) ac->on_highlight(ac->on_highlight_data);
        _elm_access_object_highlight(obj);

        for (type = ELM_ACCESS_INFO_FIRST + 1; type < ELM_ACCESS_INFO_LAST; type++)
          {
             txt = _elm_access_text_get(ac, type, obj);
             if (txt && (strlen(txt) > 0))
               {
                  if (eina_strbuf_length_get(strbuf) > 0)
                    eina_strbuf_append_printf(strbuf, ", %s", txt);
                  else
                    eina_strbuf_append(strbuf, txt);

                  free(txt);
               }
          }
     }

   txt = eina_strbuf_string_steal(strbuf);
   eina_strbuf_free(strbuf);

   _elm_access_say(txt);
   free(txt);
}

static Eina_Bool
_access_obj_over_timeout_cb(void *data)
{
   Elm_Access_Info *ac;
   Evas_Object *ho;

   if (!data) return EINA_FALSE;

   ac = evas_object_data_get(data, "_elm_access");
   if (!ac) return EINA_FALSE;

   ho = _access_highlight_object_get(data);
   if (ho != data) _access_highlight_read(ac, data);

   ac->delay_timer = NULL;
   return EINA_FALSE;
}

static void
_access_hover_mouse_in_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info  EINA_UNUSED)
{
   Elm_Access_Info *ac;
   if (!mouse_event_enable) return;

    ac = evas_object_data_get(data, "_elm_access");
   if (!ac) return;

   ELM_SAFE_FREE(ac->delay_timer, ecore_timer_del);

   if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
      ac->delay_timer = ecore_timer_add(0.2, _access_obj_over_timeout_cb, data);
}

static void
_access_hover_mouse_out_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Access_Info *ac;
   if (!mouse_event_enable) return;

   ac = evas_object_data_get(data, "_elm_access");
   if (!ac) return;

   _elm_access_object_unhilight(data);

   ELM_SAFE_FREE(ac->delay_timer, ecore_timer_del);
}

static void
_access_read_done(void *data EINA_UNUSED)
{
   DBG("read done");
   // FIXME: produce event here
}

static void
_access_2nd_click_del_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Ecore_Timer *t;

   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_DEL,
                                       _access_2nd_click_del_cb, NULL);
   t = evas_object_data_get(obj, "_elm_2nd_timeout");
   if (t)
     {
        ecore_timer_del(t);
        evas_object_data_del(obj, "_elm_2nd_timeout");
     }
}

static Eina_Bool
_access_2nd_click_timeout_cb(void *data)
{
   evas_object_event_callback_del_full(data, EVAS_CALLBACK_DEL,
                                       _access_2nd_click_del_cb, NULL);
   evas_object_data_del(data, "_elm_2nd_timeout");
   return EINA_FALSE;
}

static void
_access_obj_hilight_del_cb(void *data EINA_UNUSED, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _elm_access_object_highlight_disable(e);
}

static void
_access_obj_hilight_hide_cb(void *data EINA_UNUSED, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _elm_access_object_highlight_disable(e);
}

static void
_access_obj_hilight_move_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Coord x, y;
   Evas_Object *o;

   o = evas_object_name_find(evas_object_evas_get(obj), "_elm_access_disp");
   if (!o) return;
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   evas_object_move(o, x, y);
}

static void
_access_obj_hilight_resize_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Coord w, h;
   Evas_Object *o;

   o = evas_object_name_find(evas_object_evas_get(obj), "_elm_access_disp");
   if (!o) return;
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   evas_object_resize(o, w, h);
}

void
_elm_access_mouse_event_enabled_set(Eina_Bool enabled)
{
   enabled = !!enabled;
   if (mouse_event_enable == enabled) return;
   mouse_event_enable = enabled;
}

void
_elm_access_auto_highlight_set(Eina_Bool enabled)
{
   enabled = !!enabled;
   if (auto_highlight == enabled) return;
   auto_highlight = enabled;
}

Eina_Bool
_elm_access_auto_highlight_get(void)
{
   return auto_highlight;
}

void
_elm_access_shutdown()
{
   _access_shutdown();
}

static void
_access_order_del_cb(void *data,
                     Evas *e EINA_UNUSED,
                     Evas_Object *obj,
                     void *event_info EINA_UNUSED)
{
   Elm_Widget_Item *item = data;

   item->access_order = eina_list_remove(item->access_order, obj);
}

void
_elm_access_widget_item_access_order_set(Elm_Widget_Item *item,
                                         Eina_List *objs)
{
   Eina_List *l;
   Evas_Object *o;

   if (!item) return;

   _elm_access_widget_item_access_order_unset(item);

   EINA_LIST_FOREACH(objs, l, o)
     {
        evas_object_event_callback_add(o, EVAS_CALLBACK_DEL,
                                       _access_order_del_cb, item);
     }

   item->access_order = objs;
}

const Eina_List *
_elm_access_widget_item_access_order_get(const Elm_Widget_Item *item)
{
   if (!item) return NULL;
   return item->access_order;
}

void
_elm_access_widget_item_access_order_unset(Elm_Widget_Item *item)
{
   Eina_List *l, *l_next;
   Evas_Object *o;

   if (!item) return;

   EINA_LIST_FOREACH_SAFE(item->access_order, l, l_next, o)
     {
        evas_object_event_callback_del_full
          (o, EVAS_CALLBACK_DEL, _access_order_del_cb, item);
        item->access_order = eina_list_remove_list(item->access_order, l);
     }
}

static Eina_Bool
_access_highlight_next_get(Evas_Object *obj, Elm_Focus_Direction dir)
{
   int type;
   Evas_Object *ho, *parent, *target;
   Eina_Bool ret;

   target = NULL;
   ret = EINA_FALSE;

   if (!elm_widget_is(obj)) return ret;

   ho = _access_highlight_object_get(obj);
   if (!ho) ho = obj;

   parent = ho;

   /* find highlight root */
   do
     {
        ELM_WIDGET_DATA_GET_OR_RETURN(parent, wd, ret);
        if (wd->highlight_root)
          {
             /* change highlight root */
             obj = parent;
             break;
          }
        parent = elm_widget_parent_get(parent);
     }
   while (parent);

   _elm_access_auto_highlight_set(EINA_TRUE);

   if (dir == ELM_FOCUS_NEXT)
     type = ELM_ACCESS_ACTION_HIGHLIGHT_NEXT;
   else
     type = ELM_ACCESS_ACTION_HIGHLIGHT_PREV;

   /* this value is used in _elm_access_object_highlight();
      to inform the target object of how to get highlight */
   action_by = type;

   if (!_access_action_callback_call(ho, type, NULL))
     {
        if (ho)
          {
             Elm_Access_Info *info = _elm_access_info_get(ho);
             if (type == ELM_ACCESS_ACTION_HIGHLIGHT_NEXT && info->next)
               target = info->next;
             else if (type == ELM_ACCESS_ACTION_HIGHLIGHT_PREV && info->prev)
               target = info->prev;
          }

        if (target)
          {
             _elm_access_highlight_set(target);
             elm_widget_focus_region_show(target);
             ret = EINA_TRUE;
          }
        else
          {
             ret = elm_widget_focus_next_get(obj, dir, &target);
             if (ret && target)
               _elm_access_highlight_set(target);
          }
     }

   action_by = ELM_ACCESS_ACTION_FIRST;

   _elm_access_auto_highlight_set(EINA_FALSE);

   return ret;
}

//-------------------------------------------------------------------------//
EAPI void
_elm_access_highlight_set(Evas_Object* obj)
{
   Elm_Access_Info *ac;
   Evas_Object *ho;

   if (!obj) return;

   ho = _access_highlight_object_get(obj);
   if (ho == obj) return;

   ac = evas_object_data_get(obj, "_elm_access");
   if (!ac) return;

   _access_highlight_read(ac, obj);
}

EAPI void
_elm_access_clear(Elm_Access_Info *ac)
{
   Elm_Access_Item *ai;

   if (!ac) return;
   ELM_SAFE_FREE(ac->delay_timer, ecore_timer_del);
   EINA_LIST_FREE(ac->items, ai)
     {
        if (!ai->func)
          {
             eina_stringshare_del(ai->data);
          }
        free(ai);
     }
}

EAPI void
_elm_access_text_set(Elm_Access_Info *ac, int type, const char *text)
{
   Elm_Access_Item *ai = _access_add_set(ac, type);
   if (!ai) return;
   ai->func = NULL;
   ai->data = eina_stringshare_add(text);
}

EAPI void
_elm_access_callback_set(Elm_Access_Info *ac, int type, Elm_Access_Info_Cb func, const void *data)
{
   Elm_Access_Item *ai = _access_add_set(ac, type);
   if (!ai) return;
   ai->func = func;
   ai->data = data;
}

EAPI void
_elm_access_on_highlight_hook_set(Elm_Access_Info           *ac,
                                  Elm_Access_On_Highlight_Cb func,
                                  void                      *data)
{
    if (!ac) return;
    ac->on_highlight = func;
    ac->on_highlight_data = data;
}

EAPI void
_elm_access_activate_callback_set(Elm_Access_Info           *ac,
                                  Elm_Access_Activate_Cb     func,
                                  void                      *data)
{
   if (!ac) return;
   ac->activate = func;
   ac->activate_data = data;
}

EAPI void
_elm_access_highlight_object_activate(Evas_Object *obj, Elm_Activate act)
{
   Evas_Object *highlight;

   highlight = _access_highlight_object_get(obj);
   if (!highlight) return;

   _elm_access_auto_highlight_set(EINA_FALSE);

   if (!elm_object_focus_get(highlight))
     elm_object_focus_set(highlight, EINA_TRUE);

   elm_widget_activate(highlight, act);
   return;
}

EAPI void
_elm_access_highlight_cycle(Evas_Object *obj, Elm_Focus_Direction dir)
{
   int type;
   Evas_Object *ho, *parent;

   ho = _access_highlight_object_get(obj);
   if (!ho) return;

   parent = ho;

   /* find highlight root */
   do
     {
        ELM_WIDGET_DATA_GET_OR_RETURN(parent, wd);
        if (wd->highlight_root)
          {
             /* change highlight root */
             obj = parent;
             break;
          }
        parent = elm_widget_parent_get(parent);
     }
   while (parent);

   _elm_access_auto_highlight_set(EINA_TRUE);

   if (dir == ELM_FOCUS_NEXT)
     type = ELM_ACCESS_ACTION_HIGHLIGHT_NEXT;
   else
     type = ELM_ACCESS_ACTION_HIGHLIGHT_PREV;

   action_by = type;

   if (!_access_action_callback_call(ho, type, NULL))
     {
        Elm_Access_Info *info = _elm_access_info_get(ho);
        Evas_Object *comming = NULL;
        if (type == ELM_ACCESS_ACTION_HIGHLIGHT_NEXT)
          {
             if ((info) && (info->next)) comming = info->next;
          }
        else
          {
             if ((info) && (info->prev)) comming = info->prev;
          }
        if (comming)
          {
             _elm_access_highlight_set(comming);
             elm_widget_focus_region_show(comming);
          }
        else
          elm_widget_focus_cycle(obj, dir);
     }

   action_by = ELM_ACCESS_ACTION_FIRST;

   _elm_access_auto_highlight_set(EINA_FALSE);
}

EAPI char *
_elm_access_text_get(const Elm_Access_Info *ac, int type, const Evas_Object *obj)
{
   Elm_Access_Item *ai;
   Eina_List *l;

   if (!ac) return NULL;
   EINA_LIST_FOREACH(ac->items, l, ai)
     {
        if (ai->type == type)
          {
             if (ai->func) return ai->func((void *)(ai->data), (Evas_Object *)obj);
             else if (ai->data) return strdup(ai->data);
             return NULL;
          }
     }
   return NULL;
}

EAPI void
_elm_access_read(Elm_Access_Info *ac, int type, const Evas_Object *obj)
{
   char *txt = _elm_access_text_get(ac, type, obj);

   _access_init();
   if (mapi)
     {
        if (mapi->out_done_callback_set)
           mapi->out_done_callback_set(_access_read_done, NULL);
        if (type == ELM_ACCESS_DONE)
          {
             if (mapi->out_read_done) mapi->out_read_done();
          }
        else if (type == ELM_ACCESS_CANCEL)
          {
             if (mapi->out_cancel) mapi->out_cancel();
          }
        else
          {
             if (txt)
               {
                  if (mapi->out_read) mapi->out_read(txt);
                  if (mapi->out_read) mapi->out_read(".\n");
               }
          }
     }
   free(txt);
}

EAPI void
_elm_access_say(const char *txt)
{
   if (!_elm_config->access_mode) return;

   _access_init();
   if (mapi)
     {
        if (mapi->out_done_callback_set)
           mapi->out_done_callback_set(_access_read_done, NULL);
        if (mapi->out_cancel) mapi->out_cancel();
        if (txt)
          {
             if (mapi->out_read) mapi->out_read(txt);
             if (mapi->out_read) mapi->out_read(".\n");
          }
        if (mapi->out_read_done) mapi->out_read_done();
     }
}

EAPI Elm_Access_Info *
_elm_access_info_get(const Evas_Object *obj)
{
   return evas_object_data_get(obj, "_elm_access");
}

EAPI Elm_Access_Info *
_elm_access_object_get(const Evas_Object *obj)
{
   return _elm_access_info_get(obj);
}

static Evas_Object *
_elm_access_widget_target_get(Evas_Object *obj)
{
   Evas_Object *o = obj;

   do
     {
        if (elm_widget_is(o))
          break;
        else
          {
             o = elm_widget_parent_widget_get(o);
             if (!o)
               o = evas_object_smart_parent_get(o);
          }
     }
   while (o);

   return o;
}

EAPI void
_elm_access_object_highlight(Evas_Object *obj)
{
   Evas_Object *o, *widget;
   Evas_Coord x, y, w, h;
   Eina_Bool in_theme = EINA_FALSE;

   o = evas_object_name_find(evas_object_evas_get(obj), "_elm_access_disp");
   if (!o)
     {
        o = edje_object_add(evas_object_evas_get(obj));
        evas_object_name_set(o, "_elm_access_disp");
        evas_object_layer_set(o, ELM_OBJECT_LAYER_TOOLTIP);
     }
   else
     {
        Evas_Object *ptarget = evas_object_data_get(o, "_elm_access_target");
        if (ptarget)
          {
             evas_object_data_del(o, "_elm_access_target");
             elm_widget_parent_highlight_set(ptarget, EINA_FALSE);

             evas_object_event_callback_del_full(ptarget, EVAS_CALLBACK_DEL,
                                                 _access_obj_hilight_del_cb, NULL);
             evas_object_event_callback_del_full(ptarget, EVAS_CALLBACK_HIDE,
                                                 _access_obj_hilight_hide_cb, NULL);
             evas_object_event_callback_del_full(ptarget, EVAS_CALLBACK_MOVE,
                                                 _access_obj_hilight_move_cb, NULL);
             evas_object_event_callback_del_full(ptarget, EVAS_CALLBACK_RESIZE,
                                                 _access_obj_hilight_resize_cb, NULL);

             widget = _elm_access_widget_target_get(ptarget);
             if (widget)
               {
                  if (elm_widget_access_highlight_in_theme_get(widget))
                    {
                       elm_widget_signal_emit(widget, "elm,action,access_highlight,hide", "elm");
                    }
               }
          }
     }
   evas_object_data_set(o, "_elm_access_target", obj);
   elm_widget_parent_highlight_set(obj, EINA_TRUE);

   elm_widget_theme_object_set(obj, o, "access", "base", "default");

   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
                                  _access_obj_hilight_del_cb, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_HIDE,
                                  _access_obj_hilight_hide_cb, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE,
                                  _access_obj_hilight_move_cb, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE,
                                  _access_obj_hilight_resize_cb, NULL);
   evas_object_raise(o);
   evas_object_geometry_get(obj, &x, &y, &w, &h);
   evas_object_move(o, x, y);
   evas_object_resize(o, w, h);

   widget = _elm_access_widget_target_get(obj);
   if (widget)
     {
        if (elm_widget_access_highlight_in_theme_get(widget))
          {
             in_theme = EINA_TRUE;
             elm_widget_signal_emit(widget, "elm,action,access_highlight,show", "elm");
          }
     }
   /* use callback, should an access object do below every time when
    * a window gets a client message ECORE_X_ATOM_E_ILLMUE_ACTION_READ? */
   if (!in_theme &&
       !_access_action_callback_call(obj, ELM_ACCESS_ACTION_HIGHLIGHT, NULL))
     evas_object_show(o);
   else
     evas_object_hide(o);
}

EAPI void
_elm_access_object_unhilight(Evas_Object *obj)
{
   Evas_Object *o, *ptarget;

   o = evas_object_name_find(evas_object_evas_get(obj), "_elm_access_disp");
   if (!o) return;
   ptarget = evas_object_data_get(o, "_elm_access_target");
   if (ptarget == obj)
     {
        evas_object_event_callback_del_full(ptarget, EVAS_CALLBACK_DEL,
                                            _access_obj_hilight_del_cb, NULL);
        evas_object_event_callback_del_full(ptarget, EVAS_CALLBACK_HIDE,
                                            _access_obj_hilight_hide_cb, NULL);
        evas_object_event_callback_del_full(ptarget, EVAS_CALLBACK_MOVE,
                                            _access_obj_hilight_move_cb, NULL);
        evas_object_event_callback_del_full(ptarget, EVAS_CALLBACK_RESIZE,
                                            _access_obj_hilight_resize_cb, NULL);
        evas_object_del(o);
        elm_widget_parent_highlight_set(ptarget, EINA_FALSE);
     }
}

static void
_content_resize(void *data, Evas *e EINA_UNUSED, Evas_Object *obj,
                void *event_info EINA_UNUSED)
{
   Evas_Object *accessobj;
   Evas_Coord w, h;

   accessobj = data;
   if (!accessobj) return;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   evas_object_resize(accessobj, w, h);
}

static void
_content_move(void *data, Evas *e EINA_UNUSED, Evas_Object *obj,
              void *event_info EINA_UNUSED)
{
   Evas_Object *accessobj;
   Evas_Coord x, y;

   accessobj = data;
   if (!accessobj) return;

   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   evas_object_move(accessobj, x, y);
}

static Evas_Object *
_access_object_register(Evas_Object *obj, Evas_Object *parent)
{
   Evas_Object *ao;
   Elm_Access_Info *ac;
   Evas_Coord x, y, w, h;

   if (!obj) return NULL;

   /* create access object */
   ao = _elm_access_add(parent);
   if (!ao) return NULL;

   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE,
                                  _content_resize, ao);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE,
                                  _content_move, ao);

   evas_object_geometry_get(obj, &x, &y, &w, &h);
   evas_object_move(ao, x, y);
   evas_object_resize(ao, w, h);
   evas_object_show(ao);

   /* register access object */
   _elm_access_object_register(ao, obj);

   /* set access object */
   evas_object_data_set(obj, "_part_access_obj", ao);

   /* set owner part object */
   ac = evas_object_data_get(ao, "_elm_access");
   ac->part_object = obj;

   return ao;
}

static void
_access_object_unregister(Evas_Object *obj)
{
   Elm_Access_Info *ac;
   Evas_Object *ao;

   if (!obj) return;

   ao = evas_object_data_get(obj, "_part_access_obj");

   if (ao)
     {
        /* delete callbacks and unregister access object in _access_obj_del_cb*/
        evas_object_del(ao);
     }
   else
     {
        /* button, check, label etc. */
        ac = evas_object_data_get(obj, "_elm_access");
        if (ac && ac->hoverobj)
          _elm_access_object_unregister(obj, ac->hoverobj);
     }
}

EAPI Evas_Object *
_elm_access_edje_object_part_object_register(Evas_Object* obj,
                                             const Evas_Object *eobj,
                                             const char* part)
{
   Evas_Object *ao, *po;

   po = (Evas_Object *)edje_object_part_object_get(eobj, part);
   if (!obj || !po) return NULL;

   /* check previous access object */
   ao = evas_object_data_get(po, "_part_access_obj");
   if (ao)
     _elm_access_edje_object_part_object_unregister(obj, eobj, part);

   ao = _access_object_register(po, obj);

   return ao;
}

//FIXME: unused obj should be removed from here and each widget.
EAPI void
_elm_access_edje_object_part_object_unregister(Evas_Object* obj EINA_UNUSED,
                                               const Evas_Object *eobj,
                                               const char* part)
{
   Evas_Object *po;

   po = (Evas_Object *)edje_object_part_object_get(eobj, part);
   if (!po) return;

   _access_object_unregister(po);
}

EAPI void
_elm_access_object_highlight_disable(Evas *e)
{
   Evas_Object *o, *ptarget;

   o = evas_object_name_find(e, "_elm_access_disp");
   if (!o) return;
   ptarget = evas_object_data_get(o, "_elm_access_target");
   if (ptarget)
     {
        evas_object_event_callback_del_full(ptarget, EVAS_CALLBACK_DEL,
                                            _access_obj_hilight_del_cb, NULL);
        evas_object_event_callback_del_full(ptarget, EVAS_CALLBACK_HIDE,
                                            _access_obj_hilight_hide_cb, NULL);
        evas_object_event_callback_del_full(ptarget, EVAS_CALLBACK_MOVE,
                                            _access_obj_hilight_move_cb, NULL);
        evas_object_event_callback_del_full(ptarget, EVAS_CALLBACK_RESIZE,
                                            _access_obj_hilight_resize_cb, NULL);
     }
   evas_object_del(o);
   elm_widget_parent_highlight_set(ptarget, EINA_FALSE);
}

static void
_access_obj_del_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{

   Ecore_Job *ao_del_job = NULL;

   evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL, _access_obj_del_cb);

   if (data) /* hover object */
     {
        evas_object_event_callback_del_full(data, EVAS_CALLBACK_RESIZE,
                                            _content_resize, obj);
        evas_object_event_callback_del_full(data, EVAS_CALLBACK_MOVE,
                                            _content_move, obj);

        _elm_access_object_unregister(obj, data);
     }

   ao_del_job = evas_object_data_get(obj, "_access_obj_del_job");

   if (ao_del_job)
     {
        ecore_job_del(ao_del_job);
        evas_object_data_del(obj, "_access_obj_del_job");
     }
}

static void
_access_obj_del_job(void *data)
{
   if (!data) return;

   evas_object_data_del(data, "_access_obj_del_job");

   evas_object_event_callback_del(data, EVAS_CALLBACK_DEL, _access_obj_del_cb);
   evas_object_del(data);
}

static void
_access_hover_del_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Ecore_Job *ao_del_job = NULL;

   /* data - access object - could be NULL */
   if (!data) return;

   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_RESIZE,
                                       _content_resize, data);
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_MOVE,
                                       _content_move, data);

   _elm_access_object_unregister(data, obj);

   /* delete access object in job */
   ao_del_job = evas_object_data_get(data, "_access_obj_del_job");
   if (ao_del_job)
     {
        ecore_job_del(ao_del_job);
        evas_object_data_del(data, "_access_obj_del_job");
     }

   ao_del_job = ecore_job_add(_access_obj_del_job, data);
   evas_object_data_set(data, "_access_obj_del_job", ao_del_job);
}

EAPI void
_elm_access_object_register(Evas_Object *obj, Evas_Object *hoverobj)
{
   Elm_Access_Info *ac;

   evas_object_event_callback_add(hoverobj, EVAS_CALLBACK_MOUSE_IN,
                                  _access_hover_mouse_in_cb, obj);
   evas_object_event_callback_add(hoverobj, EVAS_CALLBACK_MOUSE_OUT,
                                  _access_hover_mouse_out_cb, obj);
   evas_object_event_callback_add(hoverobj, EVAS_CALLBACK_DEL,
                                  _access_hover_del_cb, obj);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
                                  _access_obj_del_cb, hoverobj);

   ac = calloc(1, sizeof(Elm_Access_Info));
   evas_object_data_set(obj, "_elm_access", ac);

   ac->hoverobj = hoverobj;
}

EAPI void
_elm_access_object_unregister(Evas_Object *obj, Evas_Object *hoverobj)
{
   Elm_Access_Info *ac;
   Evas_Object *ao;

   evas_object_event_callback_del_full(hoverobj, EVAS_CALLBACK_MOUSE_IN,
                                       _access_hover_mouse_in_cb, obj);
   evas_object_event_callback_del_full(hoverobj, EVAS_CALLBACK_MOUSE_OUT,
                                       _access_hover_mouse_out_cb, obj);
   evas_object_event_callback_del_full(hoverobj, EVAS_CALLBACK_DEL,
                                       _access_hover_del_cb, obj);

   /* _access_obj_del_cb and _access_hover_del_cb calls this function,
      both do not need _part_access_obj data, so delete the data here. */
   ao = evas_object_data_get(hoverobj, "_part_access_obj");
   if (ao) evas_object_data_del(hoverobj, "_part_access_obj");

   ac = evas_object_data_get(obj, "_elm_access");
   evas_object_data_del(obj, "_elm_access");
   if (ac)
     {
        /* widget could delete VIEW(it) only and register item again,
           in this case _elm_access_widget_item_register could try to delete
           access object again in _elm_access_widget_item_unregister */
        if (ac->widget_item) ac->widget_item->access_obj = NULL;

        _elm_access_clear(ac);
        free(ac);
     }

   Action_Info *a;
   a = evas_object_data_get(obj, "_elm_access_action_info");
   evas_object_data_del(obj,  "_elm_access_action_info");
   free(a);
}

EAPI void
_elm_access_widget_item_register(Elm_Widget_Item *item)
{
   Evas_Object *ao, *ho;
   Evas_Coord x, y, w, h;
   Elm_Access_Info *ac;

   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);

   /* check previous access object */
   if (item->access_obj)
     _elm_access_widget_item_unregister(item);

   // create access object
   ho = item->view;
   ao = _elm_access_add(item->widget);
   if (!ao) return;

   evas_object_event_callback_add(ho, EVAS_CALLBACK_RESIZE,
                                  _content_resize, ao);
   evas_object_event_callback_add(ho, EVAS_CALLBACK_MOVE,
                                  _content_move, ao);

   evas_object_geometry_get(ho, &x, &y, &w, &h);
   evas_object_move(ao, x, y);
   evas_object_resize(ao, w, h);
   evas_object_show(ao);

   // register access object
   _elm_access_object_register(ao, ho);

   item->access_obj = ao;

   /* set owner widget item */
   ac = evas_object_data_get(ao, "_elm_access");
   ac->widget_item = item;
}

EAPI void
_elm_access_widget_item_unregister(Elm_Widget_Item *item)
{
   Evas_Object *ao;

   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);

   if (!item->access_obj) return;

   /* delete callbacks and unregister access object in _access_obj_del_cb*/
   ao = item->access_obj;
   item->access_obj = NULL;

   evas_object_del(ao);
}

EAPI Eina_Bool
_elm_access_2nd_click_timeout(Evas_Object *obj)
{
   Ecore_Timer *t;

   t = evas_object_data_get(obj, "_elm_2nd_timeout");
   if (t)
     {
        ecore_timer_del(t);
        evas_object_data_del(obj, "_elm_2nd_timeout");
        evas_object_event_callback_del_full(obj, EVAS_CALLBACK_DEL,
                                            _access_2nd_click_del_cb, NULL);
        return EINA_TRUE;
     }
   t = ecore_timer_add(0.3, _access_2nd_click_timeout_cb, obj);
   evas_object_data_set(obj, "_elm_2nd_timeout", t);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
                                  _access_2nd_click_del_cb, NULL);
   return EINA_FALSE;
}

static Evas_Object *
_elm_access_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

EOLIAN static void
_elm_access_eo_base_constructor(Eo *obj, void *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY));
}

EAPI Evas_Object *
elm_access_object_register(Evas_Object *obj, Evas_Object *parent)
{
   return _access_object_register(obj, parent);
}

EAPI void
elm_access_object_unregister(Evas_Object *obj)
{
   _access_object_unregister(obj);
}

EAPI Evas_Object *
elm_access_object_get(const Evas_Object *obj)
{
   return evas_object_data_get(obj, "_part_access_obj");
}

EAPI void
elm_access_info_set(Evas_Object *obj, int type, const char *text)
{
   _elm_access_text_set(_elm_access_info_get(obj), type, text);
}

EAPI char *
elm_access_info_get(const Evas_Object *obj, int type)
{
   return _elm_access_text_get(_elm_access_info_get(obj), type, obj);
}

EAPI void
elm_access_info_cb_set(Evas_Object *obj, int type,
                          Elm_Access_Info_Cb func, const void *data)
{
   _elm_access_callback_set(_elm_access_info_get(obj), type, func, data);
}

EAPI void
elm_access_activate_cb_set(Evas_Object *obj,
                           Elm_Access_Activate_Cb  func, void *data)
{
   Elm_Access_Info *ac;

   ac = _elm_access_info_get(obj);
   if (!ac) return;

   ac->activate = func;
   ac->activate_data = data;
}

EAPI void
elm_access_say(const char *text)
{
   if (!text) return;

   _elm_access_say(text);
}

EAPI void
elm_access_highlight_set(Evas_Object* obj)
{
   _elm_access_highlight_set(obj);
}

EAPI Eina_Bool
elm_access_action(Evas_Object *obj, const Elm_Access_Action_Type type, Elm_Access_Action_Info *action_info)
{
   Evas *evas;
   Evas_Object *ho;
   Elm_Access_Action_Info *a = action_info;

   switch (type)
     {
      case ELM_ACCESS_ACTION_READ:
      case ELM_ACCESS_ACTION_HIGHLIGHT:
        evas = evas_object_evas_get(obj);
        if (!evas) return EINA_FALSE;

        evas_event_feed_mouse_in(evas, 0, NULL);

        _elm_access_mouse_event_enabled_set(EINA_TRUE);
        evas_event_feed_mouse_move(evas, a->x, a->y, 0, NULL);
        _elm_access_mouse_event_enabled_set(EINA_FALSE);

        ho = _access_highlight_object_get(obj);
        if (ho)
          _access_action_callback_call(ho, ELM_ACCESS_ACTION_READ, a);
        break;

      case ELM_ACCESS_ACTION_UNHIGHLIGHT:
        evas = evas_object_evas_get(obj);
        if (!evas) return EINA_FALSE;
        _elm_access_object_highlight_disable(evas);
        break;

      case ELM_ACCESS_ACTION_HIGHLIGHT_NEXT:
        if (a->highlight_cycle)
          _elm_access_highlight_cycle(obj, ELM_FOCUS_NEXT);
        else
          return _access_highlight_next_get(obj, ELM_FOCUS_NEXT);
        break;

      case ELM_ACCESS_ACTION_HIGHLIGHT_PREV:
        if (a->highlight_cycle)
          _elm_access_highlight_cycle(obj, ELM_FOCUS_PREVIOUS);
        else
          return _access_highlight_next_get(obj, ELM_FOCUS_PREVIOUS);
        break;

      case ELM_ACCESS_ACTION_ACTIVATE:
        _elm_access_highlight_object_activate(obj, ELM_ACTIVATE_DEFAULT);
        break;

      case ELM_ACCESS_ACTION_UP:
        _elm_access_highlight_object_activate(obj, ELM_ACTIVATE_UP);
        break;

      case ELM_ACCESS_ACTION_DOWN:
        _elm_access_highlight_object_activate(obj, ELM_ACTIVATE_DOWN);
        break;

      case ELM_ACCESS_ACTION_SCROLL:
        //TODO: SCROLL HIGHLIGHT OBJECT
        break;

      case ELM_ACCESS_ACTION_BACK:
        break;

      default:
        break;
     }

   return EINA_TRUE;
}

EAPI void
elm_access_action_cb_set(Evas_Object *obj, const Elm_Access_Action_Type type, const Elm_Access_Action_Cb cb, const void *data)
{
   Action_Info *a;
   a =  evas_object_data_get(obj, "_elm_access_action_info");

   if (!a)
     {
        a = calloc(1, sizeof(Action_Info));
        evas_object_data_set(obj, "_elm_access_action_info", a);
     }

   a->obj = obj;
   a->fn[type].cb = cb;
   a->fn[type].user_data = (void *)data;
}
EAPI void
elm_access_external_info_set(Evas_Object *obj, const char *text)
{
   _elm_access_text_set
     (_elm_access_info_get(obj), ELM_ACCESS_CONTEXT_INFO, text);
}

EAPI char *
elm_access_external_info_get(const Evas_Object *obj)
{
   Elm_Access_Info *ac;

   ac = _elm_access_info_get(obj);
   return _elm_access_text_get(ac, ELM_ACCESS_CONTEXT_INFO, obj);
}

EAPI void
elm_access_highlight_next_set(Evas_Object *obj, Elm_Highlight_Direction dir, Evas_Object *next)
{
   EINA_SAFETY_ON_FALSE_RETURN(obj);
   EINA_SAFETY_ON_FALSE_RETURN(next);

   Elm_Access_Info *info = _elm_access_info_get(obj);
   Elm_Access_Info *info_next = _elm_access_info_get(next);

   if (!info || !info_next)
     {
        ERR("There is no access information");
        return;
     }

   if (dir == ELM_HIGHLIGHT_DIR_NEXT)
     {
        info_next->prev = obj;
        info->next = next;
     }
   else if (dir == ELM_HIGHLIGHT_DIR_PREVIOUS)
     {
        info_next->next = obj;
        info->prev = next;
     }
   else
      ERR("Not supported focus direction for access highlight [%d]", dir);
}

EOLIAN static void
_elm_access_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#include "elm_access.eo.c"
