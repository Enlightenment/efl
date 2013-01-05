#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SNDFILE
#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Audio.h"
#include "ecore_audio_private.h"

#include <sndfile.h>

static Ecore_Audio_Module *module = NULL;

/* Virtual IO wrapper functions */

static sf_count_t _sndfile_wrap_get_filelen(void *data)
{
  Ecore_Audio_Object *obj = data;
  struct _Ecore_Audio_Sndfile *sndfile = obj->module_data;

  if (!sndfile->vio)
    goto error;

  if (sndfile->vio->get_length)
    return sndfile->vio->get_length(obj);

error:
  return -1;
}

static sf_count_t _sndfile_wrap_seek(sf_count_t offset, int whence, void *data)
{
  Ecore_Audio_Object *obj = data;
  struct _Ecore_Audio_Sndfile *sndfile = obj->module_data;

  if (!sndfile->vio)
    goto error;

  if (sndfile->vio->seek)
    return sndfile->vio->seek(obj, offset, whence);

error:
  return -1;
}

static sf_count_t _sndfile_wrap_read(void *buffer, sf_count_t count, void *data)
{
  Ecore_Audio_Object *obj = data;
  struct _Ecore_Audio_Sndfile *sndfile = obj->module_data;

  if (!sndfile->vio)
    goto error;

  if (sndfile->vio->read)
    return sndfile->vio->read(obj, buffer, count);

error:
  return 0;
}

static sf_count_t _sndfile_wrap_write(const void *buffer, sf_count_t count, void *data)
{
  Ecore_Audio_Object *obj = data;
  struct _Ecore_Audio_Sndfile *sndfile = obj->module_data;

  if (!sndfile->vio)
    goto error;

  if (sndfile->vio->write)
    return sndfile->vio->write(obj, buffer, count);

error:
  return 0;
}

static sf_count_t _sndfile_wrap_tell(void *data)
{
  Ecore_Audio_Object *obj = data;
  struct _Ecore_Audio_Sndfile *sndfile = obj->module_data;

  if (!sndfile->vio)
    goto error;

  if (sndfile->vio->tell)
    return sndfile->vio->tell(obj);

error:
  return -1;
}

/* End virtual IO wrapper functions */

static Ecore_Audio_Object *
_sndfile_input_new(Ecore_Audio_Object *input)
{
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
   struct _Ecore_Audio_Sndfile *sndfile;

   sndfile = calloc(1, sizeof(struct _Ecore_Audio_Sndfile));
   if (!sndfile)
     {
        ERR("Could not allocate memory for private structure.");
        free(in);
        return NULL;
     }

   in->module_data = sndfile;

   sndfile->sfinfo.format = 0;
   sndfile->volume = 1.0;

   return (Ecore_Audio_Object *)in;
}

static void
_sndfile_input_del(Ecore_Audio_Object *input)
{
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
   struct _Ecore_Audio_Sndfile *sndfile = in->module_data;

   eina_stringshare_del(sndfile->filename);

   sf_close(sndfile->handle);
   free(sndfile);
}

static int
_sndfile_input_read(Ecore_Audio_Object *input, void *data, int len)
{
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
   struct _Ecore_Audio_Sndfile *sndfile = in->module_data;
   int read;

   /* FIXME: Nicer way to return bytes read instead of items */
   read = sf_read_float(sndfile->handle, data, len / 4) * 4;

   return read;
}

static double
_sndfile_input_seek(Ecore_Audio_Object *input, double offs, int mode)
{
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
   struct _Ecore_Audio_Sndfile *sndfile = in->module_data;
   sf_count_t count, pos;

   count = offs * sndfile->sfinfo.samplerate;
   pos = sf_seek(sndfile->handle, count, mode);
   return (double)pos / sndfile->sfinfo.samplerate;
}

static struct input_api inops = {
   .input_new = _sndfile_input_new,
   .input_del = _sndfile_input_del,
   .input_read = _sndfile_input_read,
   .input_seek = _sndfile_input_seek,
};

EAPI void ecore_audio_input_sndfile_format_set(Ecore_Audio_Object *input, int format)
{
   Ecore_Audio_Output *in = (Ecore_Audio_Output *)input;
   struct _Ecore_Audio_Sndfile *sndfile = in->module_data;

   sndfile->sfinfo.format = format;
}

void ecore_audio_input_sndfile_filename_set(Ecore_Audio_Object *input, const char *filename)
{
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
   struct _Ecore_Audio_Sndfile *sndfile = in->module_data;

   if (sndfile->handle)
     sf_close(sndfile->handle);

   if (sndfile->filename)
     eina_stringshare_del(sndfile->filename);

   if (sndfile->vio)
     {
        free(sndfile->vio);
        sndfile->vio = NULL;
     }

   sndfile->filename = eina_stringshare_add(filename);

   sndfile->handle = sf_open(sndfile->filename, SFM_READ, &sndfile->sfinfo);

   in->length = (double)sndfile->sfinfo.frames / sndfile->sfinfo.samplerate;

   // XXX: Propagate samplerate/channel update to output
   in->samplerate = sndfile->sfinfo.samplerate;
   in->channels = sndfile->sfinfo.channels;
}

