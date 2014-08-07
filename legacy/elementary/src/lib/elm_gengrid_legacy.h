/**
 * Add a new gengrid widget to the given parent Elementary
 * (container) object
 *
 * @param parent The parent object
 * @return a new gengrid widget handle or @c NULL, on errors
 *
 * This function inserts a new gengrid widget on the canvas.
 *
 * @see elm_gengrid_item_size_set()
 * @see elm_gengrid_group_item_size_set()
 * @see elm_gengrid_horizontal_set()
 * @see elm_gengrid_item_append()
 * @see elm_object_item_del()
 * @see elm_gengrid_clear()
 *
 * @ingroup Gengrid
 */
EAPI Evas_Object                  *elm_gengrid_add(Evas_Object *parent);

/**
 * Get the nth item, in a given gengrid widget, placed at position @p nth, in
 * its internal items list
 *
 * @param obj The gengrid object
 * @param nth The number of the item to grab (0 being the first)
 *
 * @return The item stored in @p obj at position @p nth or @c NULL, if there's
 * no item with that index (and on errors)
 *
 * @ingroup Genilst
 * @since 1.8
 */
EAPI Elm_Object_Item *elm_gengrid_nth_item_get(const Evas_Object *obj, unsigned int nth);

/**
 * Show a specific virtual region within the gengrid content object by page number.
 *
 * @param obj The gengrid object
 * @param h_pagenumber The horizontal page number
 * @param v_pagenumber The vertical page number
 *
 * 0, 0 of the indicated page is located at the top-left of the viewport.
 * This will jump to the page directly without animation.
 *
 * Example of usage:
 *
 * @code
 * sc = elm_gengrid_add(win);
 * elm_gengrid_content_set(sc, content);
 * elm_gengrid_page_relative_set(sc, 1, 0);
 * elm_gengrid_current_page_get(sc, &h_page, &v_page);
 * elm_gengrid_page_show(sc, h_page + 1, v_page);
 * @endcode
 *
 * @see elm_gengrid_page_bring_in()
 */
EAPI void                          elm_gengrid_page_show(const Evas_Object *obj, int h_pagenumber, int v_pagenumber);

/**
 * Set the scrollbar policy
 *
 * @param obj The gengrid object
 * @param policy_h Horizontal scrollbar policy.
 * @param policy_v Vertical scrollbar policy.
 *
 * This sets the scrollbar visibility policy for the given gengrid
 * scroller. #ELM_SCROLLER_POLICY_AUTO means the scrollbar is made
 * visible if it is needed, and otherwise kept
 * hidden. #ELM_SCROLLER_POLICY_ON turns it on all the time, and
 * #ELM_SCROLLER_POLICY_OFF always keeps it off.  This applies
 * respectively for the horizontal and vertical scrollbars.  Default
 * is #ELM_SCROLLER_POLICY_AUTO.
 *
 * @deprecated Use elm_scroller_policy_set() instead.
 *
 * @see elm_scroller_policy_set()
 *
 * @see elm_gengrid_scroller_policy_get()
 *
 * @ingroup Gengrid
 */
EINA_DEPRECATED EAPI void          elm_gengrid_scroller_policy_set(Evas_Object *obj, Elm_Scroller_Policy policy_h, Elm_Scroller_Policy policy_v);

/**
 * Get the scrollbar policy
 *
 * @param obj The gengrid object
 * @param policy_h Pointer to store the horizontal scrollbar policy.
 * @param policy_v Pointer to store the vertical scrollbar policy.
 *
 * @deprecated Use elm_scroller_policy_get() instead.
 *
 * @see elm_scroller_policy_get()
 *
 * @see elm_gengrid_scroller_policy_set()
 *
 * @ingroup Gengrid
 */
EINA_DEPRECATED EAPI void          elm_gengrid_scroller_policy_get(const Evas_Object *obj, Elm_Scroller_Policy *policy_h, Elm_Scroller_Policy *policy_v);

/**
 * Enable or disable bouncing effect for a given gengrid widget
 *
 * @param obj The gengrid object
 * @param h_bounce @c EINA_TRUE, to enable @b horizontal bouncing,
 * @c EINA_FALSE to disable it
 * @param v_bounce @c EINA_TRUE, to enable @b vertical bouncing,
 * @c EINA_FALSE to disable it
 *
 * The bouncing effect occurs whenever one reaches the gengrid's
 * edge's while panning it -- it will scroll past its limits a
 * little bit and return to the edge again, in a animated for,
 * automatically.
 *
 * @note By default, gengrids have bouncing enabled on both axis
 *
 * @deprecated Use elm_scroller_bounce_set() instead.
 *
 * @see elm_scroller_bounce_set()
 *
 * @ingroup Gengrid
 */
