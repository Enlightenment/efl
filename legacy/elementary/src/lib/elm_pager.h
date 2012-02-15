/**
 * @defgroup Pager Pager
 *
 * @image html img/widget/pager/preview-00.png
 * @image latex img/widget/pager/preview-00.eps
 *
 * @brief Widget that allows flipping between one or more “pages”
 * of objects.
 *
 * The flipping between pages of objects is animated. All content
 * in the pager is kept in a stack, the top being the last content added
 * (visible one) on the top of that stack.
 *
 * Objects can be pushed or popped from the stack or deleted as
 * well. Pushes and pops will animate the widget according to its
 * style (a pop will also delete the child object once the
 * animation is finished). Any object already in the pager can be
 * promoted to the top (from its current stacking position) through
 * the use of elm_pager_content_promote(). New objects are pushed
 * to the top with elm_pager_content_push(). When the top item is
 * no longer wanted, simply pop it with elm_pager_content_pop() and
 * it will also be deleted. If an object is no longer needed and is
 * not the top item, just delete it as normal. You can get the objects 
 * at the top and bottom with
 * elm_pager_content_top_get() and elm_pager_content_bottom_get() 
 * respectively.
 *
 * Signals that you can add callbacks for are:
 * - @c "show,finished" - when a new page is actually shown on the top
 * - @c "hide,finished" - when a previous page is hidden
 *
 * Only after the first of that signals the child object is
 * guaranteed to be visible, as in @c evas_object_visible_get().
 *
 * This widget has the following styles available:
 * - @c "default"
 * - @c "fade"
 * - @c "fade_translucide"
 * - @c "fade_invisible"
 *
 * @note These styles affect only the flipping animations on the
 * default theme; the appearance when not animating is unaffected
 * by them.
 *
 * @ref tutorial_pager gives a good overview of the usage of the API.
 * @{
 */

/**
 * Add a new pager to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Pager
 */
EAPI Evas_Object *elm_pager_add(Evas_Object *parent);

/**
 * @brief Push an object to the top of the pager stack (and show it).
 *
 * @param obj The pager object
 * @param content The object to push
 *
 * The object pushed becomes a child of the pager, it will be controlled and
 * deleted when the pager is deleted.
 *
 * @note If the content is already in the stack use
 * elm_pager_content_promote().
 * @warning Using this function on @p content already in the stack results in
 * undefined behavior.
 */
EAPI void         elm_pager_content_push(Evas_Object *obj, Evas_Object *content);

/**
 * @brief Pop the object that is on top of the stack
 *
 * @param obj The pager object
 *
 * This pops the object that is on the top(visible) of the pager, makes it
 * disappear, then deletes the object. The object that was underneath it on
 * the stack will become visible.
 */
EAPI void         elm_pager_content_pop(Evas_Object *obj);

/**
 * @brief Moves an object already in the pager stack to the top of the stack.
 *
 * @param obj The pager object
 * @param content The object to promote
 *
 * This will take the @p content and move it to the top of the stack as
 * if it had been pushed there.
 *
 * @note If the content isn't already in the stack use
 * elm_pager_content_push().
 * @warning Using this function on @p content not already in the stack
 * results in undefined behavior.
 */
EAPI void         elm_pager_content_promote(Evas_Object *obj, Evas_Object *content);

/**
 * @brief Return the object at the bottom of the pager stack
 *
 * @param obj The pager object
 * @return The bottom object or NULL if none
 */
EAPI Evas_Object *elm_pager_content_bottom_get(const Evas_Object *obj);

/**
 * @brief  Return the object at the top of the pager stack
 *
 * @param obj The pager object
 * @return The top object or NULL if none
 */
EAPI Evas_Object *elm_pager_content_top_get(const Evas_Object *obj);

/**
 * @}
 */
