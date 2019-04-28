#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "elm_priv.h"
#include "efl_ui_active_view_view_manager_plain.eo.h"

typedef struct _Efl_Ui_Active_View_Container_Data
{
   Eina_List *content_list;
   Eo *page_root, *event;
   struct {
      Eina_Size2D sz;
   } page_spec;
   struct {
      int page;
      double pos;
   } curr;
   struct {
     int from;
     int to;
     double last_pos;
     Eina_Bool active;
   } show_request;
   Efl_Ui_Active_View_View_Manager *transition;
   Efl_Ui_Active_View_Indicator *indicator;
   double position;
   Eina_Bool fill_width: 1;
   Eina_Bool fill_height: 1;
   Eina_Bool prevent_transition_interaction : 1;
   Efl_Ui_Active_View_Container_Gravity gravity;
} Efl_Ui_Active_View_Container_Data;

#define MY_CLASS EFL_UI_ACTIVE_VIEW_CONTAINER_CLASS

static void _unpack(Eo *obj, Efl_Ui_Active_View_Container_Data *pd, Efl_Gfx_Entity *subobj, int index);
static void _unpack_all(Eo *obj EINA_UNUSED, Efl_Ui_Active_View_Container_Data *pd, Eina_Bool clear);

static int
clamp_index(Efl_Ui_Active_View_Container_Data *pd, int index)
{
   if (index < ((int)eina_list_count(pd->content_list)) * -1)
     return -1;
   else if (index > (int)eina_list_count(pd->content_list) - 1)
     return 1;
   return 0;
}

static int
index_rollover(Efl_Ui_Active_View_Container_Data *pd, int index)
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
_transition_end(Eo *obj EINA_UNUSED, Efl_Ui_Active_View_Container_Data *pd)
{
   Efl_Ui_Active_View_Transition_Event ev;

   if (pd->prevent_transition_interaction) return;

   ev.from = pd->show_request.from;
   ev.to = pd->show_request.to;
   efl_event_callback_call(obj, EFL_UI_ACTIVE_VIEW_EVENT_TRANSITION_END, &ev);
   pd->show_request.active = EINA_FALSE;
   pd->show_request.from = -1;
   pd->show_request.to = -1;
}

static void
_transition_start(Eo *obj EINA_UNUSED, Efl_Ui_Active_View_Container_Data *pd, int from, int to, double progress)
{
   Efl_Ui_Active_View_Transition_Event ev;

   if (pd->prevent_transition_interaction) return;

   if (pd->show_request.active)
     _transition_end(obj, pd);

   pd->show_request.active = EINA_TRUE;
   pd->show_request.from = from;
   pd->show_request.to = to;
   pd->show_request.last_pos = progress;
   ev.from = pd->show_request.from;
   ev.to = pd->show_request.to;
   efl_event_callback_call(obj, EFL_UI_ACTIVE_VIEW_EVENT_TRANSITION_START, &ev);
}

static void
_position_set(Eo *obj EINA_UNUSED, Efl_Ui_Active_View_Container_Data *pd, double progress)
{
   if (progress < -1.0) progress = -1.0;
   if (progress > eina_list_count(pd->content_list)) progress = eina_list_count(pd->content_list);
   if (pd->indicator)
     {
        efl_ui_active_view_indicator_position_update(pd->indicator, progress);
     }
   pd->position = progress;
}

static void
_transition_event_emission(Eo *obj EINA_UNUSED, Efl_Ui_Active_View_Container_Data *pd)
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
        if (pd->position == pd->show_request.to)
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
_resize_cb(void *data, const Efl_Event *ev)
{
   Efl_Ui_Active_View_Container_Data *pd = data;
   Eina_Size2D sz;

   sz = efl_gfx_entity_size_get(ev->object);

   if (pd->fill_width) pd->page_spec.sz.w = sz.w;
   if (pd->fill_height) pd->page_spec.sz.h = sz.h;

   if (pd->transition)
     efl_ui_active_view_view_manager_view_size_set(pd->transition, pd->page_spec.sz);
}

EOLIAN static Eo *
_efl_ui_active_view_container_efl_object_constructor(Eo *obj,
                                     Efl_Ui_Active_View_Container_Data *pd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "pager");

   obj = efl_constructor(efl_super(obj, MY_CLASS));

   if (elm_widget_theme_object_set(obj, wd->resize_obj,
                                       elm_widget_theme_klass_get(obj),
                                       elm_widget_theme_element_get(obj),
                                       elm_widget_theme_style_get(obj)) == EFL_UI_THEME_APPLY_ERROR_GENERIC)
     CRI("Failed to set layout!");

   pd->position = -1;
   pd->curr.page = -1;
   pd->curr.pos = 0.0;

   pd->transition = NULL;
   pd->indicator = NULL;

   pd->fill_width = EINA_TRUE;
   pd->fill_height = EINA_TRUE;

   efl_ui_active_view_size_set(obj, EINA_SIZE2D(-1, -1));
   efl_ui_active_view_gravity_set(obj, EFL_UI_ACTIVE_VIEW_CONTAINER_GRAVITY_CONTENT);

   elm_widget_can_focus_set(obj, EINA_FALSE);

   pd->page_root = efl_add(EFL_CANVAS_GROUP_CLASS, evas_object_evas_get(obj));
   efl_content_set(efl_part(obj, "efl.page_root"), pd->page_root);

   efl_event_callback_add(pd->page_root, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _resize_cb, pd);

   pd->event = efl_add(EFL_CANVAS_RECTANGLE_CLASS,
                       evas_object_evas_get(obj));
   evas_object_color_set(pd->event, 0, 0, 0, 0);
   evas_object_repeat_events_set(pd->event, EINA_TRUE);
   efl_content_set(efl_part(obj, "efl.event"), pd->event);

   return obj;
}

EOLIAN static Efl_Object*
_efl_ui_active_view_container_efl_object_finalize(Eo *obj, Efl_Ui_Active_View_Container_Data *pd EINA_UNUSED)
{
   Efl_Ui_Active_View_View_Manager *manager;

   obj = efl_finalize(efl_super(obj, MY_CLASS));

   manager = efl_ui_active_view_manager_get(obj);
   //set a view manager in case nothing is here
   if (!manager)
     {
        Eo *plain;
        plain = efl_add(EFL_UI_ACTIVE_VIEW_VIEW_MANAGER_PLAIN_CLASS, obj);
        efl_ui_active_view_manager_set(obj, plain);
     }
   else
     {
        efl_ui_active_view_view_manager_animation_enabled_set(manager, EINA_TRUE);
     }

   return obj;
}

EOLIAN static void
_efl_ui_active_view_container_efl_object_invalidate(Eo *obj,
                                    Efl_Ui_Active_View_Container_Data *pd)
{
   _unpack_all(obj, pd, EINA_TRUE);
   efl_invalidate(efl_super(obj, MY_CLASS));
}

EOLIAN static int
_efl_ui_active_view_container_efl_container_content_count(Eo *obj EINA_UNUSED,
                                          Efl_Ui_Active_View_Container_Data *pd)
{
   return eina_list_count(pd->content_list);
}

static void
_child_inv(void *data, const Efl_Event *ev)
{
   Efl_Ui_Active_View_Container_Data *pd = efl_data_scope_get(data, MY_CLASS);
   int index = eina_list_data_idx(pd->content_list, ev->object);
   _unpack(data, pd, ev->object, index);
}

static Eina_Bool
_register_child(Eo *obj EINA_UNUSED, Efl_Ui_Active_View_Container_Data *pd, Efl_Gfx_Entity *subobj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(subobj, EINA_FALSE);
   if (eina_list_data_find(pd->content_list, subobj))
     {
        ERR("Object %p is already part of this!", subobj);
        return EINA_FALSE;
     }
   if (!efl_ui_widget_sub_object_add(obj, subobj))
     return EINA_FALSE;

   efl_event_callback_add(subobj, EFL_EVENT_INVALIDATE, _child_inv, obj);

   return EINA_TRUE;
}


EOLIAN static void
_efl_ui_active_view_container_active_view_gravity_set(Eo *obj EINA_UNUSED, Efl_Ui_Active_View_Container_Data *pd, Efl_Ui_Active_View_Container_Gravity gravity)
{
   pd->gravity = gravity;
}

EOLIAN static Efl_Ui_Active_View_Container_Gravity
_efl_ui_active_view_container_active_view_gravity_get(const Eo *obj EINA_UNUSED, Efl_Ui_Active_View_Container_Data *pd)
{
   return pd->gravity;
}

static void
_update_internals(Eo *obj EINA_UNUSED, Efl_Ui_Active_View_Container_Data *pd, Efl_Gfx_Entity *subobj EINA_UNUSED, int index)
{
   Eina_Bool curr_page_update = EINA_FALSE;

   if (pd->gravity == EFL_UI_ACTIVE_VIEW_CONTAINER_GRAVITY_CONTENT && pd->curr.page >= index)
     {
        pd->curr.page++;
        curr_page_update = EINA_TRUE;
     }

   pd->prevent_transition_interaction = EINA_TRUE;
   if (pd->transition)
     efl_ui_active_view_view_manager_content_add(pd->transition, subobj, index);
   if (pd->indicator)
     efl_ui_active_view_indicator_content_add(pd->indicator, subobj, index);
   if (curr_page_update && !pd->transition && eina_list_count(pd->content_list) != 1)
     _position_set(obj, pd, pd->curr.page);
   pd->prevent_transition_interaction = EINA_FALSE;
   if (eina_list_count(pd->content_list) == 1)
     efl_ui_active_view_active_index_set(obj, 0);
}

EOLIAN static Eina_Bool
_efl_ui_active_view_container_efl_pack_linear_pack_begin(Eo *obj EINA_UNUSED,
                                         Efl_Ui_Active_View_Container_Data *pd,
                                         Efl_Gfx_Entity *subobj)
{
   if (!_register_child(obj, pd, subobj)) return EINA_FALSE;
   pd->content_list = eina_list_prepend(pd->content_list, subobj);
   _update_internals(obj, pd, subobj, 0);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_active_view_container_efl_pack_linear_pack_end(Eo *obj EINA_UNUSED,
                                       Efl_Ui_Active_View_Container_Data *pd,
                                       Efl_Gfx_Entity *subobj)
{
   if (!_register_child(obj, pd, subobj)) return EINA_FALSE;
   pd->content_list = eina_list_append(pd->content_list, subobj);
   _update_internals(obj, pd, subobj, eina_list_count(pd->content_list) - 1);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_active_view_container_efl_pack_linear_pack_before(Eo *obj EINA_UNUSED,
                                          Efl_Ui_Active_View_Container_Data *pd,
                                          Efl_Gfx_Entity *subobj,
                                          const Efl_Gfx_Entity *existing)
{
   if (!_register_child(obj, pd, subobj)) return EINA_FALSE;
   int index = eina_list_data_idx(pd->content_list, (void *)existing);
   if (index == -1) return EINA_FALSE;
   pd->content_list = eina_list_prepend_relative(pd->content_list, subobj, existing);
   _update_internals(obj, pd, subobj, index);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_active_view_container_efl_pack_linear_pack_after(Eo *obj EINA_UNUSED,
                                         Efl_Ui_Active_View_Container_Data *pd,
                                         Efl_Gfx_Entity *subobj,
                                         const Efl_Gfx_Entity *existing)
{
   if (!_register_child(obj, pd, subobj)) return EINA_FALSE;
   int index = eina_list_data_idx(pd->content_list, (void *)existing);
   if (index == -1) return EINA_FALSE;
   pd->content_list = eina_list_append_relative(pd->content_list, subobj, existing);
   _update_internals(obj, pd, subobj, index + 1);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_active_view_container_efl_pack_linear_pack_at(Eo *obj,
                                      Efl_Ui_Active_View_Container_Data *pd,
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
_efl_ui_active_view_container_efl_pack_linear_pack_content_get(Eo *obj EINA_UNUSED,
                                               Efl_Ui_Active_View_Container_Data *pd,
                                               int index)
{
   return eina_list_nth(pd->content_list, index_rollover(pd, index));
}

EOLIAN static int
_efl_ui_active_view_container_efl_pack_linear_pack_index_get(Eo *obj EINA_UNUSED,
                                             Efl_Ui_Active_View_Container_Data *pd,
                                             const Efl_Gfx_Entity *subobj)
{
   return eina_list_data_idx(pd->content_list, (void *)subobj);
}

EOLIAN static void
_efl_ui_active_view_container_active_index_set(Eo *obj EINA_UNUSED,
                               Efl_Ui_Active_View_Container_Data *pd,
                               int index)
{
   int before;

   if ((index < 0) || (index > ((int)eina_list_count(pd->content_list) - 1)))
     {
        ERR("index %d out of range", index);
        return;
     }

   before = pd->curr.page;
   pd->show_request.last_pos = pd->curr.page;
   pd->show_request.from = pd->curr.page;
   pd->show_request.to = index;

   if (pd->show_request.active && pd->show_request.from == -1 && pd->show_request.to)
     pd->show_request.to = index; //we just edit this here, a user animation will end when the progress is at the goal.
   else
     {
        _transition_start(obj, pd, before, index, before);
     }

   int old_curr_page = pd->curr.page;
   pd->curr.page = index;
   efl_ui_active_view_view_manager_switch_to(pd->transition, old_curr_page, pd->curr.page);
}

EOLIAN static int
_efl_ui_active_view_container_active_index_get(const Eo *obj EINA_UNUSED,
                               Efl_Ui_Active_View_Container_Data *pd)
{
   return pd->curr.page;
}

EOLIAN Eina_Size2D
_efl_ui_active_view_container_active_view_size_get(const Eo *obj EINA_UNUSED,
                            Efl_Ui_Active_View_Container_Data *pd)
{
   return pd->page_spec.sz;
}

EOLIAN static void
_efl_ui_active_view_container_active_view_size_set(Eo *obj,
                            Efl_Ui_Active_View_Container_Data *pd,
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
     efl_ui_active_view_view_manager_view_size_set(pd->transition, pd->page_spec.sz);
}

static void
_unpack_all(Eo *obj EINA_UNUSED,
            Efl_Ui_Active_View_Container_Data *pd,
            Eina_Bool clear)
{
   pd->curr.page = -1;

   while(pd->content_list)
     {
        Eo *content = eina_list_data_get(pd->content_list);

        _unpack(obj, pd, content, 0);

        if (clear)
          efl_del(content);

        pd->content_list = eina_list_remove(pd->content_list, content);
     }
}

EOLIAN static Eina_Bool
_efl_ui_active_view_container_efl_pack_pack_clear(Eo *obj EINA_UNUSED,
                                  Efl_Ui_Active_View_Container_Data *pd)
{
   _unpack_all(obj, pd, EINA_TRUE);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_active_view_container_efl_pack_unpack_all(Eo *obj EINA_UNUSED,
                                  Efl_Ui_Active_View_Container_Data *pd)
{
   _unpack_all(obj, pd, EINA_FALSE);

   return EINA_TRUE;
}

static void
_unpack(Eo *obj,
        Efl_Ui_Active_View_Container_Data *pd,
        Efl_Gfx_Entity *subobj,
        int index)
{
   int early_curr_page = pd->curr.page;

   pd->content_list = eina_list_remove(pd->content_list, subobj);
   _elm_widget_sub_object_redirect_to_top(obj, subobj);

   if (pd->gravity == EFL_UI_ACTIVE_VIEW_CONTAINER_GRAVITY_CONTENT && index < pd->curr.page)
     pd->curr.page--;

   if (pd->transition)
     efl_ui_active_view_view_manager_content_del(pd->transition, subobj, index);
   if (pd->indicator)
     efl_ui_active_view_indicator_content_del(pd->indicator, subobj, index);

   //we deleted the current index
   if (pd->gravity == EFL_UI_ACTIVE_VIEW_CONTAINER_GRAVITY_CONTENT && early_curr_page == index)
     {
        int new_curr_page = MIN(MAX(early_curr_page, 0), (int)eina_list_count(pd->content_list) - 1);
        pd->curr.page = -1;
        if (eina_list_count(pd->content_list) > 0 && efl_alive_get(obj))
          efl_ui_active_view_active_index_set(obj, new_curr_page);
     }
   //position has updated
   if (early_curr_page != pd->curr.page && early_curr_page != index &&
       pd->indicator && !pd->transition)
     efl_ui_active_view_indicator_position_update(pd->indicator, pd->curr.page);

   efl_event_callback_del(subobj, EFL_EVENT_INVALIDATE, _child_inv, obj);
}

EOLIAN static Eina_Bool
_efl_ui_active_view_container_efl_pack_unpack(Eo *obj,
                              Efl_Ui_Active_View_Container_Data *pd,
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
_efl_ui_active_view_container_efl_pack_linear_pack_unpack_at(Eo *obj,
                                             Efl_Ui_Active_View_Container_Data *pd,
                                             int index)
{
   Efl_Gfx_Entity *subobj = eina_list_nth(pd->content_list, index_rollover(pd, index_rollover(pd, index)));

   _unpack(obj, pd, subobj, index);

   return subobj;
}

EOLIAN static Eina_Bool
_efl_ui_active_view_container_efl_pack_pack(Eo *obj, Efl_Ui_Active_View_Container_Data *pd EINA_UNUSED, Efl_Gfx_Entity *subobj)
{
   return efl_pack_begin(obj, subobj);
}

EOLIAN static Eina_Iterator*
_efl_ui_active_view_container_efl_container_content_iterate(Eo *obj EINA_UNUSED, Efl_Ui_Active_View_Container_Data *pd)
{
  return eina_list_iterator_new(pd->content_list);
}

static void
_pos_updated(void *data, const Efl_Event *event)
{
   Efl_Ui_Active_View_Container_Data *pd = efl_data_scope_get(data, MY_CLASS);
   double progress = *((double*)event->info);
   //ignore this here, this could result in unintendet transition,start / end calls
   if (EINA_DBL_EQ(progress, pd->position))
     return;
   _position_set(data, pd, progress);
   _transition_event_emission(data, pd);
}

EOLIAN static void
_efl_ui_active_view_container_view_manager_set(Eo *obj, Efl_Ui_Active_View_Container_Data *pd, Efl_Ui_Active_View_View_Manager *transition)
{
   if (!transition)
     pd->transition = efl_add(EFL_UI_ACTIVE_VIEW_VIEW_MANAGER_PLAIN_CLASS, obj);
   else
     EINA_SAFETY_ON_FALSE_RETURN(efl_isa(transition, EFL_UI_ACTIVE_VIEW_VIEW_MANAGER_CLASS));

   if (pd->transition)
     {
        efl_ui_active_view_view_manager_bind(pd->transition, NULL, NULL);
        efl_del(pd->transition);
     }

   pd->transition = transition;

   if (pd->transition)
     {
        efl_parent_set(pd->transition, obj);
        //disable animation when not finalized yet, this help reducing the overhead of scheduling a animation that will not be displayed
        efl_ui_active_view_view_manager_animation_enabled_set(pd->transition, efl_finalized_get(obj));
        efl_ui_active_view_view_manager_bind(pd->transition, obj,
          pd->page_root);
        efl_ui_active_view_view_manager_view_size_set(pd->transition, pd->page_spec.sz);
        efl_event_callback_add(pd->transition, EFL_UI_ACTIVE_VIEW_VIEW_MANAGER_EVENT_POS_UPDATE, _pos_updated, obj);
     }

}

EOLIAN static Efl_Ui_Active_View_View_Manager*
_efl_ui_active_view_container_view_manager_get(const Eo *obj EINA_UNUSED, Efl_Ui_Active_View_Container_Data *pd)
{
   if (efl_isa(pd->transition, EFL_UI_ACTIVE_VIEW_VIEW_MANAGER_PLAIN_CLASS))
     return NULL;
   else
     return pd->transition;
}

EOLIAN static void
_efl_ui_active_view_container_indicator_set(Eo *obj, Efl_Ui_Active_View_Container_Data *pd, Efl_Ui_Active_View_Indicator *indicator)
{
   if (pd->indicator)
     {
        efl_ui_active_view_indicator_bind(pd->indicator, obj);
        efl_del(pd->indicator);
     }
   pd->indicator = indicator;
   if (pd->indicator)
     {
        efl_ui_active_view_indicator_bind(pd->indicator, obj);
        if (pd->position != -1)
          efl_ui_active_view_indicator_position_update(pd->indicator, pd->position);
     }
}

EOLIAN static Efl_Ui_Active_View_Indicator*
_efl_ui_active_view_container_indicator_get(const Eo *obj EINA_UNUSED, Efl_Ui_Active_View_Container_Data *pd)
{
   return pd->indicator;
}

#include "efl_ui_active_view_container.eo.c"
