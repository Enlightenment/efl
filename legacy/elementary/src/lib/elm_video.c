#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_video.h"

#ifdef HAVE_EMOTION
# include <Emotion.h>
#endif

/* TODO: add buffering support to Emotion and display buffering
 * progress in the theme when needed */

EAPI const char ELM_VIDEO_SMART_NAME[] = "elm_video";

EVAS_SMART_SUBCLASS_NEW
  (ELM_VIDEO_SMART_NAME, _elm_video, Elm_Video_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, NULL);

static Eina_Bool
_elm_video_smart_event(Evas_Object *obj,
                       Evas_Object *src __UNUSED__,
                       Evas_Callback_Type type,
                       void *event_info)
{
#ifdef HAVE_EMOTION
   Evas_Event_Key_Down *ev = event_info;

   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;

   if ((!strcmp(ev->keyname, "Left")) ||
       ((!strcmp(ev->keyname, "KP_Left")) && (!ev->string)))
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

   if ((!strcmp(ev->keyname, "Right")) ||
       ((!strcmp(ev->keyname, "KP_Right")) && (!ev->string)))
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
        if (elm_video_is_playing_get(obj))
          elm_video_pause(obj);
        else
          elm_video_play(obj);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

        return EINA_TRUE;
     }

   INF("keyname: '%s' not handled", ev->keyname);

   return EINA_FALSE;
#else

   (void) obj;
   (void) type;
   (void) event_info;

   return EINA_FALSE;
#endif
}

static void
_elm_video_smart_sizing_eval(Evas_Object *obj)
{
#ifdef HAVE_EMOTION
   ELM_VIDEO_DATA_GET(obj, sd);

   Evas_Coord minw = -1, minh = -1;
   Evas_Coord w, h;

   evas_object_size_hint_request_get(sd->emotion, &minw, &minh);
   evas_object_size_hint_aspect_set
     (sd->emotion, EVAS_ASPECT_CONTROL_BOTH, minw, minh);
   edje_object_size_min_calc(ELM_WIDGET_DATA(sd)->resize_obj, &w, &h);

   if (w != 0 && h != 0)
     {
        minw = w;
        minh = h;
     }

   evas_object_size_hint_aspect_set(obj, EVAS_ASPECT_CONTROL_BOTH, minw, minh);
#else

   (void) obj;
#endif
}

#ifdef HAVE_EMOTION
static void
_on_size_hints_changed(void *data __UNUSED__,
                       Evas *e __UNUSED__,
                       Evas_Object *obj,
                       void *event_info __UNUSED__)
{
   elm_layout_sizing_eval(obj);
}

static void
_on_open_done(void *data,
              Evas_Object *obj __UNUSED__,
              void *event_info __UNUSED__)
{
   elm_layout_signal_emit(data, "elm,video,open", "elm");
}

static void
_on_playback_started(void *data,
                     Evas_Object *obj __UNUSED__,
                     void *event_info __UNUSED__)
{
   elm_layout_signal_emit(data, "elm,video,play", "elm");
}

static void
_on_playback_finished(void *data,
                      Evas_Object *obj __UNUSED__,
                      void *event_info __UNUSED__)
{
   elm_layout_signal_emit(data, "elm,video,end", "elm");
}

static void
_on_aspect_ratio_updated(void *data,
                         Evas_Object *obj __UNUSED__,
                         void *event_info __UNUSED__)
{
   elm_layout_sizing_eval(data);
}

static void
_on_title_changed(void *data,
                  Evas_Object *obj __UNUSED__,
                  void *event_info __UNUSED__)
{
   const char *title;

   ELM_VIDEO_DATA_GET(data, sd);

   title = emotion_object_title_get(sd->emotion);
   elm_layout_text_set(data, "elm,title", title);
   elm_layout_signal_emit(data, "elm,video,title", "elm");
}

static void
_on_audio_level_changed(void *data,
                        Evas_Object *obj __UNUSED__,
                        void *event_info __UNUSED__)
{
   (void)data;
}

static Eina_Bool
_suspend_cb(void *data)
{
   double interval;

   ELM_VIDEO_DATA_GET(data, sd);

   interval = ecore_timer_interval_get(sd->timer);
   if (interval <= 20)
     emotion_object_suspend_set(sd->emotion, EMOTION_SLEEP);
   else if (interval <= 30)
     emotion_object_suspend_set(sd->emotion, EMOTION_DEEP_SLEEP);
   else
     {
        emotion_object_suspend_set(sd->emotion, EMOTION_HIBERNATE);
        sd->timer = NULL;

        return ECORE_CALLBACK_CANCEL;
     }

   ecore_timer_interval_set(sd->timer, interval + 10);
   return ECORE_CALLBACK_RENEW;
}

#endif

Eina_Bool
_elm_video_check(Evas_Object *video)
{
#ifdef HAVE_EMOTION
   ELM_VIDEO_CHECK(video) EINA_FALSE;

   return EINA_TRUE;
#else
   (void)video;

   return EINA_FALSE;
#endif
}

