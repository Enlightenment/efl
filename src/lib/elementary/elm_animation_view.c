#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_animation_view.h"

#define MY_CLASS ELM_ANIMATION_VIEW_CLASS

#define MY_CLASS_NAME "Elm_Animation_View"
#define MY_CLASS_NAME_LEGACY "elm_animation_view"

static const char SIG_FOCUSED[] = "focused";
static const char SIG_UNFOCUSED[] = "unfocused";
static const char SIG_PLAY_START[] = "play,start";
static const char SIG_PLAY_REPEAT[] = "play,repeat";
static const char SIG_PLAY_DONE[] = "play,done";
static const char SIG_PLAY_PAUSE[] = "play,pause";
static const char SIG_PLAY_RESUME[] = "play,resume";
static const char SIG_PLAY_STOP[] = "play,stop";
static const char SIG_PLAY_UPDATE[] = "play,update";

/* smart callbacks coming from Elm_Animation_View objects: */
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_FOCUSED, ""},
   {SIG_UNFOCUSED, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_PLAY_START, ""},
   {SIG_PLAY_REPEAT, ""},
   {SIG_PLAY_DONE, ""},
   {SIG_PLAY_PAUSE, ""},
   {SIG_PLAY_RESUME, ""},
   {SIG_PLAY_STOP, ""},
   {NULL, NULL}
};

static void
_sizing_eval(void *data)
{
   Elm_Animation_View_Data *pd = data;
   if (!pd->file) return;

   double hw,hh;
   efl_gfx_size_hint_weight_get(pd->obj, &hw, &hh);

   Eina_Size2D size = efl_canvas_vg_object_default_size_get(pd->vg);

   Eina_Size2D min = {-1, -1};
   if (hw == 0) min.w = size.w;
   if (hh == 0) min.h = size.h;

   efl_gfx_size_hint_min_set(pd->obj, min);
}

static void
_size_hint_event_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   _sizing_eval(data);
}

static void
_transit_go_facade(Elm_Animation_View_Data *pd)
{
   pd->repeat_times = 0;
   elm_transit_go(pd->transit);
   evas_object_smart_callback_call(pd->obj, SIG_PLAY_START, NULL);
}

static Eina_Bool
_visible_check(Eo *obj)
{
   if (!efl_gfx_entity_visible_get(obj)) return EINA_FALSE;

   //TODO: Check Smart parents visibilities?

   Eina_Size2D size = efl_gfx_entity_size_get(obj);
   if (size.w == 0 || size.h == 0) return EINA_FALSE;

   Evas_Coord output_w, output_h;
   evas_output_size_get(evas_object_evas_get(obj), &output_w, &output_h);

   Eina_Position2D pos = efl_gfx_entity_position_get(obj);

   //Outside viewport
   if ((pos.x + size.w < 0) || (pos.x > output_w) ||
       (pos.y + size.h < 0) || (pos.y > output_h))
     return EINA_FALSE;

   //Inside viewport
   return EINA_TRUE;
}

static void
_auto_play(Elm_Animation_View_Data *pd, Eina_Bool vis)
{
   if (!pd->auto_play || !pd->transit) return;

   //Resume Animation
   if (vis)
     {
        if (pd->state == ELM_ANIMATION_VIEW_STATE_PAUSE && pd->auto_play_pause)
          {
             elm_transit_paused_set(pd->transit, EINA_FALSE);
             if (pd->play_back)
               pd->state = ELM_ANIMATION_VIEW_STATE_PLAY_BACK;
             else
               pd->state = ELM_ANIMATION_VIEW_STATE_PLAY;
             pd->auto_play_pause = EINA_FALSE;
             evas_object_smart_callback_call(pd->obj, SIG_PLAY_RESUME, NULL);
          }
     }
   //Pause Animation
   else
     {
        if ((pd->state == ELM_ANIMATION_VIEW_STATE_PLAY) ||
            (pd->state == ELM_ANIMATION_VIEW_STATE_PLAY_BACK))
          {
             elm_transit_paused_set(pd->transit, EINA_TRUE);
             pd->state = ELM_ANIMATION_VIEW_STATE_PAUSE;
             pd->auto_play_pause = EINA_TRUE;
             evas_object_smart_callback_call(pd->obj, SIG_PLAY_PAUSE, NULL);
          }
     }
}

