#ifndef ECORE_AUDIO_IN_SNDFILE_H
#define ECORE_AUDIO_IN_SNDFILE_H

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
 * @file ecore_audio_obj_in_sndfile.h
 * @brief Ecore_Audio sndfile input
 */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup ecore_audio_obj_in_sndfile - Ecore_Audio sndfile input
 * @ingroup Ecore_Audio_Group
 * @{
 */
#include "ecore_audio_in_sndfile.eo.h"
/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif