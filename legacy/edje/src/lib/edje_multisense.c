#include "config.h"
#include <string.h>
#include <fcntl.h>
#include <Eina.h>
#include <Edje.h>
#include "edje_private.h"

typedef struct _Multisense_Data
{
   Edje_Multisense_Env *msenv;
#ifdef HAVE_LIBREMIX
   RemixDeck *deck;
   RemixTrack *track;
   RemixLayer *snd_layer, *player_layer;
   RemixBase *player;
   RemixBase *player_snd;
   int remaining;
   int offset;
   Eina_List *snd_src_list;

   MULTISENSE_SOUND_PLAYER_GET_FUNC multisense_sound_player_get;
#endif
}Multisense_Data;

#define BUF_LEN 64
#define SND_PROCESS_LENGTH 2048

static Ecore_Thread *player_thread = NULL;
static Eina_Bool pipe_initialized = EINA_FALSE;
static int command_pipe[2];

typedef enum _Edje_Sound_Action_Type
{
   EDJE_PLAY_SAMPLE = 0,
   EDJE_PLAY_TONE,
   /*
   EDJE_PLAY_PATTERN,
   EDJE_PLAY_INSTRUMENT,
   EDJE_PLAY_SONG,
   */
   EDJE_SOUND_LAST
} Edje_Sound_Action_Type;

typedef struct _Edje_Sample_Action Edje_Sample_Action;
typedef struct _Edje_Tone_Action Edje_Tone_Action;
typedef struct _Edje_Multisense_Sound_Action Edje_Multisense_Sound_Action;

struct _Edje_Sample_Action
{
   char sample_name[BUF_LEN];
   double speed;
};

struct _Edje_Tone_Action
{
   char tone_name[BUF_LEN];
   double duration;
};

struct _Edje_Multisense_Sound_Action
{
   Edje *ed;
   Edje_Sound_Action_Type action;
   union {
      Edje_Sample_Action sample;
      Edje_Tone_Action tone;
   } type;
};

static Multisense_Data *
init_multisense_environment(void)
{
   Multisense_Data *msdata;
   char ms_factory[BUF_LEN];
   char *ms_factory_env;
   Eina_Module *m = NULL;
   MULTISENSE_FACTORY_INIT_FUNC multisense_factory_init;

   msdata = calloc(1, sizeof(Multisense_Data));
   if (!msdata) goto err;

   msdata->msenv = calloc(1, sizeof(Edje_Multisense_Env));
   if (!msdata->msenv) goto err;

   ms_factory_env = getenv("MULTISENSE_FACTORY");
   if (ms_factory_env)
     strncpy(ms_factory, ms_factory_env, BUF_LEN);
   else
     strcpy(ms_factory, "multisense_factory");
   
   m = edje_module_load(ms_factory);
   if (!m) goto err;
   
#ifdef HAVE_LIBREMIX
   msdata->msenv->remixenv = remix_init();
#endif

   multisense_factory_init = 
     eina_module_symbol_get(m, "multisense_factory_init");
   if (multisense_factory_init) multisense_factory_init(msdata->msenv);

#ifdef HAVE_LIBREMIX
   msdata->multisense_sound_player_get = 
     eina_module_symbol_get(m, "multisense_sound_player_get");
   if (!msdata->multisense_sound_player_get) goto err;

   msdata->deck = remix_deck_new(msdata->msenv->remixenv);
   msdata->track = remix_track_new(msdata->msenv->remixenv, msdata->deck);
   msdata->snd_layer = remix_layer_new_ontop(msdata->msenv->remixenv,
                                             msdata->track,
                                             REMIX_TIME_SAMPLES);
   msdata->player_layer = remix_layer_new_ontop(msdata->msenv->remixenv,
                                                msdata->track,
                                                REMIX_TIME_SAMPLES);
   msdata->player = msdata->multisense_sound_player_get(msdata->msenv);
   if (!msdata->player) goto err;
   msdata->player_snd = remix_sound_new(msdata->msenv->remixenv,
                                        msdata->player, msdata->player_layer,
                                        REMIX_SAMPLES(0),
                                        REMIX_SAMPLES(REMIX_COUNT_INFINITE));
#endif
   return msdata;

err:
   if (msdata)
     {
#ifdef HAVE_LIBREMIX
        if (msdata->deck) remix_destroy(msdata->msenv->remixenv, msdata->deck);
        if (msdata->msenv->remixenv) remix_purge(msdata->msenv->remixenv);
#endif
        if (msdata->msenv) free(msdata->msenv);
        free(msdata);
     }
   return NULL;
}

