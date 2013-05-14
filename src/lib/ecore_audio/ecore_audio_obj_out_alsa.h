#ifndef ECORE_AUDIO_OUT_ALSA_H
#define ECORE_AUDIO_OUT_ALSA_H

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
 * @file ecore_audio_obj_out_alsa.h
 * @brief Ecore_Audio alsa output
 */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup ecore_audio_obj_out_alsa - Ecore_Audio alsa output
 * @intogroup Ecore_Audio_Group
 * @{
 */

#define ECORE_AUDIO_OBJ_OUT_ALSA_CLASS ecore_audio_obj_out_alsa_class_get() /**< Ecore_Audio alsa output */

/**
 * @brief Get the Eo class ID
 *
 * @return The Eo class ID
 */
const Eo_Class *ecore_audio_obj_out_alsa_class_get() EINA_CONST;

extern EAPI Eo_Op ECORE_AUDIO_OBJ_OUT_ALSA_BASE_ID;

enum _Ecore_Audio_Obj_Out_Alsa_Sub_Ids
{
   ECORE_AUDIO_OBJ_OUT_ALSA_SUB_ID_LAST
};

#define ECORE_AUDIO_OBJ_OUT_ALSA_ID(sub_id) (ECORE_AUDIO_OBJ_OUT_ALSA_BASE_ID + EO_TYPECHECK(enum _Ecore_Audio_Obj_Out_Alsa_Sub_Ids, sub_id))

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
