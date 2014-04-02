#ifndef ECORE_AUDIO_PRIVATE_H_
#define ECORE_AUDIO_PRIVATE_H_

#ifdef __linux__
#include <features.h>
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_ALSA
#include <alsa/asoundlib.h>
#endif

#ifdef HAVE_PULSE
#include <pulse/pulseaudio.h>
#endif

#ifdef HAVE_SNDFILE
#include <sndfile.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include "Ecore.h"
#include "ecore_private.h"

#include "Ecore_Audio.h"
#include "ecore_audio_protected.h"

extern int _ecore_audio_log_dom;

#ifdef ECORE_AUDIO_DEFAULT_LOG_COLOR
#undef ECORE_AUDIO_DEFAULT_LOG_COLOR
#endif
#define ECORE_AUDIO_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_ecore_audio_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_ecore_audio_log_dom, __VA_ARGS__)

#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_ecore_audio_log_dom, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_ecore_audio_log_dom, __VA_ARGS__)

#ifdef CRI
#undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_ecore_audio_log_dom, __VA_ARGS__)

/**
 * @defgroup Ecore_Audio_Module_API_Group Ecore_Audio_Module_API - API for modules
 * @ingroup Ecore_Audio_Group
 *
 * @internal These functions are internal
 *
 * @{
 */

typedef struct _Ecore_Audio_Input Ecore_Audio_Input;
typedef struct _Ecore_Audio_Output Ecore_Audio_Output;

/**
 * @brief The structure representing an Ecore_Audio module
 */
struct _Ecore_Audio_Module
{
   ECORE_MAGIC;
   Ecore_Audio_Type type;
   char              *name;
   Eina_List         *inputs;
   Eina_List         *outputs;

   void              *priv;

   struct input_api  *in_ops;
   struct output_api *out_ops;
};

struct _Ecore_Audio_Vio_Internal {
    Ecore_Audio_Vio *vio;
    void *data;
    eo_key_data_free_func free_func;
};
typedef struct _Ecore_Audio_Vio_Internal Ecore_Audio_Vio_Internal;

/**
 * @brief A common structure, could be input or output
 */
struct _Ecore_Audio_Object
{
   const char         *name;
   const char         *source;

   Eina_Bool           paused;
   double              volume;
   Ecore_Audio_Format  format;
   Ecore_Audio_Vio_Internal *vio;
};

/**
 * @brief The structure representing an Ecore_Audio output
 */
struct _Ecore_Audio_Output
{
   Eina_List          *inputs; /**< The inputs that are connected to this output */
   Ecore_Idler        *write_idler;
   Eina_Bool           need_writer;
};

/**
 * @brief The structure representing an Ecore_Audio input
 */
struct _Ecore_Audio_Input
{
   Eina_Bool           paused; /**< Is the input paused? */
   Eina_Bool           seekable;
   Eina_Bool           seekable_prev;

   Eo                 *output; /**< The output this input is connected to */

   int                 samplerate;
   int                 channels;
   Eina_Bool           looped; /**< Loop the sound */
   double              speed;
   double              length; /**< Length of the sound */
   Eina_Bool           preloaded;
   Eina_Bool           ended;
};

extern Eina_List *ecore_audio_modules;

#ifdef HAVE_ALSA
/* ecore_audio_alsa */
struct _Ecore_Audio_Alsa
{
   ECORE_MAGIC;
   snd_pcm_t   *handle;
   unsigned int channels;
   unsigned int samplerate;
};

Ecore_Audio_Module *ecore_audio_alsa_init(void);
void                ecore_audio_alsa_shutdown(void);
#endif /* HAVE_ALSA */

#ifdef HAVE_PULSE
Ecore_Audio_Module *ecore_audio_pulse_init(void);
void                ecore_audio_pulse_shutdown(void);
#endif /* HAVE_PULSE */

#ifdef HAVE_SNDFILE
/* ecore_audio_sndfile */
Ecore_Audio_Module *ecore_audio_sndfile_init(void);
void                ecore_audio_sndfile_shutdown(void);
#endif /* HAVE_SNDFILE */

Ecore_Audio_Module *ecore_audio_tone_init(void);
void                ecore_audio_tone_shutdown(void);

Ecore_Audio_Module *ecore_audio_custom_init(void);
void                ecore_audio_custom_shutdown(void);

/**
 * @}
 */
#endif
