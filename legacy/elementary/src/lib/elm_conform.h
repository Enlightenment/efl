/**
 * @defgroup Conformant Conformant
 * @ingroup Elementary
 *
 * @image html img/widget/conformant/preview-00.png
 * @image latex img/widget/conformant/preview-00.eps width=\textwidth
 *
 * @image html img/conformant.png
 * @image latex img/conformant.eps width=\textwidth
 *
 * The aim is to provide a widget that can be used in elementary apps to
 * account for space taken up by the indicator, virtual keypad & softkey
 * windows when running the illume2 module of E17.
 *
 * So conformant content will be sized and positioned considering the
 * space required for such stuff, and when they popup, as a keyboard
 * shows when an entry is selected, conformant content won't change.
 *
 * Available styles for it:
 * - @c "default"
 *
 * Default contents parts of the conformant widget that you can use for are:
 * @li "default" - A content of the conformant
 *
 * See how to use this widget in this example:
 * @ref conformant_example
 */

/**
 * @addtogroup Conformant
 * @{
 */

/**
 * Add a new conformant widget to the given parent Elementary
 * (container) object.
 *
 * @param parent The parent object.
 * @return A new conformant widget handle or @c NULL, on errors.
 *
 * This function inserts a new conformant widget on the canvas.
 *
 * @ingroup Conformant
 */
EAPI Evas_Object *
                                  elm_conformant_add(Evas_Object *parent)
EINA_ARG_NONNULL(1);

/**
 * Set the content of the conformant widget.
 *
 * @param obj The conformant object.
 * @param content The content to be displayed by the conformant.
 *
 * Content will be sized and positioned considering the space required
 * to display a virtual keyboard. So it won't fill all the conformant
 * size. This way is possible to be sure that content won't resize
 * or be re-positioned after the keyboard is displayed.
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_object_content_unset() function.
 *
 * @see elm_object_content_unset()
 * @see elm_object_content_get()
 *
 * @deprecated use elm_object_content_set() instead
 *
 * @ingroup Conformant
 */
EINA_DEPRECATED EAPI void         elm_conformant_content_set(Evas_Object *obj, Evas_Object *content) EINA_ARG_NONNULL(1);

/**
 * Get the content of the conformant widget.
 *
 * @param obj The conformant object.
 * @return The content that is being used.
 *
 * Return the content object which is set for this widget.
 * It won't be unparent from conformant. For that, use
 * elm_object_content_unset().
 *
 * @see elm_object_content_set().
 * @see elm_object_content_unset()
 *
 * @deprecated use elm_object_content_get() instead
 *
 * @ingroup Conformant
 */
EINA_DEPRECATED EAPI Evas_Object *elm_conformant_content_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Unset the content of the conformant widget.
 *
 * @param obj The conformant object.
 * @return The content that was being used.
 *
 * Unparent and return the content object which was set for this widget.
 *
 * @see elm_object_content_set().
 *
 * @deprecated use elm_object_content_unset() instead
 *
 * @ingroup Conformant
 */
EINA_DEPRECATED EAPI Evas_Object *elm_conformant_content_unset(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Returns the Evas_Object that represents the content area.
 *
 * @param obj The conformant object.
 * @return The content area of the widget.
 *
 * @ingroup Conformant
 */
EAPI Evas_Object                 *elm_conformant_content_area_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @}
 */
