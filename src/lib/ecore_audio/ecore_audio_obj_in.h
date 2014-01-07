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
 * @brief Ecore_Audio Input Object
 */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup ecore_audio_obj_in - Ecore_Audio input object
 * @ingroup Ecore_Audio_Group
 * @{
 */

#define ECORE_AUDIO_OBJ_IN_CLASS ecore_audio_obj_in_class_get() /**< Ecore_Audio input object class */

/**
 * @brief Get the Eo class ID
 *
 * @return The Eo class ID
 */
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

/**
 * @brief Set the playback speed of the input
 *
 * @since 1.8
 *
 * @param[in] speed The speed, 1.0 is the default
 */
#define ecore_audio_obj_in_speed_set(speed) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_SPEED_SET), EO_TYPECHECK(double, speed)

/**
 * @brief Get the playback speed of the input
 *
 * @since 1.8
 *
 * This will trigger the ECORE_AUDIO_EV_IN_SAMPLERATE_CHANGED event.
 *
 * @param[out] speed The speed
 */
#define ecore_audio_obj_in_speed_get(speed) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_SPEED_GET), EO_TYPECHECK(double *, speed)

/**
 * @brief Set the sample-rate of the input
 *
 * @since 1.8
 *
 * This will trigger the ECORE_AUDIO_EV_IN_SAMPLERATE_CHANGED event.
 *
 * @param[in] samplerate The samplerate in Hz
 */
#define ecore_audio_obj_in_samplerate_set(samplerate) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_SAMPLERATE_SET), EO_TYPECHECK(int, samplerate)

/**
 * @brief Get the sample-rate of the input
 *
 * @since 1.8
 *
 * @param[out] samplerate The samplerate in Hz
 */
#define ecore_audio_obj_in_samplerate_get(samplerate) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_SAMPLERATE_GET), EO_TYPECHECK(int *, samplerate)

/**
 * @brief Set the amount of channels the input has
 *
 * @since 1.8
 *
 * @param[in] channels The number of channels
 */
#define ecore_audio_obj_in_channels_set(channels) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_CHANNELS_SET), EO_TYPECHECK(int, channels)

/**
 * @brief Get the amount of channels the input has
 *
 * @since 1.8
 *
 * @param[out] channels The number of channels
 */
#define ecore_audio_obj_in_channels_get(channels) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_CHANNELS_GET), EO_TYPECHECK(int *, channels)

/**
 * @brief Set the preloaded state of the input
 *
 * @since 1.8
 *
 * @param[in] preloaded EINA_TRUE if the input should be cached, EINA_FALSE otherwise
 */
#define ecore_audio_obj_in_preloaded_set(preloaded) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_PRELOADED_SET), EO_TYPECHECK(Eina_Bool, preloaded)

/**
 * @brief Get the preloaded state of the input
 *
 * @since 1.8
 *
 * @param[out] preloaded EINA_TRUE if the input is cached, EINA_FALSE otherwise
 */
#define ecore_audio_obj_in_preloaded_get(preloaded) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_PRELOADED_GET), EO_TYPECHECK(Eina_Bool *, preloaded)

/**
 * @brief Set the looped state of the input
 *
 * @since 1.8
 *
 * If the input is looped and reaches the end it will start from the
 * beginning again. At the same time the event @ref ECORE_AUDIO_EV_IN_LOOPED
 * will be emitted
 *
 * @param[in] looped EINA_TRUE if the input should be looped, EINA_FALSE otherwise
 */
#define ecore_audio_obj_in_looped_set(looped) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_LOOPED_SET), EO_TYPECHECK(Eina_Bool, looped)

/**
 * @brief Get the looped state of the input
 *
 * @since 1.8
 *
 * @see ecore_audio_obj_in_looped_set
 *
 * @param[out] ret EINA_TRUE if the input is looped, EINA_FALSE otherwise
 */
#define ecore_audio_obj_in_looped_get(ret) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_LOOPED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @brief Set the length of the input
 *
 * @since 1.8
 *
 * This function is only implemented by some classes
 * (i.e. ECORE_AUDIO_OBJ_IN_TONE_CLASS)
 *
 * @param[in] length The length of the input in seconds
 */
#define ecore_audio_obj_in_length_set(length) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_LENGTH_SET), EO_TYPECHECK(double, length)

/**
 * @brief Get the length of the input
 *
 * @since 1.8
 *
 * @param[out] ret The length of the input in seconds
 */
#define ecore_audio_obj_in_length_get(ret) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_LENGTH_GET), EO_TYPECHECK(double *, ret)

/**
 * @brief Read from the input
 *
 * @since 1.8
 *
 * @param[out] buf The buffer to read into
 * @param[in] len The amount of samples to read
 * @param[out] ret The amount of samples written to buf
 */
#define ecore_audio_obj_in_read(buf, len, ret) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_READ), EO_TYPECHECK(void *, buf), EO_TYPECHECK(size_t, len), EO_TYPECHECK(ssize_t *, ret)

/**
 * @brief Seek within the input
 *
 * @since 1.8
 *
 * @param[in] offs The offset in seconds
 * @param[in] mode The seek mode. Is absolute with SEEK_SET, relative to the
 * current position with SEEK_CUR and relative to the end with SEEK_END.
 * @param[out] ret The current absolute position in seconds within the input
 */
#define ecore_audio_obj_in_seek(offs, mode, ret) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_SEEK), EO_TYPECHECK(double, offs), EO_TYPECHECK(int, mode), EO_TYPECHECK(double *, ret)

/**
 * @brief Get the output that this input is attached to
 *
 * @since 1.8
 *
 * @param[out] ret The output
 */
#define ecore_audio_obj_in_output_get(ret) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_OUTPUT_GET), EO_TYPECHECK(Eo **, ret)

/**
 * @brief Get the remaining time of the input
 *
 * @param[out] ret The amount of time in seconds left to play
 */
#define ecore_audio_obj_in_remaining_get(ret) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_REMAINING_GET), EO_TYPECHECK(double *, ret)

extern const Eo_Event_Description _ECORE_AUDIO_EV_IN_LOOPED;

/**
 * @brief The input looped
 *
 * @since 1.8
 *
 * Emitted when @ref ecore_audio_obj_in_looped_set is set to EINA_TRUE and
 * the input restarted playing after reaching the end.
 */
#define ECORE_AUDIO_EV_IN_LOOPED (&(_ECORE_AUDIO_EV_IN_LOOPED))

extern const Eo_Event_Description _ECORE_AUDIO_EV_IN_STOPPED;

/**
 * @brief The input stopped playing
 *
 * @since 1.8
 *
 * Emitted when the input stopped playing after reaching the end.
 */
#define ECORE_AUDIO_EV_IN_STOPPED (&(_ECORE_AUDIO_EV_IN_STOPPED))

extern const Eo_Event_Description _ECORE_AUDIO_EV_IN_SAMPLERATE_CHANGED;

/**
 * @brief The sample rate changed
 *
 * @since 1.8
 *
 * Emitted when the samplerate changed - this can happen if you call
 * @ref ecore_audio_obj_in_samplerate_set or
 * @ref ecore_audio_obj_in_speed_set.
 */
#define ECORE_AUDIO_EV_IN_SAMPLERATE_CHANGED (&(_ECORE_AUDIO_EV_IN_SAMPLERATE_CHANGED))

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif

