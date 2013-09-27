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
 * @brief Ecore_Audio output object
 */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup ecore_audio_obj_out - Ecore_Audio output object
 * @ingroup Ecore_Audio_Group
 * @{
 */

#define ECORE_AUDIO_OBJ_OUT_CLASS ecore_audio_obj_out_class_get() /**< Ecore_Audio output object class */

/**
 * @brief Get the Eo class ID
 *
 * @return The Eo class ID
 */
const Eo_Class *ecore_audio_obj_out_class_get() EINA_CONST;

extern EAPI Eo_Op ECORE_AUDIO_OBJ_OUT_BASE_ID;

enum Ecore_Audio_Obj_Out_Sub_Ids
{
   ECORE_AUDIO_OBJ_OUT_SUB_ID_INPUT_ATTACH,
   ECORE_AUDIO_OBJ_OUT_SUB_ID_INPUT_DETACH,
   ECORE_AUDIO_OBJ_OUT_SUB_ID_INPUTS_GET,
   ECORE_AUDIO_OBJ_OUT_SUB_ID_LAST
};

#define ECORE_AUDIO_OBJ_OUT_ID(sub_id) (ECORE_AUDIO_OBJ_OUT_BASE_ID + EO_TYPECHECK(enum Ecore_Audio_Obj_Out_Sub_Ids, sub_id))

/**
 * @brief Attach an input to an output
 *
 * @since 1.8
 *
 * @param[in] input The input to attach to the output
 * @param[out] ret EINA_TRUE if the input was attached, EINA_FALSE otherwise
 */
#define ecore_audio_obj_out_input_attach(input, ret) ECORE_AUDIO_OBJ_OUT_ID(ECORE_AUDIO_OBJ_OUT_SUB_ID_INPUT_ATTACH), EO_TYPECHECK(Eo *, input), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @brief Detach an input from an output
 *
 * @since 1.8
 *
 * @param[in] input The input to detach to the output
 * @param[out] ret EINA_TRUE if the input was detached, EINA_FALSE otherwise
 */
#define ecore_audio_obj_out_input_detach(input, ret) ECORE_AUDIO_OBJ_OUT_ID(ECORE_AUDIO_OBJ_OUT_SUB_ID_INPUT_DETACH), EO_TYPECHECK(Eo *, input), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @brief Detach an input from an output
 *
 * @since 1.8
 *
 * @param[out] inputs An @ref Eina_List of the inputs that are attached to
 * the output
 */
#define ecore_audio_obj_out_inputs_get(inputs) ECORE_AUDIO_OBJ_OUT_ID(ECORE_AUDIO_OBJ_OUT_SUB_ID_INPUTS_GET), EO_TYPECHECK(Eina_List **, inputs)

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif

