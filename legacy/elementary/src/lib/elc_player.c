#include <Elementary.h>
#include "elm_priv.h"

#ifdef HAVE_EMOTION
# include <Emotion.h>
#endif

typedef struct _Widget_Data Widget_Data;
struct _Widget_Data
{
   Evas_Object *layout;
   Evas_Object *video;
   Evas_Object *emotion;

   Evas_Object *forward;
   Evas_Object *info;
   Evas_Object *next;
   Evas_Object *pause;
   Evas_Object *play;
   Evas_Object *prev;
   Evas_Object *rewind;
   Evas_Object *stop;

   Evas_Object *slider;
};

#ifdef HAVE_EMOTION
static const char *widtype = NULL;

static const char SIG_FORWARD_CLICKED[] = "forward,clicked";
static const char SIG_INFO_CLICKED[] = "info,clicked";
static const char SIG_NEXT_CLICKED[] = "next,clicked";
static const char SIG_PAUSE_CLICKED[] = "pause,clicked";
static const char SIG_PLAY_CLICKED[] = "play,clicked";
static const char SIG_PREV_CLICKED[] = "prev,clicked";
static const char SIG_REWIND_CLICKED[] = "rewind,clicked";
static const char SIG_STOP_CLICKED[] = "stop,clicked";

static const Evas_Smart_Cb_Description _signals[] = {
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

static void _del_hook(Evas_Object *obj);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _on_focus_hook(void *data, Evas_Object *obj);
static Eina_Bool _event_hook(Evas_Object *obj, Evas_Object *src,
                             Evas_Callback_Type type, void *event_info);

static Eina_Bool
_event_hook(Evas_Object *obj, Evas_Object *src __UNUSED__, Evas_Callback_Type type, void *event_info)
{
   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   Evas_Event_Key_Down *ev = event_info;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if (!wd->video) return EINA_FALSE;
   if ((!strcmp(ev->keyname, "Left")) || (!strcmp(ev->keyname, "KP_Left")))
     {
        double current, last;

        current = elm_video_play_position_get(wd->video);
        last = elm_video_play_length_get(wd->video);

        if (current < last)
          {
             current += last / 100;
             elm_video_play_position_set(wd->video, current);
          }

        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   if ((!strcmp(ev->keyname, "Right")) || (!strcmp(ev->keyname, "KP_Right")))
     {
        double current, last;

        current = elm_video_play_position_get(wd->video);
        last = elm_video_play_length_get(wd->video);

        if (current > 0)
          {
             current -= last / 100;
             if (current < 0) current = 0;
             elm_video_play_position_set(wd->video, current);
          }

        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   if (!strcmp(ev->keyname, "space"))
     {
        if (elm_video_is_playing(wd->video))
          elm_video_pause(wd->video);
        else
          elm_video_play(wd->video);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   fprintf(stderr, "keyname: '%s' not handle\n", ev->keyname);
   return EINA_FALSE;
}

static void
_on_focus_hook(void *data __UNUSED__, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_focus_get(obj))
     {
        edje_object_signal_emit(wd->layout, "elm,action,focus", "elm");
        evas_object_focus_set(wd->layout, EINA_TRUE);
     }
   else
     {
        edje_object_signal_emit(wd->layout, "elm,action,unfocus", "elm");
        evas_object_focus_set(wd->layout, EINA_FALSE);
     }
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_mirrored_set(wd->layout, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _elm_theme_object_set(obj, wd->layout, "video", "base", elm_widget_style_get(obj));
   edje_object_scale_set(wd->layout, elm_widget_scale_get(obj) *
                         _elm_config->scale);

#define UPDATE_THEME(Obj, Target, Layout, Name)                         \
   if (Target)                                                          \
     {                                                                  \
        elm_object_style_set(Target, elm_widget_style_get(Obj));	\
        if (!edje_object_part_swallow(Layout, Name, Target))            \
          evas_object_hide(Target);                                     \
        elm_object_disabled_set(Target, elm_widget_disabled_get(Obj));  \
     }

   UPDATE_THEME(obj, wd->forward, wd->layout, "media_player/forward");
   UPDATE_THEME(obj, wd->info, wd->layout, "media_player/info");
   UPDATE_THEME(obj, wd->next, wd->layout, "media_player/next");
   UPDATE_THEME(obj, wd->pause, wd->layout, "media_player/pause");
   UPDATE_THEME(obj, wd->play, wd->layout, "media_player/play");
   UPDATE_THEME(obj, wd->prev, wd->layout, "media_player/prev");
   UPDATE_THEME(obj, wd->rewind, wd->layout, "media_player/rewind");
   UPDATE_THEME(obj, wd->next, wd->layout, "media_player/next");
   UPDATE_THEME(obj, wd->slider, wd->layout, "media_player/slider");

   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord w, h;

   if (!wd) return;
   edje_object_size_min_get(wd->layout, &w, &h);
   edje_object_size_min_restricted_calc(wd->layout, &w, &h, w, h);
   evas_object_size_hint_min_set(obj, w, h);
}

static void
_update_slider(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *player = data;
   Widget_Data *wd = elm_widget_data_get(player);
   double pos, length;
   Eina_Bool seekable;

   if (!wd) return ;
   seekable = elm_video_is_seekable(wd->video);
   length = elm_video_play_length_get(wd->video);
   pos = elm_video_play_position_get(wd->video);

   elm_object_disabled_set(wd->slider, !seekable);
   elm_slider_min_max_set(wd->slider, 0, length);
   elm_slider_value_set(wd->slider, pos);
}

static void
_update_position(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *player = data;
   Widget_Data *wd = elm_widget_data_get(player);

   if (!wd) return ;
   elm_video_play_position_set(wd->video, elm_slider_value_get(wd->slider));
}

static void
_forward(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *player = data;
   Widget_Data *wd = elm_widget_data_get(player);
   double pos, length;

   if (!wd) return ;

   pos = elm_video_play_position_get(wd->video);
   length = elm_video_play_length_get(wd->video);

   pos += length * 0.3;
   elm_video_play_position_set(wd->video, pos);

   evas_object_smart_callback_call(player, SIG_FORWARD_CLICKED, NULL);
   edje_object_signal_emit(wd->layout, "elm,button,forward", "elm");
}

static void
_info(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *player = data;
   Widget_Data *wd = elm_widget_data_get(player);

   if (!wd) return ;

   evas_object_smart_callback_call(player, SIG_INFO_CLICKED, NULL);
   edje_object_signal_emit(wd->layout, "elm,button,info", "elm");
}

static void
_next(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *player = data;
   Widget_Data *wd = elm_widget_data_get(player);
   double pos, length;

   if (!wd) return ;

   pos = elm_video_play_position_get(wd->video);
   length = elm_video_play_length_get(wd->video);

   pos += length * 0.1;
   elm_video_play_position_set(wd->video, pos);

   evas_object_smart_callback_call(player, SIG_NEXT_CLICKED, NULL);
   edje_object_signal_emit(wd->layout, "elm,button,next", "elm");
}

static void
_pause(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *player = data;
   Widget_Data *wd = elm_widget_data_get(player);

   if (!wd) return ;

   edje_object_signal_emit(wd->layout, "elm,player,pause", "elm");
   elm_video_pause(wd->video);

   evas_object_smart_callback_call(player, SIG_PAUSE_CLICKED, NULL);
   edje_object_signal_emit(wd->layout, "elm,button,pause", "elm");
}

static void
_play(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *player = data;
   Widget_Data *wd = elm_widget_data_get(player);

   if (!wd) return ;
   elm_video_play(wd->video);
   evas_object_smart_callback_call(player, SIG_PLAY_CLICKED, NULL);
   edje_object_signal_emit(wd->layout, "elm,button,play", "elm");
}

static void
_prev(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *player = data;
   Widget_Data *wd = elm_widget_data_get(player);
   double pos, length;

   if (!wd) return ;

   pos = elm_video_play_position_get(wd->video);
   length = elm_video_play_length_get(wd->video);

   pos -= length * 0.1;
   elm_video_play_position_set(wd->video, pos);
   evas_object_smart_callback_call(player, SIG_PREV_CLICKED, NULL);
   edje_object_signal_emit(wd->layout, "elm,button,prev", "elm");
}

static void
_rewind(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *player = data;
   Widget_Data *wd = elm_widget_data_get(player);

   if (!wd) return ;
   elm_video_play_position_set(wd->video, 0);
   evas_object_smart_callback_call(player, SIG_REWIND_CLICKED, NULL);
   edje_object_signal_emit(wd->layout, "elm,button,rewind", "elm");
}

static void
_stop(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *player = data;
   Widget_Data *wd = elm_widget_data_get(player);

   if (!wd) return ;

   evas_object_smart_callback_call(player, SIG_STOP_CLICKED, NULL);
   edje_object_signal_emit(wd->layout, "elm,button,stop", "elm");
}

static void
_play_started(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *player = data;
   Widget_Data *wd = elm_widget_data_get(player);

   if (!wd) return ;

   edje_object_signal_emit(wd->layout, "elm,player,play", "elm");
}

static void
_play_finished(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *player = data;
   Widget_Data *wd = elm_widget_data_get(player);

   if (!wd) return ;

   edje_object_signal_emit(wd->layout, "elm,player,pause", "elm");
}

static void
_cleanup_callback(Widget_Data *wd)
{
   if (!wd || !wd->emotion) return;

   evas_object_smart_callback_del(wd->emotion, "frame_decode",
                                  _update_slider);
   evas_object_smart_callback_del(wd->emotion, "frame_resize",
                                  _update_slider);
   evas_object_smart_callback_del(wd->emotion, "length_change",
                                  _update_slider);
   evas_object_smart_callback_del(wd->emotion, "position_update",
                                  _update_slider);
   evas_object_smart_callback_del(wd->emotion, "playback_started",
                                  _play_started);
   evas_object_smart_callback_del(wd->emotion, "playback_finished",
                                  _play_finished);
   elm_object_disabled_set(wd->slider, EINA_TRUE);
   elm_object_disabled_set(wd->forward, EINA_TRUE);
   elm_object_disabled_set(wd->info, EINA_TRUE);
   elm_object_disabled_set(wd->next, EINA_TRUE);
   elm_object_disabled_set(wd->pause, EINA_TRUE);
   elm_object_disabled_set(wd->play, EINA_TRUE);
   elm_object_disabled_set(wd->prev, EINA_TRUE);
   elm_object_disabled_set(wd->rewind, EINA_TRUE);
   elm_object_disabled_set(wd->next, EINA_TRUE);
   wd->video = NULL;
   wd->emotion = NULL;
}

static void
_track_video(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   _cleanup_callback(wd);
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   evas_object_smart_callback_del(wd->forward, "clicked", _forward);
   evas_object_smart_callback_del(wd->info, "clicked", _info);
   evas_object_smart_callback_del(wd->next, "clicked", _next);
   evas_object_smart_callback_del(wd->pause, "clicked", _pause);
   evas_object_smart_callback_del(wd->play, "clicked", _play);
   evas_object_smart_callback_del(wd->prev,  "clicked", _prev);
   evas_object_smart_callback_del(wd->rewind, "clicked", _rewind);
   evas_object_smart_callback_del(wd->next, "clicked", _next);
   _cleanup_callback(wd);
   free(wd);
}

static Evas_Object *
_player_button_add(Evas_Object *parent, Evas_Object *obj, Evas_Object *layout, const char *name, Evas_Smart_Cb func)
{
   Evas_Object *ic;
   Evas_Object *bt;

   ic = elm_icon_add(parent);
   elm_icon_standard_set(ic, name);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   bt = elm_button_add(parent);
   elm_widget_mirrored_automatic_set(bt, EINA_FALSE);
   elm_object_content_set(bt, ic);
   evas_object_size_hint_align_set(bt, 0.0, 0.0);
   elm_object_style_set(bt, "anchor");
   evas_object_smart_callback_add(bt, "clicked", func, obj);
   elm_widget_sub_object_add(obj, bt);

   if (!edje_object_part_swallow(layout, name, bt))
     evas_object_hide(bt);
   return bt;
}

static const char *
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

   return eina_stringshare_add(buf);
}
#endif

static void
_content_set_hook(Evas_Object *obj, const char *part __UNUSED__, Evas_Object *content)
{
#ifdef HAVE_EMOTION
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   double pos, length;
   Eina_Bool seekable;

   if (!_elm_video_check(content)) return ;

   _cleanup_callback(wd);

   wd->video = content;

   if (!wd->video)
     {
        wd->emotion = NULL;
        return ;
     }

   elm_object_disabled_set(wd->slider, EINA_FALSE);
   elm_object_disabled_set(wd->forward, EINA_FALSE);
   elm_object_disabled_set(wd->info, EINA_FALSE);
   elm_object_disabled_set(wd->next, EINA_FALSE);
   elm_object_disabled_set(wd->pause, EINA_FALSE);
   elm_object_disabled_set(wd->play, EINA_FALSE);
   elm_object_disabled_set(wd->prev, EINA_FALSE);
   elm_object_disabled_set(wd->rewind, EINA_FALSE);
   elm_object_disabled_set(wd->next, EINA_FALSE);

   wd->emotion = elm_video_emotion_get(wd->video);
   emotion_object_priority_set(wd->emotion, EINA_TRUE);
   evas_object_event_callback_add(wd->video, EVAS_CALLBACK_DEL,
                                  _track_video, obj);

   seekable = elm_video_is_seekable(wd->video);
   length = elm_video_play_length_get(wd->video);
   pos = elm_video_play_position_get(wd->video);

   elm_object_disabled_set(wd->slider, !seekable);
   elm_slider_min_max_set(wd->slider, 0, length);
   elm_slider_value_set(wd->slider, pos);

   if (elm_video_is_playing(wd->video)) edje_object_signal_emit(wd->layout, "elm,player,play", "elm");
   else edje_object_signal_emit(wd->layout, "elm,player,pause", "elm");

   evas_object_smart_callback_add(wd->emotion, "frame_decode",
                                  _update_slider, obj);
   evas_object_smart_callback_add(wd->emotion, "frame_resize",
                                  _update_slider, obj);
   evas_object_smart_callback_add(wd->emotion, "length_change",
                                  _update_slider, obj);
   evas_object_smart_callback_add(wd->emotion, "position_update",
                                  _update_slider, obj);
   evas_object_smart_callback_add(wd->emotion, "playback_started",
                                  _play_started, obj);
   evas_object_smart_callback_add(wd->emotion, "playback_finished",
                                  _play_finished, obj);

   /* FIXME: track info from video */
#else
   (void) obj;
   (void) content;
#endif

}

EAPI Evas_Object *
elm_player_add(Evas_Object *parent)
{
#ifdef HAVE_EMOTION
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);
   ELM_SET_WIDTYPE(widtype, "player");
   elm_widget_type_set(obj, "player");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_event_hook_set(obj, _event_hook);
   elm_widget_content_set_hook_set(obj, _content_set_hook);

   wd->layout = edje_object_add(e);
   _elm_theme_object_set(obj, wd->layout, "player", "base", "default");
   elm_widget_resize_object_set(obj, wd->layout);
   elm_widget_sub_object_add(obj, wd->layout);
   evas_object_show(wd->layout);
   evas_object_size_hint_weight_set(wd->layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   wd->forward = _player_button_add(parent, obj, wd->layout, "media_player/forward", _forward);
   wd->info = _player_button_add(parent, obj, wd->layout, "media_player/info", _info);
   wd->next = _player_button_add(parent, obj, wd->layout, "media_player/next", _next);
   wd->pause = _player_button_add(parent, obj, wd->layout, "media_player/pause", _pause);
   wd->play = _player_button_add(parent, obj, wd->layout, "media_player/play", _play);
   wd->prev = _player_button_add(parent, obj, wd->layout, "media_player/prev", _prev);
   wd->rewind = _player_button_add(parent, obj, wd->layout, "media_player/rewind", _rewind);
   wd->stop = _player_button_add(parent, obj, wd->layout, "media_player/stop", _stop);

   wd->slider = elm_slider_add(parent);
   elm_widget_sub_object_add(obj, wd->slider);
   elm_slider_indicator_format_function_set(wd->slider, _double_to_time, eina_stringshare_del);
   elm_slider_units_format_function_set(wd->slider, _double_to_time, eina_stringshare_del);
   elm_slider_min_max_set(wd->slider, 0, 0);
   elm_slider_value_set(wd->slider, 0);
   elm_object_disabled_set(wd->slider, EINA_TRUE);
   evas_object_size_hint_align_set(wd->slider, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(wd->slider, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   edje_object_part_swallow(wd->layout, "media_player/slider", wd->slider);
   evas_object_smart_callback_add(wd->slider, "changed", _update_position, obj);

   wd->emotion = NULL;
   wd->video = NULL;

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _sizing_eval(obj);

   return obj;
#else
   (void) parent;
   return NULL;
#endif
}

EAPI void
elm_player_video_set(Evas_Object *player, Evas_Object *video)
{
   _content_set_hook(player, NULL, video);
}