static void
_elm_video_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Video_Smart_Data);

   _elm_emotion_init();

   ELM_WIDGET_CLASS(_elm_video_parent_sc)->base.add(obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

#ifdef HAVE_EMOTION
   priv->stop = EINA_FALSE;
   priv->remember = EINA_FALSE;

   priv->emotion = emotion_object_add(evas_object_evas_get(obj));
   emotion_object_init(priv->emotion, NULL);

   elm_layout_theme_set(obj, "video", "base", elm_widget_style_get(obj));
   elm_layout_content_set(obj, "elm.swallow.video", priv->emotion);

   evas_object_smart_callback_add
     (priv->emotion, "open_done", _on_open_done, obj);
   evas_object_smart_callback_add
     (priv->emotion, "playback_started", _on_playback_started, obj);
   evas_object_smart_callback_add
     (priv->emotion, "playback_finished", _on_playback_finished, obj);
   evas_object_smart_callback_add
     (priv->emotion, "frame_resize", _on_aspect_ratio_updated, obj);
   evas_object_smart_callback_add
     (priv->emotion, "title_change", _on_title_changed, obj);
   evas_object_smart_callback_add
     (priv->emotion, "audio_level_change", _on_audio_level_changed, obj);

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_size_hints_changed, NULL);

   priv->timer = ecore_timer_add(20.0, _suspend_cb, obj);
#endif
}

static void
_elm_video_smart_del(Evas_Object *obj)
{
   ELM_VIDEO_DATA_GET(obj, sd);

#ifdef HAVE_EMOTION
   if (sd->timer) ecore_timer_del(sd->timer);
   if (sd->remember) emotion_object_last_position_save(sd->emotion);
#else
   (void) sd;
#endif

   ELM_WIDGET_CLASS(_elm_video_parent_sc)->base.del(obj);
}

static void
_elm_video_smart_set_user(Elm_Video_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_video_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_video_smart_del;

   ELM_WIDGET_CLASS(sc)->event = _elm_video_smart_event;

   /* not a 'focus chain manager' */
   ELM_WIDGET_CLASS(sc)->focus_next = NULL;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_video_smart_sizing_eval;
}

