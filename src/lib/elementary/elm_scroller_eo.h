#ifndef _ELM_SCROLLER_EO_H_
#define _ELM_SCROLLER_EO_H_

#ifndef _ELM_SCROLLER_EO_CLASS_TYPE
#define _ELM_SCROLLER_EO_CLASS_TYPE

typedef Eo Elm_Scroller;

#endif

#ifndef _ELM_SCROLLER_EO_TYPES
#define _ELM_SCROLLER_EO_TYPES


#endif
/** Elementary scroller class
 *
 * @ingroup Elm_Scroller
 */
#define ELM_SCROLLER_CLASS elm_scroller_class_get()

EWAPI const Efl_Class *elm_scroller_class_get(void);

/**
 * @brief Set custom theme elements for the scroller
 *
 * @param[in] obj The object.
 * @param[in] klass Klass name
 * @param[in] group Group name
 *
 * @ingroup Elm_Scroller
 */
EOAPI void elm_obj_scroller_custom_widget_base_theme_set(Eo *obj, const char *klass, const char *group);

/**
 * @brief Set the maximum of the movable page at a flicking.
 *
 * The value of maximum movable page should be more than 1.
 *
 * @param[in] obj The object.
 * @param[in] page_limit_h The maximum of the movable horizontal page
 * @param[in] page_limit_v The maximum of the movable vertical page
 *
 * @since 1.8
 *
 * @ingroup Elm_Scroller
 */
EOAPI void elm_obj_scroller_page_scroll_limit_set(const Eo *obj, int page_limit_h, int page_limit_v);

/**
 * @brief Get the maximum of the movable page at a flicking.
 *
 * @param[in] obj The object.
 * @param[in] page_limit_h The maximum of the movable horizontal page
 * @param[in] page_limit_v The maximum of the movable vertical page
 *
 * @since 1.8
 *
 * @ingroup Elm_Scroller
 */
EOAPI void elm_obj_scroller_page_scroll_limit_get(const Eo *obj, int *page_limit_h, int *page_limit_v);

EWAPI extern const Efl_Event_Description _ELM_SCROLLER_EVENT_SCROLL_PAGE_CHANGED;

/** Called when scroll page changed
 *
 * @ingroup Elm_Scroller
 */
#define ELM_SCROLLER_EVENT_SCROLL_PAGE_CHANGED (&(_ELM_SCROLLER_EVENT_SCROLL_PAGE_CHANGED))

EWAPI extern const Efl_Event_Description _ELM_SCROLLER_EVENT_HBAR_UNPRESS;

/** Called when horizontal bar is no longer pressed
 *
 * @ingroup Elm_Scroller
 */
#define ELM_SCROLLER_EVENT_HBAR_UNPRESS (&(_ELM_SCROLLER_EVENT_HBAR_UNPRESS))

EWAPI extern const Efl_Event_Description _ELM_SCROLLER_EVENT_HBAR_PRESS;

/** Called when horizontal bar is pressed
 *
 * @ingroup Elm_Scroller
 */
#define ELM_SCROLLER_EVENT_HBAR_PRESS (&(_ELM_SCROLLER_EVENT_HBAR_PRESS))

EWAPI extern const Efl_Event_Description _ELM_SCROLLER_EVENT_HBAR_DRAG;

/** Called when horizontal bar is dragged
 *
 * @ingroup Elm_Scroller
 */
#define ELM_SCROLLER_EVENT_HBAR_DRAG (&(_ELM_SCROLLER_EVENT_HBAR_DRAG))

EWAPI extern const Efl_Event_Description _ELM_SCROLLER_EVENT_VBAR_UNPRESS;

/** Called when vertical bar is no longer pressed
 *
 * @ingroup Elm_Scroller
 */
#define ELM_SCROLLER_EVENT_VBAR_UNPRESS (&(_ELM_SCROLLER_EVENT_VBAR_UNPRESS))

EWAPI extern const Efl_Event_Description _ELM_SCROLLER_EVENT_VBAR_PRESS;

/** Called when vertical bar is pressed
 *
 * @ingroup Elm_Scroller
 */
#define ELM_SCROLLER_EVENT_VBAR_PRESS (&(_ELM_SCROLLER_EVENT_VBAR_PRESS))

EWAPI extern const Efl_Event_Description _ELM_SCROLLER_EVENT_VBAR_DRAG;

/** Called when vertical bar is dragged
 *
 * @ingroup Elm_Scroller
 */
#define ELM_SCROLLER_EVENT_VBAR_DRAG (&(_ELM_SCROLLER_EVENT_VBAR_DRAG))

EWAPI extern const Efl_Event_Description _ELM_SCROLLER_EVENT_SCROLL_LEFT;

/** Called when scrolling to left
 *
 * @ingroup Elm_Scroller
 */
#define ELM_SCROLLER_EVENT_SCROLL_LEFT (&(_ELM_SCROLLER_EVENT_SCROLL_LEFT))

EWAPI extern const Efl_Event_Description _ELM_SCROLLER_EVENT_SCROLL_RIGHT;

/** Called when scrolling to right
 *
 * @ingroup Elm_Scroller
 */
#define ELM_SCROLLER_EVENT_SCROLL_RIGHT (&(_ELM_SCROLLER_EVENT_SCROLL_RIGHT))

EWAPI extern const Efl_Event_Description _ELM_SCROLLER_EVENT_SCROLL_UP;

/** Called scrolled upwards
 *
 * @ingroup Elm_Scroller
 */
#define ELM_SCROLLER_EVENT_SCROLL_UP (&(_ELM_SCROLLER_EVENT_SCROLL_UP))

EWAPI extern const Efl_Event_Description _ELM_SCROLLER_EVENT_SCROLL_DOWN;

/** Called when scrolled downwards
 *
 * @ingroup Elm_Scroller
 */
#define ELM_SCROLLER_EVENT_SCROLL_DOWN (&(_ELM_SCROLLER_EVENT_SCROLL_DOWN))

EWAPI extern const Efl_Event_Description _ELM_SCROLLER_EVENT_EDGE_LEFT;

/** Called when hitting the left edge
 *
 * @ingroup Elm_Scroller
 */
#define ELM_SCROLLER_EVENT_EDGE_LEFT (&(_ELM_SCROLLER_EVENT_EDGE_LEFT))

EWAPI extern const Efl_Event_Description _ELM_SCROLLER_EVENT_EDGE_RIGHT;

/** Called when hitting the right edge
 *
 * @ingroup Elm_Scroller
 */
#define ELM_SCROLLER_EVENT_EDGE_RIGHT (&(_ELM_SCROLLER_EVENT_EDGE_RIGHT))

EWAPI extern const Efl_Event_Description _ELM_SCROLLER_EVENT_EDGE_TOP;

/** Called when hitting the top edge
 *
 * @ingroup Elm_Scroller
 */
#define ELM_SCROLLER_EVENT_EDGE_TOP (&(_ELM_SCROLLER_EVENT_EDGE_TOP))

EWAPI extern const Efl_Event_Description _ELM_SCROLLER_EVENT_EDGE_BOTTOM;

/** Called when hitting the bottom edge
 *
 * @ingroup Elm_Scroller
 */
#define ELM_SCROLLER_EVENT_EDGE_BOTTOM (&(_ELM_SCROLLER_EVENT_EDGE_BOTTOM))

#endif
