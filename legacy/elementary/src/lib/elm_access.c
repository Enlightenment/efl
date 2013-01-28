#include <Elementary.h>
#include "elm_priv.h"

#define MY_CLASS ELM_OBJ_ACCESS_CLASS

#define MY_CLASS_NAME "elm_access"

static Eina_Bool mouse_event_enable = EINA_TRUE;
static Eina_Bool read_mode = EINA_FALSE;

static Evas_Object * _elm_access_add(Evas_Object *parent);

static void
_elm_access_smart_add(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, evas_obj_smart_add());

   elm_widget_can_focus_set(obj, EINA_TRUE);
}

static void
_elm_access_smart_activate(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Activate act = va_arg(*list, Elm_Activate);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   if (act != ELM_ACTIVATE_DEFAULT) return;

   Elm_Access_Info *ac = evas_object_data_get(obj, "_elm_access");
   if (!ac) return;

   if (ac->activate)
     ac->activate(ac->activate_data, ac->part_object,
                  (Elm_Object_Item *)ac->widget_item);

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_access_smart_on_focus(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   evas_object_focus_set(obj, elm_widget_focus_get(obj));

   if (ret) *ret = EINA_TRUE;
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
        _elm_access_read(ac, ELM_ACCESS_CANCEL, data);
        _elm_access_read(ac, ELM_ACCESS_TYPE,   data);
        _elm_access_read(ac, ELM_ACCESS_INFO,   data);
        _elm_access_read(ac, ELM_ACCESS_STATE,  data);
        _elm_access_read(ac, ELM_ACCESS_CONTEXT_INFO, data);
        _elm_access_read(ac, ELM_ACCESS_DONE,   data);
     }
   ac->delay_timer = NULL;
   return EINA_FALSE;
}

static void
_access_obj_mouse_in_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info  __UNUSED__)
{
   Elm_Access_Info *ac;
   if (!mouse_event_enable) return;

    ac = evas_object_data_get(data, "_elm_access");
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
   Elm_Access_Info *ac;
   if (!mouse_event_enable) return;

   ac = evas_object_data_get(data, "_elm_access");
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
   _elm_access_object_unregister(data, obj);

   // _elm_access_edje_object_part_object_register(); set below object data
   evas_object_data_del(obj, "_part_access_obj");
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

static Evas_Object *
_access_highlight_object_get(Evas_Object *obj)
{
   Evas_Object *o, *ho;

   o = evas_object_name_find(evas_object_evas_get(obj), "_elm_access_disp");
   if (!o) return NULL;

   ho = evas_object_data_get(o, "_elm_access_target");

   return ho;
}

void _elm_access_mouse_event_enabled_set(Eina_Bool enabled)
{
   enabled = !!enabled;
   if (mouse_event_enable == enabled) return;
   mouse_event_enable = enabled;
}

void _elm_access_read_mode_set(Eina_Bool enabled)
{
   enabled = !!enabled;
   if (read_mode == enabled) return;
   read_mode = enabled;
}

Eina_Bool _elm_access_read_mode_get()
{
   return read_mode;
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
   Elm_Access_Info *ac;

   highlight = _access_highlight_object_get(obj);
   if (!highlight) return;

   if (elm_widget_is(highlight))
     {
        _elm_access_read_mode_set(EINA_FALSE);

        if (!elm_object_focus_get(highlight))
          elm_object_focus_set(highlight, EINA_TRUE);

        elm_widget_activate(highlight, act);
        return;
     }

   ac = evas_object_data_get(highlight, "_elm_access");
   if (!ac) return;

   if (ac->activate)
     ac->activate(ac->activate_data, highlight,
                  (Elm_Object_Item *)ac->widget_item);
}

EAPI void
_elm_access_highlight_cycle(Evas_Object *obj, Elm_Focus_Direction dir)
{
   _elm_access_read_mode_set(EINA_TRUE);
   elm_widget_focus_cycle(obj, dir);
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
             elm_widget_parent_highlight_set(ptarget, EINA_FALSE);

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
        elm_widget_parent_highlight_set(ptarget, EINA_FALSE);
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
   Evas_Object *ao;

   if (!obj) return;

   ao = evas_object_data_get(obj, "_part_access_obj");
   if (!ao) return;

   evas_object_data_del(obj, "_part_access_obj");

   /* delete callbacks */
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_RESIZE,
                                       _content_resize, ao);
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_MOVE,
                                       _content_move, ao);

   /* unregister access object */
   _elm_access_object_unregister(ao, obj);

   evas_object_del(ao);
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
_elm_access_edje_object_part_object_unregister(Evas_Object* obj __UNUSED__,
                                               const Evas_Object *eobj,
                                               const char* part)
{
   Evas_Object *po;

   po = (Evas_Object *)edje_object_part_object_get(eobj, part);
   if (!po) return;

   _access_object_unregister(po);
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
   elm_widget_parent_highlight_set(ptarget, EINA_FALSE);
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

EAPI void
_elm_access_object_unregister(Evas_Object *obj, Evas_Object *hoverobj)
{
   Elm_Access_Info *ac;

   evas_object_event_callback_del_full(hoverobj, EVAS_CALLBACK_MOUSE_IN,
                                       _access_obj_mouse_in_cb, obj);
   evas_object_event_callback_del_full(hoverobj, EVAS_CALLBACK_MOUSE_OUT,
                                       _access_obj_mouse_out_cb, obj);
   evas_object_event_callback_del_full(hoverobj, EVAS_CALLBACK_DEL,
                                       _access_obj_del_cb, obj);

   ac = evas_object_data_get(obj, "_elm_access");
   evas_object_data_del(obj, "_elm_access");
   if (ac)
     {
        _elm_access_clear(ac);
        free(ac);
     }
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
   Evas_Object *ho;

   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);

   if (!item->access_obj) return;

   ho = item->view;
   evas_object_event_callback_del_full(ho, EVAS_CALLBACK_RESIZE,
                                  _content_resize, item->access_obj);
   evas_object_event_callback_del_full(ho, EVAS_CALLBACK_MOVE,
                                  _content_move, item->access_obj);

   _elm_access_object_unregister(item->access_obj, ho);

   evas_object_del(item->access_obj);
   item->access_obj = NULL;
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

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME));

   Evas_Object *parent = eo_parent_get(obj);
   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);
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
   _elm_access_text_set(_elm_access_object_get(obj), type, text);
}

EAPI char *
elm_access_info_get(const Evas_Object *obj, int type)
{
   return _elm_access_text_get(_elm_access_object_get(obj), type, obj);
}

EAPI void
elm_access_info_cb_set(Evas_Object *obj, int type,
                          Elm_Access_Info_Cb func, const void *data)
{
   _elm_access_callback_set(_elm_access_object_get(obj), type, func, data);
}

EAPI void
elm_access_activate_cb_set(Evas_Object *obj,
                           Elm_Access_Activate_Cb  func, void *data)
{
   Elm_Access_Info *ac;

   ac = _elm_access_object_get(obj);
   if (!ac) return;

   ac->activate = func;
   ac->activate_data = data;
}

EAPI void
elm_access_external_info_set(Evas_Object *obj, const char *text)
{
   _elm_access_text_set
     (_elm_access_object_get(obj), ELM_ACCESS_CONTEXT_INFO, text);
}

EAPI char *
elm_access_external_info_get(const Evas_Object *obj)
{
   Elm_Access_Info *ac;

   ac = _elm_access_object_get(obj);
   return _elm_access_text_get(ac, ELM_ACCESS_CONTEXT_INFO, obj);
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_access_smart_add),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_ON_FOCUS), _elm_access_smart_on_focus),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_ACTIVATE), _elm_access_smart_activate),

        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     0,
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_obj_access_class_get, &class_desc, ELM_OBJ_WIDGET_CLASS, NULL);
