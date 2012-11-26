/**
 * @defgroup Frame Frame
 * @ingroup Elementary
 *
 * @image html frame_inheritance_tree.png
 * @image latex frame_inheritance_tree.eps
 *
 * @image html img/widget/frame/preview-00.png
 * @image latex img/widget/frame/preview-00.eps
 *
 * @brief Frame is a widget that holds some content and has a title.
 *
 * The default look is a frame with a title, but Frame supports multiple
 * styles:
 * @li default
 * @li pad_small
 * @li pad_medium
 * @li pad_large
 * @li pad_huge
 * @li outdent_top
 * @li outdent_bottom
 *
 * Of all this styles only default shows the title.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for frame objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * - @c "clicked" - The user has clicked the frame's label
 *
 * Default content parts of the frame widget that you can use for are:
 * @li "default" - A content of the frame
 *
 * Default text parts of the frame widget that you can use for are:
 * @li "default" - Label of the frame
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_part_text_set
 * @li @ref elm_object_part_text_get
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_get
 * @li @ref elm_object_part_content_unset
 *
 * For a detailed example see the @ref tutorial_frame.
 *
 * @{
 */

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
 */
#define elm_obj_frame_collapse_get(ret) ELM_OBJ_FRAME_ID(ELM_OBJ_FRAME_SUB_ID_COLLAPSE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @brief Add a new frame to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Frame
 */
EAPI Evas_Object                 *elm_frame_add(Evas_Object *parent);

/**
 * @brief Toggle autocollapsing of a frame
 * @param obj The frame
 * @param autocollapse Whether to enable autocollapse
 *
 * When @p enable is EINA_TRUE, clicking a frame's label will collapse the frame
 * vertically, shrinking it to the height of the label.
 * By default, this is DISABLED.
 *
 * @ingroup Frame
 */
EAPI void elm_frame_autocollapse_set(Evas_Object *obj, Eina_Bool autocollapse);

/**
 * @brief Determine autocollapsing of a frame
 * @param obj The frame
 * @return Whether autocollapse is enabled
 *
 * When this returns EINA_TRUE, clicking a frame's label will collapse the frame
 * vertically, shrinking it to the height of the label.
 * By default, this is DISABLED.
 *
 * @ingroup Frame
 */
EAPI Eina_Bool elm_frame_autocollapse_get(const Evas_Object *obj);

/**
 * @brief Manually collapse a frame without animations
 * @param obj The frame
 * @param collapse true to collapse, false to expand
 *
 * Use this to toggle the collapsed state of a frame, bypassing animations.
 *
 * @ingroup Frame
 */
EAPI void elm_frame_collapse_set(Evas_Object *obj, Eina_Bool collapse);

/**
 * @brief Determine the collapse state of a frame
 * @param obj The frame
 * @return true if collapsed, false otherwise
 *
 * Use this to determine the collapse state of a frame.
 *
 * @ingroup Frame
 */
EAPI Eina_Bool elm_frame_collapse_get(const Evas_Object *obj);

/**
 * @brief Manually collapse a frame with animations
 * @param obj The frame
 * @param collapse true to collapse, false to expand
 *
 * Use this to toggle the collapsed state of a frame, triggering animations.
 *
 * @ingroup Frame
 */
EAPI void elm_frame_collapse_go(Evas_Object *obj, Eina_Bool collapse);

/**
 * @}
 */
