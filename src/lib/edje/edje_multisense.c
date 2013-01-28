#include "edje_private.h"
#include "Ecore_Audio.h"
#include <sndfile.h>

static Ecore_Audio_Object *out = NULL;

static Eina_Bool _play_finished(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Audio_Object *in = (Ecore_Audio_Object *)event;
   ecore_audio_input_del(in);
   return EINA_FALSE;
}

struct _edje_multisense_eet_data
{
   sf_count_t  offset, length;
   const char *data;
   Ecore_Audio_Vio vio;
};

static int
eet_snd_file_get_length(Ecore_Audio_Object *in)
{
   struct _edje_multisense_eet_data *vf = ecore_audio_input_userdata_get(in);
   return vf->length;
}

static int
eet_snd_file_seek(Ecore_Audio_Object *in, int offset, int whence)
{
   struct _edje_multisense_eet_data *vf = ecore_audio_input_userdata_get(in);

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
eet_snd_file_read(Ecore_Audio_Object *in, void *buffer, int count)
{
   struct _edje_multisense_eet_data *vf = ecore_audio_input_userdata_get(in);

   if ((vf->offset + count) > vf->length)
     count = vf->length - vf->offset;
   memcpy(buffer, vf->data + vf->offset, count);
   vf->offset += count;
   return count;
}

static int
eet_snd_file_tell(Ecore_Audio_Object *in)
{
   struct _edje_multisense_eet_data *vf = ecore_audio_input_userdata_get(in);

   return vf->offset;
}

Eina_Bool
_edje_multisense_internal_sound_sample_play(Edje *ed, const char *sample_name, const double speed EINA_UNUSED)
{
 #ifdef ENABLE_MULTISENSE
   Ecore_Audio_Object *in;
   Edje_Sound_Sample *sample;
   char snd_id_str[255];
   int i;

    if (!sample_name)
      {
         ERR("Given Sample Name is NULL\n");
         return EINA_FALSE;
      }

   if ((!ed) || (!ed->file) || (!ed->file->sound_dir))
     return EINA_FALSE;

   for(i=0; i<(int)ed->file->sound_dir->samples_count; i++)
     {
       sample = &ed->file->sound_dir->samples[i];
       if (!strcmp(sample->name, sample_name))
         {
            struct _edje_multisense_eet_data *eet_data;
            Eet_File *ef;

            snprintf(snd_id_str, sizeof(snd_id_str), "edje/sounds/%i", sample->id);
            in = ecore_audio_input_add(ECORE_AUDIO_TYPE_SNDFILE);
            ecore_audio_input_name_set(in, snd_id_str);

            eet_data = calloc(1, sizeof(struct _edje_multisense_eet_data));
            ef = eet_open(ed->file->path, EET_FILE_MODE_READ);

            eet_data->data = eet_read(ef, snd_id_str, (int *)&eet_data->length);
            eet_close(ef);

            /* action->speed */

            eet_data->vio.get_length = eet_snd_file_get_length;
            eet_data->vio.seek = eet_snd_file_seek;
            eet_data->vio.read = eet_snd_file_read;
            eet_data->vio.tell = eet_snd_file_tell;

            eet_data->offset = 0;

            ecore_audio_input_userdata_set(in, eet_data);
            ecore_audio_input_sndfile_vio_set(in, &eet_data->vio);

            if (!out)
              out = ecore_audio_output_add(ECORE_AUDIO_TYPE_PULSE);

            ecore_audio_output_input_add(out, in);
         }
     }
   return EINA_TRUE;
#else
   // warning shh
   (void) ed;
   (void) sample_name;
   (void) speed;
   return EINA_FALSE;
#endif
}

Eina_Bool
_edje_multisense_internal_sound_tone_play(Edje *ed, const char *tone_name, const double duration)
{
#ifdef ENABLE_MULTISENSE
   unsigned int i;
   Edje_Sound_Tone *tone;

   Ecore_Audio_Object *in;
   if (!tone_name)
     {
        ERR("Given Tone Name is NULL\n");
        return EINA_FALSE;
     }
   if ((!ed) || (!ed->file) || (!ed->file->sound_dir))
     return EINA_FALSE;

   for (i=0; i<ed->file->sound_dir->tones_count; i++)
     {
        tone = &ed->file->sound_dir->tones[i];
        if (!strcmp(tone->name, tone_name))
          {
             in = ecore_audio_input_add(ECORE_AUDIO_TYPE_TONE);
             ecore_audio_input_name_set(in, "tone");
             ecore_audio_input_tone_frequency_set(in, tone->value);
             ecore_audio_input_tone_duration_set(in, duration);

             if (!out)
               out = ecore_audio_output_add(ECORE_AUDIO_TYPE_PULSE);

             ecore_audio_output_input_add(out, in);
          }
     }
   return EINA_TRUE;
#else
   // warning shh
   (void) ed;
   (void) duration;
   (void) tone_name;
   return EINA_FALSE;
#endif

}

/* Initialize the modules in main thread. to avoid dlopen issue in the Threads */
void
_edje_multisense_init(void)
{
#ifdef ENABLE_MULTISENSE
   ecore_audio_init();
   ecore_event_handler_add(ECORE_AUDIO_INPUT_ENDED, _play_finished, NULL);
#endif
}

void
_edje_multisense_shutdown(void)
{
#ifdef ENABLE_MULTISENSE
   ecore_audio_shutdown();
#endif
}