#ifdef HAVE_LIBREMIX
static RemixBase *
eet_sound_reader_get(Edje_Multisense_Env *msenv, const char *path, const char *sound_id, const double speed)
{
   RemixPlugin *sf_plugin = NULL;
   RemixBase * eet_snd_reader = NULL;
   int sf_path_key = 0;
   int sf_sound_id_key = 0;
   int sf_speed_key = 0;
   CDSet *sf_parms = NULL;
   RemixEnv *env = msenv->remixenv;

   if (sf_plugin == NULL)
     {
        sf_plugin = remix_find_plugin(env, "eet_sndfile_reader");
        if (sf_plugin == NULL)
          {
             ERR ("Multisense EET Sound reader plugin NULL\n");
             return NULL;
          }
        
        sf_path_key = remix_get_init_parameter_key(env, sf_plugin, "path");
        sf_sound_id_key = remix_get_init_parameter_key(env, sf_plugin, "sound_id");
        sf_speed_key = remix_get_init_parameter_key(env, sf_plugin, "speed");
     }
   sf_parms = cd_set_replace(env, sf_parms, sf_path_key, CD_STRING(path));
   sf_parms = cd_set_replace(env, sf_parms, sf_sound_id_key, CD_STRING(sound_id));
   sf_parms = cd_set_replace(env, sf_parms, sf_speed_key, CD_DOUBLE(speed));
   eet_snd_reader = remix_new(env, sf_plugin, sf_parms);
   
   return eet_snd_reader;
}


static RemixBase *
edje_remix_sample_create(Multisense_Data *msdata, Edje*ed, Edje_Sample_Action *action)
{
   RemixBase *remix_snd = NULL;
   Edje_Sound_Sample *sample;
   int i;
   char snd_id_str[16];

   if ((!ed) || (!ed->file) || (!ed->file->sound_dir))
     return NULL;

   for (i = 0; i < (int)ed->file->sound_dir->samples_count; i++)
     {
        sample = &ed->file->sound_dir->samples[i];
        if (!strcmp(sample->name, action->sample_name))
          {
             snprintf(snd_id_str, sizeof(snd_id_str), "edje/sounds/%i", sample->id);
             remix_snd = eet_sound_reader_get(msdata->msenv, ed->file->path,
                                              snd_id_str, action->speed);
             break;
          }
     }
   return remix_snd;
}

static RemixBase *
edje_remix_tone_create(Multisense_Data *msdata, Edje*ed, Edje_Tone_Action *action)
{
   Edje_Sound_Tone *tone;
   RemixSquareTone *square;
   unsigned int i;

   if ((!ed) || (!ed->file) || (!ed->file->sound_dir))
     return NULL;
   
   for (i = 0; i < ed->file->sound_dir->tones_count; i++)
     {
        tone = &ed->file->sound_dir->tones[i];
        if (!strcmp(tone->name, action->tone_name))
          {
             square = remix_squaretone_new (msdata->msenv->remixenv, tone->value);
             break;
          }
     }
   return square;
}

static void
sound_command_handler(Multisense_Data *msdata)
{
   RemixCount length;
   Edje_Multisense_Sound_Action command;
   RemixBase *base = NULL;
   RemixBase *sound;
   
   if (read(command_pipe[0], &command, sizeof(command)) <= 0) return;
   switch (command.action)
     {
      case EDJE_PLAY_SAMPLE:
        base = edje_remix_sample_create(msdata, command.ed,
                                        &command.type.sample);
        length = remix_length(msdata->msenv->remixenv, base);
        break;
      case EDJE_PLAY_TONE:
        base = edje_remix_tone_create(msdata, command.ed, &command.type.tone);
        length = (command.type.tone.duration *
                              remix_get_samplerate(msdata->msenv->remixenv));
        break;
      default:
        ERR("Invalid Sound Play Command\n");
        break;
     }
   if (base)
     {
        sound = remix_sound_new(msdata->msenv->remixenv, base, msdata->snd_layer,
                                REMIX_SAMPLES(msdata->offset),
                                REMIX_SAMPLES(length));
        if (msdata->remaining < length) msdata->remaining = length;
        msdata->snd_src_list = eina_list_append(msdata->snd_src_list, sound);
        msdata->snd_src_list = eina_list_append(msdata->snd_src_list, base);
     }
}
#endif

