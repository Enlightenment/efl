#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "elm_priv.h"
#include "efl_ui_spotlight_plain_manager.eo.h"

typedef struct _Efl_Ui_Spotlight_Container_Data
{
   Eina_List *content_list;
   Eo *event;
   struct {
      Eina_Size2D sz;
   } page_spec;
   struct {
      Efl_Ui_Widget *page;
      double pos;
   } curr;
   struct {
     int from;
     int to;
     double last_pos;
     Eina_Bool active;
   } show_request;
   struct {
     Eina_Promise *transition_done;
     Efl_Gfx_Entity *content;
   } transition_done;
   Efl_Ui_Spotlight_Manager *transition;
   Efl_Ui_Spotlight_Indicator *indicator;
   Eina_Size2D min, max;
   double position;
   Eina_Bool fill_width: 1;
   Eina_Bool fill_height: 1;
   Eina_Bool prevent_transition_interaction : 1;
   Eina_Bool animation_enabled_internal : 1;
   Eina_Bool animation_enabled : 1;
} Efl_Ui_Spotlight_Container_Data;

#define MY_CLASS EFL_UI_SPOTLIGHT_CONTAINER_CLASS

static void
_fetch_partners(Eina_List *list, Eo *subobj, Eo **next, Eo **prev)
{
   Eina_List *node = eina_list_data_find_list(list, subobj);
   *next = eina_list_data_get(eina_list_next(node));
   *prev = eina_list_data_get(eina_list_prev(node));
}

static void _unpack(Eo *obj, Efl_Ui_Spotlight_Container_Data *pd, Efl_Gfx_Entity *subobj, int index);
static void _unpack_all(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Container_Data *pd, Eina_Bool clear);

static int
clamp_index(Efl_Ui_Spotlight_Container_Data *pd, int index)
{
   if (index < ((int)eina_list_count(pd->content_list)) * -1)
     return -1;
   else if (index > (int)eina_list_count(pd->content_list) - 1)
     return 1;
   return 0;
}

static int
index_rollover(Efl_Ui_Spotlight_Container_Data *pd, int index)
{
   int c = eina_list_count(pd->content_list);
   if (index < c * -1)
     return 0;
   else if (index > c - 1)
     return c - 1;
   else if (index < 0)
     return index + c;
   return index;
}

static void
_transition_end(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Container_Data *pd)
{
   Efl_Ui_Spotlight_Transition_Event ev;

   if (pd->prevent_transition_interaction) return;

   if (pd->transition_done.content)
     {
        Eina_Value v = eina_value_object_init(pd->transition_done.content);
        //first store the fields, then NULL them, then resolve the situation, otherwise we might get trapped in a endless recursion
        Eina_Promise *p = pd->transition_done.transition_done;
        Eo *content = pd->transition_done.content;
        pd->transition_done.transition_done = NULL;
        pd->transition_done.content = NULL;
        efl_pack_unpack(obj, content);
        eina_promise_resolve(p , v);
     }

   ev.from = pd->show_request.from;
   ev.to = pd->show_request.to;
   efl_event_callback_call(obj, EFL_UI_SPOTLIGHT_EVENT_TRANSITION_END, &ev);
   pd->show_request.active = EINA_FALSE;
   pd->show_request.from = -1;
   pd->show_request.to = -1;
}

static void
_transition_start(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Container_Data *pd, int from, int to, double progress)
{
   Efl_Ui_Spotlight_Transition_Event ev;

   if (pd->prevent_transition_interaction) return;

   if (pd->show_request.active)
     _transition_end(obj, pd);

   pd->show_request.active = EINA_TRUE;
   pd->show_request.from = from;
   pd->show_request.to = to;
   pd->show_request.last_pos = progress;
   ev.from = pd->show_request.from;
   ev.to = pd->show_request.to;
   efl_event_callback_call(obj, EFL_UI_SPOTLIGHT_EVENT_TRANSITION_START, &ev);
}

static void
_position_set(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Container_Data *pd, double progress)
{
   if (progress < -1.0) progress = -1.0;
   if (progress > eina_list_count(pd->content_list)) progress = eina_list_count(pd->content_list);
   if (pd->indicator)
     {
        efl_ui_spotlight_indicator_position_update(pd->indicator, progress);
     }
   pd->position = progress;
}

