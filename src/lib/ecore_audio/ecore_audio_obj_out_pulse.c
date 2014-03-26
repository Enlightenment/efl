#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

EAPI Eo_Op ECORE_AUDIO_OBJ_OUT_PULSE_BASE_ID = EO_NOOP;

#define MY_CLASS ECORE_AUDIO_OBJ_OUT_PULSE_CLASS
#define MY_CLASS_NAME "Ecore_Audio_Out_Pulse"

EAPI const Eo_Event_Description _ECORE_AUDIO_EV_OUT_PULSE_CONTEXT_READY =
  EO_EVENT_DESCRIPTION("context,ready", "Called when the output is ready for playback.");
#define ECORE_AUDIO_EV_OUT_PULSE_CONTEXT_READY (&(_ECORE_AUDIO_EV_OUT_PULSE_CONTEXT_READY))
EAPI const Eo_Event_Description _ECORE_AUDIO_EV_OUT_PULSE_CONTEXT_FAIL =
  EO_EVENT_DESCRIPTION("context,fail", "Called when context fails.");
#define ECORE_AUDIO_EV_OUT_PULSE_CONTEXT_FAIL (&(_ECORE_AUDIO_EV_OUT_PULSE_CONTEXT_FAIL))


struct _Ecore_Audio_Pulse_Class {
  pa_mainloop_api *api;
  pa_context *context;
  pa_context_state_t state;
  Ecore_Job *state_job;
  Eina_List *outputs;
};

static struct _Ecore_Audio_Pulse_Class class_vars = {
    .api = &functable,
};

struct _Ecore_Audio_Pulse
{
  char *foo;
};

typedef struct _Ecore_Audio_Pulse Ecore_Audio_Pulse;

static void _volume_set(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
  Eo *in;
  pa_stream *stream;
  Eina_List *input;
  uint32_t idx;
  pa_cvolume pa_volume;
  Ecore_Audio_Output *out_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_OUT_CLASS);

  double volume = va_arg(*list, double);

  if (volume < 0)
    volume = 0;

  pa_cvolume_set(&pa_volume, 2, volume * PA_VOLUME_NORM);

  eo_do_super(eo_obj, MY_CLASS, ecore_audio_obj_volume_set(volume));

  EINA_LIST_FOREACH(out_obj->inputs, input, in) {
      eo_do(in, eo_base_data_get("pulse_data", (void **)&stream));
      idx = pa_stream_get_index(stream);
      pa_operation_unref(pa_context_set_sink_input_volume(class_vars.context, idx, &pa_volume, NULL, NULL));
  }

}

static void _write_cb(pa_stream *stream, size_t len, void *data)
{
  Eo *in = data;

  void *buf;
  ssize_t bread;
  size_t wlen = len;

  pa_stream_begin_write(stream, &buf, &wlen);

  eo_do(in, ecore_audio_obj_in_read(buf, wlen, &bread));

  pa_stream_write(stream, buf, bread, NULL, 0, PA_SEEK_RELATIVE);
  if (bread < (int)len)
    {
      pa_operation_unref(pa_stream_trigger(stream, NULL, NULL));
    }
}

static Eina_Bool _update_samplerate_cb(void *data EINA_UNUSED, Eo *eo_obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
  pa_stream *stream;
  int samplerate;
  double speed;

  eo_do(eo_obj, ecore_audio_obj_in_samplerate_get(&samplerate));
  eo_do(eo_obj, ecore_audio_obj_in_speed_get(&speed));

  eo_do(eo_obj, eo_base_data_get("pulse_data", (void **)&stream));

  pa_operation_unref(pa_stream_update_sample_rate(stream, samplerate * speed, NULL, NULL));

  return EINA_TRUE;
}

