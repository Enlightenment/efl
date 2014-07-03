typedef enum
{
   ELM_CTXPOPUP_DIRECTION_DOWN, /**< ctxpopup show appear below clicked area */
   ELM_CTXPOPUP_DIRECTION_RIGHT, /**< ctxpopup show appear to the right of the clicked area */
   ELM_CTXPOPUP_DIRECTION_LEFT, /**< ctxpopup show appear to the left of the clicked area */
   ELM_CTXPOPUP_DIRECTION_UP, /**< ctxpopup show appear above the clicked area */
   ELM_CTXPOPUP_DIRECTION_UNKNOWN, /**< ctxpopup does not determine it's direction yet*/
} Elm_Ctxpopup_Direction; /**< Direction in which to show the popup */

/**
 * Get the item before @p it in a ctxpopup widget's internal list of
 * items.
 *
 * @param it The item to fetch previous from
 * @return The item before the @p it in its parent's list. If there is no
 *         previous item for @p it or there's an error, @c NULL is returned.
 *
 * @see elm_ctxpopup_item_next_get()
 *
 * @since 1.11
 * @ingroup Ctxpopup
 */
EAPI Elm_Object_Item *elm_ctxpopup_item_prev_get(const Elm_Object_Item *it);

/**
 * Get the item after @p it in a ctxpopup widget's
 * internal list of items.
 *
 * @param it The item to fetch next from
 * @return The item after the @p it in its parent's list. If there is no next
 *         item for @p it or there's an error, @c NULL is returned.
 *
 * @see elm_ctxpopup_item_prev_get()
 *
 * @since 1.11
 * @ingroup Ctxpopup
 */
EAPI Elm_Object_Item *elm_ctxpopup_item_next_get(const Elm_Object_Item *it);

