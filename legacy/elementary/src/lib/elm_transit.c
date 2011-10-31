#include <Elementary.h>
#include "elm_priv.h"

#define ELM_TRANSIT_CHECK_OR_RETURN(transit, ...) \
   do { \
      if (!transit) { \
         CRITICAL("Elm_Transit " # transit " is NULL!"); \
         return __VA_ARGS__; \
      } \
      if (!EINA_MAGIC_CHECK(transit, ELM_TRANSIT_MAGIC)) { \
         EINA_MAGIC_FAIL(transit, ELM_TRANSIT_MAGIC); \
         return __VA_ARGS__; \
      } \
      if (transit->deleted){ \
         ERR("Elm_Transit " # transit " has already been deleted!"); \
         return __VA_ARGS__; \
      } \
   } while (0)


#define _TRANSIT_FOCAL 2000

struct _Elm_Transit
{
#define ELM_TRANSIT_MAGIC 0xd27f190a
   EINA_MAGIC;

   Ecore_Animator *animator;
   Eina_Inlist *effect_list;
   Eina_List *objs;
   Elm_Transit *prev_chain_transit;
   Eina_List *next_chain_transits;
   Elm_Transit_Tween_Mode tween_mode;
   struct {
      Elm_Transit_Del_Cb func;
      void *arg;
   } del_data;
   struct {
      double delayed;
      double paused;
      double duration;
      double begin;
      double current;
   } time;
   struct {
      int count;
      int current;
      Eina_Bool reverse;
   } repeat;
   double progress;
   unsigned int effects_pending_del;
   int walking;
   Eina_Bool auto_reverse : 1;
   Eina_Bool event_enabled : 1;
   Eina_Bool deleted : 1;
   Eina_Bool state_keep : 1;
   Eina_Bool finished : 1;
};

struct _Elm_Transit_Effect_Module
{
   EINA_INLIST;
   Elm_Transit_Effect_Transition_Cb transition_cb;
   Elm_Transit_Effect_End_Cb end_cb;
   Elm_Transit_Effect *effect;
   Eina_Bool deleted : 1;
};

struct _Elm_Transit_Obj_State
{
   Evas_Coord x, y, w, h;
   int r,g,b,a;
   Evas_Map *map;
   Eina_Bool map_enabled : 1;
   Eina_Bool visible : 1;
};

struct _Elm_Transit_Obj_Data
{
   struct _Elm_Transit_Obj_State *state;
   Eina_Bool freeze_events : 1;
};

typedef struct _Elm_Transit_Effect_Module Elm_Transit_Effect_Module;
typedef struct _Elm_Transit_Obj_Data Elm_Transit_Obj_Data;
typedef struct _Elm_Transit_Obj_State Elm_Transit_Obj_State;

static void _transit_obj_data_update(Elm_Transit *transit, Evas_Object *obj);
static void _transit_obj_data_recover(Elm_Transit *transit, Evas_Object *obj);
static void _transit_obj_states_save(Evas_Object *obj, Elm_Transit_Obj_Data *obj_data);
static void _transit_obj_remove_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__);
static void _transit_obj_remove(Elm_Transit *transit, Evas_Object *obj);
static void _transit_effect_del(Elm_Transit *transit, Elm_Transit_Effect_Module *effect_module);
static void _transit_remove_dead_effects(Elm_Transit *transit);
static void _transit_del(Elm_Transit *transit);
static Eina_Bool _transit_animate_op(Elm_Transit *transit, double progress);
static Eina_Bool _transit_animate_cb(void *data);

static char *_transit_key= "_elm_transit_key";

static void
_transit_obj_data_update(Elm_Transit *transit, Evas_Object *obj)
{
   Elm_Transit_Obj_Data *obj_data = evas_object_data_get(obj, _transit_key);

   if (!obj_data)
     obj_data = ELM_NEW(Elm_Transit_Obj_Data);

   obj_data->freeze_events = evas_object_freeze_events_get(obj);

   if ((!transit->state_keep) && (obj_data->state))
     {
        free(obj_data->state);
        obj_data->state = NULL;
     }
   else
     {
       _transit_obj_states_save(obj, obj_data);
     }

   evas_object_data_set(obj, _transit_key, obj_data);
}

static void
_transit_obj_states_save(Evas_Object *obj, Elm_Transit_Obj_Data *obj_data)
{
   Elm_Transit_Obj_State *state = obj_data->state;

   if (!state)
     state = calloc(1, sizeof(Elm_Transit_Obj_State));
   if (!state) return;

   evas_object_geometry_get(obj, &state->x, &state->y, &state->w, &state->h);
   evas_object_color_get(obj, &state->r, &state->g, &state->b, &state->a);
   state->visible = evas_object_visible_get(obj);
   state->map_enabled = evas_object_map_enable_get(obj);
   if (evas_object_map_get(obj))
     state->map = evas_map_dup(evas_object_map_get(obj));
   obj_data->state = state;
}

static void
_remove_obj_from_list(Elm_Transit *transit, Evas_Object *obj)
{
   //Remove duplicated objects
   //TODO: Need to consider about optimizing here
   while(1)
     {
        if (!eina_list_data_find_list(transit->objs, obj))
          break;
        transit->objs = eina_list_remove(transit->objs, obj);
        evas_object_event_callback_del_full(obj, EVAS_CALLBACK_DEL,
                                       _transit_obj_remove_cb,
                                       transit);
     }
}

static void
_transit_obj_remove_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Elm_Transit *transit = data;
   Elm_Transit_Obj_Data *obj_data = evas_object_data_get(obj, _transit_key);
   if (obj_data)
     {
        if (obj_data->state)
          free(obj_data->state);
        free(obj_data);
     }
   _remove_obj_from_list(transit, obj);
   if (!transit->objs) elm_transit_del(transit);
}

static void
_transit_obj_data_recover(Elm_Transit *transit, Evas_Object *obj)
{
   Elm_Transit_Obj_Data *obj_data;
   Elm_Transit_Obj_State *state;

   obj_data = evas_object_data_get(obj, _transit_key);
   if (!obj_data) return;
   evas_object_data_del(obj, _transit_key);
   evas_object_freeze_events_set(obj, obj_data->freeze_events);
   state = obj_data->state;
   if (state)
     {
        //recover the states of the object.
        if (!transit->state_keep)
          {
             evas_object_move(obj, state->x, state->y);
             evas_object_resize(obj, state->w, state->h);
             evas_object_color_set(obj, state->r, state->g, state->b, state->a);
             if (state->visible) evas_object_show(obj);
             else evas_object_hide(obj);
             if (state->map_enabled)
               evas_object_map_enable_set(obj, EINA_TRUE);
             else
               evas_object_map_enable_set(obj, EINA_FALSE);
             if (state->map)
               evas_object_map_set(obj, state->map);
          }
        free(state);
     }
   free(obj_data);
}

static void
_transit_obj_remove(Elm_Transit *transit, Evas_Object *obj)
{
   _remove_obj_from_list(transit, obj);
   _transit_obj_data_recover(transit, obj);
}

static void
_transit_effect_del(Elm_Transit *transit, Elm_Transit_Effect_Module *effect_module)
{
   if (effect_module->end_cb)
     effect_module->end_cb(effect_module->effect, transit);
   free(effect_module);
}

static void
_transit_remove_dead_effects(Elm_Transit *transit)
{
   Elm_Transit_Effect_Module *effect_module;

   EINA_INLIST_FOREACH(transit->effect_list, effect_module)
     {
        if (effect_module->deleted)
          {
             _transit_effect_del(transit, effect_module);
             transit->effects_pending_del--;
             if (!transit->effects_pending_del) return;
          }
     }
}

static void
_transit_del(Elm_Transit *transit)
{
   Elm_Transit_Effect_Module *effect_module;
   Elm_Transit *chain_transit;
   Eina_List *elist, *elist_next;

   if (transit->animator)
     ecore_animator_del(transit->animator);

   //remove effects
   while (transit->effect_list)
     {
        effect_module = EINA_INLIST_CONTAINER_GET(transit->effect_list, Elm_Transit_Effect_Module);
        transit->effect_list = eina_inlist_remove(transit->effect_list, transit->effect_list);
        _transit_effect_del(transit, effect_module);
     }

   //remove objects.
   while (transit->objs)
     _transit_obj_remove(transit, eina_list_data_get(transit->objs));

   transit->deleted = EINA_TRUE;

   if (transit->del_data.func)
     transit->del_data.func(transit->del_data.arg, transit);

   //cut off the chain transit relationship
   EINA_LIST_FOREACH_SAFE(transit->next_chain_transits, elist, elist_next, chain_transit)
     chain_transit->prev_chain_transit = NULL;

   if (transit->prev_chain_transit)
     transit->prev_chain_transit->next_chain_transits =
        eina_list_remove(transit->prev_chain_transit->next_chain_transits, transit);

   // run chain transits
   if (transit->finished && transit->next_chain_transits)
     {
        EINA_LIST_FOREACH_SAFE(transit->next_chain_transits, elist, elist_next, chain_transit)
          elm_transit_go(chain_transit);
     }

   eina_list_free(transit->next_chain_transits);

   EINA_MAGIC_SET(transit, EINA_MAGIC_NONE);
   free(transit);
}

