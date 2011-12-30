/**
 * @defgroup Frame Frame
 *
 * @image html img/widget/frame/preview-00.png
 * @image latex img/widget/frame/preview-00.eps
 *
 * @brief Frame is a widget that holds some content and has a title.
 *
 * The default look is a frame with a title, but Frame supports multple
 * styles:
 * @li default
 * @li pad_small
 * @li pad_medium
 * @li pad_large
 * @li pad_huge
 * @li outdent_top
 * @li outdent_bottom
 *
 * Of all this styles only default shows the title. Frame emits no signals.
 *
 * Default contents parts of the frame widget that you can use for are:
 * @li "default" - A content of the frame
 *
 * Default text parts of the frame widget that you can use for are:
 * @li "elm.text" - Label of the frame
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
 */
EAPI Evas_Object                 *elm_frame_add(Evas_Object *parent) EINA_ARG_NONNULL(1);

/**
 * @}
 */
