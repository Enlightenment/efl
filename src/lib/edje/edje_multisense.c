#include "edje_private.h"

#ifdef ENABLE_MULTISENSE
#include "Ecore_Audio.h"

static Eo *out = NULL;
static int outs = 0;
static Eina_Bool outfail = EINA_FALSE;

static void
_play_finished(void *data EINA_UNUSED, const Efl_Event *event)
{
   efl_unref(event->object);
}

static void
_out_fail(void *data EINA_UNUSED, const Efl_Event *event)
{
   outfail = EINA_TRUE;
   efl_unref(event->object);
   out = NULL;
}

struct _edje_multisense_eet_data
{
   unsigned int    offset, length;
   Eet_File       *ef;
   const char     *data;
   Ecore_Audio_Vio vio;
};

static int
eet_snd_file_get_length(void *data, Eo *eo_obj EINA_UNUSED)
{
   struct _edje_multisense_eet_data *vf = data;
   return vf->length;
}

static int
eet_snd_file_seek(void *data, Eo *eo_obj EINA_UNUSED, int offset, int whence)
{
   struct _edje_multisense_eet_data *vf = data;

   switch (whence)
     {
      case SEEK_SET:
        vf->offset = offset;
        break;

      case SEEK_CUR:
        vf->offset += offset;
        break;

      case SEEK_END:
        vf->offset = vf->length + offset;
        break;

      default:
        break;
     }
   return vf->offset;
}

static int
eet_snd_file_read(void *data, Eo *eo_obj EINA_UNUSED, void *buffer, int count)
{
   struct _edje_multisense_eet_data *vf = data;

   if ((vf->offset + count) > vf->length)
     count = vf->length - vf->offset;
   memcpy(buffer, vf->data + vf->offset, count);
   vf->offset += count;
   return count;
}

static int
eet_snd_file_tell(void *data, Eo *eo_obj EINA_UNUSED)
{
   struct _edje_multisense_eet_data *vf = data;

   return vf->offset;
}

static void
_free(void *data)
{
   struct _edje_multisense_eet_data *eet_data = data;

   if (eet_data->ef) eet_close(eet_data->ef);
// don't free if eet_data->data  comes from eet_read_direct
//  free(eet_data->data);
   free(data);
   outs--;
}

static Eina_Bool _channel_mute_states[8] = { 0 };

static Eina_Bool
_channel_mute(Edje *ed EINA_UNUSED, int channel)
{
   // ed lets use set mute per object... but for now no api's for this
   // if all are muted ... then all!
   if (_channel_mute_states[7]) return EINA_TRUE;
   if ((channel < 0) || (channel > 7)) return EINA_FALSE;
   return _channel_mute_states[channel];
   return EINA_FALSE;
}

#endif

EAPI void
edje_audio_channel_mute_set(Edje_Channel channel, Eina_Bool mute)
{
#ifdef ENABLE_MULTISENSE
   if ((unsigned)channel > 7) return;
   _channel_mute_states[channel] = mute;
#else
   (void)channel;
   (void)mute;
#endif
}

EAPI Eina_Bool
edje_audio_channel_mute_get(Edje_Channel channel)
{
#ifdef ENABLE_MULTISENSE
   if ((unsigned)channel > 7) return EINA_FALSE;
   return _channel_mute_states[channel];
#else
   (void)channel;
   return EINA_TRUE;
#endif
}

Eina_Bool
_edje_multisense_internal_sound_sample_play(Edje *ed, const char *sample_name, const double speed, int channel)
{
#ifdef ENABLE_MULTISENSE
   Eo *in;
   Edje_Sound_Sample *sample;
   char snd_id_str[255];
   int i;
   Eina_Bool ret = EINA_FALSE;

   if (_channel_mute(ed, channel)) return EINA_FALSE;

   if (outfail) return EINA_FALSE;

   if (!sample_name)
     {
        ERR("Given Sample Name is NULL\n");
        return EINA_FALSE;
     }

   if ((!ed) || (!ed->file) || (!ed->file->sound_dir))
     return EINA_FALSE;

   for (i = 0; i < (int)ed->file->sound_dir->samples_count; i++)
     {
        sample = &ed->file->sound_dir->samples[i];
        if (!strcmp(sample->name, sample_name))
          {
             struct _edje_multisense_eet_data *eet_data;
             int len;

             snprintf(snd_id_str, sizeof(snd_id_str), "edje/sounds/%i", sample->id);

             eet_data = calloc(1, sizeof(struct _edje_multisense_eet_data));
             if (!eet_data)
               {
                  ERR("Out of memory in allocating multisense sample info");
                  return EINA_FALSE;
               }
             // open eet file again to esnure we have  reference because we
             // use eet_read_direct to avoid duplicating/copying into memory
             // by relying on a direct mmap, but this means we need to close
             // the eet file handle instead of freeing data
             eet_data->ef = eet_mmap(ed->file->f);
             if (!eet_data->ef)
               {
                  ERR("Cannot open edje file '%s' for samples", ed->path);
                  free(eet_data);
                  return EINA_FALSE;
               }
             eet_data->data = eet_read_direct(eet_data->ef, snd_id_str, &len);
             if (len <= 0)
               {
                  ERR("Sample form edj file '%s' is 0 length", ed->path);
                  eet_close(eet_data->ef);
                  free(eet_data);
                  return EINA_FALSE;
               }
             eet_data->length = len;
             /* action->speed */

             eet_data->vio.get_length = eet_snd_file_get_length;
             eet_data->vio.seek = eet_snd_file_seek;
             eet_data->vio.read = eet_snd_file_read;
             eet_data->vio.tell = eet_snd_file_tell;
             eet_data->offset = 0;

             in = efl_add_ref(ECORE_AUDIO_IN_SNDFILE_CLASS, NULL, efl_name_set(efl_added, snd_id_str), ecore_audio_obj_in_speed_set(efl_added, speed), ecore_audio_obj_vio_set(efl_added, &eet_data->vio, eet_data, _free), efl_event_callback_add(efl_added, ECORE_AUDIO_IN_EVENT_IN_STOPPED, _play_finished, NULL));
             if (!out)
               {

# ifdef _WIN32
                  out = efl_add_ref(ECORE_AUDIO_OUT_WASAPI_CLASS, NULL, efl_event_callback_add(efl_added, ECORE_AUDIO_OUT_WASAPI_EVENT_CONTEXT_FAIL, _out_fail, NULL));
# else
#  ifdef HAVE_PULSE
                  out = efl_add_ref(ECORE_AUDIO_OUT_PULSE_CLASS, NULL, efl_event_callback_add(efl_added, ECORE_AUDIO_OUT_PULSE_EVENT_CONTEXT_FAIL, _out_fail, NULL));
#  endif
# endif
                  if (out) outs++;
               }
             if (!out)
               {
                  static Eina_Bool complained = EINA_FALSE;

                  if (!complained)
                    {
                       complained = EINA_TRUE;
# ifdef _WIN32
                       ERR("Could not create multisense audio out (wasapi)");
# else
#  ifdef HAVE_PULSE
                       ERR("Could not create multisense audio out (pulse)");
#  endif
# endif
                    }
                  efl_unref(in);
                  return EINA_FALSE;
               }
             ret = ecore_audio_obj_out_input_attach(out, in);
             if (!ret)
               {
                  static Eina_Bool complained = EINA_FALSE;

                  if (!complained)
                    {
                       complained = EINA_TRUE;
                       ERR("Could not attach input");
                    }
                  efl_unref(in);
                  return EINA_FALSE;
               }
          }
     }
   return EINA_TRUE;
#else
   // warning shh
   (void)ed;
   (void)sample_name;
   (void)speed;
   (void)channel;
   return EINA_FALSE;
#endif
}

