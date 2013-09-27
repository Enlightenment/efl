#ifndef ECORE_AUDIO_OUT_SNDFILE_H
#define ECORE_AUDIO_OUT_SNDFILE_H

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
 * @file ecore_audio_obj_out_sndfile.h
 * @brief Ecore_Audio sndfile output
 */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup ecore_audio_obj_out_sndfile - Ecore_Audio sndfile output
 * @ingroup Ecore_Audio_Group
 * @{
 */

#define ECORE_AUDIO_OBJ_OUT_SNDFILE_CLASS ecore_audio_obj_out_sndfile_class_get() /**< Ecore_Audio sndfile output */

/**
 * @brief Get the Eo class ID 
 *
 * @return The Eo class ID
 */
const Eo_Class *ecore_audio_obj_out_sndfile_class_get() EINA_CONST;

extern EAPI Eo_Op ECORE_AUDIO_OBJ_OUT_SNDFILE_BASE_ID;

enum _Ecore_Audio_Obj_Out_Sndfile_Sub_Ids
{
   ECORE_AUDIO_OBJ_OUT_SNDFILE_SUB_ID_LAST
};

#define ECORE_AUDIO_OBJ_OUT_SNDFILE_ID(sub_id) (ECORE_AUDIO_OBJ_OUT_SNDFILE_BASE_ID + EO_TYPECHECK(enum _Ecore_Audio_Obj_Out_Sndfile_Sub_Ids, sub_id))

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
