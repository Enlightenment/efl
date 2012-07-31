#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_slideshow.h"

EAPI const char ELM_SLIDESHOW_SMART_NAME[] = "elm_slideshow";

static const char SIG_CHANGED[] = "changed";
static const char SIG_TRANSITION_END[] = "transition,end";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {SIG_TRANSITION_END, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_SLIDESHOW_SMART_NAME, _elm_slideshow, Elm_Slideshow_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, _smart_callbacks);

static Eina_Bool
_elm_slideshow_smart_event(Evas_Object *obj,
                           Evas_Object *src __UNUSED__,
                           Evas_Callback_Type type,
                           void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;

   if (elm_widget_disabled_get(obj)) return EINA_FALSE;

   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

   ELM_SLIDESHOW_DATA_GET(obj, sd);

   if ((!strcmp(ev->keyname, "Left")) ||
       ((!strcmp(ev->keyname, "KP_Left")) && (!ev->string)))
     {
        elm_slideshow_previous(obj);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

        return EINA_TRUE;
     }

   if ((!strcmp(ev->keyname, "Right")) ||
       ((!strcmp(ev->keyname, "KP_Right")) && (!ev->string)))
     {
        elm_slideshow_next(obj);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

        return EINA_TRUE;
     }

   if ((!strcmp(ev->keyname, "Return")) ||
       (!strcmp(ev->keyname, "KP_Enter")) ||
       (!strcmp(ev->keyname, "space")))
     {
        if (sd->timeout)
          {
             if (sd->timer)
               {
                  ecore_timer_del(sd->timer);
                  sd->timer = NULL;
               }
             else
               elm_slideshow_timeout_set(obj, sd->timeout);
          }
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

        return EINA_TRUE;
     }

   return EINA_FALSE;
}

static void
_elm_slideshow_smart_sizing_eval(Evas_Object *obj)
{
   ELM_SLIDESHOW_DATA_GET(obj, sd);
   Evas_Coord minw = -1, minh = -1;

   edje_object_size_min_calc(ELM_WIDGET_DATA(sd)->resize_obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, minw, minh);
}

static Elm_Slideshow_Item *
_item_prev_get(Elm_Slideshow_Item *item)
{
   ELM_SLIDESHOW_DATA_GET(WIDGET(item), sd);
   Elm_Slideshow_Item *prev = eina_list_data_get(eina_list_prev(item->l));

   if ((!prev) && (sd->loop))
     prev = eina_list_data_get(eina_list_last(item->l));

   return prev;
}

static Elm_Slideshow_Item *
_item_next_get(Elm_Slideshow_Item *item)
{
   ELM_SLIDESHOW_DATA_GET(WIDGET(item), sd);
   Elm_Slideshow_Item *next = eina_list_data_get(eina_list_next(item->l));

   if ((!next) && (sd->loop))
     next = eina_list_data_get(sd->items);

   return next;
}

static void
_on_size_hints_changed(void *data,
                       Evas *e __UNUSED__,
                       Evas_Object *obj __UNUSED__,
                       void *event_info __UNUSED__)
{
   elm_layout_sizing_eval(data);
}

