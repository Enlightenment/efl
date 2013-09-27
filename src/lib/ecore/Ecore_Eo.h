/* This include has been added to support Eo in Ecore */
#include <Eo.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup Ecore_Poller_Group
 *
 * @{
 */
#define ECORE_POLLER_CLASS ecore_poller_class_get()
const Eo_Class *ecore_poller_class_get(void) EINA_CONST;
extern EAPI Eo_Op ECORE_POLLER_BASE_ID;

enum
{
   ECORE_POLLER_SUB_ID_CONSTRUCTOR,
   ECORE_POLLER_SUB_ID_INTERVAL_SET,
   ECORE_POLLER_SUB_ID_INTERVAL_GET,
   ECORE_POLLER_SUB_ID_LAST,
};

#define ECORE_POLLER_ID(sub_id) (ECORE_POLLER_BASE_ID + sub_id)

/**
 * @def ecore_poller_constructor
 * @since 1.8
 *
 * Contructor with parameters for Ecore Poller.
 *
 * @param[in] type
 * @param[in] interval
 * @param[in] func
 * @param[in] data
 *
 */
#define ecore_poller_constructor(type, interval, func, data) ECORE_POLLER_ID(ECORE_POLLER_SUB_ID_CONSTRUCTOR), EO_TYPECHECK(Ecore_Poller_Type, type), EO_TYPECHECK(int, interval), EO_TYPECHECK(Ecore_Task_Cb, func), EO_TYPECHECK(const void *, data)

/**
 * @def ecore_poller_interval_set
 * @since 1.8
 *
 * Changes the polling interval rate of poller.
 *
 * @param[in] interval
 * @param[out] ret
 *
 * @see ecore_poller_poller_interval_set
 */
#define ecore_poller_interval_set(interval, ret) ECORE_POLLER_ID(ECORE_POLLER_SUB_ID_INTERVAL_SET), EO_TYPECHECK(int, interval), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def ecore_poller_interval_get
 * @since 1.8
 *
 * Gets the polling interval rate of poller.
 *
 * @param[out] ret
 *
 * @see ecore_poller_poller_interval_get
 */
#define ecore_poller_interval_get(ret) ECORE_POLLER_ID(ECORE_POLLER_SUB_ID_INTERVAL_GET), EO_TYPECHECK(int *, ret)

/**
 * @}
 */

/**
 * @ingroup Ecore_Animator_Group
 *
 * @{
 */

#define ECORE_ANIMATOR_CLASS ecore_animator_class_get()
const Eo_Class *ecore_animator_class_get(void) EINA_CONST;

extern EAPI Eo_Op ECORE_ANIMATOR_BASE_ID;

enum
{
   ECORE_ANIMATOR_SUB_ID_CONSTRUCTOR,
   ECORE_ANIMATOR_SUB_ID_TIMELINE_CONSTRUCTOR,
   ECORE_ANIMATOR_SUB_ID_LAST
};

#define ECORE_ANIMATOR_ID(sub_id) (ECORE_ANIMATOR_BASE_ID + sub_id)

/**
 * @def ecore_animator_constructor
 * @since 1.8
 *
 * Contructor.
 *
 * @param[in] func
 * @param[in] data
 *
 */
#define ecore_animator_constructor(func, data) ECORE_ANIMATOR_ID(ECORE_ANIMATOR_SUB_ID_CONSTRUCTOR), EO_TYPECHECK(Ecore_Task_Cb, func), EO_TYPECHECK(const void *, data)

/**
 * @def ecore_animator_timeline_constructor
 * @since 1.8
 *
 * Contructor.
 *
 * @param[in] runtime
 * @param[in] func
 * @param[in] data
 *
 */
#define ecore_animator_timeline_constructor(runtime, func, data) ECORE_ANIMATOR_ID(ECORE_ANIMATOR_SUB_ID_TIMELINE_CONSTRUCTOR), EO_TYPECHECK(double, runtime), EO_TYPECHECK(Ecore_Timeline_Cb, func), EO_TYPECHECK(const void *, data)

/**
 * @}
 */

/**
 * @ingroup Ecore_Timer_Group
 *
 * @{
 */

#define ECORE_TIMER_CLASS ecore_timer_class_get()
const Eo_Class *ecore_timer_class_get(void) EINA_CONST;
extern EAPI Eo_Op ECORE_TIMER_BASE_ID;

enum
{
   ECORE_TIMER_SUB_ID_CONSTRUCTOR,
   ECORE_TIMER_SUB_ID_LOOP_CONSTRUCTOR,
   ECORE_TIMER_SUB_ID_INTERVAL_SET,
   ECORE_TIMER_SUB_ID_INTERVAL_GET,
   ECORE_TIMER_SUB_ID_DELAY,
   ECORE_TIMER_SUB_ID_RESET,
   ECORE_TIMER_SUB_ID_PENDING_GET,
   ECORE_TIMER_SUB_ID_LAST,
};

#define ECORE_TIMER_ID(sub_id) (ECORE_TIMER_BASE_ID + sub_id)

/**
 * @def ecore_timer_constructor
 * @since 1.8
 *
 * Contructor.
 *
 * @param[in] in
 * @param[in] func
 * @param[in] data
 *
 */
#define ecore_timer_constructor(in, func, data) ECORE_TIMER_ID(ECORE_TIMER_SUB_ID_CONSTRUCTOR), EO_TYPECHECK(double, in), EO_TYPECHECK(Ecore_Task_Cb, func), EO_TYPECHECK(const void *, data)

/**
 * @def ecore_timer_loop_constructor
 * @since 1.8
 *
 * Contructor.
 *
 * @param[in] in
 * @param[in] func
 * @param[in] data
 *
 */
#define ecore_timer_loop_constructor(in, func, data) ECORE_TIMER_ID(ECORE_TIMER_SUB_ID_LOOP_CONSTRUCTOR), EO_TYPECHECK(double, in), EO_TYPECHECK(Ecore_Task_Cb, func), EO_TYPECHECK(const void *, data)

/**
 * @def ecore_obj_timer_interval_set
 * @since 1.8
 *
 * Change the interval the timer ticks of.
 *
 * @param[in] in
 *
 * @see ecore_timer_interval_set
 */
#define ecore_obj_timer_interval_set(in) ECORE_TIMER_ID(ECORE_TIMER_SUB_ID_INTERVAL_SET), EO_TYPECHECK(double, in)

/**
 * @def ecore_obj_timer_interval_get
 * @since 1.8
 *
 * Get the interval the timer ticks on.
 *
 * @param[out] ret
 *
 * @see ecore_timer_interval_get
 */
#define ecore_obj_timer_interval_get(ret) ECORE_TIMER_ID(ECORE_TIMER_SUB_ID_INTERVAL_GET), EO_TYPECHECK(double *, ret)

/**
 * @def ecore_obj_timer_delay
 * @since 1.8
 *
 * Add some delay for the next occurrence of a timer.
 *
 * @param[in] add
 *
 * @see ecore_timer_delay
 */
#define ecore_obj_timer_delay(add) ECORE_TIMER_ID(ECORE_TIMER_SUB_ID_DELAY), EO_TYPECHECK(double, add)

/**
 * @def ecore_obj_timer_reset
 * @since 1.8
 *
 * Reset a timer to its full interval.
 *
 * @see ecore_timer_reset
 */
#define ecore_obj_timer_reset() ECORE_TIMER_ID(ECORE_TIMER_SUB_ID_RESET)

/**
 * @def ecore_obj_timer_pending_get
 * @since 1.8
 *
 * Get the pending time regarding a timer.
 *
 * @param[out] ret
 *
 * @see ecore_timer_pending_get
 */
#define ecore_obj_timer_pending_get(ret) ECORE_TIMER_ID(ECORE_TIMER_SUB_ID_PENDING_GET), EO_TYPECHECK(double *, ret)

/**
 * @}
 */

/**
 * @ingroup Ecore_Idle_Group
 *
 * @{
 */

#define ECORE_IDLER_CLASS ecore_idler_class_get()
const Eo_Class *ecore_idler_class_get(void) EINA_CONST;

extern EAPI Eo_Op ECORE_IDLER_BASE_ID;

enum
{
   ECORE_IDLER_SUB_ID_CONSTRUCTOR,
   ECORE_IDLER_SUB_ID_LAST
};

#define ECORE_IDLER_ID(sub_id) (ECORE_IDLER_BASE_ID + sub_id)

/**
 * @def ecore_idler_constructor
 * @since 1.8
 *
 * Contructor.
 *
 * @param[in] func
 * @param[in] data
 *
 */
#define ecore_idler_constructor(func, data) ECORE_IDLER_ID(ECORE_IDLER_SUB_ID_CONSTRUCTOR), EO_TYPECHECK(Ecore_Task_Cb, func), EO_TYPECHECK(const void *, data)

/**
 *
 */

#define ECORE_IDLE_ENTERER_CLASS ecore_idle_enterer_class_get()
const Eo_Class *ecore_idle_enterer_class_get(void) EINA_CONST;

extern EAPI Eo_Op ECORE_IDLE_ENTERER_BASE_ID;

enum
{
   ECORE_IDLE_ENTERER_SUB_ID_AFTER_CONSTRUCTOR,
   ECORE_IDLE_ENTERER_SUB_ID_BEFORE_CONSTRUCTOR,
   ECORE_IDLE_ENTERER_SUB_ID_LAST
};

#define ECORE_IDLE_ENTERER_ID(sub_id) (ECORE_IDLE_ENTERER_BASE_ID + sub_id)

/**
 * @def ecore_idle_enterer_after_constructor
 * @since 1.8
 *
 * Contructor. Will insert the handler at the end of the list.
 *
 * @param[in] func
 * @param[in] data
 *
 */
#define ecore_idle_enterer_after_constructor(func, data) ECORE_IDLE_ENTERER_ID(ECORE_IDLE_ENTERER_SUB_ID_AFTER_CONSTRUCTOR), EO_TYPECHECK(Ecore_Task_Cb, func), EO_TYPECHECK(const void *, data)

/**
 * @def ecore_idle_enterer_before_constructor
 * @since 1.8
 *
 * Contructor. Will insert the handler at the beginning of the list.
 *
 * @param[in] func
 * @param[in] data
 *
 */
#define ecore_idle_enterer_before_constructor(func, data) ECORE_IDLE_ENTERER_ID(ECORE_IDLE_ENTERER_SUB_ID_BEFORE_CONSTRUCTOR), EO_TYPECHECK(Ecore_Task_Cb, func), EO_TYPECHECK(const void *, data)

/**
 *
 */

#define ECORE_IDLE_EXITER_CLASS ecore_idle_exiter_class_get()
const Eo_Class *ecore_idle_exiter_class_get(void) EINA_CONST;

extern EAPI Eo_Op ECORE_IDLE_EXITER_BASE_ID;

enum
{
   ECORE_IDLE_EXITER_SUB_ID_CONSTRUCTOR,
   ECORE_IDLE_EXITER_SUB_ID_LAST
};

#define ECORE_IDLE_EXITER_ID(sub_id) (ECORE_IDLE_EXITER_BASE_ID + sub_id)

/**
 * @def ecore_idle_exiter_constructor
 * @since 1.8
 *
 * Contructor.
 *
 * @param[in] func
 * @param[in] data
 *
 */
#define ecore_idle_exiter_constructor(func, data) ECORE_IDLE_EXITER_ID(ECORE_IDLE_EXITER_SUB_ID_CONSTRUCTOR), EO_TYPECHECK(Ecore_Task_Cb, func), EO_TYPECHECK(const void *, data)

/**
 * @}
 */

/**
 * @ingroup Ecore_Job_Group
 *
 * @{
 */

#define ECORE_JOB_CLASS ecore_job_class_get()
const Eo_Class *ecore_job_class_get(void) EINA_CONST;

extern EAPI Eo_Op ECORE_JOB_BASE_ID;

enum
{
   ECORE_JOB_SUB_ID_CONSTRUCTOR,
   ECORE_JOB_SUB_ID_LAST
};

#define ECORE_JOB_ID(sub_id) (ECORE_JOB_BASE_ID + sub_id)

/**
 * @def ecore_job_constructor
 * @since 1.8
 *
 * Contructor.
 *
 * @param[in] func
 * @param[in] data
 *
 */
#define ecore_job_constructor(func, data) ECORE_JOB_ID(ECORE_JOB_SUB_ID_CONSTRUCTOR), EO_TYPECHECK(Ecore_Cb, func), EO_TYPECHECK(const void *, data)

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
