#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_media_private.h"

#define MY_CLASS ECORDOVA_MEDIA_CLASS
#define MY_CLASS_NAME "Ecordova_Media"

typedef void(* Action_Cb)(Ecordova_Media_Data *, void *);

typedef struct {
  Action_Cb  callback;
  void      *data;
} Action;

static void _completed_cb(void *);
static void _error_cb(int, void *);
static void _prepared_cb(void *);
static void _set_position_cb(void *);
static void _execute(Ecordova_Media_Data *, Action_Cb, void *);
static void _pause_action(Ecordova_Media_Data *, void *);
static void _start_action(Ecordova_Media_Data *, void *);
static void _stop_action(Ecordova_Media_Data *, void *);
static void _get_position_action(Ecordova_Media_Data *, void *);
static void _set_position_action(Ecordova_Media_Data *, void *);
static void _set_volume_action(Ecordova_Media_Data *, void *);
static void _update_status(Ecordova_Media_Data *);
static void _release(Ecordova_Media_Data *);
static void _state_changed_cb(recorder_state_e, recorder_state_e, bool, void *);
static void _start_record(Ecordova_Media_Data *);

static Eo_Base *
_ecordova_media_eo_base_constructor(Eo *obj, Ecordova_Media_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;
   pd->player = NULL;
   pd->recorder = NULL;
   pd->status = ECORDOVA_MEDIA_STATUS_MEDIA_NONE;
   pd->pending = NULL;
   pd->record_pending = false;

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_media_constructor(Eo *obj,
                            Ecordova_Media_Data *pd,
                            const char *src)
{
   DBG("(%p)", obj);
   EINA_SAFETY_ON_NULL_RETURN(src);

   int ret = player_create(&pd->player);
   EINA_SAFETY_ON_FALSE_RETURN(PLAYER_ERROR_NONE == ret);

   ret = recorder_create_audiorecorder(&pd->recorder);
   EINA_SAFETY_ON_FALSE_RETURN(RECORDER_ERROR_NONE == ret);

   ret = player_set_uri(pd->player, src);
   if (PLAYER_ERROR_NONE != ret)
     {
        ERR("Error setting source media: %s", src);
        _release(pd);
        return;
     }

   ret = player_set_completed_cb(pd->player,
                                 _completed_cb,
                                 pd);
   if (PLAYER_ERROR_NONE != ret)
     {
        ERR("Error setting completed callback: %d", ret);
        _release(pd);
        return;
     }

   ret = player_set_error_cb(pd->player,
                             _error_cb,
                             pd);
   if (PLAYER_ERROR_NONE != ret)
     {
        ERR("Error setting completed callback: %d", ret);
        _release(pd);
        return;
     }

   ret = recorder_set_state_changed_cb(pd->recorder,
                                       _state_changed_cb,
                                       pd);
   if (RECORDER_ERROR_NONE != ret)
     {
        ERR("Error setting recorder state changed callback: %d", ret);
        _release(pd);
        return;
     }

   ret = recorder_set_audio_encoder(pd->recorder, RECORDER_AUDIO_CODEC_AAC);
   if (RECORDER_ERROR_NONE != ret)
     {
        ERR("Error setting audio encoder: %d", ret);
        _release(pd);
        return;
     }

   ret = recorder_attr_set_audio_samplerate(pd->recorder, 44100);
   if (RECORDER_ERROR_NONE != ret)
     {
        ERR("Error setting audio sample rate: %d", ret);
        _release(pd);
        return;
     }

   ret = recorder_set_file_format(pd->recorder, RECORDER_FILE_FORMAT_3GP);
   if (RECORDER_ERROR_NONE != ret)
     {
        ERR("Error setting file format for audio encoder: %d", ret);
        _release(pd);
        return;
     }

   ret = recorder_attr_set_audio_encoder_bitrate(pd->recorder, 28800);
   if (RECORDER_ERROR_NONE != ret)
     {
        ERR("Error setting audio encoder bitrate: %d", ret);
        _release(pd);
        return;
     }

   ret = recorder_set_filename(pd->recorder, src);
   if (RECORDER_ERROR_NONE != ret)
     {
        ERR("Error setting recorder filename: %d", ret);
        _release(pd);
        return;
     }
}

static void
_ecordova_media_eo_base_destructor(Eo *obj, Ecordova_Media_Data *pd)
{
   DBG("(%p)", obj);

   _release(pd);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static void
_ecordova_media_current_position_get(Eo *obj, Ecordova_Media_Data *pd)
{
   DBG("(%p)", obj);
   _execute(pd, _get_position_action, NULL);
}

static int
_ecordova_media_duration_get(Eo *obj EINA_UNUSED, Ecordova_Media_Data *pd)
{
   int duration = -1;

   int ret = player_get_duration(pd->player, &duration);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(PLAYER_ERROR_NONE == ret, -1);

   return duration / 1000;
}

static void
_ecordova_media_pause(Eo *obj, Ecordova_Media_Data *pd)
{
   DBG("(%p)", obj);
   _execute(pd, _pause_action, NULL);
}

static void
_ecordova_media_play(Eo *obj, Ecordova_Media_Data *pd)
{
   DBG("(%p)", obj);
   _execute(pd, _start_action, NULL);
}

static void
_ecordova_media_release(Eo *obj, Ecordova_Media_Data *pd)
{
   DBG("(%p)", obj);
   _release(pd);
}

static void
_ecordova_media_seek(Eo *obj, Ecordova_Media_Data *pd, int milliseconds)
{
   DBG("(%p)", obj);
   int *data = malloc(sizeof(milliseconds));
   *data = milliseconds;
   _execute(pd, _set_position_action, data);
}

static void
_ecordova_media_volume_set(Eo *obj, Ecordova_Media_Data *pd, double volume)
{
   DBG("(%p)", obj);
   double *data = malloc(sizeof(volume));
   *data = volume;
   _execute(pd, _set_volume_action, data);
}

static void
_ecordova_media_record_start(Eo *obj, Ecordova_Media_Data *pd)
{
   DBG("(%p)", obj);

   recorder_state_e state = RECORDER_STATE_NONE;
   int ret = recorder_get_state(pd->recorder, &state);
   EINA_SAFETY_ON_FALSE_RETURN(RECORDER_ERROR_NONE == ret);

   switch (state)
     {
      case RECORDER_STATE_NONE:
      case RECORDER_STATE_CREATED:
        ret = recorder_prepare(pd->recorder);
        EINA_SAFETY_ON_FALSE_RETURN(RECORDER_ERROR_NONE == ret);
        pd->record_pending = true;
        return;
      case RECORDER_STATE_READY:
        if (pd->record_pending)
          {
             ERR("%s", "The recorder has already been started and is pending.");
             return;
          }
        _start_record(pd);
        return;
      case RECORDER_STATE_RECORDING:
        ERR("%s", "The recorder is already recording.");
        return;
      case RECORDER_STATE_PAUSED:
        ERR("%s", "Unreachable!");
        return;
     }
}

static void
_ecordova_media_stop(Eo *obj, Ecordova_Media_Data *pd)
{
   DBG("(%p)", obj);
   _execute(pd, _stop_action, NULL);
}

static void
_ecordova_media_record_stop(Eo *obj, Ecordova_Media_Data *pd)
{
   DBG("(%p)", obj);

   recorder_state_e state = RECORDER_STATE_NONE;
   int ret = recorder_get_state(pd->recorder, &state);
   EINA_SAFETY_ON_FALSE_RETURN(RECORDER_ERROR_NONE == ret);

   switch (state)
     {
      case RECORDER_STATE_NONE:
      case RECORDER_STATE_CREATED:
      case RECORDER_STATE_READY:
        ERR("%s", "The recorder is not recording.");
        return;
      case RECORDER_STATE_RECORDING:
        ret = recorder_commit(pd->recorder);
        return;
      case RECORDER_STATE_PAUSED:
        ERR("%s", "Unreachable!");
        return;
     }
}

static void
_completed_cb(void *data)
{
   Ecordova_Media_Data *pd = data;
   DBG("(%p)", pd->obj);

   // TODO: Check what thread this callback is running to make sure it's on the main thread

   _update_status(pd);

   eo_do(pd->obj, eo_event_callback_call(ECORDOVA_MEDIA_EVENT_MEDIA_SUCCESS, NULL));
}

static void
_error_cb(int error EINA_UNUSED, void *data)
{
   Ecordova_Media_Data *pd = data;
   DBG("(%p)", pd->obj);

   // TODO: ¿ check error ?

   // TODO: Check what thread this callback is running to make sure it's on the main thread

   _update_status(pd);

   eo_do(pd->obj, eo_event_callback_call(ECORDOVA_MEDIA_EVENT_MEDIA_ERROR, NULL));
}

static void
_prepared_cb(void *data)
{
   Ecordova_Media_Data *pd = data;
   DBG("(%p)", pd->obj);

   Action *action;
   EINA_LIST_FREE(pd->pending, action)
     {
        action->callback(pd, action->data);
        free(action->data);
        free(action);
     }

}

static void
_set_position_cb(void *data)
{
   Ecordova_Media_Data *pd = data;
   DBG("(%p)", pd->obj);

   // TODO: Check what thread this callback is running to make sure it's on the main thread

   eo_do(pd->obj, eo_event_callback_call(ECORDOVA_MEDIA_EVENT_MEDIA_SUCCESS, NULL));
}

static void
_execute(Ecordova_Media_Data *pd, Action_Cb action_cb, void *data)
{
   DBG("(%p)", pd->obj);

   switch (pd->status)
     {
      case ECORDOVA_MEDIA_STATUS_MEDIA_NONE:
        {
           int ret = player_prepare_async(pd->player,
                                          _prepared_cb,
                                          pd);
           EINA_SAFETY_ON_FALSE_RETURN(PLAYER_ERROR_NONE == ret);

           pd->status = ECORDOVA_MEDIA_STATUS_MEDIA_STARTING;
           eo_do(pd->obj,
                 eo_event_callback_call(ECORDOVA_MEDIA_EVENT_MEDIA_STATUS, &pd->status));
           // fall-through
        }
      case ECORDOVA_MEDIA_STATUS_MEDIA_STARTING:
        {
           Action *action = malloc(sizeof(Action));
           action->callback = action_cb;
           action->data = data;

           pd->pending = eina_list_append(pd->pending, action);
           return;
        }
      default:
        action_cb(pd, data);
     }
}

static void
_pause_action(Ecordova_Media_Data *pd, void *data EINA_UNUSED)
{
   DBG("(%p)", pd->obj);

   int ret = player_pause(pd->player);
   EINA_SAFETY_ON_FALSE_RETURN(PLAYER_ERROR_NONE == ret);

   _update_status(pd);
}

static void
_stop_action(Ecordova_Media_Data *pd, void *data EINA_UNUSED)
{
   DBG("(%p)", pd->obj);

   int ret = player_stop(pd->player);
   EINA_SAFETY_ON_FALSE_RETURN(PLAYER_ERROR_NONE == ret);

   _update_status(pd);
}

static void
_start_action(Ecordova_Media_Data *pd, void *data EINA_UNUSED)
{
   DBG("(%p)", pd->obj);

   int ret = player_stop(pd->player);
   EINA_SAFETY_ON_FALSE_RETURN(PLAYER_ERROR_NONE == ret);

   _update_status(pd);
}

static void
_get_position_action(Ecordova_Media_Data *pd, void *data EINA_UNUSED)
{
   DBG("(%p)", pd->obj);

   int position = 0;
   int ret = player_get_play_position(pd->player, &position);
   EINA_SAFETY_ON_FALSE_RETURN(PLAYER_ERROR_NONE == ret);

   position /= 1000;
   eo_do(pd->obj,
         eo_event_callback_call(ECORDOVA_MEDIA_EVENT_MEDIA_POSITION, &position));
}

static void
_set_position_action(Ecordova_Media_Data *pd, void *data)
{
   DBG("(%p)", pd->obj);

   int *position = data;
   int ret = player_set_play_position(pd->player,
                                      *position,
                                      false,
                                      _set_position_cb,
                                      pd);
   free(position);
   EINA_SAFETY_ON_FALSE_RETURN(PLAYER_ERROR_NONE == ret);
}

static void
_set_volume_action(Ecordova_Media_Data *pd, void *data)
{
   DBG("(%p)", pd->obj);

   double *volume = data;
   int ret = player_set_volume(pd->player, *volume, *volume);
   free(volume);
   EINA_SAFETY_ON_FALSE_RETURN(PLAYER_ERROR_NONE == ret);
   eo_do(pd->obj, eo_event_callback_call(ECORDOVA_MEDIA_EVENT_MEDIA_SUCCESS, NULL));
}

static void
_update_status(Ecordova_Media_Data *pd)
{
   player_state_e state = PLAYER_STATE_NONE;

   int ret = player_get_state(pd->player, &state);
   EINA_SAFETY_ON_FALSE_RETURN(PLAYER_ERROR_NONE == ret);

   Ecordova_Media_Status new_status = ECORDOVA_MEDIA_STATUS_MEDIA_NONE;
   switch (state)
     {
      case PLAYER_STATE_NONE:
        new_status = ECORDOVA_MEDIA_STATUS_MEDIA_NONE;
        break;
      case PLAYER_STATE_IDLE:
        new_status = ECORDOVA_MEDIA_STATUS_MEDIA_STARTING;
        break;
      case PLAYER_STATE_READY:
        new_status = ECORDOVA_MEDIA_STATUS_MEDIA_STOPPED;
        break;
      case PLAYER_STATE_PLAYING:
        new_status = ECORDOVA_MEDIA_STATUS_MEDIA_RUNNING;
        break;
      case PLAYER_STATE_PAUSED:
        new_status = ECORDOVA_MEDIA_STATUS_MEDIA_PAUSED;
        break;
     }

   if (new_status == pd->status)
     return;

   pd->status = new_status;
   eo_do(pd->obj, eo_event_callback_call(ECORDOVA_MEDIA_EVENT_MEDIA_STATUS, &pd->status));
}

static void
_release(Ecordova_Media_Data *pd)
{
   if (pd->player)
     {
        player_unset_error_cb(pd->player);
        player_unset_completed_cb(pd->player);
        if (ECORDOVA_MEDIA_STATUS_MEDIA_NONE != pd->status)
          player_unprepare(pd->player);
        player_destroy(pd->player);
        pd->player = NULL;
     }

   if (pd->recorder)
     {
        recorder_unset_state_changed_cb(pd->recorder);
        recorder_state_e state = RECORDER_STATE_NONE;
        if (RECORDER_ERROR_NONE == recorder_get_state(pd->recorder, &state) &&
            RECORDER_STATE_NONE != state && RECORDER_STATE_CREATED != state)
          recorder_unprepare(pd->recorder);
        recorder_destroy(pd->recorder);
        pd->recorder = NULL;
     }

   Action *action;
   EINA_LIST_FREE(pd->pending, action)
     {
        free(action->data);
        free(action);
     }
}

static void
_state_changed_cb(recorder_state_e previous,
                  recorder_state_e current,
                  bool by_policy EINA_UNUSED,
                  void *data)
{
   Ecordova_Media_Data *pd = data;
   if (RECORDER_STATE_READY == current && pd->record_pending)
     {
        _start_record(pd);
        return;
     }

   if (RECORDER_STATE_RECORDING == current)
     pd->record_pending = false;

   // TODO: Check what thread this callback is running to make sure it's on the main thread
   if (RECORDER_STATE_RECORDING == previous && RECORDER_STATE_READY == current)
     eo_do(pd->obj, eo_event_callback_call(ECORDOVA_MEDIA_EVENT_MEDIA_SUCCESS, NULL));
}

static void
_start_record(Ecordova_Media_Data *pd)
{
   int ret = recorder_start(pd->recorder);
   EINA_SAFETY_ON_FALSE_RETURN(RECORDER_ERROR_NONE == ret);
}

#include "ecordova_media.eo.c"
