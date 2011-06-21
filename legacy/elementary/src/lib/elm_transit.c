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

/**
 *
 * @defgroup Transit Transit
 * @ingroup Elementary
 *
 * Transit (see Warning below) is designed to set the various effects for the
 * Evas_Object such like translation, rotation, etc. For using Effects, Create
 * transit and insert effects which are interesting.
 * Once effects are inserted into transit, transit will manage those effects.
 * (ex deleting).
 *
 * Example:
 * @code
 * Elm_Transit *trans = elm_transit_add();
 * elm_transit_object_add(trans, obj);
 * void *effect_context = elm_transit_effect_translation_context_new(0.0, 0.0,
 *                                                               280.0, 280.0);
 * elm_transit_effect_add(transit,
 *                        elm_transit_effect_translation_op, effect_context,
 *                        elm_transit_effect_translation_context_free);
 * elm_transit_duration_set(transit, 1);
 * elm_transit_auto_reverse_set(transit, EINA_TRUE);
 * elm_transit_tween_mode_set(transit, ELM_TRANSIT_TWEEN_MODE_DECELERATE);
 * elm_transit_repeat_times_set(transit, 3);
 * @endcode
 *
 * @warning We strongly recomend to use elm_transit just when edje can not do
 * the trick. Edje has more advantage than Elm_Transit, it has more flexibility and
 * animations can be manipulated inside the theme.
 */

#define _TRANSIT_FOCAL 2000

struct _Elm_Transit
{
#define ELM_TRANSIT_MAGIC 0xd27f190a
   EINA_MAGIC;

   Ecore_Animator *animator;
   Eina_Inlist *effect_list;
   Eina_List *objs;
   Eina_Hash *objs_data_hash;
   Elm_Transit *prev_chain_transit;
   Eina_List *next_chain_transits;
   Elm_Transit_Tween_Mode tween_mode;
   struct {
      Elm_Transit_Effect_End_Cb func;
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
};

struct _Elm_Transit_Effect_Module
{
   EINA_INLIST;
   Elm_Transit_Effect_Transition_Cb transition_cb;
   Elm_Transit_Effect_End_Cb end_cb;
   Elm_Transit_Effect *effect;
   Eina_Bool deleted : 1;
};

struct _Elm_Obj_State
{
   Evas_Coord x, y, w, h;
   int r,g,b,a;
   Evas_Map *map;
   Eina_Bool map_enabled : 1;
   Eina_Bool visible : 1;
};

struct _Elm_Obj_Data
{
   struct _Elm_Obj_State *state;
   Eina_Bool pass_events : 1;
};

typedef struct _Elm_Transit_Effect_Module Elm_Transit_Effect_Module;
typedef struct _Elm_Obj_Data Elm_Obj_Data;
typedef struct _Elm_Obj_State Elm_Obj_State;

static void
_elm_transit_obj_states_save(Evas_Object *obj, Elm_Obj_Data *obj_data)
{
   Elm_Obj_State *state;

   if (obj_data->state) return;
   state = calloc(1, sizeof(Elm_Obj_State));
   if (!state) return;
   evas_object_geometry_get(obj, &state->x, &state->y, &state->w, &state->h);
   evas_object_color_get(obj, &state->r, &state->g, &state->b, &state->a);
   state->visible = evas_object_visible_get(obj);
   state->map_enabled = evas_object_map_enable_get(obj);
   if (evas_object_map_get(obj))
     state->map = evas_map_dup(evas_object_map_get(obj));
   obj_data->state = state;
}

static Eina_Bool
_hash_foreach_pass_events_set(const Eina_Hash *hash __UNUSED__, const void *key, void *data __UNUSED__, void *fdata)
{
   Elm_Transit *transit = fdata;
   evas_object_pass_events_set((Evas_Object*) key, transit->event_enabled);
   return EINA_TRUE;
}

static Eina_Bool
_hash_foreach_obj_states_save(const Eina_Hash *hash __UNUSED__, const void *key, void *data, void *fdata __UNUSED__)
{
   _elm_transit_obj_states_save((Evas_Object *) key, (Elm_Obj_Data *) data);
   return EINA_TRUE;
}

static void
_elm_transit_object_remove_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Elm_Transit *transit;
   Elm_Obj_Data *obj_data;
   Eina_List *list;

   transit = data;
   list = eina_list_data_find_list(transit->objs, obj);
   obj_data = eina_hash_find(transit->objs_data_hash, list);
   if (!obj_data) return;
   eina_hash_del_by_key(transit->objs_data_hash, list);
   evas_object_pass_events_set(obj, obj_data->pass_events);
   if (obj_data->state)
     free(obj_data->state);
   free(obj_data);
   transit->objs = eina_list_remove(transit->objs, obj);
   if (!transit->objs) elm_transit_del(transit);
}

//TODO: Remove!
//Since evas map have a afterimage bug for this time.
//This function is added temporary.
static void
_obj_damage_area_set(Evas_Object *obj)
{
   const Evas_Map *map;
   Evas_Coord_Point coords;
   Evas_Coord_Point min, max;
   int i;

   map  = evas_object_map_get(obj);
   if (!map) return;

   evas_map_point_coord_get(map, 0, &coords.x, &coords.y, NULL);

   max = min = coords;

   for (i = 1; i < 4; ++i)
     {
        evas_map_point_coord_get(map, i, &coords.x, &coords.y, NULL);

        if (coords.x < min.x)
          min.x = coords.x;
        else if (coords.x > max.x)
          max.x = coords.x;

        if (coords.y < min.y)
          min.y = coords.y;
        else if (coords.y > max.y)
          max.y = coords.y;
     }

   evas_damage_rectangle_add(evas_object_evas_get(obj),
                             min.x, min.y,
                             max.x - min.x, max.y - min.y);
}

static void
_remove_obj_from_list(Elm_Transit *transit, Evas_Object *obj)
{
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_DEL,
                                       _elm_transit_object_remove_cb,
                                       transit);

   //Remove duplicated objects
   //TODO: Need to consider about optimizing here
   while(1)
     {
        if (!eina_list_data_find_list(transit->objs, obj))
          break;
        transit->objs = eina_list_remove(transit->objs, obj);
     }
}

static void
_elm_transit_object_remove(Elm_Transit *transit, Evas_Object *obj)
{
   Elm_Obj_Data *obj_data;
   Elm_Obj_State *state;
   Eina_List *list;

   list = eina_list_data_find_list(transit->objs, obj);
   obj_data = eina_hash_find(transit->objs_data_hash, list);
   if (!obj_data)
     {
        _remove_obj_from_list(transit, obj);
        return;
     }
   eina_hash_del_by_key(transit->objs_data_hash, list);
   _remove_obj_from_list(transit, obj);
   evas_object_pass_events_set(obj, obj_data->pass_events);
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

             //TODO: Remove!
             //Since evas map have a afterimage bug for this time.
             //This line is added temporary.
             _obj_damage_area_set(obj);

          }
        free(state);
     }
   free(obj_data);

}

static void
_elm_transit_effect_del(Elm_Transit *transit, Elm_Transit_Effect_Module *effect_module)
{
   if (effect_module->end_cb)
     effect_module->end_cb(effect_module->effect, transit);
   free(effect_module);
}

static void
_remove_dead_effects(Elm_Transit *transit)
{
   Elm_Transit_Effect_Module *effect_module;

   EINA_INLIST_FOREACH(transit->effect_list, effect_module)
     {
        if (effect_module->deleted)
          {
             _elm_transit_effect_del(transit, effect_module);
             transit->effects_pending_del--;
             if (!transit->effects_pending_del) return;
          }
     }
}

static void
_elm_transit_del(Elm_Transit *transit)
{
   Elm_Transit_Effect_Module *effect_module;
   Elm_Transit *chain_transit;
   Eina_List *elist, *elist_next;

   EINA_LIST_FOREACH_SAFE(transit->next_chain_transits, elist, elist_next, chain_transit)
     {
        if (transit->prev_chain_transit)
          transit->prev_chain_transit->next_chain_transits = eina_list_remove(transit->prev_chain_transit->next_chain_transits, transit);
        chain_transit->prev_chain_transit = NULL;
     }

   eina_list_free(transit->next_chain_transits);

   if (transit->animator)
     ecore_animator_del(transit->animator);

   while (transit->effect_list)
     {
        effect_module = EINA_INLIST_CONTAINER_GET(transit->effect_list, Elm_Transit_Effect_Module);
        transit->effect_list = eina_inlist_remove(transit->effect_list, transit->effect_list);
        _elm_transit_effect_del(transit, effect_module);
     }

   while (transit->objs)
     _elm_transit_object_remove(transit, eina_list_data_get(transit->objs));

   transit->deleted = EINA_TRUE;

   if (transit->del_data.func)
     transit->del_data.func(transit->del_data.arg, transit);

   eina_hash_free(transit->objs_data_hash);

   EINA_MAGIC_SET(transit, EINA_MAGIC_NONE);
   free(transit);
}

static void
_chain_transits_go(Elm_Transit *transit)
{
   Eina_List *elist, *elist_next;
   Elm_Transit *chain_transit;

   EINA_LIST_FOREACH_SAFE(transit->next_chain_transits, elist, elist_next, chain_transit)
     elm_transit_go(chain_transit);
}

static void
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

   if (transit->walking) return;

   if (transit->deleted) _elm_transit_del(transit);
   else if (transit->effects_pending_del) _remove_dead_effects(transit);
}

static Eina_Bool
_animator_animate_cb(void *data)
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

   if (transit->time.duration > 0) _transit_animate_op(transit, transit->progress);

   /* Not end. Keep going. */
   if (elapsed_time < duration) return ECORE_CALLBACK_RENEW;

   /* Repeat and reverse and time done! */
   if ((transit->repeat.count >= 0) &&
       (transit->repeat.current == transit->repeat.count) &&
       ((!transit->auto_reverse) || transit->repeat.reverse))
     {
        /* run chain transit */
        if (transit->next_chain_transits)
          _chain_transits_go(transit);

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

/**
 * Add new transit.
 *
 * @note Is not necessary to delete the transit object, it will be deleted at
 * the end of its operation.
 * @note The transit will start playing when the program enter in the main loop, is not
 * necessary to give a start to the transit.
 *
 * @param duration The duration of the transit in seconds. When transit starts
 * to run, it will last a @p duration time.
 * @return The transit object.
 *
 * @ingroup Transit
 */
EAPI Elm_Transit *
elm_transit_add(void)
{
   Elm_Transit *transit = ELM_NEW(Elm_Transit);
   if (!transit) return NULL;

   EINA_MAGIC_SET(transit, ELM_TRANSIT_MAGIC);

   elm_transit_tween_mode_set(transit, ELM_TRANSIT_TWEEN_MODE_LINEAR);

   transit->objs_data_hash = eina_hash_int32_new(NULL);

   return transit;
}
/**
 * Stops the animation and delete the @p transit object.
 *
 * Call this function if you wants to stop the animation before the duration
 * time. Make sure the @p transit object is still alive with
 * elm_transit_del_cb_set() function.
 * All added effects will be deleted, calling its repective data_free_cb
 * functions. The function setted by elm_transit_del_cb_set() will be called.
 *
 * @see elm_transit_del_cb_set()
 *
 * @param transit The transit object to be deleted.
 *
 * @ingroup Transit
 * @warning Just call this function if you are sure the transit is alive.
 */
EAPI void
elm_transit_del(Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);

   if (transit->walking) transit->deleted = EINA_TRUE;
   else _elm_transit_del(transit);
}

/**
 * Add a new effect to the transit.
 *
 * @note The cb function and the data are the key to the effect. If you try to
 * add an already added effect, nothing is done.
 * @note After the first addition of an effect in @p transit, if its
 * effect list become empty again, the @p transit will be killed by
 * elm_transit_del(transit) function.
 *
 * Exemple:
 * @code
 * Elm_Transit *transit = elm_transit_add();
 * elm_transit_effect_add(transit,
 *                        elm_transit_effect_blend_op,
 *                        elm_transit_effect_blend_context_new(),
 *                        elm_transit_effect_blend_context_free);
 * @endcode
 *
 * @param transit The transit object.
 * @param cb The operation function. It is called when the animation begins,
 * it is the function that actually performs the animation. It is called with
 * the @p data, @p transit and the time progression of the animation (a double
 * value between 0.0 and 1.0).
 * @param data The context data of the effect.
 * @param data_free_cb The function to free the context data, it will be called
 * at the end of the effect, it must finalize the animation and free the
 * @p data.
 *
 * @ingroup Transit
 * @warning The transit free the context data at the and of the transition with
 * the data_free_cb function, do not use the context data in another transit.
 */
EAPI void
elm_transit_effect_add(Elm_Transit *transit, Elm_Transit_Effect_Transition_Cb transition_cb, Elm_Transit_Effect *effect, Elm_Transit_Effect_End_Cb end_cb)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);
   EINA_SAFETY_ON_NULL_RETURN(transition_cb);
   Elm_Transit_Effect_Module *effect_module;

   EINA_INLIST_FOREACH(transit->effect_list, effect_module)
     if ((effect_module->transition_cb == transition_cb) && (effect_module->effect == effect)) return;

   effect_module = ELM_NEW(Elm_Transit_Effect_Module);
   if (!effect_module) return;

   effect_module->end_cb = end_cb;
   effect_module->transition_cb = transition_cb;
   effect_module->effect = effect;

   transit->effect_list = eina_inlist_append(transit->effect_list, (Eina_Inlist*) effect_module);
}

/**
 * Delete an added effect.
 *
 * This function will remove the effect from the @p transit, calling the
 * data_free_cb to free the @p data.
 *
 * @see elm_transit_effect_add()
 *
 * @note If the effect is not found, nothing is done.
 * @note If the effect list become empty, this function will call
 * elm_transit_del(transit), that is, it will kill the @p transit.
 *
 * @param transit The transit object.
 * @param cb The operation function.
 * @param data The context data of the effect.
 *
 * @ingroup Transit
 */
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
                  _elm_transit_effect_del(transit, effect_module);
                  if (!transit->effect_list) elm_transit_del(transit);
               }
             return;
          }
     }
}

/**
 * Add new object to apply the effects.
 *
 * @note After the first addition of an object in @p transit, if its
 * object list become empty again, the @p transit will be killed by
 * elm_transit_del(transit) function.
 * @note If the @p obj belongs to another transit, the @p obj will be
 * removed from it and it will only belong to the @p transit. If the old
 * transit stays without objects, it will die.
 * @note When you add an object into the @p transit, its state from
 * evas_object_pass_events_get(obj) is saved, and it is applied when the
 * transit ends, if you change this state whith evas_object_pass_events_set()
 * after add the object, this state will change again when @p transit stops to
 * run.
 *
 * @param transit The transit object.
 * @param obj Object to be animated.
 *
 * @ingroup Transit
 * @warning It is not allowed to add a new object after transit begins to go.
 */
EAPI void
elm_transit_object_add(Elm_Transit *transit, Evas_Object *obj)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);
   EINA_SAFETY_ON_NULL_RETURN(obj);
   Elm_Obj_Data *obj_data;
   Eina_List * list;

//TODO: Check the remove case of the same objects in this transit.
   obj_data = ELM_NEW(Elm_Obj_Data);
   obj_data->pass_events = evas_object_pass_events_get(obj);
   if (!transit->event_enabled)
     evas_object_pass_events_set(obj, EINA_TRUE);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
                                  _elm_transit_object_remove_cb,
                                  transit);

   transit->objs = eina_list_append(transit->objs, obj);
   list = eina_list_last(transit->objs);
   eina_hash_add(transit->objs_data_hash, list, obj_data);

   if (!transit->state_keep)
     _elm_transit_obj_states_save(obj, obj_data);
}

