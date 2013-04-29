/**
 * @ingroup Prefs
 *
 * @{
 */
#define ELM_OBJ_PREFS_CLASS elm_obj_prefs_class_get()

const Eo_Class *elm_obj_prefs_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_PREFS_BASE_ID;

enum
{
   ELM_OBJ_PREFS_SUB_ID_FILE_SET,
   ELM_OBJ_PREFS_SUB_ID_FILE_GET,
   ELM_OBJ_PREFS_SUB_ID_DATA_SET,
   ELM_OBJ_PREFS_SUB_ID_DATA_GET,
   ELM_OBJ_PREFS_SUB_ID_AUTOSAVE_SET,
   ELM_OBJ_PREFS_SUB_ID_AUTOSAVE_GET,
   ELM_OBJ_PREFS_SUB_ID_RESET,
   ELM_OBJ_PREFS_SUB_ID_ITEM_VALUE_SET,
   ELM_OBJ_PREFS_SUB_ID_ITEM_VALUE_GET,
   ELM_OBJ_PREFS_SUB_ID_ITEM_OBJECT_GET,
   ELM_OBJ_PREFS_SUB_ID_ITEM_VISIBLE_SET,
   ELM_OBJ_PREFS_SUB_ID_ITEM_VISIBLE_GET,
   ELM_OBJ_PREFS_SUB_ID_ITEM_DISABLED_SET,
   ELM_OBJ_PREFS_SUB_ID_ITEM_DISABLED_GET,
   ELM_OBJ_PREFS_SUB_ID_ITEM_EDITABLE_SET,
   ELM_OBJ_PREFS_SUB_ID_ITEM_EDITABLE_GET,
   ELM_OBJ_PREFS_SUB_ID_ITEM_SWALLOW,
   ELM_OBJ_PREFS_SUB_ID_ITEM_UNSWALLOW,
   ELM_OBJ_PREFS_SUB_ID_LAST
};

#define ELM_OBJ_PREFS_ID(sub_id) (ELM_OBJ_PREFS_BASE_ID + sub_id)

/**
 * @def elm_obj_prefs_file_set
 *
 * Set file and page to populate a given prefs widget's interface.
 *
 * @param[in] file
 * @param[in] page
 * @param[out] ret
 *
 * @see elm_prefs_file_set()
 *
 * @since 1.8
 */
#define elm_obj_prefs_file_set(file, page, ret)          \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_FILE_SET), \
        EO_TYPECHECK(const char *, file),                \
        EO_TYPECHECK(const char *, page),                \
        EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_prefs_file_get
 *
 * Retrieve file and page bound to a given prefs widget.
 *
 * @param[out] file
 * @param[out] page
 * @param[out] ret
 *
 * @see elm_prefs_file_get()
 *
 * @since 1.8
 */
#define elm_obj_prefs_file_get(file, page, ret)          \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_FILE_GET), \
        EO_TYPECHECK(const char **, file),               \
        EO_TYPECHECK(const char **, page),               \
        EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_prefs_data_set
 *
 * Set user data for a given prefs widget
 *
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_prefs_data_set()
 *
 * @since 1.8
 */
#define elm_obj_prefs_data_set(data, ret)                \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_DATA_SET), \
        EO_TYPECHECK(Elm_Prefs_Data *, data),            \
        EO_TYPECHECK(Eina_Bool *, ret)
//FIXME: prefs_data == eobject?

/**
 * @def elm_obj_prefs_data_get
 *
 * Retrieve user data for a given prefs widget
 *
 * @param[out] ret
 *
 * @see elm_prefs_data_get()
 *
 * @since 1.8
 */
#define elm_obj_prefs_data_get(ret)                      \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_DATA_GET), \
        EO_TYPECHECK(Elm_Prefs_Data **, ret)

/**
 * @def elm_obj_prefs_autosave_set
 *
 * Set whether a given prefs widget should save its values back (on
 * the user data file, if set) automatically on every UI element
 * changes.
 *
 * @param[in] autosave
 *
 * @see elm_prefs_autosave_get()
 *
 * @since 1.8
 */
#define elm_obj_prefs_autosave_set(autosave)                 \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_AUTOSAVE_SET), \
        EO_TYPECHECK(Eina_Bool, autosave)

/**
 * @def elm_obj_prefs_autosave_get
 *
 * Get whether a given prefs widget is saving its values back
 * automatically on changes.
 *
 * @param[out] ret
 *
 * @see elm_prefs_autosave_set()
 *
 * @since 1.8
 */
#define elm_obj_prefs_autosave_get(ret)                      \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_AUTOSAVE_GET), \
        EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_prefs_reset
 *
 * Reset the values of a given prefs widget to a previous state.
 *
 * @param[in] mode
 *
 * @since 1.8
 */
#define elm_obj_prefs_reset(mode)                     \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_RESET), \
        EO_TYPECHECK(Elm_Prefs_Reset_Mode, mode)

/**
 * @def elm_obj_prefs_item_value_set
 *
 * Set the value on a given prefs widget's item.
 *
 * @param[in] name
 * @param[in] value
 * @param[out] ret
 *
 * @see elm_prefs_item_value_get()
 *
 * @since 1.8
 */
#define elm_obj_prefs_item_value_set(name, value, ret)         \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_ITEM_VALUE_SET), \
        EO_TYPECHECK(const char *, name),                      \
        EO_TYPECHECK(const Eina_Value *, value),               \
        EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_prefs_item_value_get
 *
 * Retrieve the value of a given prefs widget's item.
 *
 * @param[in] name
 * @param[out] value
 * @param[out] ret
 *
 * @see elm_prefs_item_value_set()
 *
 * @since 1.8
 */
#define elm_obj_prefs_item_value_get(name, value, ret)         \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_ITEM_VALUE_GET), \
        EO_TYPECHECK(const char *, name),                      \
        EO_TYPECHECK(Eina_Value *, value),                     \
        EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_prefs_item_object_get
 *
 * Retrieve the Elementary widget bound to a given prefs widget's
 * item.
 *
 * @param[in] name
 * @param[out] ret
 *
 * @see elm_prefs_item_value_get()
 *
 * @since 1.8
 */
#define elm_obj_prefs_item_object_get(name, ret)                \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_ITEM_OBJECT_GET), \
        EO_TYPECHECK(const char *, name),                       \
        EO_TYPECHECK(const Evas_Object **, ret) //FIXME: return EObject?

/**
 * @def elm_obj_prefs_item_visible_set
 *
 * Set whether the widget bound to given prefs widget's item should be
 * visible or not.
 *
 * @param[in] name
 * @param[in] visible
 *
 * @see elm_prefs_item_visible_set()
 *
 * @since 1.8
 */
#define elm_obj_prefs_item_visible_set(name, visible)            \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_ITEM_VISIBLE_SET), \
        EO_TYPECHECK(const char *, name),                        \
        EO_TYPECHECK(Eina_Bool, visible)

/**
 * @def elm_obj_prefs_item_visible_get
 *
 * Retrieve whether the widget bound to a given prefs widget's item is
 * visible or not.
 *
 * @param[in] name
 * @param[out] ret
 *
 * @see elm_prefs_item_visible_get()
 *
 * @since 1.8
 */
#define elm_obj_prefs_item_visible_get(name, ret)                \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_ITEM_VISIBLE_GET), \
        EO_TYPECHECK(const char *,name),                         \
        EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_prefs_item_disabled_set
 *
 * Set whether the widget bound to a given prefs widget's item is
 * disabled or not.
 *
 * @param[in] name
 * @param[in] disabled
 *
 * @see elm_prefs_item_disabled_set()
 *
 * @since 1.8
 */
#define elm_obj_prefs_item_disabled_set(name, disabled)           \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_ITEM_DISABLED_SET), \
        EO_TYPECHECK(const char *, name),                         \
        EO_TYPECHECK(Eina_Bool, disabled)

/**
 * @def elm_obj_prefs_item_disabled_get
 *
 * Retrieve whether the widget bound to a given prefs widget's item is
 * disabled or not.
 *
 * @param[in] name
 * @param[out] ret
 *
 * @see elm_prefs_item_disabled_get()
 *
 * @since 1.8
 */
#define elm_obj_prefs_item_disabled_get(name, ret)                \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_ITEM_DISABLED_GET), \
        EO_TYPECHECK(const char *, name),                         \
        EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_prefs_item_editable_set
 *
 * Set whether the widget bound to a given prefs widget's item is
 * editable or not.
 *
 * @param[in] name
 * @param[in] editable
 *
 * @see elm_prefs_item_editable_set()
 *
 * @since 1.8
 */
#define elm_obj_prefs_item_editable_set(name, editable)           \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_ITEM_EDITABLE_SET), \
        EO_TYPECHECK(const char *, name),                         \
        EO_TYPECHECK(Eina_Bool, editable)

/**
 * @def elm_obj_prefs_item_editable_get
 *
 * Retrieve whether the widget bound to a given prefs widget's item is
 * editable or not.
 *
 * @param[in] name
 * @param[out] ret
 *
 * @see elm_prefs_item_editable_get()
 *
 * @since 1.8
 */
#define elm_obj_prefs_item_editable_get(name, ret)                \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_ITEM_EDITABLE_GET), \
        EO_TYPECHECK(const char *, name),                         \
        EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_prefs_item_swallow
 *
 * "Swallows" an object into a SWALLOW item of a prefs widget.
 *
 * @param[in] name
 * @param[in] child
 * @param[out] ret
 *
 * @see elm_prefs_item_swallow()
 *
 * @since 1.8
 */
#define elm_obj_prefs_item_swallow(name, child, ret)         \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_ITEM_SWALLOW), \
        EO_TYPECHECK(const char *, name),                    \
        EO_TYPECHECK(Evas_Object *, child),                  \
        EO_TYPECHECK(Eina_Bool *, ret)
//FIXME: child == EObject?

/**
 * @def elm_obj_prefs_item_unswallow
 *
 * Unswallow an object from a SWALLOW item of a prefs widget.
 *
 * @param[in] name
 * @param[out] ret
 *
 * @see elm_prefs_item_unswallow()
 *
 * @since 1.8
 */
#define elm_obj_prefs_item_unswallow(name, ret)                \
        ELM_OBJ_PREFS_ID(ELM_OBJ_PREFS_SUB_ID_ITEM_UNSWALLOW), \
        EO_TYPECHECK(const char *,name),                       \
        EO_TYPECHECK(Evas_Object **, ret)
//FIXME: ret == EObject?
/**
 * @}
 */
