/*
 * Remix ALSA Player: ALSA audio output
 *
 * Govindaraju SM <govi.sm@samsung.com>, October 2011
 * Prince Kumar Dubey <prince.dubey@samsung.com>, October 2011
 */

#include "config.h"
#include <stdio.h>
#include <remix/remix.h>
#include <alsa/asoundlib.h>
#include <Eina.h>

#define ALSA_PLAYER_BUFFERLEN 2048

typedef struct _AlsaPlayerData AlsaPlayerData;
typedef short PLAYER_PCM;

struct _AlsaPlayerData
{
   RemixPCM databuffer[ALSA_PLAYER_BUFFERLEN];
   snd_pcm_t *alsa_dev;
   unsigned int stereo;
   unsigned channels;
   unsigned int frequency;
};

/* Optimisation dependencies: none */
static RemixBase *alsa_player_optimise(RemixEnv *env, RemixBase *base);

static snd_pcm_t *
alsa_open(int channels, unsigned samplerate)
{
   const char *device = "default";
   snd_pcm_t *alsa_dev = NULL;
   snd_pcm_hw_params_t *hw_params;
   snd_pcm_uframes_t alsa_buffer_frames;
   snd_pcm_uframes_t alsa_period_size;
   int err;
   
   alsa_buffer_frames = ALSA_PLAYER_BUFFERLEN;
   alsa_period_size = ALSA_PLAYER_BUFFERLEN / 4;
   
   if ((err = snd_pcm_open(&alsa_dev, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
     {
        printf("cannot open alsa playback stream (%s)\n", snd_strerror(err));
        goto catch_error;
     }
   if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0)
     {
        printf("cannot alloc snd hw params (%s)\n", snd_strerror(err));
        goto catch_error;
     }
   if ((err = snd_pcm_hw_params_any(alsa_dev, hw_params)) < 0)
     {
        printf("cannot initialize snd hw params (%s)\n", snd_strerror(err));
        goto catch_error;
     }
   if ((err = snd_pcm_hw_params_set_access(alsa_dev, hw_params, 
                                           SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
     {
         printf("cannot set access type (%s)\n", snd_strerror(err));
         goto catch_error;
     }
   if ((err = snd_pcm_hw_params_set_format(alsa_dev, hw_params, 
                                           SND_PCM_FORMAT_FLOAT)) < 0)
     {
        // FIXME: handle if float format not possible
        printf("cannot set sample format (%s)\n", snd_strerror(err));
        goto catch_error;
     }
   if ((err = snd_pcm_hw_params_set_rate_near(alsa_dev, hw_params, 
                                              &samplerate, 0)) < 0)
     {
        // FIXME: get actual sample rate and tell remix
        printf("cannot set sample rate (%s)\n", snd_strerror(err));
        goto catch_error;
     }
   if ((err = snd_pcm_hw_params_set_channels(alsa_dev, hw_params, channels)) < 0)
     {
        printf("cannot set channel count (%s)\n", snd_strerror(err));
        goto catch_error;
     }
   if ((err = snd_pcm_hw_params_set_buffer_size_near(alsa_dev, hw_params, 
                                                     &alsa_buffer_frames)) < 0)
     {
        fprintf(stderr, "cannot set buffer size (%s)\n", snd_strerror(err));
        goto catch_error;
     }
   if ((err = snd_pcm_hw_params_set_period_size_near(alsa_dev, hw_params, 
                                                     &alsa_period_size, 0)) < 0)
     {
        fprintf(stderr, "cannot set period size (%s)\n", snd_strerror(err));
        goto catch_error;
     }
   if ((err = snd_pcm_hw_params(alsa_dev, hw_params)) < 0)
     {
        printf("cannot set parameters (%s)\n", snd_strerror(err));
        goto catch_error;
     }
   snd_pcm_hw_params_free(hw_params);
   if ((err = snd_pcm_prepare(alsa_dev)) < 0)
     {
        printf("cannot prepare audio for use (%s)\n", snd_strerror(err));
        goto catch_error;
     }

catch_error:
   if ((err < 0) && (alsa_dev != NULL))
     {
        snd_pcm_close(alsa_dev);
        return NULL;
     }
   return alsa_dev;
}

static RemixBase *
alsa_player_reset_device(RemixEnv *env, RemixBase *base)
{
   AlsaPlayerData *player_data = remix_base_get_instance_data(env, base);

   if (player_data->alsa_dev)
     {
        snd_pcm_drain(player_data->alsa_dev);
        snd_pcm_close(player_data->alsa_dev);
     }
   player_data->alsa_dev = alsa_open(player_data->channels,
                                     player_data->frequency);
   if (!player_data->alsa_dev)
     {
        remix_set_error(env, REMIX_ERROR_SYSTEM);
        return RemixNone;
     }
   return base;
}

static RemixBase *
alsa_player_init(RemixEnv *env, RemixBase *base, CDSet *parameters __UNUSED__)
{
   CDSet *channels;
   AlsaPlayerData *player_data = calloc(1, sizeof(AlsaPlayerData));

   if (!player_data)
     {
        remix_set_error(env, REMIX_ERROR_SYSTEM);
        return RemixNone;
     }
   
   remix_base_set_instance_data(env, base, player_data);
   channels = remix_get_channels(env);

   player_data->channels = cd_set_size(env, channels);
   if (player_data->channels == 1) player_data->stereo = 0;
   else if (player_data->channels == 2) player_data->stereo = 1;
   
   player_data->frequency = remix_get_samplerate(env);
   alsa_player_reset_device(env, base);
   base = alsa_player_optimise(env, base);
   return base;
}

static RemixBase *
alsa_player_clone(RemixEnv *env, RemixBase *base __UNUSED__)
{
   RemixBase *new_player = remix_base_new(env);
   alsa_player_init(env, new_player, NULL);
   return new_player;
}

static int
alsa_player_destroy(RemixEnv *env, RemixBase *base)
{
   AlsaPlayerData *player_data = remix_base_get_instance_data(env, base);
   
   if (player_data->alsa_dev)
     {
        snd_pcm_drain(player_data->alsa_dev);
        snd_pcm_close(player_data->alsa_dev);
     }
   free(player_data);
   return 0;
}

static int
alsa_player_ready(RemixEnv *env, RemixBase *base)
{
   AlsaPlayerData *player_data = remix_base_get_instance_data(env, base);
   RemixCount nr_channels;
   CDSet *channels;
   int samplerate;

   channels = remix_get_channels(env);
   samplerate = (int)remix_get_samplerate(env);
   nr_channels = cd_set_size(env, channels);
   return ((samplerate == (int)player_data->frequency) &&
           (((nr_channels == 1) && (player_data->stereo == 0)) ||
               ((nr_channels > 1) && (player_data->stereo == 1))));
}

static RemixBase *
alsa_player_prepare(RemixEnv *env, RemixBase *base)
{
   alsa_player_reset_device(env, base);
   return base;
}

static RemixCount
alsa_player_playbuffer(RemixEnv *env __UNUSED__, AlsaPlayerData *player, RemixPCM *data, RemixCount count)
{
   return snd_pcm_writei(player->alsa_dev, data, count);
}

static RemixCount
alsa_player_chunk(RemixEnv *env, RemixChunk *chunk, RemixCount offset, RemixCount count, int channelname __UNUSED__, void *data)
{
   AlsaPlayerData *player = data;
   RemixCount remaining = count, written = 0, n, playcount;
   RemixPCM *d;
   
   while (remaining > 0)
     {
        playcount = MIN(remaining, ALSA_PLAYER_BUFFERLEN);

        d = &chunk->data[offset];
        n = alsa_player_playbuffer(env, player, d, playcount);

        if (n == -1) return -1;
        else n /= sizeof(PLAYER_PCM);
        
        offset += n;
        written += n;
        remaining -= n;
     }
   return written;
}

static RemixCount
alsa_player_process(RemixEnv *env, RemixBase *base, RemixCount count, RemixStream *input, RemixStream *output __UNUSED__)
{
   AlsaPlayerData *player_data = remix_base_get_instance_data(env, base);
   RemixCount nr_channels = remix_stream_nr_channels(env, input);
   RemixCount remaining = count, processed = 0, n, nn;
   
   if ((nr_channels == 1) && (player_data->stereo == 0))
     { /*MONO*/
        return remix_stream_chunkfuncify(env, input, count,
                                         alsa_player_chunk, player_data);
     }
   else if ((nr_channels == 2) && (player_data->stereo == 1))
     { /*STEREO*/
        while (processed < count)
          {
             n = MIN(remaining, ALSA_PLAYER_BUFFERLEN);
             n = remix_stream_interleave_2(env, input, 
                                           REMIX_CHANNEL_LEFT,
                                           REMIX_CHANNEL_RIGHT,
                                           player_data->databuffer, n);
             nn = alsa_player_playbuffer(env, player_data,
                                         player_data->databuffer, n);
             processed += n;
             remaining -= n;
          }
        return processed;
     }
   printf("[alsa_player_process] unsupported stream/output channel\n");
   printf("combination %ld / %d\n", nr_channels, player_data->stereo ? 2 : 1);
   return -1;
}

static RemixCount
alsa_player_length(RemixEnv *env __UNUSED__, RemixBase *base __UNUSED__)
{
   return REMIX_COUNT_INFINITE;
}

static RemixCount
alsa_player_seek(RemixEnv *env __UNUSED__, RemixBase *base __UNUSED__, RemixCount count __UNUSED__)
{
   return count;
}

static int
alsa_player_flush(RemixEnv *env, RemixBase *base)
{
   alsa_player_reset_device(env, base);
   return 0;
}

static struct _RemixMethods _alsa_player_methods =
{
   alsa_player_clone,
   alsa_player_destroy,
   alsa_player_ready,
   alsa_player_prepare,
   alsa_player_process,
   alsa_player_length,
   alsa_player_seek,
   alsa_player_flush,
};

static RemixBase *
alsa_player_optimise(RemixEnv *env, RemixBase *base)
{
   remix_base_set_methods(env, base, &_alsa_player_methods);
   return base;
}

static struct _RemixMetaText alsa_player_metatext =
{
   "alsa_snd_player",
   "ALSA sound player for Remix",
   "Output the audio stream into ALSA Driver",
   "Copyright (C) 2011, Samsung Electronics Co., Ltd.",
   "http://www.samsung.com",
   REMIX_ONE_AUTHOR("Govindaraju SM", "prince.dubey@samsung.com"),
};

static struct _RemixPlugin alsa_player_plugin =
{
   &alsa_player_metatext,
   REMIX_FLAGS_NONE,
   CD_EMPTY_SET, /* init scheme */
   alsa_player_init,
   CD_EMPTY_SET, /* process scheme */
   NULL, /* suggests */
   NULL, /* plugin data */
   NULL  /* destroy */
};

EAPI CDList *
remix_load(RemixEnv *env)
{
   CDList *plugins = cd_list_new(env);
   plugins = cd_list_prepend(env, plugins, CD_POINTER(&alsa_player_plugin));
   return plugins;
}
