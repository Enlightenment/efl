#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libgen.h>

#ifdef HAVE_FEATURES_H
#include <features.h>
#endif

#include <Eo.h>
#include "ecore_audio_private.h"
#include <pulse/pulseaudio.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

extern pa_mainloop_api functable;

#define MY_CLASS ECORE_AUDIO_OUT_PULSE_CLASS
#define MY_CLASS_NAME "Ecore_Audio_Out_Pulse"

struct _Ecore_Audio_Out_Pulse_Data
{
   pa_mainloop_api *api;
   pa_context *context;
   pa_context_state_t state;
   Ecore_Job *state_job;
   Eina_List *outputs;
};

typedef struct _Ecore_Audio_Out_Pulse_Data Ecore_Audio_Out_Pulse_Data;

EOLIAN static void
_ecore_audio_out_pulse_ecore_audio_volume_set(Eo *eo_obj, Ecore_Audio_Out_Pulse_Data *pd, double volume)
{
  Eo *in;
  pa_stream *stream = NULL;
  Eina_List *input;
  uint32_t idx;
  pa_cvolume pa_volume;
  Ecore_Audio_Output *out_obj = efl_data_scope_get(eo_obj, ECORE_AUDIO_OUT_CLASS);

  if (!EPA_LOAD()) return;
  if (volume < 0)
    volume = 0;

  EPA_CALL(pa_cvolume_set)(&pa_volume, 2, volume * PA_VOLUME_NORM);

  ecore_audio_obj_volume_set(efl_super(eo_obj, MY_CLASS), volume);

  EINA_LIST_FOREACH(out_obj->inputs, input, in) {
      stream = efl_key_data_get(in, "pulse_data");
      idx = EPA_CALL(pa_stream_get_index)(stream);
      EPA_CALL(pa_operation_unref)(EPA_CALL(pa_context_set_sink_input_volume)(pd->context, idx, &pa_volume, NULL, NULL));
  }
}

static void _write_cb(pa_stream *stream, size_t len, void *data)
{
  Eo *in = data;

  void *buf;
  ssize_t bread = 0;
  size_t wlen = len;

  if (!EPA_LOAD()) return;
  EPA_CALL(pa_stream_begin_write)(stream, &buf, &wlen);

  bread = ecore_audio_obj_in_read(in, buf, wlen);

  if ((bread < (int)len) && bread)
    {
      memset((char *)buf + bread, 0, wlen - bread);
      EPA_CALL(pa_stream_write)(stream, buf, wlen, NULL, 0, PA_SEEK_RELATIVE);
      EPA_CALL(pa_operation_unref)(EPA_CALL(pa_stream_trigger)(stream, NULL, NULL));
    }
  else
    EPA_CALL(pa_stream_write)(stream, buf, bread, NULL, 0, PA_SEEK_RELATIVE);
}

static void _update_samplerate_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
  pa_stream *stream = NULL;
  int samplerate = 0;
  double speed = 0;

  samplerate = ecore_audio_obj_in_samplerate_get(event->object);
  speed = ecore_audio_obj_in_speed_get(event->object);

  stream = efl_key_data_get(event->object, "pulse_data");

  EPA_CALL(pa_operation_unref)(EPA_CALL(pa_stream_update_sample_rate)(stream, samplerate * speed, NULL, NULL));
}

static Eina_Bool _input_attach_internal(Eo *eo_obj, Eo *in)
{
  const char *name = NULL;
  pa_sample_spec ss;
  double speed = 0;
  pa_stream *stream;
  Eina_Bool ret = EINA_FALSE;
  Ecore_Audio_Object *ea_obj = efl_data_scope_get(eo_obj, ECORE_AUDIO_CLASS);
  Ecore_Audio_Out_Pulse_Data *pd = efl_data_scope_get(eo_obj, MY_CLASS);

  if (!EPA_LOAD()) return EINA_FALSE;
  ret = ecore_audio_obj_out_input_attach(efl_super(eo_obj, MY_CLASS), in);
  if (!ret)
    return EINA_FALSE;

  ss.format = PA_SAMPLE_FLOAT32LE;
  ss.rate = ecore_audio_obj_in_samplerate_get(in);
  speed = ecore_audio_obj_in_speed_get(in);
  ss.channels = ecore_audio_obj_in_channels_get(in);
  name = efl_name_get(in);

  ss.rate = ss.rate * speed;

  stream = EPA_CALL(pa_stream_new)(pd->context, name, &ss, NULL);
  if (!stream) {
      ERR("Could not create stream");
      ecore_audio_obj_out_input_detach(efl_super(eo_obj, MY_CLASS), in);
      return EINA_FALSE;
  }

  efl_event_callback_add(in, ECORE_AUDIO_IN_EVENT_IN_SAMPLERATE_CHANGED, _update_samplerate_cb, eo_obj);

  efl_key_data_set(in, "pulse_data", stream);


  EPA_CALL(pa_stream_set_write_callback)(stream, _write_cb, in);
  EPA_CALL(pa_stream_connect_playback)(stream, NULL, NULL, PA_STREAM_VARIABLE_RATE, NULL, NULL);

  if (ea_obj->paused)
    EPA_CALL(pa_operation_unref)(EPA_CALL(pa_stream_cork)(stream, 1, NULL, NULL));

  return ret;
}

static void _delayed_attach_cb(void *data, const Efl_Event *event)
{
  efl_event_callback_del(event->object, ECORE_AUDIO_OUT_PULSE_EVENT_CONTEXT_READY, _delayed_attach_cb, data);

  _input_attach_internal(event->object, data);
}

static Eina_Bool
_is_input_attached(Eo *eo_obj, Eo *in)
{
   Ecore_Audio_Output *out_obj = efl_data_scope_get(eo_obj, ECORE_AUDIO_OUT_CLASS);
   if (!out_obj->inputs) return EINA_FALSE;
   return !!eina_list_data_find(out_obj->inputs, in);
}

EOLIAN static Eina_Bool
_ecore_audio_out_pulse_ecore_audio_out_input_attach(Eo *eo_obj, Ecore_Audio_Out_Pulse_Data *pd, Eo *in)
{
  Eina_Bool retval = EINA_TRUE;

  if (_is_input_attached(eo_obj, in)) return EINA_TRUE;

  if (pd->state != PA_CONTEXT_READY) {
    DBG("Delaying input_attach because PA context is not ready.");
    efl_event_callback_add(eo_obj, ECORE_AUDIO_OUT_PULSE_EVENT_CONTEXT_READY, _delayed_attach_cb, in);
  } else {
    retval = _input_attach_internal(eo_obj, in);
  }

  return retval;
}

static void _drain_cb(pa_stream *stream, int success EINA_UNUSED, void *data EINA_UNUSED)
{
  if (!EPA_LOAD()) return;
  EPA_CALL(pa_stream_disconnect)(stream);
  EPA_CALL(pa_stream_unref)(stream);
}

EOLIAN static Eina_Bool
_ecore_audio_out_pulse_ecore_audio_out_input_detach(Eo *eo_obj, Ecore_Audio_Out_Pulse_Data *pd, Eo *in)
{
  pa_stream *stream = NULL;
  Eina_Bool ret2 = EINA_FALSE;
  pa_operation *op;

  if (!EPA_LOAD())
    {
       ERR("Failed to load PA!");
       return EINA_FALSE;
    }
  if (!_is_input_attached(eo_obj, in))
    {
       ERR("Input object passed is not currently attached to this output!");
       return EINA_FALSE;
    }
  if (pd->state != PA_CONTEXT_READY)
    efl_event_callback_del(in, ECORE_AUDIO_OUT_PULSE_EVENT_CONTEXT_READY, _delayed_attach_cb, pd);
  else
    efl_event_callback_del(in, ECORE_AUDIO_IN_EVENT_IN_SAMPLERATE_CHANGED, _update_samplerate_cb, eo_obj);
  ret2 = ecore_audio_obj_out_input_detach(efl_super(eo_obj, MY_CLASS), in);
  if (!ret2)
    {
       ERR("Super call failed for ecore_audio_obj_out_input_detach!");
       return EINA_FALSE;
    }

  stream = efl_key_data_get(in, "pulse_data");

  EPA_CALL(pa_stream_set_write_callback)(stream, NULL, NULL);
  op = EPA_CALL(pa_stream_drain) (stream, _drain_cb, NULL);
  if (!op)
    {
       op = EPA_CALL(pa_stream_flush)(stream, _drain_cb, NULL);
       if (!op)
         {
            EPA_CALL(pa_stream_disconnect)(stream);
            EPA_CALL(pa_stream_unref)(stream);
         }
       return EINA_FALSE;
    }

  EPA_CALL(pa_operation_unref)(op);
  return EINA_TRUE;
}

static void _state_cb(pa_context *context, void *data)
{
   Eina_List *out, *tmp;
   Eo *eo_obj;
   pa_context_state_t state;
   Ecore_Audio_Out_Pulse_Data *pd = data;

   if (!EPA_LOAD()) return;
   state = EPA_CALL(pa_context_get_state)(context);
   pd->state = state;

   //ref everything in the list to be sure...
   EINA_LIST_FOREACH(pd->outputs, out, eo_obj) {
      efl_ref(eo_obj);
   }
   // the callback here can delete things in the list..
   if (state == PA_CONTEXT_READY) {
      DBG("PA context ready.");
      EINA_LIST_FOREACH(pd->outputs, out, eo_obj) {
         efl_event_callback_call(eo_obj, ECORE_AUDIO_OUT_PULSE_EVENT_CONTEXT_READY, NULL);
      }
   } else if ((state == PA_CONTEXT_FAILED) || (state == PA_CONTEXT_TERMINATED)) {
      DBG("PA context fail.");
      EINA_LIST_FOREACH(pd->outputs, out, eo_obj) {
         efl_event_callback_call(eo_obj, ECORE_AUDIO_OUT_PULSE_EVENT_CONTEXT_FAIL, NULL);
      }
   } else {
      DBG("Connection state %i", state);
   }
   // now unref everything safely
   EINA_LIST_FOREACH_SAFE(pd->outputs, out, tmp, eo_obj) {
      efl_unref(eo_obj);
   }
}

