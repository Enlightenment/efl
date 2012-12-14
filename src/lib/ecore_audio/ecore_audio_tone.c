#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Audio.h"
#include "ecore_audio_private.h"

#include <math.h>

static Ecore_Audio_Module *module = NULL;

static Ecore_Audio_Object *
_tone_input_new(Ecore_Audio_Object *input)
{
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
   struct _Ecore_Audio_Tone *tone;

   tone = calloc(1, sizeof(struct _Ecore_Audio_Tone));
   if (!tone)
     {
        ERR("Could not allocate memory for private structure.");
        free(in);
        return NULL;
     }

   in->samplerate = 44100;
   in->channels = 1;

   in->module_data = tone;

   in->length = 1;
   tone->freq = 1000;
   tone->phase = 0;

   return (Ecore_Audio_Object *)in;
}

static void
_tone_input_del(Ecore_Audio_Object *input)
{
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
   free(in->module_data);
}

static int
_tone_input_read(Ecore_Audio_Object *input, void *data, int len)
{
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
   struct _Ecore_Audio_Tone *tone = in->module_data;
   float *val = data;
   int i, remain;

   remain = in->length * in->samplerate * 4 - tone->phase * 4;
   if (remain > len)
     remain = len;

   for (i = 0; i < remain / 4; i++)
     {
        val[i] = sin(2 * M_PI * tone->freq * (tone->phase + i) / in->samplerate);
     }
   tone->phase += i;

   return remain;
}

static double
_tone_input_seek(Ecore_Audio_Object *input, double offs, int mode)
{
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
   struct _Ecore_Audio_Tone *tone = in->module_data;
   int tmp;

   switch (mode) {
     case SEEK_SET:
       tmp = offs * in->samplerate;
       break;
     case SEEK_CUR:
       tmp = tone->phase + offs * in->samplerate;
       break;
     case SEEK_END:
       tmp = (in->length + offs) * in->samplerate;
       break;
     default:
       return -1;
   }

   if ((tmp < 0) || (tmp > in->length * in->samplerate))
     return -1;

   tone->phase = tmp;
   return (double)tone->phase / in->samplerate;
}

static struct input_api inops = {
   .input_new = _tone_input_new,
   .input_del = _tone_input_del,
   .input_read = _tone_input_read,
   .input_seek = _tone_input_seek,
};

EAPI void ecore_audio_input_tone_frequency_set(Ecore_Audio_Object *input, int freq)
{
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;
   struct _Ecore_Audio_Tone *tone = in->module_data;

   tone->freq = freq;
}

EAPI void ecore_audio_input_tone_duration_set(Ecore_Audio_Object *input, double duration)
{
   Ecore_Audio_Input *in = (Ecore_Audio_Input *)input;

   in->length = duration;
}


/**
 * @brief Initialize the Ecore_Audio sndfile module
 *
 * @return the initialized module on success, NULL on error
 */
Ecore_Audio_Module *
ecore_audio_tone_init(void)
{
   module = calloc(1, sizeof(Ecore_Audio_Module));
   if (!module)
     {
        ERR("Could not allocate memory for module.");
        return NULL;
     }

   ECORE_MAGIC_SET(module, ECORE_MAGIC_AUDIO_MODULE);
   module->type = ECORE_AUDIO_TYPE_TONE;
   module->name = "tone";
   module->priv = NULL;
   module->inputs = NULL;
   module->outputs = NULL;
   module->in_ops = &inops;
   module->out_ops = NULL;

   DBG("Initialized");
   return module;
}

/**
 * @brief Shut down the Ecore_Audio sndfile module
 */
void
ecore_audio_tone_shutdown(void)
{
   free(module);
   module = NULL;

   DBG("Shutting down");
}

