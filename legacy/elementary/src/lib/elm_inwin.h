/**
 * @defgroup Inwin Inwin
 * @ingroup Win
 *
 * @image html inwin_inheritance_tree.png
 * @image latex inwin_inheritance_tree.eps
 *
 * @image html img/widget/inwin/preview-00.png
 * @image latex img/widget/inwin/preview-00.eps
 * @image html img/widget/inwin/preview-01.png
 * @image latex img/widget/inwin/preview-01.eps
 * @image html img/widget/inwin/preview-02.png
 * @image latex img/widget/inwin/preview-02.eps
 *
 * An inwin is a window inside a window that is useful for a quick popup.
 * It does not hover.
 *
 * It works by creating an object that will occupy the entire window, so it
 * must be created using an @ref Win "elm_win" as parent only. The inwin
 * object can be hidden or restacked below every other object if it's
 * needed to show what's behind it without destroying it. If this is done,
 * the elm_win_inwin_activate() function can be used to bring it back to
 * full visibility again.
 *
 * There are three styles available in the default theme. These are:
 * @li default: The inwin is sized to take over most of the window it's
 * placed in.
 * @li minimal: The size of the inwin will be the minimum necessary to show
 * its contents.
 * @li minimal_vertical: Horizontally, the inwin takes as much space as
 * possible, but it's sized vertically the most it needs to fit its\
 * contents.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for inner windown objects. It also
 * emits the signals inherited from @ref Layout.
 *
 * Some examples of Inwin can be found in the following:
 * @li @ref inwin_example_01
 *
 * @{
 */

/**
 * Adds an inwin to the current window
 *
 * The @p obj used as parent @b MUST be an @ref Win "Elementary Window".
 * Never call this function with anything other than the top-most window
 * as its parameter, unless you are fond of undefined behavior.
 *
 * After creating the object, the widget will set itself as resize object
 * for the window with elm_win_resize_object_add(), so when shown it will
 * appear to cover almost the entire window (how much of it depends on its
 * content and the style used). It must not be added into other container
 * objects and it needs not be moved or resized manually.
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Inwin
 */
EAPI Evas_Object *elm_win_inwin_add(Evas_Object *parent);

/**
 * Activates an inwin object, ensuring its visibility
 *
 * This function will make sure that the inwin @p obj is completely visible
 * by calling evas_object_show() and evas_object_raise() on it, to bring it
 * to the front. It also sets the keyboard focus to it, which will be passed
 * onto its content.
 *
 * The object's theme will also receive the signal "elm,action,show" with
 * source "elm".
 *
 * @param obj The inwin to activate
 *
 * @ingroup Inwin
 */
EAPI void         elm_win_inwin_activate(Evas_Object *obj);

/**
 * Set the content of an inwin object.
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_win_inwin_content_unset() function.
 *
 * @param obj The inwin object
 * @param content The object to set as content
 *
 * @ingroup Inwin
 */
EAPI void         elm_win_inwin_content_set(Evas_Object *obj, Evas_Object *content);

/**
 * Get the content of an inwin object.
 *
 * Return the content object for this widget.
 *
 * The returned object is valid as long as the inwin is still alive and no
 * other content is set on it. Deleting the object will notify the inwin
 * about it and this one will be left empty.
 *
 * If you need to remove an inwin's content to be reused somewhere else,
 * see elm_win_inwin_content_unset().
 *
 * @param obj The inwin object
 * @return The content that is being used
 *
 * @ingroup Inwin
 */
EAPI Evas_Object *elm_win_inwin_content_get(const Evas_Object *obj);

/**
 * Unset the content of an inwin object.
 *
 * Unparent and return the content object which was set for this widget.
 *
 * @param obj The inwin object
 * @return The content that was being used
 *
 * @ingroup Inwin
 */
EAPI Evas_Object *elm_win_inwin_content_unset(Evas_Object *obj);

/**
 * @}
 */

