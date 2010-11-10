#include <Elementary.h>
#include "elm_priv.h"

/**
 *
 * @defgroup Transit Transit
 * @ingroup Elementary
 *
 * Transit is designed to set the various effects for the Evas_Object such like
 * translation, rotation, etc. For using Effects, Create transit and insert
 * effects which are interesting.
 * Once effects are inserted into transit, transit will manage those effects.
 * (ex) deleting).
*/

static const char _transit_key[] = "_elm_transit";

struct _Elm_Transit
{
   Eina_List *effect_list;
   Eina_Bool block:1;
   void (*completion_op) (void *data, Elm_Transit *transit);
   void *completion_arg;
   int walking;
   Eina_List *objs;
   Ecore_Animator *animator;
   double begin_time;
   double cur_time;
   double duration;
   int repeat_cnt;
   int cur_repeat_cnt;
   Elm_Transit_Tween_Mode tween_mode;
   Eina_Bool auto_reverse:1;
};

struct _Elm_Effect
{
   void (*animation_op) (void *data, Elm_Transit *transit, double progress);
   void (*user_data_free) (void *data, Elm_Transit *transit);
   void *user_data;
   Eina_Bool deleted:1;
};

typedef struct _Elm_Effect Elm_Effect;

static void _transit_animate_op(Elm_Transit *transit, double progress);
static void _elm_transit_effect_del(Elm_Transit *transit, Elm_Effect *effect);
static void _remove_dead_effects(Elm_Transit *transit);
static void _elm_transit_object_remove_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__);
static void _elm_transit_object_remove(Elm_Transit *transit, Evas_Object *obj);
static Eina_Bool _animator_animate_cb(void *data);
static unsigned int _animator_compute_no_reverse_repeat_count(unsigned int cnt);
static unsigned int _animator_compute_reverse_repeat_count(unsigned int cnt);

static void
_remove_dead_effects(Elm_Transit *transit)
{
   Eina_List *elist, *elist_next;

   Elm_Effect *effect;

   EINA_LIST_FOREACH_SAFE(transit->effect_list, elist, elist_next, effect)
     {
        if (effect->deleted)
          _elm_transit_effect_del(transit, effect);
     }
}

static void
_transit_animate_op(Elm_Transit *transit, double progress)
{
   Eina_List *elist;

   Elm_Effect *effect;

   transit->walking++;
   EINA_LIST_FOREACH(transit->effect_list, elist, effect)
     {
        if (!effect->deleted)
          effect->animation_op(effect->user_data, transit, progress);
     }
   transit->walking--;

   if (!transit->walking)
     _remove_dead_effects(transit);
}

/**
 * Add a new effect to the transit
 *
 * The @p cb function is called when the animation begins, it is the function
 * that actually performs the animation. It is called with the @p data, @p
 * transit and the time progression of the animation (it is a number between 0
 * and 1). The @p data_free_cb is the function that will be called at the end
 * of the animation, it must free the @p data and finalize the animation.
 *
 * The cb function and the data are the key to the effect. If you try to add an
 * already added effect, nothing is done.
 * If the transit is null, nothing is done.
 *
 * @param transit Transit object
 * @param cb The operation function
 * @param data The context data of the effect
 * @param data_free_cb The function to free the context data
 *
 * @ingroup Transit
 * @warning The transit free the context data at the and of the transition.
 */
EAPI void
elm_transit_effect_add(Elm_Transit *transit, void (*cb)(void *data, Elm_Transit *transit, double progress), void *data, void (*data_free_cb)(void *data, Elm_Transit *transit))
{
   if (!transit) return;
   Elm_Effect *effect;

   Eina_List *elist;

   EINA_LIST_FOREACH(transit->effect_list, elist, effect)
     if ((effect->animation_op == cb) && (effect->user_data == data)) return;

   effect = ELM_NEW(Elm_Effect);
   if (!effect) return;

   effect->user_data_free = data_free_cb;
   effect->animation_op = cb;
   effect->user_data = data;

   transit->effect_list = eina_list_append(transit->effect_list, effect);
}

/**
 * Delete an added effect
 *
 * @param transit Transit object
 * @param cb The operation function
 * @param data The context data of the effect
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_effect_del(Elm_Transit *transit, void (*cb)(void *data, Elm_Transit *transit, double progress), void *data)
{
   Eina_List *elist, *elist_next;
   Elm_Effect *effect;

   EINA_LIST_FOREACH_SAFE(transit->effect_list, elist, elist_next, effect)
     {
        if ((effect->animation_op == cb) && (effect->user_data == data))
          {
             _elm_transit_effect_del(transit, effect);
             return;
          }
     }
}

static void
_elm_transit_effect_del(Elm_Transit *transit, Elm_Effect *effect)
{
   if (effect->user_data_free)
     {
        effect->user_data_free(effect->user_data, transit);
        effect->user_data = NULL;
     }
   if (transit->walking)
     {
        effect->deleted = EINA_TRUE;
        return;
     }

   transit->effect_list = eina_list_remove(transit->effect_list, effect);
   free(effect);
}

/**
 * Set the event blocked when transit is operating.
 *
 * If @p disabled is EINA_TRUE, the objects of the transit will be disabled.
 *
 * @param transit Transit object
 * @param disabled Disable or enable
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_event_block_set(Elm_Transit *transit, Eina_Bool disabled)
{
   if (!transit) return;
   if (transit->block == disabled) return;

   Evas_Object *obj;
   Eina_List *elist;

   transit->block = disabled;

   if (!transit->block)
     {
        EINA_LIST_FOREACH(transit->objs, elist, obj)
          {
             Eina_Bool *state = evas_object_data_get(obj, _transit_key);
             evas_object_pass_events_set(obj, *state);
          }
     }
   else
     {
        EINA_LIST_FOREACH(transit->objs, elist, obj)
          evas_object_pass_events_set(obj, EINA_TRUE);
     }
}

/**
 * Get the value of event blocked status.
 *
 * @param transit Transit
 * @return EINA_TRUE, when event block is disabled
 *
 * @ingroup Transit
 */
