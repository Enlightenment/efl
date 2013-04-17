#ifndef ECORE_AUDIO_OBJ_IN_H
#define ECORE_AUDIO_OBJ_IN_H

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
 * @file ecore_audio_obj_in.h
 * @brief Audio Input Object
 */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @addtogroup Ecore_Audio_Group
 * @{
 */

#define ECORE_AUDIO_OBJ_IN_CLASS ecore_audio_obj_in_class_get()

const Eo_Class *ecore_audio_obj_in_class_get() EINA_CONST;

extern EAPI Eo_Op ECORE_AUDIO_OBJ_IN_BASE_ID;

enum Ecore_Audio_Obj_In_Sub_Ids
{
   ECORE_AUDIO_OBJ_IN_SUB_ID_SPEED_SET,
   ECORE_AUDIO_OBJ_IN_SUB_ID_SPEED_GET,
   ECORE_AUDIO_OBJ_IN_SUB_ID_SAMPLERATE_SET,
   ECORE_AUDIO_OBJ_IN_SUB_ID_SAMPLERATE_GET,
   ECORE_AUDIO_OBJ_IN_SUB_ID_CHANNELS_SET,
   ECORE_AUDIO_OBJ_IN_SUB_ID_CHANNELS_GET,
   ECORE_AUDIO_OBJ_IN_SUB_ID_PRELOADED_SET,
   ECORE_AUDIO_OBJ_IN_SUB_ID_PRELOADED_GET,
   ECORE_AUDIO_OBJ_IN_SUB_ID_LOOPED_SET,
   ECORE_AUDIO_OBJ_IN_SUB_ID_LOOPED_GET,
   ECORE_AUDIO_OBJ_IN_SUB_ID_LENGTH_SET,
   ECORE_AUDIO_OBJ_IN_SUB_ID_LENGTH_GET,
   ECORE_AUDIO_OBJ_IN_SUB_ID_READ,
   ECORE_AUDIO_OBJ_IN_SUB_ID_READ_INTERNAL,
   ECORE_AUDIO_OBJ_IN_SUB_ID_SEEK,
   ECORE_AUDIO_OBJ_IN_SUB_ID_OUTPUT_GET,
   ECORE_AUDIO_OBJ_IN_SUB_ID_REMAINING_GET,
   ECORE_AUDIO_OBJ_IN_SUB_ID_LAST
};

#define ECORE_AUDIO_OBJ_IN_ID(sub_id) (ECORE_AUDIO_OBJ_IN_BASE_ID + EO_TYPECHECK(enum Ecore_Audio_Obj_In_Sub_Ids, sub_id))

/*
 * @def ecore_audio_obj_in_speed_set
 * @since 1.8
 *
 * Sets the playback speed of the input
 *
 * @param[in] speed
 */
#define ecore_audio_obj_in_speed_set(speed) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_SPEED_SET), EO_TYPECHECK(double, speed)

/*
 * @def ecore_audio_obj_in_speed_get
 * @since 1.8
 *
 * Gets the playback speed of the input
 *
 * @param[out] speed
 */
#define ecore_audio_obj_in_speed_get(speed) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_SPEED_GET), EO_TYPECHECK(double *, speed)


#define ecore_audio_obj_in_samplerate_set(samplerate) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_SAMPLERATE_SET), EO_TYPECHECK(int, samplerate)

#define ecore_audio_obj_in_samplerate_get(samplerate) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_SAMPLERATE_GET), EO_TYPECHECK(int *, samplerate)

#define ecore_audio_obj_in_channels_set(channels) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_CHANNELS_SET), EO_TYPECHECK(int, channels)

#define ecore_audio_obj_in_channels_get(channels) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_CHANNELS_GET), EO_TYPECHECK(int *, channels)

#define ecore_audio_obj_in_preloaded_set(preloaded) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_PRELOADED_SET), EO_TYPECHECK(Eina_Bool, preloaded)

#define ecore_audio_obj_in_preloaded_get(preloaded) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_PRELOADED_GET), EO_TYPECHECK(Eina_Bool *, preloaded)

#define ecore_audio_obj_in_looped_set(looped) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_LOOPED_SET), EO_TYPECHECK(Eina_Bool, looped)

#define ecore_audio_obj_in_looped_get(ret) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_LOOPED_GET), EO_TYPECHECK(Eina_Bool *, ret)

#define ecore_audio_obj_in_length_set(length) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_LENGTH_SET), EO_TYPECHECK(double, length)

#define ecore_audio_obj_in_length_get(ret) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_LENGTH_GET), EO_TYPECHECK(double *, ret)

#define ecore_audio_obj_in_read(buf, len, ret) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_READ), EO_TYPECHECK(char *, buf), EO_TYPECHECK(size_t, len), EO_TYPECHECK(ssize_t *, ret)

#define ecore_audio_obj_in_seek(offs, mode, ret) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_SEEK), EO_TYPECHECK(double, offs), EO_TYPECHECK(int, mode), EO_TYPECHECK(double *, ret)

#define ecore_audio_obj_in_output_get(ret) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_OUTPUT_GET), EO_TYPECHECK(Eo **, ret)

#define ecore_audio_obj_in_remaining_get(ret) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_REMAINING_GET), EO_TYPECHECK(double *, ret)

extern const Eo_Event_Description _ECORE_AUDIO_EV_IN_LOOPED;
#define ECORE_AUDIO_EV_IN_LOOPED (&(_ECORE_AUDIO_EV_IN_LOOPED))

extern const Eo_Event_Description _ECORE_AUDIO_EV_IN_STOPPED;
#define ECORE_AUDIO_EV_IN_STOPPED (&(_ECORE_AUDIO_EV_IN_STOPPED))

extern const Eo_Event_Description _ECORE_AUDIO_EV_IN_SAMPLERATE_CHANGED;
#define ECORE_AUDIO_EV_IN_SAMPLERATE_CHANGED (&(_ECORE_AUDIO_EV_IN_SAMPLERATE_CHANGED))

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif

