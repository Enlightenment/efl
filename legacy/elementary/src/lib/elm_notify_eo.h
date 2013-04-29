/**
 * @ingroup Notify
 *
 * @{
 */
#define ELM_OBJ_NOTIFY_CLASS elm_obj_notify_class_get()

const Eo_Class *elm_obj_notify_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_NOTIFY_BASE_ID;

enum
{
   ELM_OBJ_NOTIFY_SUB_ID_TIMEOUT_SET,
   ELM_OBJ_NOTIFY_SUB_ID_TIMEOUT_GET,
   ELM_OBJ_NOTIFY_SUB_ID_ALLOW_EVENTS_SET,
   ELM_OBJ_NOTIFY_SUB_ID_ALLOW_EVENTS_GET,
   ELM_OBJ_NOTIFY_SUB_ID_ALIGN_SET,
   ELM_OBJ_NOTIFY_SUB_ID_ALIGN_GET,
   ELM_OBJ_NOTIFY_SUB_ID_LAST
};

#define ELM_OBJ_NOTIFY_ID(sub_id) (ELM_OBJ_NOTIFY_BASE_ID + sub_id)

/**
 * @def elm_obj_notify_timeout_set
 * @since 1.8
 *
 * @brief Set the time interval after which the notify window is going to be
 *
 * @param[in] timeout
 *
 * @see elm_notify_timeout_set
 */
#define elm_obj_notify_timeout_set(timeout) ELM_OBJ_NOTIFY_ID(ELM_OBJ_NOTIFY_SUB_ID_TIMEOUT_SET), EO_TYPECHECK(double, timeout)

/**
 * @def elm_obj_notify_timeout_get
 * @since 1.8
 *
 * @brief Return the timeout value (in seconds)
 *
 * @param[out] ret
 *
 * @see elm_notify_timeout_get
 */
#define elm_obj_notify_timeout_get(ret) ELM_OBJ_NOTIFY_ID(ELM_OBJ_NOTIFY_SUB_ID_TIMEOUT_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_notify_allow_events_set
 * @since 1.8
 *
 * @brief Sets whether events should be passed to by a click outside
 *
 * @param[in] allow
 *
 * @see elm_notify_allow_events_set
 */
#define elm_obj_notify_allow_events_set(allow) ELM_OBJ_NOTIFY_ID(ELM_OBJ_NOTIFY_SUB_ID_ALLOW_EVENTS_SET), EO_TYPECHECK(Eina_Bool, allow)

/**
 * @def elm_obj_notify_allow_events_get
 * @since 1.8
 *
 * @brief Return true if events are allowed below the notify object
 *
 * @param[out] ret
 *
 * @see elm_notify_allow_events_get
 */
#define elm_obj_notify_allow_events_get(ret) ELM_OBJ_NOTIFY_ID(ELM_OBJ_NOTIFY_SUB_ID_ALLOW_EVENTS_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_notify_align_set
 * @since 1.8
 *
 * @brief Set the alignment of the notify object
 *
 * @param[in] horizontal
 * @param[in] vertical
 *
 * @see elm_notify_align_set
 */
#define elm_obj_notify_align_set(horizontal, vertical) ELM_OBJ_NOTIFY_ID(ELM_OBJ_NOTIFY_SUB_ID_ALIGN_SET), EO_TYPECHECK(double, horizontal), EO_TYPECHECK(double, vertical)

/**
 * @def elm_obj_notify_align_get
 * @since 1.8
 *
 * @brief Return the alignment of the notify object
 *
 * @param[out] horizontal
 * @param[out] vertical
 *
 * @see elm_notify_align_get
 */
#define elm_obj_notify_align_get(horizontal, vertical) ELM_OBJ_NOTIFY_ID(ELM_OBJ_NOTIFY_SUB_ID_ALIGN_GET), EO_TYPECHECK(double *, horizontal), EO_TYPECHECK(double *, vertical)


/**
 * @}
 */
