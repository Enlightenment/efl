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

int _ecore_audio_log_dom = -1;
static int _ecore_audio_init_count = 0;
Eina_List *ecore_audio_modules;

EAPI int ECORE_AUDIO_INPUT_STARTED;
EAPI int ECORE_AUDIO_INPUT_STOPPED;
EAPI int ECORE_AUDIO_INPUT_LOOPED;
EAPI int ECORE_AUDIO_INPUT_ENDED;
EAPI int ECORE_AUDIO_INPUT_PAUSED;
EAPI int ECORE_AUDIO_OUTPUT_INPUT_ADDED;
EAPI int ECORE_AUDIO_OUTPUT_INPUT_REMOVED;

static void _dummy_free(void *foo EINA_UNUSED, void *bar EINA_UNUSED)
{
}

static int
module_cmp(const void *a, const void *b)
{
   Ecore_Audio_Module *mod = (Ecore_Audio_Module *)a;
   Ecore_Audio_Type type = *(Ecore_Audio_Type *)b;

   return !(mod->type == type);
}

static Ecore_Audio_Module *
get_module_by_type(Ecore_Audio_Type type)
{
   return eina_list_search_unsorted(ecore_audio_modules, module_cmp, &type);
}

/* externally accessible functions */

EAPI int
ecore_audio_init(void)
{
   Ecore_Audio_Module *mod;

   if (++_ecore_audio_init_count != 1)
     return _ecore_audio_init_count;

   if (!ecore_init())
     return --_ecore_audio_init_count;

   _ecore_audio_log_dom = eina_log_domain_register("ecore_audio", ECORE_AUDIO_DEFAULT_LOG_COLOR);
   if (_ecore_audio_log_dom < 0)
     {
        EINA_LOG_ERR("Impossible to create a log domain for the ecore audio module.");
        return --_ecore_audio_init_count;
     }

   DBG("Ecore_Audio init");
   ecore_audio_modules = NULL;

   ECORE_AUDIO_INPUT_STARTED = ecore_event_type_new();
   ECORE_AUDIO_INPUT_STOPPED = ecore_event_type_new();
   ECORE_AUDIO_INPUT_LOOPED = ecore_event_type_new();
   ECORE_AUDIO_INPUT_ENDED = ecore_event_type_new();
   ECORE_AUDIO_INPUT_PAUSED = ecore_event_type_new();
   ECORE_AUDIO_OUTPUT_INPUT_ADDED = ecore_event_type_new();
   ECORE_AUDIO_OUTPUT_INPUT_REMOVED = ecore_event_type_new();

#ifdef HAVE_ALSA
   mod = ecore_audio_alsa_init();
   if (mod)
     ecore_audio_modules = eina_list_append(ecore_audio_modules, mod);
#endif
#ifdef HAVE_PULSE
   mod = ecore_audio_pulse_init();
   if (mod)
     ecore_audio_modules = eina_list_append(ecore_audio_modules, mod);
#endif
#ifdef HAVE_SNDFILE
   mod = ecore_audio_sndfile_init();
   if (mod)
     ecore_audio_modules = eina_list_append(ecore_audio_modules, mod);
#endif

   mod = ecore_audio_tone_init();
   if (mod)
     ecore_audio_modules = eina_list_append(ecore_audio_modules, mod);

   mod = ecore_audio_custom_init();
   if (mod)
     ecore_audio_modules = eina_list_append(ecore_audio_modules, mod);

   return _ecore_audio_init_count;
}

EAPI int
ecore_audio_shutdown(void)
{
   DBG("Ecore_Audio shutdown");
   if (--_ecore_audio_init_count != 0)
     return _ecore_audio_init_count;

   /* FIXME: Shutdown all the inputs and outputs first */

#ifdef HAVE_ALSA
   ecore_audio_alsa_shutdown();
#endif
#ifdef HAVE_PULSE
   ecore_audio_pulse_shutdown();
#endif
#ifdef HAVE_SNDFILE
   ecore_audio_sndfile_shutdown();
#endif
   ecore_audio_tone_shutdown();
   ecore_audio_custom_shutdown();

   eina_list_free(ecore_audio_modules);

   eina_log_domain_unregister(_ecore_audio_log_dom);
   _ecore_audio_log_dom = -1;

   ecore_shutdown();

   return _ecore_audio_init_count;
}

/* Output operations */

EAPI Ecore_Audio_Object *
ecore_audio_output_add(Ecore_Audio_Type type)
{
   Ecore_Audio_Output *out;
   Ecore_Audio_Module *module;

   module = get_module_by_type(type);

   if (!module)
     return NULL;

   out = calloc(1, sizeof(Ecore_Audio_Output));
   if (!out)
     {
        ERR("Could not allocate memory for output.");
        return NULL;
     }

   ECORE_MAGIC_SET(out, ECORE_MAGIC_AUDIO_OUTPUT);
   out->module = module;
   out->inputs = NULL;

   return module->out_ops->output_new((Ecore_Audio_Object *)out);
}

EAPI void
ecore_audio_output_del(Ecore_Audio_Object *output)
{
   Ecore_Audio_Output *out = (Ecore_Audio_Output *)output;
   Eina_Bool del;
   Ecore_Audio_Object *in;

   EINA_SAFETY_ON_NULL_RETURN(out);

   /* Cleanup */
   EINA_LIST_FREE (out->inputs, in)
     {
        del = ecore_audio_output_input_del(output, in);
        if (!del)
          WRN("Disconnecting in %p and out %p failed.", in, output);
     }

   out->module->out_ops->output_del(output);
   eina_stringshare_del(output->name);
   free(output);
}

EAPI void ecore_audio_output_userdata_set(Ecore_Audio_Object *output, void *data)
{
  Ecore_Audio_Output *out = (Ecore_Audio_Output *)output;
  out->user_data = data;
}

EAPI void *ecore_audio_output_userdata_get(Ecore_Audio_Object *output)
{
  Ecore_Audio_Output *out = (Ecore_Audio_Output *)output;
  return out->user_data;
}

EAPI const char *ecore_audio_output_name_get(Ecore_Audio_Object *output)
{
   Ecore_Audio_Output *out = (Ecore_Audio_Output *)output;
   EINA_SAFETY_ON_NULL_RETURN_VAL(out, NULL);

   return out->name;
}
EAPI void ecore_audio_output_name_set(Ecore_Audio_Object *output, const char *name)
{
   Ecore_Audio_Output *out = (Ecore_Audio_Output *)output;
   EINA_SAFETY_ON_NULL_RETURN(out);

   if (out->name)
     eina_stringshare_del(out->name);

   out->name = eina_stringshare_add(name);
}

EAPI void
ecore_audio_output_volume_set(Ecore_Audio_Object *output, double volume)
{
   EINA_SAFETY_ON_NULL_RETURN(output);

   Ecore_Audio_Module *outmod = output->module;

   outmod->out_ops->output_volume_set(output, volume);
}

EAPI double
ecore_audio_output_volume_get(Ecore_Audio_Object *output)
{
  EINA_SAFETY_ON_NULL_RETURN_VAL(output, 0);
  Ecore_Audio_Module *outmod = output->module;

  return outmod->out_ops->output_volume_get(output);
}

EAPI void
ecore_audio_output_paused_set(Ecore_Audio_Object *output, Eina_Bool paused)
{
  EINA_SAFETY_ON_NULL_RETURN(output);
  Ecore_Audio_Module *outmod = output->module;

  outmod->out_ops->output_paused_set(output, paused);
}

EAPI Eina_Bool
ecore_audio_output_paused_get(Ecore_Audio_Object *output)
{
  EINA_SAFETY_ON_NULL_RETURN_VAL(output, EINA_TRUE);

  return output->paused;
}

EAPI Eina_Bool
ecore_audio_output_input_add(Ecore_Audio_Object *output, Ecore_Audio_Object *input)
{
   Ecore_Audio_Output *out = (Ecore_Audio_Output *)output;
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(input, EINA_FALSE);
   Ecore_Audio_Module *outmod = output->module;
   Eina_Bool ret;

   if (in->output != NULL)
     ecore_audio_output_input_del((Ecore_Audio_Object *)in->output, input);

   ret = outmod->out_ops->output_add_input(output, input);
   if (ret)
     {
        in->output = out;
        out->inputs = eina_list_append(out->inputs, in);
        ecore_event_add(ECORE_AUDIO_OUTPUT_INPUT_ADDED, in, _dummy_free, NULL);
     }

   return ret;
}

EAPI Eina_Bool
ecore_audio_output_input_del(Ecore_Audio_Object *output, Ecore_Audio_Object *input)
{
   Ecore_Audio_Output *out = (Ecore_Audio_Output *)output;
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(input, EINA_FALSE);
   Ecore_Audio_Module *outmod = output->module;
   Eina_Bool ret;

   ret = outmod->out_ops->output_del_input(output, input);
   if (ret)
     {
        in->output = NULL;
        out->inputs = eina_list_remove(out->inputs, in);
        ecore_event_add(ECORE_AUDIO_OUTPUT_INPUT_REMOVED, in, _dummy_free, NULL);
     }

   return ret;
}

EAPI Eina_List          *ecore_audio_output_inputs_get(Ecore_Audio_Object *output)
{
  Ecore_Audio_Output *out = (Ecore_Audio_Output *)output;
  EINA_SAFETY_ON_NULL_RETURN_VAL(out, EINA_FALSE);
  return out->inputs;
}

EAPI Eina_Bool           ecore_audio_output_input_chain_after(Ecore_Audio_Object *output, Ecore_Audio_Object *after, Ecore_Audio_Object *input)
{
  Ecore_Audio_Output *out = (Ecore_Audio_Output *)output;
  Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
  EINA_SAFETY_ON_NULL_RETURN_VAL(out, EINA_FALSE);
  EINA_SAFETY_ON_NULL_RETURN_VAL(in, EINA_FALSE);
  EINA_SAFETY_ON_NULL_RETURN_VAL(after, EINA_FALSE);

  if (!eina_list_data_find(out->inputs, after))
    return EINA_FALSE;

  /* XXX: implement */
  return EINA_FALSE;
}


/* Input operations */

EAPI Ecore_Audio_Object *
ecore_audio_input_add(Ecore_Audio_Type type)
{
   Ecore_Audio_Input *in;
   Ecore_Audio_Module *module;

   module = get_module_by_type(type);

   if (!module)
     return NULL;

   in = calloc(1, sizeof(Ecore_Audio_Input));
   if (!in)
     {
        ERR("Could not allocate memory for input.");
        return NULL;
     }

   ECORE_MAGIC_SET(in, ECORE_MAGIC_AUDIO_INPUT);
   in->module = module;
   in->output = NULL;
   in->paused = EINA_FALSE;

   return module->in_ops->input_new((Ecore_Audio_Object *)in);
}

EAPI void
ecore_audio_input_del(Ecore_Audio_Object *input)
{
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
   Eina_Bool del;
   Ecore_Audio_Module *module = input->module;

   if (in->output) {
       del = ecore_audio_output_input_del((Ecore_Audio_Object *)in->output, input);
       if (!del)
         WRN("Disconnecting in %p and out %p failed.", input, in->output);
       in->output = NULL;
   }

   module->in_ops->input_del(input);
   eina_stringshare_del(in->name);
   free(in);
}

EAPI void ecore_audio_input_userdata_set(Ecore_Audio_Object *input, void *data)
{
  Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
  in->user_data = data;
}

EAPI void *ecore_audio_input_userdata_get(Ecore_Audio_Object *input)
{
  Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
  return in->user_data;
}

EAPI const char *ecore_audio_input_name_get(Ecore_Audio_Object *input)
{
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
   EINA_SAFETY_ON_NULL_RETURN_VAL(in, NULL);
   return in->name;
}

EAPI void ecore_audio_input_name_set(Ecore_Audio_Object *input, const char *name)
{
  Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
  EINA_SAFETY_ON_NULL_RETURN(in);

  if (in->name)
    eina_stringshare_del(in->name);

  in->name = eina_stringshare_add(name);
}

EAPI int ecore_audio_input_samplerate_get(Ecore_Audio_Object *input)
{
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
   EINA_SAFETY_ON_NULL_RETURN_VAL(in, -1);

   return in->samplerate;
}

EAPI void ecore_audio_input_samplerate_set(Ecore_Audio_Object *input, int samplerate)
{
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
   EINA_SAFETY_ON_NULL_RETURN(input);
   Ecore_Audio_Module *outmod;

   if (in->samplerate == samplerate)
     return;

   in->samplerate = samplerate;

   if (in->output)
     {
        outmod = in->output->module;
        outmod->out_ops->output_update_input_format((Ecore_Audio_Object *)in->output, input);
     }
}

EAPI int ecore_audio_input_channels_get(Ecore_Audio_Object *input)
{
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
   EINA_SAFETY_ON_NULL_RETURN_VAL(in, -1);

   return in->channels;
}

EAPI void ecore_audio_input_channels_set(Ecore_Audio_Object *input, int channels)
{
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
   EINA_SAFETY_ON_NULL_RETURN(in);

   if (in->channels == channels)
     return;

   in->channels = channels;

/* XXX: Change channel number for connected streams?
   if (in->output)
     {
        outmod = in->output->module;
        outmod->out_ops->output_update_input_format(in->output, in);
     } */
}

