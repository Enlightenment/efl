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
#include <sndfile.h>

#define MY_CLASS ECORE_AUDIO_IN_SNDFILE_CLASS
#define MY_CLASS_NAME "Ecore_Audio_In_Sndfile"

extern SF_VIRTUAL_IO vio_wrapper;

struct _Ecore_Audio_In_Sndfile_Data
{
  SNDFILE *handle;
  SF_INFO sfinfo;
};

typedef struct _Ecore_Audio_In_Sndfile_Data Ecore_Audio_In_Sndfile_Data;

EOLIAN static ssize_t
_ecore_audio_in_sndfile_ecore_audio_in_read_internal(Eo *eo_obj EINA_UNUSED, Ecore_Audio_In_Sndfile_Data *obj, void *data, size_t len)
{
  return sf_read_float(obj->handle, data, len/4)*4;
}

EOLIAN static double
_ecore_audio_in_sndfile_ecore_audio_in_seek(Eo *eo_obj EINA_UNUSED, Ecore_Audio_In_Sndfile_Data *obj, double offs, int mode)
{
  sf_count_t count, pos;

  count = offs * obj->sfinfo.samplerate;
  pos = sf_seek(obj->handle, count, mode);

  return (double)pos / obj->sfinfo.samplerate;
}

EOLIAN static Eina_Bool
_ecore_audio_in_sndfile_ecore_audio_source_set(Eo *eo_obj, Ecore_Audio_In_Sndfile_Data *obj, const char *source)
{
  Ecore_Audio_Object *ea_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_CLASS);
  Ecore_Audio_Input *in_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_IN_CLASS);

  if (obj->handle) {
    sf_close(obj->handle);
    obj->handle = NULL;
  }

  eina_stringshare_replace(&ea_obj->source, source);

  if (!ea_obj->source)
    return EINA_FALSE;

  obj->handle = sf_open(ea_obj->source, SFM_READ, &obj->sfinfo);

  if (!obj->handle) {
    eina_stringshare_del(ea_obj->source);
    ea_obj->source = NULL;
    return EINA_FALSE;
  }

  in_obj->seekable = EINA_TRUE;
  in_obj->length = (double)obj->sfinfo.frames / obj->sfinfo.samplerate;

  in_obj->samplerate =  obj->sfinfo.samplerate;
  in_obj->channels =  obj->sfinfo.channels;

  if (obj->sfinfo.format& SF_FORMAT_WAV)
    ea_obj->format = ECORE_AUDIO_FORMAT_WAV;
  else if (obj->sfinfo.format& SF_FORMAT_OGG)
    ea_obj->format = ECORE_AUDIO_FORMAT_OGG;
  else if (obj->sfinfo.format& SF_FORMAT_FLAC)
    ea_obj->format = ECORE_AUDIO_FORMAT_FLAC;
  else
    ea_obj->format = ECORE_AUDIO_FORMAT_AUTO;

   return EINA_TRUE;
}

EOLIAN static const char*
_ecore_audio_in_sndfile_ecore_audio_source_get(Eo *eo_obj, Ecore_Audio_In_Sndfile_Data *_pd EINA_UNUSED)
{
  Ecore_Audio_Object *obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_CLASS);
  return obj->source;
}

EOLIAN static Eina_Bool
_ecore_audio_in_sndfile_ecore_audio_format_set(Eo *eo_obj, Ecore_Audio_In_Sndfile_Data *obj, Ecore_Audio_Format format)
{
  Ecore_Audio_Object *ea_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_CLASS);

  if (ea_obj->source) {
      ERR("Input is already open - cannot change format");
      return EINA_FALSE;
  }

  switch (format) {
    case ECORE_AUDIO_FORMAT_AUTO:
      obj->sfinfo.format = 0;
      break;
    case ECORE_AUDIO_FORMAT_WAV:
      obj->sfinfo.format = SF_FORMAT_WAV|SF_FORMAT_PCM_16;
      break;
    case ECORE_AUDIO_FORMAT_OGG:
      obj->sfinfo.format = SF_FORMAT_OGG|SF_FORMAT_VORBIS;
      break;
    case ECORE_AUDIO_FORMAT_FLAC:
      obj->sfinfo.format = SF_FORMAT_FLAC;
      break;
    default:
      ERR("Format not supported!");
      return EINA_FALSE;
  }
  ea_obj->format = format;

  return EINA_TRUE;
}

