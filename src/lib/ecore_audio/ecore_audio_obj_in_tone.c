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
#include <math.h>

EAPI Eo_Op ECORE_AUDIO_OBJ_IN_TONE_BASE_ID = EO_NOOP;

#define MY_CLASS ECORE_AUDIO_OBJ_IN_TONE_CLASS
#define MY_CLASS_NAME "ecore_audio_obj_in_tone"

struct _Ecore_Audio_Tone
{
  int freq;
  int phase;
};

typedef struct _Ecore_Audio_Tone Ecore_Audio_Tone;

static void _read(Eo *eo_obj, void *_pd, va_list *list)
{
  int i, remain;
  Ecore_Audio_Tone *obj = _pd;
  Ecore_Audio_Input *in_obj = eo_data_get(eo_obj, ECORE_AUDIO_OBJ_IN_CLASS);


  void *data = va_arg(*list, void *);
  int len = va_arg(*list, int);
  int *ret = va_arg(*list, int *);

  float *val = data;

  remain = in_obj->length * in_obj->samplerate * 4 - obj->phase * 4;
  if (remain > len)
    remain = len;

  for (i=0; i<remain/4; i++) {
      val[i] = sin(2* M_PI * obj->freq * (obj->phase + i) / in_obj->samplerate);
  }

  obj->phase += i;

  if (ret)
    *ret = remain;
}

static void _seek(Eo *eo_obj, void *_pd, va_list *list)
{
  int tmp;
  Ecore_Audio_Tone *obj = _pd;
  Ecore_Audio_Input *in_obj = eo_data_get(eo_obj, ECORE_AUDIO_OBJ_IN_CLASS);

  double offs = va_arg(*list, double);
  int mode = va_arg(*list, int);
  double *ret = va_arg(*list, double *);

  switch (mode) {
    case SEEK_SET:
      tmp = offs * in_obj->samplerate;
      break;
    case SEEK_CUR:
      tmp = obj->phase + offs * in_obj->samplerate;
      break;
    case SEEK_END:
      tmp = (in_obj->length + offs) * in_obj->samplerate;
      break;
    default:
      goto err;
  }
  if ((tmp < 0) || (tmp > in_obj->length * in_obj->samplerate))
    goto err;

  obj->phase = tmp;

  if (ret)
    *ret = (double)obj->phase / in_obj->samplerate;

err:
  if (ret)
    *ret = -1.0;
}

static void _source_set(Eo *eo_obj, void *_pd, va_list *list)
{
  Ecore_Audio_Tone *obj = _pd;

  Ecore_Audio_Object *ea_obj = eo_data_get(eo_obj, ECORE_AUDIO_OBJ_CLASS);
  Ecore_Audio_Input *in_obj = eo_data_get(eo_obj, ECORE_AUDIO_OBJ_IN_CLASS);

  const char *source = va_arg(*list, const char *);

  eina_stringshare_replace(&ea_obj->source, source);

  if (!ea_obj->source)
    return;

  ea_obj->format = ECORE_AUDIO_FORMAT_AUTO;
}

static void _source_get(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
  Ecore_Audio_Object *obj = eo_data_get(eo_obj, ECORE_AUDIO_OBJ_CLASS);

  const char **ret = va_arg(*list, const char **);

  if (ret)
    *ret = obj->source;
}

static void _format_set(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
  Ecore_Audio_Object *ea_obj = eo_data_get(eo_obj, ECORE_AUDIO_OBJ_CLASS);

  Ecore_Audio_Format format= va_arg(*list, Ecore_Audio_Format);

  if (ea_obj->source) {
      ERR("Input is already open - cannot change format");
      return;
  }

  switch (format) {
    default:
      ERR("Format not supported!");
      return;
  }
  ea_obj->format = format;
}

static void _format_get(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
  Ecore_Audio_Object *obj = eo_data_get(eo_obj, ECORE_AUDIO_OBJ_CLASS);

  Ecore_Audio_Format *ret = va_arg(*list, Ecore_Audio_Format *);

  if (ret)
    *ret = obj->format;
}

static void _constructor(Eo *eo_obj, void *_pd, va_list *list EINA_UNUSED)
{
  Ecore_Audio_Tone *obj = _pd;
  Ecore_Audio_Input *in_obj = eo_data_get(eo_obj, ECORE_AUDIO_OBJ_IN_CLASS);

  eo_do_super(eo_obj, MY_CLASS, eo_constructor());

  in_obj->channels = 1;
  in_obj->samplerate = 44100;
  in_obj->length = 1;

  obj->freq = 1000;
}

static void _class_constructor(Eo_Class *klass)
{
  const Eo_Op_Func_Description func_desc[] = {
      /* Virtual functions of parent class implemented in this class */
      EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
      //EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _destructor),

      EO_OP_FUNC(ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_SOURCE_SET), _source_set),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_SOURCE_GET), _source_get),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_FORMAT_SET), _format_set),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_FORMAT_GET), _format_get),

      EO_OP_FUNC(ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_SEEK), _seek),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_READ_INTERNAL), _read),

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
    EO_CLASS_DESCRIPTION_OPS(&ECORE_AUDIO_OBJ_IN_TONE_BASE_ID, op_desc, ECORE_AUDIO_OBJ_IN_TONE_SUB_ID_LAST),
    NULL,
    sizeof(Ecore_Audio_Tone),
    _class_constructor,
    NULL
};

EO_DEFINE_CLASS(ecore_audio_obj_in_tone_class_get, &class_desc, ECORE_AUDIO_OBJ_IN_CLASS, NULL);
