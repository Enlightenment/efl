#define ELM_OBJ_FLIPSELECTOR_CLASS elm_obj_flipselector_class_get()

const Eo_Class *elm_obj_flipselector_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_FLIPSELECTOR_BASE_ID;

typedef void (*flipselector_func_type)(void *, Evas_Object *, void *);

enum
{
   ELM_OBJ_FLIPSELECTOR_SUB_ID_FLIP_NEXT,
   ELM_OBJ_FLIPSELECTOR_SUB_ID_FLIP_PREV,
   ELM_OBJ_FLIPSELECTOR_SUB_ID_ITEM_APPEND,
   ELM_OBJ_FLIPSELECTOR_SUB_ID_ITEM_PREPEND,
   ELM_OBJ_FLIPSELECTOR_SUB_ID_ITEMS_GET,
   ELM_OBJ_FLIPSELECTOR_SUB_ID_FIRST_ITEM_GET,
   ELM_OBJ_FLIPSELECTOR_SUB_ID_LAST_ITEM_GET,
   ELM_OBJ_FLIPSELECTOR_SUB_ID_SELECTED_ITEM_GET,
   ELM_OBJ_FLIPSELECTOR_SUB_ID_FIRST_INTERVAL_SET,
   ELM_OBJ_FLIPSELECTOR_SUB_ID_FIRST_INTERVAL_GET,
   ELM_OBJ_FLIPSELECTOR_SUB_ID_LAST
};

#define ELM_OBJ_FLIPSELECTOR_ID(sub_id) (ELM_OBJ_FLIPSELECTOR_BASE_ID + sub_id)


/**
 * @def elm_obj_flipselector_flip_next
 * @since 1.8
 *
 * Programmatically select the next item of a flip selector widget
 *
 *
 * @see elm_flipselector_flip_next
 *
 * @ingroup Flipselector
 */
#define elm_obj_flipselector_flip_next() ELM_OBJ_FLIPSELECTOR_ID(ELM_OBJ_FLIPSELECTOR_SUB_ID_FLIP_NEXT)

/**
 * @def elm_obj_flipselector_flip_prev
 * @since 1.8
 *
 * Programmatically select the previous item of a flip selector
 *
 *
 * @see elm_flipselector_flip_prev
 *
 * @ingroup Flipselector
 */
#define elm_obj_flipselector_flip_prev() ELM_OBJ_FLIPSELECTOR_ID(ELM_OBJ_FLIPSELECTOR_SUB_ID_FLIP_PREV)


/**
 * @def elm_obj_flipselector_item_append
 * @since 1.8
 *
 * Append a (text) item to a flip selector widget
 *
 * @param[in] label
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_flipselector_item_append
 *
 * @ingroup Flipselector
 */
#define elm_obj_flipselector_item_append(label, func, data, ret) ELM_OBJ_FLIPSELECTOR_ID(ELM_OBJ_FLIPSELECTOR_SUB_ID_ITEM_APPEND), EO_TYPECHECK(const char *, label), EO_TYPECHECK(flipselector_func_type, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Flipselector_Item **, ret)

/**
 * @def elm_obj_flipselector_item_prepend
 * @since 1.8
 *
 * Prepend a (text) item to a flip selector widget
 *
 * @param[in] label
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_flipselector_item_prepend
 *
 * @ingroup Flipselector
 */
#define elm_obj_flipselector_item_prepend(label, func, data, ret) ELM_OBJ_FLIPSELECTOR_ID(ELM_OBJ_FLIPSELECTOR_SUB_ID_ITEM_PREPEND), EO_TYPECHECK(const char *, label), EO_TYPECHECK(flipselector_func_type, func), EO_TYPECHECK(void *, data), EO_TYPECHECK(Elm_Flipselector_Item **, ret)


/**
 * @def elm_obj_flipselector_items_get
 * @since 1.8
 *
 * Get the internal list of items in a given flip selector widget.
 *
 * @param[out] ret
 *
 * @see elm_flipselector_items_get
 *
 * @ingroup Flipselector
 */
#define elm_obj_flipselector_items_get(ret) ELM_OBJ_FLIPSELECTOR_ID(ELM_OBJ_FLIPSELECTOR_SUB_ID_ITEMS_GET), EO_TYPECHECK(const Eina_List **, ret)

/**
 * @def elm_obj_flipselector_first_item_get
 * @since 1.8
 *
 * Get the first item in the given flip selector widget's list of
 *
 * @param[out] ret
 *
 * @see elm_flipselector_first_item_get
 *
 * @ingroup Flipselector
 */
#define elm_obj_flipselector_first_item_get(ret) ELM_OBJ_FLIPSELECTOR_ID(ELM_OBJ_FLIPSELECTOR_SUB_ID_FIRST_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_flipselector_last_item_get
 * @since 1.8
 *
 * Get the last item in the given flip selector widget's list of
 *
 * @param[out] ret
 *
 * @see elm_flipselector_last_item_get
 *
 * @ingroup Flipselector
 */
#define elm_obj_flipselector_last_item_get(ret) ELM_OBJ_FLIPSELECTOR_ID(ELM_OBJ_FLIPSELECTOR_SUB_ID_LAST_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_flipselector_selected_item_get
 * @since 1.8
 *
 * Get the currently selected item in a flip selector widget.
 *
 * @param[out] ret
 *
 * @see elm_flipselector_selected_item_get
 *
 * @ingroup Flipselector
 */
#define elm_obj_flipselector_selected_item_get(ret) ELM_OBJ_FLIPSELECTOR_ID(ELM_OBJ_FLIPSELECTOR_SUB_ID_SELECTED_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_flipselector_first_interval_set
 * @since 1.8
 *
 * Set the interval on time updates for a user mouse button hold
 *
 * @param[in] interval
 *
 * @see elm_flipselector_first_interval_set
 *
 * @ingroup Flipselector
 */
#define elm_obj_flipselector_first_interval_set(interval) ELM_OBJ_FLIPSELECTOR_ID(ELM_OBJ_FLIPSELECTOR_SUB_ID_FIRST_INTERVAL_SET), EO_TYPECHECK(double, interval)

/**
 * @def elm_obj_flipselector_first_interval_get
 * @since 1.8
 *
 * Get the interval on time updates for an user mouse button hold
 *
 * @param[out] ret
 *
 * @see elm_flipselector_first_interval_get
 *
 * @ingroup Flipselector
 */
#define elm_obj_flipselector_first_interval_get(ret) ELM_OBJ_FLIPSELECTOR_ID(ELM_OBJ_FLIPSELECTOR_SUB_ID_FIRST_INTERVAL_GET), EO_TYPECHECK(double *, ret)
