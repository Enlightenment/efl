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
 * @brief Audio Object
 */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @addtogroup Ecore_Audio_Group
 * @{
 */

#define ECORE_AUDIO_OBJ_CLASS ecore_audio_obj_class_get()

const Eo_Class *ecore_audio_obj_class_get() EINA_CONST;

extern EAPI Eo_Op ECORE_AUDIO_OBJ_BASE_ID;

enum
{
   ECORE_AUDIO_OBJ_SUB_ID_NAME_SET,
   ECORE_AUDIO_OBJ_SUB_ID_NAME_GET,
   ECORE_AUDIO_OBJ_SUB_ID_PAUSED_SET,
   ECORE_AUDIO_OBJ_SUB_ID_PAUSED_GET,
   ECORE_AUDIO_OBJ_SUB_ID_VOLUME_SET,
   ECORE_AUDIO_OBJ_SUB_ID_VOLUME_GET,
   ECORE_AUDIO_OBJ_SUB_ID_SOURCE_SET,
   ECORE_AUDIO_OBJ_SUB_ID_SOURCE_GET,
   ECORE_AUDIO_OBJ_SUB_ID_LAST
};

#define ECORE_AUDIO_OBJ_ID(sub_id) (ECORE_AUDIO_OBJ_BASE_ID + sub_id)

/*
 * @def ecore_audio_obj_name_set
 * @since 1.8
 *
 * Sets the name of the object
 *
 * @param[in] name
 */
#define ecore_audio_obj_name_set(name) ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_NAME_SET), EO_TYPECHECK(const char *, name)

/*
 * @def ecore_audio_obj_name_get
 * @since 1.8
 *
 * Gets the name of the object
 *
 * @param[out] name
 */
#define ecore_audio_obj_name_get(name) ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_NAME_GET), EO_TYPECHECK(const char **, name)

#define ecore_audio_obj_paused_set(paused) ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_PAUSED_SET), EO_TYPECHECK(Eina_Bool, paused)

#define ecore_audio_obj_paused_get(ret) ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_PAUSED_GET), EO_TYPECHECK(Eina_Bool *, ret)

#define ecore_audio_obj_volume_set(volume) ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_VOLUME_SET), EO_TYPECHECK(double, volume)

#define ecore_audio_obj_volume_get(ret) ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_VOLUME_GET), EO_TYPECHECK(double *, ret)

#define ecore_audio_obj_source_set(source) ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_SOURCE_SET), EO_TYPECHECK(const char *, source)

#define ecore_audio_obj_source_get(source) ECORE_AUDIO_OBJ_ID(ECORE_AUDIO_OBJ_SUB_ID_SOURCE_GET), EO_TYPECHECK(const char **, source)

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