static void
_transit_del_cb(Elm_Transit_Effect *effect, Elm_Transit *transit)
{
   Elm_Animation_View_Data *pd = (Elm_Animation_View_Data *) effect;

   if ((pd->state == ELM_ANIMATION_VIEW_STATE_PLAY && pd->keyframe == 1) ||
       (pd->state == ELM_ANIMATION_VIEW_STATE_PLAY_BACK && pd->keyframe == 0))
     evas_object_smart_callback_call(pd->obj, SIG_PLAY_DONE, NULL);

   if (pd->transit != transit) return;

   Elm_Animation_View_State prev_state = pd->state;
   pd->state = ELM_ANIMATION_VIEW_STATE_STOP;
   pd->transit = NULL;
   pd->auto_play_pause = EINA_FALSE;

   if (prev_state != ELM_ANIMATION_VIEW_STATE_STOP)
     {
        evas_object_smart_callback_call(pd->obj, SIG_PLAY_STOP, NULL);
        pd->keyframe = 0;
     }
}

static void
_transit_cb(Elm_Transit_Effect *effect, Elm_Transit *transit, double progress)
{
   Elm_Animation_View_Data *pd = (Elm_Animation_View_Data *) effect;

   if (!pd->vg)
     {
        ERR("Vector Object is removed in wrong way!, Elm_Animation_View = %p", pd->obj);
        elm_transit_del(transit);
        return;
     }

   if (pd->play_back)
     {
        pd->state = ELM_ANIMATION_VIEW_STATE_PLAY_BACK;
        progress = 1 - progress;
     }
   else pd->state = ELM_ANIMATION_VIEW_STATE_PLAY;

   pd->keyframe = progress;
   evas_object_vg_animated_frame_set(pd->vg, (int) (pd->frame_cnt * progress));

   if (pd->auto_repeat)
     {
        int repeat_times = elm_transit_current_repeat_times_get(pd->transit);
        if (pd->repeat_times != repeat_times)
          {
             evas_object_smart_callback_call(pd->obj, SIG_PLAY_REPEAT, NULL);
             pd->repeat_times = repeat_times;
          }
     }

   evas_object_smart_callback_call(pd->obj, SIG_PLAY_UPDATE, NULL);
}

EOLIAN static void
_elm_animation_view_efl_canvas_group_group_add(Eo *obj, Elm_Animation_View_Data *priv)
{
   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   // Create vg to render vector animation
   Eo *vg = evas_object_vg_add(evas_object_evas_get(obj));
   elm_widget_resize_object_set(obj, vg);
   efl_event_callback_add(obj, EFL_GFX_ENTITY_EVENT_CHANGE_SIZE_HINTS, _size_hint_event_cb, priv);

   priv->vg = vg;
   priv->speed = 1;
   priv->frame_duration = 0;
}

EOLIAN static void
_elm_animation_view_efl_canvas_group_group_del(Eo *obj, Elm_Animation_View_Data *pd EINA_UNUSED)
{
   if (pd->file) eina_stringshare_del(pd->file);
   if (pd->transit)
     {
        pd->transit = NULL;   //Skip perform transit_del_cb()
        elm_transit_del(pd->transit);
     }
   pd->state = ELM_ANIMATION_VIEW_STATE_NOT_READY;

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_elm_animation_view_efl_object_destructor(Eo *obj,
                                          Elm_Animation_View_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Eo *
_elm_animation_view_efl_object_constructor(Eo *obj,
                                           Elm_Animation_View_Data *pd)
{
   pd->obj = obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);

   return obj;
}

static Eina_Bool
_ready_play(Elm_Animation_View_Data *pd)
{
   pd->auto_play_pause = EINA_FALSE;
   pd->state = ELM_ANIMATION_VIEW_STATE_STOP;

   if (pd->transit) elm_transit_del(pd->transit);

   pd->frame_cnt = (double) evas_object_vg_animated_frame_count_get(pd->vg);
   pd->frame_duration = evas_object_vg_animated_frame_duration_get(pd->vg, 0, 0);
   evas_object_vg_animated_frame_set(pd->vg, 0);

   if (pd->frame_duration > 0)
     {
        Elm_Transit *transit = elm_transit_add();
        elm_transit_object_add(transit, pd->vg);
        if (pd->auto_repeat) elm_transit_repeat_times_set(transit, -1);
        elm_transit_duration_set(transit, pd->frame_duration * (1/pd->speed));
        elm_transit_effect_add(transit, _transit_cb, pd, _transit_del_cb);
        elm_transit_progress_value_set(transit, pd->keyframe);
        elm_transit_objects_final_state_keep_set(transit, EINA_TRUE);
        pd->transit = transit;

        return EINA_TRUE;
     }
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_animation_view_efl_file_file_set(Eo *obj EINA_UNUSED, Elm_Animation_View_Data *pd, const char *file, const char *key)
{
   if (!evas_object_vg_file_set(pd->vg, file, key)) return EINA_FALSE;

   if (pd->file) eina_stringshare_del(pd->file);
   pd->file = eina_stringshare_add(file);
   pd->keyframe = 0;

   _sizing_eval(pd);

   if (!pd->file)
     {
        pd->state = ELM_ANIMATION_VIEW_STATE_NOT_READY;
        pd->frame_cnt = 0;
        pd->frame_duration = 0;
        if (pd->transit) elm_transit_del(pd->transit);
        return EINA_FALSE;
     }

   if (!_ready_play(pd)) return EINA_TRUE;

   if (pd->auto_play)
     {
        _transit_go_facade(pd);

        if (!_visible_check(obj))
          {
             elm_transit_paused_set(pd->transit, EINA_TRUE);
             pd->state = ELM_ANIMATION_VIEW_STATE_PAUSE;
             pd->auto_play_pause = EINA_TRUE;
             evas_object_smart_callback_call(pd->obj, SIG_PLAY_PAUSE, NULL);
          }
     }
   return EINA_TRUE;
}

EOLIAN static void
_elm_animation_view_efl_gfx_entity_position_set(Eo *obj,
                                                Elm_Animation_View_Data *pd,
                                                Eina_Position2D pos EINA_UNUSED)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, pos.x, pos.y))
     return;

   efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), pos);

   _auto_play(pd, _visible_check(obj));
}

