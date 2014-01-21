/**
 * @ingroup Fileselector
 *
 * @{
 */
#define ELM_OBJ_FILESELECTOR_CLASS elm_obj_fileselector_class_get()

const Eo_Class *elm_obj_fileselector_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_FILESELECTOR_BASE_ID;

enum
{
   ELM_OBJ_FILESELECTOR_SUB_ID_BUTTONS_OK_CANCEL_SET,
   ELM_OBJ_FILESELECTOR_SUB_ID_BUTTONS_OK_CANCEL_GET,
   ELM_OBJ_FILESELECTOR_SUB_ID_LAST
};

#define ELM_OBJ_FILESELECTOR_ID(sub_id) (ELM_OBJ_FILESELECTOR_BASE_ID + sub_id)

/**
 * @def elm_obj_fileselector_buttons_ok_cancel_set
 * @since 1.8
 *
 * Enable/disable the "ok" and "cancel" buttons on a given file
 *
 * @param[in] visible
 *
 * @see elm_fileselector_buttons_ok_cancel_set
 */
#define elm_obj_fileselector_buttons_ok_cancel_set(visible) ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_BUTTONS_OK_CANCEL_SET), EO_TYPECHECK(Eina_Bool, visible)

/**
 * @def elm_obj_fileselector_buttons_ok_cancel_get
 * @since 1.8
 *
 * Get whether the "ok" and "cancel" buttons on a given file
 *
 * @param[out] ret
 *
 * @see elm_fileselector_buttons_ok_cancel_get
 */
#define elm_obj_fileselector_buttons_ok_cancel_get(ret) ELM_OBJ_FILESELECTOR_ID(ELM_OBJ_FILESELECTOR_SUB_ID_BUTTONS_OK_CANCEL_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @}
 */
