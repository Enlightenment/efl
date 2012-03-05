/**
 * @defgroup Anchorblock Anchorblock
 *
 * @image html img/widget/anchorblock/preview-00.png
 * @image latex img/widget/anchorblock/preview-00.eps
 *
 * Anchorblock is for displaying text that contains markup with anchors
 * like <c>\<a href=1234\>something\</\></c> in it.
 *
 * Besides being styled differently, the anchorblock widget provides the
 * necessary functionality so that clicking on these anchors brings up a
 * popup with user defined content such as "call", "add to contacts" or
 * "open web page". This popup is provided using the @ref Hover widget.
 *
 * This widget emits the following signals:
 * @li "anchor,clicked": will be called when an anchor is clicked. The
 * @p event_info parameter on the callback will be a pointer of type
 * ::Elm_Entry_Anchorblock_Info.
 *
 * @see Anchorview
 * @see Entry
 * @see Hover
 *
 * Default text parts of the anchorblock widget that you can use for are:
 * @li "default" - A label of the anchorblock
 *
 * Supported elm_object common APIs.
 * @li elm_object_part_text_set
 * @li elm_object_part_text_get
 *
 * Since examples are usually better than plain words, we might as well
 * try @ref tutorial_anchorblock_example "one".
 */

/**
 * @addtogroup Anchorblock
 * @{
 */

/* XXX: Remove this awful widget. */

/**
 * @typedef Elm_Entry_Anchorblock_Info
 *
 * The info sent in the callback for "anchor,clicked" signals emitted by
 * the Anchorblock widget.
 */
typedef struct _Elm_Entry_Anchorblock_Info Elm_Entry_Anchorblock_Info;

/**
 * @struct _Elm_Entry_Anchorblock_Info
 *
 * The info sent in the callback for "anchor,clicked" signals emitted by
 * the Anchorblock widget.
 */
struct _Elm_Entry_Anchorblock_Info
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
 * Add a new Anchorblock object
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 */
EAPI Evas_Object                *elm_anchorblock_add(Evas_Object *parent);

/**
 * Set the parent of the hover popup
 *
 * Sets the parent object to use by the hover created by the anchorblock
 * when an anchor is clicked. See @ref Hover for more details on this.
 *
 * @param obj The anchorblock object
 * @param parent The object to use as parent for the hover
 */
EAPI void                        elm_anchorblock_hover_parent_set(Evas_Object *obj, Evas_Object *parent);

/**
 * Get the parent of the hover popup
 *
 * Get the object used as parent for the hover created by the anchorblock
 * widget. See @ref Hover for more details on this.
 * If no parent is set, the same anchorblock object will be used.
 *
 * @param obj The anchorblock object
 * @return The object used as parent for the hover, NULL if none is set.
 */
EAPI Evas_Object                *elm_anchorblock_hover_parent_get(const Evas_Object *obj);

/**
 * Set the style that the hover should use
 *
 * When creating the popup hover, anchorblock will request that it's
 * themed according to @p style.
 *
 * @param obj The anchorblock object
 * @param style The style to use for the underlying hover
 *
 * @see elm_object_style_set()
 */
EAPI void                        elm_anchorblock_hover_style_set(Evas_Object *obj, const char *style);

/**
 * Get the style that the hover should use
 *
 * Get the style, the hover created by anchorblock will use.
 *
 * @param obj The anchorblock object
 * @return The style to use by the hover. NULL means the default is used.
 *
 * @see elm_object_style_set()
 */
EAPI const char                 *elm_anchorblock_hover_style_get(const Evas_Object *obj);

/**
 * Ends the hover popup in the anchorblock
 *
 * When an anchor is clicked, the anchorblock widget will create a hover
 * object to use as a popup with user provided content. This function
 * terminates this popup, returning the anchorblock to its normal state.
 *
 * @param obj The anchorblock object
 */
EAPI void                        elm_anchorblock_hover_end(Evas_Object *obj);

/**
 * Appends a custom item provider to the given anchorblock
 *
 * Appends the given function to the list of items providers. This list is
 * called, one function at a time, with the given @p data pointer, the
 * anchorblock object and, in the @p item parameter, the item name as
 * referenced in its href string. Following functions in the list will be
 * called in order until one of them returns something different to NULL,
 * which should be an Evas_Object which will be used in place of the item
 * element.
 *
 * Items in the markup text take the form \<item relsize=16x16 vsize=full
 * href=item/name\>\</item\>
 *
 * @param obj The anchorblock object
 * @param func The function to add to the list of providers
 * @param data User data that will be passed to the callback function
 *
 * @see elm_entry_item_provider_append()
 */
EAPI void                        elm_anchorblock_item_provider_append(Evas_Object *obj, Evas_Object * (*func)(void *data, Evas_Object * anchorblock, const char *item), void *data);

/**
 * Prepend a custom item provider to the given anchorblock
 *
 * Like elm_anchorblock_item_provider_append(), but it adds the function
 * @p func to the beginning of the list, instead of the end.
 *
 * @param obj The anchorblock object
 * @param func The function to add to the list of providers
 * @param data User data that will be passed to the callback function
 */
EAPI void                        elm_anchorblock_item_provider_prepend(Evas_Object *obj, Evas_Object * (*func)(void *data, Evas_Object * anchorblock, const char *item), void *data);

/**
 * Remove a custom item provider from the list of the given anchorblock
 *
 * Removes the function and data pairing that matches @p func and @p data.
 * That is, unless the same function and same user data are given, the
 * function will not be removed from the list. This allows us to add the
 * same callback several times, with different @p data pointers and be
 * able to remove them later without conflicts.
 *
 * @param obj The anchorblock object
 * @param func The function to remove from the list
 * @param data The data matching the function to remove from the list
 */
EAPI void                        elm_anchorblock_item_provider_remove(Evas_Object *obj, Evas_Object * (*func)(void *data, Evas_Object * anchorblock, const char *item), void *data);

/**
 * @}
 */
