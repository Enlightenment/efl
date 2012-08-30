#include <Elementary.h>
#include "elm_priv.h"

static const char ACCESS_SMART_NAME[] = "elm_access";

ELM_INTERNAL_SMART_SUBCLASS_NEW
  (ACCESS_SMART_NAME, _elm_access, Elm_Widget_Smart_Class,
  Elm_Widget_Smart_Class, elm_widget_smart_class_get, NULL);

static Evas_Object * _elm_access_add(Evas_Object *parent);

static void
_elm_access_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Widget_Smart_Data);
   ELM_WIDGET_CLASS(_elm_access_parent_sc)->base.add(obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);
}

static Eina_Bool
_elm_access_smart_on_focus(Evas_Object *obj)
{
   evas_object_focus_set(obj, elm_widget_focus_get(obj));

   return EINA_TRUE;
}

static void
_elm_access_smart_set_user(Elm_Widget_Smart_Class *sc)
{
   sc->base.add = _elm_access_smart_add;

   /* not a 'focus chain manager' */
   sc->focus_next = NULL;
   sc->focus_direction = NULL;
   sc->on_focus = _elm_access_smart_on_focus;

   return;
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
                  if (ai->data) eina_stringshare_del(ai->data);
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

static Eina_Bool
_access_obj_over_timeout_cb(void *data)
{
   Elm_Access_Info *ac = evas_object_data_get(data, "_elm_access");
   if (!ac) return EINA_FALSE;
   if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
     {
        if (ac->on_highlight) ac->on_highlight(ac->on_highlight_data);
        _elm_access_object_hilight(data);
        _elm_access_read(ac, ELM_ACCESS_CANCEL, data, NULL);
        _elm_access_read(ac, ELM_ACCESS_TYPE,   data, NULL);
        _elm_access_read(ac, ELM_ACCESS_INFO,   data, NULL);
        _elm_access_read(ac, ELM_ACCESS_STATE,  data, NULL);
        _elm_access_read(ac, ELM_ACCESS_DONE,   data, NULL);
     }
   ac->delay_timer = NULL;
   return EINA_FALSE;
}

static void
_access_obj_mouse_in_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info  __UNUSED__)
{
   Elm_Access_Info *ac = evas_object_data_get(data, "_elm_access");
   if (!ac) return;

   if (ac->delay_timer)
     {
        ecore_timer_del(ac->delay_timer);
        ac->delay_timer = NULL;
     }
   if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
      ac->delay_timer = ecore_timer_add(0.2, _access_obj_over_timeout_cb, data);
}

static void
_access_obj_mouse_out_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Access_Info *ac = evas_object_data_get(data, "_elm_access");
   if (!ac) return;
   _elm_access_object_unhilight(data);
   if (ac->delay_timer)
     {
        ecore_timer_del(ac->delay_timer);
        ac->delay_timer = NULL;
     }
}

static void
_access_obj_del_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Elm_Access_Info *ac;

   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_MOUSE_IN,
                                       _access_obj_mouse_in_cb, data);
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_MOUSE_OUT,
                                       _access_obj_mouse_out_cb, data);
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_DEL,
                                       _access_obj_del_cb, data);
   ac = evas_object_data_get(data, "_elm_access");
   evas_object_data_del(data, "_elm_access");
   if (ac)
     {
        _elm_access_clear(ac);
        free(ac);
     }
}

static void
_access_read_done(void *data __UNUSED__)
{
   printf("read done\n");
   // FIXME: produce event here
}

static void
_access_2nd_click_del_cb(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
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
_access_obj_hilight_del_cb(void *data __UNUSED__, Evas *e, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _elm_access_object_hilight_disable(e);
}

static void
_access_obj_hilight_hide_cb(void *data __UNUSED__, Evas *e, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _elm_access_object_hilight_disable(e);
}

static void
_access_obj_hilight_move_cb(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Coord x, y;
   Evas_Object *o;

   o = evas_object_name_find(evas_object_evas_get(obj), "_elm_access_disp");
   if (!o) return;
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   evas_object_move(o, x, y);
}

static void
_access_obj_hilight_resize_cb(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Coord w, h;
   Evas_Object *o;

   o = evas_object_name_find(evas_object_evas_get(obj), "_elm_access_disp");
   if (!o) return;
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   evas_object_resize(o, w, h);
}



//-------------------------------------------------------------------------//
EAPI void
_elm_access_highlight_set(Evas_Object* obj)
{
   _access_obj_over_timeout_cb(obj);
}

EAPI void
_elm_access_clear(Elm_Access_Info *ac)
{
   Elm_Access_Item *ai;

   if (!ac) return;
   if (ac->delay_timer)
     {
        ecore_timer_del(ac->delay_timer);
        ac->delay_timer = NULL;
     }
   EINA_LIST_FREE(ac->items, ai)
     {
        if (!ai->func)
          {
             if (ai->data) eina_stringshare_del(ai->data);
          }
        free(ai);
     }
}

EAPI void
_elm_access_text_set(Elm_Access_Info *ac, int type, const char *text)
{
   Elm_Access_Item *ai = _access_add_set(ac, type);
   if (!ai) return;
   ai->data = eina_stringshare_add(text);
}

EAPI void
_elm_access_callback_set(Elm_Access_Info *ac, int type, Elm_Access_Content_Cb func, const void *data)
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

EAPI char *
_elm_access_text_get(const Elm_Access_Info *ac, int type, Evas_Object *obj, Elm_Widget_Item *item)
{
   Elm_Access_Item *ai;
   Eina_List *l;

   if (!ac) return NULL;
   EINA_LIST_FOREACH(ac->items, l, ai)
     {
        if (ai->type == type)
          {
             if (ai->func) return ai->func((void *)(ai->data), obj, item);
             else if (ai->data) return strdup(ai->data);
             return NULL;
          }
     }
   return NULL;
}

EAPI void
_elm_access_read(Elm_Access_Info *ac, int type, Evas_Object *obj, Elm_Widget_Item *item)
{
   char *txt = _elm_access_text_get(ac, type, obj, item);

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
   if (txt) free(txt);
}

EAPI void
_elm_access_say(const char *txt)
{
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
_elm_access_object_get(const Evas_Object *obj)
{
   return evas_object_data_get(obj, "_elm_access");
}

EAPI Elm_Access_Info *
_elm_access_item_get(const Elm_Widget_Item *it)
{
   return it->access;
}

EAPI void
_elm_access_object_hilight(Evas_Object *obj)
{
   Evas_Object *o;
   Evas_Coord x, y, w, h;

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
             evas_object_event_callback_del_full(ptarget, EVAS_CALLBACK_DEL,
                                                 _access_obj_hilight_del_cb, NULL);
             evas_object_event_callback_del_full(ptarget, EVAS_CALLBACK_HIDE,
                                                 _access_obj_hilight_hide_cb, NULL);
             evas_object_event_callback_del_full(ptarget, EVAS_CALLBACK_MOVE,
                                                 _access_obj_hilight_move_cb, NULL);
             evas_object_event_callback_del_full(ptarget, EVAS_CALLBACK_RESIZE,
                                                 _access_obj_hilight_resize_cb, NULL);
          }
     }
   evas_object_data_set(o, "_elm_access_target", obj);
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
   evas_object_show(o);
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
     }
}

static void
_content_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj,
                void *event_info __UNUSED__)
{
   Evas_Object *accessobj;
   Evas_Coord w, h;

   accessobj = data;
   if (!accessobj) return;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   evas_object_resize(accessobj, w, h);
}

static void
_content_move(void *data, Evas *e __UNUSED__, Evas_Object *obj,
              void *event_info __UNUSED__)
{
   Evas_Object *accessobj;
   Evas_Coord x, y;

   accessobj = data;
   if (!accessobj) return;

   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   evas_object_move(accessobj, x, y);
}

static char *
_part_access_info_cb(void *data, Evas_Object *obj,
                Elm_Widget_Item *item __UNUSED__)
{
   Evas_Object *eobj = data;
   if (!eobj) return NULL;

   const char *part = evas_object_data_get(obj, "_elm_access_part");
   const char *txt = edje_object_part_text_get(eobj, part);
   if (txt) return strdup(txt);
   return NULL;
}

static void
_access_obj_del(void *data __UNUSED__, Evas *e __UNUSED__,
                Evas_Object *obj, void *event_info __UNUSED__)
{
   char *part = evas_object_data_get(obj, "_elm_access_part");
   evas_object_data_del(obj, "_elm_access_part");
   if (part) free(part);
}

EAPI Evas_Object *
_elm_access_edje_object_part_object_register(Evas_Object* obj,
                                             const Evas_Object *eobj,
                                             const char* part)
{
   Evas_Object *ao;
   Evas_Object *po = (Evas_Object *)edje_object_part_object_get(eobj, part);
   Evas_Coord x, y, w, h;

   if (!obj || !po) return NULL;

   // create access object
   ao = _elm_access_add(obj);
   evas_object_event_callback_add(po, EVAS_CALLBACK_RESIZE,
                                  _content_resize, ao);
   evas_object_event_callback_add(po, EVAS_CALLBACK_MOVE,
                                  _content_move, ao);

   evas_object_geometry_get(po, &x, &y, &w, &h);
   evas_object_move(ao, x, y);
   evas_object_resize(ao, w, h);
   evas_object_show(ao);

   // register access object
   _elm_access_object_register(ao, po);
   _elm_access_text_set(_elm_access_object_get(ao),
                        ELM_ACCESS_TYPE, evas_object_type_get(po));
   evas_object_data_set(ao, "_elm_access_part", strdup(part));
   evas_object_event_callback_add(ao, EVAS_CALLBACK_DEL,
                                  _access_obj_del, NULL);
   _elm_access_callback_set(_elm_access_object_get(ao),
                            ELM_ACCESS_INFO,
                            _part_access_info_cb, eobj);
   return ao;
}

