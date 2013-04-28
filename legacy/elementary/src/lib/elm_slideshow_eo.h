#define ELM_OBJ_SLIDESHOW_CLASS elm_obj_slideshow_class_get()

const Eo_Class *elm_obj_slideshow_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_SLIDESHOW_BASE_ID;

enum
{
   ELM_OBJ_SLIDESHOW_SUB_ID_ITEM_ADD,
   ELM_OBJ_SLIDESHOW_SUB_ID_ITEM_SORTED_INSERT,
   ELM_OBJ_SLIDESHOW_SUB_ID_NEXT,
   ELM_OBJ_SLIDESHOW_SUB_ID_PREVIOUS,
   ELM_OBJ_SLIDESHOW_SUB_ID_TRANSITIONS_GET,
   ELM_OBJ_SLIDESHOW_SUB_ID_LAYOUTS_GET,
   ELM_OBJ_SLIDESHOW_SUB_ID_TRANSITION_SET,
   ELM_OBJ_SLIDESHOW_SUB_ID_TRANSITION_GET,
   ELM_OBJ_SLIDESHOW_SUB_ID_TIMEOUT_SET,
   ELM_OBJ_SLIDESHOW_SUB_ID_TIMEOUT_GET,
   ELM_OBJ_SLIDESHOW_SUB_ID_LOOP_SET,
   ELM_OBJ_SLIDESHOW_SUB_ID_LAYOUT_GET,
   ELM_OBJ_SLIDESHOW_SUB_ID_LAYOUT_SET,
   ELM_OBJ_SLIDESHOW_SUB_ID_LOOP_GET,
   ELM_OBJ_SLIDESHOW_SUB_ID_CLEAR,
   ELM_OBJ_SLIDESHOW_SUB_ID_ITEMS_GET,
   ELM_OBJ_SLIDESHOW_SUB_ID_ITEM_CURRENT_GET,
   ELM_OBJ_SLIDESHOW_SUB_ID_CACHE_BEFORE_GET,
   ELM_OBJ_SLIDESHOW_SUB_ID_CACHE_BEFORE_SET,
   ELM_OBJ_SLIDESHOW_SUB_ID_CACHE_AFTER_GET,
   ELM_OBJ_SLIDESHOW_SUB_ID_CACHE_AFTER_SET,
   ELM_OBJ_SLIDESHOW_SUB_ID_ITEM_NTH_GET,
   ELM_OBJ_SLIDESHOW_SUB_ID_COUNT_GET,
   ELM_OBJ_SLIDESHOW_SUB_ID_LAST
};

#define ELM_OBJ_SLIDESHOW_ID(sub_id) (ELM_OBJ_SLIDESHOW_BASE_ID + sub_id)


/**
 * @def elm_obj_slideshow_item_add
 * @since 1.8
 *
 * Add (append) a new item in a given slideshow widget.
 *
 * @param[in] itc
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_slideshow_item_add
 *
 * @ingroup Slideshow
 */
#define elm_obj_slideshow_item_add(itc, data, ret) ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_ITEM_ADD), EO_TYPECHECK(const Elm_Slideshow_Item_Class *, itc), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_slideshow_item_sorted_insert
 * @since 1.8
 *
 * Insert a new item into the given slideshow widget, using the func
 * function to sort items (by item handles).
 *
 * @param[in] itc
 * @param[in] data
 * @param[in] func
 * @param[out] ret
 *
 * @see elm_slideshow_item_sorted_insert
 *
 * @ingroup Slideshow
 */
#define elm_obj_slideshow_item_sorted_insert(itc, data, func, ret) ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_ITEM_SORTED_INSERT), EO_TYPECHECK(const Elm_Slideshow_Item_Class *, itc), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Eina_Compare_Cb, func), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_slideshow_next
 * @since 1.8
 *
 * Slide to the next item, in a given slideshow widget
 *
 *
 * @see elm_slideshow_next
 *
 * @ingroup Slideshow
 */
