/*
 * RemixSnd_eetfile: a libsnd EET Virtual file handler
 *
 * Govindaraju SM <govi.sm@samsung.com>, August 2011
 * Prince Kumar Dubey <prince.dubey@samsung.com>, August 2011
 */

#include "config.h"
#include <math.h>
#include <sndfile.h>
#include <remix/remix.h>
#include <Eet.h>

#define PATH_KEY     1
#define SOUND_ID_KEY 2
#define SPEED_KEY    3
#define BLOCK_FRAMES 8192

static RemixBase *remix_eet_sndfile_optimise(RemixEnv *env, RemixBase *sndfile);

typedef struct _VIO_DATA VIO_DATA;
typedef struct _SndInstanceData SndInstanceData;

struct _VIO_DATA
{
   sf_count_t  offset, length;
   const char *data;
};

struct _SndInstanceData
{
   /* plugin parameters */
   char       *path;
   char       *sound_id;
   double      speed;

   /* Edj & Sndfile Reader */
   Eet_File   *efp;
   SNDFILE    *pcm_fp;
   SF_INFO    *snd_info;
   VIO_DATA   *vio_data;

   /* PCM buffers */
   RemixPCM   *readbuf;
   RemixPCM   *inbuf;
   RemixPCM   *outbuf;

   /* Resample stuffs */
   RemixPCM    prevreadbuf[2];
   int         enable_resample;
   double      rs_ratio;
   RemixCount  resample_len;
   RemixCount  in_avail;
   RemixCount  out_generated;
   RemixCount  required_resamples;
};

static sf_count_t
eet_snd_file_get_length(void *user_data)
{
   VIO_DATA *vf = user_data;
   return vf->length;
}

static sf_count_t
eet_snd_file_seek(sf_count_t offset, int whence, void *user_data)
{
   VIO_DATA *vf = user_data;
   
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

static sf_count_t
eet_snd_file_read(void *ptr, sf_count_t count, void *user_data)
{
   VIO_DATA *vf = user_data;

   if ((vf->offset + count) > vf->length)
     count = vf->length - vf->offset;
   memcpy(ptr, vf->data + vf->offset, count);
   vf->offset += count;
   return count;
}

static sf_count_t
eet_snd_file_tell(void *user_data)
{
   VIO_DATA *vf = user_data;
   return vf->offset;
}

static int
remix_init_resampler_data(RemixEnv *env, RemixBase *base)
{
   SndInstanceData *si = remix_base_get_instance_data(env, base);

   si->rs_ratio = remix_get_samplerate(env) / si->snd_info->samplerate;
   si->rs_ratio /= si->speed;
   si->resample_len = (si->snd_info->frames * si->rs_ratio);

   si->outbuf = malloc(sizeof(RemixPCM) * BLOCK_FRAMES * 2);
   if (!si->outbuf) return 0;
   if ((si->rs_ratio == 1.0)/* && (si->snd_info->channels == 2)*/)
     {
        si->enable_resample = 0;
        return 1;
     }
   else
     si->enable_resample = 1;

   si->in_avail = 0;
   si->out_generated = 0;
   si->inbuf = malloc(sizeof(RemixPCM) * BLOCK_FRAMES *
                      si->snd_info->channels);
   if (!si->inbuf) return 0;
   return 1;
}

static RemixBase *
remix_eet_sndfile_create(RemixEnv *env, RemixBase *sndfile, const char *path, const char *sound_id, const double speed)
{
   SF_VIRTUAL_IO *eet_vio;
   SndInstanceData *si;
   const void *sound_data;
   int sound_size;

   if ((!path) || (!sound_id)) return NULL;

   si = calloc(1, sizeof(SndInstanceData));
   if (!si) goto err;
   remix_base_set_instance_data(env, sndfile, si);

   si->path = strdup(path);
   si->sound_id = strdup(sound_id);
   si->speed = speed;

   si->efp = eet_open(path, EET_FILE_MODE_READ);
   if (!si->efp) goto err;

   // xxx: eet_read_direct does not work on Threads, using eet_read.
   sound_data = eet_read(si->efp, sound_id, &(sound_size));
   eet_close(si->efp);
   si->efp = NULL;
   if (sound_data == NULL) goto err;

   eet_vio = calloc(1, sizeof(SF_VIRTUAL_IO));
   if (!eet_vio) goto err;

   /* Set up func pointers to read snd file directly from EET. */
   eet_vio->get_filelen = eet_snd_file_get_length;
   eet_vio->seek = eet_snd_file_seek;
   eet_vio->read = eet_snd_file_read;
   eet_vio->tell = eet_snd_file_tell;

   si->vio_data = calloc(1, sizeof(VIO_DATA));
   if (!si->vio_data) goto err;
   si->vio_data->offset = 0;
   si->vio_data->length = sound_size;
   si->vio_data->data = sound_data;

   si->snd_info = calloc(1, sizeof(SF_INFO));
   if (!si->snd_info) goto err;

   si->pcm_fp = sf_open_virtual(eet_vio, SFM_READ, si->snd_info, si->vio_data);
   if (!si->pcm_fp) goto err;
   free(eet_vio);
   eet_vio = NULL;

   if (!remix_init_resampler_data(env, sndfile)) goto err;
   si->out_generated = 0;

   return sndfile;

err:
   if (eet_vio) free(eet_vio);
   remix_set_error(env, REMIX_ERROR_SYSTEM);
   remix_destroy(env, (RemixBase *)sndfile);
   return RemixNone;
}

static RemixBase *
remix_eet_sndfile_reader_init(RemixEnv *env, RemixBase *base, CDSet *parameters)
{
   char *file_path, *sound_id;
   double speed;

   file_path = (cd_set_find(env, parameters, PATH_KEY)).s_string;
   sound_id = (cd_set_find(env, parameters, SOUND_ID_KEY)).s_string;
   speed = (cd_set_find(env, parameters, SPEED_KEY)).s_double;

   if (!remix_eet_sndfile_create(env, base, file_path, sound_id, speed))
     return RemixNone;
   remix_eet_sndfile_optimise (env, base);
   return base;
}

static RemixBase *
remix_eet_sndfile_clone(RemixEnv *env, RemixBase *base)
{
   SndInstanceData *si = remix_base_get_instance_data(env, base);
   RemixBase *new_sndfile = remix_base_new(env);

   remix_eet_sndfile_create(env, new_sndfile, si->path, si->sound_id, si->speed);
   remix_eet_sndfile_optimise(env, new_sndfile);
   return new_sndfile;
}

static int
remix_eet_sndfile_destroy(RemixEnv *env, RemixBase *base)
{
   SndInstanceData *si = remix_base_get_instance_data(env, base);
   if (si)
     {
        sf_close (si->pcm_fp);
        eet_close(si->efp);
        if (si->path) free(si->path);
        if (si->sound_id) free(si->sound_id);
        if (si->snd_info) free(si->snd_info);
        if (si->efp) eet_close(si->efp);
        if (si->inbuf) free(si->inbuf);
        if (si->outbuf) free(si->outbuf);
        if (si->vio_data) free(si->vio_data);
        free(si);
      }
   if (base) free (base);
   return 0;
}

static int
remix_pcm_resample(SndInstanceData *si)
{
   RemixPCM *src, *dst, *srcbase;
   int i = 0, in_samples, pos, total, chnum, reqsamp, avail;
   int interp = 1;
   
   dst = si->outbuf + (si->out_generated * 2);
   in_samples = (double)si->required_resamples / si->rs_ratio;
   chnum = si->snd_info->channels;
   reqsamp = si->required_resamples;
   avail = si->in_avail;
   srcbase = si->readbuf;
   if ((interp) && (si->rs_ratio >= 1.0))
     {
        // linear interpolation of resampling for lower quality samples
        // so they don't get high requency aliasing effects
        for (i = 0; i < reqsamp; i++)
          {
             float fpos, fpos1;
             RemixPCM psam[2];
             
             fpos = (float)(i * in_samples) / (float)reqsamp;
             pos = fpos;
             if (pos >= avail) break;
             fpos -= pos;
             fpos1 = 1.0 - fpos;
             src = srcbase + (pos * chnum);
             if (chnum == 2)
               {
                  if (i == 0)
                    {
                       psam[0] = si->prevreadbuf[0];
                       psam[1] = si->prevreadbuf[1];
                    }
                  else
                    {
                       psam[0] = src[0 - 2];
                       psam[1] = src[1 - 2];
                    }
                  *dst++ = (src[0] * fpos) + (psam[0] * fpos1);
                  *dst++ = (src[1] * fpos) + (psam[1] * fpos1);
               }
             else
               {
                  if (i == 0)
                    psam[0] = si->prevreadbuf[0];
                  else
                    psam[0] = src[0 - 1];
                  *dst++ = (src[0] * fpos) + (psam[0] * fpos1);
               }
          }
     }
   else
     {
        // simple sample-picking/nearest. faster and simpler
        for (i = 0; i < reqsamp; i++)
          {
             pos = (i * in_samples) / reqsamp;
             if (pos >= avail) break;
             src = srcbase + (pos * chnum);
             if (chnum == 2)
               {
                  *dst++ = src[0];
                  *dst++ = src[1];
               }
             else
               *dst++ = src[0];
          }
     }
   si->out_generated += i;
   total = (i * in_samples) / reqsamp;
   si->readbuf += total * chnum;
   si->in_avail -= total;
   return total;
}

/* An RemixChunkFunc for creating sndfile */
static RemixCount
remix_eet_sndfile_read_update(RemixEnv *env, RemixBase *sndfile, RemixCount count)
{
   SndInstanceData *si = remix_base_get_instance_data(env, sndfile);
   
   si->out_generated = 0;
   if (si->enable_resample)
     {
        RemixCount gen = 0;

        while (gen < count)
          {
             if (si->in_avail <= 0)
               {
                  si->in_avail = sf_readf_float(si->pcm_fp, si->inbuf, BLOCK_FRAMES);
                  si->readbuf = si->inbuf;
               }
             si->required_resamples = (count - gen);
             remix_pcm_resample(si);
             if (si->snd_info->channels == 2)
               {
                  si->prevreadbuf[0] = si->readbuf[-2];
                  si->prevreadbuf[1] = si->readbuf[-1];
               }
             else
               {
                  si->prevreadbuf[0] = si->readbuf[-1];
               }
             gen += si->out_generated;
          }
        si->out_generated = gen;
     }
   else
     {
        si->out_generated = sf_readf_float(si->pcm_fp, si->outbuf, count);
     }
   return si->out_generated;
}

static RemixCount
remix_eet_sndfile_read_into_chunk(RemixEnv *env, RemixChunk *chunk, RemixCount offset, RemixCount count, int channelname, void *data)
{
   RemixBase *sndfile = data;
   SndInstanceData *si = remix_base_get_instance_data(env, sndfile);
   RemixPCM *d, *p;
   RemixCount remaining = count, written = 0, n, i;

   d = &chunk->data[offset];
   n = MIN(remaining, BLOCK_FRAMES);
   // Need parameter support to advance the data reading
   if (channelname == 0)
     remix_eet_sndfile_read_update(env, sndfile, n);
   n = MIN(si->out_generated, remaining);
   p = si->outbuf;
   if (si->snd_info->channels > 1) p += channelname;
   for (i = 0; i < n; i++)
     {
        *d++ = *p;
        p += si->snd_info->channels;
     }
   if (n == 0) n = _remix_pcm_set(d, 0.0, remaining);
   remaining -= n;
   written += n;
   return written;
}

static RemixCount
remix_eet_sndfile_reader_process(RemixEnv *env, RemixBase *base, RemixCount count, RemixStream *input __UNUSED__, RemixStream *output)
{
   return remix_stream_chunkfuncify(env, output, count, 
                                    remix_eet_sndfile_read_into_chunk,
                                    base);
}

static RemixCount
remix_eet_sndfile_length(RemixEnv *env, RemixBase *base)
{
   SndInstanceData *si = remix_base_get_instance_data(env, base);
   return si->resample_len;
}

static RemixCount
remix_eet_sndfile_seek(RemixEnv *env, RemixBase *base, RemixCount offset)
{
   SndInstanceData *si = remix_base_get_instance_data(env, base);
   return sf_seek(si->pcm_fp, offset, SEEK_SET);
}

static struct _RemixMethods _remix_eet_sndfile_reader_methods =
{
   remix_eet_sndfile_clone,
   remix_eet_sndfile_destroy,
   NULL, /* ready */
   NULL, /* prepare */
   remix_eet_sndfile_reader_process,
   remix_eet_sndfile_length,
   remix_eet_sndfile_seek,
   NULL, /* flush */
};

static RemixBase *
remix_eet_sndfile_optimise(RemixEnv *env, RemixBase *sndfile)
{
   remix_base_set_methods(env, sndfile, &_remix_eet_sndfile_reader_methods);
   return sndfile;
}

static struct _RemixParameterScheme path_scheme =
{
   "path",
   "Path to sound file",
   REMIX_TYPE_STRING,
   REMIX_CONSTRAINT_TYPE_NONE,
   {NULL},
   REMIX_HINT_FILENAME,
};

static struct _RemixParameterScheme sound_id_scheme =
{
   "sound_id",
   "Sound Id (Key) inside EET",
   REMIX_TYPE_STRING,
   REMIX_CONSTRAINT_TYPE_NONE,
   {NULL},
   REMIX_HINT_DEFAULT,
};

static struct _RemixParameterScheme speed_scheme =
{
   "speed",
   "Sound Play Speed",
   REMIX_TYPE_FLOAT,
   REMIX_CONSTRAINT_TYPE_NONE,
   {NULL},
   REMIX_HINT_DEFAULT,
};

static struct _RemixMetaText eet_sndfile_reader_metatext =
{
   "eet_sndfile_reader",
   "File:: Sound file Reader from EET",
   "Reads PCM audio files from EET bundle using libsndfile",
   "Copyright (C) 2011, Samsung Electronics Co., Ltd.",
   "http://www.samsung.com",
   REMIX_ONE_AUTHOR ("govi.sm@samsung.com", "prince.dubey@samsung.com"),
};

static struct _RemixPlugin eet_sndfile_reader_plugin =
{
   &eet_sndfile_reader_metatext,
   REMIX_FLAGS_NONE,
   CD_EMPTY_SET, /* init scheme */
   remix_eet_sndfile_reader_init,
   CD_EMPTY_SET, /* process scheme */
   NULL, /* suggests */
   NULL, /* plugin data */
   NULL  /* destroy */
};

EAPI CDList *
remix_load(RemixEnv *env)
{
   CDList *plugins = cd_list_new(env);
   
   eet_sndfile_reader_plugin.init_scheme =
     cd_set_insert(env, eet_sndfile_reader_plugin.init_scheme, PATH_KEY,
                   CD_POINTER(&path_scheme));
   eet_sndfile_reader_plugin.init_scheme =
     cd_set_insert(env, eet_sndfile_reader_plugin.init_scheme, SOUND_ID_KEY,
                   CD_POINTER(&sound_id_scheme));
   eet_sndfile_reader_plugin.init_scheme =
     cd_set_insert(env, eet_sndfile_reader_plugin.init_scheme, SPEED_KEY,
                   CD_POINTER(&speed_scheme));
   
   plugins = cd_list_prepend(env, plugins,
                             CD_POINTER(&eet_sndfile_reader_plugin));
   return plugins;
}
