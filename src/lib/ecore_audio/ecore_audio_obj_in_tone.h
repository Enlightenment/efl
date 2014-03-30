#ifndef ECORE_AUDIO_IN_TONE_H
#define ECORE_AUDIO_IN_TONE_H

#include <Eina.h>
#include <Eo.h>

#ifdef EAPI
#undef EAPI
#endif

#ifdef __GNUC__
#if __GNUC__ >= 4
#define EAPI __attribute__ ((visibility("default")))
#else
#define EAPI
#endif
#else
#define EAPI
#endif

/**
 * @file ecore_audio_obj_in_tone.h
 * @brief Ecore_Audio tone input
 */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup ecore_audio_obj_in_tone - Ecore_Audio tone input
 * @ingroup Ecore_Audio_Group
 * @{
 */

/**
 * @brief The frequency of the tone in Hz
 *
 * Set with @ref eo_key_data_set()
 */
#define ECORE_AUDIO_ATTR_TONE_FREQ "ecore_audio_freq"

#include "ecore_audio_in_tone.eo.h"

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
