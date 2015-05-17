/**
 * @addtogroup Toolbar
 *
 * @{
 */

typedef struct _Elm_Toolbar_Item_State Elm_Toolbar_Item_State;    /**< State of a Elm_Toolbar_Item. Can be created with elm_toolbar_item_state_add() and removed with elm_toolbar_item_state_del(). */

/**
 * Unset the state of @p it.
 *
 * @param it The toolbar item.
 *
 * The default icon and label from this item will be displayed.
 *
 * @see elm_toolbar_item_state_set() for more details.
 */
EAPI void                         elm_toolbar_item_state_unset(Elm_Object_Item *it);

/**
 * @}
 */
