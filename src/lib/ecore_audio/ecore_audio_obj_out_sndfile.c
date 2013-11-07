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

EAPI Eo_Op ECORE_AUDIO_OBJ_OUT_SNDFILE_BASE_ID = EO_NOOP;

#define MY_CLASS ECORE_AUDIO_OBJ_OUT_SNDFILE_CLASS
#define MY_CLASS_NAME "Ecore_Audio_Out_Sndfile"

extern SF_VIRTUAL_IO vio_wrapper;

struct _Ecore_Audio_Sndfile
{
  SNDFILE *handle;
  SF_INFO sfinfo;
  Ecore_Audio_Vio *vio;
};

typedef struct _Ecore_Audio_Sndfile Ecore_Audio_Sndfile;

static Eina_Bool _write_cb(void *data)
{
  Eo *eo_obj = data;
  Eo *in;

  Ecore_Audio_Sndfile *obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_OUT_SNDFILE_CLASS);
  Ecore_Audio_Output *out_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_OUT_CLASS);
  Ecore_Audio_Object *ea_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_CLASS);

  ssize_t written, bread;
  float buf[1024];

  /* TODO: Support mixing of multiple inputs */
  in = eina_list_data_get(out_obj->inputs);

  eo_do(in, ecore_audio_obj_in_read(buf, 4*1024, &bread));

  if (bread == 0) {
      sf_write_sync(obj->handle);
      ea_obj->paused = EINA_TRUE;
      out_obj->write_idler = NULL;
      return EINA_FALSE;
  }
  written = sf_write_float(obj->handle, buf, bread/4)*4;

  if (written != bread)
    ERR("Short write! (%s)\n", sf_strerror(obj->handle));

  return EINA_TRUE;
}

static void _input_attach(Eo *eo_obj, void *_pd, va_list *list)
{
  Ecore_Audio_Sndfile *obj = _pd;
  Ecore_Audio_Object *ea_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_CLASS);
  Ecore_Audio_Output *out_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_OUT_CLASS);
  Eina_Bool ret2;

  Eo *in = va_arg(*list, Eo *);
  Eina_Bool *ret = va_arg(*list, Eina_Bool *);

  if (ret)
    *ret = EINA_FALSE;

  eo_do_super(eo_obj, MY_CLASS, ecore_audio_obj_out_input_attach(in, &ret2));
  if (!ret2)
    return;

  eo_do(in, ecore_audio_obj_in_samplerate_get(&obj->sfinfo.samplerate));
  eo_do(in, ecore_audio_obj_in_channels_get(&obj->sfinfo.channels));

  obj->handle = sf_open(ea_obj->source, SFM_WRITE, &obj->sfinfo);

  if (!obj->handle) {
    eina_stringshare_del(ea_obj->source);
    ea_obj->source = NULL;
    eo_do_super(eo_obj, MY_CLASS, ecore_audio_obj_out_input_detach(in, NULL));
    return;
  }

  if (ret)
    *ret = EINA_TRUE;

  if (ea_obj->paused)
    return;

  if (out_obj->inputs) {
    out_obj->write_idler = ecore_idler_add(_write_cb, eo_obj);
  }
}

static void _source_set(Eo *eo_obj, void *_pd, va_list *list)
{
  Ecore_Audio_Sndfile *obj = _pd;

  Ecore_Audio_Object *ea_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_CLASS);

  const char *source = va_arg(*list, const char *);
  Eina_Bool *ret = va_arg(*list, Eina_Bool *);

  if (ret)
    *ret = EINA_FALSE;

  if (obj->handle) {
    sf_close(obj->handle);
    obj->handle = NULL;
  }

  eina_stringshare_replace(&ea_obj->source, source);

  if (!ea_obj->source)
    return;

  //FIXME: Open the file here

  if (ret)
    *ret = EINA_TRUE;

}

static void _source_get(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
  Ecore_Audio_Object *obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_CLASS);

  const char **ret = va_arg(*list, const char **);

  if (ret)
    *ret = obj->source;
}

static void _format_set(Eo *eo_obj, void *_pd, va_list *list)
{
  Ecore_Audio_Sndfile *obj = _pd;
  Ecore_Audio_Object *ea_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_CLASS);

  Ecore_Audio_Format format= va_arg(*list, Ecore_Audio_Format);
  Eina_Bool *ret = va_arg(*list, Eina_Bool *);

  if (ea_obj->source) {
      ERR("Input is already open - cannot change format");
      if (ret)
        *ret = EINA_FALSE;
      return;
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
      if (ret)
        *ret = EINA_FALSE;
      return;
  }
  ea_obj->format = format;
  if (ret)
    *ret = EINA_TRUE;
}

static void _format_get(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
  Ecore_Audio_Object *obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_CLASS);

  Ecore_Audio_Format *ret = va_arg(*list, Ecore_Audio_Format *);

  if (ret)
    *ret = obj->format;
}

static void _constructor(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
  Ecore_Audio_Output *out_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_OUT_CLASS);

  eo_do_super(eo_obj, MY_CLASS, eo_constructor());

  eo_do(eo_obj, ecore_audio_obj_format_set(ECORE_AUDIO_FORMAT_OGG, NULL));

  // FIXME: Use writer from output
  out_obj->need_writer = EINA_FALSE;
}

static void _destructor(Eo *eo_obj, void *_pd, va_list *list EINA_UNUSED)
{
  Ecore_Audio_Sndfile *obj = _pd;
  Ecore_Audio_Output *out_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_OBJ_OUT_CLASS);

  if (obj->handle)
    sf_close(obj->handle);
  if (out_obj->write_idler)
    ecore_idler_del(out_obj->write_idler);

  eo_do_super(eo_obj, MY_CLASS, eo_destructor());
}

static void _class_constructor(Eo_Class *klass)
{
  const Eo_Op_Func_Description func_desc[] = {
      /* Virtual functions of parent class implemented in this class */
      EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
      EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _destructor),

      EO_OP_FUNC(ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_SOURCE_SET), _source_set),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_SOURCE_GET), _source_get),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_FORMAT_SET), _format_set),
      EO_OP_FUNC(ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_FORMAT_GET), _format_get),

      EO_OP_FUNC(ECORE_AUDIO_OBJ_OUT_ID(ECORE_AUDIO_OBJ_OUT_SUB_ID_INPUT_ATTACH), _input_attach),
      //EO_OP_FUNC(ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_OUT_SUB_ID_FORMAT_GET), _format_get),
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
    EO_CLASS_DESCRIPTION_OPS(&ECORE_AUDIO_OBJ_OUT_SNDFILE_BASE_ID, op_desc, ECORE_AUDIO_OBJ_OUT_SNDFILE_SUB_ID_LAST),
    NULL,
    sizeof(Ecore_Audio_Sndfile),
    _class_constructor,
    NULL
};

EO_DEFINE_CLASS(ecore_audio_obj_out_sndfile_class_get, &class_desc, ECORE_AUDIO_OBJ_OUT_CLASS, NULL);
