/**
 * @defgroup Tooltips Tooltips
 * @ingroup Elementary
 *
 * The Tooltip is an (internal, for now) smart object used to show a
 * content in a frame on mouse hover of objects(or widgets), with
 * tips/information about them.
 *
 * @{
 */

/**
 * @brief Possible orient values for tooltip.
 *
 * These values should be used in conjunction to elm_object_tooltip_orient_set() to
 * set the position around which the tooltip should appear(relative to its parent)
 *
 * @ingroup Tooltips
 */
typedef enum
{
   ELM_TOOLTIP_ORIENT_NONE = 0, /**< Default value, Tooltip moves with mouse pointer */
   ELM_TOOLTIP_ORIENT_TOP_LEFT, /**< Tooltip should appear at the top left of parent */
   ELM_TOOLTIP_ORIENT_TOP, /**< Tooltip should appear at the top of parent */
   ELM_TOOLTIP_ORIENT_TOP_RIGHT, /**< Tooltip should appear at the top right of parent */
   ELM_TOOLTIP_ORIENT_LEFT, /**< Tooltip should appear at the left of parent */
   ELM_TOOLTIP_ORIENT_CENTER, /**< Tooltip should appear at the center of parent */
   ELM_TOOLTIP_ORIENT_RIGHT, /**< Tooltip should appear at the right of parent */
   ELM_TOOLTIP_ORIENT_BOTTOM_LEFT, /**< Tooltip should appear at the bottom left of parent */
   ELM_TOOLTIP_ORIENT_BOTTOM, /**< Tooltip should appear at the bottom of parent */
   ELM_TOOLTIP_ORIENT_BOTTOM_RIGHT, /**< Tooltip should appear at the bottom right of parent */
   ELM_TOOLTIP_ORIENT_LAST /**< Sentinel value, @b don't use */
 } Elm_Tooltip_Orient;

/**
 * This increments the tooltip movement freeze count by one. If the count
 * is more than 0, the tooltip position will be fixed.
 *
 * @param obj The tooltip's anchor object
 *
 * @ingroup Tooltips
 * @see elm_object_tooltip_move_freeze_pop()
 * @see elm_tooltio_move_freeze_get()
 * @since 1.9
 */
EAPI void elm_object_tooltip_move_freeze_push(Evas_Object *obj);

/**
 * This decrements the tooltip freeze count by one.
 *
 * @param obj The tooltip's anchor object
 *
 * @ingroup Tooltips
 * @see elm_object_tooltip_move_freeze_push()
 * @since 1.9
 */
EAPI void elm_object_tooltip_move_freeze_pop(Evas_Object *obj);

/**
 * Get the movement freeze by 1
 *
 * This gets the movement freeze count by one.
 *
 * @param obj The tooltip's anchor object
 * @return The movement freeze count
 *
 * @ingroup Tooltips
 * @see elm_object_tooltip_move_freeze_push()
 * @since 1.9
 */
EAPI int elm_object_tooltip_move_freeze_get(const Evas_Object *obj);

/**
 * @def elm_object_tooltip_orient_set
 * @since 1.9
 *
 * @brief Sets the orientation of the tooltip around the owner region
 *
 * Sets the position in which tooltip will appear around its owner. By default,
 * #ELM_TOOLTIP_ORIENT_NONE is set.
 *
 * @param[in] obj owner widget.
 * @param[in] orient orientation.
 *
 * @ingroup Tooltips
 * @see @ref Elm_Tooltip_Orient for possible values.
 */
EAPI void elm_object_tooltip_orient_set(Evas_Object *obj, Elm_Tooltip_Orient orient);

/**
 * @brief Returns the orientation of Tooltip
 *
 * @param obj The owner object
 * @return The orientation of the tooltip
 *
 * @ingroup Tooltips
 * @see elm_object_tooltip_orient_set()
 * @ref Elm_Tooltip_Orient for possible values.
 */
EAPI Elm_Tooltip_Orient elm_object_tooltip_orient_get(const Evas_Object *obj);

/**
 * Called back when a widget's tooltip is activated and needs content.
 * @param data user-data given to elm_object_tooltip_content_cb_set()
 * @param obj owner widget.
 * @param tooltip The tooltip object (affix content to this!)
 */
typedef Evas_Object *(*Elm_Tooltip_Content_Cb)(void *data, Evas_Object *obj, Evas_Object *tooltip);

/**
 * Called back when a widget's item tooltip is activated and needs content.
 * @param data user-data given to elm_object_tooltip_content_cb_set()
 * @param obj owner widget.
 * @param tooltip The tooltip object (affix content to this!)
 * @param item context dependent item. As an example, if tooltip was
 *        set on elm_list item, then it is of this type.
 */
typedef Evas_Object *(*Elm_Tooltip_Item_Content_Cb)(void *data, Evas_Object *obj, Evas_Object *tooltip, void *item);

EAPI void        elm_object_tooltip_show(Evas_Object *obj);
EAPI void        elm_object_tooltip_hide(Evas_Object *obj);

/**
 * Set the text to be displayed inside the tooltip.
 *
 * @param obj The tooltip object.
 * @param text The text to be displayed.
 *
 * @ingroup Tooltips
 */
EAPI void        elm_object_tooltip_text_set(Evas_Object *obj, const char *text);
EAPI void        elm_object_tooltip_domain_translatable_text_set(Evas_Object *obj, const char *domain, const char *text);
#define elm_object_tooltip_translatable_text_set(obj, text) elm_object_tooltip_domain_translatable_text_set((obj), NULL, (text))
EAPI void        elm_object_tooltip_content_cb_set(Evas_Object *obj, Elm_Tooltip_Content_Cb func, const void *data, Evas_Smart_Cb del_cb);
EAPI void        elm_object_tooltip_unset(Evas_Object *obj);

/**
 * Sets a different style for this object tooltip.
 *
 * @note before you set a style you should define a tooltip with
 *       elm_object_tooltip_content_cb_set() or
 *       elm_object_tooltip_text_set().
 *
 * @param obj an object with tooltip already set.
 * @param style the theme style to use (default, transparent, ...)
 *
 * @ingroup Tooltips
 */
EAPI void        elm_object_tooltip_style_set(Evas_Object *obj, const char *style);

/**
 * Get the style for this object tooltip.
 *
 * @param obj an object with tooltip already set.
 * @return style the theme style in use, defaults to "default". If the
 *         object does not have a tooltip set, then NULL is returned.
 *
 * @ingroup Tooltips
 */
EAPI const char *elm_object_tooltip_style_get(const Evas_Object *obj);

/**
 * @brief Disable size restrictions on an object's tooltip
 * @param obj The tooltip's anchor object
 * @param disable If EINA_TRUE, size restrictions are disabled
 * @return EINA_FALSE on failure, EINA_TRUE on success
 *
 * This function allows a tooltip to expand beyond its parent window's canvas.
 * It will instead be limited only by the size of the display.
 *
 * @ingroup Tooltips
 */
EAPI Eina_Bool   elm_object_tooltip_window_mode_set(Evas_Object *obj, Eina_Bool disable);

/**
 * @brief Retrieve size restriction state of an object's tooltip
 * @param obj The tooltip's anchor object
 * @return If EINA_TRUE, size restrictions are disabled
 *
 * This function returns whether a tooltip is allowed to expand beyond
 * its parent window's canvas.
 * It will instead be limited only by the size of the display.
 *
 * @ingroup Tooltips
 */
EAPI Eina_Bool   elm_object_tooltip_window_mode_get(const Evas_Object *obj);

/**
 * @}
 */