EAPI Eina_Bool
elm_transit_event_block_get(const Elm_Transit *transit)
{
   if (!transit) return EINA_FALSE;
   return transit->block;
}

/**
 * Set the user-callback function when the transit operation is done.
 *
 * @param transit	Transit
 * @param op Callback function pointer
 * @param data Callback funtion user data
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_del_cb_set(Elm_Transit *transit, void (*op) (void *data, Elm_Transit *transit), void *data)
{
   if (!transit) return;
   transit->completion_op = op;
   transit->completion_arg = data;
}

/**
 * Delete transit.
 *
 * Stops the animation and delete the @p transit object
 *
 * @param transit	Transit to be deleted
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_del(Elm_Transit *transit)
{
   Eina_List *elist, *elist_next;
   Elm_Effect *effect;
   if (!transit) return;

   if (transit->block)
     elm_transit_event_block_set(transit, EINA_FALSE);

   ecore_animator_del(transit->animator);

   if (transit->completion_op)
     transit->completion_op(transit->completion_arg, transit);

   EINA_LIST_FOREACH_SAFE(transit->effect_list, elist,elist_next, effect)
     _elm_transit_effect_del(transit, effect);

   while (transit->objs)
     _elm_transit_object_remove(transit, eina_list_data_get(transit->objs));

   free(transit);
}

static double
_tween_progress_calc(Elm_Transit *transit, double progress)
{
  switch (transit->tween_mode)
    {
     case ELM_TRANSIT_TWEEN_MODE_ACCELERATE:
        return 1.0 - sin((ELM_PI / 2.0) + (progress * ELM_PI / 2.0));
     case ELM_TRANSIT_TWEEN_MODE_DECELERATE:
        return sin(progress * ELM_PI / 2.0);
     case ELM_TRANSIT_TWEEN_MODE_SINUSOIDAL:
        return (1.0 - cos(progress * ELM_PI)) / 2.0;
     default:
        return progress;
    }
}

/**
 * Set the transit animation acceleration style.
 *
 * @param transit	Transit
 * @param tween_mode The tween type
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_tween_mode_set(Elm_Transit *transit, Elm_Transit_Tween_Mode tween_mode)
{
   if (!transit) return;
   transit->tween_mode = tween_mode;
}

/**
 * Add new transit.
 *
 * @param duration Duration of the transit
 * @return transit
 *
 * @ingroup Transit
 */
EAPI Elm_Transit *
elm_transit_add(double duration)
{
   Elm_Transit *transit = ELM_NEW(Elm_Transit);

   if (!transit) return NULL;

   elm_transit_tween_mode_set(transit, ELM_TRANSIT_TWEEN_MODE_LINEAR);

   transit->duration = duration;

   transit->begin_time = ecore_loop_time_get();
   transit->animator = ecore_animator_add(_animator_animate_cb, transit);
   return transit;
}

static Eina_Bool
_animator_animate_cb(void *data)
{
   Elm_Transit *transit = data;

   transit->cur_time = ecore_loop_time_get();
   double elapsed_time = transit->cur_time - transit->begin_time;

   if (elapsed_time > transit->duration)
     elapsed_time = transit->duration;

   double progress = _tween_progress_calc(transit,
                                             elapsed_time / transit->duration);

   /* Reverse? */
   if (transit->auto_reverse)
     {
	if ((transit->cur_repeat_cnt % 2))
	  progress = 1 - progress;
     }

   if (transit->duration > 0)
     _transit_animate_op(transit, progress);

   /* Not end. Keep going. */
   if (elapsed_time < transit->duration)
     return ECORE_CALLBACK_RENEW;

   /* Repeat and reverse and time done! */
   if (transit->cur_repeat_cnt == transit->repeat_cnt)
     {
        elm_transit_del(transit);
	return ECORE_CALLBACK_CANCEL;
     }

   /* Repeat Case */
   transit->cur_repeat_cnt++;
   transit->begin_time = ecore_loop_time_get();

   return ECORE_CALLBACK_RENEW;
}

/**
 * Add new object to apply the effects
 *
 * @note After the first addition of an object in @p transit, if its
 * object list become empty again, the @p transit will be killed by
 * elm_transit_del(transit, obj) function.
 *
 * @param transit Transit object
 * @param obj Object
 * @return transit
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_object_add(Elm_Transit *transit, Evas_Object *obj)
{
   if (!transit) return;
   if (!obj) return;
   if (eina_list_data_find(transit->objs, obj)) return;

   Eina_Bool *state;

   transit->objs = eina_list_append(transit->objs, obj);

   state = ELM_NEW(Eina_Bool);
   *state = evas_object_pass_events_get(obj);

   evas_object_data_set(obj, _transit_key, state);

   if (transit->block)
     evas_object_pass_events_set(obj, EINA_TRUE);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
                                  _elm_transit_object_remove_cb, transit);
}

static void
_elm_transit_object_remove(Elm_Transit *transit, Evas_Object *obj)
{
   Eina_Bool *state;

   state = evas_object_data_del(obj, _transit_key);
   free(state);

   transit->objs = eina_list_remove(transit->objs, obj);

   evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL,
                                  _elm_transit_object_remove_cb);
}

static void
_elm_transit_object_remove_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Elm_Transit *transit = data;

   _elm_transit_object_remove(transit, obj);

   if (!transit->objs)
     elm_transit_del(transit);
}

/**
 * Removes an added object from the transit
 *
 * @note If the list become empty, this function will call
 * elm_transit_del(transit, obj), that is, it will kill the @p transit.
 *
 * @param transit Transit object
 * @param obj Object
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_object_remove(Elm_Transit *transit, Evas_Object *obj)
{
   if (!transit) return;
   if (!obj) return;

   _elm_transit_object_remove(transit, obj);

   if (!transit->objs)
     elm_transit_del(transit);
}

/**
 * Get the objects of the transit
 *
 * @param transit Transit object
 * @return a Eina_List with the objects from the transit
 *
 * @ingroup Transit
 */
EAPI const Eina_List *
elm_transit_objects_get(const Elm_Transit *transit)
{
   if (!transit) return NULL;
   return transit->objs;
}