EAPI void
_elm_access_object_hilight_disable(Evas *e)
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
}

EAPI void
_elm_access_object_register(Evas_Object *obj, Evas_Object *hoverobj)
{
   Elm_Access_Info *ac;

   evas_object_event_callback_add(hoverobj, EVAS_CALLBACK_MOUSE_IN,
                                  _access_obj_mouse_in_cb, obj);
   evas_object_event_callback_add(hoverobj, EVAS_CALLBACK_MOUSE_OUT,
                                  _access_obj_mouse_out_cb, obj);
   evas_object_event_callback_add(hoverobj, EVAS_CALLBACK_DEL,
                                  _access_obj_del_cb, obj);
   ac = calloc(1, sizeof(Elm_Access_Info));
   evas_object_data_set(obj, "_elm_access", ac);
}

static Eina_Bool
_access_item_over_timeout_cb(void *data)
{
   Elm_Access_Info *ac = ((Elm_Widget_Item *)data)->access;
   if (!ac) return EINA_FALSE;
   if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
     {
        _elm_access_object_hilight(((Elm_Widget_Item *)data)->view);
        _elm_access_read(ac, ELM_ACCESS_CANCEL, NULL, data);
        _elm_access_read(ac, ELM_ACCESS_TYPE,   NULL, data);
        _elm_access_read(ac, ELM_ACCESS_INFO,   NULL, data);
        _elm_access_read(ac, ELM_ACCESS_STATE,  NULL, data);
        _elm_access_read(ac, ELM_ACCESS_DONE,   NULL, data);
     }
   ac->delay_timer = NULL;
   return EINA_FALSE;
}

static void
_access_item_mouse_in_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info  __UNUSED__)
{
   Elm_Access_Info *ac = ((Elm_Widget_Item *)data)->access;
   if (!ac) return;

   if (ac->delay_timer)
     {
        ecore_timer_del(ac->delay_timer);
        ac->delay_timer = NULL;
     }
   if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
      ac->delay_timer = ecore_timer_add(0.2, _access_item_over_timeout_cb, data);
}

static void
_access_item_mouse_out_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Access_Info *ac = ((Elm_Widget_Item *)data)->access;
   if (!ac) return;

   _elm_access_object_unhilight(((Elm_Widget_Item *)data)->view);
   if (ac->delay_timer)
     {
        ecore_timer_del(ac->delay_timer);
        ac->delay_timer = NULL;
     }
}

static void _access_item_del_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__);

EAPI void
_elm_access_item_unregister(Elm_Widget_Item *item)
{
   Elm_Access_Info *ac;

   ac = item->access;
   if (ac)
     {
        evas_object_event_callback_del_full(ac->hoverobj,
                                            EVAS_CALLBACK_MOUSE_IN,
                                            _access_item_mouse_in_cb, item);
        evas_object_event_callback_del_full(ac->hoverobj,
                                            EVAS_CALLBACK_MOUSE_OUT,
                                            _access_item_mouse_out_cb, item);
        evas_object_event_callback_del_full(ac->hoverobj,
                                            EVAS_CALLBACK_DEL,
                                            _access_item_del_cb, item);
        item->access = NULL;
        _elm_access_clear(ac);
        free(ac);
     }
}

static void
_access_item_del_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _elm_access_item_unregister((Elm_Widget_Item *)data);
}

EAPI void
_elm_access_widget_item_register(Elm_Widget_Item *item)
{
   Evas_Object *ao;
   Evas_Object *ho = item->view;
   Evas_Coord x, y, w, h;

   if (!item) return;

   // create access object
   ao = _elm_access_add(item->widget);
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
}

EAPI void
_elm_access_widget_item_unregister(Elm_Widget_Item *item)
{
   Evas_Object *ho;

   if (!item || item->access_obj) return;

   ho = item->view;
   evas_object_event_callback_del_full(ho, EVAS_CALLBACK_RESIZE,
                                  _content_resize, item->access_obj);
   evas_object_event_callback_del_full(ho, EVAS_CALLBACK_MOVE,
                                  _content_move, item->access_obj);

   evas_object_del(item->access_obj);
}

EAPI void
_elm_access_item_register(Elm_Widget_Item *item, Evas_Object *hoverobj)
{
   Elm_Access_Info *ac;

   evas_object_event_callback_add(hoverobj, EVAS_CALLBACK_MOUSE_IN,
                                  _access_item_mouse_in_cb, item);
   evas_object_event_callback_add(hoverobj, EVAS_CALLBACK_MOUSE_OUT,
                                  _access_item_mouse_out_cb, item);
   evas_object_event_callback_add(hoverobj, EVAS_CALLBACK_DEL,
                                  _access_item_del_cb, item);
   ac = calloc(1, sizeof(Elm_Access_Info));
   ac->hoverobj = hoverobj;
   item->access = ac;
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
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_access_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}