/**
 * Removes an added object from the transit.
 *
 * @note If the @p obj is not in the @p transit, nothing is done.
 * @note If the list become empty, this function will call
 * elm_transit_del(transit), that is, it will kill the @p transit.
 *
 * @param transit The transit object.
 * @param obj Object to be removed from @p transit.
 *
 * @ingroup Transit
 * @warning It is not allowed to remove objects after transit begins to go.
 */
EAPI void
elm_transit_object_remove(Elm_Transit *transit, Evas_Object *obj)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);
   EINA_SAFETY_ON_NULL_RETURN(obj);

   _elm_transit_object_remove(transit, obj);
   if (!transit->objs) elm_transit_del(transit);
}

/**
 * Get the objects of the transit.
 *
 * @param transit The transit object.
 * @return a Eina_List with the objects from the transit.
 *
 * @ingroup Transit
 */
EAPI const Eina_List *
elm_transit_objects_get(const Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);
   return transit->objs;
}

/**
 * Set the event enabled when transit is operating.
 *
 * If @p enabled is EINA_TRUE, the objects of the transit will receives
 * events from mouse and keyboard during the animation.
 * @note When you add an object with elm_transit_object_add(), its state from
 * evas_object_pass_events_get(obj) is saved, and it is applied when the
 * transit ends, if you change this state with evas_object_pass_events_set()
 * after adding the object, this state will change again when @p transit stops
 * to run.
 *
 * @param transit The transit object.
 * @param enabled Disable or enable.
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_event_enabled_set(Elm_Transit *transit, Eina_Bool enabled)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);

   if (transit->event_enabled == enabled) return;
   transit->event_enabled = !!enabled;
   eina_hash_foreach(transit->objs_data_hash, _hash_foreach_pass_events_set, transit);
}

/**
 * Get the value of event enabled status.
 *
 * @see elm_transit_event_enabled_set()
 *
 * @param transit The Transit object
 * @return EINA_TRUE, when event is enabled. If @p transit is NULL
 * EINA_FALSE is returned
 *
 * @ingroup Transit
 */
EAPI Eina_Bool
elm_transit_event_enabled_get(const Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, EINA_FALSE);
   return transit->event_enabled;
}


/**
 * Set the event enabled when transit is operating.
 *
 * If @p disabled is EINA_TRUE, the objects of the transit will receives
 * events from mouse and keyboard during the animation.
 * @note When you add an object with elm_transit_object_add(), its state from
 * evas_object_pass_events_get(obj) is saved, and it is applied when the
 * transit ends, if you change this state with evas_object_pass_events_set()
 * after add the object, this state will change again when @p transit stops to
 * run.
 *
 * @see elm_transit_event_enabled_set()
 *
 * @param transit The transit object.
 * @param disabled Disable or enable.
 *
 * @ingroup Transit
 */
EINA_DEPRECATED EAPI void
elm_transit_event_block_set(Elm_Transit *transit, Eina_Bool disabled)
{
   elm_transit_event_enabled_set(transit, disabled);
}


/**
 * Get the value of event block enabled  status.
 *
 * @see elm_transit_event_enabled_set(), elm_transit_event_enabled_get()
 *
 * @param transit The Transit object
 * @return EINA_TRUE, when event is enabled. If @p transit is NULL
 * EINA_FALSE is returned
 *
 * @ingroup Transit
 */
EINA_DEPRECATED EAPI Eina_Bool
elm_transit_event_block_get(const Elm_Transit *transit)
{
   return !elm_transit_event_enabled_get(transit);
}

/**
 * Set the user-callback function when the transit is deleted.
 *
 * @note Using this function twice will overwrite the first function setted.
 * @note the @p transit object will be deleted after call @p cb function.
 *
 * @param transit The transit object.
 * @param cb Callback function pointer. This function will be called before
 * the deletion of the transit.
 * @param data Callback funtion user data. It is the @p op parameter.
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_del_cb_set(Elm_Transit *transit, void (*cb) (void *data, Elm_Transit *transit), void *data)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);
   transit->del_data.func = cb;
   transit->del_data.arg = data;
}

/**
 * Set reverse effect automatically.
 *
 * If auto reverse is setted, after running the effects with the progress
 * parameter from 0 to 1, it will call the effecs again with the progress
 * from 1 to 0. The transit will last for a time iqual to (2 * duration * repeat),
 * where the duration was setted with the function elm_transit_add and
 * the repeat with the function elm_transit_repeat_times_set().
 *
 * @param transit The transit object.
 * @param reverse EINA_TRUE means the auto_reverse is on.
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_auto_reverse_set(Elm_Transit *transit, Eina_Bool reverse)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);
   transit->auto_reverse = reverse;
}

/**
 * Get if the auto reverse is on.
 *
 * @see elm_transit_auto_reverse_set()
 *
 * @param transit The transit object.
 * @return EINA_TRUE means auto reverse is on. If @p transit is NULL
 * EINA_FALSE is returned
 *
 * @ingroup Transit
 */
EAPI Eina_Bool
elm_transit_auto_reverse_get(const Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, EINA_FALSE);
   return transit->auto_reverse;
}

/**
 * Set the transit repeat count. Effect will be repeated by repeat count.
 *
 * This function sets the number of repetition the transit will run after
 * the first one, that is, if @p repeat is 1, the transit will run 2 times.
 * If the @p repeat is a negative number, it will repeat infinite times.
 *
 * @note If this function is called during the transit execution, the transit
 * will run @p repeat times, ignoring the times it already performed.
 *
 * @param transit The transit object
 * @param repeat Repeat count
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_repeat_times_set(Elm_Transit *transit, int repeat)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);
   transit->repeat.count = repeat;
   transit->repeat.current = 0;
}

/**
 * Get the transit repeat count.
 *
 * @see elm_transit_repeat_times_set()
 *
 * @param transit The Transit object.
 * @return The repeat count. If @p transit is NULL
 * 0 is returned
 *
 * @ingroup Transit
 */
EAPI int
elm_transit_repeat_times_get(const Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, 0);
   return transit->repeat.count;
}

/**
 * Set the transit animation acceleration type.
 *
 * This function sets the tween mode of the transit that can be:
 * ELM_TRANSIT_TWEEN_MODE_LINEAR - The default mode.
 * ELM_TRANSIT_TWEEN_MODE_SINUSOIDAL - Starts in accelerate mode and ends decelerating.
 * ELM_TRANSIT_TWEEN_MODE_DECELERATE - The animation will be slowed over time.
 * ELM_TRANSIT_TWEEN_MODE_ACCELERATE - The animation will accelerate over time.
 *
 * @param transit The transit object.
 * @param tween_mode The tween type.
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_tween_mode_set(Elm_Transit *transit, Elm_Transit_Tween_Mode tween_mode)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);
   transit->tween_mode = tween_mode;
}

/**
 * Get the transit animation acceleration type.
 *
 * @note @p transit can not be NULL
 *
 * @param transit The transit object.
 * @return The tween type. If @p transit is NULL
 * ELM_TRANSIT_TWEEN_MODE_LINEAR is returned.
 *
 * @ingroup Transit
 */
EAPI Elm_Transit_Tween_Mode
elm_transit_tween_mode_get(const Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, ELM_TRANSIT_TWEEN_MODE_LINEAR);
   return transit->tween_mode;
}

/**
 * Set the transit animation time
 *
 * @note @p transit can not be NULL
 *
 * @param transit The transit object.
 * @param duration The animation time.
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_duration_set(Elm_Transit *transit, double duration)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);
   if (transit->animator) return;
   transit->time.duration = duration;
}

/**
 * Get the transit animation time
 *
 * @note @p transit can not be NULL
 *
 * @param transit The transit object.
 *
 * @return The transit animation time.
 *
 * @ingroup Transit
 */
EAPI double
elm_transit_duration_get(const Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, 0.0);
   return transit->time.duration;
}

/**
 * Starts the transition.
 * Once this API is called, the transit begins to measure the time.
 *
 * @note @p transit can not be NULL
 *
 * @param transit The transit object.
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_go(Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);

   if (transit->animator)
     ecore_animator_del(transit->animator);

   transit->time.paused = 0;
   transit->time.delayed = 0;
   transit->time.begin = ecore_loop_time_get();
   transit->animator = ecore_animator_add(_animator_animate_cb, transit);
}

/**
 * Pause/Resume the transition.
 * If you call elm_transit_go again, paused states will affect no anymore.
 *
 * @note @p transit can not be NULL
 *
 * @param transit The transit object.
 *
 * @ingroup Transit
 */
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

/**
 * Get the value of paused status.
 *
 * @see elm_transit_paused_set()
 *
 * @note @p transit can not be NULL
 *
 * @param transit The transit object.
 * @return EINA_TRUE means transition is paused. If @p transit is NULL
 * EINA_FALSE is returned
 *
 * @ingroup Transit
 */
EAPI Eina_Bool
elm_transit_paused_get(const Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, EINA_FALSE);

   if (transit->time.paused == 0)
     return EINA_FALSE;

   return EINA_TRUE;
}

/**
 * Get the time progression of the animation (a double value between 0.0 and 1.0).
 *
 * @note @p transit can not be NULL
 *
 * @param transit The transit object.
 *
 * @return The time progression value. If @p transit is NULL
 * 0 is returned
 *
 * @ingroup Transit
 */
EAPI double
elm_transit_progress_value_get(const Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, 0);
   return transit->progress;
}



/**
 * Enable/disable keeping up the objects states.
 * If it is not kept, the objects states will be reset when transition ends.
 *
 * @note @p transit can not be NULL.
 * @note One state includes geometry, color, map data.
 *
 * @param transit The transit object.
 * @param state_keep Keeping or Non Keeping.
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_objects_final_state_keep_set(Elm_Transit *transit, Eina_Bool state_keep)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);
   if (transit->state_keep == state_keep) return;
   if (transit->animator) return;
   transit->state_keep = !!state_keep;
   if (state_keep) return;
   eina_hash_foreach(transit->objs_data_hash, _hash_foreach_obj_states_save, NULL);
}

/**
 * Get a value whether the objects states will be reset or not.
 *
 * @note @p transit can not be NULL
 *
 * @see elm_transit_objects_final_state_keep_set()
 *
 * @param transit The transit object.
 * @return EINA_TRUE means the states of the objects will be reset.
 * If @p transit is NULL, EINA_FALSE is returned
 *
 * @ingroup Transit
 */
EAPI Eina_Bool
elm_transit_objects_final_state_keep_get(const Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, EINA_FALSE);
   return transit->state_keep;
}

/**
 * Makes the chain relationship between two transits.
 *
 * @note @p transit can not be NULL. Transit would have multiple chain transits.
 * @note @p chain_transit can not be NULL. Chain transits could be chained to the only one transit.
 *
 * @param transit The transit object.
 * @param chain_transit The chain transit object. This transit will be operated
 *        after transit is done.
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_chain_transit_add(Elm_Transit *transit, Elm_Transit *chain_transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit);
   ELM_TRANSIT_CHECK_OR_RETURN(chain_transit);

   if (transit == chain_transit) return;
   if (transit == chain_transit->prev_chain_transit) return;

   if (chain_transit->prev_chain_transit)
     chain_transit->prev_chain_transit->next_chain_transits = eina_list_remove(chain_transit->prev_chain_transit->next_chain_transits, chain_transit);

   chain_transit->prev_chain_transit = transit;
   transit->next_chain_transits = eina_list_append(transit->next_chain_transits, chain_transit);
}

/**
 * Get the current chain transit list.
 *
 * @note @p transit can not be NULL.
 *
 * @param transit The transit object.
 * @return chain transit list.
 *
 * @ingroup Transit
 */
EAPI Eina_List *
elm_transit_chain_transits_get(const Elm_Transit * transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);
   return transit->next_chain_transits;
}

///////////////////////////////////////////////////////////////////////////////
//Resizing Effect
///////////////////////////////////////////////////////////////////////////////
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

/**
 * Add the Resizing Effect to Elm_Transit.
 *
 * @note This API is one of the facades. It creates resizing effect context
 * and add it's required APIs to elm_transit_effect_add.
 *
 * @see elm_transit_effect_add()
 *
 * @param transit Transit object.
 * @param from_w Object width size when effect begins.
 * @param from_h Object height size when effect begins.
 * @param to_w Object width size when effect ends.
 * @param to_h Object height size when effect ends.
 * @return Resizing effect context data.
 *
 * @ingroup Transit
 */
EAPI Elm_Transit_Effect *
elm_transit_effect_resizing_add(Elm_Transit *transit, Evas_Coord from_w, Evas_Coord from_h, Evas_Coord to_w, Evas_Coord to_h)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);
   Elm_Transit_Effect *effect = _transit_effect_resizing_context_new(from_w, from_h, to_w, to_h);

   if (!effect) return NULL;
   elm_transit_effect_add(transit,
                          _transit_effect_resizing_op, effect,
                          _transit_effect_resizing_context_free);
   return effect;
}

///////////////////////////////////////////////////////////////////////////////
//Translation Effect
///////////////////////////////////////////////////////////////////////////////
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

/**
 * Add the Translation Effect to Elm_Transit.
 *
 * @note This API is one of the facades. It creates translation effect context
 * and add it's required APIs to elm_transit_effect_add.
 *
 * @see elm_transit_effect_add()
 *
 * @param transit Transit object.
 * @param from_dx X Position variation when effect begins.
 * @param from_dy Y Position variation when effect begins.
 * @param to_dx X Position variation when effect ends.
 * @param to_dy Y Position variation when effect ends.
 * @return Translation effect context data.
 *
 * @ingroup Transit
 * @warning Is higher recommended just create a transit with this effect when
 * the window that the objects of the transit belongs has already been created.
 * This is because this effect needs the geometry information about the objects,
 * and if the window was not created yet, it can get a wrong information.
 */
EAPI Elm_Transit_Effect *
elm_transit_effect_translation_add(Elm_Transit *transit, Evas_Coord from_dx, Evas_Coord from_dy, Evas_Coord to_dx, Evas_Coord to_dy)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);
   Elm_Transit_Effect *effect_context = _transit_effect_translation_context_new(from_dx, from_dy, to_dx, to_dy);

   if (!effect_context) return NULL;
   elm_transit_effect_add(transit,
                          _transit_effect_translation_op, effect_context,
                          _transit_effect_translation_context_free);
   return effect_context;
}


///////////////////////////////////////////////////////////////////////////////
//Zoom Effect
///////////////////////////////////////////////////////////////////////////////
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

/**
 * Add the Zoom Effect to Elm_Transit.
 *
 * @note This API is one of the facades. It creates zoom effect context
 * and add it's required APIs to elm_transit_effect_add.
 *
 * @see elm_transit_effect_add()
 *
 * @param transit Transit object.
 * @param from_rate Scale rate when effect begins (1 is current rate).
 * @param to_rate Scale rate when effect ends.
 * @return Zoom effect context data.
 *
 * @ingroup Transit
 * @warning Is higher recommended just create a transit with this effect when
 * the window that the objects of the transit belongs has already been created.
 * This is because this effect needs the geometry information about the objects,
 * and if the window was not created yet, it can get a wrong information.
 */
EAPI Elm_Transit_Effect *
elm_transit_effect_zoom_add(Elm_Transit *transit, float from_rate, float to_rate)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);
   Elm_Transit_Effect *effect_context = _transit_effect_zoom_context_new(from_rate, to_rate);

   if (!effect_context) return NULL;
   elm_transit_effect_add(transit,
                          _transit_effect_zoom_op, effect_context,
                          _transit_effect_zoom_context_free);
   return effect_context;
}


///////////////////////////////////////////////////////////////////////////////
//Flip Effect
///////////////////////////////////////////////////////////////////////////////
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

