/**
 * @defgroup Bubble Bubble
 *
 * @image html img/widget/bubble/preview-00.png
 * @image latex img/widget/bubble/preview-00.eps
 * @image html img/widget/bubble/preview-01.png
 * @image latex img/widget/bubble/preview-01.eps
 * @image html img/widget/bubble/preview-02.png
 * @image latex img/widget/bubble/preview-02.eps
 *
 * @brief The Bubble is a widget to show text similar to how speech is
 * represented in comics.
 *
 * The bubble widget contains 5 important visual elements:
 * @li The frame is a rectangle with rounded edjes and an "arrow".
 * @li The @p icon is an image to which the frame's arrow points to.
 * @li The @p label is a text which appears to the right of the icon if the
 * corner is "top_left" or "bottom_left" and is right aligned to the frame
 * otherwise.
 * @li The @p info is a text which appears to the right of the label. Info's
 * font is of a ligther color than label.
 * @li The @p content is an evas object that is shown inside the frame.
 *
 * The position of the arrow, icon, label and info depends on which corner is
 * selected. The four available corners are:
 * @li "top_left" - Default
 * @li "top_right"
 * @li "bottom_left"
 * @li "bottom_right"
 *
 * Signals that you can add callbacks for are:
 * @li "clicked" - This is called when a user has clicked the bubble.
 *
 * Default contents parts of the bubble that you can use for are:
 * @li "default" - A content of the bubble
 * @li "icon" - An icon of the bubble
 *
 * Default text parts of the button widget that you can use for are:
 * @li NULL - Label of the bubble
 *
 * For an example of using a buble see @ref bubble_01_example_page "this".
 *
 * @{
 */

/**
 * Add a new bubble to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * This function adds a text bubble to the given parent evas object.
 */
EAPI Evas_Object                 *elm_bubble_add(Evas_Object *parent) EINA_ARG_NONNULL(1);

/**
 * Set the corner of the bubble
 *
 * @param obj The bubble object.
 * @param corner The given corner for the bubble.
 *
 * This function sets the corner of the bubble. The corner will be used to
 * determine where the arrow in the frame points to and where label, icon and
 * info are shown.
 *
 * Possible values for corner are:
 * @li "top_left" - Default
 * @li "top_right"
 * @li "bottom_left"
 * @li "bottom_right"
 */
EAPI void                         elm_bubble_corner_set(Evas_Object *obj, const char *corner) EINA_ARG_NONNULL(1, 2);

/**
 * Get the corner of the bubble
 *
 * @param obj The bubble object.
 * @return The given corner for the bubble.
 *
 * This function gets the selected corner of the bubble.
 */
EAPI const char                  *elm_bubble_corner_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @}
 */