static void
_player_job(void *data __UNUSED__, Ecore_Thread *th)
{
   fd_set wait_fds;
#ifdef HAVE_LIBREMIX
   RemixBase *sound;
   RemixCount process_len;
#endif
   Multisense_Data *msdata = init_multisense_environment();
   if (!msdata) return;

   fcntl(command_pipe[0], F_SETFL, O_NONBLOCK);
   FD_ZERO(&wait_fds);
   FD_SET(command_pipe[0], &wait_fds);

#ifdef HAVE_LIBREMIX
   while (!ecore_thread_check(th))
     {
        if (!msdata->remaining)
          {
             //Cleanup already played sound sources
             EINA_LIST_FREE(msdata->snd_src_list, sound)
               {
                  remix_destroy(msdata->msenv->remixenv, sound);
               }
             //wait for new sound
             select(command_pipe[0] + 1, &wait_fds, NULL, NULL, 0);
          }
        //read sound command , if any
        sound_command_handler(msdata);
        process_len = MIN(msdata->remaining, SND_PROCESS_LENGTH);
        remix_process(msdata->msenv->remixenv, msdata->deck, process_len,
                      RemixNone, RemixNone);
        msdata->offset += process_len;
        msdata->remaining -= process_len;
     }

   //Cleanup last played sound sources
   EINA_LIST_FREE(msdata->snd_src_list, sound)
     {
        remix_destroy(msdata->msenv->remixenv, sound);
     }

   //cleanup Remix stuffs
   remix_destroy(msdata->msenv->remixenv, msdata->player);
   remix_destroy(msdata->msenv->remixenv, msdata->deck);
   remix_purge(msdata->msenv->remixenv);
#endif

   free(msdata->msenv);
   free(msdata);
   close(command_pipe[0]);
   close(command_pipe[1]);
}

static void
_player_cancel(void *data __UNUSED__, Ecore_Thread *th __UNUSED__)
{
   player_thread = NULL;
}

static void
_player_end(void *data __UNUSED__, Ecore_Thread *th __UNUSED__)
{
   player_thread = NULL;
}

Eina_Bool
_edje_multisense_internal_sound_sample_play(Edje *ed, const char *sample_name, const double speed)
{
   ssize_t size = 0;
#ifdef ENABLE_MULTISENSE
   Edje_Multisense_Sound_Action command;
   
   if ((!pipe_initialized) && (!player_thread)) return EINA_FALSE;

   command.action = EDJE_PLAY_SAMPLE;
   command.ed = ed;
   strncpy(command.type.sample.sample_name, sample_name, BUF_LEN);
   command.type.sample.speed = speed;
   size = write(command_pipe[1], &command, sizeof(command));
#endif
   return (size == sizeof(Edje_Multisense_Sound_Action));
}

Eina_Bool
_edje_multisense_internal_sound_tone_play(Edje *ed, const char *tone_name, const double duration)
{
   ssize_t size = 0;
#ifdef ENABLE_MULTISENSE
   Edje_Multisense_Sound_Action command;
   
   if ((!pipe_initialized) && (!player_thread)) return EINA_FALSE;
   command.action = EDJE_PLAY_TONE;
   command.ed = ed;
   strncpy(command.type.tone.tone_name, tone_name, BUF_LEN);
   command.type.tone.duration = duration;
   size = write(command_pipe[1], &command, sizeof(command));
#endif
   return (size == sizeof(Edje_Multisense_Sound_Action));

}

/* Initialize the modules in main thread. to avoid dlopen issue in the Threads */
void
_edje_multisense_init(void)
{
#ifdef ENABLE_MULTISENSE
   if (!pipe_initialized && (pipe(command_pipe) != -1))
     pipe_initialized = EINA_TRUE;
   
   if (!player_thread)
     player_thread = ecore_thread_run(_player_job, _player_end, _player_cancel, NULL);
#endif
}

void
_edje_multisense_shutdown(void)
{
#ifdef ENABLE_MULTISENSE
   if (pipe_initialized)
     {
        close(command_pipe[1]);
        close(command_pipe[0]);
     }
   if (player_thread) ecore_thread_cancel(player_thread);
#endif
}
