#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <unistd.h>

#include "Ecore.h"
#include "ecore_private.h"

static int throttle_val = 0;

/**
 * @addtogroup Ecore_Throttle_Group Ecore Throttle functions
 *
 * @{
 */

/**
 * Increase throttle amount
 *
 * This will increase or decrease (if @p amount is positive or negative) the
 * amount of "voluntary throttling" ecore will do to its main loop while
 * running. This is intended to be used to limit animations and wakeups when
 * in a strict power management state. The higher the current throttle value
 * (which can be retrieved by ecore_throttle_get() ), the more throttling
 * takes place. If the current throttle value is 0, then no throttling takes
 * place at all.
 *
 * The value represents how long the ecore main loop will sleep (in seconds)
 * before it goes into a fully idle state waiting for events, input or
 * timing events to wake it up. For example, if the current throttle level
 * is 0.5, then after every time the main loop cycles and goes into idle
 * affter processing all events, the main loop will explicitly sleep for 0.5
 * seconds before sitting and waiting for incoming events or timeouts, thus
 * preventing animation, async IO and network handling etc. for that period
 * of time. Of course these events, data and timeouts will be buffered,
 * thus not losing anything, simply delaying when they get handled by the
 * throttle value.
 *
 * Example:
 * @code
 * void enter_powersave(void) {
 *    ecore_throttle_adjust(0.2);
 *    printf("Now at throttle level: %1.3f\n", ecore_throttle_get());
 * }
 *
 * void enter_deep_powersave(void) {
 *    ecore_throttle_adjust(0.5);
 *    printf("Now at throttle level: %1.3f\n", ecore_throttle_get());
 * }
 *
 * void exit_powersave(void) {
 *    ecore_throttle_adjust(-0.2);
 *    printf("Now at throttle level: %1.3f\n", ecore_throttle_get());
 * }
 *
 * void exit_deep_powersave(void) {
 *    ecore_throttle_adjust(-0.5);
 *    printf("Now at throttle level: %1.3f\n", ecore_throttle_get());
 * }
 * @endcode
 *
 * @param   amount Amount (in seconds) to adjust by
 */
EAPI void
ecore_throttle_adjust(double amount)
{
   int adj = amount * 1000000.0;
   throttle_val += adj;
   if (throttle_val < 0) throttle_val = 0;
}

/**
 * Get current throttle level
 *
 * This gets the current throttling level, which can be adjusted by
 * ecore_throttle_adjust(). The value is in seconds. Please see
 * ecore_throttle_adjust() for more information.
 *
 * @return  The current throttle level
 */
EAPI double
ecore_throttle_get(void)
{
   return (double)throttle_val / 1000000.0;
}

/**
 * @}
 */

void
_ecore_throttle(void)
{
   if (throttle_val <= 0) return;
   usleep(throttle_val);
}