static Eina_Bool _input_attach_internal(Eo *eo_obj, Eo *in)
{
  const char *name;
  pa_sample_spec ss;
  double speed;
  pa_stream *stream;
  Eina_Bool ret;
  Ecore_Audio_Object *ea_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_CLASS);

  eo_do_super(eo_obj, MY_CLASS, ecore_audio_obj_out_input_attach(in, &ret));
  if (!ret)
    return EINA_FALSE;

  ss.format = PA_SAMPLE_FLOAT32LE;
  eo_do(in, ecore_audio_obj_in_samplerate_get((int *)&ss.rate));
  eo_do(in, ecore_audio_obj_in_speed_get(&speed));
  eo_do(in, ecore_audio_obj_in_channels_get((int *)&ss.channels));
  eo_do(in, ecore_audio_obj_name_get(&name));

  ss.rate = ss.rate * speed;

  stream = pa_stream_new(class_vars.context, name, &ss, NULL);
  if (!stream) {
      ERR("Could not create stream");
      eo_do_super(eo_obj, MY_CLASS, ecore_audio_obj_out_input_detach(in, NULL));
      return EINA_FALSE;
  }

  eo_do(in, eo_event_callback_add(ECORE_AUDIO_IN_EVENT_IN_SAMPLERATE_CHANGED, _update_samplerate_cb, eo_obj));

  eo_do(in, eo_base_data_set("pulse_data", stream, NULL));


  pa_stream_set_write_callback(stream, _write_cb, in);
  pa_stream_connect_playback(stream, NULL, NULL, PA_STREAM_VARIABLE_RATE, NULL, NULL);

  if (ea_obj->paused)
    pa_operation_unref(pa_stream_cork(stream, 1, NULL, NULL));

  return ret;
}

static Eina_Bool _delayed_attach_cb(void *data, Eo *eo_obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
  Eo *in = data;
  eo_do(eo_obj, eo_event_callback_del(ECORE_AUDIO_EV_OUT_PULSE_CONTEXT_READY, _delayed_attach_cb, in));

  _input_attach_internal(eo_obj, in);

  return EINA_TRUE;
}

static void _input_attach(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
  Eina_Bool retval = EINA_TRUE;

  Eo *in = va_arg(*list, Eo *);
  Eina_Bool *ret = va_arg(*list, Eina_Bool *);

  if (class_vars.state != PA_CONTEXT_READY) {
    DBG("Delaying input_attach because PA context is not ready.");
    eo_do(eo_obj, eo_event_callback_add(ECORE_AUDIO_EV_OUT_PULSE_CONTEXT_READY, _delayed_attach_cb, in));
  } else {
    retval = _input_attach_internal(eo_obj, in);
  }
  if (ret)
    *ret = retval;
}

static void _drain_cb(pa_stream *stream, int success EINA_UNUSED, void *data EINA_UNUSED)
{
  pa_stream_disconnect(stream);
  pa_stream_unref(stream);
}

static void _input_detach(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
  pa_stream *stream;
  Eina_Bool ret2;

  Eo *in = va_arg(*list, Eo *);
  Eina_Bool *ret = va_arg(*list, Eina_Bool *);

  if (ret)
    *ret = EINA_FALSE;

  eo_do_super(eo_obj, MY_CLASS, ecore_audio_obj_out_input_detach(in, &ret2));
  if (!ret2)
    return;

  eo_do(in, eo_base_data_get("pulse_data", (void **)&stream));

  pa_stream_set_write_callback(stream, NULL, NULL);
  pa_operation_unref(pa_stream_drain(stream, _drain_cb, NULL));

  if (ret)
    *ret = EINA_TRUE;
}

static void _state_cb(pa_context *context, void *data EINA_UNUSED)
{
   Eina_List *out, *tmp;
   Eo *eo_obj;
   pa_context_state_t state;
   
   state = pa_context_get_state(context);
   class_vars.state = state;
   
   //ref everything in the list to be sure...
   EINA_LIST_FOREACH(class_vars.outputs, out, eo_obj) {
      eo_ref(eo_obj);
   }
   // the callback here can delete things in the list..
   if (state == PA_CONTEXT_READY) {
      DBG("PA context ready.");
      EINA_LIST_FOREACH(class_vars.outputs, out, eo_obj) {
         eo_do(eo_obj, eo_event_callback_call(ECORE_AUDIO_EV_OUT_PULSE_CONTEXT_READY, NULL, NULL));
      }
   } else if ((state == PA_CONTEXT_FAILED) || (state == PA_CONTEXT_TERMINATED)) {
      DBG("PA context fail.");
      EINA_LIST_FOREACH(class_vars.outputs, out, eo_obj) {
         eo_do(eo_obj, eo_event_callback_call(ECORE_AUDIO_EV_OUT_PULSE_CONTEXT_FAIL, NULL, NULL));
      }
   } else {
      DBG("Connection state %i", state);
   }
   // now unref everything safely
   EINA_LIST_FOREACH_SAFE(class_vars.outputs, out, tmp, eo_obj) {
      eo_unref(eo_obj);
   }
}

