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