//If the transit is deleted then EINA_FALSE is retruned.
static Eina_Bool
_transit_animate_op(Elm_Transit *transit, double progress)
{
   Elm_Transit_Effect_Module *effect_module;

   transit->walking++;
   EINA_INLIST_FOREACH(transit->effect_list, effect_module)
     {
        if (transit->deleted) break;
        if (!effect_module->deleted)
          effect_module->transition_cb(effect_module->effect, transit, progress);
     }
   transit->walking--;

   if (transit->walking) return EINA_TRUE;

   if (transit->deleted)
     {
        _transit_del(transit);
        return EINA_FALSE;
     }

   else if (transit->effects_pending_del) _transit_remove_dead_effects(transit);

   return EINA_TRUE;
}

static Eina_Bool
_transit_animate_cb(void *data)
{
   Elm_Transit *transit = data;
   double elapsed_time, duration;

   transit->time.current = ecore_loop_time_get();
   elapsed_time = transit->time.current - transit->time.begin;
   duration = transit->time.duration + transit->time.delayed;

   if (elapsed_time > duration)
     elapsed_time = duration;

   transit->progress = elapsed_time / duration;
   switch (transit->tween_mode)
     {
      case ELM_TRANSIT_TWEEN_MODE_ACCELERATE:
         transit->progress = 1.0 - sin((ELM_PI / 2.0) + (transit->progress * ELM_PI / 2.0));
         break;
      case ELM_TRANSIT_TWEEN_MODE_DECELERATE:
         transit->progress = sin(transit->progress * ELM_PI / 2.0);
         break;
      case ELM_TRANSIT_TWEEN_MODE_SINUSOIDAL:
         transit->progress = (1.0 - cos(transit->progress * ELM_PI)) / 2.0;
         break;
      default:
         break;
     }

   /* Reverse? */
   if (transit->repeat.reverse) transit->progress = 1 - transit->progress;

   if (transit->time.duration > 0)
     {
        if (!_transit_animate_op(transit, transit->progress))
          return ECORE_CALLBACK_CANCEL;
     }

   /* Not end. Keep going. */
   if (elapsed_time < duration) return ECORE_CALLBACK_RENEW;

   /* Repeat and reverse and time done! */
   if ((transit->repeat.count >= 0) &&
       (transit->repeat.current == transit->repeat.count) &&
       ((!transit->auto_reverse) || transit->repeat.reverse))
     {
        transit->finished = EINA_TRUE;
        elm_transit_del(transit);
        return ECORE_CALLBACK_CANCEL;
     }

   /* Repeat Case */
   if (!transit->auto_reverse || transit->repeat.reverse)
     {
        transit->repeat.current++;
        transit->repeat.reverse = EINA_FALSE;
     }
   else transit->repeat.reverse = EINA_TRUE;

   transit->time.begin = ecore_loop_time_get();

   return ECORE_CALLBACK_RENEW;
}

EAPI Elm_Transit *
elm_transit_add(void)
{
   Elm_Transit *transit = ELM_NEW(Elm_Transit);
   if (!transit)
     {
        ERR("Failed to allocate a elm_transit object!");
        return NULL;
     }

   EINA_MAGIC_SET(transit, ELM_TRANSIT_MAGIC);

   elm_transit_tween_mode_set(transit, ELM_TRANSIT_TWEEN_MODE_LINEAR);

   return transit;
}

EAPI void
elm_transit_del(Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);

   if (transit->walking) transit->deleted = EINA_TRUE;
   else _transit_del(transit);
}

EAPI void
elm_transit_effect_add(Elm_Transit *transit, Elm_Transit_Effect_Transition_Cb transition_cb, Elm_Transit_Effect *effect, Elm_Transit_Effect_End_Cb end_cb)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);
   EINA_SAFETY_ON_NULL_RETURN(transition_cb);
   Elm_Transit_Effect_Module *effect_module;

   EINA_INLIST_FOREACH(transit->effect_list, effect_module)
     if ((effect_module->transition_cb == transition_cb) && (effect_module->effect == effect))
       {
          WRN("elm_transit does not allow to add the duplicated effect! : transit=%p", transit);
          return;
       }

   effect_module = ELM_NEW(Elm_Transit_Effect_Module);
   if (!effect_module)
     {
        ERR("Failed to allocate a new effect!: transit=%p", transit);
        return;
     }

   effect_module->end_cb = end_cb;
   effect_module->transition_cb = transition_cb;
   effect_module->effect = effect;

   transit->effect_list = eina_inlist_append(transit->effect_list, (Eina_Inlist*) effect_module);
}

EAPI void
elm_transit_effect_del(Elm_Transit *transit, Elm_Transit_Effect_Transition_Cb transition_cb, Elm_Transit_Effect *effect)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);
   EINA_SAFETY_ON_NULL_RETURN(transition_cb);
   Elm_Transit_Effect_Module *effect_module;

   EINA_INLIST_FOREACH(transit->effect_list, effect_module)
     {
        if ((effect_module->transition_cb == transition_cb) && (effect_module->effect == effect))
          {
             if (transit->walking)
               {
                  effect_module->deleted = EINA_TRUE;
                  transit->effects_pending_del++;
               }
             else
               {
                  _transit_effect_del(transit, effect_module);
                  if (!transit->effect_list) elm_transit_del(transit);
               }
             return;
          }
     }
}

EAPI void
elm_transit_object_add(Elm_Transit *transit, Evas_Object *obj)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);
   EINA_SAFETY_ON_NULL_RETURN(obj);

   if (transit->animator)
     {
        if (!evas_object_data_get(obj, _transit_key))
          {
             _transit_obj_data_update(transit, obj);
             evas_object_freeze_events_set(obj, EINA_TRUE);
          }
     }

   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
                                  _transit_obj_remove_cb,
                                  transit);

   transit->objs = eina_list_append(transit->objs, obj);
}

EAPI void
elm_transit_object_remove(Elm_Transit *transit, Evas_Object *obj)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);
   EINA_SAFETY_ON_NULL_RETURN(obj);

   _transit_obj_remove(transit, obj);
   if (!transit->objs) elm_transit_del(transit);
}

EAPI const Eina_List *
elm_transit_objects_get(const Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);
   return transit->objs;
}

EAPI void
elm_transit_event_enabled_set(Elm_Transit *transit, Eina_Bool enabled)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);

   Eina_List *list;
   Evas_Object *obj;

   if (transit->event_enabled == enabled) return;
   transit->event_enabled = !!enabled;
   if (!transit->animator) return;

   EINA_LIST_FOREACH(transit->objs, list, obj)
     evas_object_freeze_events_set(obj, enabled);
}

EAPI Eina_Bool
elm_transit_event_enabled_get(const Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, EINA_FALSE);
   return transit->event_enabled;
}

EAPI void
elm_transit_del_cb_set(Elm_Transit *transit, void (*cb) (void *data, Elm_Transit *transit), void *data)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);
   transit->del_data.func = cb;
   transit->del_data.arg = data;
}

EAPI void
elm_transit_auto_reverse_set(Elm_Transit *transit, Eina_Bool reverse)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);
   transit->auto_reverse = reverse;
}

EAPI Eina_Bool
elm_transit_auto_reverse_get(const Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, EINA_FALSE);
   return transit->auto_reverse;
}

EAPI void
elm_transit_repeat_times_set(Elm_Transit *transit, int repeat)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);
   transit->repeat.count = repeat;
   transit->repeat.current = 0;
}

EAPI int
elm_transit_repeat_times_get(const Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, 0);
   return transit->repeat.count;
}

EAPI void
elm_transit_tween_mode_set(Elm_Transit *transit, Elm_Transit_Tween_Mode tween_mode)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);
   transit->tween_mode = tween_mode;
}

EAPI Elm_Transit_Tween_Mode
elm_transit_tween_mode_get(const Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, ELM_TRANSIT_TWEEN_MODE_LINEAR);
   return transit->tween_mode;
}

EAPI void
elm_transit_duration_set(Elm_Transit *transit, double duration)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);
   if (transit->animator)
     {
        WRN("elm_transit does not allow to set the duration time in operating! : transit=%p", transit);
        return;
     }
   transit->time.duration = duration;
}

EAPI double
elm_transit_duration_get(const Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, 0.0);
   return transit->time.duration;
}

EAPI void
elm_transit_go(Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);

   Eina_List *elist;
   Evas_Object *obj;

   if (transit->animator)
     ecore_animator_del(transit->animator);

   EINA_LIST_FOREACH(transit->objs, elist, obj)
     _transit_obj_data_update(transit, obj);

   if (!transit->event_enabled)
     {
        EINA_LIST_FOREACH(transit->objs, elist, obj)
          evas_object_freeze_events_set(obj, EINA_TRUE);
     }

   transit->time.paused = 0;
   transit->time.delayed = 0;
   transit->time.begin = ecore_loop_time_get();
   transit->animator = ecore_animator_add(_transit_animate_cb, transit);
}

EAPI void
elm_transit_paused_set(Elm_Transit *transit, Eina_Bool paused)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);

   if (!transit->animator) return;

   if (paused)
     {
        if (transit->time.paused > 0)
          return;
        ecore_animator_freeze(transit->animator);
        transit->time.paused = ecore_loop_time_get();
     }
   else
     {
        if (transit->time.paused == 0)
          return;
        ecore_animator_thaw(transit->animator);
        transit->time.delayed += (ecore_loop_time_get() - transit->time.paused);
        transit->time.paused = 0;
     }
}

EAPI Eina_Bool
elm_transit_paused_get(const Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, EINA_FALSE);

   if (transit->time.paused == 0)
     return EINA_FALSE;

   return EINA_TRUE;
}

EAPI double
elm_transit_progress_value_get(const Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, 0);

   return transit->progress;
}

