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
