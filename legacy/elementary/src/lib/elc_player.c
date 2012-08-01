#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_player.h"

#ifdef HAVE_EMOTION
# include <Emotion.h>
#endif

EAPI const char ELM_PLAYER_SMART_NAME[] = "elm_player";

static const char SIG_FORWARD_CLICKED[] = "forward,clicked";
static const char SIG_INFO_CLICKED[] = "info,clicked";
static const char SIG_NEXT_CLICKED[] = "next,clicked";
static const char SIG_PAUSE_CLICKED[] = "pause,clicked";
static const char SIG_PLAY_CLICKED[] = "play,clicked";
static const char SIG_PREV_CLICKED[] = "prev,clicked";
static const char SIG_REWIND_CLICKED[] = "rewind,clicked";
static const char SIG_STOP_CLICKED[] = "stop,clicked";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   { SIG_FORWARD_CLICKED, "" },
   { SIG_INFO_CLICKED, "" },
   { SIG_NEXT_CLICKED, "" },
   { SIG_PAUSE_CLICKED, "" },
   { SIG_PLAY_CLICKED, "" },
   { SIG_PREV_CLICKED, "" },
   { SIG_REWIND_CLICKED, "" },
   { SIG_STOP_CLICKED, "" },
   { NULL, NULL }
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_PLAYER_SMART_NAME, _elm_player, Elm_Player_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, _smart_callbacks);

#ifdef HAVE_EMOTION
static Eina_Bool
_elm_player_smart_event(Evas_Object *obj,
                        Evas_Object *src __UNUSED__,
                        Evas_Callback_Type type,
                        void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;

   ELM_PLAYER_DATA_GET(obj, sd);

   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (!sd->video) return EINA_FALSE;

   if ((!strcmp(ev->keyname, "Left")) ||
       ((!strcmp(ev->keyname, "KP_Left")) && (!ev->string)))
     {
        double current, last;

        current = elm_video_play_position_get(sd->video);
        last = elm_video_play_length_get(sd->video);

        if (current < last)
          {
             current -= last / 100;
             elm_video_play_position_set(sd->video, current);
          }

        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   if ((!strcmp(ev->keyname, "Right")) ||
       ((!strcmp(ev->keyname, "KP_Right")) && (!ev->string)))
     {
        double current, last;

        current = elm_video_play_position_get(sd->video);
        last = elm_video_play_length_get(sd->video);

        if (current > 0)
          {
             current += last / 100;
             if (current < 0) current = 0;
             elm_video_play_position_set(sd->video, current);
          }

        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   if (!strcmp(ev->keyname, "space"))
     {
        if (elm_video_is_playing_get(sd->video))
          elm_video_pause(sd->video);
        else
          elm_video_play(sd->video);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   fprintf(stderr, "keyname: '%s' not handle\n", ev->keyname);

   return EINA_FALSE;
}

static Eina_Bool
_elm_player_smart_theme(Evas_Object *obj)
{
   ELM_PLAYER_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_player_parent_sc)->theme(obj)) return EINA_FALSE;

#define UPDATE_THEME(Target, Name)                                    \
  if (Target)                                                         \
    {                                                                 \
       elm_object_style_set(Target, elm_widget_style_get(obj));       \
       if (!elm_layout_content_set(obj, Name, Target))                \
         evas_object_hide(Target);                                    \
       elm_object_disabled_set(Target, elm_widget_disabled_get(obj)); \
    }

   UPDATE_THEME(sd->forward, "media_player/forward");
   UPDATE_THEME(sd->info, "media_player/info");
   UPDATE_THEME(sd->next, "media_player/next");
   UPDATE_THEME(sd->pause, "media_player/pause");
   UPDATE_THEME(sd->play, "media_player/play");
   UPDATE_THEME(sd->prev, "media_player/prev");
   UPDATE_THEME(sd->rewind, "media_player/rewind");
   UPDATE_THEME(sd->next, "media_player/next");
   UPDATE_THEME(sd->slider, "media_player/slider");

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static void
_elm_player_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord w, h;

   ELM_PLAYER_DATA_GET(obj, sd);

   edje_object_size_min_get(ELM_WIDGET_DATA(sd)->resize_obj, &w, &h);
   edje_object_size_min_restricted_calc
     (ELM_WIDGET_DATA(sd)->resize_obj, &w, &h, w, h);
   evas_object_size_hint_min_set(obj, w, h);
}

static void
_update_slider(void *data,
               Evas_Object *obj __UNUSED__,
               void *event_info __UNUSED__)
{
   double pos, length;
   Eina_Bool seekable;

   ELM_PLAYER_DATA_GET(data, sd);

   seekable = elm_video_is_seekable_get(sd->video);
   length = elm_video_play_length_get(sd->video);
   pos = elm_video_play_position_get(sd->video);

   elm_object_disabled_set(sd->slider, !seekable);
   elm_slider_min_max_set(sd->slider, 0, length);
   elm_slider_value_set(sd->slider, pos);
}

static void
_update_position(void *data,
                 Evas_Object *obj __UNUSED__,
                 void *event_info __UNUSED__)
{
   ELM_PLAYER_DATA_GET(data, sd);

   elm_video_play_position_set(sd->video, elm_slider_value_get(sd->slider));
}

static void
_forward(void *data,
         Evas_Object *obj __UNUSED__,
         void *event_info __UNUSED__)
{
   double pos, length;

   ELM_PLAYER_DATA_GET(data, sd);

   pos = elm_video_play_position_get(sd->video);
   length = elm_video_play_length_get(sd->video);

   pos += length * 0.3;
   elm_video_play_position_set(sd->video, pos);

   elm_layout_signal_emit(data, "elm,button,forward", "elm");
   evas_object_smart_callback_call(data, SIG_FORWARD_CLICKED, NULL);
}

static void
_info(void *data,
      Evas_Object *obj __UNUSED__,
      void *event_info __UNUSED__)
{
   elm_layout_signal_emit(data, "elm,button,info", "elm");
   evas_object_smart_callback_call(data, SIG_INFO_CLICKED, NULL);
}

static void
_next(void *data,
      Evas_Object *obj __UNUSED__,
      void *event_info __UNUSED__)
{
   double pos, length;

   ELM_PLAYER_DATA_GET(data, sd);

   pos = elm_video_play_position_get(sd->video);
   length = elm_video_play_length_get(sd->video);

   pos += length * 0.1;
   elm_video_play_position_set(sd->video, pos);

   elm_layout_signal_emit(data, "elm,button,next", "elm");
   evas_object_smart_callback_call(data, SIG_NEXT_CLICKED, NULL);
}

static void
_pause(void *data,
       Evas_Object *obj __UNUSED__,
       void *event_info __UNUSED__)
{
   ELM_PLAYER_DATA_GET(data, sd);

   elm_layout_signal_emit(data, "elm,player,pause", "elm");
   elm_video_pause(sd->video);

   elm_layout_signal_emit(data, "elm,button,pause", "elm");
   evas_object_smart_callback_call(data, SIG_PAUSE_CLICKED, NULL);
}

static void
_play(void *data,
      Evas_Object *obj __UNUSED__,
      void *event_info __UNUSED__)
{
   ELM_PLAYER_DATA_GET(data, sd);

   elm_video_play(sd->video);
   elm_layout_signal_emit(data, "elm,button,play", "elm");
   evas_object_smart_callback_call(data, SIG_PLAY_CLICKED, NULL);
}

static void
_prev(void *data,
      Evas_Object *obj __UNUSED__,
      void *event_info __UNUSED__)
{
   double pos, length;

   ELM_PLAYER_DATA_GET(data, sd);

   pos = elm_video_play_position_get(sd->video);
   length = elm_video_play_length_get(sd->video);

   pos -= length * 0.1;
   elm_video_play_position_set(sd->video, pos);
   evas_object_smart_callback_call(data, SIG_PREV_CLICKED, NULL);
   elm_layout_signal_emit(data, "elm,button,prev", "elm");
}

static void
_rewind(void *data,
        Evas_Object *obj __UNUSED__,
        void *event_info __UNUSED__)
{
   ELM_PLAYER_DATA_GET(data, sd);

   elm_video_play_position_set(sd->video, 0);
   elm_layout_signal_emit(data, "elm,button,rewind", "elm");
   evas_object_smart_callback_call(data, SIG_REWIND_CLICKED, NULL);
}

static void
_stop(void *data,
      Evas_Object *obj __UNUSED__,
      void *event_info __UNUSED__)
{
   elm_layout_signal_emit(data, "elm,button,stop", "elm");
   evas_object_smart_callback_call(data, SIG_STOP_CLICKED, NULL);
}

static void
_play_started(void *data,
              Evas_Object *obj __UNUSED__,
              void *event_info __UNUSED__)
{
   elm_layout_signal_emit(data, "elm,player,play", "elm");
}

static void
_play_finished(void *data,
               Evas_Object *obj __UNUSED__,
               void *event_info __UNUSED__)
{
   elm_layout_signal_emit(data, "elm,player,pause", "elm");
}

static void
_on_video_del(Elm_Player_Smart_Data *sd)
{
   elm_object_disabled_set(sd->slider, EINA_TRUE);
   elm_object_disabled_set(sd->forward, EINA_TRUE);
   elm_object_disabled_set(sd->info, EINA_TRUE);
   elm_object_disabled_set(sd->next, EINA_TRUE);
   elm_object_disabled_set(sd->pause, EINA_TRUE);
   elm_object_disabled_set(sd->play, EINA_TRUE);
   elm_object_disabled_set(sd->prev, EINA_TRUE);
   elm_object_disabled_set(sd->rewind, EINA_TRUE);
   elm_object_disabled_set(sd->next, EINA_TRUE);

   sd->video = NULL;
   sd->emotion = NULL;
}

static void
_video_del(void *data,
           Evas *e __UNUSED__,
           Evas_Object *obj __UNUSED__,
           void *event_info __UNUSED__)
{
   _on_video_del(data);
}

static Evas_Object *
_player_button_add(Evas_Object *obj,
                   const char *name,
                   Evas_Smart_Cb func)
{
   Evas_Object *ic;
   Evas_Object *bt;

   ic = elm_icon_add(obj);
   elm_icon_standard_set(ic, name);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   bt = elm_button_add(obj);
   elm_widget_mirrored_automatic_set(bt, EINA_FALSE);
   elm_object_part_content_set(bt, "icon", ic);
   evas_object_size_hint_align_set(bt, 0.0, 0.0);
   elm_object_style_set(bt, "anchor");
   evas_object_smart_callback_add(bt, "clicked", func, obj);

   if (!elm_layout_content_set(obj, name, bt))
     {
        elm_widget_sub_object_add(obj, bt);
        evas_object_hide(bt);
     }

   return bt;
}

static char *
_double_to_time(double value)
{
   char buf[256];
   int ph, pm, ps, pf;

   ph = value / 3600;
   pm = value / 60 - (ph * 60);
   ps = value - (pm * 60);
   pf = value * 100 - (ps * 100) - (pm * 60 * 100) - (ph * 60 * 60 * 100);

   if (ph)
     snprintf(buf, sizeof(buf), "%i:%02i:%02i.%02i",
              ph, pm, ps, pf);
   else if (pm)
     snprintf(buf, sizeof(buf), "%02i:%02i.%02i",
              pm, ps, pf);
   else
     snprintf(buf, sizeof(buf), "%02i.%02i",
              ps, pf);

   return (char *)eina_stringshare_add(buf);
}

static void
_str_free(char *data)
{
   eina_stringshare_del(data);
}

/* a video object is never parented by a player one, just tracked.
 * treating this special case here and delegating other objects to own
 * layout */
static Eina_Bool
_elm_player_smart_content_set(Evas_Object *obj,
                              const char *part,
                              Evas_Object *content)
{
   double pos, length;
   Eina_Bool seekable;

   if (part && strcmp(part, "video"))
     return ELM_CONTAINER_CLASS(_elm_player_parent_sc)->content_set
              (obj, part, content);

   ELM_PLAYER_DATA_GET(obj, sd);

   if (!_elm_video_check(content)) return EINA_FALSE;
   if (sd->video == content) return EINA_TRUE;

   if (sd->video) evas_object_del(sd->video);

   sd->video = content;

   if (!content) return EINA_TRUE;

   elm_object_disabled_set(sd->slider, EINA_FALSE);
   elm_object_disabled_set(sd->forward, EINA_FALSE);
   elm_object_disabled_set(sd->info, EINA_FALSE);
   elm_object_disabled_set(sd->next, EINA_FALSE);
   elm_object_disabled_set(sd->pause, EINA_FALSE);
   elm_object_disabled_set(sd->play, EINA_FALSE);
   elm_object_disabled_set(sd->prev, EINA_FALSE);
   elm_object_disabled_set(sd->rewind, EINA_FALSE);
   elm_object_disabled_set(sd->next, EINA_FALSE);

   sd->emotion = elm_video_emotion_get(sd->video);
   emotion_object_priority_set(sd->emotion, EINA_TRUE);
   evas_object_event_callback_add
     (sd->video, EVAS_CALLBACK_DEL, _video_del, sd);

   seekable = elm_video_is_seekable_get(sd->video);
   length = elm_video_play_length_get(sd->video);
   pos = elm_video_play_position_get(sd->video);

   elm_object_disabled_set(sd->slider, !seekable);
   elm_slider_min_max_set(sd->slider, 0, length);
   elm_slider_value_set(sd->slider, pos);

   if (elm_video_is_playing_get(sd->video))
     elm_layout_signal_emit(obj, "elm,player,play", "elm");
   else elm_layout_signal_emit(obj, "elm,player,pause", "elm");

   evas_object_smart_callback_add(sd->emotion, "frame_decode",
                                  _update_slider, obj);
   evas_object_smart_callback_add(sd->emotion, "frame_resize",
                                  _update_slider, obj);
   evas_object_smart_callback_add(sd->emotion, "length_change",
                                  _update_slider, obj);
   evas_object_smart_callback_add(sd->emotion, "position_update",
                                  _update_slider, obj);
   evas_object_smart_callback_add(sd->emotion, "playback_started",
                                  _play_started, obj);
   evas_object_smart_callback_add(sd->emotion, "playback_finished",
                                  _play_finished, obj);

   /* FIXME: track info from video */

   return EINA_TRUE;
}

static void
_elm_player_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Player_Smart_Data);

   ELM_WIDGET_CLASS(_elm_player_parent_sc)->base.add(obj);

   elm_layout_theme_set(obj, "player", "base", elm_widget_style_get(obj));

   priv->forward = _player_button_add(obj, "media_player/forward", _forward);
   priv->info = _player_button_add(obj, "media_player/info", _info);
   priv->next = _player_button_add(obj, "media_player/next", _next);
   priv->pause = _player_button_add(obj, "media_player/pause", _pause);
   priv->play = _player_button_add(obj, "media_player/play", _play);
   priv->prev = _player_button_add(obj, "media_player/prev", _prev);
   priv->rewind = _player_button_add(obj, "media_player/rewind", _rewind);
   priv->stop = _player_button_add(obj, "media_player/stop", _stop);

   priv->slider = elm_slider_add(obj);
   elm_slider_indicator_format_function_set
     (priv->slider, _double_to_time, _str_free);
   elm_slider_units_format_function_set
     (priv->slider, _double_to_time, _str_free);
   elm_slider_min_max_set(priv->slider, 0, 0);
   elm_slider_value_set(priv->slider, 0);
   elm_object_disabled_set(priv->slider, EINA_TRUE);
   evas_object_size_hint_align_set(priv->slider, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set
     (priv->slider, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   elm_layout_content_set(obj, "media_player/slider", priv->slider);
   evas_object_smart_callback_add
     (priv->slider, "changed", _update_position, obj);

   elm_layout_sizing_eval(obj);
   elm_widget_can_focus_set(obj, EINA_TRUE);
}

#endif

static void
_elm_player_smart_set_user(Elm_Player_Smart_Class *sc)
{
#ifdef HAVE_EMOTION
   ELM_WIDGET_CLASS(sc)->base.add = _elm_player_smart_add;

   ELM_WIDGET_CLASS(sc)->event = _elm_player_smart_event;
   ELM_WIDGET_CLASS(sc)->theme = _elm_player_smart_theme;

   ELM_CONTAINER_CLASS(sc)->content_set = _elm_player_smart_content_set;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_player_smart_sizing_eval;
#else
   (void) sc;
#endif
}

EAPI const Elm_Player_Smart_Class *
elm_player_smart_class_get(void)
{
   static Elm_Player_Smart_Class _sc =
     ELM_PLAYER_SMART_CLASS_INIT_NAME_VERSION(ELM_PLAYER_SMART_NAME);
   static const Elm_Player_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_player_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_player_add(Evas_Object *parent)
{
#ifdef HAVE_EMOTION
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_player_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
#else
   (void) parent;
   (void) _elm_player_smart_class_new;
   return NULL;
#endif
}