EAPI void
elm_transit_objects_final_state_keep_set(Elm_Transit *transit, Eina_Bool state_keep)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);

   if (transit->state_keep == state_keep) return;
   if (transit->animator)
     {
        WRN("elm_transit does not allow to change final state keep mode in operating! : transit=%p", transit);
        return;
     }
   transit->state_keep = !!state_keep;
}

EAPI Eina_Bool
elm_transit_objects_final_state_keep_get(const Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, EINA_FALSE);
   return transit->state_keep;
}

EAPI void
elm_transit_chain_transit_add(Elm_Transit *transit, Elm_Transit *chain_transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);
   ELM_TRANSIT_CHECK_OR_RETURN(chain_transit);

   if (transit == chain_transit)
     {
        WRN("You add a same transit as a chain transit! : transit=%p, chain_transit=%p", transit, chain_transit);
        return;
     }
   if (transit == chain_transit->prev_chain_transit)
     return;

   if (chain_transit->prev_chain_transit)
     chain_transit->prev_chain_transit->next_chain_transits = eina_list_remove(chain_transit->prev_chain_transit->next_chain_transits, chain_transit);

   chain_transit->prev_chain_transit = transit;
   transit->next_chain_transits = eina_list_append(transit->next_chain_transits, chain_transit);
}

EAPI void
elm_transit_chain_transit_del(Elm_Transit *transit, Elm_Transit *chain_transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);
   ELM_TRANSIT_CHECK_OR_RETURN(chain_transit);

   if (chain_transit->prev_chain_transit != transit)
     {
        WRN("A pair of transits does not have the chain relationship! : transit=%p, chain_transit=%p", transit, chain_transit);
        return;
     }

   chain_transit->prev_chain_transit = NULL;
   transit->next_chain_transits = eina_list_remove(transit->next_chain_transits, chain_transit);
}

EAPI Eina_List *
elm_transit_chain_transits_get(const Elm_Transit * transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);
   return transit->next_chain_transits;
}

///////////////////////////////////////////////////////////////////////////
//Resizing Effect
///////////////////////////////////////////////////////////////////////////
typedef struct _Elm_Transit_Effect_Resizing Elm_Transit_Effect_Resizing;

struct _Elm_Transit_Effect_Resizing
{
   struct _size {
      Evas_Coord w, h;
   } from, to;
};

static void
_transit_effect_resizing_context_free(Elm_Transit_Effect *effect, Elm_Transit *transit __UNUSED__)
{
   Elm_Transit_Effect_Resizing *resizing = effect;
   free(resizing);
}

static void
_transit_effect_resizing_op(Elm_Transit_Effect *effect, Elm_Transit *transit, double progress)
{
   EINA_SAFETY_ON_NULL_RETURN(effect);
   EINA_SAFETY_ON_NULL_RETURN(transit);
   Evas_Coord w, h;
   Evas_Object *obj;
   Eina_List *elist;
   Elm_Transit_Effect_Resizing *resizing = effect;

   w = resizing->from.w + (resizing->to.w * progress);
   h = resizing->from.h + (resizing->to.h * progress);

   EINA_LIST_FOREACH(transit->objs, elist, obj)
     evas_object_resize(obj, w, h);
}

static Elm_Transit_Effect *
_transit_effect_resizing_context_new(Evas_Coord from_w, Evas_Coord from_h, Evas_Coord to_w, Evas_Coord to_h)
{
   Elm_Transit_Effect_Resizing *resizing;

   resizing = ELM_NEW(Elm_Transit_Effect_Resizing);
   if (!resizing) return NULL;

   resizing->from.w = from_w;
   resizing->from.h = from_h;
   resizing->to.w = to_w - from_w;
   resizing->to.h = to_h - from_h;

   return resizing;
}

EAPI Elm_Transit_Effect *
elm_transit_effect_resizing_add(Elm_Transit *transit, Evas_Coord from_w, Evas_Coord from_h, Evas_Coord to_w, Evas_Coord to_h)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);
   Elm_Transit_Effect *effect = _transit_effect_resizing_context_new(from_w, from_h, to_w, to_h);

   if (!effect)
     {
        ERR("Failed to allocate resizing effect! : transit=%p", transit);
        return NULL;
     }
   elm_transit_effect_add(transit,
                          _transit_effect_resizing_op, effect,
                          _transit_effect_resizing_context_free);
   return effect;
}

///////////////////////////////////////////////////////////////////////////
//Translation Effect
///////////////////////////////////////////////////////////////////////////
typedef struct _Elm_Transit_Effect_Translation Elm_Transit_Effect_Translation;
typedef struct _Elm_Transit_Effect_Translation_Node Elm_Transit_Effect_Translation_Node;

struct _Elm_Transit_Effect_Translation_Node
{
   Evas_Object *obj;
   Evas_Coord x, y;
};

struct _Elm_Transit_Effect_Translation
{
   struct _position_variation {
      Evas_Coord dx, dy;
   } from, to;
   Eina_List *nodes;
};

static void
_translation_object_del_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Elm_Transit_Effect_Translation *translation = data;
   Eina_List *elist;
   Elm_Transit_Effect_Translation_Node *translation_node;

   EINA_LIST_FOREACH(translation->nodes, elist, translation_node)
     {
        if (translation_node->obj != obj) continue;
        translation->nodes = eina_list_remove_list(translation->nodes, elist);
        free(translation_node);
        break;
     }
}

static Eina_List *
_translation_nodes_build(Elm_Transit *transit, Elm_Transit_Effect_Translation *translation)
{
   Elm_Transit_Effect_Translation_Node *translation_node;
   const Eina_List *elist;
   Evas_Object *obj;
   Eina_List *data_list = NULL;
   const Eina_List *objs = elm_transit_objects_get(transit);

   EINA_LIST_FOREACH(objs, elist, obj)
     {
        translation_node = ELM_NEW(Elm_Transit_Effect_Translation_Node);
        if (!translation_node)
          {
             eina_list_free(data_list);
             return NULL;
          }
        translation_node->obj = obj;
        evas_object_geometry_get(obj, &(translation_node->x),
                                 &(translation_node->y), NULL, NULL);
        data_list = eina_list_append(data_list, translation_node);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
                                       _translation_object_del_cb, translation);
     }
   return data_list;
}

void
_transit_effect_translation_context_free(Elm_Transit_Effect *effect, Elm_Transit *transit __UNUSED__)
{
   EINA_SAFETY_ON_NULL_RETURN(effect);
   Elm_Transit_Effect_Translation *translation = effect;
   Eina_List *elist, *elist_next;
   Elm_Transit_Effect_Translation_Node *translation_node;

   EINA_LIST_FOREACH_SAFE(translation->nodes,
                          elist, elist_next, translation_node)
     {
        evas_object_event_callback_del(translation_node->obj,
                                       EVAS_CALLBACK_DEL, _translation_object_del_cb);
        translation->nodes = eina_list_remove_list(translation->nodes, elist);
        free(translation_node);
     }
   free(translation);
}

void
_transit_effect_translation_op(Elm_Transit_Effect *effect, Elm_Transit *transit, double progress __UNUSED__)
{
   EINA_SAFETY_ON_NULL_RETURN(effect);
   EINA_SAFETY_ON_NULL_RETURN(transit);
   Evas_Coord x, y;
   Elm_Transit_Effect_Translation *translation = effect;
   Elm_Transit_Effect_Translation_Node *translation_node;
   Eina_List *elist;

   if (!translation->nodes)
     translation->nodes = _translation_nodes_build(transit, translation);

   EINA_LIST_FOREACH(translation->nodes, elist, translation_node)
     {
        x = translation_node->x + translation->from.dx
           + (translation->to.dx * progress);
        y = translation_node->y + translation->from.dy
           + (translation->to.dy * progress);
        evas_object_move(translation_node->obj, x, y);
     }
}

static Elm_Transit_Effect *
_transit_effect_translation_context_new(Evas_Coord from_dx, Evas_Coord from_dy, Evas_Coord to_dx, Evas_Coord to_dy)
{
   Elm_Transit_Effect_Translation *translation;

   translation = ELM_NEW(Elm_Transit_Effect_Translation);
   if (!translation) return NULL;

   translation->from.dx = from_dx;
   translation->from.dy = from_dy;
   translation->to.dx = to_dx - from_dx;
   translation->to.dy = to_dy - from_dy;

   return translation;
}

EAPI Elm_Transit_Effect *
elm_transit_effect_translation_add(Elm_Transit *transit, Evas_Coord from_dx, Evas_Coord from_dy, Evas_Coord to_dx, Evas_Coord to_dy)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);
   Elm_Transit_Effect *effect = _transit_effect_translation_context_new(from_dx, from_dy, to_dx, to_dy);

   if (!effect)
     {
        ERR("Failed to allocate translation effect! : transit=%p", transit);
        return NULL;
     }
   elm_transit_effect_add(transit,
                          _transit_effect_translation_op, effect,
                          _transit_effect_translation_context_free);
   return effect;
}

///////////////////////////////////////////////////////////////////////////
//Zoom Effect
///////////////////////////////////////////////////////////////////////////
typedef struct _Elm_Transit_Effect_Zoom Elm_Transit_Effect_Zoom;

struct _Elm_Transit_Effect_Zoom
{
   float from, to;
};

void
_transit_effect_zoom_context_free(Elm_Transit_Effect *effect, Elm_Transit *transit __UNUSED__)
{
   Elm_Transit_Effect_Zoom *zoom = effect;
   free(zoom);
}

static void
_transit_effect_zoom_op(Elm_Transit_Effect *effect, Elm_Transit *transit , double progress)
{
   EINA_SAFETY_ON_NULL_RETURN(effect);
   EINA_SAFETY_ON_NULL_RETURN(transit);
   Evas_Object *obj;
   Eina_List *elist;
   Elm_Transit_Effect_Zoom *zoom = effect;
   Evas_Map *map;
   Evas_Coord x, y, w, h;

   map = evas_map_new(4);
   if (!map) return;

   EINA_LIST_FOREACH(transit->objs, elist, obj)
     {
        evas_object_geometry_get(obj, &x, &y, &w, &h);
        evas_map_util_points_populate_from_object_full(map, obj, zoom->from +
                                                       (progress * zoom->to));
        evas_map_util_3d_perspective(map, x + (w / 2), y + (h / 2), 0, _TRANSIT_FOCAL);
        evas_object_map_set(obj, map);
        evas_object_map_enable_set(obj, EINA_TRUE);
     }
   evas_map_free(map);
}

static Elm_Transit_Effect *
_transit_effect_zoom_context_new(float from_rate, float to_rate)
{
   Elm_Transit_Effect_Zoom *zoom;

   zoom = ELM_NEW(Elm_Transit_Effect_Zoom);
   if (!zoom) return NULL;

   zoom->from = (_TRANSIT_FOCAL - (from_rate * _TRANSIT_FOCAL)) * (1 / from_rate);
   zoom->to = ((_TRANSIT_FOCAL - (to_rate * _TRANSIT_FOCAL)) * (1 / to_rate)) - zoom->from;

   return zoom;
}

EAPI Elm_Transit_Effect *
elm_transit_effect_zoom_add(Elm_Transit *transit, float from_rate, float to_rate)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);
   Elm_Transit_Effect *effect = _transit_effect_zoom_context_new(from_rate, to_rate);

   if (!effect)
     {
        ERR("Failed to allocate zoom effect! : transit=%p", transit);
        return NULL;
     }
   elm_transit_effect_add(transit,
                          _transit_effect_zoom_op, effect,
                          _transit_effect_zoom_context_free);
   return effect;
}

///////////////////////////////////////////////////////////////////////////
//Flip Effect
///////////////////////////////////////////////////////////////////////////
typedef struct _Elm_Transit_Effect_Flip Elm_Transit_Effect_Flip;

struct _Elm_Transit_Effect_Flip
{
   Elm_Transit_Effect_Flip_Axis axis;
   Eina_Bool cw : 1;
};

static void
_transit_effect_flip_context_free(Elm_Transit_Effect *effect, Elm_Transit *transit)
{
   EINA_SAFETY_ON_NULL_RETURN(effect);
   EINA_SAFETY_ON_NULL_RETURN(transit);
   Elm_Transit_Effect_Flip *flip = effect;
   Evas_Object *front, *back;
   int i;
   int count = eina_list_count(transit->objs);

   for (i = 0; i < (count - 1); i += 2)
     {
        front = eina_list_nth(transit->objs, i);
        back = eina_list_nth(transit->objs, i+1);
        evas_object_map_enable_set(front, EINA_FALSE);
        evas_object_map_enable_set(back, EINA_FALSE);
     }
   free(flip);
}

static void
_transit_effect_flip_op(Elm_Transit_Effect *effect, Elm_Transit *transit, double progress)
{
   EINA_SAFETY_ON_NULL_RETURN(effect);
   EINA_SAFETY_ON_NULL_RETURN(transit);
   Evas_Object *obj, *front, *back;
   int count, i;
   Elm_Transit_Effect_Flip *flip = effect;
   Evas_Map *map;
   float degree;
   Evas_Coord x, y, w, h;

   map = evas_map_new(4);
   if (!map) return;

   if (flip->cw) degree = (float)(progress * 180);
   else degree = (float)(progress * -180);

   count = eina_list_count(transit->objs);

   for (i = 0; i < (count - 1); i += 2)
     {
        Evas_Coord half_w, half_h;

        front = eina_list_nth(transit->objs, i);
        back = eina_list_nth(transit->objs, i+1);

        if ((degree < 90) && (degree > -90))
          {
             obj = front;
             if (front != back)
               {
                  evas_object_hide(back);
                  evas_object_show(front);
               }
          }
        else
          {
             obj = back;
             if (front != back)
               {
                  evas_object_hide(front);
                  evas_object_show(back);
               }
          }

        evas_map_util_points_populate_from_object_full(map, obj, 0);
        evas_object_geometry_get(obj, &x, &y, &w, &h);
        half_w = (w / 2);
        half_h = (h / 2);

        if (flip->axis == ELM_TRANSIT_EFFECT_FLIP_AXIS_Y)
          {
             if ((degree >= 90) || (degree <= -90))
               {
                  evas_map_point_image_uv_set(map, 0, w, 0);
                  evas_map_point_image_uv_set(map, 1, 0, 0);
                  evas_map_point_image_uv_set(map, 2, 0, h);
                  evas_map_point_image_uv_set(map, 3, w, h);
               }
             evas_map_util_3d_rotate(map, 0, degree,
                                     0, x + half_w, y + half_h, 0);
          }
        else
          {
             if ((degree >= 90) || (degree <= -90))
               {
                  evas_map_point_image_uv_set(map, 0, 0, h);
                  evas_map_point_image_uv_set(map, 1, w, h);
                  evas_map_point_image_uv_set(map, 2, w, 0);
                  evas_map_point_image_uv_set(map, 3, 0, 0);
               }
             evas_map_util_3d_rotate(map, degree,
                                     0, 0, x + half_w, y + half_h, 0);
          }
        evas_map_util_3d_perspective(map, x + half_w, y + half_h, 0, _TRANSIT_FOCAL);
        evas_object_map_enable_set(front, EINA_TRUE);
        evas_object_map_enable_set(back, EINA_TRUE);
        evas_object_map_set(obj, map);
     }
   evas_map_free(map);
}

static Elm_Transit_Effect *
_transit_effect_flip_context_new(Elm_Transit_Effect_Flip_Axis axis, Eina_Bool cw)
{
   Elm_Transit_Effect_Flip *flip;

   flip = ELM_NEW(Elm_Transit_Effect_Flip);
   if (!flip) return NULL;

   flip->cw = cw;
   flip->axis = axis;

   return flip;
}

EAPI Elm_Transit_Effect *
elm_transit_effect_flip_add(Elm_Transit *transit, Elm_Transit_Effect_Flip_Axis axis, Eina_Bool cw)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);
   Elm_Transit_Effect *effect = _transit_effect_flip_context_new(axis, cw);

   if (!effect)
     {
        ERR("Failed to allocate flip effect! : transit=%p", transit);
        return NULL;
     }
   elm_transit_effect_add(transit,
                          _transit_effect_flip_op, effect,
                          _transit_effect_flip_context_free);
   return effect;
}

///////////////////////////////////////////////////////////////////////////
//ResizableFlip Effect
///////////////////////////////////////////////////////////////////////////
typedef struct _Elm_Transit_Effect_Resizable_Flip Elm_Transit_Effect_ResizableFlip;
typedef struct _Elm_Transit_Effect_Resizable_Flip_Node Elm_Transit_Effect_ResizableFlip_Node;

struct _Elm_Transit_Effect_Resizable_Flip_Node
{
   Evas_Object *front;
   Evas_Object *back;
   struct _vector2d {
      float x, y;
   } from_pos, from_size, to_pos, to_size;
};

struct _Elm_Transit_Effect_Resizable_Flip
{
   Eina_List *nodes;
   Eina_Bool cw : 1;
   Elm_Transit_Effect_Flip_Axis axis;
};

static void
_resizable_flip_object_del_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Elm_Transit_Effect_ResizableFlip *resizable_flip = data;
   Eina_List *elist;
   Elm_Transit_Effect_ResizableFlip_Node *resizable_flip_node;

   EINA_LIST_FOREACH(resizable_flip->nodes, elist, resizable_flip_node)
     {
        if (resizable_flip_node->front == obj)
          evas_object_event_callback_del(resizable_flip_node->back,
                                         EVAS_CALLBACK_DEL, _resizable_flip_object_del_cb);
        else if (resizable_flip_node->back == obj)
          evas_object_event_callback_del(resizable_flip_node->front,
                                         EVAS_CALLBACK_DEL, _resizable_flip_object_del_cb);
        else continue;

        resizable_flip->nodes = eina_list_remove_list(resizable_flip->nodes,
                                                      elist);
        free(resizable_flip_node);
        break;
     }
}

static Eina_List *
_resizable_flip_nodes_build(Elm_Transit *transit, Elm_Transit_Effect_ResizableFlip *resizable_flip)
{
   Elm_Transit_Effect_ResizableFlip_Node *resizable_flip_node;
   Eina_List *data_list = NULL;
   Evas_Coord front_x, front_y, front_w, front_h;
   Evas_Coord back_x, back_y, back_w, back_h;
   int i, count;

   count = eina_list_count(transit->objs);
   for (i = 0; i < (count - 1); i += 2)
     {
        resizable_flip_node = ELM_NEW(Elm_Transit_Effect_ResizableFlip_Node);
        if (!resizable_flip_node)
          {
             eina_list_free(data_list);
             return NULL;
          }

        resizable_flip_node->front = eina_list_nth(transit->objs, i);
        resizable_flip_node->back = eina_list_nth(transit->objs, i+1);

        evas_object_geometry_get(resizable_flip_node->front,
                                 &front_x, &front_y, &front_w, &front_h);
        evas_object_geometry_get(resizable_flip_node->back,
                                 &back_x, &back_y, &back_w, &back_h);

        resizable_flip_node->from_pos.x = front_x;
        resizable_flip_node->from_pos.y = front_y;
        resizable_flip_node->to_pos.x = back_x - front_x;
        resizable_flip_node->to_pos.y = back_y - front_y;

        resizable_flip_node->from_size.x = front_w;
        resizable_flip_node->from_size.y = front_h;
        resizable_flip_node->to_size.x = back_w - front_w;
        resizable_flip_node->to_size.y = back_h - front_h;

        data_list = eina_list_append(data_list, resizable_flip_node);

        evas_object_event_callback_add(resizable_flip_node->back,
                                       EVAS_CALLBACK_DEL, _resizable_flip_object_del_cb, resizable_flip);
        evas_object_event_callback_add(resizable_flip_node->front,
                                       EVAS_CALLBACK_DEL, _resizable_flip_object_del_cb, resizable_flip);
     }

   return data_list;
}

static void
_set_image_uv_by_axis_y(Evas_Map *map, Elm_Transit_Effect_ResizableFlip_Node *flip, float degree)
{
   if ((degree >= 90) || (degree <= -90))
     {
        evas_map_point_image_uv_set(map, 0,
                                    (flip->from_size.x * 2) + flip->to_size.x,
                                    0);
        evas_map_point_image_uv_set(map, 1, 0, 0);
        evas_map_point_image_uv_set(map, 2, 0,
                                    (flip->from_size.y * 2) + flip->to_size.y);
        evas_map_point_image_uv_set(map, 3,
                                    (flip->from_size.x * 2) + flip->to_size.x,
                                    (flip->from_size.y * 2) + flip->to_size.y);
     }
   else
     {
        evas_map_point_image_uv_set(map, 0, 0, 0);
        evas_map_point_image_uv_set(map, 1, flip->from_size.x, 0);
        evas_map_point_image_uv_set(map, 2, flip->from_size.x,
                                    flip->from_size.y);
        evas_map_point_image_uv_set(map, 3, 0, flip->from_size.y);
     }
}

static void
_set_image_uv_by_axis_x(Evas_Map *map, Elm_Transit_Effect_ResizableFlip_Node *flip, float degree)
{
   if ((degree >= 90) || (degree <= -90))
     {
        evas_map_point_image_uv_set(map, 0, 0,
                                    (flip->from_size.y * 2) + flip->to_size.y);
        evas_map_point_image_uv_set(map, 1,
                                    (flip->from_size.x * 2) + flip->to_size.x,
                                    (flip->from_size.y * 2) + flip->to_size.y);
        evas_map_point_image_uv_set(map, 2,
                                    (flip->from_size.x * 2) + flip->to_size.x,
                                    0);
        evas_map_point_image_uv_set(map, 3, 0, 0);
     }
   else
     {
        evas_map_point_image_uv_set(map, 0, 0, 0);
        evas_map_point_image_uv_set(map, 1, flip->from_size.x, 0);
        evas_map_point_image_uv_set(map, 2, flip->from_size.x,
                                    flip->from_size.y);
        evas_map_point_image_uv_set(map, 3, 0, flip->from_size.y);
     }
}

void
_transit_effect_resizable_flip_context_free(Elm_Transit_Effect *effect, Elm_Transit *transit __UNUSED__)
{
   EINA_SAFETY_ON_NULL_RETURN(effect);

   Elm_Transit_Effect_ResizableFlip *resizable_flip = effect;
   Eina_List *elist, *elist_next;
   Elm_Transit_Effect_ResizableFlip_Node *resizable_flip_node;

   EINA_LIST_FOREACH_SAFE(resizable_flip->nodes,
                          elist, elist_next, resizable_flip_node)
     {
        evas_object_map_enable_set(resizable_flip_node->front, EINA_FALSE);
        evas_object_map_enable_set(resizable_flip_node->back, EINA_FALSE);

        resizable_flip->nodes = eina_list_remove_list(resizable_flip->nodes,
                                                      elist);

        evas_object_event_callback_del(resizable_flip_node->back,
                                       EVAS_CALLBACK_DEL, _resizable_flip_object_del_cb);
        evas_object_event_callback_del(resizable_flip_node->front,
                                       EVAS_CALLBACK_DEL, _resizable_flip_object_del_cb);
        free(resizable_flip_node);
     }
   free(resizable_flip);
}

void
_transit_effect_resizable_flip_op(Elm_Transit_Effect *effect, Elm_Transit *transit __UNUSED__, double progress)
{
   EINA_SAFETY_ON_NULL_RETURN(effect);
   Evas_Map *map;
   Evas_Object *obj;
   float x, y, w, h;
   float degree;
   Evas_Coord half_w, half_h;
   Elm_Transit_Effect_ResizableFlip *resizable_flip = effect;
   Elm_Transit_Effect_ResizableFlip_Node *resizable_flip_node;
   Eina_List *elist;

   map = evas_map_new(4);
   if (!map) return;

   if (resizable_flip->cw) degree = (float)(progress * 180);
   else degree = (float)(progress * -180);

   if (!resizable_flip->nodes)
     resizable_flip->nodes = _resizable_flip_nodes_build(transit,
                                                         resizable_flip);

   EINA_LIST_FOREACH(resizable_flip->nodes, elist, resizable_flip_node)
     {
        if ((degree < 90) && (degree > -90))
          {
             obj = resizable_flip_node->front;
             if (resizable_flip_node->front != resizable_flip_node->back)
               {
                  evas_object_hide(resizable_flip_node->back);
                  evas_object_show(resizable_flip_node->front);
               }
          }
        else
          {
             obj = resizable_flip_node->back;
             if (resizable_flip_node->front != resizable_flip_node->back)
               {
                  evas_object_hide(resizable_flip_node->front);
                  evas_object_show(resizable_flip_node->back);
               }
          }

        x = resizable_flip_node->from_pos.x +
           (resizable_flip_node->to_pos.x * progress);
        y = resizable_flip_node->from_pos.y +
           (resizable_flip_node->to_pos.y * progress);
        w = resizable_flip_node->from_size.x +
           (resizable_flip_node->to_size.x * progress);
        h = resizable_flip_node->from_size.y +
           (resizable_flip_node->to_size.y * progress);
        evas_map_point_coord_set(map, 0, x, y, 0);
        evas_map_point_coord_set(map, 1, x + w, y, 0);
        evas_map_point_coord_set(map, 2, x + w, y + h, 0);
        evas_map_point_coord_set(map, 3, x, y + h, 0);

        half_w = (Evas_Coord)(w / 2);
        half_h = (Evas_Coord)(h / 2);

        if (resizable_flip->axis == ELM_TRANSIT_EFFECT_FLIP_AXIS_Y)
          {
             _set_image_uv_by_axis_y(map, resizable_flip_node, degree);
             evas_map_util_3d_rotate(map, 0, degree,
                                     0, x + half_w, y + half_h, 0);
          }
        else
          {
             _set_image_uv_by_axis_x(map, resizable_flip_node, degree);
             evas_map_util_3d_rotate(map, degree, 0,
                                     0, x + half_w, y + half_h, 0);
          }

        evas_map_util_3d_perspective(map, x + half_w, y + half_h, 0, _TRANSIT_FOCAL);
        evas_object_map_enable_set(resizable_flip_node->front, EINA_TRUE);
        evas_object_map_enable_set(resizable_flip_node->back, EINA_TRUE);
        evas_object_map_set(obj, map);
     }
   evas_map_free(map);
}

static Elm_Transit_Effect *
_transit_effect_resizable_flip_context_new(Elm_Transit_Effect_Flip_Axis axis, Eina_Bool cw)
{
   Elm_Transit_Effect_ResizableFlip *resizable_flip;

   resizable_flip = ELM_NEW(Elm_Transit_Effect_ResizableFlip);
   if (!resizable_flip) return NULL;

   resizable_flip->cw = cw;
   resizable_flip->axis = axis;

   return resizable_flip;
}

EAPI Elm_Transit_Effect *
elm_transit_effect_resizable_flip_add(Elm_Transit *transit, Elm_Transit_Effect_Flip_Axis axis, Eina_Bool cw)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);
   Elm_Transit_Effect *effect = _transit_effect_resizable_flip_context_new(axis, cw);

   if (!effect)
     {
        ERR("Failed to allocate resizable_flip effect! : transit=%p", transit);
        return NULL;
     }
   elm_transit_effect_add(transit,
                          _transit_effect_resizable_flip_op, effect,
                          _transit_effect_resizable_flip_context_free);
   return effect;
}

///////////////////////////////////////////////////////////////////////////
//Wipe Effect
///////////////////////////////////////////////////////////////////////////
typedef struct _Elm_Transit_Effect_Wipe Elm_Transit_Effect_Wipe;

struct _Elm_Transit_Effect_Wipe
{
   Elm_Transit_Effect_Wipe_Type type;
   Elm_Transit_Effect_Wipe_Dir dir;
};

static void
_elm_fx_wipe_hide(Evas_Map * map, Elm_Transit_Effect_Wipe_Dir dir, float x, float y, float w, float h, float progress)
{
   float w2, h2;

   switch (dir)
     {
      case ELM_TRANSIT_EFFECT_WIPE_DIR_LEFT:
         w2 = w - (w * progress);
         h2 = (y + h);
         evas_map_point_image_uv_set(map, 0, 0, 0);
         evas_map_point_image_uv_set(map, 1, w2, 0);
         evas_map_point_image_uv_set(map, 2, w2, h);
         evas_map_point_image_uv_set(map, 3, 0, h);
         evas_map_point_coord_set(map, 0, x, y, 0);
         evas_map_point_coord_set(map, 1, x + w2, y, 0);
         evas_map_point_coord_set(map, 2, x + w2, h2, 0);
         evas_map_point_coord_set(map, 3, x, h2, 0);
         break;
      case ELM_TRANSIT_EFFECT_WIPE_DIR_RIGHT:
         w2 = (w * progress);
         h2 = (y + h);
         evas_map_point_image_uv_set(map, 0, w2, 0);
         evas_map_point_image_uv_set(map, 1, w, 0);
         evas_map_point_image_uv_set(map, 2, w, h);
         evas_map_point_image_uv_set(map, 3, w2, h);
         evas_map_point_coord_set(map, 0, x + w2, y, 0);
         evas_map_point_coord_set(map, 1, x + w, y, 0);
         evas_map_point_coord_set(map, 2, x + w, h2, 0);
         evas_map_point_coord_set(map, 3, x + w2, h2, 0);
         break;
      case ELM_TRANSIT_EFFECT_WIPE_DIR_UP:
         w2 = (x + w);
         h2 = h - (h * progress);
         evas_map_point_image_uv_set(map, 0, 0, 0);
         evas_map_point_image_uv_set(map, 1, w, 0);
         evas_map_point_image_uv_set(map, 2, w, h2);
         evas_map_point_image_uv_set(map, 3, 0, h2);
         evas_map_point_coord_set(map, 0, x, y, 0);
         evas_map_point_coord_set(map, 1, w2, y, 0);
         evas_map_point_coord_set(map, 2, w2, y+h2, 0);
         evas_map_point_coord_set(map, 3, x, y+h2, 0);
         break;
      case ELM_TRANSIT_EFFECT_WIPE_DIR_DOWN:
         w2 = (x + w);
         h2 = (h * progress);
         evas_map_point_image_uv_set(map, 0, 0, h2);
         evas_map_point_image_uv_set(map, 1, w, h2);
         evas_map_point_image_uv_set(map, 2, w, h);
         evas_map_point_image_uv_set(map, 3, 0, h);
         evas_map_point_coord_set(map, 0, x, y + h2, 0);
         evas_map_point_coord_set(map, 1, w2, y + h2, 0);
         evas_map_point_coord_set(map, 2, w2, y + h, 0);
         evas_map_point_coord_set(map, 3, x, y + h, 0);
         break;
      default:
         break;
     }
   evas_map_util_3d_perspective(map, x + (w / 2), y + (h / 2), 0, _TRANSIT_FOCAL);
}

static void
_elm_fx_wipe_show(Evas_Map *map, Elm_Transit_Effect_Wipe_Dir dir, float x, float y, float w, float h, float progress)
{
   float w2, h2;

   switch (dir)
     {
      case ELM_TRANSIT_EFFECT_WIPE_DIR_LEFT:
         w2 = (w - (w * progress));
         h2 = (y + h);
         evas_map_point_image_uv_set(map, 0, w2, 0);
         evas_map_point_image_uv_set(map, 1, w, 0);
         evas_map_point_image_uv_set(map, 2, w, h);
         evas_map_point_image_uv_set(map, 3, w2, h);
         evas_map_point_coord_set(map, 0, x + w2, y, 0);
         evas_map_point_coord_set(map, 1, w, y, 0);
         evas_map_point_coord_set(map, 2, w, h2, 0);
         evas_map_point_coord_set(map, 3, x + w2, h2, 0);
         break;
      case ELM_TRANSIT_EFFECT_WIPE_DIR_RIGHT:
         w2 = (w * progress);
         h2 = (y + h);
         evas_map_point_image_uv_set(map, 0, 0, 0);
         evas_map_point_image_uv_set(map, 1, w2, 0);
         evas_map_point_image_uv_set(map, 2, w2, h);
         evas_map_point_image_uv_set(map, 3, 0, h);
         evas_map_point_coord_set(map, 0, x, y, 0);
         evas_map_point_coord_set(map, 1, x + w2, y, 0);
         evas_map_point_coord_set(map, 2, x + w2, h2, 0);
         evas_map_point_coord_set(map, 3, x, h2, 0);
         break;
      case ELM_TRANSIT_EFFECT_WIPE_DIR_UP:
         w2 = (x + w);
         h2 = (h - (h * progress));
         evas_map_point_image_uv_set(map, 0, 0, h2);
         evas_map_point_image_uv_set(map, 1, w, h2);
         evas_map_point_image_uv_set(map, 2, w, h);
         evas_map_point_image_uv_set(map, 3, 0, h);
         evas_map_point_coord_set(map, 0, x, y + h2, 0);
         evas_map_point_coord_set(map, 1, w2, y + h2, 0);
         evas_map_point_coord_set(map, 2, w2, y + h, 0);
         evas_map_point_coord_set(map, 3, x, y + h, 0);
         break;
      case ELM_TRANSIT_EFFECT_WIPE_DIR_DOWN:
         w2 = (x + w);
         h2 = (h * progress);
         evas_map_point_image_uv_set(map, 0, 0, 0);
         evas_map_point_image_uv_set(map, 1, w, 0);
         evas_map_point_image_uv_set(map, 2, w, h2);
         evas_map_point_image_uv_set(map, 3, 0, h2);
         evas_map_point_coord_set(map, 0, x, y, 0);
         evas_map_point_coord_set(map, 1, w2, y, 0);
         evas_map_point_coord_set(map, 2, w2, y + h2, 0);
         evas_map_point_coord_set(map, 3, x, y + h2, 0);
         break;
      default:
         break;
     }
   evas_map_util_3d_perspective(map, x + (w / 2), y + (h / 2), 0, _TRANSIT_FOCAL);
}

static void
_transit_effect_wipe_context_free(Elm_Transit_Effect *effect, Elm_Transit *transit)
{
   EINA_SAFETY_ON_NULL_RETURN(effect);
   EINA_SAFETY_ON_NULL_RETURN(transit);
   Eina_List *elist;
   Evas_Object *obj;
   Elm_Transit_Effect_Wipe *wipe = effect;
   Eina_Bool reverse = elm_transit_auto_reverse_get(transit);

   EINA_LIST_FOREACH(transit->objs, elist, obj)
     {
        if ((wipe->type == ELM_TRANSIT_EFFECT_WIPE_TYPE_SHOW && !reverse)
            || (wipe->type == ELM_TRANSIT_EFFECT_WIPE_TYPE_HIDE && reverse))
          evas_object_show(obj);
        else evas_object_hide(obj);
        evas_object_map_enable_set(obj, EINA_FALSE);
     }

   free(wipe);
}

static void
_transit_effect_wipe_op(Elm_Transit_Effect *effect, Elm_Transit *transit, double progress)
{
   EINA_SAFETY_ON_NULL_RETURN(effect);
   EINA_SAFETY_ON_NULL_RETURN(transit);
   Elm_Transit_Effect_Wipe *wipe = effect;
   Evas_Map *map;
   Evas_Coord _x, _y, _w, _h;
   Eina_List *elist;
   Evas_Object *obj;

   map = evas_map_new(4);
   if (!map) return;

   EINA_LIST_FOREACH(transit->objs, elist, obj)
     {
        evas_object_geometry_get(obj, &_x, &_y, &_w, &_h);

        if (wipe->type == ELM_TRANSIT_EFFECT_WIPE_TYPE_SHOW)
          _elm_fx_wipe_show(map, wipe->dir, _x, _y, _w, _h, (float)progress);
        else
          _elm_fx_wipe_hide(map, wipe->dir, _x, _y, _w, _h, (float)progress);

        evas_object_map_enable_set(obj, EINA_TRUE);
        evas_object_map_set(obj, map);
     }
   evas_map_free(map);
}

static Elm_Transit_Effect *
_transit_effect_wipe_context_new(Elm_Transit_Effect_Wipe_Type type, Elm_Transit_Effect_Wipe_Dir dir)
{
   Elm_Transit_Effect_Wipe *wipe;

   wipe = ELM_NEW(Elm_Transit_Effect_Wipe);
   if (!wipe) return NULL;

   wipe->type = type;
   wipe->dir = dir;

   return wipe;
}

EAPI void *
elm_transit_effect_wipe_add(Elm_Transit *transit, Elm_Transit_Effect_Wipe_Type type, Elm_Transit_Effect_Wipe_Dir dir)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);
   void *effect = _transit_effect_wipe_context_new(type, dir);

   if (!effect)
     {
        ERR("Failed to allocate wipe effect! : transit=%p", transit);
        return NULL;
     }
   elm_transit_effect_add(transit,
                          _transit_effect_wipe_op, effect,
                          _transit_effect_wipe_context_free);
   return effect;
}

///////////////////////////////////////////////////////////////////////////
//Color Effect
///////////////////////////////////////////////////////////////////////////
typedef struct _Elm_Transit_Effect_Color Elm_Transit_Effect_Color;

struct _Elm_Transit_Effect_Color
{
   struct _unsigned_color {
      unsigned int r, g, b, a;
   } from;
   struct _signed_color {
      int r, g, b, a;
   } to;
};

static void
_transit_effect_color_context_free(Elm_Transit_Effect *effect, Elm_Transit *transit __UNUSED__)
{
   Elm_Transit_Effect_Color *color = effect;
   free(color);
}

static void
_transit_effect_color_op(Elm_Transit_Effect *effect, Elm_Transit *transit, double progress)
{
   EINA_SAFETY_ON_NULL_RETURN(effect);
   EINA_SAFETY_ON_NULL_RETURN(transit);
   Elm_Transit_Effect_Color *color = effect;
   Evas_Object *obj;
   Eina_List *elist;
   unsigned int r, g, b, a;

   r = (color->from.r + (int)((float)color->to.r * progress));
   g = (color->from.g + (int)((float)color->to.g * progress));
   b = (color->from.b + (int)((float)color->to.b * progress));
   a = (color->from.a + (int)((float)color->to.a * progress));

   EINA_LIST_FOREACH(transit->objs, elist, obj)
     evas_object_color_set(obj, r, g, b, a);
}

static Elm_Transit_Effect *
_transit_effect_color_context_new(unsigned int from_r, unsigned int from_g, unsigned int from_b, unsigned int from_a, unsigned int to_r, unsigned int to_g, unsigned int to_b, unsigned int to_a)
{
   Elm_Transit_Effect_Color *color;

   color = ELM_NEW(Elm_Transit_Effect_Color);
   if (!color) return NULL;

   color->from.r = from_r;
   color->from.g = from_g;
   color->from.b = from_b;
   color->from.a = from_a;
   color->to.r = to_r - from_r;
   color->to.g = to_g - from_g;
   color->to.b = to_b - from_b;
   color->to.a = to_a - from_a;

   return color;
}

EAPI Elm_Transit_Effect *
elm_transit_effect_color_add(Elm_Transit *transit, unsigned int from_r, unsigned int from_g, unsigned int from_b, unsigned int from_a, unsigned int to_r, unsigned int to_g, unsigned int to_b, unsigned int to_a)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);
   Elm_Transit_Effect *effect = _transit_effect_color_context_new(from_r, from_g, from_b, from_a, to_r, to_g, to_b, to_a);

   if (!effect)
     {
        ERR("Failed to allocate color effect! : transit=%p", transit);
        return NULL;
     }
   elm_transit_effect_add(transit,
                          _transit_effect_color_op, effect,
                          _transit_effect_color_context_free);
   return effect;
}

///////////////////////////////////////////////////////////////////////////
//Fade Effect
///////////////////////////////////////////////////////////////////////////
typedef struct _Elm_Transit_Effect_Fade Elm_Transit_Effect_Fade;
typedef struct _Elm_Transit_Effect_Fade_Node Elm_Transit_Effect_Fade_Node;

struct _Elm_Transit_Effect_Fade_Node
{
   Evas_Object *before;
   Evas_Object *after;
   struct _signed_color before_color, after_color;
   int before_alpha;
   int after_alpha;
   Eina_Bool inversed : 1;
};

struct _Elm_Transit_Effect_Fade
{
   Eina_List *nodes;
};

static void
_fade_object_del_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Elm_Transit_Effect_Fade *fade = data;
   Eina_List *elist;
   Elm_Transit_Effect_Fade_Node *fade_node;

   EINA_LIST_FOREACH(fade->nodes, elist, fade_node)
     {
        if (fade_node->before == obj)
          evas_object_event_callback_del(fade_node->after,
                                         EVAS_CALLBACK_DEL, _fade_object_del_cb);
        else if (fade_node->after == obj)
          evas_object_event_callback_del(fade_node->before,
                                         EVAS_CALLBACK_DEL, _fade_object_del_cb);
        else continue;

        fade->nodes = eina_list_remove_list(fade->nodes, elist);
        free(fade_node);
        break;
     }
}

static Eina_List *
_fade_nodes_build(Elm_Transit *transit, Elm_Transit_Effect_Fade *fade_data)
{
   Elm_Transit_Effect_Fade_Node *fade;
   Eina_List *data_list = NULL;
   int i, count;

   count = eina_list_count(transit->objs);
   for (i = 0; i < count; i += 2)
     {
        fade = ELM_NEW(Elm_Transit_Effect_Fade_Node);
        if (!fade)
          {
             eina_list_free(data_list);
             return NULL;
          }

        fade->before = eina_list_nth(transit->objs, i);
        fade->after = eina_list_nth(transit->objs, i+1);

        evas_object_color_get(fade->before,
                              &fade->before_color.r, &fade->before_color.g,
                              &fade->before_color.b, &fade->before_color.a);
        evas_object_color_get(fade->after,
                              &fade->after_color.r, &fade->after_color.g,
                              &fade->after_color.b, &fade->after_color.a);

        fade->before_alpha = (255 - fade->before_color.a);
        fade->after_alpha = (255 - fade->after_color.a);

        data_list = eina_list_append(data_list, fade);

        evas_object_event_callback_add(fade->before,
                                       EVAS_CALLBACK_DEL, _fade_object_del_cb, fade_data);
        evas_object_event_callback_add(fade->after,
                                       EVAS_CALLBACK_DEL, _fade_object_del_cb, fade_data);
     }
   return data_list;
}

static void
_transit_effect_fade_context_free(Elm_Transit_Effect *effect, Elm_Transit *transit __UNUSED__)
{
   EINA_SAFETY_ON_NULL_RETURN(effect);
   Elm_Transit_Effect_Fade *fade = effect;
   Elm_Transit_Effect_Fade_Node *fade_node;
   Eina_List *elist, *elist_next;

   EINA_LIST_FOREACH_SAFE(fade->nodes, elist, elist_next, fade_node)
     {
        evas_object_color_set(fade_node->before, fade_node->before_color.r,
                              fade_node->before_color.g,
                              fade_node->before_color.b,
                              fade_node->before_color.a);
        evas_object_color_set(fade_node->after, fade_node->after_color.r,
                              fade_node->after_color.g,
                              fade_node->after_color.b,
                              fade_node->after_color.a);

        fade->nodes = eina_list_remove_list(fade->nodes, elist);
        evas_object_event_callback_del(fade_node->before,
                                       EVAS_CALLBACK_DEL, _fade_object_del_cb);
        evas_object_event_callback_del(fade_node->after,
                                       EVAS_CALLBACK_DEL, _fade_object_del_cb);
        free(fade_node);
     }

   free(fade);
}

static void
_transit_effect_fade_op(Elm_Transit_Effect *effect, Elm_Transit *transit __UNUSED__, double progress)
{
   EINA_SAFETY_ON_NULL_RETURN(effect);
   Elm_Transit_Effect_Fade *fade = effect;
   Eina_List *elist;
   Elm_Transit_Effect_Fade_Node *fade_node;
   float _progress;

   if (!fade->nodes)
     fade->nodes = _fade_nodes_build(transit, fade);

   EINA_LIST_FOREACH(fade->nodes, elist, fade_node)
     {
        if (progress < 0.5)
          {
             if (!fade_node->inversed)
               {
                  evas_object_hide(fade_node->after);
                  evas_object_show(fade_node->before);
                  fade_node->inversed = EINA_TRUE;
               }

             _progress = (1 - (progress * 2));

             evas_object_color_set(fade_node->before,
                                   fade_node->before_color.r * _progress,
                                   fade_node->before_color.g * _progress,
                                   fade_node->before_color.b * _progress,
                                   fade_node->before_color.a +
                                   fade_node->before_alpha * (1 - _progress));
          }
        else
          {
             if (fade_node->inversed)
               {
                  evas_object_hide(fade_node->before);
                  evas_object_show(fade_node->after);
                  fade_node->inversed = EINA_FALSE;
               }

             _progress = ((progress - 0.5) * 2);

             evas_object_color_set(fade_node->after,
                                   fade_node->after_color.r * _progress,
                                   fade_node->after_color.g * _progress,
                                   fade_node->after_color.b * _progress,
                                   fade_node->after_color.a +
                                   fade_node->after_alpha * (1 - _progress));
          }
     }
}

static Elm_Transit_Effect *
_transit_effect_fade_context_new(void)
{
   Elm_Transit_Effect_Fade *fade;
   fade = ELM_NEW(Elm_Transit_Effect_Fade);
   if (!fade) return NULL;
   return fade;
}

EAPI Elm_Transit_Effect *
elm_transit_effect_fade_add(Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);

   Elm_Transit_Effect *effect = _transit_effect_fade_context_new();

   if (!effect)
     {
        ERR("Failed to allocate fade effect! : transit=%p", transit);
        return NULL;
     }
   elm_transit_effect_add(transit,
                          _transit_effect_fade_op, effect,
                          _transit_effect_fade_context_free);
   return effect;
}

///////////////////////////////////////////////////////////////////////////
//Blend Effect
///////////////////////////////////////////////////////////////////////////
typedef struct _Elm_Transit_Effect_Blend Elm_Transit_Effect_Blend;
typedef struct _Elm_Transit_Effect_Blend_Node Elm_Transit_Effect_Blend_Node;

struct _Elm_Transit_Effect_Blend_Node
{
   Evas_Object *before;
   Evas_Object *after;
   struct _signed_color from, to;
};

struct _Elm_Transit_Effect_Blend
{
   Eina_List *nodes;
};

static void
_blend_object_del_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Elm_Transit_Effect_Blend *blend = data;
   Eina_List *elist;
   Elm_Transit_Effect_Blend_Node *blend_node;

   EINA_LIST_FOREACH(blend->nodes, elist, blend_node)
     {
        if (blend_node->after == obj)
          evas_object_event_callback_del(blend_node->before,
                                         EVAS_CALLBACK_DEL, _blend_object_del_cb);
        else if (blend_node->before == obj)
          evas_object_event_callback_del(blend_node->after,
                                         EVAS_CALLBACK_DEL, _blend_object_del_cb);
        else continue;

        blend->nodes = eina_list_remove_list(blend->nodes, elist);
        free(blend_node);
        break;
     }
}

static Eina_List *
_blend_nodes_build(Elm_Transit *transit, Elm_Transit_Effect_Blend *blend)
{
   Elm_Transit_Effect_Blend_Node *blend_node;
   Eina_List *data_list = NULL;
   int i, count;

   count = eina_list_count(transit->objs);
   for (i = 0; i < (count - 1); i += 2)
     {
        blend_node = ELM_NEW(Elm_Transit_Effect_Blend_Node);
        if (!blend_node)
          {
             eina_list_free(data_list);
             return NULL;
          }

        blend_node->before = eina_list_nth(transit->objs, i);
        blend_node->after = eina_list_nth(transit->objs, i + 1);
        evas_object_show(blend_node->before);
        evas_object_show(blend_node->after);

        evas_object_color_get(blend_node->before, &blend_node->from.r,
                              &blend_node->from.g, &blend_node->from.b,
                              &blend_node->from.a);
        evas_object_color_get(blend_node->after, &blend_node->to.r,
                              &blend_node->to.g, &blend_node->to.b,
                              &blend_node->to.a);

        data_list = eina_list_append(data_list, blend_node);

        evas_object_event_callback_add(blend_node->before,
                                       EVAS_CALLBACK_DEL, _blend_object_del_cb, blend);
        evas_object_event_callback_add(blend_node->after,
                                       EVAS_CALLBACK_DEL, _blend_object_del_cb, blend);
     }
   return data_list;
}

void
_transit_effect_blend_context_free(Elm_Transit_Effect *effect, Elm_Transit *transit __UNUSED__)
{
   EINA_SAFETY_ON_NULL_RETURN(effect);
   Elm_Transit_Effect_Blend *blend = effect;
   Elm_Transit_Effect_Blend_Node *blend_node;
   Eina_List *elist, *elist_next;

   EINA_LIST_FOREACH_SAFE(blend->nodes, elist, elist_next, blend_node)
     {
        evas_object_color_set(blend_node->before,
                              blend_node->from.r, blend_node->from.g,
                              blend_node->from.b, blend_node->from.a);
        evas_object_color_set(blend_node->after, blend_node->to.r,
                              blend_node->to.g, blend_node->to.b,
                              blend_node->to.a);

        if (elm_transit_auto_reverse_get(transit))
          evas_object_hide(blend_node->after);
        else
          evas_object_hide(blend_node->before);

        blend->nodes = eina_list_remove_list(blend->nodes, elist);

        evas_object_event_callback_del(blend_node->before,
                                       EVAS_CALLBACK_DEL, _blend_object_del_cb);
        evas_object_event_callback_del(blend_node->after,
                                       EVAS_CALLBACK_DEL, _blend_object_del_cb);
        free(blend_node);
     }
   free(blend);
}

void
_transit_effect_blend_op(Elm_Transit_Effect *effect, Elm_Transit *transit, double progress)
{
   EINA_SAFETY_ON_NULL_RETURN(effect);
   EINA_SAFETY_ON_NULL_RETURN(transit);
   Elm_Transit_Effect_Blend *blend = effect;
   Elm_Transit_Effect_Blend_Node *blend_node;
   Eina_List *elist;

   if (!blend->nodes) blend->nodes = _blend_nodes_build(transit, blend);

   EINA_LIST_FOREACH(blend->nodes, elist, blend_node)
     {
        evas_object_color_set(blend_node->before,
                              (int)(blend_node->from.r * (1 - progress)),
                              (int)(blend_node->from.g * (1 - progress)),
                              (int)(blend_node->from.b * (1 - progress)),
                              (int)(blend_node->from.a * (1 - progress)));
        evas_object_color_set(blend_node->after,
                              (int)(blend_node->to.r * progress),
                              (int)(blend_node->to.g * progress),
                              (int)(blend_node->to.b * progress),
                              (int)(blend_node->to.a * progress));
     }
}

static Elm_Transit_Effect *
_transit_effect_blend_context_new(void)
{
   Elm_Transit_Effect_Blend *blend;

   blend = ELM_NEW(Elm_Transit_Effect_Blend);
   if (!blend) return NULL;
   return blend;
}

EAPI Elm_Transit_Effect *
elm_transit_effect_blend_add(Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);
   Elm_Transit_Effect *effect = _transit_effect_blend_context_new();

   if (!effect)
     {
        ERR("Failed to allocate blend effect! : transit=%p", transit);
        return NULL;
     }
   elm_transit_effect_add(transit,
                          _transit_effect_blend_op, effect,
                          _transit_effect_blend_context_free);
   return effect;
}

///////////////////////////////////////////////////////////////////////////
//Rotation Effect
///////////////////////////////////////////////////////////////////////////
typedef struct _Elm_Transit_Effect_Rotation Elm_Transit_Effect_Rotation;

struct _Elm_Transit_Effect_Rotation
{
   float from, to;
};

static void
_transit_effect_rotation_context_free(Elm_Transit_Effect *effect, Elm_Transit *transit __UNUSED__)
{
   Elm_Transit_Effect_Rotation *rotation = effect;
   free(rotation);
}

static void
_transit_effect_rotation_op(Elm_Transit_Effect *effect, Elm_Transit *transit, double progress)
{
   EINA_SAFETY_ON_NULL_RETURN(effect);
   EINA_SAFETY_ON_NULL_RETURN(transit);
   Elm_Transit_Effect_Rotation *rotation = effect;
   Evas_Map *map;
   Evas_Coord x, y, w, h;
   float degree;
   float half_w, half_h;
   Eina_List *elist;
   Evas_Object *obj;

   map = evas_map_new(4);
   if (!map) return;

   EINA_LIST_FOREACH(transit->objs, elist, obj)
     {
        evas_map_util_points_populate_from_object_full(map, obj, 0);
        degree = rotation->from + (float)(progress * rotation->to);

        evas_object_geometry_get(obj, &x, &y, &w, &h);

        half_w = (float)w * 0.5;
        half_h = (float)h * 0.5;

        evas_map_util_rotate(map, degree, x + half_w, y + half_h);
        evas_map_util_3d_perspective(map, x + half_w, y + half_h, 0, _TRANSIT_FOCAL);
        evas_object_map_enable_set(obj, EINA_TRUE);
        evas_object_map_set(obj, map);
     }
   evas_map_free(map);
}

static Elm_Transit_Effect *
_transit_effect_rotation_context_new(float from_degree, float to_degree)
{
   Elm_Transit_Effect_Rotation *rotation;

   rotation = ELM_NEW(Elm_Transit_Effect_Rotation);
   if (!rotation) return NULL;

   rotation->from = from_degree;
   rotation->to = to_degree - from_degree;

   return rotation;
}

EAPI Elm_Transit_Effect *
elm_transit_effect_rotation_add(Elm_Transit *transit, float from_degree, float to_degree)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);
   Elm_Transit_Effect *effect = _transit_effect_rotation_context_new(from_degree, to_degree);

   if (!effect)
     {
        ERR("Failed to allocate rotation effect! : transit=%p", transit);
        return NULL;
     }
   elm_transit_effect_add(transit,
                          _transit_effect_rotation_op, effect,
                          _transit_effect_rotation_context_free);
   return effect;
}

///////////////////////////////////////////////////////////////////////////
//ImageAnimation Effect
///////////////////////////////////////////////////////////////////////////
typedef struct _Elm_Transit_Effect_Image_Animation Elm_Transit_Effect_Image_Animation;

struct _Elm_Transit_Effect_Image_Animation
{
   Eina_List *images;
};

static void
_transit_effect_image_animation_context_free(Elm_Transit_Effect *effect, Elm_Transit *transit __UNUSED__)
{
   EINA_SAFETY_ON_NULL_RETURN(effect);
   Elm_Transit_Effect_Image_Animation *image_animation = effect;
   const char *image;
   Eina_List *elist, *elist_next;

   EINA_LIST_FOREACH_SAFE(image_animation->images, elist, elist_next, image)
     {
        image_animation->images =
           eina_list_remove_list(image_animation->images, elist);
        eina_stringshare_del(image);
     }

   free(image_animation);
}

static void
_transit_effect_image_animation_op(Elm_Transit_Effect *effect, Elm_Transit *transit, double progress)
{
   EINA_SAFETY_ON_NULL_RETURN(effect);
   EINA_SAFETY_ON_NULL_RETURN(transit);
   Eina_List *elist;
   Evas_Object *obj;
   const char *type;
   Elm_Transit_Effect_Image_Animation *image_animation = effect;
   unsigned int count = 0;
   int len;

   type = eina_stringshare_add("icon");
   len = eina_list_count(image_animation->images);

   if (!len) count = floor(progress * len);
   else count = floor(progress * (len - 1));

   EINA_LIST_FOREACH(transit->objs, elist, obj)
     {
        if (elm_widget_type_check(obj, type, __func__))
          elm_icon_file_set(obj,
                            eina_list_nth(image_animation->images, count), NULL);
     }

   eina_stringshare_del(type);
}

static Elm_Transit_Effect *
_transit_effect_image_animation_context_new(Eina_List *images)
{
   Elm_Transit_Effect_Image_Animation *image_animation;
   image_animation = ELM_NEW(Elm_Transit_Effect_Image_Animation);

   if (!image_animation) return NULL;
   image_animation->images = images;
   return image_animation;
}

EAPI Elm_Transit_Effect *
elm_transit_effect_image_animation_add(Elm_Transit *transit, Eina_List *images)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);
   Elm_Transit_Effect *effect = _transit_effect_image_animation_context_new(images);

   if (!effect)
     {
        ERR("Failed to allocate image_animation effect! : transit=%p", transit);
        return NULL;
     }
   elm_transit_effect_add(transit,
                          _transit_effect_image_animation_op, effect,
                          _transit_effect_image_animation_context_free);
   return effect;
}