EINA_DEPRECATED EAPI void          elm_gengrid_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce);

/**
 * Get whether bouncing effects are enabled or disabled, for a
 * given gengrid widget, on each axis
 *
 * @param obj The gengrid object
 * @param h_bounce Pointer to a variable where to store the
 * horizontal bouncing flag.
 * @param v_bounce Pointer to a variable where to store the
 * vertical bouncing flag.
 *
 * @deprecated Use elm_scroller_bounce_get() instead.
 *
 * @see elm_scroller_bounce_get()
 *
 * @ingroup Gengrid
 */
EINA_DEPRECATED EAPI void          elm_gengrid_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce);

/**
 * @brief Get gengrid current page number.
 *
 * @param obj The gengrid object
 * @param h_pagenumber The horizontal page number
 * @param v_pagenumber The vertical page number
 *
 * The page number starts from 0. 0 is the first page.
 * Current page means the page which meet the top-left of the viewport.
 * If there are two or more pages in the viewport, it returns the number of page
 * which meet the top-left of the viewport.
 *
 * @deprecated Use elm_scroller_current_page_set() instead.
 *
 * @see elm_scroller_current_page_set()
 *
 * @see elm_gengrid_last_page_get()
 * @see elm_gengrid_page_show()
 * @see elm_gengrid_page_bring_in()
 */
EINA_DEPRECATED EAPI void          elm_gengrid_current_page_get(const Evas_Object *obj, int *h_pagenumber, int *v_pagenumber);

/**
 * @brief Get gengrid last page number.
 *
 * @param obj The gengrid object
 * @param h_pagenumber The horizontal page number
 * @param v_pagenumber The vertical page number
 *
 * The page number starts from 0. 0 is the first page.
 * This returns the last page number among the pages.
 *
 * @deprecated Use elm_scroller_last_page_set() instead.
 *
 * @see elm_scroller_last_page_set()
 *
 * @see elm_gengrid_current_page_get()
 * @see elm_gengrid_page_show()
 * @see elm_gengrid_page_bring_in()
 */
EINA_DEPRECATED EAPI void          elm_gengrid_last_page_get(const Evas_Object *obj, int *h_pagenumber, int *v_pagenumber);

/**
 * Show a specific virtual region within the gengrid content object by page number.
 *
 * @param obj The gengrid object
 * @param h_pagenumber The horizontal page number
 * @param v_pagenumber The vertical page number
 *
 * 0, 0 of the indicated page is located at the top-left of the viewport.
 * This will slide to the page with animation.
 *
 * Example of usage:
 *
 * @code
 * sc = elm_gengrid_add(win);
 * elm_gengrid_content_set(sc, content);
 * elm_gengrid_page_relative_set(sc, 1, 0);
 * elm_gengrid_last_page_get(sc, &h_page, &v_page);
 * elm_gengrid_page_bring_in(sc, h_page, v_page);
 * @endcode
 *
 * @deprecated Use elm_scroller_page_bring_in() instead.
 *
 * @see elm_scroller_page_bring_in()
 *
 * @see elm_gengrid_page_show()
 */
EINA_DEPRECATED EAPI void          elm_gengrid_page_bring_in(const Evas_Object *obj, int h_pagenumber, int v_pagenumber);

typedef enum _Elm_Gengrid_Reorder_Type
{
   ELM_GENGRID_REORDER_TYPE_NORMAL,
   ELM_GENGRID_REORDER_TYPE_SWAP
} Elm_Gengrid_Reorder_Type;

/**
 * Enable the gengrid widget mode reordered with keys
 *
 * @param obj The gengrid object
 * @param tween_mode Position mappings for animation
 * @see _Ecore_Map_Pos
 *
 * @see elm_gengrid_reorder_mode_stop()
 * @since 1.10
 *
 * @ingroup Gengrid
 */
EAPI void                        elm_gengrid_reorder_mode_start(Evas_Object *obj, Ecore_Pos_Map tween_mode);

/**
 * Stop the gengrid widget mode reorder.
 *
 * @param obj The gengrid object
 *
 * @see elm_gengrid_reorder_mode_start()
 * @since 1.10
 *
 * @ingroup Gengrid
 */
EAPI void                        elm_gengrid_reorder_mode_stop(Evas_Object *obj);

/**
 * Set the reorder type
 *
 * @param obj The gengrid object
 * @param type Elm_Gengrid_Reorder_Type value
 *
 * @see Elm_Gengrid_Reorder_Type
 * @since 1.11
 *
 * @ingroup Gengrid
 */
EAPI void                        elm_gengrid_reorder_type_set(Evas_Object *obj, Elm_Gengrid_Reorder_Type type);

#include "elm_gengrid.eo.legacy.h"
