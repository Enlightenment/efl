#ifndef ECORE_AUDIO_OBJ_H
#define ECORE_AUDIO_OBJ_H

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
 * @file ecore_audio_obj.h
 * @brief Base Ecore_Audio object
 */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup ecore_audio_obj - Base Ecore_Audio object
 * @ingroup Ecore_Audio_Group
 * @{
 */

#define ECORE_AUDIO_OBJ_CLASS ecore_audio_obj_class_get() /**< Ecore_Audio object class */

/**
 * @brief Get the Eo class ID 
 *
 * @return The Eo class ID
 */
const Eo_Class *ecore_audio_obj_class_get() EINA_CONST;

extern EAPI Eo_Op ECORE_AUDIO_OBJ_BASE_ID;

enum Ecore_Audio_Obj_Sub_Ids
{
   ECORE_AUDIO_OBJ_SUB_ID_NAME_SET,
   ECORE_AUDIO_OBJ_SUB_ID_NAME_GET,
   ECORE_AUDIO_OBJ_SUB_ID_PAUSED_SET,
   ECORE_AUDIO_OBJ_SUB_ID_PAUSED_GET,
   ECORE_AUDIO_OBJ_SUB_ID_VOLUME_SET,
   ECORE_AUDIO_OBJ_SUB_ID_VOLUME_GET,
   ECORE_AUDIO_OBJ_SUB_ID_SOURCE_SET,
   ECORE_AUDIO_OBJ_SUB_ID_SOURCE_GET,
   ECORE_AUDIO_OBJ_SUB_ID_FORMAT_SET,
   ECORE_AUDIO_OBJ_SUB_ID_FORMAT_GET,
   ECORE_AUDIO_OBJ_SUB_ID_VIO_SET,
   ECORE_AUDIO_OBJ_SUB_ID_LAST
};

#define ECORE_AUDIO_OBJ_ID(sub_id) (ECORE_AUDIO_OBJ_BASE_ID + EO_TYPECHECK(enum Ecore_Audio_Obj_Sub_Ids, sub_id))

/**
 * @brief Set the name of the object
 *
 * @since 1.8
 *
 * @param[in] name
 */
#define ecore_audio_obj_name_set(name) ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_NAME_SET), EO_TYPECHECK(const char *, name)

/**
 * @brief Get the name of the object
 *
 * @since 1.8
 *
 * @param[out] ret
 */
#define ecore_audio_obj_name_get(ret) ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_NAME_GET), EO_TYPECHECK(const char **, ret)

/**
 * @brief Set the paused state of the object
 *
 * @since 1.8
 *
 * @param[in]   paused EINA_TRUE to pause the object, EINA_FALSE to resume
 */
#define ecore_audio_obj_paused_set(paused) ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_PAUSED_SET), EO_TYPECHECK(Eina_Bool, paused)

/**
 * @brief Get the paused state of the object
 *
 * @since 1.8
 *
 * @param[out]   ret EINA_TRUE if object is paused, EINA_FALSE if not
 */
#define ecore_audio_obj_paused_get(ret) ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_PAUSED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @brief Set the volume of the object
 *
 * @since 1.8
 *
 * @param[in] volume The volume, 1.0 is the default, can be > 1.0
 */
#define ecore_audio_obj_volume_set(volume) ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_VOLUME_SET), EO_TYPECHECK(double, volume)

/**
 * @brief Get the volume of the object
 *
 * @since 1.8
 *
 * @param[out] ret The volume
 */
#define ecore_audio_obj_volume_get(ret) ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_VOLUME_GET), EO_TYPECHECK(double *, ret)

/**
 * @brief Set the source of an object
 *
 * @since 1.8
 *
 * What sources are supported depends on the actual object. For example,
 * the libsndfile class accepts WAV, OGG, FLAC files as source.
 *
 * @param[in] source The source to set to (i.e. file, URL, device)
 * @param[out] ret EINA_TRUE if the source was set correctly (i.e. the file was opened), EINA_FALSE otherwise
 */
#define ecore_audio_obj_source_set(source, ret) ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_SOURCE_SET), EO_TYPECHECK(const char *, source), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @brief Get the source of an object
 *
 * @since 1.8
 *
 * @param[out] ret The currently set source
 */
#define ecore_audio_obj_source_get(ret) ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_SOURCE_GET), EO_TYPECHECK(const char **, ret)

/**
 * @brief Set the format of an object
 *
 * @since 1.8
 *
 * What formats are supported depends on the actual object. Default is
 * ECORE_AUDIO_FORMAT_AUTO
 *
 * @param[in] format The format to set, of type Ecore_Audio_Format
 * @param[out] ret EINA_TRUE if the format was supported, EINA_FALSE otherwise
 */
#define ecore_audio_obj_format_set(format, ret) ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_FORMAT_SET), EO_TYPECHECK(Ecore_Audio_Format, format), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @brief Get the format of an object
 *
 * @since 1.8
 *
 * After setting the source if the format was ECORE_AUDIO_FORMAT_AUTO this
 * function will now return the actual format.
 *
 * @param[out] ret The format of the object
 */
#define ecore_audio_obj_format_get(ret) ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_FORMAT_GET), EO_TYPECHECK(Ecore_Audio_Format *, ret)

/**
 * @brief Set the virtual IO functions
 *
 * @since 1.8
 *
 * @param[in] vio The @ref Ecore_Audio_Vio struct with the function callbacks
 * @param[in] data User data to pass to the VIO functions
 * @param[in] free_func This function takes care to clean up @ref data when
 * the VIO is destroyed. NULL means do nothing.
 */
#define ecore_audio_obj_vio_set(vio, data, free_func) ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_VIO_SET), EO_TYPECHECK(Ecore_Audio_Vio *, vio), EO_TYPECHECK(void *, data), EO_TYPECHECK(eo_base_data_free_func, free_func)

/**
 * @}
 */
#ifdef __cplusplus
}
#endif

#endif
