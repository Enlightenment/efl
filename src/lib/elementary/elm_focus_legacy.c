#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

//we need those for legacy compatible code
#include "elm_genlist.eo.h"
#include "elm_gengrid.eo.h"

#define API_ENTRY()\
   EINA_SAFETY_ON_NULL_RETURN(obj); \
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(obj, EFL_UI_WIDGET_CLASS)); \
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, pd); \
   EINA_SAFETY_ON_FALSE_RETURN(elm_widget_is_legacy(obj));

#define API_ENTRY_VAL(val)\
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, val); \
   EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(obj, EFL_UI_WIDGET_CLASS), val); \
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, pd, val); \
   EINA_SAFETY_ON_FALSE_RETURN_VAL(elm_widget_is_legacy(obj), val);

#define MAPPING() \
        MAP(PREVIOUS, prev) \
        MAP(NEXT, next) \
        MAP(UP, up) \
        MAP(DOWN, down) \
        MAP(LEFT, left) \
        MAP(RIGHT, right)



static Eina_List*
_custom_chain_get(const Efl_Ui_Widget *node)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(node, pd, NULL);

   return pd->legacy_focus.custom_chain;
}

static void
_flush_manager(Efl_Ui_Widget *obj, Elm_Widget_Smart_Data *pd)
{
   Efl_Ui_Focus_Manager *manager;

   manager = efl_ui_focus_object_focus_manager_get(obj);
   if (manager)
     {
        Eina_List *order;

        if (pd->legacy_focus.custom_chain)
          order = eina_list_clone(pd->legacy_focus.custom_chain);
        else
          order = eina_list_clone(pd->subobjs);

        efl_ui_focus_manager_calc_update_order(manager, obj, order);
     }
}

static void
_manager_changed(void *data EINA_UNUSED, const Efl_Event *ev)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(ev->object, pd);

   _flush_manager(ev->object, pd);
}

static void
_custom_chain_set(Efl_Ui_Widget *node, Eina_List *lst)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(node, pd);
   Efl_Ui_Widget *list_item;
   Eina_List *n;

   pd->legacy_focus.custom_chain = eina_list_free(pd->legacy_focus.custom_chain);
   pd->legacy_focus.custom_chain = lst;

   EINA_LIST_FOREACH(pd->legacy_focus.custom_chain, n, list_item)
     {
        EINA_SAFETY_ON_FALSE_RETURN(efl_isa(list_item, EFL_UI_WIDGET_CLASS));
        EINA_SAFETY_ON_FALSE_RETURN(efl_ui_widget_parent_get(list_item) == node);
     }

   _elm_widget_full_eval_children(node, pd);

   if (pd->legacy_focus.custom_chain && !pd->legacy_focus.listen_to_manager)
     {
        efl_event_callback_add(node, EFL_UI_FOCUS_OBJECT_EVENT_MANAGER_CHANGED, _manager_changed, NULL);
        pd->legacy_focus.listen_to_manager = EINA_TRUE;
     }
   else if (!pd->legacy_focus.custom_chain && pd->legacy_focus.listen_to_manager)
     {
        efl_event_callback_del(node, EFL_UI_FOCUS_OBJECT_EVENT_MANAGER_CHANGED, _manager_changed, NULL);
        pd->legacy_focus.listen_to_manager = EINA_FALSE;
     }

   _flush_manager(node, pd);
}

EAPI void
elm_object_focus_next_object_set(Evas_Object        *obj,
                                 Evas_Object        *next,
                                 Elm_Focus_Direction dir)
{
   API_ENTRY()
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(next, EFL_UI_WIDGET_CLASS));
   ELM_WIDGET_DATA_GET_OR_RETURN(next, next_pd);

   #define MAP(direction, field)  if (dir == EFL_UI_FOCUS_DIRECTION_ ##direction) pd->legacy_focus.field = next;
   MAPPING()
   #undef MAP
   dir = efl_ui_focus_util_direction_complement(dir);
   #define MAP(direction, field)  if (dir == EFL_UI_FOCUS_DIRECTION_ ##direction) next_pd->legacy_focus.field = obj;
   MAPPING()
   #undef MAP
}

