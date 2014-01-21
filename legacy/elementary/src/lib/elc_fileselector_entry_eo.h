#define ELM_OBJ_FILESELECTOR_ENTRY_CLASS elm_obj_fileselector_entry_class_get()

const Eo_Class *elm_obj_fileselector_entry_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_FILESELECTOR_ENTRY_BASE_ID;

enum
{
   ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_WINDOW_TITLE_SET,
   ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_WINDOW_TITLE_GET,
   ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_WINDOW_SIZE_SET,
   ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_WINDOW_SIZE_GET,
   ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_INWIN_MODE_SET,
   ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_INWIN_MODE_GET,
   ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_LAST
};

#define ELM_OBJ_FILESELECTOR_ENTRY_ID(sub_id) (ELM_OBJ_FILESELECTOR_ENTRY_BASE_ID + sub_id)

/**
 * @def elm_obj_fileselector_entry_window_title_set
 * @since 1.8
 *
 * Set the title for a given file selector entry widget's window
 *
 * @param[in] title
 *
 * @see elm_fileselector_entry_window_title_set
 *
 * @ingroup File_Selector_Entry
 */
#define elm_obj_fileselector_entry_window_title_set(title) ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_WINDOW_TITLE_SET), EO_TYPECHECK(const char *, title)

/**
 * @def elm_obj_fileselector_entry_window_title_get
 * @since 1.8
 *
 * Get the title set for a given file selector entry widget's
 * window
 *
 * @param[out] ret
 *
 * @see elm_fileselector_entry_window_title_get
 *
 * @ingroup File_Selector_Entry
 */
#define elm_obj_fileselector_entry_window_title_get(ret) ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_WINDOW_TITLE_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_fileselector_entry_window_size_set
 * @since 1.8
 *
 * Set the size of a given file selector entry widget's window,
 * holding the file selector itself.
 *
 * @param[in] width
 * @param[in] height
 *
 * @see elm_fileselector_entry_window_size_set
 *
 * @ingroup File_Selector_Entry
 */
#define elm_obj_fileselector_entry_window_size_set(width, height) ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_WINDOW_SIZE_SET), EO_TYPECHECK(Evas_Coord, width), EO_TYPECHECK(Evas_Coord, height)

/**
 * @def elm_obj_fileselector_entry_window_size_get
 * @since 1.8
 *
 * Get the size of a given file selector entry widget's window,
 * holding the file selector itself.
 *
 * @param[out] width
 * @param[out] height
 *
 * @see elm_fileselector_entry_window_size_get
 *
 * @ingroup File_Selector_Entry
 */
#define elm_obj_fileselector_entry_window_size_get(width, height) ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_WINDOW_SIZE_GET), EO_TYPECHECK(Evas_Coord *, width), EO_TYPECHECK(Evas_Coord *, height)

/**
 * @def elm_obj_fileselector_entry_inwin_mode_set
 * @since 1.8
 *
 * Set whether a given file selector entry widget's internal file
 *
 * @param[in] value
 *
 * @see elm_fileselector_entry_inwin_mode_set
 *
 * @ingroup File_Selector_Entry
 */
#define elm_obj_fileselector_entry_inwin_mode_set(value) ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_INWIN_MODE_SET), EO_TYPECHECK(Eina_Bool, value)

/**
 * @def elm_obj_fileselector_entry_inwin_mode_get
 * @since 1.8
 *
 * Get whether a given file selector entry widget's internal file
 *
 * @param[out] ret
 *
 * @see elm_fileselector_entry_inwin_mode_get
 *
 * @ingroup File_Selector_Entry
 */
#define elm_obj_fileselector_entry_inwin_mode_get(ret) ELM_OBJ_FILESELECTOR_ENTRY_ID(ELM_OBJ_FILESELECTOR_ENTRY_SUB_ID_INWIN_MODE_GET), EO_TYPECHECK(Eina_Bool *, ret)
