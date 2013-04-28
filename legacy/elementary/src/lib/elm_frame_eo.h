#define ELM_OBJ_FRAME_CLASS elm_obj_frame_class_get()

const Eo_Class *elm_obj_frame_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_FRAME_BASE_ID;

enum
{
   ELM_OBJ_FRAME_SUB_ID_AUTOCOLLAPSE_SET,
   ELM_OBJ_FRAME_SUB_ID_AUTOCOLLAPSE_GET,
   ELM_OBJ_FRAME_SUB_ID_COLLAPSE_SET,
   ELM_OBJ_FRAME_SUB_ID_COLLAPSE_GO,
   ELM_OBJ_FRAME_SUB_ID_COLLAPSE_GET,
   ELM_OBJ_FRAME_SUB_ID_LAST
};

#define ELM_OBJ_FRAME_ID(sub_id) (ELM_OBJ_FRAME_BASE_ID + sub_id)


/**
 * @def elm_obj_frame_autocollapse_set
 * @since 1.8
 *
 * @brief Toggle autocollapsing of a frame
 *
 * @param[in] autocollapse
 *
 * @see elm_frame_autocollapse_set
 *
 * @ingroup Frame
 */
#define elm_obj_frame_autocollapse_set(autocollapse) ELM_OBJ_FRAME_ID(ELM_OBJ_FRAME_SUB_ID_AUTOCOLLAPSE_SET), EO_TYPECHECK(Eina_Bool, autocollapse)

/**
 * @def elm_obj_frame_autocollapse_get
 * @since 1.8
 *
 * @brief Determine autocollapsing of a frame
 *
 * @param[out] ret
 *
 * @see elm_frame_autocollapse_get
 *
 * @ingroup Frame
 */
#define elm_obj_frame_autocollapse_get(ret) ELM_OBJ_FRAME_ID(ELM_OBJ_FRAME_SUB_ID_AUTOCOLLAPSE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_frame_collapse_set
 * @since 1.8
 *
 * @brief Manually collapse a frame without animations
 *
 * @param[in] collapse
 *
 * @see elm_frame_collapse_set
 *
 * @ingroup Frame
 */
#define elm_obj_frame_collapse_set(collapse) ELM_OBJ_FRAME_ID(ELM_OBJ_FRAME_SUB_ID_COLLAPSE_SET), EO_TYPECHECK(Eina_Bool, collapse)

/**
 * @def elm_obj_frame_collapse_go
 * @since 1.8
 *
 * @brief Manually collapse a frame with animations
 *
 * @param[in] collapse
 *
 * @see elm_frame_collapse_go
 */
#define elm_obj_frame_collapse_go(collapse) ELM_OBJ_FRAME_ID(ELM_OBJ_FRAME_SUB_ID_COLLAPSE_GO), EO_TYPECHECK(Eina_Bool, collapse)

/**
 * @def elm_obj_frame_collapse_get
 * @since 1.8
 *
 * @brief Determine the collapse state of a frame
 *
 * @param[out] ret
 *
 * @see elm_frame_collapse_get
 *
 * @ingroup Frame
 */
#define elm_obj_frame_collapse_get(ret) ELM_OBJ_FRAME_ID(ELM_OBJ_FRAME_SUB_ID_COLLAPSE_GET), EO_TYPECHECK(Eina_Bool *, ret)