EAPI const Elm_Video_Smart_Class *
elm_video_smart_class_get(void)
{
   static Elm_Video_Smart_Class _sc =
     ELM_VIDEO_SMART_CLASS_INIT_NAME_VERSION(ELM_VIDEO_SMART_NAME);
   static const Elm_Video_Smart_Class *class = NULL;

   if (class)
     return class;

   _elm_video_smart_set(&_sc);
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_video_add(Evas_Object *parent)
{
#ifdef HAVE_EMOTION
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_video_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
#else
   (void) parent;
   (void) _elm_video_smart_class_new;

   return NULL;
#endif
}

EAPI Eina_Bool
elm_video_file_set(Evas_Object *obj,
                   const char *filename)
{
#ifdef HAVE_EMOTION
   ELM_VIDEO_CHECK(obj) EINA_FALSE;
   ELM_VIDEO_DATA_GET(obj, sd);

   if (sd->remember) emotion_object_last_position_save(sd->emotion);
   sd->stop = EINA_FALSE;
   if (!emotion_object_file_set(sd->emotion, filename)) return EINA_FALSE;

   if (filename && ((!strncmp(filename, "file://", 7)) || (!strstr(filename, "://"))))
     emotion_object_last_position_load(sd->emotion);

   elm_layout_signal_emit(obj, "elm,video,load", "elm");

   return EINA_TRUE;
#else
   (void)obj;
   (void)filename;

   return EINA_FALSE;
#endif
}

EAPI Evas_Object *
elm_video_emotion_get(const Evas_Object *obj)
{
#ifdef HAVE_EMOTION
   ELM_VIDEO_CHECK(obj) NULL;
   ELM_VIDEO_DATA_GET(obj, sd);

   return sd->emotion;
#else
   (void)obj;
   return NULL;
#endif
}

EAPI void
elm_video_play(Evas_Object *obj)
{
#ifdef HAVE_EMOTION
   ELM_VIDEO_CHECK(obj);
   ELM_VIDEO_DATA_GET(obj, sd);

   if (emotion_object_play_get(sd->emotion)) return;

   if (sd->timer) ecore_timer_del(sd->timer);
   sd->timer = NULL;
   sd->stop = EINA_FALSE;
   emotion_object_play_set(sd->emotion, EINA_TRUE);
#else
   (void)obj;
#endif
}

/* FIXME: pause will setup timer and go into sleep or
 * hibernate after a while without activity.
 */
EAPI void
elm_video_pause(Evas_Object *obj)
{
#ifdef HAVE_EMOTION
   ELM_VIDEO_CHECK(obj);
   ELM_VIDEO_DATA_GET(obj, sd);

   if (!emotion_object_play_get(sd->emotion)) return;

   if (!sd->timer) sd->timer = ecore_timer_add(20.0, _suspend_cb, obj);
   emotion_object_play_set(sd->emotion, EINA_FALSE);
   elm_layout_signal_emit(obj, "elm,video,pause", "elm");
#else
   (void)obj;
#endif
}

/* FIXME: stop should go into hibernate state directly.
 */
EAPI void
elm_video_stop(Evas_Object *obj)
{
#ifdef HAVE_EMOTION
   ELM_VIDEO_CHECK(obj);
   ELM_VIDEO_DATA_GET(obj, sd);

   if (!emotion_object_play_get(sd->emotion) && sd->stop) return;

   if (sd->timer) ecore_timer_del(sd->timer);
   sd->timer = NULL;

   sd->stop = EINA_TRUE;
   emotion_object_play_set(sd->emotion, EINA_FALSE);
   elm_layout_signal_emit(obj, "elm,video,stop", "elm");
   emotion_object_suspend_set(sd->emotion, EMOTION_HIBERNATE);
#else
   (void)obj;
#endif
}

EAPI Eina_Bool
elm_video_is_playing_get(const Evas_Object *obj)
{
#ifdef HAVE_EMOTION
   ELM_VIDEO_CHECK(obj) EINA_FALSE;
   ELM_VIDEO_DATA_GET(obj, sd);

   return emotion_object_play_get(sd->emotion);
#else
   (void)obj;
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_video_is_seekable_get(const Evas_Object *obj)
{
#ifdef HAVE_EMOTION
   ELM_VIDEO_CHECK(obj) EINA_FALSE;
   ELM_VIDEO_DATA_GET(obj, sd);

   return emotion_object_seekable_get(sd->emotion);
#else
   (void)obj;
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
elm_video_audio_mute_get(const Evas_Object *obj)
{
#ifdef HAVE_EMOTION
   ELM_VIDEO_CHECK(obj) EINA_FALSE;
   ELM_VIDEO_DATA_GET(obj, sd);

   return emotion_object_audio_mute_get(sd->emotion);
#else
   (void)obj;
   return EINA_FALSE;
#endif
}

EAPI void
elm_video_audio_mute_set(Evas_Object *obj,
                         Eina_Bool mute)
{
#ifdef HAVE_EMOTION
   ELM_VIDEO_CHECK(obj);
   ELM_VIDEO_DATA_GET(obj, sd);

   emotion_object_audio_mute_set(sd->emotion, mute);
#else
   (void)obj;
   (void)mute;
#endif
}

EAPI double
elm_video_audio_level_get(const Evas_Object *obj)
{
#ifdef HAVE_EMOTION
   ELM_VIDEO_CHECK(obj) 0.0;
   ELM_VIDEO_DATA_GET(obj, sd);

   return emotion_object_audio_volume_get(sd->emotion);
#else
   (void)obj;
   return 0.0;
#endif
}

EAPI void
elm_video_audio_level_set(Evas_Object *obj,
                          double volume)
{
#ifdef HAVE_EMOTION
   ELM_VIDEO_CHECK(obj);
   ELM_VIDEO_DATA_GET(obj, sd);

   emotion_object_audio_volume_set(sd->emotion, volume);
#else
   (void)obj;
   (void)volume;
#endif
}

EAPI double
elm_video_play_position_get(const Evas_Object *obj)
{
#ifdef HAVE_EMOTION
   ELM_VIDEO_CHECK(obj) 0.0;
   ELM_VIDEO_DATA_GET(obj, sd);

   return emotion_object_position_get(sd->emotion);
#else
   (void)obj;
   return 0.0;
#endif
}

EAPI void
elm_video_play_position_set(Evas_Object *obj,
                            double position)
{
#ifdef HAVE_EMOTION
   ELM_VIDEO_CHECK(obj);
   ELM_VIDEO_DATA_GET(obj, sd);

   emotion_object_position_set(sd->emotion, position);
#else
   (void)obj;
   (void)position;
#endif
}

EAPI double
elm_video_play_length_get(const Evas_Object *obj)
{
#ifdef HAVE_EMOTION
   ELM_VIDEO_CHECK(obj) 0.0;
   ELM_VIDEO_DATA_GET(obj, sd);

   return emotion_object_play_length_get(sd->emotion);
#else
   (void)obj;
   return 0.0;
#endif
}

EAPI const char *
elm_video_title_get(const Evas_Object *obj)
{
#ifdef HAVE_EMOTION
   ELM_VIDEO_CHECK(obj) NULL;
   ELM_VIDEO_DATA_GET(obj, sd);

   return emotion_object_title_get(sd->emotion);
#else
   (void)obj;
   return NULL;
#endif
}

EAPI void
elm_video_remember_position_set(Evas_Object *obj,
                                Eina_Bool remember)
{
#ifdef HAVE_EMOTION
   ELM_VIDEO_CHECK(obj);
   ELM_VIDEO_DATA_GET(obj, sd);

   sd->remember = remember;
#else
   (void)obj;
   (void)remember;
#endif
}

EAPI Eina_Bool
elm_video_remember_position_get(const Evas_Object *obj)
{
#ifdef HAVE_EMOTION
   ELM_VIDEO_CHECK(obj) EINA_FALSE;
   ELM_VIDEO_DATA_GET(obj, sd);

   return sd->remember;
#else
   (void)obj;
   return EINA_FALSE;
#endif
}
