#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_FEATURES_H
#include <features.h>
#endif
#include <ctype.h>
#include <errno.h>

#include "ecore_audio_private.h"

EAPI Eo_Op ECORE_AUDIO_OBJ_IN_BASE_ID = EO_NOOP;

EAPI const Eo_Event_Description _ECORE_AUDIO_EV_IN_LOOPED =
         EO_EVENT_DESCRIPTION("in,looped", "Called when an input has looped.");
EAPI const Eo_Event_Description _ECORE_AUDIO_EV_IN_STOPPED =
         EO_EVENT_DESCRIPTION("in,stopped", "Called when an input has stopped playing.");
EAPI const Eo_Event_Description _ECORE_AUDIO_EV_IN_SAMPLERATE_CHANGED =
         EO_EVENT_DESCRIPTION("in,samplerate,changed", "Called when the input samplerate has changed.");

#define MY_CLASS ECORE_AUDIO_OBJ_IN_CLASS
#define MY_CLASS_NAME "Ecore_Audio_In"

static void _speed_set(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
  Ecore_Audio_Input *obj = _pd;

  double speed = va_arg(*list, double);

  if (speed < 0.2)
    speed = 0.2;
  if (speed > 5.0)
    speed = 5.0;

  obj->speed = speed;

  eo_do(eo_obj, eo_event_callback_call(ECORE_AUDIO_EV_IN_SAMPLERATE_CHANGED, NULL, NULL));
}

static void _speed_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
  const Ecore_Audio_Input *obj = _pd;

  double *speed = va_arg(*list, double *);

  if (speed)
    *speed = obj->speed;
}

static void _samplerate_set(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
  Ecore_Audio_Input *obj = _pd;

  int samplerate = va_arg(*list, int);

  obj->samplerate = samplerate;

  eo_do(eo_obj, eo_event_callback_call(ECORE_AUDIO_EV_IN_SAMPLERATE_CHANGED, NULL, NULL));
}

static void _samplerate_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
  const Ecore_Audio_Input *obj = _pd;

  int *samplerate = va_arg(*list, int *);

  if (samplerate)
    *samplerate = obj->samplerate;
}

static void _channels_set(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
  Ecore_Audio_Input *obj = _pd;

  int channels = va_arg(*list, int);

  obj->channels = channels;

  /* TODO: Notify output */

}

static void _channels_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
  const Ecore_Audio_Input *obj = _pd;

  int *channels = va_arg(*list, int *);

  if (channels)
    *channels = obj->channels;
}

static void _looped_set(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
  Ecore_Audio_Input *obj = _pd;

  Eina_Bool looped = va_arg(*list, int);

  obj->looped = looped;
}

static void _looped_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
  const Ecore_Audio_Input *obj = _pd;

  Eina_Bool *ret = va_arg(*list, Eina_Bool *);

  if (ret)
    *ret = obj->looped;
}

static void _length_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
  const Ecore_Audio_Input *obj = _pd;

  double *ret = va_arg(*list, double *);

  if (ret) {
    *ret = obj->length;
  }
}

static void _remaining_get(Eo *eo_obj, void *_pd, va_list *list)
{
  const Ecore_Audio_Input *obj = _pd;

  double *ret = va_arg(*list, double *);

  if (!obj->seekable && ret) {
      *ret = -1;
  } else if (ret) {
    eo_do(eo_obj, ecore_audio_obj_in_seek(0, SEEK_CUR, ret));
    *ret = obj->length - *ret;
  }
}

static void _in_read(Eo *eo_obj, void *_pd, va_list *list)
{
  const Ecore_Audio_Input *obj = _pd;
  ssize_t len_read = 0;
  const Ecore_Audio_Object *ea_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_CLASS);

  void *buf = va_arg(*list, void *);
  size_t len = va_arg(*list, size_t);
  ssize_t *ret = va_arg(*list, ssize_t *);

  if (ea_obj->paused) {
    memset(buf, 0, len);
    len_read = len;
  } else {
      eo_do(eo_obj, ecore_audio_obj_in_read_internal(buf, len, &len_read));
      if (len_read == 0) {
          if (!obj->looped || !obj->seekable) {
              eo_do(eo_obj, eo_event_callback_call(ECORE_AUDIO_EV_IN_STOPPED, NULL, NULL));
          } else {
              eo_do(eo_obj, ecore_audio_obj_in_seek(0, SEEK_SET, NULL));
              eo_do(eo_obj, ecore_audio_obj_in_read_internal(buf, len, &len_read));
              eo_do(eo_obj, eo_event_callback_call(ECORE_AUDIO_EV_IN_LOOPED, NULL, NULL));
          }
      }

  }

  if (ret)
    *ret = len_read;
}

static void _read_internal(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
  ssize_t len_read = 0;
  const Ecore_Audio_Object *ea_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_CLASS);

  void *buf = va_arg(*list, void *);
  size_t len = va_arg(*list, size_t);
  ssize_t *ret = va_arg(*list, ssize_t *);

  if (ea_obj->vio && ea_obj->vio->vio->read) {
      len_read = ea_obj->vio->vio->read(ea_obj->vio->data, eo_obj, buf, len);
  }

  if (ret)
    *ret = len_read;
}

static void _output_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
  const Ecore_Audio_Input *obj = _pd;

  Eo **ret = va_arg(*list, Eo **);

  if (ret)
    *ret = obj->output;
}

static void _free_vio(Ecore_Audio_Object *ea_obj)
{
  if (ea_obj->vio->free_func)
    ea_obj->vio->free_func(ea_obj->vio->data);

  free(ea_obj->vio);
  ea_obj->vio = NULL;
}

static void _vio_set(Eo *eo_obj, void *_pd, va_list *list)
{
  Ecore_Audio_Input *obj = _pd;
  Ecore_Audio_Object *ea_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_CLASS);

  Ecore_Audio_Vio *vio = va_arg(*list, Ecore_Audio_Vio *);
  void *data = va_arg(*list, Ecore_Audio_Vio *);
  eo_base_data_free_func free_func = va_arg(*list, eo_base_data_free_func);

  if (ea_obj->vio)
    _free_vio(ea_obj);

  obj->seekable = obj->seekable_prev;

  if (!vio)
    return;

  ea_obj->vio = calloc(1, sizeof(Ecore_Audio_Vio_Internal));
  ea_obj->vio->vio = vio;
  ea_obj->vio->data = data;
  ea_obj->vio->free_func = free_func;

  obj->seekable_prev = obj->seekable;
  obj->seekable = (vio->seek != NULL);
}

static void _constructor(Eo *eo_obj, void *_pd, va_list *list EINA_UNUSED)
{
  Ecore_Audio_Input *obj = _pd;

  eo_do_super(eo_obj, MY_CLASS, eo_constructor());

  obj->speed = 1.0;
}

static void _destructor(Eo *eo_obj, void *_pd, va_list *list EINA_UNUSED)
{
  Ecore_Audio_Input *obj = _pd;

  if(obj->output)
    eo_do(obj->output, ecore_audio_obj_out_input_detach(eo_obj, NULL));

  eo_do_super(eo_obj, MY_CLASS, eo_destructor());
}

static void _class_constructor(Eo_Class *klass)
{
  const Eo_Op_Func_Description func_desc[] = {
      /* Virtual functions of parent class implemented in this class */
      EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
      EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _destructor),

      EO_OP_FUNC(ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_VIO_SET), _vio_set),

      /* Specific functions to this class */
      EO_OP_FUNC(ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_SPEED_SET), _speed_set),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_SPEED_GET), _speed_get),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_SAMPLERATE_SET), _samplerate_set),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_SAMPLERATE_GET), _samplerate_get),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_CHANNELS_SET), _channels_set),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_CHANNELS_GET), _channels_get),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_LOOPED_SET), _looped_set),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_LOOPED_GET), _looped_get),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_LENGTH_GET), _length_get),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_REMAINING_GET), _remaining_get),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_READ), _in_read),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_READ_INTERNAL), _read_internal),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_OUTPUT_GET), _output_get),

      EO_OP_FUNC_SENTINEL
  };

  eo_class_funcs_set(klass, func_desc);
}

#define S(val) "Sets the " #val " of the input."
#define G(val) "Gets the " #val " of the input."

static const Eo_Op_Description op_desc[] = {
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_IN_SUB_ID_SPEED_SET, S(speed)),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_IN_SUB_ID_SPEED_GET, G(speed)),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_IN_SUB_ID_SAMPLERATE_SET, S(samplerate)),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_IN_SUB_ID_SAMPLERATE_GET, G(samplerate)),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_IN_SUB_ID_CHANNELS_SET, S(channels)),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_IN_SUB_ID_CHANNELS_GET, G(channels)),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_IN_SUB_ID_PRELOADED_SET, S(preloaded)),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_IN_SUB_ID_PRELOADED_GET, G(preloaded)),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_IN_SUB_ID_LOOPED_SET, S(looped)),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_IN_SUB_ID_LOOPED_GET, G(looped)),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_IN_SUB_ID_LENGTH_SET, S(length)),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_IN_SUB_ID_LENGTH_GET, G(length)),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_IN_SUB_ID_READ, "Read from the input"),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_IN_SUB_ID_READ_INTERNAL, "Internal implementation for the read"),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_IN_SUB_ID_SEEK, "Seek within the input"),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_IN_SUB_ID_OUTPUT_GET, G(output)),
    EO_OP_DESCRIPTION(ECORE_AUDIO_OBJ_IN_SUB_ID_REMAINING_GET, G(remaining)),
    EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Event_Description *event_desc[] = {
    ECORE_AUDIO_EV_IN_LOOPED,
    ECORE_AUDIO_EV_IN_STOPPED,
    ECORE_AUDIO_EV_IN_SAMPLERATE_CHANGED,
    NULL
};

static const Eo_Class_Description class_desc = {
    EO_VERSION,
    MY_CLASS_NAME,
    EO_CLASS_TYPE_REGULAR,
    EO_CLASS_DESCRIPTION_OPS(&ECORE_AUDIO_OBJ_IN_BASE_ID, op_desc, ECORE_AUDIO_OBJ_IN_SUB_ID_LAST),
    event_desc,
    sizeof(Ecore_Audio_Input),
    _class_constructor,
    NULL
};

EO_DEFINE_CLASS(ecore_audio_obj_in_class_get, &class_desc, ECORE_AUDIO_OBJ_CLASS, NULL);