/**
 * Add the Flip Effect to Elm_Transit.
 *
 * @note This API is one of the facades. It creates flip effect context
 * and add it's required APIs to elm_transit_effect_add.
 * @note This effect is applied to each pair of objects in the order they are listed
 * in the transit list of objects. The first object in the pair will be the
 * "front" object and the second will be the "back" object.
 *
 * @see elm_transit_effect_add()
 *
 * @param transit Transit object.
 * @param axis Flipping Axis(X or Y).
 * @param cw Flipping Direction. EINA_TRUE is clock-wise.
 * @return Flip effect context data.
 *
 * @ingroup Transit
 * @warning Is higher recommended just create a transit with this effect when
 * the window that the objects of the transit belongs has already been created.
 * This is because this effect needs the geometry information about the objects,
 * and if the window was not created yet, it can get a wrong information.
 */
EAPI Elm_Transit_Effect *
elm_transit_effect_flip_add(Elm_Transit *transit, Elm_Transit_Effect_Flip_Axis axis, Eina_Bool cw)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);
   Elm_Transit_Effect *effect_context = _transit_effect_flip_context_new(axis, cw);

   if (!effect_context) return NULL;
   elm_transit_effect_add(transit,
                          _transit_effect_flip_op, effect_context,
                          _transit_effect_flip_context_free);
   return effect_context;
}

///////////////////////////////////////////////////////////////////////////////
//ResizableFlip Effect
///////////////////////////////////////////////////////////////////////////////
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

/**
 * Add the Resizable Flip Effect to Elm_Transit.
 *
 * @note This API is one of the facades. It creates resizable flip effect context
 * and add it's required APIs to elm_transit_effect_add.
 * @note This effect is applied to each pair of objects in the order they are listed
 * in the transit list of objects. The first object in the pair will be the
 * "front" object and the second will be the "back" object.
 *
 * @see elm_transit_effect_add()
 *
 * @param transit Transit object.
 * @param axis Flipping Axis(X or Y).
 * @param cw Flipping Direction. EINA_TRUE is clock-wise.
 * @return Resizable flip effect context data.
 *
 * @ingroup Transit
 * @warning Is higher recommended just create a transit with this effect when
 * the window that the objects of the transit belongs has already been created.
 * This is because this effect needs the geometry information about the objects,
 * and if the window was not created yet, it can get a wrong information.
 */
EAPI Elm_Transit_Effect *
elm_transit_effect_resizable_flip_add(Elm_Transit *transit, Elm_Transit_Effect_Flip_Axis axis, Eina_Bool cw)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);
   Elm_Transit_Effect *effect_context = _transit_effect_resizable_flip_context_new(axis, cw);

   if (!effect_context) return NULL;
   elm_transit_effect_add(transit,
                          _transit_effect_resizable_flip_op, effect_context,
                          _transit_effect_resizable_flip_context_free);
   return effect_context;
}


///////////////////////////////////////////////////////////////////////////////
//Wipe Effect
///////////////////////////////////////////////////////////////////////////////
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

/**
 * Add the Wipe Effect to Elm_Transit.
 *
 * @note This API is one of the facades. It creates wipe effect context
 * and add it's required APIs to elm_transit_effect_add.
 *
 * @see elm_transit_effect_add()
 *
 * @param transit Transit object.
 * @param type Wipe type. Hide or show.
 * @param dir Wipe Direction.
 * @return Wipe effect context data.
 *
 * @ingroup Transit
 * @warning Is higher recommended just create a transit with this effect when
 * the window that the objects of the transit belongs has already been created.
 * This is because this effect needs the geometry information about the objects,
 * and if the window was not created yet, it can get a wrong information.
 */
EAPI void *
elm_transit_effect_wipe_add(Elm_Transit *transit, Elm_Transit_Effect_Wipe_Type type, Elm_Transit_Effect_Wipe_Dir dir)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);
   void *effect_context = _transit_effect_wipe_context_new(type, dir);

   if (!effect_context) return NULL;
   elm_transit_effect_add(transit,
                          _transit_effect_wipe_op, effect_context,
                          _transit_effect_wipe_context_free);
   return effect_context;
}


///////////////////////////////////////////////////////////////////////////////
//Color Effect
///////////////////////////////////////////////////////////////////////////////
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

/**
 * Add the Color Effect to Elm_Transit.
 *
 * @note This API is one of the facades. It creates color effect context
 * and add it's required APIs to elm_transit_effect_add.
 *
 * @see elm_transit_effect_add()
 *
 * @param transit        Transit object.
 * @param  from_r        RGB R when effect begins.
 * @param  from_g        RGB G when effect begins.
 * @param  from_b        RGB B when effect begins.
 * @param  from_a        RGB A when effect begins.
 * @param  to_r          RGB R when effect ends.
 * @param  to_g          RGB G when effect ends.
 * @param  to_b          RGB B when effect ends.
 * @param  to_a          RGB A when effect ends.
 * @return               Color effect context data.
 *
 * @ingroup Transit
 */
EAPI Elm_Transit_Effect *
elm_transit_effect_color_add(Elm_Transit *transit, unsigned int from_r, unsigned int from_g, unsigned int from_b, unsigned int from_a, unsigned int to_r, unsigned int to_g, unsigned int to_b, unsigned int to_a)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);
   Elm_Transit_Effect *effect_context = _transit_effect_color_context_new(from_r, from_g, from_b, from_a, to_r, to_g, to_b, to_a);

   if (!effect_context) return NULL;
   elm_transit_effect_add(transit,
                          _transit_effect_color_op, effect_context,
                          _transit_effect_color_context_free);
   return effect_context;
}

///////////////////////////////////////////////////////////////////////////////
//Fade Effect
///////////////////////////////////////////////////////////////////////////////
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

