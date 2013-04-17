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
 * @brief Audio Module
 */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @addtogroup Ecore_Audio_Group
 * @{
 */

#define ECORE_AUDIO_OBJ_OUT_PULSE_CLASS ecore_audio_obj_out_pulse_class_get()

const Eo_Class *ecore_audio_obj_out_pulse_class_get() EINA_CONST;

extern EAPI Eo_Op ECORE_AUDIO_OBJ_OUT_PULSE_BASE_ID;

enum _Ecore_Audio_Obj_Out_Pulse_Sub_Ids
{
   ECORE_AUDIO_OBJ_OUT_PULSE_SUB_ID_LAST
};

#define ECORE_AUDIO_OBJ_OUT_PULSE_ID(sub_id) (ECORE_AUDIO_OBJ_OUT_PULSE_BASE_ID + EO_TYPECHECK(enum _Ecore_Audio_Obj_Out_Pulse_Sub_Ids, sub_id))

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
