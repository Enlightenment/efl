#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Emotion.h>

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_layout.h"
#include "elm_widget_video.h"

/* TODO: add buffering support to Emotion and display buffering
 * progress in the theme when needed */

EAPI Eo_Op ELM_OBJ_VIDEO_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_VIDEO_CLASS

#define MY_CLASS_NAME "Elm_Video"
#define MY_CLASS_NAME_LEGACY "elm_video"

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};

static void
_elm_video_smart_event(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *src = va_arg(*list, Evas_Object *);
   Evas_Callback_Type type = va_arg(*list, Evas_Callback_Type);
   Evas_Event_Key_Down *ev = va_arg(*list, void *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);

   if (ret) *ret = EINA_FALSE;
   (void) src;

   if (elm_widget_disabled_get(obj)) return;
   if (type != EVAS_CALLBACK_KEY_DOWN) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   if ((!strcmp(ev->key, "Left")) ||
       ((!strcmp(ev->key, "KP_Left")) && (!ev->string)))
     {
        double current, last;

        current = elm_video_play_position_get(obj);
        last = elm_video_play_length_get(obj);

        if (current < last)
          {
             current += last / 100;
             elm_video_play_position_set(obj, current);
          }

        goto success;
     }
   else if ((!strcmp(ev->key, "Right")) ||
            ((!strcmp(ev->key, "KP_Right")) && (!ev->string)))
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

        goto success;
     }
   else if (!strcmp(ev->key, "space"))
     {
        if (elm_video_is_playing_get(obj))
          elm_video_pause(obj);
        else
          elm_video_play(obj);

        goto success;
     }

   INF("keyname: '%s' not handled", ev->key);
   return;

success:
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   if (ret) *ret = EINA_TRUE;
}

static void
_elm_video_smart_sizing_eval(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Video_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   Evas_Coord minw = -1, minh = -1;
   Evas_Coord w, h;

   evas_object_size_hint_request_get(sd->emotion, &minw, &minh);
   evas_object_size_hint_aspect_set
     (sd->emotion, EVAS_ASPECT_CONTROL_BOTH, minw, minh);
   edje_object_size_min_calc(wd->resize_obj, &w, &h);

   if (w != 0 && h != 0)
     {
        minw = w;
        minh = h;
     }

   evas_object_size_hint_aspect_set(obj, EVAS_ASPECT_CONTROL_BOTH, minw, minh);
}

static void
_on_size_hints_changed(void *data EINA_UNUSED,
                       Evas *e EINA_UNUSED,
                       Evas_Object *obj,
                       void *event_info EINA_UNUSED)
{
   elm_layout_sizing_eval(obj);
}

static void
_on_open_done(void *data,
              Evas_Object *obj EINA_UNUSED,
              void *event_info EINA_UNUSED)
{
   elm_layout_signal_emit(data, "elm,video,open", "elm");
}

static void
_on_playback_started(void *data,
                     Evas_Object *obj EINA_UNUSED,
                     void *event_info EINA_UNUSED)
{
   elm_layout_signal_emit(data, "elm,video,play", "elm");
}

static void
_on_playback_finished(void *data,
                      Evas_Object *obj EINA_UNUSED,
                      void *event_info EINA_UNUSED)
{
   ELM_VIDEO_DATA_GET(data, sd);
   emotion_object_play_set(sd->emotion, EINA_FALSE);
   elm_layout_signal_emit(data, "elm,video,end", "elm");
}

static void
_on_aspect_ratio_updated(void *data,
                         Evas_Object *obj EINA_UNUSED,
                         void *event_info EINA_UNUSED)
{
   elm_layout_sizing_eval(data);
}

static void
_on_title_changed(void *data,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED)
{
   const char *title;

   ELM_VIDEO_DATA_GET(data, sd);

   title = emotion_object_title_get(sd->emotion);
   elm_layout_text_set(data, "elm,title", title);
   elm_layout_signal_emit(data, "elm,video,title", "elm");
}

static void
_on_audio_level_changed(void *data,
                        Evas_Object *obj EINA_UNUSED,
                        void *event_info EINA_UNUSED)
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

Eina_Bool
_elm_video_check(Evas_Object *video)
{
   ELM_VIDEO_CHECK(video) EINA_FALSE;
   return EINA_TRUE;
}

static void
_elm_video_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Video_Smart_Data *priv = _pd;

   _elm_emotion_init();

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);
   elm_widget_can_focus_set(obj, EINA_TRUE);

   priv->emotion = emotion_object_add(evas_object_evas_get(obj));
   emotion_object_init(priv->emotion, NULL);

   if (!elm_layout_theme_set(obj, "video", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

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
}

static void
_elm_video_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Video_Smart_Data *sd = _pd;

   ecore_timer_del(sd->timer);
   if (sd->remember) emotion_object_last_position_save(sd->emotion);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EAPI Evas_Object *
elm_video_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));
}

EAPI Eina_Bool
elm_video_file_set(Evas_Object *obj,
                   const char *filename)
{
   ELM_VIDEO_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_video_file_set(filename, &ret));
   return ret;
}

static void
_file_set(Eo *obj, void *_pd, va_list *list)
{
   const char *filename = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Elm_Video_Smart_Data *sd = _pd;

   if (sd->remember) emotion_object_last_position_save(sd->emotion);
   sd->stop = EINA_FALSE;
   if (!emotion_object_file_set(sd->emotion, filename)) return;

   if (filename && ((!strncmp(filename, "file://", 7)) || (!strstr(filename, "://"))))
     emotion_object_last_position_load(sd->emotion);

   elm_layout_signal_emit(obj, "elm,video,load", "elm");

   if (ret) *ret = EINA_TRUE;
}

EAPI Evas_Object *
elm_video_emotion_get(const Evas_Object *obj)
{
   ELM_VIDEO_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do((Eo *) obj, elm_obj_video_emotion_get(&ret));
   return ret;
}

static void
_emotion_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   *ret = NULL;
   Elm_Video_Smart_Data *sd = _pd;
   *ret = sd->emotion;
}

EAPI void
elm_video_play(Evas_Object *obj)
{
   ELM_VIDEO_CHECK(obj);
   eo_do(obj, elm_obj_video_play());
}

static void
_play(Eo *obj EINA_UNUSED, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Video_Smart_Data *sd = _pd;

   if (emotion_object_play_get(sd->emotion)) return;

   ELM_SAFE_FREE(sd->timer, ecore_timer_del);
   sd->stop = EINA_FALSE;
   emotion_object_play_set(sd->emotion, EINA_TRUE);
   elm_layout_signal_emit(obj, "elm,video,play", "elm");
}

/* FIXME: pause will setup timer and go into sleep or
 * hibernate after a while without activity.
 */
EAPI void
elm_video_pause(Evas_Object *obj)
{
   ELM_VIDEO_CHECK(obj);
   eo_do(obj, elm_obj_video_pause());
}

static void
_pause(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Video_Smart_Data *sd = _pd;

   if (!emotion_object_play_get(sd->emotion)) return;

   if (!sd->timer) sd->timer = ecore_timer_add(20.0, _suspend_cb, obj);
   emotion_object_play_set(sd->emotion, EINA_FALSE);
   elm_layout_signal_emit(obj, "elm,video,pause", "elm");
}

/* FIXME: stop should go into hibernate state directly.
 */
EAPI void
elm_video_stop(Evas_Object *obj)
{
   ELM_VIDEO_CHECK(obj);
   eo_do(obj, elm_obj_video_stop());
}

static void
_stop(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Video_Smart_Data *sd = _pd;

   if (!emotion_object_play_get(sd->emotion) && sd->stop) return;

   ELM_SAFE_FREE(sd->timer, ecore_timer_del);

   sd->stop = EINA_TRUE;
   emotion_object_play_set(sd->emotion, EINA_FALSE);
   elm_layout_signal_emit(obj, "elm,video,stop", "elm");
   emotion_object_suspend_set(sd->emotion, EMOTION_HIBERNATE);
}

EAPI Eina_Bool
elm_video_is_playing_get(const Evas_Object *obj)
{
   ELM_VIDEO_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_video_is_playing_get(&ret));
   return ret;
}

static void
_is_playing_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
   Elm_Video_Smart_Data *sd = _pd;
   *ret = emotion_object_play_get(sd->emotion);
}

EAPI Eina_Bool
elm_video_is_seekable_get(const Evas_Object *obj)
{
   ELM_VIDEO_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_video_is_seekable_get(&ret));
   return ret;
}

static void
_is_seekable_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Video_Smart_Data *sd = _pd;
   *ret = emotion_object_seekable_get(sd->emotion);
}