static void
_transition_event_emission(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Container_Data *pd)
{
   if (pd->show_request.active)
     {
        if ((pd->show_request.to != -1 || pd->show_request.from != -1) &&
            fabs(pd->show_request.last_pos - pd->show_request.to) < fabs(pd->position - pd->show_request.to))
          {
             //abort event here, movement is not in the direction we request
             pd->show_request.to = -1;
             _transition_end(obj, pd);
          }
        if (EINA_DBL_EQ(pd->position, pd->show_request.to))
          {
             //successfully there
             _transition_end(obj, pd);
          }
     }
   else
     {
        //the progress changed without a show_request beeing active. instaciate a new one
        _transition_start(obj, pd, -1, -1, pd->position);
     }
}

static void
_emit_page_size(Efl_Ui_Spotlight_Container *obj, Efl_Ui_Spotlight_Container_Data *pd)
{
   Eina_Size2D sz;

   sz = efl_gfx_entity_size_get(obj);

   if (!pd->fill_width)
     sz.w = MIN(pd->page_spec.sz.w, sz.w);

   if (!pd->fill_height)
     sz.h = MIN(pd->page_spec.sz.h, sz.h);

   if (pd->transition)
     efl_ui_spotlight_manager_size_set(pd->transition, sz);
}

static void
_resize_cb(void *data, const Efl_Event *ev)
{
   _emit_page_size(ev->object, data);
}

static void
_position_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   _emit_page_size(ev->object, data);
}

EFL_CALLBACKS_ARRAY_DEFINE(spotlight_resized,
  {EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _resize_cb},
  {EFL_GFX_ENTITY_EVENT_POSITION_CHANGED, _position_cb},
)

EOLIAN static Eo *
_efl_ui_spotlight_container_efl_object_constructor(Eo *obj,
                                     Efl_Ui_Spotlight_Container_Data *pd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   obj = efl_constructor(efl_super(obj, MY_CLASS));

   pd->max = EINA_SIZE2D(INT_MAX, INT_MAX);
   pd->animation_enabled = EINA_TRUE;
   pd->position = -1;
   pd->curr.page = NULL;
   pd->curr.pos = 0.0;

   pd->transition = NULL;
   pd->indicator = NULL;

   pd->fill_width = EINA_TRUE;
   pd->fill_height = EINA_TRUE;

   efl_ui_spotlight_size_set(obj, EINA_SIZE2D(-1, -1));

   elm_widget_can_focus_set(obj, EINA_FALSE);

   efl_event_callback_array_add(obj, spotlight_resized(), pd);
   return obj;
}

static void
_animated_transition_manager_eval(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Container_Data *pd)
{
   efl_ui_spotlight_manager_animated_transition_set(pd->transition, pd->animation_enabled_internal && pd->animation_enabled);
}

EOLIAN static Efl_Object*
_efl_ui_spotlight_container_efl_object_finalize(Eo *obj, Efl_Ui_Spotlight_Container_Data *pd)
{
   Efl_Ui_Spotlight_Manager *manager;

   obj = efl_finalize(efl_super(obj, MY_CLASS));
   pd->animation_enabled_internal = EINA_TRUE;
   manager = efl_ui_spotlight_manager_get(obj);
   //set a view manager in case nothing is here
   if (!manager)
     {
         efl_ui_spotlight_manager_set(obj, efl_new(EFL_UI_SPOTLIGHT_PLAIN_MANAGER_CLASS));
     }
   else
     {

        _animated_transition_manager_eval(obj, pd);
     }

   return obj;
}

EOLIAN static void
_efl_ui_spotlight_container_efl_object_invalidate(Eo *obj,
                                    Efl_Ui_Spotlight_Container_Data *pd)
{
   _unpack_all(obj, pd, EINA_TRUE);
   efl_invalidate(efl_super(obj, MY_CLASS));
}

EOLIAN static int
_efl_ui_spotlight_container_efl_container_content_count(Eo *obj EINA_UNUSED,
                                          Efl_Ui_Spotlight_Container_Data *pd)
{
   return eina_list_count(pd->content_list);
}

static void
_child_inv(void *data, const Efl_Event *ev)
{
   Efl_Ui_Spotlight_Container_Data *pd = efl_data_scope_get(data, MY_CLASS);
   int index = eina_list_data_idx(pd->content_list, ev->object);
   _unpack(data, pd, ev->object, index);
}

#define ADJUST_PRIVATE_MIN_MAX(obj, subobj, pd) \
  do \
    { \
       min = efl_gfx_hint_size_combined_min_get(subobj); \
       max = efl_gfx_hint_size_combined_max_get(subobj); \
       pd->min.w = MAX(pd->min.w, min.w); \
       pd->min.h = MAX(pd->min.h, min.h); \
       pd->max.w = MIN(pd->max.w, max.w); \
       pd->max.h = MIN(pd->max.h, max.h); \
    } \
  while(0)

#define FLUSH_MIN_MAX(obj, pd) \
  do \
    { \
       efl_gfx_hint_size_restricted_min_set(obj, pd->min); \
       efl_gfx_hint_size_restricted_max_set(obj, pd->max); \
    } \
  while(0)

static void
_hints_changed_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   efl_canvas_group_change(data);
}

EFL_CALLBACKS_ARRAY_DEFINE(children_evt,
  {EFL_EVENT_INVALIDATE, _child_inv},
  {EFL_GFX_ENTITY_EVENT_HINTS_CHANGED, _hints_changed_cb}
)

static Eina_Bool
_register_child(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Container_Data *pd, Efl_Gfx_Entity *subobj)
{
   Eina_Size2D min, max;
   EINA_SAFETY_ON_NULL_RETURN_VAL(subobj, EINA_FALSE);
   if (eina_list_data_find(pd->content_list, subobj))
     {
        ERR("Object %p is already part of this!", subobj);
        return EINA_FALSE;
     }
   if (!efl_ui_widget_sub_object_add(obj, subobj))
     return EINA_FALSE;

   efl_event_callback_array_add(subobj, children_evt(), obj);

   ADJUST_PRIVATE_MIN_MAX(obj, subobj, pd);
   FLUSH_MIN_MAX(obj, pd);

   return EINA_TRUE;
}

static void
_update_internals(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Container_Data *pd, Efl_Gfx_Entity *subobj EINA_UNUSED, int index)
{
   pd->prevent_transition_interaction = EINA_TRUE;
   if (pd->transition)
     efl_ui_spotlight_manager_content_add(pd->transition, subobj, index);
   if (pd->indicator)
     efl_ui_spotlight_indicator_content_add(pd->indicator, subobj, index);
   pd->prevent_transition_interaction = EINA_FALSE;
   if (eina_list_count(pd->content_list) == 1)
     efl_ui_spotlight_active_element_set(obj, subobj);
}

EOLIAN static Eina_Bool
_efl_ui_spotlight_container_efl_pack_linear_pack_begin(Eo *obj EINA_UNUSED,
                                         Efl_Ui_Spotlight_Container_Data *pd,
                                         Efl_Gfx_Entity *subobj)
{
   if (!_register_child(obj, pd, subobj)) return EINA_FALSE;
   pd->content_list = eina_list_prepend(pd->content_list, subobj);
   _update_internals(obj, pd, subobj, 0);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_spotlight_container_efl_pack_linear_pack_end(Eo *obj EINA_UNUSED,
                                       Efl_Ui_Spotlight_Container_Data *pd,
                                       Efl_Gfx_Entity *subobj)
{
   if (!_register_child(obj, pd, subobj)) return EINA_FALSE;
   pd->content_list = eina_list_append(pd->content_list, subobj);
   _update_internals(obj, pd, subobj, eina_list_count(pd->content_list) - 1);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_spotlight_container_efl_pack_linear_pack_before(Eo *obj EINA_UNUSED,
                                          Efl_Ui_Spotlight_Container_Data *pd,
                                          Efl_Gfx_Entity *subobj,
                                          const Efl_Gfx_Entity *existing)
{
   int index = eina_list_data_idx(pd->content_list, (void *)existing);
   if (existing)
     EINA_SAFETY_ON_FALSE_RETURN_VAL(index >= 0, EINA_FALSE);

   if (!_register_child(obj, pd, subobj)) return EINA_FALSE;
   pd->content_list = eina_list_prepend_relative(pd->content_list, subobj, existing);
   _update_internals(obj, pd, subobj, index);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_spotlight_container_efl_pack_linear_pack_after(Eo *obj EINA_UNUSED,
                                         Efl_Ui_Spotlight_Container_Data *pd,
                                         Efl_Gfx_Entity *subobj,
                                         const Efl_Gfx_Entity *existing)
{
   int index = eina_list_data_idx(pd->content_list, (void *)existing);
   if (existing)
     EINA_SAFETY_ON_FALSE_RETURN_VAL(index >= 0, EINA_FALSE);

   if (!_register_child(obj, pd, subobj)) return EINA_FALSE;
   pd->content_list = eina_list_append_relative(pd->content_list, subobj, existing);
   _update_internals(obj, pd, subobj, index + 1);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_spotlight_container_efl_pack_linear_pack_at(Eo *obj,
                                      Efl_Ui_Spotlight_Container_Data *pd,
                                      Efl_Gfx_Entity *subobj,
                                      int index)
{
   Efl_Gfx_Entity *existing = NULL;

   if (!_register_child(obj, pd, subobj)) return EINA_FALSE;
   int clamp = clamp_index(pd, index);
   int pass_index = -1;
   if (clamp == 0)
     {
        existing = eina_list_nth(pd->content_list, index_rollover(pd, index));
        pd->content_list = eina_list_prepend_relative(
           pd->content_list, subobj, existing);
     }
   else if (clamp == 1)
     {
        pd->content_list = eina_list_append(pd->content_list, subobj);
        pass_index = eina_list_count(pd->content_list);
     }
   else
     {
        pd->content_list = eina_list_prepend(pd->content_list, subobj);
        pass_index = 0;
     }
   _update_internals(obj, pd, subobj, pass_index);

   return EINA_TRUE;
}

EOLIAN static Efl_Gfx_Entity *
_efl_ui_spotlight_container_efl_pack_linear_pack_content_get(Eo *obj EINA_UNUSED,
                                               Efl_Ui_Spotlight_Container_Data *pd,
                                               int index)
{
   return eina_list_nth(pd->content_list, index_rollover(pd, index));
}

EOLIAN static int
_efl_ui_spotlight_container_efl_pack_linear_pack_index_get(Eo *obj EINA_UNUSED,
                                             Efl_Ui_Spotlight_Container_Data *pd,
                                             const Efl_Gfx_Entity *subobj)
{
   return eina_list_data_idx(pd->content_list, (void *)subobj);
}

static void
_active_element_set(Eo *obj, Efl_Ui_Spotlight_Container_Data *pd, Efl_Ui_Widget *new_page, Efl_Ui_Spotlight_Manager_Switch_Reason reason)
{
   int before = -1;
   int index;

   if (pd->curr.page)
     before = efl_pack_index_get(obj, pd->curr.page);
   index = efl_pack_index_get(obj, new_page);

   EINA_SAFETY_ON_FALSE_RETURN(index != -1);

   pd->show_request.last_pos = efl_pack_index_get(obj, pd->curr.page);
   pd->show_request.from = efl_pack_index_get(obj, pd->curr.page);
   pd->show_request.to = index;

   if (pd->show_request.active && pd->show_request.from == -1 && pd->show_request.to)
     pd->show_request.to = index; //we just edit this here, a user animation will end when the progress is at the goal.
   else
     {
        _transition_start(obj, pd, before, index, before);
     }

   pd->curr.page = new_page;
   efl_ui_spotlight_manager_switch_to(pd->transition, before, index, reason);

}

EOLIAN static void
_efl_ui_spotlight_container_active_element_set(Eo *obj EINA_UNUSED,
                               Efl_Ui_Spotlight_Container_Data *pd,
                               Efl_Ui_Widget *new_page)
{
   _active_element_set(obj, pd, new_page, EFL_UI_SPOTLIGHT_MANAGER_SWITCH_REASON_JUMP);
}

EOLIAN static Efl_Ui_Widget*
_efl_ui_spotlight_container_active_element_get(const Eo *obj EINA_UNUSED,
                               Efl_Ui_Spotlight_Container_Data *pd)
{
   return pd->curr.page;
}

EOLIAN Eina_Size2D
_efl_ui_spotlight_container_spotlight_size_get(const Eo *obj EINA_UNUSED,
                            Efl_Ui_Spotlight_Container_Data *pd)
{
   return pd->page_spec.sz;
}

EOLIAN static void
_efl_ui_spotlight_container_spotlight_size_set(Eo *obj,
                            Efl_Ui_Spotlight_Container_Data *pd,
                            Eina_Size2D sz)
{
   Eina_Size2D size;
   if (sz.w < -1 || sz.h < -1) return;

   pd->page_spec.sz = sz;
   pd->fill_width = sz.w == -1 ? EINA_TRUE : EINA_FALSE;
   pd->fill_height = sz.h == -1 ? EINA_TRUE : EINA_FALSE;

   size = efl_gfx_entity_size_get(obj);
   if (pd->fill_height)
     pd->page_spec.sz.h = size.h;
   if (pd->fill_width)
     pd->page_spec.sz.w = size.w;

   if (pd->transition)
     efl_ui_spotlight_manager_size_set(pd->transition, pd->page_spec.sz);
}

static void
_unpack_all(Eo *obj EINA_UNUSED,
            Efl_Ui_Spotlight_Container_Data *pd,
            Eina_Bool clear)
{
   pd->curr.page = NULL;

   while(pd->content_list)
     {
        Eo *content = eina_list_data_get(pd->content_list);

        if (clear)
          efl_del(content);
        else
          _unpack(obj, pd, content, 0);
     }
}

EOLIAN static Eina_Bool
_efl_ui_spotlight_container_efl_pack_pack_clear(Eo *obj EINA_UNUSED,
                                  Efl_Ui_Spotlight_Container_Data *pd)
{
   _unpack_all(obj, pd, EINA_TRUE);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_spotlight_container_efl_pack_unpack_all(Eo *obj EINA_UNUSED,
                                  Efl_Ui_Spotlight_Container_Data *pd)
{
   _unpack_all(obj, pd, EINA_FALSE);

   return EINA_TRUE;
}

static void
_unpack(Eo *obj,
        Efl_Ui_Spotlight_Container_Data *pd,
        Efl_Gfx_Entity *subobj,
        int index)
{
   int early_curr_page = efl_pack_index_get(obj, pd->curr.page);
   Eina_Bool deletion_of_active = (subobj == pd->curr.page);
   Efl_Ui_Widget *next, *prev;

   _fetch_partners(pd->content_list, subobj, &next, &prev);
   pd->content_list = eina_list_remove(pd->content_list, subobj);
   _elm_widget_sub_object_redirect_to_top(obj, subobj);

   if (!efl_alive_get(obj)) return;

   if (pd->transition)
     efl_ui_spotlight_manager_content_del(pd->transition, subobj, index);
   if (pd->indicator)
     efl_ui_spotlight_indicator_content_del(pd->indicator, subobj, index);

   //we deleted the current index
   if (deletion_of_active)
     {
        if (eina_list_count(pd->content_list) == 0)
          {
             pd->curr.page = NULL;
          }
        else
          {
             //when we delete the active index and we are not updating the index,
             // then force a update, so the same sort of animation is triggered from the right direction
             if (early_curr_page == efl_pack_index_get(obj, prev))
               pd->curr.page = eina_list_nth(pd->content_list, early_curr_page - 1);

             if (prev)
               efl_ui_spotlight_active_element_set(obj, prev);
             else
               efl_ui_spotlight_active_element_set(obj, next);
          }
     }

   //position has updated
   if (deletion_of_active &&
       pd->indicator && !pd->transition)
     efl_ui_spotlight_indicator_position_update(pd->indicator, efl_pack_index_get(obj, pd->curr.page));

   efl_event_callback_array_del(subobj, children_evt(), obj);
   efl_canvas_group_change(obj);
}

EOLIAN static Eina_Bool
_efl_ui_spotlight_container_efl_pack_unpack(Eo *obj,
                              Efl_Ui_Spotlight_Container_Data *pd,
                              Efl_Gfx_Entity *subobj)
{
   if (!subobj) return EINA_FALSE;

   int index = eina_list_data_idx(pd->content_list, subobj);
   if (index == -1)
     {
        ERR("Item %p is not part of this container", subobj);
        return EINA_FALSE;
     }

   _unpack(obj, pd, subobj, index);

   return EINA_TRUE;
}

EOLIAN static Efl_Gfx_Entity *
_efl_ui_spotlight_container_efl_pack_linear_pack_unpack_at(Eo *obj,
                                             Efl_Ui_Spotlight_Container_Data *pd,
                                             int index)
{
   Efl_Gfx_Entity *subobj = eina_list_nth(pd->content_list, index_rollover(pd, index_rollover(pd, index)));

   _unpack(obj, pd, subobj, index);

   return subobj;
}

EOLIAN static Eina_Bool
_efl_ui_spotlight_container_efl_pack_pack(Eo *obj, Efl_Ui_Spotlight_Container_Data *pd EINA_UNUSED, Efl_Gfx_Entity *subobj)
{
   return efl_pack_begin(obj, subobj);
}

EOLIAN static Eina_Iterator*
_efl_ui_spotlight_container_efl_container_content_iterate(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Container_Data *pd)
{
  return eina_list_iterator_new(pd->content_list);
}

static void
_pos_updated(void *data, const Efl_Event *event)
{
   Efl_Ui_Spotlight_Container_Data *pd = efl_data_scope_get(data, MY_CLASS);
   double progress = *((double*)event->info);
   //ignore this here, this could result in unintendet transition,start / end calls
   if (EINA_DBL_EQ(progress, pd->position))
     return;
   _position_set(data, pd, progress);
   _transition_event_emission(data, pd);
}

EOLIAN static void
_efl_ui_spotlight_container_spotlight_manager_set(Eo *obj, Efl_Ui_Spotlight_Container_Data *pd, Efl_Ui_Spotlight_Manager *transition)
{
   if (!transition)
     pd->transition = efl_add(EFL_UI_SPOTLIGHT_PLAIN_MANAGER_CLASS, obj);
   else
     EINA_SAFETY_ON_FALSE_RETURN(efl_isa(transition, EFL_UI_SPOTLIGHT_MANAGER_CLASS));

   if (pd->transition)
     {
        efl_ui_spotlight_manager_bind(pd->transition, NULL);
        efl_del(pd->transition);
     }

   pd->transition = transition;

   if (pd->transition)
     {
        EINA_SAFETY_ON_FALSE_RETURN(efl_ownable_get(pd->transition));
        efl_parent_set(pd->transition, obj);
        //the api indicates that the caller passes ownership to this function, so we need to unref here
        efl_unref(pd->transition);
        //disable animation when not finalized yet, this help reducing the overhead of scheduling a animation that will not be displayed
        _animated_transition_manager_eval(obj, pd);
        efl_ui_spotlight_manager_animated_transition_set(pd->transition, efl_finalized_get(obj));
        efl_ui_spotlight_manager_bind(pd->transition, obj);
        _emit_page_size(obj, pd);
        efl_event_callback_add(pd->transition, EFL_UI_SPOTLIGHT_MANAGER_EVENT_POS_UPDATE, _pos_updated, obj);
     }

}

EOLIAN static Efl_Ui_Spotlight_Manager*
_efl_ui_spotlight_container_spotlight_manager_get(const Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Container_Data *pd)
{
   if (efl_isa(pd->transition, EFL_UI_SPOTLIGHT_PLAIN_MANAGER_CLASS))
     return NULL;
   else
     return pd->transition;
}

EOLIAN static void
_efl_ui_spotlight_container_indicator_set(Eo *obj, Efl_Ui_Spotlight_Container_Data *pd, Efl_Ui_Spotlight_Indicator *indicator)
{
   if (pd->indicator)
     {
        efl_del(pd->indicator);
     }
   pd->indicator = indicator;
   if (pd->indicator)
     {
        EINA_SAFETY_ON_FALSE_RETURN(efl_ownable_get(pd->indicator));
        efl_parent_set(pd->indicator, obj);
        //the api indicates that the caller passes ownership to this function, so we need to unref here
        efl_unref(pd->indicator);
        efl_ui_spotlight_indicator_bind(pd->indicator, obj);
        if (!EINA_DBL_EQ(pd->position, -1))
          efl_ui_spotlight_indicator_position_update(pd->indicator, pd->position);
     }
}

EOLIAN static Efl_Ui_Spotlight_Indicator*
_efl_ui_spotlight_container_indicator_get(const Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Container_Data *pd)
{
   return pd->indicator;
}

EOLIAN static void
_efl_ui_spotlight_container_push(Eo *obj, Efl_Ui_Spotlight_Container_Data *pd EINA_UNUSED, Efl_Gfx_Entity *view)
{
   if (efl_ui_spotlight_active_element_get(obj))
     {
        if (!efl_pack_after(obj, view, efl_ui_spotlight_active_element_get(obj)))
          return;
     }
   else
     {
        if (!efl_pack_end(obj, view))
          return;
     }
   _active_element_set(obj, pd, view, EFL_UI_SPOTLIGHT_MANAGER_SWITCH_REASON_PUSH);
}

static Eina_Value
_delete_obj(void *data EINA_UNUSED, const Eina_Value value, const Eina_Future *dead_future EINA_UNUSED)
{
   efl_del(eina_value_object_get(&value));

   return EINA_VALUE_EMPTY;
}

EOLIAN static Eina_Future*
_efl_ui_spotlight_container_pop(Eo *obj, Efl_Ui_Spotlight_Container_Data *pd, Eina_Bool del)
{
   Eina_Future *transition_done;
   Eina_Value v;
   int new_index;
   int count;
   Eo *content;

   count = (int)eina_list_count(pd->content_list);

   if (count == 0) return NULL;

   content = efl_ui_spotlight_active_element_get(obj);

   //pop() unpacks content without transition if there is one content.
   if (count == 1)
     {
        efl_pack_unpack(obj, content);
        pd->curr.page = NULL;

        if (del)
          {
             efl_del(content);
             v = EINA_VALUE_EMPTY;
          }
        else
          {
             v = eina_value_object_init(content);
          }
        return efl_loop_future_resolved(obj, v);
     }

   new_index = efl_pack_index_get(obj, efl_ui_spotlight_active_element_get(obj)) - 1;
   if (new_index < 0)
     new_index += 2;

   pd->transition_done.content = content;
   pd->transition_done.transition_done = efl_loop_promise_new(obj);

   transition_done = eina_future_new(pd->transition_done.transition_done);
   if (del)
     transition_done = eina_future_then(transition_done, _delete_obj, NULL);

   _active_element_set(obj, pd, efl_pack_content_get(obj, new_index), EFL_UI_SPOTLIGHT_MANAGER_SWITCH_REASON_POP);

   return transition_done;
}

EOLIAN static void
_efl_ui_spotlight_container_animated_transition_set(Eo *obj, Efl_Ui_Spotlight_Container_Data *pd, Eina_Bool enable)
{
   pd->animation_enabled = enable;
   _animated_transition_manager_eval(obj, pd);
}

EOLIAN static Eina_Bool
_efl_ui_spotlight_container_animated_transition_get(const Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Container_Data *pd)
{
   return pd->animation_enabled;
}

EOLIAN static void
_efl_ui_spotlight_container_efl_canvas_group_group_calculate(Eo *obj EINA_UNUSED, Efl_Ui_Spotlight_Container_Data *pd)
{
   Efl_Ui_Widget *content;
   Eina_List *n;

   efl_canvas_group_calculate(efl_super(obj, MY_CLASS));

   pd->min = EINA_SIZE2D(0,0);
   pd->max = EINA_SIZE2D(INT_MAX, INT_MAX);

   EINA_LIST_FOREACH(pd->content_list, n, content)
     {
        Eina_Size2D min, max;

        min = efl_gfx_hint_size_combined_min_get(content);
        max = efl_gfx_hint_size_combined_max_get(content);

        ADJUST_PRIVATE_MIN_MAX(obj, content, pd);
     }
   FLUSH_MIN_MAX(obj, pd);
}


#include "efl_ui_spotlight_container.eo.c"