/**
 * Set reverse effect automatically.
 *
 * If auto reverse is setted, after running the effects with the progress
 * parameter from 0 to 1, it will call the effecs again with the progress
 * from 1 to 0. The transit will last fot a time iqual to (2 * duration * repeat),
 * where the duration was setted with the function elm_transit_add and
 * the repeat with the function elm_transit_repeat_times_set().
 *
 * @param transit Transit
 * @param reverse EINA_TRUE is reverse.
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_auto_reverse_set(Elm_Transit *transit, Eina_Bool reverse)
{
   if (!transit) return;
   if (transit->auto_reverse == reverse) return;
   transit->auto_reverse = reverse;
   if (reverse)
     {
	transit->repeat_cnt =
	  _animator_compute_reverse_repeat_count(transit->repeat_cnt);
     }
   else
     {
	transit->repeat_cnt =
	  _animator_compute_no_reverse_repeat_count(transit->repeat_cnt);
     }
}

static unsigned int
_animator_compute_no_reverse_repeat_count(unsigned int cnt)
{
   return cnt / 2;
}

static unsigned int
_animator_compute_reverse_repeat_count(unsigned int cnt)
{
   return ((cnt + 1) * 2) - 1;
}

/**
 * Set the transit repeat count. Effect will be repeated by repeat count.
 *
 * This function define the number of repetition the transit will run after
 * the first one, that is, if @p repeat is 1, the transit will run 2 times.
 * If the @p repeat is a negative number, it will repeat infinite times.
 *
 * @note If this function is called during the transit execution, the transit
 * will run @p repeat times, ignoring the times it already performed.
 *
 * @param transit Transit
 * @param repeat Repeat count
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_repeat_times_set(Elm_Transit *transit, int repeat)
{
   if (!transit) return;

   transit->repeat_cnt = repeat;
   transit->cur_repeat_cnt = 0;

   if (!transit->auto_reverse || repeat < 0)
     transit->repeat_cnt = repeat;
   else
     {
	transit->repeat_cnt =
	  _animator_compute_reverse_repeat_count(repeat);
     }
}

///////////////////////////////////////////////////////////////////////////////
//Resizing FX
///////////////////////////////////////////////////////////////////////////////
typedef struct _Elm_Fx_Resizing Elm_Fx_Resizing;

struct _Elm_Fx_Resizing
{
   struct _size
     {
        Evas_Coord w, h;
     } from, to;
};

/**
 * The Free function to Resizing Effect context data.
 *
 * @see elm_transit_effect_add()
 *
 * @param data The Resizing context data.
 * @param transt Transit object
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_effect_resizing_context_free(void *data, Elm_Transit *transit __UNUSED__)
{
   free(data);
}

/**
 * Operation function to the Resizing Effect
 *
 * @see elm_transit_effect_add()
 *
 * @param data The Resizing context data.
 * @param transt Transit object
 * @param progress The time progression, it is a number between 0 and 1
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_effect_resizing_op(void *data, Elm_Transit *transit, double progress)
{
   if (!data) return;
   Evas_Coord w, h;
   Evas_Object *obj;
   Eina_List *elist;

   Elm_Fx_Resizing *resizing = data;

   w = resizing->from.w + (Evas_Coord) ((float)resizing->to.w * (float)progress);
   h = resizing->from.h + (Evas_Coord) ((float)resizing->to.h * (float)progress);

   EINA_LIST_FOREACH(transit->objs, elist, obj)
     evas_object_resize(obj, w, h);
}

/**
 * Get a new context data of Resizing Effect
 *
 * @see elm_transit_effect_add()
 *
 * @param from_w Object width size when effect begins
 * @param from_h Object height size when effect begins
 * @param to_w Object width size when effect ends
 * @param to_h Object height size when effect ends
 * @return Resizing effect context data
 *
 * @ingroup Transit
 */
EAPI void *
elm_transit_effect_resizing_context_new(Evas_Coord from_w, Evas_Coord from_h, Evas_Coord to_w, Evas_Coord to_h)
{
   Elm_Fx_Resizing *resizing;

   resizing = ELM_NEW(Elm_Fx_Resizing);
   if (!resizing) return NULL;

   resizing->from.w = from_w;
   resizing->from.h = from_h;
   resizing->to.w = to_w - from_w;
   resizing->to.h = to_h - from_h;

   return resizing;
}

///////////////////////////////////////////////////////////////////////////////
//Translation FX
///////////////////////////////////////////////////////////////////////////////
typedef struct _Elm_Fx_Translation Elm_Fx_Translation;

struct _Elm_Fx_Translation
{
   struct _point
     {
        Evas_Coord x, y;
     } from, to;
};

/**
 * The Free function to Translation Effect context data.
 *
 * @see elm_transit_effect_add()
 *
 * @param data The Translation context data.
 * @param transt Transit object
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_effect_translation_context_free(void *data, Elm_Transit *transit __UNUSED__)
{
   free(data);
}

/**
 * Operation function to the Translation Effect
 *
 * @see elm_transit_effect_add()
 *
 * @param data The Translation context data.
 * @param transt Transit object
 * @param progress The time progression, it is a number between 0 and 1
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_effect_translation_op(void *data, Elm_Transit *transit, double progress)
{
   if (!data) return;
   Evas_Coord x, y;
   Evas_Object *obj;
   Eina_List *elist;

   Elm_Fx_Translation *translation = data;

   x = translation->from.x +
      (Evas_Coord) ((float)translation->to.x * (float)progress);
   y = translation->from.y +
      (Evas_Coord) ((float)translation->to.y * (float)progress);

   EINA_LIST_FOREACH(transit->objs, elist, obj)
     evas_object_move(obj, x, y);
}

/**
 * Get a new context data of Translation Effect
 *
 * @see elm_transit_effect_add()
 *
 * @param from_x Position X when effect begins
 * @param from_y Position Y when effect begins
 * @param to_x Position X when effect ends
 * @param to_y Position Y when effect ends
 * @return Translation effect context data
 *
 * @ingroup Transit
 */
EAPI void *
elm_transit_effect_translation_context_new(Evas_Coord from_x, Evas_Coord from_y, Evas_Coord to_x, Evas_Coord to_y)
{
   Elm_Fx_Translation *translation;

   translation = ELM_NEW(Elm_Fx_Translation);

   if (!translation) return NULL;

   translation->from.x = from_x;
   translation->from.y = from_y;
   translation->to.x = to_x - from_x;
   translation->to.y = to_y - from_y;

   return translation;
}

///////////////////////////////////////////////////////////////////////////////
//Zoom FX
///////////////////////////////////////////////////////////////////////////////
typedef struct _Elm_Fx_Zoom Elm_Fx_Zoom;

struct _Elm_Fx_Zoom
{
   float from, to;
};

/**
 * The Free function to Zoom Effect context data.
 *
 * @see elm_transit_effect_add()
 *
 * @param data The Zoom context data.
 * @param transt Transit object
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_effect_zoom_context_free(void *data, Elm_Transit *transit __UNUSED__)
{
   free(data);
}

/**
 * Operation function to the Zoom Effect
 *
 * @see elm_transit_effect_add()
 *
 * @param data The Zoom context data.
 * @param transt Transit object
 * @param progress The time progression, it is a number between 0 and 1
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_effect_zoom_op(void *data, Elm_Transit *transit , double progress)
{
   if (!data) return;
   Evas_Object *obj;
   Eina_List *elist;

   Elm_Fx_Zoom *zoom = data;

   Evas_Map *map;

   Evas_Coord x, y, w, h;

   map = evas_map_new(4);
   if (!map) return;

   EINA_LIST_FOREACH(transit->objs, elist, obj)
     {
        evas_object_geometry_get(obj, &x, &y, &w, &h);
        evas_map_smooth_set(map, EINA_TRUE);
        evas_map_util_points_populate_from_object_full(map, obj, zoom->from + \
                                                       (progress * zoom->to));
        evas_map_util_3d_perspective(map, x + (w / 2), y + (h / 2), 0, 10000);
        evas_object_map_set(obj, map);
        evas_object_map_enable_set(obj, EINA_TRUE);
     }
   evas_map_free(map);
}

/**
 * Get a new context data of Zoom Effect
 *
 * @see elm_transit_effect_add()
 *
 * @param from_rate Scale rate when effect begins (1 is current rate)
 * @param to_rate Scale rate when effect ends
 * @return Zoom effect context data
 *
 * @ingroup Transit
 */
EAPI void *
elm_transit_effect_zoom_context_new(float from_rate, float to_rate)
{
   Elm_Fx_Zoom *zoom;

   zoom = ELM_NEW(Elm_Fx_Zoom);
   if (!zoom) return NULL;

   zoom->from = (10000 - (from_rate * 10000)) * (1 / from_rate);
   zoom->to = ((10000 - (to_rate * 10000)) * (1 / to_rate)) - zoom->from;

   return zoom;

}

///////////////////////////////////////////////////////////////////////////////
//Flip FX
///////////////////////////////////////////////////////////////////////////////
typedef struct _Elm_Fx_Flip Elm_Fx_Flip;

struct _Elm_Fx_Flip
{
   Elm_Fx_Flip_Axis axis;
   Eina_Bool cw:1;
};

/**
 * The Free function to Flip Effect context data.
 *
 * @see elm_transit_effect_add()
 *
 * @param data The Flip context data.
 * @param transt Transit object
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_effect_flip_context_free(void *data, Elm_Transit *transit)
{
   Evas_Object *front, *back;
   int i;

   int count = eina_list_count(transit->objs);
   for(i = 0; i < count-1; i+=2)
     {
        front = eina_list_nth(transit->objs, i);
        back = eina_list_nth(transit->objs, i+1);
        evas_object_map_enable_set(front, EINA_FALSE);
        evas_object_map_enable_set(back, EINA_FALSE);
     }

   free(data);
}

/**
 * Operation function to the Flip Effect
 *
 * This effect is applied to each pair of objects in the order they are listed
 * in the transit list of objects. The first object in the pair will be the
 * "front" object and the second will be the "back" object.
 *
 * @see elm_transit_effect_add()
 *
 * @param data The Flip context data.
 * @param transt Transit object
 * @param progress The time progression, it is a number between 0 and 1
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_effect_flip_op(void *data, Elm_Transit *transit, double progress)
{
   if (!data) return;
   Evas_Object *obj, *front, *back;
   int count;
   int i;

   Elm_Fx_Flip *flip = data;

   Evas_Map *map;

   float degree;

   Evas_Coord x, y, w, h;

   map = evas_map_new(4);
   if (!map) return;

   if (flip->cw)
     degree = (float)(progress * 180);
   else
     degree = (float)(progress * -180);

   count = eina_list_count(transit->objs);
   for(i = 0; i < count-1; i+=2)
     {
        front = eina_list_nth(transit->objs, i);
        back = eina_list_nth(transit->objs, i+1);

        if (degree < 90 && degree > -90)
          {
             obj = front;
             evas_object_hide(back);
             evas_object_show(front);
          }
        else
          {
             obj = back;
             evas_object_hide(front);
             evas_object_show(back);
          }

        evas_map_smooth_set(map, EINA_TRUE);
        evas_map_util_points_populate_from_object_full(map, obj, 0);
        evas_object_geometry_get(obj, &x, &y, &w, &h);
        Evas_Coord half_w = (w / 2);

        Evas_Coord half_h = (h / 2);

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
        evas_map_util_3d_perspective(map, x + half_w, y + half_h, 0, 10000);
        evas_object_map_enable_set(front, EINA_TRUE);
        evas_object_map_enable_set(back, EINA_TRUE);
        evas_object_map_set(obj, map);
     }
   evas_map_free(map);
}

/**
 * Get a new context data of Flip Effect
 *
 * @see elm_transit_effect_add()
 *
 * @param axis Flipping Axis(X or Y)
 * @param cw Flipping Direction. EINA_TRUE is clock-wise
 * @return Flip effect context data
 *
 * @ingroup Transit
 */
EAPI void *
elm_transit_effect_flip_context_new(Elm_Fx_Flip_Axis axis, Eina_Bool cw)
{
   Elm_Fx_Flip *flip;

   flip = ELM_NEW(Elm_Fx_Flip);
   if (!flip) return NULL;

   flip->cw = cw;
   flip->axis = axis;

   return flip;
}

///////////////////////////////////////////////////////////////////////////////
//ResizableFlip FX
///////////////////////////////////////////////////////////////////////////////
typedef struct _Elm_Fx_Resizable_Flip Elm_Fx_ResizableFlip;
typedef struct _Elm_Fx_Resizable_Flip_Node Elm_Fx_ResizableFlip_Node;
static void _set_image_uv_by_axis_y(Evas_Map *map,
                                    Elm_Fx_ResizableFlip_Node *flip,
                                    float degree);
static void _set_image_uv_by_axis_x(Evas_Map *map,
                                    Elm_Fx_ResizableFlip_Node *flip,
                                    float degree);
static Eina_List *_resizable_flip_nodes_build(Elm_Transit *transit);

struct _Elm_Fx_Resizable_Flip_Node
{
   Evas_Object *front;
   Evas_Object *back;
   struct _vector2d
     {
        float x, y;
     } from_pos, from_size, to_pos, to_size;
};

struct _Elm_Fx_Resizable_Flip
{
   Eina_List *nodes;
   Eina_Bool cw:1;
   Elm_Fx_Flip_Axis axis;
};

/**
 * The Free function to Resizable Flip Effect context data.
 *
 * @see elm_transit_effect_add()
 *
 * @param data The Resizable Flip context data.
 * @param transt Transit object
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_effect_resizable_flip_context_free(void *data, Elm_Transit *transit __UNUSED__)
{
   Elm_Fx_ResizableFlip *resizable_flip = data;
   Eina_List *elist;
   Elm_Fx_ResizableFlip_Node *resizable_flip_node;

   EINA_LIST_FOREACH(resizable_flip->nodes, elist, resizable_flip_node)
     {
        evas_object_map_enable_set(resizable_flip_node->front, EINA_FALSE);
        evas_object_map_enable_set(resizable_flip_node->back, EINA_FALSE);
     }
   eina_list_free(resizable_flip->nodes);
   free(resizable_flip);
}

static void
_set_image_uv_by_axis_y(Evas_Map *map, Elm_Fx_ResizableFlip_Node *flip, float degree)
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
        evas_map_point_image_uv_set(map, 3, 0, flip->to_size.y);
     }
}

static void
_set_image_uv_by_axis_x(Evas_Map *map, Elm_Fx_ResizableFlip_Node *flip, float degree)
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
        evas_map_point_image_uv_set(map, 3, 0, flip->to_size.y);
     }
}

/**
 * Operation function to the Resizable Flip Effect
 *
 * This effect is applied to each pair of objects in the order they are listed
 * in the transit list of objects. The first object in the pair will be the
 * "front" object and the second will be the "back" object.
 *
 * @see elm_transit_effect_add()
 *
 * @param data The Resizable Flip context data.
 * @param transt Transit object
 * @param progress The time progression, it is a number between 0 and 1
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_effect_resizable_flip_op(void *data, Elm_Transit *transit __UNUSED__, double progress)
{
   if (!data) return;

   Evas_Map *map;
   Evas_Object *obj;
   float x, y, w, h;
   float degree;
   Evas_Coord half_w, half_h;

   Elm_Fx_ResizableFlip *resizable_flip = data;
   Elm_Fx_ResizableFlip_Node *resizable_flip_node;
   Eina_List *elist;

   map = evas_map_new(4);
   if (!map) return;

   if (resizable_flip->cw)
      degree = (float)(progress * 180);
   else
      degree = (float)(progress * -180);

   if (!resizable_flip->nodes)
     resizable_flip->nodes = _resizable_flip_nodes_build(transit);

   EINA_LIST_FOREACH(resizable_flip->nodes, elist, resizable_flip_node)
     {
        if ((degree < 90) && (degree > -90))
          {
             obj = resizable_flip_node->front;
             evas_object_hide(resizable_flip_node->back);
             evas_object_show(resizable_flip_node->front);
          }
        else
          {
             obj = resizable_flip_node->back;
             evas_object_hide(resizable_flip_node->front);
             evas_object_show(resizable_flip_node->back);
          }

        evas_map_smooth_set(map, EINA_TRUE);

        x = resizable_flip_node->from_pos.x + \
            (resizable_flip_node->to_pos.x * progress);
        y = resizable_flip_node->from_pos.y + \
            (resizable_flip_node->to_pos.y * progress);
        w = resizable_flip_node->from_size.x + \
            (resizable_flip_node->to_size.x * progress);
        h = resizable_flip_node->from_size.y + \
            (resizable_flip_node->to_size.y * progress);
        evas_map_point_coord_set(map, 0, x, y, 0);
        evas_map_point_coord_set(map, 1, x + w, y, 0);
        evas_map_point_coord_set(map, 2, x + w, y + h, 0);
        evas_map_point_coord_set(map, 3, x, y + h, 0);

        half_w = (Evas_Coord) (w / 2);
        half_h = (Evas_Coord) (h / 2);

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

        evas_map_util_3d_perspective(map, x + half_w, y + half_h, 0, 10000);
        evas_object_map_enable_set(resizable_flip_node->front, EINA_TRUE);
        evas_object_map_enable_set(resizable_flip_node->back, EINA_TRUE);
        evas_object_map_set(obj, map);
     }
   evas_map_free(map);
}

static Eina_List *
_resizable_flip_nodes_build(Elm_Transit *transit)
{
   Elm_Fx_ResizableFlip_Node *resizable_flip_node;
   Eina_List *data_list = NULL;

   Evas_Coord front_x, front_y, front_w, front_h;
   Evas_Coord back_x, back_y, back_w, back_h;

   int i, count;

   count = eina_list_count(transit->objs);
   for (i = 0; i < count-1; i+=2)
      {
         resizable_flip_node = ELM_NEW(Elm_Fx_ResizableFlip_Node);
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
      }

   return data_list;
}

/**
 * Get a new context data of Resizable Flip Effect
 *
 * @see elm_transit_effect_add()
 *
 * @param axis Flipping Axis.(X or Y)
 * @param cw Flipping Direction. EINA_TRUE is clock-wise
 * @return Resizable Flip effect context data
 *
 * @ingroup Transit
 */
EAPI void *
elm_transit_effect_resizable_flip_context_new(Elm_Fx_Flip_Axis axis, Eina_Bool cw)
{
   Elm_Fx_ResizableFlip *resizable_flip;

   resizable_flip = ELM_NEW(Elm_Fx_ResizableFlip);
   if (!resizable_flip) return NULL;

   resizable_flip->cw = cw;
   resizable_flip->axis = axis;

   return resizable_flip;
}

///////////////////////////////////////////////////////////////////////////////
//Wipe FX
///////////////////////////////////////////////////////////////////////////////
typedef struct _Elm_Fx_Wipe Elm_Fx_Wipe;
static void _elm_fx_wipe_hide(Evas_Map * map, Elm_Fx_Wipe_Dir dir,
                              float x, float y, float w, float h, float progress);
static void _elm_fx_wipe_show(Evas_Map *map, Elm_Fx_Wipe_Dir dir,
                              float x, float y, float w, float h, float progress);

struct _Elm_Fx_Wipe
{
   Elm_Fx_Wipe_Type type;
   Elm_Fx_Wipe_Dir dir;
};

/**
 * The Free function to Wipe Effect context data.
 *
 * @see elm_transit_effect_add()
 *
 * @param data The Wipe context data.
 * @param transt Transit object
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_effect_wipe_context_free(void *data, Elm_Transit *transit)
{
   Eina_List *elist;
   Evas_Object *obj;

   EINA_LIST_FOREACH(transit->objs, elist, obj)
     evas_object_map_enable_set(obj, EINA_FALSE);

   free(data);
}

static void
_elm_fx_wipe_hide(Evas_Map * map, Elm_Fx_Wipe_Dir dir, float x, float y, float w, float h, float progress)
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
         evas_map_point_coord_set(map, 2, w2, h2, 0);
         evas_map_point_coord_set(map, 3, x, h2, 0);
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

   evas_map_util_3d_perspective(map, x + (w / 2), y + (h / 2), 0, 10000);
}

static void
_elm_fx_wipe_show(Evas_Map *map, Elm_Fx_Wipe_Dir dir, float x, float y, float w, float h, float progress)
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

   evas_map_util_3d_perspective(map, x + (w / 2), y + (h / 2), 0, 10000);
}

/**
 * Operation function to the Wipe Effect
 *
 * @see elm_transit_effect_add()
 *
 * @param data The Wipe context data.
 * @param transt Transit object
 * @param progress The time progression, it is a number between 0 and 1
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_effect_wipe_op(void *data, Elm_Transit *transit, double progress)
{
   if (!data) return;
   Elm_Fx_Wipe *wipe = data;

   Evas_Map *map;

   Evas_Coord _x, _y, _w, _h;

   Eina_List *elist;
   Evas_Object *obj;

   map = evas_map_new(4);
   if (!map) return;

   evas_map_smooth_set(map, EINA_TRUE);

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

/**
 * Get a new context data of Wipe Flip Effect
 *
 * @see elm_transit_effect_add()
 *
 * @param type Wipe type. Hide or show
 * @param dir Wipe Direction
 * @return Wipe effect context data
 *
 * @ingroup Transit
 */
EAPI void *
elm_transit_effect_wipe_context_new(Elm_Fx_Wipe_Type type, Elm_Fx_Wipe_Dir dir)
{
   Elm_Fx_Wipe *wipe;

   wipe = ELM_NEW(Elm_Fx_Wipe);
   if (!wipe) return NULL;

   wipe->type = type;
   wipe->dir = dir;

   return wipe;
}

///////////////////////////////////////////////////////////////////////////////
//Color FX
///////////////////////////////////////////////////////////////////////////////
typedef struct _Elm_Fx_Color Elm_Fx_Color;

struct _Elm_Fx_Color
{
   struct _unsigned_color
     {
        unsigned int r, g, b, a;
     } from;
   struct _signed_color
     {
        int r, g, b, a;
     } to;
};

/**
 * The Free function to Color Effect context data.
 *
 * @see elm_transit_effect_add()
 *
 * @param data The Color context data.
 * @param transt Transit object
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_effect_color_context_free(void *data, Elm_Transit *transit __UNUSED__)
{
   free(data);
}

/**
 * Operation function to the Color Effect
 *
 * @see elm_transit_effect_add()
 *
 * @param data The Color context data.
 * @param transt Transit object
 * @param progress The time progression, it is a number between 0 and 1
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_effect_color_op(void *data, Elm_Transit *transit, double progress)
{
   if (!data) return;
   Elm_Fx_Color *color = data;
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

/**
 * Get a new context data of Color Effect
 *
 * @see elm_transit_effect_add()
 *
 * @param  from_r        RGB R when effect begins
 * @param  from_g        RGB G when effect begins
 * @param  from_b        RGB B when effect begins
 * @param  from_a        RGB A when effect begins
 * @param  to_r          RGB R when effect ends
 * @param  to_g          RGB G when effect ends
 * @param  to_b          RGB B when effect ends
 * @param  to_a          RGB A when effect ends
 * @return               Color effect context data
 *
 * @ingroup Transit
 */