EAPI Eina_Bool
elm_video_audio_mute_get(const Evas_Object *obj)
{
   ELM_VIDEO_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_video_audio_mute_get(&ret));
   return ret;
}

static void
_audio_mute_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
   Elm_Video_Smart_Data *sd = _pd;
   *ret = emotion_object_audio_mute_get(sd->emotion);
}

EAPI void
elm_video_audio_mute_set(Evas_Object *obj,
                         Eina_Bool mute)
{
   ELM_VIDEO_CHECK(obj);
   eo_do(obj, elm_obj_video_audio_mute_set(mute));
}

static void
_audio_mute_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool mute = va_arg(*list, int);
   Elm_Video_Smart_Data *sd = _pd;
   emotion_object_audio_mute_set(sd->emotion, mute);
}

EAPI double
elm_video_audio_level_get(const Evas_Object *obj)
{
   ELM_VIDEO_CHECK(obj) 0.0;
   double ret = 0.0;
   eo_do((Eo *) obj, elm_obj_video_audio_level_get(&ret));
   return ret;
}

static void
_audio_level_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double *ret = va_arg(*list, double *);
   *ret = 0.0;
   Elm_Video_Smart_Data *sd = _pd;
   *ret = emotion_object_audio_volume_get(sd->emotion);
}

EAPI void
elm_video_audio_level_set(Evas_Object *obj,
                          double volume)
{
   ELM_VIDEO_CHECK(obj);
   eo_do(obj, elm_obj_video_audio_level_set(volume));
}

static void
_audio_level_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double volume = va_arg(*list, double);
   Elm_Video_Smart_Data *sd = _pd;
   emotion_object_audio_volume_set(sd->emotion, volume);
}

EAPI double
elm_video_play_position_get(const Evas_Object *obj)
{
   ELM_VIDEO_CHECK(obj) 0.0;
   double ret = 0.0;
   eo_do((Eo *) obj, elm_obj_video_play_position_get(&ret));
   return ret;
}

static void
_play_position_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double *ret = va_arg(*list, double *);
   *ret = 0.0;
   Elm_Video_Smart_Data *sd = _pd;
   *ret = emotion_object_position_get(sd->emotion);
}

EAPI void
elm_video_play_position_set(Evas_Object *obj,
                            double position)
{
   ELM_VIDEO_CHECK(obj);
   eo_do(obj, elm_obj_video_play_position_set(position));
}

static void
_play_position_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double position = va_arg(*list, double);
   Elm_Video_Smart_Data *sd = _pd;
   emotion_object_position_set(sd->emotion, position);
}

EAPI double
elm_video_play_length_get(const Evas_Object *obj)
{
   ELM_VIDEO_CHECK(obj) 0.0;
   double ret = 0.0;
   eo_do((Eo *) obj, elm_obj_video_play_length_get(&ret));
   return ret;
}

static void
_play_length_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double *ret = va_arg(*list, double *);
   *ret = 0.0;
   Elm_Video_Smart_Data *sd = _pd;
   *ret = emotion_object_play_length_get(sd->emotion);
}

EAPI const char *
elm_video_title_get(const Evas_Object *obj)
{
   ELM_VIDEO_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_obj_video_title_get(&ret));
   return ret;
}

static void
_title_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **ret = va_arg(*list, const char **);
   *ret = NULL;
   Elm_Video_Smart_Data *sd = _pd;
   *ret = emotion_object_title_get(sd->emotion);
}

EAPI void
elm_video_remember_position_set(Evas_Object *obj,
                                Eina_Bool remember)
{
   ELM_VIDEO_CHECK(obj);
   eo_do(obj, elm_obj_video_remember_position_set(remember));
}

static void
_remember_position_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool remember = va_arg(*list, int);
   Elm_Video_Smart_Data *sd = _pd;
   sd->remember = remember;
}

EAPI Eina_Bool
elm_video_remember_position_get(const Evas_Object *obj)
{
   ELM_VIDEO_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_video_remember_position_get(&ret));
   return ret;
}

static void
_remember_position_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
   Elm_Video_Smart_Data *sd = _pd;
   *ret = sd->remember;
}

