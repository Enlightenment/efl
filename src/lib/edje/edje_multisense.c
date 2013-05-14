#include "edje_private.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef ENABLE_MULTISENSE

#ifdef HAVE_PULSE
#include "ecore_audio_obj_out_pulse.h"
#define MY_CLASS ECORE_AUDIO_OBJ_OUT_PULSE_CLASS
#elif HAVE_ALSA
#include "ecore_audio_obj_out_alsa.h"
#define MY_CLASS ECORE_AUDIO_OBJ_OUT_ALSA_CLASS
#else
#error "Multisense needs Pulseaudio suport!"
#endif

#include <sndfile.h>
#include "Ecore_Audio.h"

static Eo *out = NULL;
static int outs = 0;

static Eina_Bool _play_finished(void *data EINA_UNUSED, Eo *in, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
  eo_del(in);

   return EINA_TRUE;
}

struct _edje_multisense_eet_data
{
   sf_count_t  offset, length;
   Eet_File *ef;
   const char *data;
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

static void _free(void *data)
{
   struct _edje_multisense_eet_data *eet_data = data;

   if (eet_data->ef) eet_close(eet_data->ef);
// don't free if eet_data->data  comes from eet_read_direct   
//  free(eet_data->data);
   free(data);
   outs--;
}
#endif

Eina_Bool
_edje_multisense_internal_sound_sample_play(Edje *ed, const char *sample_name, const double speed)
{
 #ifdef ENABLE_MULTISENSE
   Eo *in;
   Edje_Sound_Sample *sample;
   char snd_id_str[255];
   int i;
   Eina_Bool ret;

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

            in = eo_add(ECORE_AUDIO_OBJ_IN_SNDFILE_CLASS, NULL,
                        ecore_audio_obj_name_set(snd_id_str),
                        ecore_audio_obj_in_speed_set(speed),
                        ecore_audio_obj_vio_set(&eet_data->vio, eet_data, _free),
                        eo_event_callback_add(ECORE_AUDIO_EV_IN_STOPPED, _play_finished, NULL));
            if (!out)
              {
                 out = eo_add(MY_CLASS, NULL);
                 if (out) outs++;
              }
            if (!out)
              {
                 ERR("Could not create multisense audio out (pulse)");
                 eo_del(in);
                 return EINA_FALSE;
              }
            eo_do(out, ecore_audio_obj_out_input_attach(in, &ret));
            if (!ret)
              {
                 ERR("Could not attach input");
                 eo_del(in);
                 return EINA_FALSE;
              }
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
   Eina_Bool ret;

   Eo *in;
   if (!tone_name)
     {
        ERR("Given Tone Name is NULL");
        return EINA_FALSE;
     }
   if ((!ed) || (!ed->file) || (!ed->file->sound_dir))
     return EINA_FALSE;

   for (i=0; i<ed->file->sound_dir->tones_count; i++)
     {
        tone = &ed->file->sound_dir->tones[i];
        if (!strcmp(tone->name, tone_name))
          {
             in = eo_add(ECORE_AUDIO_OBJ_IN_TONE_CLASS, NULL);
             eo_do(in, ecore_audio_obj_name_set("tone"));
             eo_do(in, eo_base_data_set(ECORE_AUDIO_ATTR_TONE_FREQ, &tone->value, NULL));
             eo_do(in, ecore_audio_obj_in_length_set(duration));
             eo_do(in, eo_event_callback_add(ECORE_AUDIO_EV_IN_STOPPED, _play_finished, NULL));

             if (!out)
               out = eo_add(MY_CLASS, NULL);

             eo_do(out, ecore_audio_obj_out_input_attach(in, &ret));
             if (!ret) {
               ERR("Could not attach input");
               eo_del(in);
               return EINA_FALSE;
             }
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
        //eo_del(out);
        out = NULL;
        outs = 0;
     }
   ecore_audio_shutdown();
#endif
}