EAPI void *
elm_transit_effect_color_context_new(unsigned int from_r, unsigned int from_g, unsigned int from_b, unsigned int from_a, unsigned int to_r, unsigned int to_g, unsigned int to_b, unsigned int to_a)
{
   Elm_Fx_Color *color;

   color = ELM_NEW(Elm_Fx_Color);
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

///////////////////////////////////////////////////////////////////////////////
//Fade FX
///////////////////////////////////////////////////////////////////////////////
typedef struct _Elm_Fx_Fade Elm_Fx_Fade;
typedef struct _Elm_Fx_Fade_Node Elm_Fx_Fade_Node;
static Eina_List *_fade_nodes_build(Elm_Transit *transit);

struct _Elm_Fx_Fade_Node
{
   Evas_Object *before;
   Evas_Object *after;
   struct _signed_color before_color, after_color;
   int before_alpha;
   int after_alpha;
   Eina_Bool inversed:1;
};

struct _Elm_Fx_Fade
{
   Eina_List *nodes;
};

/**
 * The Free function to Fade Effect context data.
 *
 * @see elm_transit_effect_add()
 *
 * @param data The Fade context data.
 * @param transt Transit object
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_effect_fade_context_free(void *data, Elm_Transit *transit __UNUSED__)
{
   Elm_Fx_Fade *fade = data;
   Elm_Fx_Fade_Node *fade_node;
   Eina_List *elist;

   EINA_LIST_FOREACH(fade->nodes, elist, fade_node)
     {
        evas_object_color_set(fade_node->before, fade_node->before_color.r,
                              fade_node->before_color.g,
                              fade_node->before_color.b,
                              fade_node->before_color.a);
        evas_object_color_set(fade_node->after, fade_node->after_color.r,
                              fade_node->after_color.g,
                              fade_node->after_color.b,
                              fade_node->after_color.a);
     }

   eina_list_free(fade->nodes);
   free(fade);
}

/**
 * Operation function to the Fade Effect
 *
 * This effect is applied to each pair of objects in the order they are listed
 * in the transit list of objects. The first object in the pair will be the
 * "before" object and the second will be the "after" object.
 *
 * @see elm_transit_effect_add()
 *
 * @param data The Fade context data.
 * @param transt Transit object
 * @param progress The time progression, it is a number between 0 and 1
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_effect_fade_op(void *data, Elm_Transit *transit __UNUSED__, double progress)
{
   if (!data) return;

   Elm_Fx_Fade *fade = data;
   Eina_List *elist;
   Elm_Fx_Fade_Node *fade_node;

   float _progress;

   if (!fade->nodes)
     fade->nodes = _fade_nodes_build(transit);

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
                                   fade_node->before_color.a + \
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
                                   fade_node->after_color.a + \
                                   fade_node->after_alpha * (1 - _progress));
          }
     }
}

static Eina_List *
_fade_nodes_build(Elm_Transit *transit)
{
   Elm_Fx_Fade_Node *fade;
   Eina_List *data_list = NULL;

   int i; int count;

   count = eina_list_count(transit->objs);
   for (i = 0; i < count-1; i+=2)
      {
         fade = ELM_NEW(Elm_Fx_Fade_Node);
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
      }

   return data_list;
}

/**
 * Get a new context data of Fade Effect
 *
 * @see elm_transit_effect_add()
 *
 * @return Fade effect context data
 *
 * @ingroup Transit
 */
EAPI void *
elm_transit_effect_fade_context_new(void)
{
   Elm_Fx_Fade *fade;
   fade = ELM_NEW(Elm_Fx_Fade);
   if (!fade) return NULL;
   return fade;
}

///////////////////////////////////////////////////////////////////////////////
//Blend FX
///////////////////////////////////////////////////////////////////////////////
typedef struct _Elm_Fx_Blend Elm_Fx_Blend;
typedef struct _Elm_Fx_Blend_Node Elm_Fx_Blend_Node;
static Eina_List * _blend_nodes_build(Elm_Transit *transit);

struct _Elm_Fx_Blend_Node
{
   Evas_Object *before;
   Evas_Object *after;
   struct _signed_color from, to;
};

struct _Elm_Fx_Blend
{
   Eina_List *nodes;
};

/**
 * The Free function to Blend Effect context data.
 *
 * @see elm_transit_effect_add()
 *
 * @param data The Blend context data.
 * @param transt Transit object
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_effect_blend_context_free(void *data, Elm_Transit *transit __UNUSED__)
{
   Elm_Fx_Blend *blend = data;
   Elm_Fx_Blend_Node *blend_node;
   Eina_List *elist;

   EINA_LIST_FOREACH(blend->nodes, elist, blend_node)
     {
        evas_object_color_set(blend_node->before,
                              blend_node->from.r, blend_node->from.g,
                              blend_node->from.b, blend_node->from.a);
        evas_object_color_set(blend_node->after, blend_node->to.r,
                              blend_node->to.g, blend_node->to.b,
                              blend_node->to.a);
     }
   eina_list_free(blend->nodes);
   free(data);
}

/**
 * Operation function to the Blend Effect
 *
 * This effect is applied to each pair of objects in the order they are listed
 * in the transit list of objects. The first object in the pair will be the
 * "before" object and the second will be the "after" object.
 *
 * @see elm_transit_effect_add()
 *
 * @param data The Blend context data.
 * @param transt Transit object
 * @param progress The time progression, it is a number between 0 and 1
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_effect_blend_op(void *data, Elm_Transit *transit, double progress)
{
   if (!data) return;
   Elm_Fx_Blend *blend = data;
   Elm_Fx_Blend_Node *blend_node;
   Eina_List *elist;

   if (!blend->nodes)
     blend->nodes = _blend_nodes_build(transit);

   EINA_LIST_FOREACH(blend->nodes, elist, blend_node)
     {
        evas_object_show(blend_node->after);
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

static Eina_List *
_blend_nodes_build(Elm_Transit *transit)
{
   Elm_Fx_Blend_Node *blend_node;
   Eina_List *data_list = NULL;

   int i, count;

   count = eina_list_count(transit->objs);
   for (i = 0; i < count-1; i+=2)
     {
         blend_node = ELM_NEW(Elm_Fx_Blend_Node);
         if (!blend_node)
           {
              eina_list_free(data_list);
              return NULL;
           }

         blend_node->before = eina_list_nth(transit->objs, i);
         blend_node->after = eina_list_nth(transit->objs, i+1);

         evas_object_color_get(blend_node->before, &blend_node->from.r,
                               &blend_node->from.g, &blend_node->from.b,
                               &blend_node->from.a);
         evas_object_color_get(blend_node->after, &blend_node->to.r,
                               &blend_node->to.g, &blend_node->to.b,
                               &blend_node->to.a);

         data_list = eina_list_append(data_list, blend_node);
     }
   return data_list;
}

/**
 * Get a new context data of Blend Effect
 *
 * @see elm_transit_effect_add()
 *
 * @return Blend effect context data
 *
 * @ingroup Transit
 */
EAPI void *
elm_transit_effect_blend_context_new(void)
{
   Elm_Fx_Blend *blend;

   blend = ELM_NEW(Elm_Fx_Blend);
   if (!blend) return NULL;

   return blend;
}

///////////////////////////////////////////////////////////////////////////////
//Rotation FX
///////////////////////////////////////////////////////////////////////////////
typedef struct _Elm_Fx_Rotation Elm_Fx_Rotation;

struct _Elm_Fx_Rotation
{
   Eina_Bool cw;
   float from, to;
};

/**
 * The Free function to Rotation Effect context data.
 *
 * @see elm_transit_effect_add()
 *
 * @param data The Rotation context data.
 * @param transt Transit object
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_effect_rotation_context_free(void *data, Elm_Transit *transit __UNUSED__)
{
   free(data);
}

/**
 * Operation function to the Rotation Effect
 *
 * @see elm_transit_effect_add()
 *
 * @param data The Rotation context data.
 * @param transt Transit object
 * @param progress The time progression, it is a number between 0 and 1
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_effect_rotation_op(void *data, Elm_Transit *transit, double progress)
{
   if (!data) return;
   Elm_Fx_Rotation *rotation = data;

   Evas_Map *map;

   Evas_Coord x, y, w, h;

   float degree;

   float half_w, half_h;

   Eina_List *elist;
   Evas_Object *obj;

   map = evas_map_new(4);
   if (!map) return;

   evas_map_smooth_set(map, EINA_TRUE);

   EINA_LIST_FOREACH(transit->objs, elist, obj)
     {
        evas_map_util_points_populate_from_object_full(map, obj, 0);
        degree = rotation->from + (float)(progress * rotation->to);

        if (!rotation->cw)
           degree *= -1;

        evas_object_geometry_get(obj, &x, &y, &w, &h);

        half_w = (float)w *0.5;

        half_h = (float)h *0.5;

        evas_map_util_3d_rotate(map, 0, 0, degree, x + half_w, y + half_h, 0);
        evas_map_util_3d_perspective(map, x + half_w, y + half_h, 0, 10000);
        evas_object_map_enable_set(obj, EINA_TRUE);
        evas_object_map_set(obj, map);
     }
   evas_map_free(map);
}

/**
 * Get a new context data of Rotation Effect
 *
 * @see elm_transit_effect_add()
 *
 * @param from_degree Degree when effect begins
 * @param to_degree Degree when effect is ends
 * @param cw Rotation direction. EINA_TRUE is clock wise
 * @return Rotation effect context data
 *
 * @ingroup Transit
 */
EAPI void *
elm_transit_effect_rotation_context_new(float from_degree, float to_degree, Eina_Bool cw)
{
   Elm_Fx_Rotation *rotation;

   rotation = ELM_NEW(Elm_Fx_Rotation);
   if (!rotation) return NULL;

   rotation->from = from_degree;
   rotation->to = to_degree - from_degree;
   rotation->cw = cw;

   return rotation;
}

///////////////////////////////////////////////////////////////////////////////
// ImageAnimation FX
///////////////////////////////////////////////////////////////////////////////
typedef struct _Elm_Fx_Image_Animation Elm_Fx_Image_Animation;

struct _Elm_Fx_Image_Animation
{
   Eina_List *images;
};

/**
 * The Free function to Imagem Animation Effect context data.
 *
 * @see elm_transit_effect_add()
 *
 * @param data The Imagem Animation context data.
 * @param transt Transit object
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_effect_image_animation_context_free(void *data, Elm_Transit *transit __UNUSED__)
{
   Elm_Fx_Image_Animation *image_animation = data;

   const char *image;
   Eina_List *elist, *elist_next;

   EINA_LIST_FOREACH_SAFE(image_animation->images, elist, elist_next, image)
     {
        image_animation->images = \
                       eina_list_remove_list(image_animation->images, elist);
        eina_stringshare_del(image);
     }

   free(data);
}

/**
 * Operation function to the Imagem Animation Effect
 *
 * This effect changes the image from an icon object in the @p transit
 *
 * @see elm_transit_effect_add()
 *
 * @param data The Imagem Animation context data.
 * @param transt Transit object
 * @param progress The time progression, it is a number between 0 and 1
 *
 * @ingroup Transit
 */
EAPI void
elm_transit_effect_image_animation_op(void *data, Elm_Transit *transit, double progress)
{
   if (!data) return;
   Eina_List *elist;
   Evas_Object *obj;
   const char *type;

   type = eina_stringshare_add("icon");

   Elm_Fx_Image_Animation *image_animation = (Elm_Fx_Image_Animation *) data;

   unsigned int count = 0;
   int len = eina_list_count(image_animation->images);

   if (!len)
      count = floor(progress * len);
   else
      count = floor(progress * (len - 1));

   EINA_LIST_FOREACH(transit->objs, elist, obj)
     {
        if (elm_widget_type_check(obj, type))
          elm_icon_file_set(obj,
                          eina_list_nth(image_animation->images, count), NULL);
     }

   eina_stringshare_del(type);
}

/**
 * Get a new context data of Imagem Animation Effect
 *
 * The @p images parameter is a list of const char* images. This list and
 * its contents will be deleted at the end of the animation.
 *
 * @see elm_transit_effect_add()
 *
 * @param images Array of image file path
 * @return ImageAnimation effect context data
 *
 * @ingroup Transit
 */
EAPI void *
elm_transit_effect_image_animation_context_new(Eina_List *images)
{
   Elm_Fx_Image_Animation *image_animation;

   image_animation = ELM_NEW(Elm_Fx_Image_Animation);

   if (!image_animation) return NULL;

   image_animation->images = images;

   return image_animation;
}
