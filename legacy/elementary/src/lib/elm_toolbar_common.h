/**
 * @enum Elm_Toolbar_Shrink_Mode
 * @typedef Elm_Toolbar_Shrink_Mode
 *
 * Set toolbar's items display behavior, it can be scrollable,
 * show a menu with exceeding items, or simply hide them.
 *
 * @note Default value is #ELM_TOOLBAR_SHRINK_MENU. It reads value
 * from elm config.
 *
 * Values <b> don't </b> work as bitmask, only one can be chosen.
 *
 * @see elm_toolbar_shrink_mode_set()
 * @see elm_toolbar_shrink_mode_get()
 *
 * @ingroup Toolbar
 */
typedef enum
{
   ELM_TOOLBAR_SHRINK_NONE, /**< Set toolbar minimum size to fit all the items. */
   ELM_TOOLBAR_SHRINK_HIDE, /**< Hide exceeding items. */
   ELM_TOOLBAR_SHRINK_SCROLL, /**< Allow accessing exceeding items through a scroller. */
   ELM_TOOLBAR_SHRINK_MENU, /**< Inserts a button to pop up a menu with exceeding items. */
   ELM_TOOLBAR_SHRINK_EXPAND, /**< Expand all items according the size of the toolbar. */
   ELM_TOOLBAR_SHRINK_LAST /**< Indicates error if returned by elm_toolbar_shrink_mode_get() */
} Elm_Toolbar_Shrink_Mode;

/**
 * Defines where to position the item in the toolbar.
 *
 * @ingroup Toolbar
 */
typedef enum
{
   ELM_TOOLBAR_ITEM_SCROLLTO_NONE = 0,   /**< no scrollto */
   ELM_TOOLBAR_ITEM_SCROLLTO_IN = (1 << 0),   /**< to the nearest viewport */
   ELM_TOOLBAR_ITEM_SCROLLTO_FIRST = (1 << 1),   /**< to the first of viewport */
   ELM_TOOLBAR_ITEM_SCROLLTO_MIDDLE = (1 << 2),   /**< to the middle of viewport */
   ELM_TOOLBAR_ITEM_SCROLLTO_LAST = (1 << 3)   /**< to the last of viewport */
} Elm_Toolbar_Item_Scrollto_Type;

typedef struct _Elm_Toolbar_Item_State Elm_Toolbar_Item_State;    /**< State of a Elm_Toolbar_Item. Can be created with elm_toolbar_item_state_add() and removed with elm_toolbar_item_state_del(). */

/**
 * Unset the state of @p it.
 *
 * @param it The toolbar item.
 *
 * The default icon and label from this item will be displayed.
 *
 * @see elm_toolbar_item_state_set() for more details.
 *
 * @ingroup Toolbar
 */
EAPI void                         elm_toolbar_item_state_unset(Elm_Object_Item *it);
