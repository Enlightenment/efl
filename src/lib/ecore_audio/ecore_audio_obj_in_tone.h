#ifndef ECORE_AUDIO_IN_TONE_H
#define ECORE_AUDIO_IN_TONE_H

#include <Eina.h>
#include <Eo.h>

/**
 * @file ecore_audio_obj_in_tone.h
 * @brief Ecore_Audio tone input.
 */

/**
 * @defgroup ecore_audio_obj_in_tone - Ecore_Audio tone input
 * @ingroup Ecore_Audio_Group
 * @{
 */

/**
 * @brief The frequency of the tone in Hz.
 *
 * Set with @ref efl_key_data_set()
 */
#define ECORE_AUDIO_ATTR_TONE_FREQ "ecore_audio_freq"

#include "ecore_audio_in_tone.eo.h"

/**
 * @}
 */

#endif
