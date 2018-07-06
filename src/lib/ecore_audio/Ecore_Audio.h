#ifndef ECORE_AUDIO_H
#define ECORE_AUDIO_H

#include <Eina.h>
#include <Eo.h>

#ifdef EAPI
#undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

/**
 * @file Ecore_Audio.h
 * @brief Audio utility functions.
 */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup Ecore_Audio_Group Ecore_Audio - Convenience audio API
 * @ingroup Ecore

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
    ECORE_AUDIO_TYPE_CORE_AUDIO, /**< Use Core Audio module (Apple) - DEPRECATED */
    ECORE_AUDIO_TYPE_CUSTOM,  /**< Use custom module */
    ECORE_AUDIO_TYPE_WASAPI,  /**< Use Wasapi module @since 1.21*/
    ECORE_AUDIO_MODULE_LAST,  /**< Sentinel */
};

/**
 * @since 1.8
 */
typedef enum _Ecore_Audio_Type Ecore_Audio_Type;

  /** @since 1.8
   */
typedef struct _Ecore_Audio_Module Ecore_Audio_Module;
/**< The audio module */

  /** @since 1.8
   */
typedef struct _Ecore_Audio_Object Ecore_Audio_Object;  /**< The audio object */

/*
 * @brief Structure to hold the callbacks needed to implement virtual file IO
 * @since 1.8
 */
struct _Ecore_Audio_Vio {
    /**
     * @brief Gets the length of the file.
     *
     * @param data User data from the ecore_audio_obj_set_vio call
     * @param eo_obj The Ecore_Audio object this operates on
     *
     * @return The length of the virtual file in bytes
     *
     * @since 1.8
     */
    int (*get_length)(void *data, Eo *eo_obj);

    /**
     * @brief Seeks to a position within the file.
     *
     * @param data User data from the ecore_audio_obj_set_vio call
     * @param eo_obj The Ecore_Audio object this operates on
     * @param offset The number of bytes to move (can be negative)
     * @param whence Accepts the same values as fseek(), which are:
     *               SEEK_SET: offset is absolute
     *               SEEK_CUR: offset is relative to the current position
     *               SEEK_END: offset is relative to the end
     *
     * @return The resulting position from the start of the file (in bytes)
     *         or -1 if an error occurred (i.e. out of bounds)
     *
     * @since 1.8
     */
    int (*seek)(void *data, Eo *eo_obj, int offset, int whence);

    /**
     * @brief Gets the current position within the file.
     *
     * @param data User data from the ecore_audio_obj_set_vio call
     * @param eo_obj The Ecore_Audio object this operates on
     *
     * @return The resulting position from the start of the file (in bytes)
     *
     * This is equivalent to calling seek() with offset 0 and whence SEEK_CUR.
     *
     * @since 1.8
     */
    int (*tell)(void *data, Eo *eo_obj);

    /**
     * @brief Reads some data from the file.
     *
     * @param data User data from the ecore_audio_obj_set_vio call
     * @param eo_obj The Ecore_Audio object this operates on
     * @param[out] buffer the buffer to write the data to
     * @param length The number of bytes to read
     *
     * @return The number of bytes read from the file. May be less than length
     *
     * @since 1.8
     */
    int (*read)(void *data, Eo *eo_obj, void *buffer, int length);

    /**
     * @brief Writes some data to the file.
     *
     * @param data User data from the ecore_audio_obj_set_vio call
     * @param eo_obj The Ecore_Audio object this operates on
     * @param buffer Write data from here to the file
     * @param length The number of bytes to write
     *
     * @return The number of bytes written to the file. May be less than length
     *
     * @since 1.8
     */
    int (*write)(void *data, Eo *eo_obj, const void *buffer, int length);
};

/**
 * @brief Holds the callback functions to implement virtual file IO.
 * @since 1.8
 */
typedef struct _Ecore_Audio_Vio Ecore_Audio_Vio;

/* Audio operations */

/**
 * @brief Initializes the Ecore_Audio library.
 *
 * @return @c 1 or greater on success, @c 0 on error.
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
 * @brief Shuts down the Ecore_Audio library.
 *
 * @return @c 0 when the library is completely shut down, @c 1 or
 * greater otherwise.
 *
 * @since 1.8
 *
 * This function shuts down the Ecore_Audio library. It returns 0 when it has
 * been called the same number of times than ecore_audio_init(). In that case
 * it shuts down all the services it uses.
 */
EAPI int                 ecore_audio_shutdown(void);

//Legacy compatibility code

/**
 * @brief Get the name of the object
 *
 * @since 1.8
 *
 */
EAPI const char*         ecore_audio_obj_name_get(const Efl_Object* obj);
/**
 * @brief Name of the object
 *
 * @since 1.8
 *
 */
EAPI void                ecore_audio_obj_name_set(Efl_Object* obj, const char *name);

#include <ecore_audio_obj.h>
#include <ecore_audio_obj_in.h>
#include <ecore_audio_obj_out.h>

#include <ecore_audio_obj_in_sndfile.h>
#include <ecore_audio_obj_out_sndfile.h>

#include <ecore_audio_obj_in_tone.h>

#include <ecore_audio_obj_out_pulse.h>

#include <ecore_audio_obj_out_wasapi.h>

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif
