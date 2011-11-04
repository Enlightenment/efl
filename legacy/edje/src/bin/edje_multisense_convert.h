#ifndef EDJE_SND_CONVERT_H__
# define EDJE_SND_CONVERT_H__
#include "edje_private.h"

#ifdef HAVE_LIBSNDFILE
#include <sndfile.h>

#define SF_CONTAINER(x)    ((x) & SF_FORMAT_TYPEMASK)
#define SF_CODEC(x)        ((x) & SF_FORMAT_SUBMASK)

typedef struct _Edje_Sound_Encode  Edje_Sound_Encode;

struct _Edje_Sound_Encode /*Encoding information*/
{
   const char *file; /* the encode sound file path */
   Eina_Bool encoded; /* True if encoding is successful else False */
   char *comp_type; /* either LOSSLESS (FLAC) or LOSSY (Ogg/Vorbis) Compression */
};

Edje_Sound_Encode *_edje_multisense_encode(const char* filename, Edje_Sound_Sample *sample, double quality);
const char *_edje_multisense_encode_to_flac(char *snd_path, SF_INFO sfinfo);
const char *_edje_multisense_encode_to_ogg_vorbis(char *snd_path, double quality, SF_INFO sfinfo);

#endif
#endif
