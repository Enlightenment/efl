#define ELM_OBJ_BUBBLE_CLASS elm_obj_bubble_class_get()

const Eo_Class *elm_obj_bubble_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_BUBBLE_BASE_ID;

enum
{
   ELM_OBJ_BUBBLE_SUB_ID_POS_SET,
   ELM_OBJ_BUBBLE_SUB_ID_POS_GET,
   ELM_OBJ_BUBBLE_SUB_ID_LAST
};

#define ELM_OBJ_BUBBLE_ID(sub_id) (ELM_OBJ_BUBBLE_BASE_ID + sub_id)


/**
 * @def elm_obj_bubble_pos_set
 * @since 1.8
 *
 * Set the corner of the bubble
 *
 * @param[in] pos
 *
 * @see elm_bubble_pos_set
 *
 * @ingroup Bubble
 */
#define elm_obj_bubble_pos_set(pos) ELM_OBJ_BUBBLE_ID(ELM_OBJ_BUBBLE_SUB_ID_POS_SET), EO_TYPECHECK(Elm_Bubble_Pos, pos)

/**
 * @def elm_obj_bubble_pos_get
 * @since 1.8
 *
 * Get the corner of the bubble
 *
 * @param[out] ret
 *
 * @see elm_bubble_pos_get
 *
 * @ingroup Bubble
 */
#define elm_obj_bubble_pos_get(ret) ELM_OBJ_BUBBLE_ID(ELM_OBJ_BUBBLE_SUB_ID_POS_GET), EO_TYPECHECK(Elm_Bubble_Pos *, ret)