EAPI int ecore_audio_input_read(Ecore_Audio_Object *input, void *data, int len)
{
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
   int ret;

   EINA_SAFETY_ON_NULL_RETURN_VAL(in, 0);
   Ecore_Audio_Module *inmod = input->module;

   if (in->paused)
     {
        memset(data, 0, len);
        return len;
     }

   ret = inmod->in_ops->input_read(input, data, len);
   if (ret == 0)
     {
        if (!in->looped)
          {
             ecore_event_add(ECORE_AUDIO_INPUT_ENDED, in, _dummy_free, NULL);
          }
        else
          {
             inmod->in_ops->input_seek(input, 0, SEEK_SET);
             ret = inmod->in_ops->input_read(input, data, len);
             ecore_event_add(ECORE_AUDIO_INPUT_LOOPED, input, _dummy_free, NULL);
          }
     }

  return ret;
}

EAPI double
ecore_audio_input_seek(Ecore_Audio_Object *input, double offs, int mode)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(input, -1);
   Ecore_Audio_Module *inmod = input->module;

   return inmod->in_ops->input_seek(input, offs, mode);
}

EAPI Eina_Bool
ecore_audio_input_paused_get(Ecore_Audio_Object *input)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(input, EINA_FALSE);

   return input->paused;
}

EAPI void
ecore_audio_input_paused_set(Ecore_Audio_Object *input, Eina_Bool paused)
{
   EINA_SAFETY_ON_NULL_RETURN(input);

   if (paused == input->paused)
     return;

   input->paused = paused;
   ecore_event_add(ECORE_AUDIO_INPUT_PAUSED, input, _dummy_free, NULL);

}

EAPI void
ecore_audio_input_volume_set(Ecore_Audio_Object *input, double volume)
{
  EINA_SAFETY_ON_NULL_RETURN(input);
  Ecore_Audio_Module *inmod = input->module;

  inmod->in_ops->input_volume_set(input, volume);
}

EAPI double
ecore_audio_input_volume_get(Ecore_Audio_Object *input)
{
  EINA_SAFETY_ON_NULL_RETURN_VAL(input, -1);
  Ecore_Audio_Module *inmod = input->module;

  return inmod->in_ops->input_volume_get(input);
}

EAPI void
ecore_audio_input_looped_set(Ecore_Audio_Object *input, Eina_Bool looped)
{
  Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
  EINA_SAFETY_ON_NULL_RETURN(input);

  in->looped = looped;
}

EAPI Eina_Bool
ecore_audio_input_looped_get(Ecore_Audio_Object *input)
{
  Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
  EINA_SAFETY_ON_NULL_RETURN_VAL(input, EINA_FALSE);

  return in->looped;
}

EAPI double
ecore_audio_input_length_get(Ecore_Audio_Object *input)
{
  Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
  EINA_SAFETY_ON_NULL_RETURN_VAL(input, -1);

  return in->length;
}

EAPI void
ecore_audio_input_preloaded_set(Ecore_Audio_Object *input, Eina_Bool preloaded)
{
  EINA_SAFETY_ON_NULL_RETURN(input);
  Ecore_Audio_Module *inmod = input->module;

  inmod->in_ops->input_preloaded_set(input, preloaded);
}

EAPI Eina_Bool
ecore_audio_input_preloaded_get(Ecore_Audio_Object *input)
{
  Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
  EINA_SAFETY_ON_NULL_RETURN_VAL(input, EINA_FALSE);
  return in->preloaded;
}

EAPI Ecore_Audio_Object *
ecore_audio_input_output_get(Ecore_Audio_Object *input)
{
  Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
  EINA_SAFETY_ON_NULL_RETURN_VAL(input, NULL);
  return (Ecore_Audio_Object *)in->output;
}

EAPI double
ecore_audio_input_remaining_get(Ecore_Audio_Object *input)
{
  Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
  EINA_SAFETY_ON_NULL_RETURN_VAL(input, -1);
  Ecore_Audio_Module *inmod = input->module;

  return in->length - inmod->in_ops->input_seek(input, 0, SEEK_CUR);
}

/* XXX: Error checking!!! */
EAPI void
ecore_audio_input_callback_setup(Ecore_Audio_Object *input, Ecore_Audio_Read_Callback read_cb, void *data)
{
  EINA_SAFETY_ON_NULL_RETURN(input);
  struct _Ecore_Audio_Callback *cb = input->module_data;

  cb->read_cb = read_cb;
  cb->data = data;
}

/**
 * @}
 */