void ecore_audio_input_sndfile_vio_set(Ecore_Audio_Object *input, Ecore_Audio_Vio *vio)
{
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
   struct _Ecore_Audio_Sndfile *sndfile = in->module_data;
   struct _Ecore_Audio_Sndfile_Private *priv = in->module->priv;

   if (sndfile->handle)
     sf_close(sndfile->handle);

   if (sndfile->filename)
     {
        eina_stringshare_del(sndfile->filename);
        sndfile->filename = NULL;
     }

   if (sndfile->vio)
     free(sndfile->vio);

   sndfile->vio = calloc(1, sizeof(Ecore_Audio_Vio));
   sndfile->vio->get_length = vio->get_length;
   sndfile->vio->seek = vio->seek;
   sndfile->vio->read = vio->read;
   sndfile->vio->tell = vio->tell;

   if (sndfile->sfinfo.format != 0) {
       sndfile->sfinfo.samplerate = in->samplerate;
       sndfile->sfinfo.channels = in->channels;
   }

   sndfile->handle = sf_open_virtual(&priv->vio_wrapper, SFM_READ, &sndfile->sfinfo, in);

   in->length = (double)sndfile->sfinfo.frames / sndfile->sfinfo.samplerate;

   // XXX: Propagate samplerate/channel update to output
   in->samplerate = sndfile->sfinfo.samplerate;
   in->channels = sndfile->sfinfo.channels;
}

static Ecore_Audio_Object *
_sndfile_output_new(Ecore_Audio_Object *output)
{
   Ecore_Audio_Output *out = (Ecore_Audio_Output *)output;
   struct _Ecore_Audio_Sndfile *sndfile;

   sndfile = calloc(1, sizeof(struct _Ecore_Audio_Sndfile));
   if (!sndfile)
     {
        ERR("Could not allocate memory for private structure.");
        free(out);
        return NULL;
     }

   out->module_data = sndfile;

   sndfile->sfinfo.format = SF_FORMAT_OGG | SF_FORMAT_VORBIS;

   return (Ecore_Audio_Object *)out;
}

static void
_sndfile_output_del(Ecore_Audio_Object *output)
{
   Ecore_Audio_Output *out = (Ecore_Audio_Output *)output;
   struct _Ecore_Audio_Sndfile *sndfile = out->module_data;

   eina_stringshare_del(sndfile->filename);
   free(sndfile->vio);

   sf_close(sndfile->handle);
   free(sndfile);
}

static void
_sndfile_output_volume_set(Ecore_Audio_Object *output, double volume)
{
   Ecore_Audio_Output *out = (Ecore_Audio_Output *)output;
   struct _Ecore_Audio_Sndfile *sndfile = out->module_data;

   if ((volume < 0) || (volume > 1.0))
     return;

   sndfile->volume = volume;
}

static double
_sndfile_output_volume_get(Ecore_Audio_Object *output)
{
   Ecore_Audio_Output *out = (Ecore_Audio_Output *)output;
   struct _Ecore_Audio_Sndfile *sndfile = out->module_data;

   return sndfile->volume;
}

static Eina_Bool
_sndfile_output_write_cb(void *data)
{
   Ecore_Audio_Output *out = (Ecore_Audio_Output *)data;
   struct _Ecore_Audio_Sndfile *sndfile = out->module_data;
   Ecore_Audio_Input *in;
   int written, bread;
   void *buf;

   buf = malloc(1024);

   in = eina_list_data_get(out->inputs);

   bread = ecore_audio_input_read((Ecore_Audio_Object *)in, buf, 1024);
   DBG("read: %i", bread);
   if (bread == 0)
     {
        DBG("read done");
        out->paused = EINA_TRUE;
        return EINA_FALSE;
     }

   // XXX: Check for errors
   written = sf_write_float(sndfile->handle, buf, bread/4)*4;
   DBG("written: %i", written);

   return EINA_TRUE;
}

static void
_sndfile_output_paused_set(Ecore_Audio_Object *output, Eina_Bool paused)
{
   struct _Ecore_Audio_Sndfile *sndfile = output->module_data;
   Ecore_Audio_Output *out = (Ecore_Audio_Output *)output;

   if (paused && sndfile->timer)
     ecore_timer_del(sndfile->timer);
   else if (out->inputs)
     sndfile->timer = ecore_timer_add(0.01, _sndfile_output_write_cb, output);
}

static Eina_Bool
_sndfile_output_add_input(Ecore_Audio_Object *output, Ecore_Audio_Object *input)
{
   struct _Ecore_Audio_Sndfile *sndfile = output->module_data;
   struct _Ecore_Audio_Sndfile_Private *priv = output->module->priv;

   Ecore_Audio_Output *out = (Ecore_Audio_Output *)output;
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;

   sndfile->sfinfo.samplerate = in->samplerate;
   sndfile->sfinfo.channels = in->channels;

   if (sndfile->vio)
     sndfile->handle = sf_open_virtual(&priv->vio_wrapper, SFM_WRITE, &sndfile->sfinfo, out);
   else if (sndfile->filename)
     sndfile->handle = sf_open(sndfile->filename, SFM_WRITE, &sndfile->sfinfo);

   if (output->paused)
     return EINA_TRUE;

   if (!out->inputs)
     sndfile->timer = ecore_timer_add(0.01, _sndfile_output_write_cb, output);

   return EINA_TRUE;
}

static Eina_Bool
_sndfile_output_del_input(Ecore_Audio_Object *out EINA_UNUSED, Ecore_Audio_Object *in EINA_UNUSED)
{
   //XXX: Check if it's the last one and pause the timer
   return EINA_TRUE;
}

static void
_sndfile_output_update_input_format(Ecore_Audio_Object *out EINA_UNUSED, Ecore_Audio_Object *in EINA_UNUSED)
{
}

static struct output_api outops = {
   .output_new = _sndfile_output_new,
   .output_del = _sndfile_output_del,
   .output_volume_set = _sndfile_output_volume_set,
   .output_volume_get = _sndfile_output_volume_get,
   .output_paused_set = _sndfile_output_paused_set,
   .output_add_input = _sndfile_output_add_input,
   .output_del_input = _sndfile_output_del_input,
   .output_update_input_format = _sndfile_output_update_input_format,
};

EAPI void ecore_audio_output_sndfile_filename_set(Ecore_Audio_Object *output, const char *filename)
{
   Ecore_Audio_Output *out = (Ecore_Audio_Output *)output;
   struct _Ecore_Audio_Sndfile *sndfile = out->module_data;

   if (sndfile->handle)
     sf_close(sndfile->handle);

   if (sndfile->filename)
     eina_stringshare_del(sndfile->filename);

   if (sndfile->vio)
     {
        free(sndfile->vio);
        sndfile->vio = NULL;
     }

   sndfile->filename = eina_stringshare_add(filename);
}

EAPI void ecore_audio_output_sndfile_format_set(Ecore_Audio_Object *output, int format)
{
   Ecore_Audio_Output *out = (Ecore_Audio_Output *)output;
   struct _Ecore_Audio_Sndfile *sndfile = out->module_data;

   sndfile->sfinfo.format = format;
}

EAPI void ecore_audio_output_sndfile_vio_set(Ecore_Audio_Object *output, Ecore_Audio_Vio *vio)
{
   struct _Ecore_Audio_Sndfile *sndfile = output->module_data;

   if (sndfile->handle)
     sf_close(sndfile->handle);

   if (sndfile->filename)
     {
        eina_stringshare_del(sndfile->filename);
        sndfile->filename = NULL;
     }

   if (sndfile->vio)
     free(sndfile->vio);

   sndfile->vio = calloc(1, sizeof(Ecore_Audio_Vio));
   sndfile->vio->get_length = vio->get_length;
   sndfile->vio->seek = vio->seek;
   sndfile->vio->write = vio->write;
   sndfile->vio->tell = vio->tell;
}

/**
 * @brief Initialize the Ecore_Audio sndfile module
 *
 * @return the initialized module on success, NULL on error
 */
Ecore_Audio_Module *
ecore_audio_sndfile_init(void)
{
   struct _Ecore_Audio_Sndfile_Private *priv;

   module = calloc(1, sizeof(Ecore_Audio_Module));
   if (!module)
     {
        ERR("Could not allocate memory for module.");
        return NULL;
     }

   priv = calloc(1, sizeof(struct _Ecore_Audio_Sndfile_Private));
   if (!priv)
     {
        ERR("Could not allocate memory for private module region.");
        free(module);
        return NULL;
     }

   priv->vio_wrapper.get_filelen = _sndfile_wrap_get_filelen;
   priv->vio_wrapper.seek = _sndfile_wrap_seek;
   priv->vio_wrapper.read = _sndfile_wrap_read;
   priv->vio_wrapper.write = _sndfile_wrap_write;
   priv->vio_wrapper.tell = _sndfile_wrap_tell;

   ECORE_MAGIC_SET(module, ECORE_MAGIC_AUDIO_MODULE);
   module->type = ECORE_AUDIO_TYPE_SNDFILE;
   module->name = "sndfile";
   module->priv = priv;
   module->inputs = NULL;
   module->outputs = NULL;
   module->in_ops = &inops;
   module->out_ops = &outops;

   DBG("Initialized");
   return module;
}

/**
 * @brief Shut down the Ecore_Audio sndfile module
 */
void
ecore_audio_sndfile_shutdown(void)
{
   struct _Ecore_Audio_Sndfile_Private *priv = (struct _Ecore_Audio_Sndfile_Private *)module->priv;

   free(priv);
   free(module);
   module = NULL;

   DBG("Shutting down");
}

#endif /* HAVE_SNDFILE */
