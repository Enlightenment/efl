#define ELM_OBJ_CLOCK_CLASS elm_obj_clock_class_get()

const Eo_Class *elm_obj_clock_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_CLOCK_BASE_ID;

enum
{
   ELM_OBJ_CLOCK_SUB_ID_TIME_SET,
   ELM_OBJ_CLOCK_SUB_ID_TIME_GET,
   ELM_OBJ_CLOCK_SUB_ID_EDIT_SET,
   ELM_OBJ_CLOCK_SUB_ID_EDIT_GET,
   ELM_OBJ_CLOCK_SUB_ID_EDIT_MODE_SET,
   ELM_OBJ_CLOCK_SUB_ID_EDIT_MODE_GET,
   ELM_OBJ_CLOCK_SUB_ID_SHOW_AM_PM_SET,
   ELM_OBJ_CLOCK_SUB_ID_SHOW_AM_PM_GET,
   ELM_OBJ_CLOCK_SUB_ID_SHOW_SECONDS_SET,
   ELM_OBJ_CLOCK_SUB_ID_SHOW_SECONDS_GET,
   ELM_OBJ_CLOCK_SUB_ID_FIRST_INTERVAL_SET,
   ELM_OBJ_CLOCK_SUB_ID_FIRST_INTERVAL_GET,
   ELM_OBJ_CLOCK_SUB_ID_PAUSE_SET,
   ELM_OBJ_CLOCK_SUB_ID_PAUSE_GET,
   ELM_OBJ_CLOCK_SUB_ID_LAST,
};

#define ELM_OBJ_CLOCK_ID(sub_id) (ELM_OBJ_CLOCK_BASE_ID + sub_id)


/**
 * @def elm_obj_clock_time_set
 * @since 1.8
 *
 * Set a clock widget's time, programmatically
 *
 * @param[in] hrs
 * @param[in] min
 * @param[in] sec
 *
 * @see elm_clock_time_set
 *
 * @ingroup Clock
 */
#define elm_obj_clock_time_set(hrs, min, sec) ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_TIME_SET), EO_TYPECHECK(int, hrs), EO_TYPECHECK(int, min), EO_TYPECHECK(int, sec)

/**
 * @def elm_obj_clock_time_get
 * @since 1.8
 *
 * Get a clock widget's time values
 *
 * @param[out] hrs
 * @param[out] min
 * @param[out] sec
 *
 * @see elm_clock_time_get
 *
 * @ingroup Clock
 */
#define elm_obj_clock_time_get(hrs, min, sec) ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_TIME_GET), EO_TYPECHECK(int *, hrs), EO_TYPECHECK(int *, min), EO_TYPECHECK(int *, sec)

/**
 * @def elm_obj_clock_edit_set
 * @since 1.8
 *
 * Set whether a given clock widget is under <b>edition mode</b> or
 * under (default) displaying-only mode.
 *
 * @param[in] edit
 *
 * @see elm_clock_edit_set
 *
 * @ingroup Clock
 */
#define elm_obj_clock_edit_set(edit) ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_EDIT_SET), EO_TYPECHECK(Eina_Bool, edit)

/**
 * @def elm_obj_clock_edit_get
 * @since 1.8
 *
 * Retrieve whether a given clock widget is under editing mode
 * under (default) displaying-only mode.
 *
 * @param[out] ret
 *
 * @see elm_clock_edit_get
 *
 * @ingroup Clock
 */
#define elm_obj_clock_edit_get(ret) ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_EDIT_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_clock_edit_mode_set
 * @since 1.8
 *
 * Set what digits of the given clock widget should be editable
 *
 * @param[in] digedit
 *
 * @see elm_clock_edit_mode_set
 *
 * @ingroup Clock
 */
#define elm_obj_clock_edit_mode_set(digedit) ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_EDIT_MODE_SET), EO_TYPECHECK(Elm_Clock_Edit_Mode, digedit)

/**
 * @def elm_obj_clock_edit_mode_get
 * @since 1.8
 *
 * Retrieve what digits of the given clock widget should be
 * editable when in edition mode.
 *
 * @param[out] ret
 *
 * @see elm_clock_edit_mode_get
 *
 * @ingroup Clock
 */
#define elm_obj_clock_edit_mode_get(ret) ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_EDIT_MODE_GET), EO_TYPECHECK(Elm_Clock_Edit_Mode *, ret)

/**
 * @def elm_obj_clock_show_am_pm_set
 * @since 1.8
 *
 * Set if the given clock widget must show hours in military or
 * am/pm mode
 *
 * @param[in] am_pm
 *
 * @see elm_clock_show_am_pm_set
 *
 * @ingroup Clock
 */
#define elm_obj_clock_show_am_pm_set(am_pm) ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_SHOW_AM_PM_SET), EO_TYPECHECK(Eina_Bool, am_pm)

/**
 * @def elm_obj_clock_show_am_pm_get
 * @since 1.8
 *
 * Get if the given clock widget shows hours in military or am/pm
 * mode
 *
 * @param[out] ret
 *
 * @see elm_clock_show_am_pm_get
 *
 * @ingroup Clock
 */
#define elm_obj_clock_show_am_pm_get(ret) ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_SHOW_AM_PM_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_clock_show_seconds_set
 * @since 1.8
 *
 * Set if the given clock widget must show time with seconds or not
 *
 * @param[in] seconds
 *
 * @see elm_clock_show_seconds_set
 *
 * @ingroup Clock
 */
#define elm_obj_clock_show_seconds_set(seconds) ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_SHOW_SECONDS_SET), EO_TYPECHECK(Eina_Bool, seconds)

/**
 * @def elm_obj_clock_show_seconds_get
 * @since 1.8
 *
 * Get whether the given clock widget is showing time with seconds
 *
 * @param[out] ret
 *
 * @see elm_clock_show_seconds_get
 *
 * @ingroup Clock
 */
#define elm_obj_clock_show_seconds_get(ret) ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_SHOW_SECONDS_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_clock_first_interval_set
 * @since 1.8
 *
 * Set the first interval on time updates for a user mouse button hold
 *
 * @param[in] interval
 *
 * @see elm_clock_first_interval_set
 *
 * @ingroup Clock
 */
#define elm_obj_clock_first_interval_set(interval) ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_FIRST_INTERVAL_SET), EO_TYPECHECK(double, interval)

/**
 * @def elm_obj_clock_first_interval_get
 * @since 1.8
 *
 * Get the first interval on time updates for a user mouse button hold
 *
 * @param[out] ret
 *
 * @see elm_clock_first_interval_get
 *
 * @ingroup Clock
 */
#define elm_obj_clock_first_interval_get(ret) ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_FIRST_INTERVAL_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_clock_pause_set
 * @since 1.9
 *
 * Set whether the given clock widget should be paused or not.
 *
 * @param[in] pause
 *
 * @see elm_clock_pause_set
 *
 * @ingroup Clock
 */
#define elm_obj_clock_pause_set(pause) ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_PAUSE_SET), EO_TYPECHECK(Eina_Bool, pause)

/**
 * @def elm_obj_clock_pause_get
 * @since 1.9
 *
 * Get whether the given clock widget is paused.
 *
 * @param[out] ret
 *
 * @see elm_clock_pause_get
 *
 * @ingroup Clock
 */
#define elm_obj_clock_pause_get(ret) ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_PAUSE_GET), EO_TYPECHECK(Eina_Bool *, ret)
