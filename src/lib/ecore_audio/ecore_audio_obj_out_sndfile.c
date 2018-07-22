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

#define MY_CLASS ECORE_AUDIO_OUT_SNDFILE_CLASS
#define MY_CLASS_NAME "Ecore_Audio_Out_Sndfile"

extern SF_VIRTUAL_IO vio_wrapper;

struct _Ecore_Audio_Out_Sndfile_Data
{
  SNDFILE *handle;
  SF_INFO sfinfo;
  Ecore_Audio_Vio *vio;
};

typedef struct _Ecore_Audio_Out_Sndfile_Data Ecore_Audio_Out_Sndfile_Data;

static Eina_Bool _write_cb(void *data)
{
  Eo *eo_obj = data;
  Eo *in;

  Ecore_Audio_Out_Sndfile_Data *obj = efl_data_scope_get(eo_obj, ECORE_AUDIO_OUT_SNDFILE_CLASS);
  Ecore_Audio_Output *out_obj = efl_data_scope_get(eo_obj, ECORE_AUDIO_OUT_CLASS);
  Ecore_Audio_Object *ea_obj = efl_data_scope_get(eo_obj, ECORE_AUDIO_CLASS);

  ssize_t written, bread = 0;
  float buf[1024];

  if (!ESF_LOAD()) return EINA_FALSE;
  /* TODO: Support mixing of multiple inputs */
  in = eina_list_data_get(out_obj->inputs);

  bread = ecore_audio_obj_in_read(in, buf, 4*1024);

  if (bread == 0) {
      ESF_CALL(sf_write_sync)(obj->handle);
      ea_obj->paused = EINA_TRUE;
      out_obj->write_idler = NULL;
      return EINA_FALSE;
  }
  written = ESF_CALL(sf_write_float)(obj->handle, buf, bread/4)*4;

  if (written != bread)
    ERR("Short write! (%s)\n", ESF_CALL(sf_strerror)(obj->handle));

  return EINA_TRUE;
}

EOLIAN static Eina_Bool
_ecore_audio_out_sndfile_ecore_audio_out_input_attach(Eo *eo_obj, Ecore_Audio_Out_Sndfile_Data *obj, Eo *in)
{
  Ecore_Audio_Object *ea_obj = efl_data_scope_get(eo_obj, ECORE_AUDIO_CLASS);
  Ecore_Audio_Output *out_obj = efl_data_scope_get(eo_obj, ECORE_AUDIO_OUT_CLASS);
  Eina_Bool ret2 = EINA_FALSE;

  if (!ESF_LOAD()) return EINA_FALSE;
  ret2 = ecore_audio_obj_out_input_attach(efl_super(eo_obj, MY_CLASS), in);
  if (!ret2)
    return EINA_FALSE;

  obj->sfinfo.samplerate = ecore_audio_obj_in_samplerate_get(in);
  obj->sfinfo.channels = ecore_audio_obj_in_channels_get(in);

  obj->handle = ESF_CALL(sf_open)(ea_obj->source, SFM_WRITE, &obj->sfinfo);

  if (!obj->handle) {
    eina_stringshare_del(ea_obj->source);
    ea_obj->source = NULL;
    ecore_audio_obj_out_input_detach(efl_super(eo_obj, MY_CLASS), in);
    return EINA_FALSE;
  }

  if (ea_obj->paused)
    return EINA_TRUE;

  if (out_obj->inputs) {
    out_obj->write_idler = ecore_idler_add(_write_cb, eo_obj);
  }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_ecore_audio_out_sndfile_ecore_audio_source_set(Eo *eo_obj, Ecore_Audio_Out_Sndfile_Data *obj, const char *source)
{
  Ecore_Audio_Object *ea_obj = efl_data_scope_get(eo_obj, ECORE_AUDIO_CLASS);

  if (!ESF_LOAD()) return EINA_FALSE;
  if (obj->handle) {
    ESF_CALL(sf_close)(obj->handle);
    obj->handle = NULL;
  }

  eina_stringshare_replace(&ea_obj->source, source);

  if (!ea_obj->source)
    return EINA_FALSE;

  //FIXME: Open the file here

  return EINA_TRUE;
}

EOLIAN static const char*
_ecore_audio_out_sndfile_ecore_audio_source_get(const Eo *eo_obj, Ecore_Audio_Out_Sndfile_Data *_pd EINA_UNUSED)
{
  Ecore_Audio_Object *obj = efl_data_scope_get(eo_obj, ECORE_AUDIO_CLASS);
  return obj->source;
}

EOLIAN static Eina_Bool
_ecore_audio_out_sndfile_ecore_audio_format_set(Eo *eo_obj, Ecore_Audio_Out_Sndfile_Data *obj, Ecore_Audio_Format format)
{
  Ecore_Audio_Object *ea_obj = efl_data_scope_get(eo_obj, ECORE_AUDIO_CLASS);

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
_ecore_audio_out_sndfile_ecore_audio_format_get(const Eo *eo_obj, Ecore_Audio_Out_Sndfile_Data *_pd EINA_UNUSED)
{
  Ecore_Audio_Object *obj = efl_data_scope_get(eo_obj, ECORE_AUDIO_CLASS);
  return obj->format;
}

EOLIAN static Eo *
_ecore_audio_out_sndfile_efl_object_constructor(Eo *eo_obj, Ecore_Audio_Out_Sndfile_Data *_pd EINA_UNUSED)
{
  Ecore_Audio_Output *out_obj = efl_data_scope_get(eo_obj, ECORE_AUDIO_OUT_CLASS);

  eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

  ecore_audio_obj_format_set(eo_obj, ECORE_AUDIO_FORMAT_OGG);

  // FIXME: Use writer from output
  out_obj->need_writer = EINA_FALSE;
  return eo_obj;
}

EOLIAN static void
_ecore_audio_out_sndfile_efl_object_destructor(Eo *eo_obj, Ecore_Audio_Out_Sndfile_Data *obj)
{
  Ecore_Audio_Output *out_obj = efl_data_scope_get(eo_obj, ECORE_AUDIO_OUT_CLASS);

  if (obj->handle)
    ESF_CALL(sf_close)(obj->handle);
  if (out_obj->write_idler)
    ecore_idler_del(out_obj->write_idler);

  efl_destructor(efl_super(eo_obj, MY_CLASS));
}

#include "ecore_audio_out_sndfile.eo.c"
