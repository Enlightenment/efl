#define ELM_OBJ_FLIP_CLASS elm_obj_flip_class_get()

const Eo_Class *elm_obj_flip_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_FLIP_BASE_ID;

enum
{
   ELM_OBJ_FLIP_SUB_ID_FRONT_VISIBLE_GET,
   ELM_OBJ_FLIP_SUB_ID_PERSPECTIVE_SET,
   ELM_OBJ_FLIP_SUB_ID_GO,
   ELM_OBJ_FLIP_SUB_ID_GO_TO,
   ELM_OBJ_FLIP_SUB_ID_INTERACTION_SET,
   ELM_OBJ_FLIP_SUB_ID_INTERACTION_GET,
   ELM_OBJ_FLIP_SUB_ID_INTERACTION_DIRECTION_ENABLED_SET,
   ELM_OBJ_FLIP_SUB_ID_INTERACTION_DIRECTION_ENABLED_GET,
   ELM_OBJ_FLIP_SUB_ID_INTERACTION_DIRECTION_HITSIZE_SET,
   ELM_OBJ_FLIP_SUB_ID_INTERACTION_DIRECTION_HITSIZE_GET,
   ELM_OBJ_FLIP_SUB_ID_LAST
};

#define ELM_OBJ_FLIP_ID(sub_id) (ELM_OBJ_FLIP_BASE_ID + sub_id)


/**
 * @def elm_obj_flip_front_visible_get
 * @since 1.8
 *
 * @brief Get flip front visibility state
 *
 * @param[out] ret
 *
 * @see elm_flip_front_visible_get
 *
 * @ingroup Flip
 */
#define elm_obj_flip_front_visible_get(ret) ELM_OBJ_FLIP_ID(ELM_OBJ_FLIP_SUB_ID_FRONT_VISIBLE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_flip_go_to
 * @since 1.8
 *
 * @brief Runs the flip animation to front or back.
 *
 * @param[in] front
 * @param[in] mode
 *
 * @see elm_flip_go_to
 *
 * @ingroup Flip
 */
#define elm_obj_flip_go_to(front, mode) ELM_OBJ_FLIP_ID(ELM_OBJ_FLIP_SUB_ID_GO_TO), EO_TYPECHECK(Eina_Bool, front), EO_TYPECHECK(Elm_Flip_Mode, mode)

/**
 * @def elm_obj_flip_go
 * @since 1.8
 *
 * @brief Runs the flip animation
 *
 * @param[in] mode
 *
 * @see elm_flip_go
 *
 * @ingroup Flip
 */
#define elm_obj_flip_go(mode) ELM_OBJ_FLIP_ID(ELM_OBJ_FLIP_SUB_ID_GO), EO_TYPECHECK(Elm_Flip_Mode, mode)

/**
 * @def elm_obj_flip_interaction_set
 * @since 1.8
 *
 * @brief Set the interactive flip mode
 *
 * @param[in] mode
 *
 * @see elm_flip_interaction_set
 *
 * @ingroup Flip
 */
#define elm_obj_flip_interaction_set(mode) ELM_OBJ_FLIP_ID(ELM_OBJ_FLIP_SUB_ID_INTERACTION_SET), EO_TYPECHECK(Elm_Flip_Interaction, mode)

/**
 * @def elm_obj_flip_interaction_get
 * @since 1.8
 *
 * @brief Get the interactive flip mode
 *
 * @param[out] ret
 *
 * @see elm_flip_interaction_get
 *
 * @ingroup Flip
 */
#define elm_obj_flip_interaction_get(ret) ELM_OBJ_FLIP_ID(ELM_OBJ_FLIP_SUB_ID_INTERACTION_GET), EO_TYPECHECK(Elm_Flip_Interaction *, ret)

/**
 * @def elm_obj_flip_interaction_direction_enabled_set
 * @since 1.8
 *
 * @brief Set which directions of the flip respond to interactive flip
 *
 * @param[in] dir
 * @param[in] enabled
 *
 * @see elm_flip_interaction_direction_enabled_set
 *
 * @ingroup Flip
 */
#define elm_obj_flip_interaction_direction_enabled_set(dir, enabled) ELM_OBJ_FLIP_ID(ELM_OBJ_FLIP_SUB_ID_INTERACTION_DIRECTION_ENABLED_SET), EO_TYPECHECK(Elm_Flip_Direction, dir), EO_TYPECHECK(Eina_Bool, enabled)

/**
 * @def elm_obj_flip_interaction_direction_enabled_get
 * @since 1.8
 *
 * @brief Get the enabled state of that flip direction
 *
 * @param[in] dir
 * @param[out] ret
 *
 * @see elm_flip_interaction_direction_enabled_get
 *
 * @ingroup Flip
 */
#define elm_obj_flip_interaction_direction_enabled_get(dir, ret) ELM_OBJ_FLIP_ID(ELM_OBJ_FLIP_SUB_ID_INTERACTION_DIRECTION_ENABLED_GET), EO_TYPECHECK(Elm_Flip_Direction, dir), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_flip_interaction_direction_hitsize_set
 * @since 1.8
 *
 * @brief Set the amount of the flip that is sensitive to interactive flip
 *
 * @param[in] dir
 * @param[in] hitsize
 *
 * @see elm_flip_interaction_direction_hitsize_set
 *
 * @ingroup Flip
 */
#define elm_obj_flip_interaction_direction_hitsize_set(dir, hitsize) ELM_OBJ_FLIP_ID(ELM_OBJ_FLIP_SUB_ID_INTERACTION_DIRECTION_HITSIZE_SET), EO_TYPECHECK(Elm_Flip_Direction, dir), EO_TYPECHECK(double, hitsize)

/**
 * @def elm_obj_flip_interaction_direction_hitsize_get
 * @since 1.8
 *
 * @brief Get the amount of the flip that is sensitive to interactive flip
 *
 * @param[in] dir
 * @param[out] ret
 *
 * @see elm_flip_interaction_direction_hitsize_get
 *
 * @ingroup Flip
 */
#define elm_obj_flip_interaction_direction_hitsize_get(dir, ret) ELM_OBJ_FLIP_ID(ELM_OBJ_FLIP_SUB_ID_INTERACTION_DIRECTION_HITSIZE_GET), EO_TYPECHECK(Elm_Flip_Direction, dir), EO_TYPECHECK(double *, ret)
