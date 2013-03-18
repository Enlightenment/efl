#ifndef ECORE_AUDIO_H
#define ECORE_AUDIO_H

#include <Eina.h>

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
 * @file Ecore_Audio.h
 * @brief Audio utility functions
 */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup Ecore_Audio_Group Ecore_Audio - Convenience audio API
 * @ingroup Ecore
 *
 * @since 1.8
 *
 * @{
 */

  /** @since 1.8
   */
enum _Ecore_Audio_Type {
    ECORE_AUDIO_TYPE_PULSE,   /**< Use Pulseaudio module */
    ECORE_AUDIO_TYPE_ALSA,    /**< Use ALSA module*/
    ECORE_AUDIO_TYPE_SNDFILE, /**< Use libsndfile module */
    ECORE_AUDIO_TYPE_TONE,    /**< Use tone module */
    ECORE_AUDIO_TYPE_CUSTOM,  /**< Use custom module */
    ECORE_AUDIO_MODULE_LAST,  /**< Sentinel */
};

typedef enum _Ecore_Audio_Type Ecore_Audio_Type;

  /** @since 1.8
   */
typedef struct _Ecore_Audio_Module Ecore_Audio_Module;
/**< The audio module */

  /** @since 1.8
   */
typedef struct _Ecore_Audio_Object Ecore_Audio_Object;  /**< The audio object */

  /** @since 1.8
   */
typedef struct _Ecore_Audio_Format Ecore_Audio_Format;
/**< The format of the audio data */

typedef int (*Ecore_Audio_Read_Callback)(void *user_data, void *data, int len);

struct _Ecore_Audio_Vio {
    int (*get_length)(Ecore_Audio_Object *in);
    int (*seek)(Ecore_Audio_Object *in, int offset, int whence);
    int (*tell)(Ecore_Audio_Object *in);
    int (*read)(Ecore_Audio_Object *in, void *buffer, int length);
    int (*write)(Ecore_Audio_Object *out, const void *buffer, int length);
};

typedef struct _Ecore_Audio_Vio Ecore_Audio_Vio; /**< Functions to implement IO virtually */

EAPI extern int ECORE_AUDIO_INPUT_STARTED; /**< Sound was started */
EAPI extern int ECORE_AUDIO_INPUT_STOPPED; /**< Sound was stopped */
EAPI extern int ECORE_AUDIO_INPUT_LOOPED;  /**< Sound looped */
EAPI extern int ECORE_AUDIO_INPUT_ENDED;   /**< Sound playback ended */
EAPI extern int ECORE_AUDIO_INPUT_PAUSED;  /**< Sound paused */
EAPI extern int ECORE_AUDIO_OUTPUT_INPUT_ADDED;   /**< Input added to output */
EAPI extern int ECORE_AUDIO_OUTPUT_INPUT_REMOVED; /**< Input removed from output */

/* Audio operations */

/**
 * @brief Initialize the Ecore_Audio library.
 *
 * @return 1 or greater on success, 0 on error.
 *
 * @since 1.8
 *
 * This function sets up Ecore_Audio and initializes the modules that
 * provide the in- and outputs to use. It returns 0 on failure, otherwise
 * it returns the number of times it has already been called.
 *
 * When Ecore_Audio is not used anymore, call ecore_audio_shutdown()
 * to shut down the Ecore_Audio library.
 */
EAPI int                 ecore_audio_init(void);

/**
 * @brief Shut down the Ecore_Audio library.
 *
 * @return 0 when the library is completely shut down, 1 or
 * greater otherwise.
 *
 * @since 1.8
 *
 * This function shuts down the Ecore_Audio library. It returns 0 when it has
 * been called the same number of times than ecore_audio_init(). In that case
 * it shuts down all the services it uses.
 */
EAPI int                 ecore_audio_shutdown(void);


/* Output operations*/

/**
 * @brief Create a new Ecore_Audio_Output instance
 *
 * @param name the name of the output to create
 * @return a new instance or NULL on error
 *
 * @since 1.8
 */
EAPI Ecore_Audio_Object *ecore_audio_output_add(Ecore_Audio_Type type);

/**
 * @brief Set the name of an output
 *
 * @param output The output
 * @param name The name
 *
 * @since 1.8
 */
EAPI void ecore_audio_output_name_set(Ecore_Audio_Object *output, const char *name);

/**
 * @brief Get the name of an output
 *
 * @param output the output
 *
 * @return the name of the output
 *
 * @since 1.8
 */
EAPI const char *ecore_audio_output_name_get(Ecore_Audio_Object *output);

/**
 * @brief Free an @ref Ecore_Audio_Output instance
 *
 * @param out the output
 *
 * @since 1.8
 */
EAPI void                ecore_audio_output_del(Ecore_Audio_Object *output);

/**
 * @brief Set the user data pointer
 *
 * @param output The output
 * @param data The pointer to set
 *
 * @since 1.8
 */
EAPI void ecore_audio_output_userdata_set(Ecore_Audio_Object *output, void *data);

/**
 * @brief Get the user data pointer
 *
 * @param output The output
 *
 * @return The pointer to the user data
 *
 * @since 1.8
 */
EAPI void *ecore_audio_output_userdata_get(Ecore_Audio_Object *output);

/**
 * @brief Set the volume of the output
 *
 * @param out the output
 * @param volume the volume
 *
 * @since 1.8
 */
EAPI void                ecore_audio_output_volume_set(Ecore_Audio_Object *output, double volume);

/**
 * @brief Get the volume of the output
 *
 * @param out the output
 *
 * @return the volume
 *
 * @since 1.8
 */
EAPI double              ecore_audio_output_volume_get(Ecore_Audio_Object *output);

/**
 * @brief Set the paused state of an output
 *
 * @param out the output
 * @param paused the paused state
 *
 * @since 1.8
 */
EAPI void                ecore_audio_output_paused_set(Ecore_Audio_Object *output, Eina_Bool paused);

/**
 * @brief Get the paused state of an output
 *
 * @param out the output
 *
 * @return the paused state
 *
 * @since 1.8
 */
EAPI Eina_Bool           ecore_audio_output_paused_get(Ecore_Audio_Object *output);

/**
 * @brief Add an input to an output.
 *
 * @param out the output
 * @param in the input
 *
 * @return True if connecting was successful, False otherwise
 *
 * @since 1.8
 */
EAPI Eina_Bool           ecore_audio_output_input_add(Ecore_Audio_Object *output, Ecore_Audio_Object *input);

/**
 * @brief Disconnect an input from an output. This will stop playback of the
 * input.
 *
 * @param out the output
 * @param in the input
 *
 * @return True if disconnecting was successful, False otherwise
 *
 * @since 1.8
 */
EAPI Eina_Bool           ecore_audio_output_input_del(Ecore_Audio_Object *output, Ecore_Audio_Object *input);

/**
 * @brief Get the inputs connected to an output.
 *
 * @param out the output
 *
 * @return A list of Ecore_Audio_Input that are connected to the output
 *
 * @since 1.8
 */
EAPI Eina_List          *ecore_audio_output_inputs_get(Ecore_Audio_Object *output);

/**
 * @brief Set up an input to play after another input.
 *
 * @param out the output
 * @param after the input relative to which the other input will be chained
 * @param in the input to chain
 *
 * @return True if chaining was successful, False otherwise
 *
 * @since 1.8
 */
EAPI Eina_Bool           ecore_audio_output_input_chain_after(Ecore_Audio_Object *output, Ecore_Audio_Object *input, Ecore_Audio_Object *after);


/* Input operations*/

/**
 * @brief Create a new Ecore_Audio_Input instance
 *
 * @param name the name of the input to create
 * @return a new instance or NULL on error
 *
 * @since 1.8
 */
EAPI Ecore_Audio_Object *ecore_audio_input_add(Ecore_Audio_Type type);

/**
 * @brief Get the name of an input
 *
 * @param input the input
 *
 * @return the name of the input
 *
 * @since 1.8
 */
EAPI const char *ecore_audio_input_name_get(Ecore_Audio_Object *input);

/**
 * @brief Set the name of an input
 *
 * @param input the input
 * @param name The name to set
 *
 * @since 1.8
 */
EAPI void ecore_audio_input_name_set(Ecore_Audio_Object *input, const char *name);

/**
 * @brief Free an @ref Ecore_Audio_Input instance
 *
 * @param in the input
 *
 * @since 1.8
 */
EAPI void                ecore_audio_input_del(Ecore_Audio_Object *input);

/**
 * @brief Set the user data pointer
 *
 * @param input The input
 * @param data The pointer to set
 *
 * @since 1.8
 */
EAPI void ecore_audio_input_userdata_set(Ecore_Audio_Object *input, void *data);

/**
 * @brief Get the user data pointer
 *
 * @param input The input
 *
 * @return The pointer to the user data
 *
 * @since 1.8
 */
EAPI void *ecore_audio_input_userdata_get(Ecore_Audio_Object *input);

/**
 * @brief Get the sample rate of the input
 *
 * @param input The input
 *
 * @return The samplerate in Hz
 *
 * @since 1.8
 */
EAPI int ecore_audio_input_samplerate_get(Ecore_Audio_Object *input);

/**
 * @brief Set the sample rate of the input
 *
 * @param input The input
 * @param samplerate The sample rate in Hz
 *
 * @since 1.8
 */
EAPI void ecore_audio_input_samplerate_set(Ecore_Audio_Object *input, int samplerate);

/**
 * @brief Get the speed the input is played back at
 *
 * @param input The input
 *
 * @return The speed
 *
 * @since 1.8
 */
EAPI double ecore_audio_input_speed_get(Ecore_Audio_Object *input);

/**
 * @brief Set the speed the input is played back at
 *
 * @param input The input
 * @param samplerate The speed (1.0 is normal speed)
 *
 * @since 1.8
 */
EAPI void ecore_audio_input_speed_set(Ecore_Audio_Object *input, double speed);

/**
 * @brief Get the channels of the input
 *
 * @param input The input
 * 
 * @return The number of channels
 *
 * @since 1.8
 */
EAPI int ecore_audio_input_channels_get(Ecore_Audio_Object *input);

/**
 * @brief Set the amount of channels in the input
 *
 * @param input The input
 * @param channels The number of channels to set
 *
 * @since 1.8
 */
EAPI void ecore_audio_input_channels_set(Ecore_Audio_Object *input, int channels);

/**
 * @brief Seek within an input stream
 *
 * @param in the input
 * @offset the offset in seconds
 * @mode seek mode (SEEK_SET, SEEK_END, or SEEK_CUR)
 * @return the current offset
 *
 * @since 1.8
 */
EAPI double              ecore_audio_input_seek(Ecore_Audio_Object *input, double offset, int mode);

/**
 * @brief Read data from an input stream
 *
 * @param in the input
 * @param data the buffer to write the audio data into
 * @param len the size of the buffer
 * 
 * @return the number of bytes that were read
 *
 * @since 1.8
 */
EAPI int ecore_audio_input_read(Ecore_Audio_Object *input, void *data, int len);

/**
 * @brief Get the paused state of an input
 *
 * @param in the input
 * @return EINA_TRUE if the input is paused, EINA_FALSE otherwise
 *
 * @since 1.8
 */
EAPI Eina_Bool           ecore_audio_input_paused_get(Ecore_Audio_Object *input);

/**
 * @brief Set the paused state of an input
 *
 * @param in the input
 * @param paused the paused state to set
 *
 * @since 1.8
 *
 * If paused is EINA_TRUE if the input is paused, if it is EINA_FALSE the
 * input plays normally.
 */
EAPI void                ecore_audio_input_paused_set(Ecore_Audio_Object *input, Eina_Bool paused);

/**
 * @brief Set the volume of the input
 *
 * @param in the input
 * @param volume the volume
 *
 * @since 1.8
 */
EAPI void                ecore_audio_input_volume_set(Ecore_Audio_Object *input, double volume);

/**
 * @brief Get the volume of the input
 *
 * @param in the input
 *
 * @return the volume
 *
 * @since 1.8
 */
EAPI double              ecore_audio_input_volume_get(Ecore_Audio_Object *input);

/**
 * @brief Set whether the input loops
 *
 * @param in the input
 * @param looped if the input should loop
 *
 * @since 1.8
 */
EAPI void                ecore_audio_input_looped_set(Ecore_Audio_Object *input, Eina_Bool looped);

/**
 * @brief Get whether the input loops
 *
 * @param in the input
 *
 * @return if the input loops
 *
 * @since 1.8
 */
EAPI Eina_Bool           ecore_audio_input_looped_get(Ecore_Audio_Object *input);

/**
 * @brief Get the length of the input in seconds
 *
 * @param in the input
 *
 * @return the length in seconds
 *
 * @since 1.8
 */
EAPI double              ecore_audio_input_length_get(Ecore_Audio_Object *input);

/**
 * @brief Set whether the input is preloaded
 *
 * @param in the input
 * @param preloaded if the input is preloaded
 *
 * @since 1.8
 */
EAPI void                ecore_audio_input_preloaded_set(Ecore_Audio_Object *input, Eina_Bool preloaded);

/**
 * @brief Get whether the input is preloaded
 *
 * @param in the input
 *
 * @return EINA_TRUE if the input is preloaded, otherwise EINA_FALSE
 *
 * @since 1.8
 */
EAPI Eina_Bool           ecore_audio_input_preloaded_get(Ecore_Audio_Object *input);

/**
 * @brief Get the outputs this input is connected to
 *
 * @param in the input
 *
 * @return A list of outputs
 *
 * @since 1.8
 */
EAPI Ecore_Audio_Object *ecore_audio_input_output_get(Ecore_Audio_Object *input);

/**
 * @brief Get the remaining time of the input
 *
 * @param in the input
 *
 * @return The remaining time in seconds
 *
 * @since 1.8
 */
EAPI double              ecore_audio_input_remaining_get(Ecore_Audio_Object *input);

/* libsndfile */

EAPI void ecore_audio_input_sndfile_filename_set(Ecore_Audio_Object *input, const char *filename);
EAPI void ecore_audio_input_sndfile_format_set(Ecore_Audio_Object *input, int format);
EAPI void ecore_audio_input_sndfile_vio_set(Ecore_Audio_Object *input, Ecore_Audio_Vio *vio);

EAPI void ecore_audio_output_sndfile_filename_set(Ecore_Audio_Object *output, const char *filename);
EAPI void ecore_audio_output_sndfile_format_set(Ecore_Audio_Object *output, int format);
EAPI void ecore_audio_output_sndfile_vio_set(Ecore_Audio_Object *output, Ecore_Audio_Vio *vio);

/* tone */
EAPI void ecore_audio_input_tone_frequency_set(Ecore_Audio_Object *input, int frequency);
EAPI void ecore_audio_input_tone_duration_set(Ecore_Audio_Object *input, double duration);

EAPI void                ecore_audio_input_callback_setup(Ecore_Audio_Object *input, Ecore_Audio_Read_Callback read_cb, void *data);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
