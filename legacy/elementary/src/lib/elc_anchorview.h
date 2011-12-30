/**
 * @defgroup Anchorview Anchorview
 *
 * @image html img/widget/anchorview/preview-00.png
 * @image latex img/widget/anchorview/preview-00.eps
 *
 * Anchorview is for displaying text that contains markup with anchors
 * like <c>\<a href=1234\>something\</\></c> in it.
 *
 * Besides being styled differently, the anchorview widget provides the
 * necessary functionality so that clicking on these anchors brings up a
 * popup with user defined content such as "call", "add to contacts" or
 * "open web page". This popup is provided using the @ref Hover widget.
 *
 * This widget is very similar to @ref Anchorblock, so refer to that
 * widget for an example. The only difference Anchorview has is that the
 * widget is already provided with scrolling functionality, so if the
 * text set to it is too large to fit in the given space, it will scroll,
 * whereas the @ref Anchorblock widget will keep growing to ensure all the
 * text can be displayed.
 *
 * This widget emits the following signals:
 * @li "anchor,clicked": will be called when an anchor is clicked. The
 * @p event_info parameter on the callback will be a pointer of type
 * ::Elm_Entry_Anchorview_Info.
 *
 * See @ref Anchorblock for an example on how to use both of them.
 *
 * @see Anchorblock
 * @see Entry
 * @see Hover
 *
 * @{
 */

/**
 * @typedef Elm_Entry_Anchorview_Info
 *
 * The info sent in the callback for "anchor,clicked" signals emitted by
 * the Anchorview widget.
 */
typedef struct _Elm_Entry_Anchorview_Info Elm_Entry_Anchorview_Info;

/**
 * @struct _Elm_Entry_Anchorview_Info
 *
 * The info sent in the callback for "anchor,clicked" signals emitted by
 * the Anchorview widget.
 */
struct _Elm_Entry_Anchorview_Info
{
   const char  *name; /**< Name of the anchor, as indicated in its href
                           attribute */
   int          button; /**< The mouse button used to click on it */
   Evas_Object *hover; /**< The hover object to use for the popup */
   struct
   {
      Evas_Coord x, y, w, h;
   } anchor, /**< Geometry selection of text used as anchor */
     hover_parent; /**< Geometry of the object used as parent by the
                        hover */
   Eina_Bool    hover_left : 1; /**< Hint indicating if there's space
                                     for content on the left side of
                                     the hover. Before calling the
                                     callback, the widget will make the
                                     necessary calculations to check
                                     which sides are fit to be set with
                                     content, based on the position the
                                     hover is activated and its distance
                                     to the edges of its parent object
                                 */
   Eina_Bool    hover_right : 1; /**< Hint indicating content fits on
                                      the right side of the hover.
                                      See @ref hover_left */
   Eina_Bool    hover_top : 1; /**< Hint indicating content fits on top
                                    of the hover. See @ref hover_left */
   Eina_Bool    hover_bottom : 1; /**< Hint indicating content fits
                                       below the hover. See @ref
                                       hover_left */
};

/**
 * Add a new Anchorview object
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 */
EAPI Evas_Object                *elm_anchorview_add(Evas_Object *parent) EINA_ARG_NONNULL(1);

/**
 * Set the parent of the hover popup
 *
 * Sets the parent object to use by the hover created by the anchorview
 * when an anchor is clicked. See @ref Hover for more details on this.
 * If no parent is set, the same anchorview object will be used.
 *
 * @param obj The anchorview object
 * @param parent The object to use as parent for the hover
 */
EAPI void                        elm_anchorview_hover_parent_set(Evas_Object *obj, Evas_Object *parent) EINA_ARG_NONNULL(1);

/**
 * Get the parent of the hover popup
 *
 * Get the object used as parent for the hover created by the anchorview
 * widget. See @ref Hover for more details on this.
 *
 * @param obj The anchorview object
 * @return The object used as parent for the hover, NULL if none is set.
 */
EAPI Evas_Object                *elm_anchorview_hover_parent_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Set the style that the hover should use
 *
 * When creating the popup hover, anchorview will request that it's
 * themed according to @p style.
 *
 * @param obj The anchorview object
 * @param style The style to use for the underlying hover
 *
 * @see elm_object_style_set()
 */
EAPI void                        elm_anchorview_hover_style_set(Evas_Object *obj, const char *style) EINA_ARG_NONNULL(1);

/**
 * Get the style that the hover should use
 *
 * Get the style the hover created by anchorview will use.
 *
 * @param obj The anchorview object
 * @return The style to use by the hover. NULL means the default is used.
 *
 * @see elm_object_style_set()
 */
EAPI const char                 *elm_anchorview_hover_style_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Ends the hover popup in the anchorview
 *
 * When an anchor is clicked, the anchorview widget will create a hover
 * object to use as a popup with user provided content. This function
 * terminates this popup, returning the anchorview to its normal state.
 *
 * @param obj The anchorview object
 */
EAPI void                        elm_anchorview_hover_end(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Set bouncing behaviour when the scrolled content reaches an edge
 *
 * Tell the internal scroller object whether it should bounce or not
 * when it reaches the respective edges for each axis.
 *
 * @param obj The anchorview object
 * @param h_bounce Whether to bounce or not in the horizontal axis
 * @param v_bounce Whether to bounce or not in the vertical axis
 *
 * @see elm_scroller_bounce_set()
 */
EAPI void                        elm_anchorview_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce) EINA_ARG_NONNULL(1);

/**
 * Get the set bouncing behaviour of the internal scroller
 *
 * Get whether the internal scroller should bounce when the edge of each
 * axis is reached scrolling.
 *
 * @param obj The anchorview object
 * @param h_bounce Pointer where to store the bounce state of the horizontal
 *                 axis
 * @param v_bounce Pointer where to store the bounce state of the vertical
 *                 axis
 *
 * @see elm_scroller_bounce_get()
 */
EAPI void                        elm_anchorview_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce) EINA_ARG_NONNULL(1);

/**
 * Appends a custom item provider to the given anchorview
 *
 * Appends the given function to the list of items providers. This list is
 * called, one function at a time, with the given @p data pointer, the
 * anchorview object and, in the @p item parameter, the item name as
 * referenced in its href string. Following functions in the list will be
 * called in order until one of them returns something different to NULL,
 * which should be an Evas_Object which will be used in place of the item
 * element.
 *
 * Items in the markup text take the form \<item relsize=16x16 vsize=full
 * href=item/name\>\</item\>
 *
 * @param obj The anchorview object
 * @param func The function to add to the list of providers
 * @param data User data that will be passed to the callback function
 *
 * @see elm_entry_item_provider_append()
 */
EAPI void                        elm_anchorview_item_provider_append(Evas_Object *obj, Evas_Object * (*func)(void *data, Evas_Object * anchorview, const char *item), void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Prepend a custom item provider to the given anchorview
 *
 * Like elm_anchorview_item_provider_append(), but it adds the function
 * @p func to the beginning of the list, instead of the end.
 *
 * @param obj The anchorview object
 * @param func The function to add to the list of providers
 * @param data User data that will be passed to the callback function
 */
EAPI void                        elm_anchorview_item_provider_prepend(Evas_Object *obj, Evas_Object * (*func)(void *data, Evas_Object * anchorview, const char *item), void *data) EINA_ARG_NONNULL(1, 2);

/**
 * Remove a custom item provider from the list of the given anchorview
 *
 * Removes the function and data pairing that matches @p func and @p data.
 * That is, unless the same function and same user data are given, the
 * function will not be removed from the list. This allows us to add the
 * same callback several times, with different @p data pointers and be
 * able to remove them later without conflicts.
 *
 * @param obj The anchorview object
 * @param func The function to remove from the list
 * @param data The data matching the function to remove from the list
 */
EAPI void                        elm_anchorview_item_provider_remove(Evas_Object *obj, Evas_Object * (*func)(void *data, Evas_Object * anchorview, const char *item), void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @}
 */
