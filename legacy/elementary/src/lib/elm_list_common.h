/**
 * @addtogroup List
 *
 * @{
 */

/**
 * Set list's resizing behavior, transverse axis scrolling and items
 * cropping. See each mode's description for more details.
 *
 * @note Default value is #ELM_LIST_SCROLL.
 *
 * Values here @b don't work as bitmasks -- only one can be chosen at
 * a time.
 *
 * @see elm_list_mode_set()
 * @see elm_list_mode_get()
 */
typedef enum
{
   ELM_LIST_COMPRESS = 0, /**< The list @b won't set any of its size hints to inform how a possible container should resize it. Then, if it's not created as a "resize object", it might end with zeroed dimensions. The list will respect the container's geometry and, if any of its items won't fit into its @b transverse axis, one won't be able to scroll it in that direction. */
   ELM_LIST_SCROLL, /**< Default value. This is the same as #ELM_LIST_COMPRESS, with the exception that if any of its items won't fit into its transverse axis, one @b will be able to scroll it in that direction. */
   ELM_LIST_LIMIT, /**< Sets a minimum size hint on the list object, so that containers may respect it (and resize itself to fit the child properly). More specifically, a @b minimum size hint will be set for its @b transverse axis, so that the @b largest item in that direction fits well. This is naturally bound by the list object's maximum size hints, set externally. */
   ELM_LIST_EXPAND, /**< Besides setting a minimum size on the transverse axis, just like on #ELM_LIST_LIMIT, the list will set a minimum size on the @b longitudinal axis, trying to reserve space to all its children to be visible at a time. . This is naturally bound by the list object's maximum size hints, set externally. */
   ELM_LIST_LAST /**< Indicates error if returned by elm_list_mode_get() */
} Elm_List_Mode;

/**
 * @}
 */