static void _state_job(void *data)
{
   Ecore_Audio_Out_Pulse_Data *pd = data;
   if ((pd->state == PA_CONTEXT_FAILED) ||
       (pd->state == PA_CONTEXT_TERMINATED))
     {
        Eo *eo_obj;
        Eina_List *out, *tmp;

        DBG("PA context fail.");
        //ref everything in the list to be sure...
        EINA_LIST_FOREACH(pd->outputs, out, eo_obj) {
           efl_ref(eo_obj);
        }
        // the callback here can delete things in the list..
        EINA_LIST_FOREACH(pd->outputs, out, eo_obj) {
           efl_event_callback_call(eo_obj, ECORE_AUDIO_OUT_PULSE_EVENT_CONTEXT_FAIL, NULL);
        }
        // now unref everything safely
        EINA_LIST_FOREACH_SAFE(pd->outputs, out, tmp, eo_obj) {
           efl_unref(eo_obj);
        }
     }
   pd->state_job = NULL;
}

EOLIAN static Eo *
_ecore_audio_out_pulse_efl_object_constructor(Eo *eo_obj, Ecore_Audio_Out_Pulse_Data *pd)
{
  int argc;
  char **argv, *disp = NULL;
  Ecore_Audio_Output *out_obj = efl_data_scope_get(eo_obj, ECORE_AUDIO_OUT_CLASS);
  static char *dispenv = NULL;

  if (!EPA_LOAD()) return NULL;
  eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));
  pd->api = &functable;

  out_obj->need_writer = EINA_FALSE;

  if (!pd->context) {

    // if we're in a wayland world rather than x11... but DISPLAY also set...
    if (getenv("WAYLAND_DISPLAY")) disp = getenv("DISPLAY");
    // make a tmp copy of display locally as we'll overwrite this
    if (disp) disp = strdup(disp);
    // if we had a previously allocated env var buffer for DISPLAY then
    // free it only if DISPLAY env var changed
    if (dispenv) {
      if (!((disp) && (!strcmp(dispenv + 8/*"DISPLAY="*/, disp)))) {
        free(dispenv);
        dispenv = NULL;
      }
    }
    // no previous display env but we have a display, then allocate a buffer
    // that stays around until the next time here with the evn var string
    // but have space for disp string too
    if ((!dispenv) && (disp)) {
      dispenv = malloc(8/*"DISPLAY="*/ + strlen(disp) + 1);
    }
    // ensure env var is empty and to a putenv as pulse wants to use DISPLAY
    // and if its non-empty it'll try connect to the xserver and we do not
    // want this to happen in a wayland universe
    if (dispenv) {
      strcpy(dispenv, "DISPLAY=");
      putenv(dispenv);
    }
    // now hopefully getenv("DISPLAY") inside pulse will return NULL or it
    // will return an empty string "" which pulse thinsk is the same as NULL

    ecore_app_args_get(&argc, &argv);
    if (!argc) {
        DBG("Could not get program name, pulse outputs will be named ecore_audio");
       pd->context = EPA_CALL(pa_context_new)(pd->api, "ecore_audio");
    } else {
       pd->context = EPA_CALL(pa_context_new)(pd->api, basename(argv[0]));
    }
    // if we had a display value and a displayenv buffer then let's restore
    // the previous value content of DISPLAY as we duplicated it above and
    // add to the env of the dispenv buffer, then putenv that back. as the
    // buffer is malloced this will be safe, but as the displayenv is local
    // and static we wont go allocating these buffers forever. just this one
    // here and then replace/re-use it.
    if ((disp) && (dispenv)) {
      strcat(dispenv, disp);
      putenv(dispenv);
    }
    // free up our temporary local DISPLAY env sring copy if we have it
    if (disp) free(disp);

    EPA_CALL(pa_context_set_state_callback)(pd->context, _state_cb, pd);
    EPA_CALL(pa_context_connect)(pd->context, NULL, PA_CONTEXT_NOFLAGS, NULL);
  }

  pd->outputs = eina_list_append(pd->outputs, eo_obj);
  pd->state_job = ecore_job_add(_state_job, pd);

  return eo_obj;
}

EOLIAN static void
_ecore_audio_out_pulse_efl_object_destructor(Eo *eo_obj, Ecore_Audio_Out_Pulse_Data *pd)
{
  pd->outputs = eina_list_remove(pd->outputs, eo_obj);
  ecore_job_del(pd->state_job);
  EPA_CALL(pa_context_unref)(pd->context);
  efl_destructor(efl_super(eo_obj, MY_CLASS));
}

#include "ecore_audio_out_pulse.eo.c"
