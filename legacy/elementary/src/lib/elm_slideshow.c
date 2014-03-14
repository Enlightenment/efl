#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_slideshow.h"

EAPI Eo_Op ELM_OBJ_SLIDESHOW_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_SLIDESHOW_CLASS

#define MY_CLASS_NAME "Elm_Slideshow"
#define MY_CLASS_NAME_LEGACY "elm_slideshow"

static const char SIG_CHANGED[] = "changed";
static const char SIG_TRANSITION_END[] = "transition,end";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {SIG_TRANSITION_END, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};

static void
_elm_slideshow_smart_event(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *src = va_arg(*list, Evas_Object *);
   Evas_Callback_Type type = va_arg(*list, Evas_Callback_Type);
   Evas_Event_Key_Down *ev = va_arg(*list, void *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);

   if (ret) *ret = EINA_FALSE;
   (void) src;

   if (elm_widget_disabled_get(obj)) return;
   if (type != EVAS_CALLBACK_KEY_DOWN) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   Elm_Slideshow_Smart_Data *sd = _pd;

   if ((!strcmp(ev->key, "Left")) ||
       ((!strcmp(ev->key, "KP_Left")) && (!ev->string)))
     {
        elm_slideshow_previous(obj);
        goto success;
     }
   else if ((!strcmp(ev->key, "Right")) ||
       ((!strcmp(ev->key, "KP_Right")) && (!ev->string)))
     {
        elm_slideshow_next(obj);
        goto success;
     }
   else if ((!strcmp(ev->key, "Return")) ||
            (!strcmp(ev->key, "KP_Enter")) ||
            (!strcmp(ev->key, "space")))
     {
        if (sd->timeout)
          {
             if (sd->timer)
               ELM_SAFE_FREE(sd->timer, ecore_timer_del);
             else
               elm_slideshow_timeout_set(obj, sd->timeout);
          }
        goto success;
     }

   return;

success:
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   if (ret) *ret = EINA_TRUE;
}

static void
_elm_slideshow_smart_sizing_eval(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Evas_Coord minw = -1, minh = -1;

   edje_object_size_min_calc(wd->resize_obj, &minw, &minh);
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
                       Evas *e EINA_UNUSED,
                       Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
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
        ELM_SAFE_FREE(VIEW(item), evas_object_del);
     }
}

static void
_on_slideshow_end(void *data,
                  Evas_Object *obj EINA_UNUSED,
                  const char *emission,
                  const char *source EINA_UNUSED)
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
   elm_layout_signal_emit(data, "elm,anim,end", "elm");
   // XXX: fort backwards compat
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
_elm_slideshow_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Slideshow_Smart_Data *priv = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   priv->count_item_pre_before = 2;
   priv->count_item_pre_after = 2;

   if (!elm_layout_theme_set
       (obj, "slideshow", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   priv->transitions = elm_widget_stringlist_get
       (edje_object_data_get(wd->resize_obj,
                             "transitions"));
   if (eina_list_count(priv->transitions) > 0)
     priv->transition =
       eina_stringshare_add(eina_list_data_get(priv->transitions));

   priv->layout.list = elm_widget_stringlist_get
       (edje_object_data_get(wd->resize_obj, "layouts"));

   if (eina_list_count(priv->layout.list) > 0)
     priv->layout.current = eina_list_data_get(priv->layout.list);

   edje_object_signal_callback_add
     (wd->resize_obj, "elm,end", "elm", _on_slideshow_end,
     obj);
   // XXX: for backwards compat :(
   edje_object_signal_callback_add
     (wd->resize_obj, "end", "slideshow", _on_slideshow_end,
     obj);

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_size_hints_changed, obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   elm_layout_sizing_eval(obj);
}

static void
_elm_slideshow_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Slideshow_Smart_Data *sd = _pd;
   const char *layout;

   elm_slideshow_clear(obj);
   elm_widget_stringlist_free(sd->transitions);
   ecore_timer_del(sd->timer);

   EINA_LIST_FREE(sd->layout.list, layout)
     eina_stringshare_del(layout);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EAPI Evas_Object *
elm_slideshow_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));
}

EAPI Elm_Object_Item *
elm_slideshow_item_add(Evas_Object *obj,
                       const Elm_Slideshow_Item_Class *itc,
                       const void *data)
{
   ELM_SLIDESHOW_CHECK(obj) NULL;
   Elm_Object_Item *ret;
   eo_do(obj, elm_obj_slideshow_item_add(itc, data, &ret));
   return ret;
}

