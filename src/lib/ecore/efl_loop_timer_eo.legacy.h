#ifndef _EFL_LOOP_TIMER_EO_LEGACY_H_
#define _EFL_LOOP_TIMER_EO_LEGACY_H_

#ifndef _EFL_LOOP_TIMER_EO_CLASS_TYPE
#define _EFL_LOOP_TIMER_EO_CLASS_TYPE

typedef Eo Efl_Loop_Timer;

#endif

#ifndef _EFL_LOOP_TIMER_EO_TYPES
#define _EFL_LOOP_TIMER_EO_TYPES


#endif

/**
 * @brief Interval the timer ticks on.
 *
 * If set during a timer call this will affect the next interval.
 *
 * @param[in] obj The object.
 * @param[in] in The new interval in seconds
 *
 * @ingroup Ecore_Timer_Group
 */
EAPI void ecore_timer_interval_set(Efl_Loop_Timer *obj, double in);

/**
 * @brief Interval the timer ticks on.
 *
 * @param[in] obj The object.
 *
 * @return The new interval in seconds
 *
 * @ingroup Ecore_Timer_Group
 */
EAPI double ecore_timer_interval_get(const Efl_Loop_Timer *obj);

/**
 * @brief Pending time regarding a timer.
 *
 * @param[in] obj The object.
 *
 * @return Pending time
 *
 * @ingroup Ecore_Timer_Group
 */
EAPI double ecore_timer_pending_get(const Efl_Loop_Timer *obj);

/**
 * @brief Resets a timer to its full interval. This effectively makes the timer
 * start ticking off from zero now.
 *
 * This is equal to delaying the timer by the already passed time, since the
 * timer started ticking
 *
 * @param[in] obj The object.
 *
 * @since 1.2
 *
 * @ingroup Ecore_Timer_Group
 */
EAPI void ecore_timer_reset(Efl_Loop_Timer *obj);

/** This effectively resets a timer but based on the time when this iteration
 * of the main loop started.
 *
 * @since 1.18
 *
 * @ingroup Ecore_Timer_Group
 */
EAPI void ecore_timer_loop_reset(Efl_Loop_Timer *obj);

/**
 * @brief Adds a delay to the next occurrence of a timer. This doesn't affect
 * the timer interval.
 *
 * @param[in] obj The object.
 * @param[in] add The amount of time by which to delay the timer in seconds
 *
 * @ingroup Ecore_Timer_Group
 */
EAPI void ecore_timer_delay(Efl_Loop_Timer *obj, double add);

#endif
