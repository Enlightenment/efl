#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_slideshow.h"

#define MY_CLASS ELM_SLIDESHOW_CLASS

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

static Eina_Bool _key_action_move(Evas_Object *obj, const char *params);
static Eina_Bool _key_action_pause(Evas_Object *obj, const char *params);

static const Elm_Action key_actions[] = {
   {"move", _key_action_move},
   {"pause", _key_action_pause},
   {NULL, NULL}
};

static Eina_Bool
_key_action_move(Evas_Object *obj, const char *params)
{
   const char *dir = params;

   if (!strcmp(dir, "left"))
     {
        elm_slideshow_previous(obj);
     }
   else if (!strcmp(dir, "right"))
     {
        elm_slideshow_next(obj);
     }
   else return EINA_FALSE;
   return EINA_TRUE;
}

static Eina_Bool
_key_action_pause(Evas_Object *obj, const char *params EINA_UNUSED)
{
   ELM_SLIDESHOW_DATA_GET(obj, sd);

   if (sd->timeout)
     {
        if (sd->timer)
          ELM_SAFE_FREE(sd->timer, ecore_timer_del);
        else
          elm_slideshow_timeout_set(obj, sd->timeout);
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_slideshow_elm_widget_event(Eo *obj, Elm_Slideshow_Data *sd EINA_UNUSED, Evas_Object *src, Evas_Callback_Type type, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   (void) src;

   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

   if (!_elm_config_key_binding_call(obj, ev, key_actions))
     return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

EOLIAN static void
_elm_slideshow_elm_layout_sizing_eval(Eo *obj, Elm_Slideshow_Data *_pd EINA_UNUSED)
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

EOLIAN static void
_elm_slideshow_evas_object_smart_add(Eo *obj, Elm_Slideshow_Data *priv)
{
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

EOLIAN static void
_elm_slideshow_evas_object_smart_del(Eo *obj, Elm_Slideshow_Data *sd)
{
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

EOLIAN static void
_elm_slideshow_eo_base_constructor(Eo *obj, Elm_Slideshow_Data *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks));
}

EOLIAN static Elm_Object_Item*
_elm_slideshow_item_add(Eo *obj, Elm_Slideshow_Data *sd, const Elm_Slideshow_Item_Class *itc, const void *data)
{
   Elm_Slideshow_Item *item;

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

EOLIAN static Elm_Object_Item*
_elm_slideshow_item_sorted_insert(Eo *obj, Elm_Slideshow_Data *sd, const Elm_Slideshow_Item_Class *itc, const void *data, Eina_Compare_Cb func)
{
   Elm_Slideshow_Item *item;

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

EOLIAN static void
_elm_slideshow_next(Eo *obj, Elm_Slideshow_Data *sd)
{
   char buf[1024];
   Elm_Slideshow_Item *next = NULL;

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

EOLIAN static void
_elm_slideshow_previous(Eo *obj, Elm_Slideshow_Data *sd)
{
   char buf[1024];
   Elm_Slideshow_Item *prev = NULL;

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

EOLIAN static const Eina_List*
_elm_slideshow_transitions_get(Eo *obj EINA_UNUSED, Elm_Slideshow_Data *sd)
{
   return sd->transitions;
}

EOLIAN static const Eina_List*
_elm_slideshow_layouts_get(Eo *obj EINA_UNUSED, Elm_Slideshow_Data *sd)
{
   return sd->layout.list;
}

EOLIAN static void
_elm_slideshow_transition_set(Eo *obj EINA_UNUSED, Elm_Slideshow_Data *sd, const char *transition)
{
   eina_stringshare_replace(&sd->transition, transition);
}

EOLIAN static const char*
_elm_slideshow_transition_get(Eo *obj EINA_UNUSED, Elm_Slideshow_Data *sd)
{
   return sd->transition;
}

EOLIAN static void
_elm_slideshow_timeout_set(Eo *obj EINA_UNUSED, Elm_Slideshow_Data *sd, double timeout)
{
   sd->timeout = timeout;

   ELM_SAFE_FREE(sd->timer, ecore_timer_del);
   if (timeout > 0.0)
     sd->timer = ecore_timer_add(timeout, _timer_cb, obj);
}

EOLIAN static double
_elm_slideshow_timeout_get(Eo *obj EINA_UNUSED, Elm_Slideshow_Data *sd)
{
   return sd->timeout;
}

EOLIAN static void
_elm_slideshow_loop_set(Eo *obj, Elm_Slideshow_Data *sd, Eina_Bool loop)
{
   ELM_SLIDESHOW_CHECK(obj);
   sd->loop = loop;
}

EOLIAN static const char*
_elm_slideshow_layout_get(Eo *obj EINA_UNUSED, Elm_Slideshow_Data *sd)
{
   return sd->layout.current;
}

EOLIAN static void
_elm_slideshow_layout_set(Eo *obj, Elm_Slideshow_Data *sd, const char *layout)
{
   char buf[PATH_MAX];


   sd->layout.current = layout;
   snprintf(buf, sizeof(buf), "elm,layout,%s", layout);
   elm_layout_signal_emit(obj, buf, "elm");
   // XXX: for bakcwards compat
   snprintf(buf, sizeof(buf), "layout,%s", layout);
   elm_layout_signal_emit(obj, buf, "slideshow");
}

EOLIAN static Eina_Bool
_elm_slideshow_loop_get(Eo *obj EINA_UNUSED, Elm_Slideshow_Data *sd)
{
   return sd->loop;
}

EOLIAN static void
_elm_slideshow_clear(Eo *obj EINA_UNUSED, Elm_Slideshow_Data *sd)
{
   Elm_Slideshow_Item *item;

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

EOLIAN static const Eina_List*
_elm_slideshow_items_get(Eo *obj EINA_UNUSED, Elm_Slideshow_Data *sd)
{
   return sd->items;
}

EOLIAN static Elm_Object_Item*
_elm_slideshow_item_current_get(Eo *obj EINA_UNUSED, Elm_Slideshow_Data *sd)
{
   return (Elm_Object_Item *)sd->current;
}

EAPI Evas_Object *
elm_slideshow_item_object_get(const Elm_Object_Item *it)
{
   ELM_SLIDESHOW_ITEM_CHECK_OR_RETURN(it, NULL);
   return VIEW(it);
}

EOLIAN static int
_elm_slideshow_cache_before_get(Eo *obj EINA_UNUSED, Elm_Slideshow_Data *sd)
{
   return sd->count_item_pre_before;
}

EOLIAN static void
_elm_slideshow_cache_before_set(Eo *obj EINA_UNUSED, Elm_Slideshow_Data *sd, int count)
{
   if (!sd) return;
   if (count < 0) count = 0;
   sd->count_item_pre_before = count;
}

EOLIAN static int
_elm_slideshow_cache_after_get(Eo *obj EINA_UNUSED, Elm_Slideshow_Data *sd)
{
   return sd->count_item_pre_after;
}

EOLIAN static void
_elm_slideshow_cache_after_set(Eo *obj EINA_UNUSED, Elm_Slideshow_Data *sd, int count)
{
   if (count < 0) count = 0;
   sd->count_item_pre_after = count;
}

EOLIAN static Elm_Object_Item*
_elm_slideshow_item_nth_get(Eo *obj EINA_UNUSED, Elm_Slideshow_Data *sd, unsigned int nth)
{
   return eina_list_nth(sd->items, nth);
}

EOLIAN static unsigned int
_elm_slideshow_count_get(Eo *obj EINA_UNUSED, Elm_Slideshow_Data *sd)
{
   return eina_list_count(sd->items);
}

EOLIAN static Eina_Bool
_elm_slideshow_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Elm_Slideshow_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_slideshow_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, Elm_Slideshow_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static void
_elm_slideshow_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#include "elm_slideshow.eo.c"
