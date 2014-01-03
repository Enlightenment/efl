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

#define ECORE_AUDIO_OBJ_OUT_PULSE_CLASS ecore_audio_obj_out_pulse_class_get() /**< Ecore_Audio pulseaudio output */

/**
 * @brief Get the Eo class ID 
 *
 * @return The Eo class ID
 */
const Eo_Class *ecore_audio_obj_out_pulse_class_get() EINA_CONST;

extern EAPI Eo_Op ECORE_AUDIO_OBJ_OUT_PULSE_BASE_ID;

enum _Ecore_Audio_Obj_Out_Pulse_Sub_Ids
{
   ECORE_AUDIO_OBJ_OUT_PULSE_SUB_ID_LAST
};

#define ECORE_AUDIO_OBJ_OUT_PULSE_ID(sub_id) (ECORE_AUDIO_OBJ_OUT_PULSE_BASE_ID + EO_TYPECHECK(enum _Ecore_Audio_Obj_Out_Pulse_Sub_Ids, sub_id))

extern EAPI const Eo_Event_Description _ECORE_AUDIO_EV_OUT_PULSE_CONTEXT_READY;
   
/**
 * @brief The output context is ready
 *
 * @since 1.8
 *
 * Emitted when the outout context is ready for playback
 */
#define ECORE_AUDIO_EV_OUT_PULSE_CONTEXT_READY (&(_ECORE_AUDIO_EV_OUT_PULSE_CONTEXT_READY))
   
extern EAPI const Eo_Event_Description _ECORE_AUDIO_EV_OUT_PULSE_CONTEXT_FAIL;
   
/**
 * @brief The output context has failed
 *
 * @since 1.8
 *
 * Emitted when the outout context has failed. At this point the output is unusable and will never work, so it is advisable to delete it.
 */
#define ECORE_AUDIO_EV_OUT_PULSE_CONTEXT_FAIL (&(_ECORE_AUDIO_EV_OUT_PULSE_CONTEXT_FAIL))
   
/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
