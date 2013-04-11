#ifndef ECORE_AUDIO_OBJ_OUT_H
#define ECORE_AUDIO_OBJ_OUT_H

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
 * @file ecore_audio_obj_out.h
 * @brief Audio Output Object
 */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @addtogroup Ecore_Audio_Group
 * @{
 */

#define ECORE_AUDIO_OBJ_OUT_CLASS ecore_audio_obj_out_class_get()

const Eo_Class *ecore_audio_obj_out_class_get() EINA_CONST;

extern EAPI Eo_Op ECORE_AUDIO_OBJ_OUT_BASE_ID;

enum
{
   ECORE_AUDIO_OBJ_OUT_SUB_ID_INPUT_ATTACH,
   ECORE_AUDIO_OBJ_OUT_SUB_ID_INPUT_DETACH,
   ECORE_AUDIO_OBJ_OUT_SUB_ID_INPUTS_GET,
   ECORE_AUDIO_OBJ_OUT_SUB_ID_LAST
};

#define ECORE_AUDIO_OBJ_OUT_ID(sub_id) (ECORE_AUDIO_OBJ_OUT_BASE_ID + sub_id)

#define ecore_audio_obj_out_input_attach(input) ECORE_AUDIO_OBJ_OUT_ID(ECORE_AUDIO_OBJ_OUT_SUB_ID_INPUT_ATTACH), EO_TYPECHECK(Eo *, input)

#define ecore_audio_obj_out_input_detach(input) ECORE_AUDIO_OBJ_OUT_ID(ECORE_AUDIO_OBJ_OUT_SUB_ID_INPUT_DETACH), EO_TYPECHECK(Eo *, input)

#define ecore_audio_obj_out_inputs_get(input) ECORE_AUDIO_OBJ_OUT_ID(ECORE_AUDIO_OBJ_OUT_SUB_ID_INPUTS_GET), EO_TYPECHECK(Eina_List **, input)



/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif

