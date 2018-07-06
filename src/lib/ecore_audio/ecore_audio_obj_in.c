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

#define MY_CLASS ECORE_AUDIO_IN_CLASS
#define MY_CLASS_NAME "Ecore_Audio_In"

EOLIAN static void
_ecore_audio_in_speed_set(Eo *eo_obj EINA_UNUSED, Ecore_Audio_Input *obj, double speed)
{
  if (speed < 0.2)
    speed = 0.2;
  if (speed > 5.0)
    speed = 5.0;

  obj->speed = speed;

  efl_event_callback_call(eo_obj, ECORE_AUDIO_IN_EVENT_IN_SAMPLERATE_CHANGED, NULL);
}

EOLIAN static double
_ecore_audio_in_speed_get(const Eo *eo_obj EINA_UNUSED, Ecore_Audio_Input *obj)
{
  return obj->speed;
}

EOLIAN static void
_ecore_audio_in_samplerate_set(Eo *eo_obj EINA_UNUSED, Ecore_Audio_Input *obj, int samplerate)
{
  obj->samplerate = samplerate;

  efl_event_callback_call(eo_obj, ECORE_AUDIO_IN_EVENT_IN_SAMPLERATE_CHANGED, NULL);
}

EOLIAN static int
_ecore_audio_in_samplerate_get(const Eo *eo_obj EINA_UNUSED, Ecore_Audio_Input *obj)
{
  return obj->samplerate;;
}

EOLIAN static void
_ecore_audio_in_channels_set(Eo *eo_obj EINA_UNUSED, Ecore_Audio_Input *obj, int channels)
{
  obj->channels = channels;

  /* TODO: Notify output */

}

EOLIAN static int
_ecore_audio_in_channels_get(const Eo *eo_obj EINA_UNUSED, Ecore_Audio_Input *obj)
{
  return obj->channels;
}

EOLIAN static void
_ecore_audio_in_looped_set(Eo *eo_obj EINA_UNUSED, Ecore_Audio_Input *obj, Eina_Bool looped)
{
  obj->looped = looped;
}

EOLIAN static Eina_Bool
_ecore_audio_in_looped_get(const Eo *eo_obj EINA_UNUSED, Ecore_Audio_Input *obj)
{
  return obj->looped;
}

EOLIAN static double
_ecore_audio_in_length_get(const Eo *eo_obj EINA_UNUSED, Ecore_Audio_Input *obj)
{
  return obj->length;;
}

EOLIAN static double
_ecore_audio_in_remaining_get(const Eo *eo_obj, Ecore_Audio_Input *obj)
{
  if (!obj->seekable) return -1;
  else {
       double ret = 0.0;
       /* XXX const */
       ret = ecore_audio_obj_in_seek((Eo *)eo_obj, 0, SEEK_CUR);
       return obj->length - ret;
  }
}

EOLIAN static ssize_t
_ecore_audio_in_read(Eo *eo_obj, Ecore_Audio_Input *obj, void *buf, size_t len)
{
  ssize_t len_read = 0;
  const Ecore_Audio_Object *ea_obj = efl_data_scope_get(eo_obj, ECORE_AUDIO_CLASS);

  if (ea_obj->paused) {
    memset(buf, 0, len);
    len_read = len;
  } else {
      len_read = ecore_audio_obj_in_read_internal(eo_obj, buf, len);
      if (len_read == 0) {
          if (!obj->looped || !obj->seekable) {
              efl_event_callback_call(eo_obj, ECORE_AUDIO_IN_EVENT_IN_STOPPED, NULL);
          } else {
              ecore_audio_obj_in_seek(eo_obj, 0, SEEK_SET);
              len_read = ecore_audio_obj_in_read_internal(eo_obj, buf, len);
              efl_event_callback_call(eo_obj, ECORE_AUDIO_IN_EVENT_IN_LOOPED, NULL);
          }
      }

  }

  return len_read;
}

EOLIAN static ssize_t
_ecore_audio_in_read_internal(Eo *eo_obj, Ecore_Audio_Input *_pd EINA_UNUSED, void *buf, size_t len)
{
  ssize_t len_read = 0;
  const Ecore_Audio_Object *ea_obj = efl_data_scope_get(eo_obj, ECORE_AUDIO_CLASS);

  if (ea_obj->vio && ea_obj->vio->vio->read) {
      len_read = ea_obj->vio->vio->read(ea_obj->vio->data, eo_obj, buf, len);
  }

  return len_read;
}

EOLIAN static Eo*
_ecore_audio_in_output_get(const Eo *eo_obj EINA_UNUSED, Ecore_Audio_Input *obj)
{
   return obj->output;
}

static void _free_vio(Ecore_Audio_Object *ea_obj)
{
  if (ea_obj->vio->free_func)
    ea_obj->vio->free_func(ea_obj->vio->data);

  free(ea_obj->vio);
  ea_obj->vio = NULL;
}

EOLIAN static void
_ecore_audio_in_ecore_audio_vio_set(Eo *eo_obj, Ecore_Audio_Input *obj, Ecore_Audio_Vio *vio, void *data, efl_key_data_free_func free_func)
{
  Ecore_Audio_Object *ea_obj = efl_data_scope_get(eo_obj, ECORE_AUDIO_CLASS);

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

EOLIAN static Eo *
_ecore_audio_in_efl_object_constructor(Eo *eo_obj, Ecore_Audio_Input *obj)
{
  eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

  obj->speed = 1.0;

  return eo_obj;
}

EOLIAN static void
_ecore_audio_in_efl_object_destructor(Eo *eo_obj, Ecore_Audio_Input *obj)
{
  if(obj->output)
    ecore_audio_obj_out_input_detach(obj->output, eo_obj);

  efl_destructor(efl_super(eo_obj, MY_CLASS));
}

#include "ecore_audio_in.eo.c"
