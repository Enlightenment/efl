#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_FEATURES_H
#include <features.h>
#endif

#include <Eo.h>

#include "ecore_audio_private.h"
#include <sndfile.h>

/* Virtual IO wrapper functions */

static sf_count_t _wrap_get_filelen(void *data)
{
  Eo *eo_obj = data;
  Ecore_Audio_Object *ea_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_CLASS);

  if (!ea_obj->vio->vio)
    goto error;

  if (ea_obj->vio->vio->get_length)
    return ea_obj->vio->vio->get_length(ea_obj->vio->data, eo_obj);

error:
  return -1;
}

static sf_count_t _wrap_seek(sf_count_t offset, int whence, void *data)
{
  Eo *eo_obj = data;
  Ecore_Audio_Object *ea_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_CLASS);

  if (!ea_obj->vio->vio)
    goto error;

  if (ea_obj->vio->vio->seek)
    return ea_obj->vio->vio->seek(ea_obj->vio->data, eo_obj, offset, whence);

error:
  return -1;
}

static sf_count_t _wrap_read(void *buffer, sf_count_t count, void *data)
{
  Eo *eo_obj = data;
  Ecore_Audio_Object *ea_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_CLASS);

  if (!ea_obj->vio->vio)
    goto error;

  if (ea_obj->vio->vio->read)
    return ea_obj->vio->vio->read(ea_obj->vio->data, eo_obj, buffer, count);

error:
  return 0;
}

static sf_count_t _wrap_write(const void *buffer, sf_count_t count, void *data)
{
  Eo *eo_obj = data;
  Ecore_Audio_Object *ea_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_CLASS);

  if (!ea_obj->vio->vio)
    goto error;

  if (ea_obj->vio->vio->write)
    return ea_obj->vio->vio->write(ea_obj->vio->data, eo_obj, buffer, count);

error:
  return 0;
}

static sf_count_t _wrap_tell(void *data)
{
  Eo *eo_obj = data;
  Ecore_Audio_Object *ea_obj = eo_data_scope_get(eo_obj, ECORE_AUDIO_CLASS);

  if (!ea_obj->vio->vio)
    goto error;

  if (ea_obj->vio->vio->tell)
    return ea_obj->vio->vio->tell(ea_obj->vio->data, eo_obj);

error:
  return -1;
}

SF_VIRTUAL_IO vio_wrapper = {
    .get_filelen = _wrap_get_filelen,
    .seek = _wrap_seek,
    .read = _wrap_read,
    .write = _wrap_write,
    .tell = _wrap_tell,
};

/* End virtual IO wrapper functions */