static void
_elm_slideshow_item_add(Eo *obj, void *_pd, va_list *list)
{
   Elm_Slideshow_Item *item;

   Elm_Slideshow_Smart_Data *sd = _pd;

   const Elm_Slideshow_Item_Class *itc = va_arg(*list, const Elm_Slideshow_Item_Class *);
   const void *data = va_arg(*list, const void *);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   *ret = NULL;

   item = elm_widget_item_new(obj, Elm_Slideshow_Item);
   if (!item) return;

   elm_widget_item_del_pre_hook_set(item, _item_del_pre_hook);
   item->itc = itc;
   item->l = eina_list_append(item->l, item);
   elm_widget_item_data_set(item, data);

   sd->items = eina_list_merge(sd->items, item->l);

   if (!sd->current) elm_slideshow_item_show((Elm_Object_Item *)item);

   *ret = (Elm_Object_Item *)item;
}

EAPI Elm_Object_Item *
elm_slideshow_item_sorted_insert(Evas_Object *obj,
                                 const Elm_Slideshow_Item_Class *itc,
                                 const void *data,
                                 Eina_Compare_Cb func)
{
   ELM_SLIDESHOW_CHECK(obj) NULL;
   Elm_Object_Item *ret;
   eo_do(obj, elm_obj_slideshow_item_sorted_insert(itc, data, func, &ret));
   return ret;
}

static void
_elm_slideshow_item_sorted_insert(Eo *obj, void *_pd, va_list *list)
{
   const Elm_Slideshow_Item_Class *itc = va_arg(*list, const Elm_Slideshow_Item_Class *);
   const void *data = va_arg(*list, const void *);
   Eina_Compare_Cb func = va_arg(*list, Eina_Compare_Cb);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   *ret = NULL;

   Elm_Slideshow_Item *item;

   Elm_Slideshow_Smart_Data *sd = _pd;

   item = elm_widget_item_new(obj, Elm_Slideshow_Item);
   if (!item) return;

   elm_widget_item_del_pre_hook_set(item, _item_del_pre_hook);
   item->itc = itc;
   item->l = eina_list_append(item->l, item);
   elm_widget_item_data_set(item, data);

   sd->items = eina_list_sorted_merge(sd->items, item->l, func);

   if (!sd->current) elm_slideshow_item_show((Elm_Object_Item *)item);

   *ret = (Elm_Object_Item *)item;
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

   ELM_SAFE_FREE(sd->timer, ecore_timer_del);
   if (sd->timeout > 0.0)
     sd->timer = ecore_timer_add(sd->timeout, _timer_cb, WIDGET(item));

   _item_realize(next);
   elm_layout_content_set(WIDGET(item), "elm.swallow.2", VIEW(next));

   if (!sd->transition)
     sprintf(buf, "elm,none,next");
   else
     snprintf(buf, sizeof(buf), "elm,%s,next", sd->transition);
   elm_layout_signal_emit(WIDGET(item), buf, "elm");
   // XXX: for backwards compat
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
   ELM_SLIDESHOW_CHECK(obj);
   eo_do(obj, elm_obj_slideshow_next());
}

static void
_elm_slideshow_next(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   char buf[1024];
   Elm_Slideshow_Item *next = NULL;

   Elm_Slideshow_Smart_Data *sd = _pd;

   if (sd->current) next = _item_next_get(sd->current);

   if ((!next) || (next == sd->current)) return;

   _on_slideshow_end(obj, obj, NULL, NULL);

   ELM_SAFE_FREE(sd->timer, ecore_timer_del);
   if (sd->timeout > 0.0)
     sd->timer = ecore_timer_add(sd->timeout, _timer_cb, obj);

   _item_realize(next);

   elm_layout_content_set(obj, "elm.swallow.2", VIEW(next));

   if (!sd->transition)
     sprintf(buf, "elm,none,next");
   else
     snprintf(buf, sizeof(buf), "elm,%s,next", sd->transition);
   elm_layout_signal_emit(obj, buf, "elm");
   // XXX: for backwards compat
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
   ELM_SLIDESHOW_CHECK(obj);
   eo_do(obj, elm_obj_slideshow_previous());
}

static void
_elm_slideshow_previous(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   char buf[1024];
   Elm_Slideshow_Item *prev = NULL;

   Elm_Slideshow_Smart_Data *sd = _pd;

   if (sd->current) prev = _item_prev_get(sd->current);

   if ((!prev) || (prev == sd->current)) return;

   _on_slideshow_end(obj, obj, NULL, NULL);

   ELM_SAFE_FREE(sd->timer, ecore_timer_del);
   if (sd->timeout > 0.0)
     sd->timer = ecore_timer_add(sd->timeout, _timer_cb, obj);

   _item_realize(prev);

   elm_layout_content_set(obj, "elm.swallow.2", VIEW(prev));

   if (!sd->transition)
     sprintf(buf, "elm,none,previous");
   else
     snprintf(buf, 1024, "elm,%s,previous", sd->transition);
   elm_layout_signal_emit(obj, buf, "elm");
   // XXX: for backwards compat
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
   const Eina_List *ret;
   eo_do((Eo *) obj, elm_obj_slideshow_transitions_get(&ret));
   return ret;
}

static void
_elm_slideshow_transitions_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const Eina_List **ret = va_arg(*list, const Eina_List **);
   Elm_Slideshow_Smart_Data *sd = _pd;
   *ret = sd->transitions;
}

EAPI const Eina_List *
elm_slideshow_layouts_get(const Evas_Object *obj)
{
   ELM_SLIDESHOW_CHECK(obj) NULL;

   const Eina_List *ret;
   eo_do((Eo *) obj, elm_obj_slideshow_layouts_get(&ret));
   return ret;
}

static void
_elm_slideshow_layouts_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const Eina_List **ret = va_arg(*list, const Eina_List **);
   Elm_Slideshow_Smart_Data *sd = _pd;
   *ret = sd->layout.list;
}

EAPI void
elm_slideshow_transition_set(Evas_Object *obj,
                             const char *transition)
{
   ELM_SLIDESHOW_CHECK(obj);
   eo_do(obj, elm_obj_slideshow_transition_set(transition));
}

static void
_elm_slideshow_transition_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *transition = va_arg(*list, const char *);
   Elm_Slideshow_Smart_Data *sd = _pd;
   eina_stringshare_replace(&sd->transition, transition);
}

EAPI const char *
elm_slideshow_transition_get(const Evas_Object *obj)
{
   ELM_SLIDESHOW_CHECK(obj) NULL;

   const char *ret;
   eo_do((Eo *) obj, elm_obj_slideshow_transition_get(&ret));
   return ret;
}

static void
_elm_slideshow_transition_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **ret = va_arg(*list, const char **);
   Elm_Slideshow_Smart_Data *sd = _pd;
   *ret = sd->transition;
}

EAPI void
elm_slideshow_timeout_set(Evas_Object *obj,
                          double timeout)
{
   ELM_SLIDESHOW_CHECK(obj);
   eo_do(obj, elm_obj_slideshow_timeout_set(timeout));
}

static void
_elm_slideshow_timeout_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double timeout = va_arg(*list, double);
   Elm_Slideshow_Smart_Data *sd = _pd;

   sd->timeout = timeout;

   ELM_SAFE_FREE(sd->timer, ecore_timer_del);
   if (timeout > 0.0)
     sd->timer = ecore_timer_add(timeout, _timer_cb, obj);
}

EAPI double
elm_slideshow_timeout_get(const Evas_Object *obj)
{
   ELM_SLIDESHOW_CHECK(obj) - 1.0;
   double ret;
   eo_do((Eo *) obj, elm_obj_slideshow_timeout_get(&ret));
   return ret;
}

static void
_elm_slideshow_timeout_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double *ret = va_arg(*list, double *);
   Elm_Slideshow_Smart_Data *sd = _pd;
   *ret = sd->timeout;
}

EAPI void
elm_slideshow_loop_set(Evas_Object *obj,
                       Eina_Bool loop)
{
   ELM_SLIDESHOW_CHECK(obj);
   eo_do(obj, elm_obj_slideshow_loop_set(loop));
}

static void
_elm_slideshow_loop_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool loop = va_arg(*list, int);
   ELM_SLIDESHOW_CHECK(obj);
   Elm_Slideshow_Smart_Data *sd = _pd;
   sd->loop = loop;
}

EAPI const char *
elm_slideshow_layout_get(const Evas_Object *obj)
{
   ELM_SLIDESHOW_CHECK(obj) EINA_FALSE;
   const char *ret;
   eo_do((Eo *) obj, elm_obj_slideshow_layout_get(&ret));
   return ret;
}

static void
_elm_slideshow_layout_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **ret = va_arg(*list, const char **);
   Elm_Slideshow_Smart_Data *sd = _pd;
   *ret = sd->layout.current;
}

EAPI void
elm_slideshow_layout_set(Evas_Object *obj,
                         const char *layout)
{
   ELM_SLIDESHOW_CHECK(obj);
   eo_do(obj, elm_obj_slideshow_layout_set(layout));
}

static void
_elm_slideshow_layout_set(Eo *obj, void *_pd, va_list *list)
{

   char buf[PATH_MAX];
   const char *layout = va_arg(*list, const char *);

   Elm_Slideshow_Smart_Data *sd = _pd;

   sd->layout.current = layout;
   snprintf(buf, sizeof(buf), "elm,layout,%s", layout);
   elm_layout_signal_emit(obj, buf, "elm");
   // XXX: for bakcwards compat
   snprintf(buf, sizeof(buf), "layout,%s", layout);
   elm_layout_signal_emit(obj, buf, "slideshow");
}

EAPI Eina_Bool
elm_slideshow_loop_get(const Evas_Object *obj)
{
   ELM_SLIDESHOW_CHECK(obj) EINA_FALSE;
   Eina_Bool ret;
   eo_do((Eo *) obj, elm_obj_slideshow_loop_get(&ret));
   return ret;
}

static void
_elm_slideshow_loop_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Slideshow_Smart_Data *sd = _pd;
   *ret = sd->loop;
}

EAPI void
elm_slideshow_clear(Evas_Object *obj)
{
   ELM_SLIDESHOW_CHECK(obj);
   eo_do(obj, elm_obj_slideshow_clear());
}

static void
_elm_slideshow_clear(Eo *obj EINA_UNUSED, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Slideshow_Item *item;

   Elm_Slideshow_Smart_Data *sd = _pd;

   sd->previous = NULL;
   sd->current = NULL;
   EINA_LIST_FREE(sd->items_built, item)
     {
        if (item->itc->func.del)
          item->itc->func.del(elm_widget_item_data_get(item), VIEW(item));
     }

   EINA_LIST_FREE(sd->items, item)
     elm_widget_item_free(item);
}

EAPI const Eina_List *
elm_slideshow_items_get(const Evas_Object *obj)
{
   ELM_SLIDESHOW_CHECK(obj) NULL;
   const Eina_List *ret;
   eo_do((Eo *) obj, elm_obj_slideshow_items_get(&ret));
   return ret;
}

static void
_elm_slideshow_items_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const Eina_List **ret = va_arg(*list, const Eina_List **);
   Elm_Slideshow_Smart_Data *sd = _pd;
   *ret = sd->items;
}

EAPI Elm_Object_Item *
elm_slideshow_item_current_get(const Evas_Object *obj)
{
   ELM_SLIDESHOW_CHECK(obj) NULL;
   Elm_Object_Item *ret;
   eo_do((Eo *) obj, elm_obj_slideshow_item_current_get(&ret));
   return ret;
}

static void
_elm_slideshow_item_current_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   Elm_Slideshow_Smart_Data *sd = _pd;
   *ret = (Elm_Object_Item *)sd->current;
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
   int ret;
   eo_do((Eo *) obj, elm_obj_slideshow_cache_before_get(&ret));
   return ret;
}

static void
_elm_slideshow_cache_before_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);
   Elm_Slideshow_Smart_Data *sd = _pd;
   *ret = sd->count_item_pre_before;
}

EAPI void
elm_slideshow_cache_before_set(Evas_Object *obj, int count)
{
   ELM_SLIDESHOW_CHECK(obj);
   eo_do(obj, elm_obj_slideshow_cache_before_set(count));
}

static void
_elm_slideshow_cache_before_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int count = va_arg(*list, int);

   Elm_Slideshow_Smart_Data *sd = _pd;
   if (!sd) return;
   if (count < 0) count = 0;
   sd->count_item_pre_before = count;
}

EAPI int
elm_slideshow_cache_after_get(const Evas_Object *obj)
{
   ELM_SLIDESHOW_CHECK(obj) - 1;
   int ret;
   eo_do((Eo *) obj, elm_obj_slideshow_cache_after_get(&ret));
   return ret;
}

static void
_elm_slideshow_cache_after_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);
   Elm_Slideshow_Smart_Data *sd = _pd;
   *ret = sd->count_item_pre_after;
}

EAPI void
elm_slideshow_cache_after_set(Evas_Object *obj,
                              int count)
{
   ELM_SLIDESHOW_CHECK(obj);
   eo_do(obj, elm_obj_slideshow_cache_after_set(count));
}

static void
_elm_slideshow_cache_after_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int count = va_arg(*list, int);
   Elm_Slideshow_Smart_Data *sd = _pd;

   if (count < 0) count = 0;
   sd->count_item_pre_after = count;
}

EAPI Elm_Object_Item *
elm_slideshow_item_nth_get(const Evas_Object *obj,
                           unsigned int nth)
{
   ELM_SLIDESHOW_CHECK(obj) NULL;
   Elm_Object_Item *ret;
   eo_do((Eo *) obj, elm_obj_slideshow_item_nth_get(nth, &ret));
   return ret;
}

static void
 _elm_slideshow_item_nth_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   unsigned int nth = va_arg(*list, unsigned int);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);

   Elm_Slideshow_Smart_Data *sd = _pd;
   *ret = eina_list_nth(sd->items, nth);
}

EAPI unsigned int
elm_slideshow_count_get(const Evas_Object *obj)
{
   ELM_SLIDESHOW_CHECK(obj) 0;
   unsigned int ret;
   eo_do((Eo *) obj, elm_obj_slideshow_count_get(&ret));
   return ret;
}

static void
_elm_slideshow_count_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   unsigned int *ret = va_arg(*list, unsigned int *);
   Elm_Slideshow_Smart_Data *sd = _pd;
   *ret = eina_list_count(sd->items);
}

static void
_elm_slideshow_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_elm_slideshow_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_slideshow_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_slideshow_smart_del),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_EVENT), _elm_slideshow_smart_event),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_slideshow_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_slideshow_smart_focus_direction_manager_is),

        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_slideshow_smart_sizing_eval),

        EO_OP_FUNC(ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_ITEM_ADD), _elm_slideshow_item_add),
        EO_OP_FUNC(ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_ITEM_SORTED_INSERT), _elm_slideshow_item_sorted_insert),
        EO_OP_FUNC(ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_NEXT), _elm_slideshow_next),
        EO_OP_FUNC(ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_PREVIOUS), _elm_slideshow_previous),
        EO_OP_FUNC(ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_TRANSITIONS_GET), _elm_slideshow_transitions_get),
        EO_OP_FUNC(ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_LAYOUTS_GET), _elm_slideshow_layouts_get),
        EO_OP_FUNC(ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_TRANSITION_SET), _elm_slideshow_transition_set),
        EO_OP_FUNC(ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_TRANSITION_GET), _elm_slideshow_transition_get),
        EO_OP_FUNC(ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_TIMEOUT_SET), _elm_slideshow_timeout_set),
        EO_OP_FUNC(ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_TIMEOUT_GET), _elm_slideshow_timeout_get),
        EO_OP_FUNC(ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_LOOP_SET), _elm_slideshow_loop_set),
        EO_OP_FUNC(ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_LAYOUT_GET), _elm_slideshow_layout_get),
        EO_OP_FUNC(ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_LAYOUT_SET), _elm_slideshow_layout_set),
        EO_OP_FUNC(ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_LOOP_GET), _elm_slideshow_loop_get),
        EO_OP_FUNC(ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_CLEAR), _elm_slideshow_clear),
        EO_OP_FUNC(ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_ITEMS_GET), _elm_slideshow_items_get),
        EO_OP_FUNC(ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_ITEM_CURRENT_GET), _elm_slideshow_item_current_get),
        EO_OP_FUNC(ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_CACHE_BEFORE_GET), _elm_slideshow_cache_before_get),
        EO_OP_FUNC(ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_CACHE_BEFORE_SET), _elm_slideshow_cache_before_set),
        EO_OP_FUNC(ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_CACHE_AFTER_GET), _elm_slideshow_cache_after_get),
        EO_OP_FUNC(ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_CACHE_AFTER_SET), _elm_slideshow_cache_after_set),
        EO_OP_FUNC(ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_ITEM_NTH_GET), _elm_slideshow_item_nth_get),
        EO_OP_FUNC(ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_COUNT_GET), _elm_slideshow_count_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}
static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_SLIDESHOW_SUB_ID_ITEM_ADD, "Add (append) a new item in a given slideshow widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_SLIDESHOW_SUB_ID_ITEM_SORTED_INSERT, "Insert a new item into the given slideshow widget, using the func function to sort items (by item handles)."),
     EO_OP_DESCRIPTION(ELM_OBJ_SLIDESHOW_SUB_ID_NEXT, "Slide to the next item, in a given slideshow widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_SLIDESHOW_SUB_ID_PREVIOUS, "Slide to the previous item, in a given slideshow widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_SLIDESHOW_SUB_ID_TRANSITIONS_GET, "Returns the list of sliding transition/effect names available, for a given slideshow widget.."),
     EO_OP_DESCRIPTION(ELM_OBJ_SLIDESHOW_SUB_ID_LAYOUTS_GET, "Returns the list of layout names available, for a given slideshow widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_SLIDESHOW_SUB_ID_TRANSITION_SET, "Set the current slide transition/effect in use for a given slideshow widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_SLIDESHOW_SUB_ID_TRANSITION_GET, "Get the current slide transition/effect in use for a given slideshow widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_SLIDESHOW_SUB_ID_TIMEOUT_SET, "Set the interval between each image transition on a given slideshow widget, and start the slideshow, itself."),
     EO_OP_DESCRIPTION(ELM_OBJ_SLIDESHOW_SUB_ID_TIMEOUT_GET, "Get the interval set for image transitions on a given slideshow widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_SLIDESHOW_SUB_ID_LOOP_SET, "Set if, after a slideshow is started, for a given slideshow widget, its items should be displayed cyclically or not."),
     EO_OP_DESCRIPTION(ELM_OBJ_SLIDESHOW_SUB_ID_LAYOUT_GET, "Get the current slide layout in use for a given slideshow widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_SLIDESHOW_SUB_ID_LAYOUT_SET, "Set the current slide layout in use for a given slideshow widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_SLIDESHOW_SUB_ID_LOOP_GET, "Get if, after a slideshow is started, for a given slideshow widget, its items are to be displayed cyclically or not."),
     EO_OP_DESCRIPTION(ELM_OBJ_SLIDESHOW_SUB_ID_CLEAR, "Remove all items from a given slideshow widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_SLIDESHOW_SUB_ID_ITEMS_GET, "Get the internal list of items in a given slideshow widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_SLIDESHOW_SUB_ID_ITEM_CURRENT_GET, "Returns the currently displayed item, in a given slideshow widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_SLIDESHOW_SUB_ID_CACHE_BEFORE_GET, "Retrieve the number of items to cache, on a given slideshow widget, before the current item."),
     EO_OP_DESCRIPTION(ELM_OBJ_SLIDESHOW_SUB_ID_CACHE_BEFORE_SET, "Set the number of items to cache, on a given slideshow widget, before the current item."),
     EO_OP_DESCRIPTION(ELM_OBJ_SLIDESHOW_SUB_ID_CACHE_AFTER_GET, "Retrieve the number of items to cache, on a given slideshow widget, after the current item."),
     EO_OP_DESCRIPTION(ELM_OBJ_SLIDESHOW_SUB_ID_CACHE_AFTER_SET, "Set the number of items to cache, on a given slideshow widget, after the current item."),
     EO_OP_DESCRIPTION(ELM_OBJ_SLIDESHOW_SUB_ID_ITEM_NTH_GET, "Get the the item, in a given slideshow widget, placed at position nth, in its internal items list."),
     EO_OP_DESCRIPTION(ELM_OBJ_SLIDESHOW_SUB_ID_COUNT_GET, "Get the number of items stored in a given slideshow widget."),
     EO_OP_DESCRIPTION_SENTINEL
};
static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_SLIDESHOW_BASE_ID, op_desc, ELM_OBJ_SLIDESHOW_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Slideshow_Smart_Data),
     _class_constructor,
     NULL
};
EO_DEFINE_CLASS(elm_obj_slideshow_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, NULL);
