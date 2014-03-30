#ifndef ECORE_AUDIO_OUT_PULSE_H
#define ECORE_AUDIO_OUT_PULSE_H

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
 * @file ecore_audio_obj_out_pulse.h
 * @brief Ecore_Audio pulseaudio output
 */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup ecore_audio_obj_out_pulse - Ecore_Audio pulseaudio output
 * @ingroup Ecore_Audio_Group
 * @{
 */
#include "ecore_audio_out_pulse.eo.h"
/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif