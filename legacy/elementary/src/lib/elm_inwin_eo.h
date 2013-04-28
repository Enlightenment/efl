#define ELM_OBJ_WIN_INWIN_CLASS elm_obj_win_inwin_class_get()

const Eo_Class *elm_obj_win_inwin_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_WIN_INWIN_BASE_ID;

enum
{
   ELM_OBJ_WIN_INWIN_SUB_ID_ACTIVATE,
   ELM_OBJ_WIN_INWIN_SUB_ID_LAST
};

#define ELM_OBJ_WIN_INWIN_ID(sub_id) (ELM_OBJ_WIN_INWIN_BASE_ID + sub_id)

/**
 * @def elm_obj_win_inwin_activate
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @ingroup Inwin
 */
#define elm_obj_win_inwin_activate() ELM_OBJ_WIN_INWIN_ID(ELM_OBJ_WIN_INWIN_SUB_ID_ACTIVATE)