EOLIAN static Ecore_Audio_Format
_ecore_audio_in_sndfile_ecore_audio_format_get(Eo *eo_obj, Ecore_Audio_In_Sndfile_Data *_pd EINA_UNUSED)
{
  Ecore_Audio_Object *obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_CLASS);
  return obj->format;;
}

static void _free_vio(Ecore_Audio_Object *ea_obj)
{
  if (ea_obj->vio->free_func)
    ea_obj->vio->free_func(ea_obj->vio->data);

  free(ea_obj->vio);
  ea_obj->vio = NULL;
}

EOLIAN static void
_ecore_audio_in_sndfile_ecore_audio_vio_set(Eo *eo_obj, Ecore_Audio_In_Sndfile_Data *obj, Ecore_Audio_Vio *vio, void *data, eo_key_data_free_func free_func)
{
  Ecore_Audio_Object *ea_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_CLASS);
  Ecore_Audio_Input *in_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_IN_CLASS);

  if (obj->handle) {
    sf_close(obj->handle);
    obj->handle = NULL;
  }

  eina_stringshare_replace(&ea_obj->source, "VIO");

  if (!ea_obj->source)
    return;
  if (ea_obj->vio)
    _free_vio(ea_obj);

  in_obj->seekable = EINA_FALSE;

  if (!vio)
    return;

  ea_obj->vio = calloc(1, sizeof(Ecore_Audio_Vio_Internal));
  ea_obj->vio->vio = vio;
  ea_obj->vio->data = data;
  ea_obj->vio->free_func = free_func;
  in_obj->seekable = (vio->seek != NULL);

  obj->handle = sf_open_virtual(&vio_wrapper, SFM_READ, &obj->sfinfo, eo_obj);

  if (!obj->handle) {
    eina_stringshare_del(ea_obj->source);
    ea_obj->source = NULL;
    return;
  }

  in_obj->seekable = EINA_TRUE;
  in_obj->length = (double)obj->sfinfo.frames / obj->sfinfo.samplerate;

  in_obj->samplerate =  obj->sfinfo.samplerate;
  in_obj->channels =  obj->sfinfo.channels;

  if (obj->sfinfo.format& SF_FORMAT_WAV)
    ea_obj->format = ECORE_AUDIO_FORMAT_WAV;
  else if (obj->sfinfo.format& SF_FORMAT_OGG)
    ea_obj->format = ECORE_AUDIO_FORMAT_OGG;
  else if (obj->sfinfo.format& SF_FORMAT_FLAC)
    ea_obj->format = ECORE_AUDIO_FORMAT_FLAC;
  else
    ea_obj->format = ECORE_AUDIO_FORMAT_AUTO;
}

EOLIAN static void
_ecore_audio_in_sndfile_eo_base_constructor(Eo *eo_obj, Ecore_Audio_In_Sndfile_Data *_pd EINA_UNUSED)
{
  eo_do_super(eo_obj, MY_CLASS, eo_constructor());

}

EOLIAN static void
_ecore_audio_in_sndfile_eo_base_destructor(Eo *eo_obj, Ecore_Audio_In_Sndfile_Data *obj)
{
  Ecore_Audio_Object *ea_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_CLASS);

  if (obj->handle)
    sf_close(obj->handle);

  if (ea_obj->vio)
    _free_vio(ea_obj);

  eo_do_super(eo_obj, MY_CLASS, eo_destructor());
}

#include "ecore_audio_in_sndfile.eo.c"