static void
_elm_video_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_elm_video_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_video_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_video_smart_del),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_EVENT), _elm_video_smart_event),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_video_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_video_smart_focus_direction_manager_is),

        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_video_smart_sizing_eval),

        EO_OP_FUNC(ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_FILE_SET), _file_set),
        EO_OP_FUNC(ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_EMOTION_GET), _emotion_get),
        EO_OP_FUNC(ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_PLAY), _play),
        EO_OP_FUNC(ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_PAUSE), _pause),
        EO_OP_FUNC(ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_STOP), _stop),
        EO_OP_FUNC(ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_IS_PLAYING_GET), _is_playing_get),
        EO_OP_FUNC(ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_IS_SEEKABLE_GET), _is_seekable_get),
        EO_OP_FUNC(ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_AUDIO_MUTE_GET), _audio_mute_get),
        EO_OP_FUNC(ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_AUDIO_MUTE_SET), _audio_mute_set),
        EO_OP_FUNC(ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_AUDIO_LEVEL_GET), _audio_level_get),
        EO_OP_FUNC(ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_AUDIO_LEVEL_SET), _audio_level_set),
        EO_OP_FUNC(ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_PLAY_POSITION_GET), _play_position_get),
        EO_OP_FUNC(ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_PLAY_POSITION_SET), _play_position_set),
        EO_OP_FUNC(ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_PLAY_LENGTH_GET), _play_length_get),
        EO_OP_FUNC(ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_TITLE_GET), _title_get),
        EO_OP_FUNC(ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_REMEMBER_POSITION_SET), _remember_position_set),
        EO_OP_FUNC(ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_REMEMBER_POSITION_GET), _remember_position_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}
static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_VIDEO_SUB_ID_FILE_SET, "Define the file or URI that will be the video source."),
     EO_OP_DESCRIPTION(ELM_OBJ_VIDEO_SUB_ID_EMOTION_GET, "Get the underlying Emotion object."),
     EO_OP_DESCRIPTION(ELM_OBJ_VIDEO_SUB_ID_PLAY, "Start to play the video."),
     EO_OP_DESCRIPTION(ELM_OBJ_VIDEO_SUB_ID_PAUSE, "Pause the video."),
     EO_OP_DESCRIPTION(ELM_OBJ_VIDEO_SUB_ID_STOP, "Stop the video."),
     EO_OP_DESCRIPTION(ELM_OBJ_VIDEO_SUB_ID_IS_PLAYING_GET, "Is the video actually playing."),
     EO_OP_DESCRIPTION(ELM_OBJ_VIDEO_SUB_ID_IS_SEEKABLE_GET, "Is it possible to seek inside the video."),
     EO_OP_DESCRIPTION(ELM_OBJ_VIDEO_SUB_ID_AUDIO_MUTE_GET, "Is the audio muted."),
     EO_OP_DESCRIPTION(ELM_OBJ_VIDEO_SUB_ID_AUDIO_MUTE_SET, "Change the mute state of the Elm_Video object."),
     EO_OP_DESCRIPTION(ELM_OBJ_VIDEO_SUB_ID_AUDIO_LEVEL_GET, "Get the audio level of the current video ."),
     EO_OP_DESCRIPTION(ELM_OBJ_VIDEO_SUB_ID_AUDIO_LEVEL_SET, "Set the audio level of an Elm_Video object."),
     EO_OP_DESCRIPTION(ELM_OBJ_VIDEO_SUB_ID_PLAY_POSITION_GET, "Get the current position (in seconds) being played in the Elm_Video object."),
     EO_OP_DESCRIPTION(ELM_OBJ_VIDEO_SUB_ID_PLAY_POSITION_SET, "Set the current position (in seconds) to be played in the Elm_Video object."),
     EO_OP_DESCRIPTION(ELM_OBJ_VIDEO_SUB_ID_PLAY_LENGTH_GET, "Get the total playing time (in seconds) of the Elm_Video object."),
     EO_OP_DESCRIPTION(ELM_OBJ_VIDEO_SUB_ID_TITLE_GET, "Get the title (for instance DVD title) from this emotion object."),
     EO_OP_DESCRIPTION(ELM_OBJ_VIDEO_SUB_ID_REMEMBER_POSITION_SET, "Set whether the object can remember the last played position."),
     EO_OP_DESCRIPTION(ELM_OBJ_VIDEO_SUB_ID_REMEMBER_POSITION_GET, "Set whether the object can remember the last played position."),
     EO_OP_DESCRIPTION_SENTINEL
};
static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_VIDEO_BASE_ID, op_desc, ELM_OBJ_VIDEO_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Video_Smart_Data),
     _class_constructor,
     NULL
};
EO_DEFINE_CLASS(elm_obj_video_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, NULL);
