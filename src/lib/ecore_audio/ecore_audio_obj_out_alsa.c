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
#include "ecore_audio_obj_out_alsa.h"
#include <alsa/asoundlib.h>

#include <ctype.h>
#include <errno.h>

EAPI Eo_Op ECORE_AUDIO_OBJ_OUT_ALSA_BASE_ID = EO_NOOP;

#define MY_CLASS ECORE_AUDIO_OBJ_OUT_ALSA_CLASS
#define MY_CLASS_NAME "ecore_audio_obj_out_alsa"


struct _Ecore_Audio_Alsa
{
  char *foo;
};

typedef struct _Ecore_Audio_Alsa Ecore_Audio_Alsa;

struct _Ecore_Audio_Alsa_Data
{
  snd_pcm_t *handle;
  Ecore_Idler *idler;
};

typedef struct _Ecore_Audio_Alsa_Data Ecore_Audio_Alsa_Data;


static Eina_Bool _close_cb(void *data)
{
  snd_pcm_t *handle = data;
  snd_pcm_close(handle);

  return EINA_FALSE;
}

static void _delayed_close(snd_pcm_t *handle)
{
  ecore_timer_add(2, _close_cb, handle);
}

static void _volume_set(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
  Eo *in;
  Ecore_Audio_Alsa_Data *alsa;
  Eina_List *input;
  Ecore_Audio_Output *out_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_OUT_CLASS);

  double volume = va_arg(*list, double);

  if (volume < 0)
    volume = 0;

  eo_do_super(eo_obj, MY_CLASS, ecore_audio_obj_volume_set(volume));

  EINA_LIST_FOREACH(out_obj->inputs, input, in) {
      eo_do(in, eo_base_data_get("alsa_data", (void **)&alsa));
      /* FIXME: Volume control */
  }

}

static Eina_Bool _write_cb(void *data)
{
  Eo *in = data;

  Ecore_Audio_Alsa_Data *alsa;
  void *buf;

  size_t avail;
  ssize_t bread;
  int ret, channels;

  eo_do(in, eo_base_data_get("alsa_data", (void **)&alsa));

  eo_do(in, ecore_audio_obj_in_channels_get(&channels));

  avail = snd_pcm_avail(alsa->handle);

  if (avail < 1000)
    return EINA_TRUE;

  buf = calloc(1, sizeof(float) * avail * channels);

  eo_do(in, ecore_audio_obj_in_read(buf, avail*channels, &bread));

  ERR("ALSA: avail %i, read %i", avail, bread/channels);

  if (bread == 0)
    goto end;

  ret = snd_pcm_writei(alsa->handle, buf, bread/channels);
  if (ret < 0)
    {
      ERR("Error when writing: %s (avail %i, read %i)", snd_strerror(ret), avail, bread);
      snd_pcm_recover(alsa->handle, ret, 0);
    }

  if (bread<avail*channels)
    snd_pcm_start(alsa->handle);

  ERR("Success: written %i", ret);
end:
  free(buf);
  return EINA_TRUE;
}

static Eina_Bool _update_samplerate_cb(void *data EINA_UNUSED, Eo *eo_obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
  Ecore_Audio_Alsa_Data *alsa;
  int samplerate, channels;
  int ret;
  double speed;

  eo_do(eo_obj, ecore_audio_obj_in_samplerate_get(&samplerate));
  eo_do(eo_obj, ecore_audio_obj_in_channels_get(&channels));
  eo_do(eo_obj, ecore_audio_obj_in_speed_get(&speed));

  samplerate *= speed;

  eo_do(eo_obj, eo_base_data_get("alsa_data", (void **)&alsa));

  ret = snd_pcm_set_params(alsa->handle, SND_PCM_FORMAT_FLOAT, SND_PCM_ACCESS_RW_INTERLEAVED, channels, samplerate, 1, 500000);

  return EINA_TRUE;
}

static Eina_Bool _input_attach_internal(Eo *eo_obj, Eo *in)
{
  Ecore_Audio_Alsa_Data *alsa;
  const char *name;
  double speed;
  unsigned int samplerate, channels;
  int ret;
  Eina_Bool success;

  Ecore_Audio_Object *ea_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_CLASS);

  eo_do_super(eo_obj, MY_CLASS, ecore_audio_obj_out_input_attach(in, &success));
  if (!success)
    return EINA_FALSE;

  eo_do(in, ecore_audio_obj_in_samplerate_get((int *)&samplerate));
  eo_do(in, ecore_audio_obj_in_speed_get(&speed));
  eo_do(in, ecore_audio_obj_in_channels_get((int *)&channels));
  eo_do(in, ecore_audio_obj_name_get(&name));

  samplerate *= speed;

  alsa = calloc(1, sizeof(Ecore_Audio_Alsa_Data));

  ret = snd_pcm_open(&alsa->handle, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
  if (ret < 0)
    {
      ERR("Could not open for playback: %s", snd_strerror(ret));
      ERR("Could not create stream");
      free(alsa);
      eo_do_super(eo_obj, MY_CLASS, ecore_audio_obj_out_input_detach(in, NULL));
      return EINA_FALSE;
    }

  ret = snd_pcm_set_params(alsa->handle, SND_PCM_FORMAT_FLOAT, SND_PCM_ACCESS_RW_INTERLEAVED, channels, samplerate, 1, 500000);
  if (ret < 0)
    {
      free(alsa);
      ERR("Could not set parameters: %s", snd_strerror(ret));
      snd_pcm_close(alsa->handle);
      return EINA_FALSE;
    }

  eo_do(in, eo_event_callback_add(ECORE_AUDIO_EV_IN_SAMPLERATE_CHANGED, _update_samplerate_cb, eo_obj));

  eo_do(in, eo_base_data_set("alsa_data", alsa, free));

  if (ea_obj->paused)
     return EINA_TRUE;

  alsa->idler = ecore_idler_add(_write_cb, in);

  return EINA_TRUE;
}

static void _input_attach(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
  Eina_Bool retval = EINA_TRUE;

  Eo *in = va_arg(*list, Eo *);
  Eina_Bool *ret = va_arg(*list, Eina_Bool *);

  retval = _input_attach_internal(eo_obj, in);
  if (ret)
    *ret = retval;
}

static void _input_detach(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
  Ecore_Audio_Alsa_Data *alsa;
  Eina_Bool ret2;

  Eo *in = va_arg(*list, Eo *);
  Eina_Bool *ret = va_arg(*list, Eina_Bool *);

  if (ret)
    *ret = EINA_FALSE;

  eo_do_super(eo_obj, MY_CLASS, ecore_audio_obj_out_input_detach(in, &ret2));
  if (!ret2)
    return;

  eo_do(in, eo_base_data_get("alsa_data", (void **)&alsa));

  ecore_idler_del(alsa->idler);

  _delayed_close(alsa->handle);

  eo_do(in, eo_base_data_del("alsa_data"));

  if (ret)
    *ret = EINA_TRUE;
}

static void _constructor(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
  Ecore_Audio_Output *out_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_OUT_CLASS);

  eo_do_super(eo_obj, MY_CLASS, eo_constructor());

  out_obj->need_writer = EINA_FALSE;
}

static void _destructor(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
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

static const Eo_Class_Description class_desc = {
    EO_VERSION,
    MY_CLASS_NAME,
    EO_CLASS_TYPE_REGULAR,
    EO_CLASS_DESCRIPTION_OPS(&ECORE_AUDIO_OBJ_OUT_ALSA_BASE_ID, op_desc, ECORE_AUDIO_OBJ_OUT_ALSA_SUB_ID_LAST),
    NULL,
    sizeof(Ecore_Audio_Alsa),
    _class_constructor,
    NULL
};

EO_DEFINE_CLASS(ecore_audio_obj_out_alsa_class_get, &class_desc, ECORE_AUDIO_OBJ_OUT_CLASS, NULL);
