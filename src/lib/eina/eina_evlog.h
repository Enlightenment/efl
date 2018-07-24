/* EINA - EFL data type library
 * Copyright (C) 2015 Carsten Haitzler
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EINA_EVLOG_H_
#define EINA_EVLOG_H_

/**
 * @addtogroup Eina_Evlog Event Log Debugging
 * @ingroup Eina
 *
 * @brief These functions are use internally by EFL in general for profiling.
 * This API is not considered stable or intended for use outside of EFL
 * itself at this stage, so do not use this unless you wish to keep up. The
 * format of debug logs may and likely will change as this feature matures.
 *
 * @{
 *
 * @since 1.15
 */

typedef struct _Eina_Evlog_Item Eina_Evlog_Item;
typedef struct _Eina_Evlog_Buf Eina_Evlog_Buf;

struct _Eina_Evlog_Item
{
   double tim; // the time when this event happened
   double srctim; // if > 0.0, then this is the src event time causing this
   unsigned long long thread; // a thread handle/id where this log happened
   unsigned long long obj; // an object associated with this event (anything)
   unsigned short event_offset; // must be provided - mem pos after item
   unsigned short detail_offset; // if not provided, 0, otherwise mem pos
   unsigned short event_next; // mem offset in bytes for next event;
};

struct _Eina_Evlog_Buf
{
   unsigned char *buf; // current buffer we fill with event logs
   unsigned int size; // the max size of the evlog buffer
   unsigned int top; // the current top byte for a new evlog item
   unsigned int overflow; // how many times this buffer has overflowed
};

/**
 * @brief Logs an event in our event log for profiling data.
 *
 * Log some interesting event inside of EFL, e.g. a wakeup (and why etc.).
 * The @p event string must always be provided and be of the form:
 *
 * "+eventname"
 * "-eventname"
 * ">eventname"
 * "<eventname"
 * "!eventname"
 * "*eventname"
 *
 * etc. The "+" char means an event is beginning (and any subsequent
 * events logged are really children of this event). The "-" char means an
 * event is ending and so all child events SHOULD have ended by now. A "!"
 * character means the event is a one-off with no beginning or end. A"*"
 * means this is special metadata and the detail field may need special
 * parsing based on the eventname, so ignore unless known.  A ">"
 * character means we begin this "state" of the process (these are separate
 * to "+" and "-" events and don't nest - are not related to a thread or
 * any other event, but just a state). "<" Ends the given state given by
 * the "eventname" part of the string.  Any string following this initial
 * character is the event or state name (and must be provided in the exact
 * same string at both "+", "<" and "-", ">" events). This is what will be
 * displayed in a debugger (and may be a well known string thus given a nice
 * UI flourish with icons, labels and colors, so don't change this string
 * unless you want to impact such visibility of these events). The event
 * string after the first character as above can be anything, including white
 * space. It is suggested to keep it human readable and as short as feasible.
 *
 * The @p object is optional, and if not used, pass in NULL. If it is used,
 * it can be a pointer to anything. It is intended simply to be of use to
 * indicate an event happens on object A vs object B. What this points to
 * is irrelevant as the pointer is never de-referenced or used other than
 * as a label to differentiate an event on 2 different objects.
 *
 * The @p srctime parameter is 0.0 if not used, or if used, contains a
 * timepoint for an event that triggered this once. For example, if a device
 * or hardware interrupt causes this event, that device may provide a
 * timestamp/timepoint as part of the device information to indicate the
 * exact time the hardware interrupt happened. This can be useful to have
 * more information as to the latency of an actual source of an event such
 * as the hardware interrupt time, and when the code actually begins seeing
 * or processing it.
 *
 * The @p detail string is optional (and if unused should be NULL). This is
 * for providing more detailed information to log such as perhaps a the
 * state at the time of the log events or a series of parameters and input
 * that caused this event.
 *
 * @param[in] event The event string - see above for format
 * @param[in] obj An optional object "pointer" to associate
 * @param[in] srctime An optional source event timestamp that caused this event
 * @param[in] detail An optional event detail string with more info
 *
 * @since 1.15
 */
EAPI void
eina_evlog(const char *event, void *obj, double srctime, const char *detail);

/**
 * @brief Steals an event log buffer from the evlog core.
 *
 * Only one buffer can be stolen at any time. If you steal a new buffer, the
 * old stolen buffer is "released" back to the evlog core.
 *
 * @return The stolen evlog buffer
 *
 * @since 1.15
 */
EAPI Eina_Evlog_Buf *
eina_evlog_steal(void);

/**
 * @brief Begins logging - until now eina_evlog is a NOOP.
 *
 * @since 1.15
 */
EAPI void
eina_evlog_start(void);

/**
 * @brief Stops logging.
 *
 * You must not be using any evlog buffers stolen by eina_evlog_steal() by
 * the time you call this function.
 *
 * @since 1.15
 */
EAPI void
eina_evlog_stop(void);

/**
 * @}
 */
#endif