#define elm_obj_slideshow_next() ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_NEXT)

/**
 * @def elm_obj_slideshow_previous
 * @since 1.8
 *
 * Slide to the previous item, in a given slideshow widget
 *
 *
 * @see elm_slideshow_previous
 *
 * @ingroup Slideshow
 */
#define elm_obj_slideshow_previous() ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_PREVIOUS)

/**
 * @def elm_obj_slideshow_transitions_get
 * @since 1.8
 *
 * Returns the list of sliding transition/effect names available, for a
 * given slideshow widget.
 *
 * @param[out] ret
 *
 * @see elm_slideshow_transitions_get
 *
 * @ingroup Slideshow
 */
#define elm_obj_slideshow_transitions_get(ret) ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_TRANSITIONS_GET), EO_TYPECHECK(const Eina_List **, ret)

/**
 * @def elm_obj_slideshow_layouts_get
 * @since 1.8
 *
 * Returns the list of layout names available, for a given
 * slideshow widget.
 *
 * @param[out] ret
 *
 * @see elm_slideshow_layouts_get
 *
 * @ingroup Slideshow
 */
#define elm_obj_slideshow_layouts_get(ret) ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_LAYOUTS_GET), EO_TYPECHECK(const Eina_List **, ret)

/**
 * @def elm_obj_slideshow_transition_set
 * @since 1.8
 *
 * Set the current slide transition/effect in use for a given
 * slideshow widget.
 *
 * @param[in] transition
 *
 * @see elm_slideshow_transition_set
 *
 * @ingroup Slideshow
 */
#define elm_obj_slideshow_transition_set(transition) ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_TRANSITION_SET), EO_TYPECHECK(const char *, transition)

/**
 * @def elm_obj_slideshow_transition_get
 * @since 1.8
 *
 * Get the current slide transition/effect in use for a given
 * slideshow widget.
 *
 * @param[out] ret
 *
 * @see elm_slideshow_transition_get
 *
 * @ingroup Slideshow
 */
#define elm_obj_slideshow_transition_get(ret) ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_TRANSITION_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_slideshow_timeout_set
 * @since 1.8
 *
 * Set the interval between each image transition on a given
 * slideshow widget.
 *
 * @param[in] timeout
 *
 * @see elm_slideshow_timeout_set
 *
 * @ingroup Slideshow
 */
#define elm_obj_slideshow_timeout_set(timeout) ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_TIMEOUT_SET), EO_TYPECHECK(double, timeout)

/**
 * @def elm_obj_slideshow_timeout_get
 * @since 1.8
 *
 * Get the interval set for image transitions on a given slideshow
 * widget.
 *
 * @param[out] ret
 *
 * @see elm_slideshow_timeout_get
 *
 * @ingroup Slideshow
 */
#define elm_obj_slideshow_timeout_get(ret) ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_TIMEOUT_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_slideshow_loop_set
 * @since 1.8
 *
 * Set if, after a slideshow is started, for a given slideshow
 * widget.
 *
 * @param[in] loop
 *
 * @see elm_slideshow_loop_set
 *
 * @ingroup Slideshow
 */
#define elm_obj_slideshow_loop_set(loop) ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_LOOP_SET), EO_TYPECHECK(Eina_Bool, loop)

/**
 * @def elm_obj_slideshow_layout_get
 * @since 1.8
 *
 * Get the current slide layout in use for a given slideshow widget
 *
 * @param[out] ret
 *
 * @see elm_slideshow_layout_get
 *
 * @ingroup Slideshow
 */
#define elm_obj_slideshow_layout_get(ret) ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_LAYOUT_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_slideshow_layout_set
 * @since 1.8
 *
 * Set the current slide layout in use for a given slideshow widget
 *
 * @param[in] layout
 *
 * @see elm_slideshow_layout_set
 *
 * @ingroup Slideshow
 */
#define elm_obj_slideshow_layout_set(layout) ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_LAYOUT_SET), EO_TYPECHECK(const char *, layout)