static void
_item_realize(Elm_Slideshow_Item *item)
{
   Elm_Slideshow_Item *_item_prev, *_item_next;
   Evas_Object *obj = WIDGET(item);
   int ac, bc, lc, ic = 0;

   ELM_SLIDESHOW_DATA_GET_OR_RETURN(obj, sd);

   if ((!VIEW(item)) && (item->itc->func.get))
     {
        VIEW(item) = item->itc->func.get(elm_widget_item_data_get(item), obj);
        item->l_built = eina_list_append(NULL, item);
        sd->items_built = eina_list_merge(sd->items_built, item->l_built);
        //FIXME: item could be shown by obj
        evas_object_hide(VIEW(item));
     }
   else if (item->l_built)
     sd->items_built = eina_list_demote_list(sd->items_built, item->l_built);

   //pre-create previous and next item
   ac = sd->count_item_pre_after;
   _item_next = item;
   bc = sd->count_item_pre_before;
   _item_prev = item;
   lc = eina_list_count(sd->items) - 1;

   while (lc > 0 && ((ac > 0) || (bc > 0)))
     {
        if (lc > 0 && ac > 0)
          {
             --ac;
             --lc;
             if (_item_next)
               {
                  _item_next = _item_next_get(_item_next);
                  if ((_item_next)
                      && (!VIEW(_item_next))
                      && (_item_next->itc->func.get))
                    {
                       ic++;
                       VIEW(_item_next) =
                         _item_next->itc->func.get(
                           elm_widget_item_data_get(_item_next), obj);
                       _item_next->l_built =
                         eina_list_append(NULL, _item_next);
                       sd->items_built = eina_list_merge
                           (sd->items_built, _item_next->l_built);
                       //FIXME: _item_next could be shown by obj later
                       evas_object_hide(VIEW(_item_next));
                    }
                  else if (_item_next && _item_next->l_built)
                    {
                       ic++;
                       sd->items_built =
                         eina_list_demote_list
                           (sd->items_built, _item_next->l_built);
                    }
               }
          }

        if (lc > 0 && bc > 0)
          {
             --bc;
             --lc;
             if (_item_prev)
               {
                  _item_prev = _item_prev_get(_item_prev);
                  if ((_item_prev)
                      && (!VIEW(_item_prev))
                      && (_item_prev->itc->func.get))
                    {
                       ic++;
                       VIEW(_item_prev) =
                         _item_prev->itc->func.get(
                           elm_widget_item_data_get(_item_prev), obj);
                       _item_prev->l_built =
                         eina_list_append(NULL, _item_prev);
                       sd->items_built = eina_list_merge
                           (sd->items_built, _item_prev->l_built);
                       //FIXME: _item_prev could be shown by obj later
                       evas_object_hide(VIEW(_item_prev));
                    }
                  else if (_item_prev && _item_prev->l_built)
                    {
                       ic++;
                       sd->items_built =
                         eina_list_demote_list
                           (sd->items_built, _item_prev->l_built);
                    }
               }
          }
     }

   //delete unused items
   lc = ic + 1;

   while ((int)eina_list_count(sd->items_built) > lc)
     {
        item = eina_list_data_get(sd->items_built);
        sd->items_built = eina_list_remove_list
            (sd->items_built, sd->items_built);
        if (item->itc->func.del)
          item->itc->func.del(elm_widget_item_data_get(item), VIEW(item));
        evas_object_del(VIEW(item));
        VIEW(item) = NULL;
     }
}

static void
_on_slideshow_end(void *data,
                  Evas_Object *obj __UNUSED__,
                  const char *emission,
                  const char *source __UNUSED__)
{
   Elm_Slideshow_Item *item;
   ELM_SLIDESHOW_DATA_GET(data, sd);

   item = sd->previous;
   if (item)
     {
        elm_layout_content_unset(data, "elm.swallow.1");
        evas_object_hide(VIEW(item));
        sd->previous = NULL;
     }

   item = sd->current;
   if ((!item) || (!VIEW(item))) return;

   _item_realize(item);
   elm_layout_content_unset(data, "elm.swallow.2");

   elm_layout_content_set(data, "elm.swallow.1", VIEW(item));
   elm_layout_signal_emit(data, "anim,end", "slideshow");

   if (emission != NULL)
     evas_object_smart_callback_call(data, SIG_TRANSITION_END, sd->current);
}

static Eina_Bool
_timer_cb(void *data)
{
   Evas_Object *obj = data;
   ELM_SLIDESHOW_DATA_GET(obj, sd);

   sd->timer = NULL;
   elm_slideshow_next(obj);

   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   Elm_Slideshow_Item *item = (Elm_Slideshow_Item *)it;
   ELM_SLIDESHOW_DATA_GET_OR_RETURN_VAL(WIDGET(item), sd, EINA_FALSE);

   if (sd->previous == item) sd->previous = NULL;
   if (sd->current == item)
     {
        Eina_List *l = eina_list_data_find_list(sd->items, item);
        Eina_List *l2 = eina_list_next(l);
        sd->current = NULL;
        if (!l2)
          {
             l2 = eina_list_prev(l);
             if (l2)
               elm_slideshow_item_show(eina_list_data_get(l2));
          }
        else
          elm_slideshow_item_show(eina_list_data_get(l2));
     }

   sd->items = eina_list_remove_list(sd->items, item->l);
   sd->items_built = eina_list_remove_list(sd->items_built, item->l_built);

   if ((VIEW(item)) && (item->itc->func.del))
     item->itc->func.del(elm_widget_item_data_get(item), VIEW(item));

   return EINA_TRUE;
}