/**
 * Add the Fade Effect to Elm_Transit.
 *
 * @note This API is one of the facades. It creates fade effect context
 * and add it's required APIs to elm_transit_effect_add.
 * @note This effect is applied to each pair of objects in the order they are listed
 * in the transit list of objects. The first object in the pair will be the
 * "before" object and the second will be the "after" object.
 *
 * @see elm_transit_effect_add()
 *
 * @param transit Transit object.
 * @return Fade effect context data.
 *
 * @ingroup Transit
 * @warning Is higher recommended just create a transit with this effect when
 * the window that the objects of the transit belongs has already been created.
 * This is because this effect needs the color information about the objects,
 * and if the window was not created yet, it can get a wrong information.
 */
EAPI Elm_Transit_Effect *
elm_transit_effect_fade_add(Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);

   Elm_Transit_Effect *effect_context = _transit_effect_fade_context_new();
   if (!effect_context) return NULL;
   elm_transit_effect_add(transit,
                          _transit_effect_fade_op, effect_context,
                          _transit_effect_fade_context_free);
   return effect_context;
}


///////////////////////////////////////////////////////////////////////////////
//Blend Effect
///////////////////////////////////////////////////////////////////////////////
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

/**
 * Add the Blend Effect to Elm_Transit.
 *
 * @note This API is one of the facades. It creates blend effect context
 * and add it's required APIs to elm_transit_effect_add.
 * @note This effect is applied to each pair of objects in the order they are listed
 * in the transit list of objects. The first object in the pair will be the
 * "before" object and the second will be the "after" object.
 *
 * @see elm_transit_effect_add()
 *
 * @param transit Transit object.
 * @return Blend effect context data.
 *
 * @ingroup Transit
 * @warning Is higher recommended just create a transit with this effect when
 * the window that the objects of the transit belongs has already been created.
 * This is because this effect needs the color information about the objects,
 * and if the window was not created yet, it can get a wrong information.
 */
EAPI Elm_Transit_Effect *
elm_transit_effect_blend_add(Elm_Transit *transit)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);
   Elm_Transit_Effect *effect_context = _transit_effect_blend_context_new();

   if (!effect_context) return NULL;
   elm_transit_effect_add(transit,
                          _transit_effect_blend_op, effect_context,
                          _transit_effect_blend_context_free);
   return effect_context;
}


///////////////////////////////////////////////////////////////////////////////
//Rotation Effect
///////////////////////////////////////////////////////////////////////////////
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

/**
 * Add the Rotation Effect to Elm_Transit.
 *
 * @note This API is one of the facades. It creates rotation effect context
 * and add it's required APIs to elm_transit_effect_add.
 *
 * @see elm_transit_effect_add()
 *
 * @param transit Transit object.
 * @param from_degree Degree when effect begins.
 * @param to_degree Degree when effect is ends.
 * @return Rotation effect context data.
 *
 * @ingroup Transit
 * @warning Is higher recommended just create a transit with this effect when
 * the window that the objects of the transit belongs has already been created.
 * This is because this effect needs the geometry information about the objects,
 * and if the window was not created yet, it can get a wrong information.
 */
EAPI Elm_Transit_Effect *
elm_transit_effect_rotation_add(Elm_Transit *transit, float from_degree, float to_degree)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);
   Elm_Transit_Effect *effect_context = _transit_effect_rotation_context_new(from_degree, to_degree);

   if (!effect_context) return NULL;
   elm_transit_effect_add(transit,
                          _transit_effect_rotation_op, effect_context,
                          _transit_effect_rotation_context_free);
   return effect_context;
}


///////////////////////////////////////////////////////////////////////////////
//ImageAnimation Effect
///////////////////////////////////////////////////////////////////////////////
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
        if (elm_widget_type_check(obj, type))
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

/**
 * Add the ImageAnimation Effect to Elm_Transit.
 *
 * @note This API is one of the facades. It creates image animation effect context
 * and add it's required APIs to elm_transit_effect_add.
 * The @p images parameter is a list images paths. This list and
 * its contents will be deleted at the end of the effect by
 * elm_transit_effect_image_animation_context_free() function.
 *
 * Example:
 * @code
 * char buf[PATH_MAX];
 * Eina_List *images = NULL;
 * Elm_Transit *transi = elm_transit_add();
 *
 * snprintf(buf, sizeof(buf), "%s/images/icon_11.png", PACKAGE_DATA_DIR);
 * images = eina_list_append(images, eina_stringshare_add(buf));
 *
 * snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
 * images = eina_list_append(images, eina_stringshare_add(buf));
 * elm_transit_effect_image_animation_add(transi, images);
 *
 * @endcode
 *
 * @see elm_transit_effect_add()
 *
 * @param transit Transit object.
 * @param images Eina_List of images file paths. This list and
 * its contents will be deleted at the end of the effect by
 * elm_transit_effect_image_animation_context_free() function.
 * @return Image Animation effect context data.
 *
 * @ingroup Transit
 */
EAPI Elm_Transit_Effect *
elm_transit_effect_image_animation_add(Elm_Transit *transit, Eina_List *images)
{
   ELM_TRANSIT_CHECK_OR_RETURN(transit, NULL);
   Elm_Transit_Effect *effect = _transit_effect_image_animation_context_new(images);

   if (!effect) return NULL;
   elm_transit_effect_add(transit,
                          _transit_effect_image_animation_op, effect,
                          _transit_effect_image_animation_context_free);
   return effect;
}