/**
 * @def elm_obj_slideshow_loop_get
 * @since 1.8
 *
 * Get if, after a slideshow is started, for a given slideshow
 * widget.
 *
 * @param[out] ret
 *
 * @see elm_slideshow_loop_get
 *
 * @ingroup Slideshow
 */
#define elm_obj_slideshow_loop_get(ret) ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_LOOP_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_slideshow_clear
 * @since 1.8
 *
 * Remove all items from a given slideshow widget
 *
 *
 * @see elm_slideshow_clear
 *
 * @ingroup Slideshow
 */
#define elm_obj_slideshow_clear() ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_CLEAR)

/**
 * @def elm_obj_slideshow_items_get
 * @since 1.8
 *
 * Get the internal list of items in a given slideshow widget.
 *
 * @param[out] ret
 *
 * @see elm_slideshow_items_get
 *
 * @ingroup Slideshow
 */
#define elm_obj_slideshow_items_get(ret) ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_ITEMS_GET), EO_TYPECHECK(const Eina_List **, ret)

/**
 * @def elm_obj_slideshow_item_current_get
 * @since 1.8
 *
 * Returns the currently displayed item, in a given slideshow widget
 *
 * @param[out] ret
 *
 * @see elm_slideshow_item_current_get
 *
 * @ingroup Slideshow
 */
#define elm_obj_slideshow_item_current_get(ret) ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_ITEM_CURRENT_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_slideshow_cache_before_get
 * @since 1.8
 *
 * Retrieve the number of items to cache, on a given slideshow widget,
 *
 * @param[out] ret
 *
 * @see elm_slideshow_cache_before_get
 *
 * @ingroup Slideshow
 */
#define elm_obj_slideshow_cache_before_get(ret) ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_CACHE_BEFORE_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_slideshow_cache_before_set
 * @since 1.8
 *
 * Set the number of items to cache, on a given slideshow widget,
 *
 * @param[in] count
 *
 * @see elm_slideshow_cache_before_set
 *
 * @ingroup Slideshow
 */
#define elm_obj_slideshow_cache_before_set(count) ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_CACHE_BEFORE_SET), EO_TYPECHECK(int, count)

/**
 * @def elm_obj_slideshow_cache_after_get
 * @since 1.8
 *
 * Retrieve the number of items to cache, on a given slideshow widget,
 * <b>after the current item</b>
 *
 * @param[out] ret
 *
 * @see elm_slideshow_cache_after_get
 *
 * @ingroup Slideshow
 */
#define elm_obj_slideshow_cache_after_get(ret) ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_CACHE_AFTER_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_slideshow_cache_after_set
 * @since 1.8
 *
 * Set the number of items to cache, on a given slideshow widget,
 * <b>after the current item</b>
 *
 * @param[in] count
 *
 * @see elm_slideshow_cache_after_set
 *
 * @ingroup Slideshow
 */
#define elm_obj_slideshow_cache_after_set(count) ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_CACHE_AFTER_SET), EO_TYPECHECK(int, count)

/**
 * @def elm_obj_slideshow_item_nth_get
 * @since 1.8
 *
 * Get the the item, in a given slideshow widget, placed at
 * position @p nth, in its internal items list
 *
 * @param[in] nth
 * @param[out] ret
 *
 * @see elm_slideshow_item_nth_get
 *
 * @ingroup Slideshow
 */
#define elm_obj_slideshow_item_nth_get(nth, ret) ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_ITEM_NTH_GET), EO_TYPECHECK(unsigned int, nth), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_slideshow_count_get
 * @since 1.8
 *
 * Get the number of items stored in a given slideshow widget
 *
 * @param[out] ret
 *
 * @see elm_slideshow_count_get
 *
 * @ingroup Slideshow
 */
#define elm_obj_slideshow_count_get(ret) ELM_OBJ_SLIDESHOW_ID(ELM_OBJ_SLIDESHOW_SUB_ID_COUNT_GET), EO_TYPECHECK(unsigned int *, ret)