static void _state_job(void *data EINA_UNUSED)
{
   if ((class_vars.state == PA_CONTEXT_FAILED) ||
       (class_vars.state == PA_CONTEXT_TERMINATED))
     {
        Eo *eo_obj;
        Eina_List *out, *tmp;
        
        DBG("PA context fail.");
        //ref everything in the list to be sure...
        EINA_LIST_FOREACH(class_vars.outputs, out, eo_obj) {
           eo_ref(eo_obj);
        }
        // the callback here can delete things in the list..
        EINA_LIST_FOREACH(class_vars.outputs, out, eo_obj) {
           eo_do(eo_obj, eo_event_callback_call(ECORE_AUDIO_EV_OUT_PULSE_CONTEXT_FAIL, NULL, NULL));
        }
        // now unref everything safely
        EINA_LIST_FOREACH_SAFE(class_vars.outputs, out, tmp, eo_obj) {
           eo_unref(eo_obj);
        }
     }
   class_vars.state_job = NULL;
}

static void _constructor(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
  int argc;
  char **argv;
  Ecore_Audio_Output *out_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_OUT_CLASS);

  eo_do_super(eo_obj, MY_CLASS, eo_constructor());

  out_obj->need_writer = EINA_FALSE;

  if (!class_vars.context) {
    ecore_app_args_get(&argc, &argv);
    if (!argc) {
        DBG("Could not get program name, pulse outputs will be named ecore_audio");
       class_vars.context = pa_context_new(class_vars.api, "ecore_audio");
    } else {
       class_vars.context = pa_context_new(class_vars.api, basename(argv[0]));
    }
    pa_context_set_state_callback(class_vars.context, _state_cb, NULL);
    pa_context_connect(class_vars.context, NULL, PA_CONTEXT_NOFLAGS, NULL);
  }

  class_vars.outputs = eina_list_append(class_vars.outputs, eo_obj);
  if (class_vars.state_job) eo_del(class_vars.state_job);
  class_vars.state_job = ecore_job_add(_state_job, NULL);
}

static void _destructor(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
  class_vars.outputs = eina_list_remove(class_vars.outputs, eo_obj);
  eo_do_super(eo_obj, MY_CLASS, eo_destructor());
}

static void _class_constructor(Eo_Class *klass)
{
  const Eo_Op_Func_Description func_desc[] = {
      /* Virtual functions of parent class implemented in this class */
      EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
      EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _destructor),

      EO_OP_FUNC(ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_VOLUME_SET), _volume_set),

      EO_OP_FUNC(ECORE_AUDIO_OBJ_OUT_ID(ECORE_AUDIO_OBJ_OUT_SUB_ID_INPUT_ATTACH), _input_attach),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_OUT_ID(ECORE_AUDIO_OBJ_OUT_SUB_ID_INPUT_DETACH), _input_detach),
      EO_OP_FUNC_SENTINEL
  };

  eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
    EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Event_Description *event_desc[] = {
    ECORE_AUDIO_EV_OUT_PULSE_CONTEXT_READY,
    ECORE_AUDIO_EV_OUT_PULSE_CONTEXT_FAIL,
    NULL
};


static const Eo_Class_Description class_desc = {
    EO_VERSION,
    MY_CLASS_NAME,
    EO_CLASS_TYPE_REGULAR,
    EO_CLASS_DESCRIPTION_OPS(&ECORE_AUDIO_OBJ_OUT_PULSE_BASE_ID, op_desc, ECORE_AUDIO_OBJ_OUT_PULSE_SUB_ID_LAST),
    event_desc,
    sizeof(Ecore_Audio_Pulse),
    _class_constructor,
    NULL
};

EO_DEFINE_CLASS(ecore_audio_obj_out_pulse_class_get, &class_desc, ECORE_AUDIO_OBJ_OUT_CLASS, NULL);
