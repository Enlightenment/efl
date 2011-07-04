#include <Elementary.h>
#include "elm_priv.h"

#ifdef HAVE_EMOTION
# include <Emotion.h>
#endif

/**
 * @defgroup Video Video
 *
 * This object take care of setting up an Emotion object to display it's content with the right
 * aspect ratio. If build with eio, it also remember the last position that was played and start
 * playing from this position. It expose the basic property of an Emotion and let the more complex
 * accessible by retrieving the Emotion object. It also needed to link it with a Elementary @ref player.
 */

/* TODO: add buffering support to Emotion and display buffering progression in the theme when needed */

typedef struct _Widget_Data Widget_Data;
struct _Widget_Data
{
   Evas_Object *layout;
   Evas_Object *emotion;

   Ecore_Timer *timer;

   Eina_Bool stop : 1;
   Eina_Bool remember : 1;
};

#ifdef HAVE_EMOTION
static const char *widtype = NULL;

static const Evas_Smart_Cb_Description _signals[] = {
  { NULL, NULL }
};

static void _del_hook(Evas_Object *obj);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
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
   if ((!strcmp(ev->keyname, "Left")) || (!strcmp(ev->keyname, "KP_Left")))
     {
        double current, last;

        current = elm_video_play_position_get(obj);
        last = elm_video_play_length_get(obj);

        if (current < last)
          {
             current += last / 100;
             elm_video_play_position_set(obj, current);
          }

        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   if ((!strcmp(ev->keyname, "Right")) || (!strcmp(ev->keyname, "KP_Right")))
     {
        double current, last;

        current = elm_video_play_position_get(obj);
        last = elm_video_play_length_get(obj);

        if (current > 0)
          {
             current -= last / 100;
             if (current < 0) current = 0;
             elm_video_play_position_set(obj, current);
          }

        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   if (!strcmp(ev->keyname, "space"))
     {
        if (elm_video_is_playing(obj))
          elm_video_pause(obj);
        else
          elm_video_play(obj);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   fprintf(stderr, "keyname: '%s' not handle\n", ev->keyname);
   return EINA_FALSE;
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   if (wd->timer) ecore_timer_del(wd->timer);
   free(wd);
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
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;
   Evas_Coord w, h;

   if (!wd) return;
   evas_object_size_hint_request_get(wd->emotion, &minw, &minh);
   evas_object_size_hint_aspect_set(wd->emotion, EVAS_ASPECT_CONTROL_BOTH, minw, minh);
   edje_object_size_min_calc(wd->layout, &w, &h);

   if (w != 0 && h != 0)
     {
        minw = w;
        minh = h;
     }
   evas_object_size_hint_aspect_set(obj, EVAS_ASPECT_CONTROL_BOTH, minw, minh);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_sub_del(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (wd->remember) emotion_object_last_position_save(wd->emotion);
}

static void
_open_done(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);

   edje_object_signal_emit(wd->layout, "elm,video,open", "elm");
}

static void
_playback_started(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);

   edje_object_signal_emit(wd->layout, "elm,video,play", "elm");
}

static void
_playback_finished(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);

   edje_object_signal_emit(wd->layout, "elm,video,end", "elm");
}

static void
_update_aspect_ratio(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_title_change(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   const char *title;

   title = emotion_object_title_get(wd->emotion);
   edje_object_part_text_set(wd->layout, "elm,title", title);
   edje_object_signal_emit(wd->layout, "elm,video,title", "elm");
}

static void
_audio_level_change(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   (void) data;
}

static Eina_Bool
_suspend_cb(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   double interval;

   interval = ecore_timer_interval_get(wd->timer);
   if (interval <= 20)
     emotion_object_suspend_set(wd->emotion, EMOTION_SLEEP);
   else if (interval <= 30)
     emotion_object_suspend_set(wd->emotion, EMOTION_DEEP_SLEEP);
   else
     {
        emotion_object_suspend_set(wd->emotion, EMOTION_HIBERNATE);
        wd->timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }

   ecore_timer_interval_set(wd->timer, interval + 10);
   return ECORE_CALLBACK_RENEW;
}
#endif

EAPI Evas_Object *
elm_video_add(Evas_Object *parent)
{
#ifdef HAVE_EMOTION
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);
   ELM_SET_WIDTYPE(widtype, "video");
   elm_widget_type_set(obj, "video");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_event_hook_set(obj, _event_hook);

   wd->stop = EINA_FALSE;
   wd->remember = EINA_FALSE;

   wd->layout = edje_object_add(e);
   _elm_theme_object_set(obj, wd->layout, "video", "base", "default");
   elm_widget_resize_object_set(obj, wd->layout);
   elm_widget_sub_object_add(obj, wd->layout);
   evas_object_show(wd->layout);
   evas_object_size_hint_weight_set(wd->layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   wd->emotion = emotion_object_add(e);
   emotion_object_init(wd->emotion, NULL);
   edje_object_part_swallow(wd->layout, "elm.swallow.video", wd->emotion);
   elm_widget_sub_object_add(obj, wd->emotion);

   evas_object_smart_callback_add(wd->emotion, "open_done", _open_done, obj);
   evas_object_smart_callback_add(wd->emotion, "playback_started", _playback_started, obj);
   evas_object_smart_callback_add(wd->emotion, "playback_finished", _playback_finished, obj);
   evas_object_smart_callback_add(wd->emotion, "frame_resize", _update_aspect_ratio, obj);
   evas_object_smart_callback_add(wd->emotion, "title_change", _title_change, obj);
   evas_object_smart_callback_add(wd->emotion, "audio_level_change", _audio_level_change, obj);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints, NULL);
   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _sizing_eval(obj);

   wd->timer = ecore_timer_add(20.0, _suspend_cb, obj);

   return obj;
#else
   (void) parent;
   return NULL;
#endif
}

EAPI void
elm_video_file_set(Evas_Object *video, const char *filename)
{
#ifdef HAVE_EMOTION
   ELM_CHECK_WIDTYPE(video, widtype);
   Widget_Data *wd = elm_widget_data_get(video);

   if (wd->remember) emotion_object_last_position_save(wd->emotion);
   wd->stop = EINA_FALSE;
   emotion_object_file_set(wd->emotion, filename);
   emotion_object_last_position_load(wd->emotion);
   edje_object_signal_emit(wd->layout, "elm,video,load", "elm");
#else
   (void) video;
   (void) filename;
#endif
}

EAPI void
elm_video_uri_set(Evas_Object *video, const char *uri)
{
#ifdef HAVE_EMOTION
   ELM_CHECK_WIDTYPE(video, widtype);
   Widget_Data *wd = elm_widget_data_get(video);

   if (wd->remember) emotion_object_last_position_save(wd->emotion);
   wd->stop = EINA_FALSE;
   emotion_object_file_set(wd->emotion, uri);
   edje_object_signal_emit(wd->layout, "elm,video,load", "elm");
#else
   (void) video;
   (void) uri;
#endif
}

EAPI Evas_Object *
elm_video_emotion_get(Evas_Object *video)
{
#ifdef HAVE_EMOTION
   ELM_CHECK_WIDTYPE(video, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(video);

   return wd->emotion;
#else
   (void) video;
   return NULL;
#endif
}

EAPI void
elm_video_play(Evas_Object *video)
{
#ifdef HAVE_EMOTION
   ELM_CHECK_WIDTYPE(video, widtype);
   Widget_Data *wd = elm_widget_data_get(video);

   if (emotion_object_play_get(wd->emotion)) return ;

   if (wd->timer) ecore_timer_del(wd->timer);
   wd->timer = NULL;
   wd->stop = EINA_FALSE;
   emotion_object_play_set(wd->emotion, EINA_TRUE);
#else
   (void) video;
#endif
}

/* FIXME: pause will setup timer and go into sleep or
 * hibernate after a while without activity.
 */

EAPI void
elm_video_pause(Evas_Object *video)
{
#ifdef HAVE_EMOTION
   ELM_CHECK_WIDTYPE(video, widtype);
   Widget_Data *wd = elm_widget_data_get(video);

   if (!emotion_object_play_get(wd->emotion)) return ;

   if (!wd->timer) wd->timer = ecore_timer_add(20.0, _suspend_cb, video);
   emotion_object_play_set(wd->emotion, EINA_FALSE);
   edje_object_signal_emit(wd->layout, "elm,video,pause", "elm");
#else
   (void) video;
#endif
}

/* FIXME: stop should go into hibernate state directly.
 */
EAPI void
elm_video_stop(Evas_Object *video)
{
#ifdef HAVE_EMOTION
   ELM_CHECK_WIDTYPE(video, widtype);
   Widget_Data *wd = elm_widget_data_get(video);

   if (!emotion_object_play_get(wd->emotion) && wd->stop) return ;

   if (wd->timer) ecore_timer_del(wd->timer);
   wd->timer = NULL;
   wd->stop = EINA_TRUE;
   emotion_object_play_set(wd->emotion, EINA_FALSE);
   edje_object_signal_emit(wd->layout, "elm,video,stop", "elm");
   emotion_object_suspend_set(wd->emotion, EMOTION_HIBERNATE);
#else
   (void) video;
#endif
}

EAPI Eina_Bool
elm_video_is_playing(Evas_Object *video)
{
#ifdef HAVE_EMOTION
   ELM_CHECK_WIDTYPE(video, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(video);

   return emotion_object_play_get(wd->emotion);
#else
   (void) video;
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_video_audio_mute_get(Evas_Object *video)
{
#ifdef HAVE_EMOTION
   ELM_CHECK_WIDTYPE(video, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(video);

   return emotion_object_audio_mute_get(wd->emotion);
#else
   (void) video;
   return EINA_FALSE;
#endif
}

EAPI void
elm_video_audio_mute_set(Evas_Object *video, Eina_Bool mute)
{
#ifdef HAVE_EMOTION
   ELM_CHECK_WIDTYPE(video, widtype);
   Widget_Data *wd = elm_widget_data_get(video);

   emotion_object_audio_mute_set(wd->emotion, mute);
#else
   (void) video;
   (void) mute;
#endif
}

EAPI double
elm_video_audio_level_get(Evas_Object *video)
{
#ifdef HAVE_EMOTION
   ELM_CHECK_WIDTYPE(video, widtype) 0.0;
   Widget_Data *wd = elm_widget_data_get(video);

   return emotion_object_audio_volume_get(wd->emotion);
#else
   (void) video;
   return 0.0;
#endif
}

EAPI void
elm_video_audio_level_set(Evas_Object *video, double volume)
{
#ifdef HAVE_EMOTION
   ELM_CHECK_WIDTYPE(video, widtype);
   Widget_Data *wd = elm_widget_data_get(video);

   emotion_object_audio_volume_set(wd->emotion, volume);
#else
   (void) video;
   (void) volume;
#endif
}

EAPI double
elm_video_play_position_get(Evas_Object *video)
{
#ifdef HAVE_EMOTION
   ELM_CHECK_WIDTYPE(video, widtype) 0.0;
   Widget_Data *wd = elm_widget_data_get(video);

   return emotion_object_position_get(wd->emotion);
#else
   (void) video;
   return 0.0;
#endif
}

EAPI void
elm_video_play_position_set(Evas_Object *video, double position)
{
#ifdef HAVE_EMOTION
   ELM_CHECK_WIDTYPE(video, widtype);
   Widget_Data *wd = elm_widget_data_get(video);

   emotion_object_position_set(wd->emotion, position);
#else
   (void) video;
   (void) position;
#endif
}

EAPI double
elm_video_play_length_get(Evas_Object *video)
{
#ifdef HAVE_EMOTION
   ELM_CHECK_WIDTYPE(video, widtype) 0.0;
   Widget_Data *wd = elm_widget_data_get(video);

   return emotion_object_play_length_get(wd->emotion);
#else
   (void) video;
   return 0.0;
#endif
}

EAPI const char *
elm_video_title_get(Evas_Object *video)
{
#ifdef HAVE_EMOTION
   ELM_CHECK_WIDTYPE(video, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(video);

   return emotion_object_title_get(wd->emotion);
#else
   (void) video;
   return NULL;
#endif
}

EAPI void
elm_video_remember_position_set(Evas_Object *video, Eina_Bool remember)
{
#ifdef HAVE_EMOTION
   ELM_CHECK_WIDTYPE(video, widtype);
   Widget_Data *wd = elm_widget_data_get(video);

   wd->remember = remember;
#else
   (void) video;
   (void) remember;
#endif
}

EAPI Eina_Bool
elm_video_remember_position_get(Evas_Object *video)
{
#ifdef HAVE_EMOTION
   ELM_CHECK_WIDTYPE(video, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(video);

   return wd->remember;
#else
   (void) video;
   return EINA_FALSE;
#endif
}
