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
 * Set with @ref eo_base_data_set()
 */
#define ECORE_AUDIO_ATTR_TONE_FREQ "ecore_audio_freq"

#define ECORE_AUDIO_OBJ_IN_TONE_CLASS ecore_audio_obj_in_tone_class_get() /**< Ecore_Audio tone input */

/**
 * @brief Get the Eo class ID 
 *
 * @return The Eo class ID
 */
const Eo_Class *ecore_audio_obj_in_tone_class_get() EINA_CONST;

extern EAPI Eo_Op ECORE_AUDIO_OBJ_IN_TONE_BASE_ID;

enum Ecore_Audio_Obj_In_Tone_Sub_Ids
{
   ECORE_AUDIO_OBJ_IN_TONE_SUB_ID_LAST
};

#define ECORE_AUDIO_OBJ_IN_TONE_ID(sub_id) (ECORE_AUDIO_OBJ_IN_TONE_BASE_ID + EO_TYPECHECK(enum Ecore_Audio_Obj_In_Tone_Sub_Ids, sub_id)

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