EAPI void
elm_object_focus_custom_chain_set(Evas_Object *obj,
                                  Eina_List   *objs)
{
   API_ENTRY()
   _custom_chain_set(obj, objs);
}

EAPI void
elm_object_focus_custom_chain_unset(Evas_Object *obj)
{
   API_ENTRY()

   _custom_chain_set(obj, NULL);
}

EAPI const Eina_List *
elm_object_focus_custom_chain_get(const Evas_Object *obj)
{
   API_ENTRY_VAL(NULL)

   return _custom_chain_get(obj);
}

EAPI void
elm_object_focus_custom_chain_append(Evas_Object *obj,
                                     Evas_Object *child,
                                     Evas_Object *relative_child)
{
   API_ENTRY()
   Eina_List *tmp;

   tmp = eina_list_clone(pd->legacy_focus.custom_chain);
   tmp = eina_list_append_relative(tmp, child, relative_child);
   _custom_chain_set(obj, tmp);
}

EAPI void
elm_object_focus_custom_chain_prepend(Evas_Object *obj,
                                      Evas_Object *child,
                                      Evas_Object *relative_child)
{
   API_ENTRY()
   Eina_List *tmp;

   tmp = eina_list_clone(pd->legacy_focus.custom_chain);
   tmp = eina_list_prepend_relative(tmp, child, relative_child);
   _custom_chain_set(obj, tmp);
}

EINA_DEPRECATED EAPI void
elm_object_focus_cycle(Evas_Object        *obj,
                       Elm_Focus_Direction dir)
{
   elm_object_focus_next(obj, dir);
}

static Evas_Object*
_get_legacy_target(EINA_UNUSED Evas_Object *eo, Elm_Widget_Smart_Data *pd, Elm_Focus_Direction dir)
{
   Evas_Object *result = NULL;

   #define MAP(direction, field)  if (dir == EFL_UI_FOCUS_DIRECTION_ ##direction && pd->legacy_focus.item_ ##field) result = elm_object_item_widget_get(pd->legacy_focus.item_ ##field);
   MAPPING()
   #undef MAP

   if (!result)
     {
        #define MAP(direction, field)  if (dir == EFL_UI_FOCUS_DIRECTION_ ##direction && pd->legacy_focus.field) result = pd->legacy_focus.field;
        MAPPING()
        #undef MAP
     }

   return result;
}

static Eina_Array*
_focus_parent_chain_gen(Efl_Ui_Focus_Object *obj)
{
   Eina_Array *result = eina_array_new(5);

   for (Eo *parent = obj; parent; parent = efl_ui_focus_object_focus_parent_get(parent))
     {
        eina_array_push(result, parent);
     }

   return result;
}

EAPI void
elm_object_focus_next(Evas_Object        *obj,
                      Elm_Focus_Direction dir)
{
   Eina_Bool legacy_focus_move = EINA_FALSE;
   Efl_Ui_Widget *o = NULL, *top;
   Efl_Ui_Focus_Object *logical;
   Efl_Ui_Focus_Manager *manager_top;
   API_ENTRY()

   top = elm_object_top_widget_get(obj);
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(top, EFL_UI_WIN_CLASS));

   manager_top = efl_ui_focus_util_active_manager(obj);
   logical = efl_ui_focus_manager_focus_get(manager_top);

   if (elm_widget_is(logical))
     {
        Efl_Ui_Focus_Object *legacy_target = NULL;
        ELM_WIDGET_DATA_GET_OR_RETURN(logical, pd_logical);

        legacy_target = _get_legacy_target(obj, pd_logical, dir);

        if (!legacy_target)
          {
             Eina_Array *old_chain = _focus_parent_chain_gen(logical);
             Eina_Array *new_chain = _focus_parent_chain_gen(efl_ui_focus_manager_request_move(top, dir, NULL, EINA_FALSE));

             //first pop off all elements that are the same
             while (eina_array_count(new_chain) > 0 && eina_array_count(old_chain) > 0 &&
                    eina_array_data_get(new_chain, (int)eina_array_count(new_chain) -1) == eina_array_data_get(old_chain, (int)eina_array_count(old_chain) - 1))
               {
                  eina_array_pop(new_chain);
                  eina_array_pop(old_chain);
               }

             for (unsigned int i = 0; i < eina_array_count(old_chain); ++i)
               {
                  Evas_Object *parent = eina_array_data_get(old_chain, i);
                  if (!elm_widget_is(parent)) continue;
                  ELM_WIDGET_DATA_GET_OR_RETURN(parent, ppd);
                  legacy_target = _get_legacy_target(parent, ppd, dir);
                  if (legacy_target) break;
               }
             eina_array_free(new_chain);
             eina_array_free(old_chain);
          }

        if (legacy_target)
          {
             efl_ui_focus_util_focus(legacy_target);
             if (elm_object_focused_object_get(top) == legacy_target)
               {
                  legacy_focus_move = EINA_TRUE;
                  o = legacy_target;
               }
          }
     }

   if (!legacy_focus_move)
     o = efl_ui_focus_manager_move(top, dir);
   if (!o)
     {
        if (dir == EFL_UI_FOCUS_DIRECTION_NEXT || dir == EFL_UI_FOCUS_DIRECTION_PREVIOUS)
          {
             Efl_Ui_Focus_Object *root;

             root = efl_ui_focus_manager_root_get(top);
             efl_ui_focus_manager_setup_on_first_touch(top, dir, root);
          }
     }
}

EAPI Evas_Object *
elm_object_focus_next_object_get(const Evas_Object  *obj,
                                 Elm_Focus_Direction dir)
{
   Efl_Ui_Widget *top = elm_object_top_widget_get(obj);
   API_ENTRY_VAL(NULL)

   #define MAP(direction, field)  if (dir == EFL_UI_FOCUS_DIRECTION_ ##direction && pd->legacy_focus.field) return pd->legacy_focus.field;
   MAPPING()
   #undef MAP

   return efl_ui_focus_manager_request_move(efl_ui_focus_util_active_manager(top), dir, NULL, EINA_FALSE);
}

EAPI Elm_Object_Item *
elm_object_focus_next_item_get(const Evas_Object  *obj,
                               Elm_Focus_Direction dir EINA_UNUSED)
{
   API_ENTRY_VAL(NULL)

   #define MAP(direction, field)  if (dir == EFL_UI_FOCUS_DIRECTION_ ##direction && pd->legacy_focus.item_ ##field) return pd->legacy_focus.item_ ##field;
   MAPPING()
   #undef MAP

   return NULL;
}

EAPI void
elm_object_focus_next_item_set(Evas_Object     *obj,
                               Elm_Object_Item *next_item EINA_UNUSED,
                               Elm_Focus_Direction dir EINA_UNUSED)
{
   API_ENTRY()

   #define MAP(direction, field)  if (dir == EFL_UI_FOCUS_DIRECTION_ ##direction) pd->legacy_focus.item_ ##field = next_item;
   MAPPING()
   #undef MAP
}

EAPI Evas_Object *
elm_object_focused_object_get(const Evas_Object *obj)
{
   API_ENTRY_VAL(NULL)
   Efl_Ui_Focus_Manager *man = elm_object_top_widget_get(obj);

   while(efl_ui_focus_manager_redirect_get(man))
     {
        man = efl_ui_focus_manager_redirect_get(man);

        // legacy compatible code, earlier those containers have not exposed theire items
        if (efl_isa(man, ELM_GENGRID_CLASS) ||
            efl_isa(man, ELM_TOOLBAR_CLASS) ||
            efl_isa(man, ELM_GENLIST_CLASS)) return man;
     }

   return efl_ui_focus_manager_focus_get(man);
}

EAPI Eina_Bool
elm_object_focus_get(const Evas_Object *obj)
{
   API_ENTRY_VAL(EINA_FALSE)

   if (!elm_widget_is(obj))
     return evas_object_focus_get(obj);

   return _elm_widget_top_win_focused_get(obj) && (efl_ui_focus_object_child_focus_get(obj) | efl_ui_focus_object_focus_get(obj));
}

EAPI void
elm_object_focus_set(Evas_Object *obj,
                     Eina_Bool    focus)
{
   // ugly, but, special case for inlined windows
   if (efl_isa(obj, EFL_UI_WIN_CLASS))
     {
        Evas_Object *inlined = elm_win_inlined_image_object_get(obj);
        if (inlined)
          {
             evas_object_focus_set(inlined, focus);
             return;
          }
     }
   else if (elm_widget_is(obj))
     {
        if (focus)
          efl_ui_focus_util_focus(obj);
        else
          {
             if (efl_ui_focus_manager_focus_get(efl_ui_focus_object_focus_manager_get(obj)) == obj)
               efl_ui_focus_manager_pop_history_stack(efl_ui_focus_object_focus_manager_get(obj));
          }
     }
   else
     {
        evas_object_focus_set(obj, focus);
     }
}

//legacy helpers that are used in code
typedef struct {
  Eina_Bool focused;
  Eo *emittee;
} Legacy_Manager_Focus_State;

static void
_focus_manager_focused(void *data, const Efl_Event *ev)
{
   Legacy_Manager_Focus_State *state = data;
   Eina_Bool currently_focused = !!efl_ui_focus_manager_focus_get(ev->object);

   if (currently_focused == state->focused) return;

   if (currently_focused)
     evas_object_smart_callback_call(state->emittee, "focused", NULL);
   else
     evas_object_smart_callback_call(state->emittee, "unfocused", NULL);

   state->focused = currently_focused;
}

static void
_focus_manager_del(void *data, const Efl_Event *ev EINA_UNUSED)
{
   free(data);
}

void
legacy_efl_ui_focus_manager_widget_legacy_signals(Efl_Ui_Focus_Manager *manager, Efl_Ui_Focus_Manager *emittee)
{
   Legacy_Manager_Focus_State *state = calloc(1, sizeof(Legacy_Manager_Focus_State));

   state->emittee = emittee;
   state->focused = EINA_FALSE;

   efl_event_callback_add(manager, EFL_UI_FOCUS_MANAGER_EVENT_MANAGER_FOCUS_CHANGED, _focus_manager_focused, state);
   efl_event_callback_add(manager, EFL_EVENT_DEL, _focus_manager_del, state);
}

typedef struct {
  Eina_Bool focused;
  Efl_Ui_Focus_Manager *registered_manager;
  Eo *emittee;
} Legacy_Object_Focus_State;

static void
_manager_focus_changed(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Legacy_Object_Focus_State *state = data;
   Eina_Bool currently_focused = efl_ui_focus_object_child_focus_get(state->emittee);

   if (currently_focused == state->focused) return;

   if (currently_focused)
     evas_object_smart_callback_call(state->emittee, "focused", NULL);
   else
     evas_object_smart_callback_call(state->emittee, "unfocused", NULL);
   state->focused = currently_focused;
}

static void
_manager_focus_object_changed(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Legacy_Object_Focus_State *state = data;
   if (state->registered_manager)
     efl_event_callback_del(state->registered_manager, EFL_UI_FOCUS_MANAGER_EVENT_MANAGER_FOCUS_CHANGED, _manager_focus_changed, state);
   state->registered_manager = efl_ui_focus_object_focus_manager_get(state->emittee);
   if (state->registered_manager)
     efl_event_callback_add(state->registered_manager, EFL_UI_FOCUS_MANAGER_EVENT_MANAGER_FOCUS_CHANGED, _manager_focus_changed, state);
}

void
legacy_child_focus_handle(Efl_Ui_Focus_Object *object)
{
   Legacy_Object_Focus_State *state = calloc(1, sizeof(Legacy_Object_Focus_State));
   state->emittee = object;

   efl_event_callback_add(object, EFL_UI_FOCUS_OBJECT_EVENT_MANAGER_CHANGED, _manager_focus_object_changed, state);
   efl_event_callback_add(object, EFL_EVENT_DEL, _focus_manager_del, state);
}

static void
_focus_event_changed(void *data EINA_UNUSED, const Efl_Event *event)
{
   if (efl_ui_focus_object_focus_get(event->object))
     evas_object_smart_callback_call(event->object, "focused", NULL);
   else
     evas_object_smart_callback_call(event->object, "unfocused", NULL);
}

void
legacy_object_focus_handle(Efl_Ui_Focus_Object *object)
{
   efl_event_callback_add(object, EFL_UI_FOCUS_OBJECT_EVENT_FOCUS_CHANGED, _focus_event_changed, NULL);
}