EOLIAN static void
_elm_animation_view_efl_gfx_entity_size_set(Eo *obj,
                                            Elm_Animation_View_Data *pd,
                                            Eina_Size2D size)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, size.w, size.h))
     return;

   efl_gfx_entity_size_set(efl_super(obj, MY_CLASS), size);

   _sizing_eval(pd);

   _auto_play(pd, _visible_check(obj));
}

EOLIAN static void
_elm_animation_view_efl_gfx_entity_visible_set(Eo *obj,
                                               Elm_Animation_View_Data *pd,
                                               Eina_Bool vis)
{
  if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_VISIBLE, 0, vis))
     return;

   efl_gfx_entity_visible_set(efl_super(obj, MY_CLASS), vis);

   _auto_play(pd, _visible_check(obj));
}

EOLIAN static void
_elm_animation_view_auto_repeat_set(Eo *obj EINA_UNUSED, Elm_Animation_View_Data *pd, Eina_Bool auto_repeat)
{
   if (pd->auto_repeat == auto_repeat) return;
   pd->auto_repeat = auto_repeat;
   if (pd->transit)
     {
        if (auto_repeat) elm_transit_repeat_times_set(pd->transit, -1);
        else elm_transit_repeat_times_set(pd->transit, 0);
     }
}

EOLIAN static Eina_Bool
_elm_animation_view_auto_repeat_get(const Eo *obj EINA_UNUSED, Elm_Animation_View_Data *pd)
{
   return pd->auto_repeat;
}

EOLIAN static void
_elm_animation_view_auto_play_set(Eo *obj EINA_UNUSED, Elm_Animation_View_Data *pd,
                                  Eina_Bool auto_play)
{
   pd->auto_play = auto_play;
   if (!auto_play) pd->auto_play_pause = EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_animation_view_auto_play_get(const Eo *obj EINA_UNUSED, Elm_Animation_View_Data *pd)
{
   return pd->auto_play;
}

EOLIAN static Eina_Bool
_elm_animation_view_play(Eo *obj EINA_UNUSED, Elm_Animation_View_Data *pd)
{
   if (pd->state == ELM_ANIMATION_VIEW_STATE_PLAY) return EINA_FALSE;

   Eina_Bool rewind = EINA_FALSE;
   if (pd->state == ELM_ANIMATION_VIEW_STATE_PLAY_BACK) rewind = EINA_TRUE;

   pd->play_back = EINA_FALSE;
   pd->auto_play_pause = EINA_FALSE;

   if (!pd->file) return EINA_FALSE;
   if (!pd->transit && !_ready_play(pd)) return EINA_FALSE;

   if (pd->state == ELM_ANIMATION_VIEW_STATE_STOP)
     _transit_go_facade(pd);
   else if (rewind)
     elm_transit_progress_value_set(pd->transit, pd->keyframe);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_animation_view_stop(Eo *obj EINA_UNUSED, Elm_Animation_View_Data *pd)
{
   if (!pd->transit) return EINA_FALSE;

   if ((pd->state == ELM_ANIMATION_VIEW_STATE_NOT_READY) ||
       (pd->state == ELM_ANIMATION_VIEW_STATE_STOP))
     return EINA_FALSE;

   evas_object_vg_animated_frame_set(pd->vg, 0);
   pd->keyframe = 0;
   pd->state = ELM_ANIMATION_VIEW_STATE_STOP;
   evas_object_smart_callback_call(pd->obj, SIG_PLAY_STOP, NULL);
   elm_transit_del(pd->transit);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_animation_view_pause(Eo *obj EINA_UNUSED, Elm_Animation_View_Data *pd)
{
   if (!pd->transit) return EINA_FALSE;

   if ((pd->state == ELM_ANIMATION_VIEW_STATE_PLAY) ||
       (pd->state == ELM_ANIMATION_VIEW_STATE_PLAY_BACK))
     {
        elm_transit_paused_set(pd->transit, EINA_TRUE);
        pd->state = ELM_ANIMATION_VIEW_STATE_PAUSE;
        pd->auto_play_pause = EINA_FALSE;
        evas_object_smart_callback_call(pd->obj, SIG_PLAY_PAUSE, NULL);
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_animation_view_resume(Eo *obj EINA_UNUSED, Elm_Animation_View_Data *pd)
{
   if (!pd->transit) return EINA_FALSE;

   if (pd->state == ELM_ANIMATION_VIEW_STATE_PAUSE)
     {
        elm_transit_paused_set(pd->transit, EINA_FALSE);
        if (pd->play_back)
          pd->state = ELM_ANIMATION_VIEW_STATE_PLAY_BACK;
        else
          pd->state = ELM_ANIMATION_VIEW_STATE_PLAY;
        pd->auto_play_pause = EINA_FALSE;

        evas_object_smart_callback_call(pd->obj, SIG_PLAY_RESUME, NULL);

        return EINA_TRUE;
     }

   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_animation_view_play_back(Eo *obj EINA_UNUSED, Elm_Animation_View_Data *pd)
{
   if (pd->state == ELM_ANIMATION_VIEW_STATE_PLAY_BACK) return EINA_FALSE;

   Eina_Bool rewind = EINA_FALSE;
   if (pd->state == ELM_ANIMATION_VIEW_STATE_PLAY) rewind = EINA_TRUE;

   pd->play_back = EINA_TRUE;
   pd->auto_play_pause = EINA_FALSE;

   if (!pd->file) return EINA_FALSE;
   if (!pd->transit && !_ready_play(pd)) return EINA_FALSE;

   if (pd->state == ELM_ANIMATION_VIEW_STATE_STOP)
     _transit_go_facade(pd);
   else if (rewind)
     elm_transit_progress_value_set(pd->transit, 1 - pd->keyframe);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_animation_view_speed_set(Eo *obj EINA_UNUSED, Elm_Animation_View_Data *pd, double speed)
{
   if (speed <= 0) return EINA_FALSE;
   pd->speed = speed;

   if (pd->transit)
     elm_transit_duration_set(pd->transit, pd->frame_duration * (1/pd->speed));

   return EINA_TRUE;
}

EOLIAN static void
_elm_animation_view_keyframe_set(Eo *obj EINA_UNUSED, Elm_Animation_View_Data *pd, double keyframe)
{
   if (keyframe < 0) keyframe = 0;
   else if (keyframe > 1) keyframe = 1;
   if (pd->keyframe == keyframe) return;

   pd->keyframe = keyframe;

   if (pd->frame_cnt > 0)
     evas_object_vg_animated_frame_set(pd->vg, (int) (pd->frame_cnt * keyframe));

   if (pd->transit)
     {
        if (pd->play_back)
          elm_transit_progress_value_set(pd->transit, 1 - keyframe);
        else
          elm_transit_progress_value_set(pd->transit, keyframe);
     }
}

EOLIAN static double
_elm_animation_view_keyframe_get(const Eo *obj EINA_UNUSED, Elm_Animation_View_Data *pd)
{
   return pd->keyframe;
}

EOLIAN static double
_elm_animation_view_speed_get(const Eo *obj EINA_UNUSED, Elm_Animation_View_Data *pd)
{
   return pd->speed;
}

EOLIAN static double
_elm_animation_view_duration_time_get(const Eo *obj EINA_UNUSED, Elm_Animation_View_Data *pd)
{
   return pd->frame_duration;
}

EOLIAN static Eina_Size2D
_elm_animation_view_default_size_get(const Eo *obj EINA_UNUSED,
                                     Elm_Animation_View_Data *pd EINA_UNUSED)
{
   return efl_canvas_vg_object_default_size_get(pd->vg);
}

EAPI Elm_Animation_View*
elm_animation_view_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

EAPI Eina_Bool
elm_animation_view_file_set(Elm_Animation_View *obj, const char *file, const char *key)
{
   return efl_file_set(obj, file, key);
}

EAPI Elm_Animation_View_State
elm_animation_view_state_get(const Elm_Animation_View *obj)
{
   ELM_ANIMATION_VIEW_DATA_GET(obj, pd);
   if (!pd) return ELM_ANIMATION_VIEW_STATE_NOT_READY;

   return pd->state;
}

EAPI Eina_Bool
elm_animation_view_is_playing_back(const Elm_Animation_View *obj)
{
   ELM_ANIMATION_VIEW_DATA_GET(obj, pd);
   if (!pd) return EINA_FALSE;

   return pd->play_back;
}

/* Internal EO APIs and hidden overrides */

#define ELM_ANIMATION_VIEW_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_animation_view)

#include "elm_animation_view.eo.c"