Eina_Bool
_edje_multisense_internal_sound_tone_play(Edje *ed, const char *tone_name, const double duration, int channel)
{
#ifdef ENABLE_MULTISENSE
   unsigned int i;
   Edje_Sound_Tone *tone;
   Eina_Bool ret = EINA_FALSE;

   Eo *in;
   if (!tone_name)
     {
        ERR("Given Tone Name is NULL");
        return EINA_FALSE;
     }

   if (_channel_mute(ed, channel)) return EINA_FALSE;

   if (outfail) return EINA_FALSE;

   if ((!ed) || (!ed->file) || (!ed->file->sound_dir))
     return EINA_FALSE;

   for (i = 0; i < ed->file->sound_dir->tones_count; i++)
     {
        tone = &ed->file->sound_dir->tones[i];
        if (!strcmp(tone->name, tone_name))
          {
             in = efl_add_ref(ECORE_AUDIO_IN_TONE_CLASS, NULL);
             efl_name_set(in, "tone");
             efl_key_data_set(in, ECORE_AUDIO_ATTR_TONE_FREQ, &tone->value);
             ecore_audio_obj_in_length_set(in, duration);
             efl_event_callback_add(in, ECORE_AUDIO_IN_EVENT_IN_STOPPED, _play_finished, NULL);

             if (!out)
               {
# ifdef _WIN32
                  out = efl_add_ref(ECORE_AUDIO_OUT_WASAPI_CLASS, NULL, efl_event_callback_add(efl_added, ECORE_AUDIO_OUT_WASAPI_EVENT_CONTEXT_FAIL, _out_fail, NULL));
# else
#  ifdef HAVE_PULSE
                  out = efl_add_ref(ECORE_AUDIO_OUT_PULSE_CLASS, NULL, efl_event_callback_add(efl_added, ECORE_AUDIO_OUT_PULSE_EVENT_CONTEXT_FAIL, _out_fail, NULL));
#  endif
# endif
                  if (out) outs++;
               }

             ret = ecore_audio_obj_out_input_attach(out, in);
             if (!ret)
               {
                  ERR("Could not attach input");
                  efl_unref(in);
                  return EINA_FALSE;
               }
          }
     }
   return EINA_TRUE;
#else
   // warning shh
   (void)ed;
   (void)duration;
   (void)tone_name;
   (void)channel;
   return EINA_FALSE;
#endif
}

Eina_Bool
_edje_multisense_internal_vibration_sample_play(Edje *ed EINA_UNUSED, const char *sample_name EINA_UNUSED, int repeat EINA_UNUSED)
{
#ifdef ENABLE_MULTISENSE
   ERR("Vibration is not supported yet, name:%s, repeat:%d", sample_name, repeat);
   return EINA_FALSE;
#else
   (void)ed;
   (void)repeat;
   return EINA_FALSE;
#endif
}

void
_edje_multisense_init(void)
{
#ifdef ENABLE_MULTISENSE
   ecore_audio_init();
#endif
}

void
_edje_multisense_shutdown(void)
{
#ifdef ENABLE_MULTISENSE
   if (outs > 0)
     {
        WRN("Shutting down audio while samples still playing");
     }
   if (out)
     {
        // XXX: this causes an abort inside of pa!!!!!
        //efl_unref(out);
        out = NULL;
        outs = 0;
     }
   ecore_audio_shutdown();
#endif
}