static void
_elm_slideshow_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Slideshow_Smart_Data);

   ELM_WIDGET_CLASS(_elm_slideshow_parent_sc)->base.add(obj);

   priv->current = NULL;
   priv->previous = NULL;

   priv->count_item_pre_before = 2;
   priv->count_item_pre_after = 2;

   elm_layout_theme_set(obj, "slideshow", "base", elm_widget_style_get(obj));

   priv->transitions = elm_widget_stringlist_get
       (edje_object_data_get(ELM_WIDGET_DATA(priv)->resize_obj,
                             "transitions"));
   if (eina_list_count(priv->transitions) > 0)
     priv->transition =
       eina_stringshare_add(eina_list_data_get(priv->transitions));

   priv->layout.list = elm_widget_stringlist_get
       (edje_object_data_get(ELM_WIDGET_DATA(priv)->resize_obj, "layouts"));

   if (eina_list_count(priv->layout.list) > 0)
     priv->layout.current = eina_list_data_get(priv->layout.list);

   edje_object_signal_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, "end", "slideshow", _on_slideshow_end,
     obj);

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_size_hints_changed, obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   elm_layout_sizing_eval(obj);
}

static void
_elm_slideshow_smart_del(Evas_Object *obj)
{
   ELM_SLIDESHOW_DATA_GET(obj, sd);
   const char *layout;

   elm_slideshow_clear(obj);
   elm_widget_stringlist_free(sd->transitions);
   if (sd->timer) ecore_timer_del(sd->timer);

   EINA_LIST_FREE (sd->layout.list, layout)
     eina_stringshare_del(layout);

   ELM_WIDGET_CLASS(_elm_slideshow_parent_sc)->base.del(obj);
}

static void
_elm_slideshow_smart_set_user(Elm_Slideshow_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_slideshow_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_slideshow_smart_del;

   ELM_WIDGET_CLASS(sc)->event = _elm_slideshow_smart_event;

   /* not a 'focus chain manager' */
   ELM_WIDGET_CLASS(sc)->focus_next = NULL;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_slideshow_smart_sizing_eval;
}

