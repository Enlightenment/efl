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
#define MY_CLASS_NAME "Ecore_Audio_In_Tone"

struct _Ecore_Audio_Tone
{
  int freq;
  int phase;
};

typedef struct _Ecore_Audio_Tone Ecore_Audio_Tone;

static void _tone_read(Eo *eo_obj, void *_pd, va_list *list)
{
  int i, remain;
  Ecore_Audio_Tone *obj = _pd;
  Ecore_Audio_Input *in_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_IN_CLASS);


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
  Ecore_Audio_Input *in_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_IN_CLASS);

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

  return;
err:
  if (ret)
    *ret = -1.0;
}

static void _length_set(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
  Ecore_Audio_Input *in_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_IN_CLASS);

  double length= va_arg(*list, double);

  in_obj->length = length;
}

static void _data_set(Eo *eo_obj, void *_pd, va_list *list)
{
  Ecore_Audio_Tone *obj = _pd;

  const char *key = va_arg(*list, const char *);
  const void *val = va_arg(*list, const void *);
  eo_base_data_free_func func = va_arg(*list, eo_base_data_free_func);

  if (!key) return;

  if (!strcmp(key, ECORE_AUDIO_ATTR_TONE_FREQ)) {
      obj->freq = *(int *)val;
  } else {
      eo_do_super(eo_obj, MY_CLASS, eo_base_data_set(key, val, func));
  }

}

static void _data_get(Eo *eo_obj, void *_pd, va_list *list)
{
  Ecore_Audio_Tone *obj = _pd;

  const char *key = va_arg(*list, const char*);
  void **ret = va_arg(*list, void **);

  if (!strcmp(key, ECORE_AUDIO_ATTR_TONE_FREQ)) {
      if (ret)
        *(int *)ret = obj->freq;
  } else {
      eo_do_super(eo_obj, MY_CLASS, eo_base_data_get(key, ret));
  }

}

static void _constructor(Eo *eo_obj, void *_pd, va_list *list EINA_UNUSED)
{
  Ecore_Audio_Tone *obj = _pd;
  Ecore_Audio_Input *in_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_IN_CLASS);

  eo_do_super(eo_obj, MY_CLASS, eo_constructor());

  in_obj->channels = 1;
  in_obj->samplerate = 44100;
  in_obj->length = 1;
  in_obj->seekable = EINA_TRUE;

  obj->freq = 1000;
}

static void _class_constructor(Eo_Class *klass)
{
  const Eo_Op_Func_Description func_desc[] = {
      /* Virtual functions of parent class implemented in this class */
      EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
      //EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _destructor),

      EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DATA_GET), _data_get),
      EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DATA_SET), _data_set),

      EO_OP_FUNC(ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_LENGTH_SET), _length_set),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_SEEK), _seek),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_READ_INTERNAL), _tone_read),

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