EAPI const Elm_Slideshow_Smart_Class *
elm_slideshow_smart_class_get(void)
{
   static Elm_Slideshow_Smart_Class _sc =
     ELM_SLIDESHOW_SMART_CLASS_INIT_NAME_VERSION(ELM_SLIDESHOW_SMART_NAME);
   static const Elm_Slideshow_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_slideshow_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_slideshow_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_slideshow_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI Elm_Object_Item *
elm_slideshow_item_add(Evas_Object *obj,
                       const Elm_Slideshow_Item_Class *itc,
                       const void *data)
{
   Elm_Slideshow_Item *item;

   ELM_SLIDESHOW_CHECK(obj) NULL;
   ELM_SLIDESHOW_DATA_GET(obj, sd);

   item = elm_widget_item_new(obj, Elm_Slideshow_Item);
   if (!item) return NULL;

   elm_widget_item_del_pre_hook_set(item, _item_del_pre_hook);
   item->itc = itc;
   item->l = eina_list_append(item->l, item);
   elm_widget_item_data_set(item, data);

   sd->items = eina_list_merge(sd->items, item->l);

   if (!sd->current) elm_slideshow_item_show((Elm_Object_Item *)item);

   return (Elm_Object_Item *)item;
}

EAPI Elm_Object_Item *
elm_slideshow_item_sorted_insert(Evas_Object *obj,
                                 const Elm_Slideshow_Item_Class *itc,
                                 const void *data,
                                 Eina_Compare_Cb func)
{
   Elm_Slideshow_Item *item;

   ELM_SLIDESHOW_CHECK(obj) NULL;
   ELM_SLIDESHOW_DATA_GET(obj, sd);

   item = elm_widget_item_new(obj, Elm_Slideshow_Item);
   if (!item) return NULL;

   elm_widget_item_del_pre_hook_set(item, _item_del_pre_hook);
   item->itc = itc;
   item->l = eina_list_append(item->l, item);
   elm_widget_item_data_set(item, data);

   sd->items = eina_list_sorted_merge(sd->items, item->l, func);

   if (!sd->current) elm_slideshow_item_show((Elm_Object_Item *)item);

   return (Elm_Object_Item *)item;
}

EAPI void
elm_slideshow_item_show(Elm_Object_Item *it)
{
   char buf[1024];
   Elm_Slideshow_Item *item, *next = NULL;

   ELM_SLIDESHOW_ITEM_CHECK(it);

   item = (Elm_Slideshow_Item *)it;
   ELM_SLIDESHOW_DATA_GET(WIDGET(item), sd);

   if (item == sd->current) return;

   next = item;
   _on_slideshow_end(WIDGET(item), WIDGET(item), NULL, NULL);

   if (sd->timer) ecore_timer_del(sd->timer);
   sd->timer = NULL;
   if (sd->timeout > 0.0)
     sd->timer = ecore_timer_add(sd->timeout, _timer_cb, WIDGET(item));

   _item_realize(next);
   elm_layout_content_set(WIDGET(item), "elm.swallow.2", VIEW(next));

   if (!sd->transition)
     sprintf(buf,"none,next");
   else
     snprintf(buf, sizeof(buf), "%s,next", sd->transition);
   elm_layout_signal_emit(WIDGET(item), buf, "slideshow");

   sd->previous = sd->current;
   sd->current = next;
   evas_object_smart_callback_call(WIDGET(item), SIG_CHANGED, sd->current);
}

EAPI void
elm_slideshow_next(Evas_Object *obj)
{
   char buf[1024];
   Elm_Slideshow_Item *next = NULL;

   ELM_SLIDESHOW_CHECK(obj);
   ELM_SLIDESHOW_DATA_GET(obj, sd);

   if (sd->current) next = _item_next_get(sd->current);

   if ((!next) || (next == sd->current)) return;

   _on_slideshow_end(obj, obj, NULL, NULL);

   if (sd->timer) ecore_timer_del(sd->timer);
   sd->timer = NULL;
   if (sd->timeout > 0.0)
     sd->timer = ecore_timer_add(sd->timeout, _timer_cb, obj);

   _item_realize(next);

   elm_layout_content_set(obj, "elm.swallow.2", VIEW(next));

   if (!sd->transition)
     sprintf(buf,"none,next");
   else
     snprintf(buf, sizeof(buf), "%s,next", sd->transition);
   elm_layout_signal_emit(obj, buf, "slideshow");

   sd->previous = sd->current;
   sd->current = next;
   evas_object_smart_callback_call(obj, SIG_CHANGED, sd->current);
}

EAPI void
elm_slideshow_previous(Evas_Object *obj)
{
   char buf[1024];
   Elm_Slideshow_Item *prev = NULL;

   ELM_SLIDESHOW_CHECK(obj);
   ELM_SLIDESHOW_DATA_GET(obj, sd);

   if (sd->current) prev = _item_prev_get(sd->current);

   if ((!prev) || (prev == sd->current)) return;

   _on_slideshow_end(obj, obj, NULL, NULL);

   if (sd->timer) ecore_timer_del(sd->timer);
   sd->timer = NULL;
   if (sd->timeout > 0.0)
     sd->timer = ecore_timer_add(sd->timeout, _timer_cb, obj);

   _item_realize(prev);

   elm_layout_content_set(obj, "elm.swallow.2", VIEW(prev));

   if (!sd->transition)
     sprintf(buf,"none,previous");
   else
     snprintf(buf, 1024, "%s,previous", sd->transition);
   elm_layout_signal_emit(obj, buf, "slideshow");

   sd->previous = sd->current;
   sd->current = prev;
   evas_object_smart_callback_call(obj, SIG_CHANGED, sd->current);
}

EAPI const Eina_List *
elm_slideshow_transitions_get(const Evas_Object *obj)
{
   ELM_SLIDESHOW_CHECK(obj) NULL;
   ELM_SLIDESHOW_DATA_GET(obj, sd);

   return sd->transitions;
}

EAPI const Eina_List *
elm_slideshow_layouts_get(const Evas_Object *obj)
{
   ELM_SLIDESHOW_CHECK(obj) NULL;
   ELM_SLIDESHOW_DATA_GET(obj, sd);

   return sd->layout.list;
}

EAPI void
elm_slideshow_transition_set(Evas_Object *obj,
                             const char *transition)
{
   ELM_SLIDESHOW_CHECK(obj);
   ELM_SLIDESHOW_DATA_GET(obj, sd);

   eina_stringshare_replace(&sd->transition, transition);
}

EAPI const char *
elm_slideshow_transition_get(const Evas_Object *obj)
{
   ELM_SLIDESHOW_CHECK(obj) NULL;
   ELM_SLIDESHOW_DATA_GET(obj, sd);

   return sd->transition;
}

EAPI void
elm_slideshow_timeout_set(Evas_Object *obj,
                          double timeout)
{
   ELM_SLIDESHOW_CHECK(obj);
   ELM_SLIDESHOW_DATA_GET(obj, sd);

   sd->timeout = timeout;

   if (sd->timer) ecore_timer_del(sd->timer);
   sd->timer = NULL;

   if (timeout > 0.0)
     sd->timer = ecore_timer_add(timeout, _timer_cb, obj);
}

EAPI double
elm_slideshow_timeout_get(const Evas_Object *obj)
{
   ELM_SLIDESHOW_CHECK(obj) - 1.0;
   ELM_SLIDESHOW_DATA_GET(obj, sd);

   return sd->timeout;
}

EAPI void
elm_slideshow_loop_set(Evas_Object *obj,
                       Eina_Bool loop)
{
   ELM_SLIDESHOW_CHECK(obj);
   ELM_SLIDESHOW_DATA_GET(obj, sd);

   sd->loop = loop;
}

EAPI const char *
elm_slideshow_layout_get(const Evas_Object *obj)
{
   ELM_SLIDESHOW_CHECK(obj) EINA_FALSE;
   ELM_SLIDESHOW_DATA_GET(obj, sd);

   return sd->layout.current;
}

EAPI void
elm_slideshow_layout_set(Evas_Object *obj,
                         const char *layout)
{
   char buf[PATH_MAX];

   ELM_SLIDESHOW_CHECK(obj);
   ELM_SLIDESHOW_DATA_GET(obj, sd);

   sd->layout.current = layout;
   snprintf(buf, sizeof(buf), "layout,%s", layout);
   elm_layout_signal_emit(obj, buf, "slideshow");
}

EAPI Eina_Bool
elm_slideshow_loop_get(const Evas_Object *obj)
{
   ELM_SLIDESHOW_CHECK(obj) EINA_FALSE;
   ELM_SLIDESHOW_DATA_GET(obj, sd);

   return sd->loop;
}

EAPI void
elm_slideshow_clear(Evas_Object *obj)
{
   Elm_Slideshow_Item *item;

   ELM_SLIDESHOW_CHECK(obj);
   ELM_SLIDESHOW_DATA_GET(obj, sd);

   sd->previous = NULL;
   sd->current = NULL;
   EINA_LIST_FREE (sd->items_built, item)
     {
        if (item->itc->func.del)
          item->itc->func.del(elm_widget_item_data_get(item), VIEW(item));
     }

   EINA_LIST_FREE (sd->items, item)
     elm_widget_item_free(item);
}

EAPI const Eina_List *
elm_slideshow_items_get(const Evas_Object *obj)
{
   ELM_SLIDESHOW_CHECK(obj) NULL;
   ELM_SLIDESHOW_DATA_GET(obj, sd);

   return sd->items;
}

EAPI Elm_Object_Item *
elm_slideshow_item_current_get(const Evas_Object *obj)
{
   ELM_SLIDESHOW_CHECK(obj) NULL;
   ELM_SLIDESHOW_DATA_GET(obj, sd);

   return (Elm_Object_Item *)sd->current;
}

EAPI Evas_Object *
elm_slideshow_item_object_get(const Elm_Object_Item *it)
{
   ELM_SLIDESHOW_ITEM_CHECK_OR_RETURN(it, NULL);
   return VIEW(it);
}

EAPI int
elm_slideshow_cache_before_get(const Evas_Object *obj)
{
   ELM_SLIDESHOW_CHECK(obj) - 1;
   ELM_SLIDESHOW_DATA_GET(obj, sd);

   return sd->count_item_pre_before;
}

EAPI void
elm_slideshow_cache_before_set(Evas_Object *obj, int count)
{
   ELM_SLIDESHOW_CHECK(obj);
   ELM_SLIDESHOW_DATA_GET(obj, sd);
   if (!sd) return;
   if (count < 0) count = 0;
   sd->count_item_pre_before = count;
}

EAPI int
elm_slideshow_cache_after_get(const Evas_Object *obj)
{
   ELM_SLIDESHOW_CHECK(obj) - 1;
   ELM_SLIDESHOW_DATA_GET(obj, sd);

   return sd->count_item_pre_after;
}

EAPI void
elm_slideshow_cache_after_set(Evas_Object *obj,
                              int count)
{
   ELM_SLIDESHOW_CHECK(obj);
   ELM_SLIDESHOW_DATA_GET(obj, sd);

   if (count < 0) count = 0;
   sd->count_item_pre_after = count;
}

EAPI Elm_Object_Item *
elm_slideshow_item_nth_get(const Evas_Object *obj,
                           unsigned int nth)
{
   ELM_SLIDESHOW_CHECK(obj) NULL;
   ELM_SLIDESHOW_DATA_GET(obj, sd);

   return eina_list_nth(sd->items, nth);
}

EAPI unsigned int
elm_slideshow_count_get(const Evas_Object *obj)
{
   ELM_SLIDESHOW_CHECK(obj) 0;
   ELM_SLIDESHOW_DATA_GET(obj, sd);

   return eina_list_count(sd->items);
}
