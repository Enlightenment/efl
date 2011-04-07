/* EINA - EFL data type library
 * Copyright (C) 2010 ProFUSION embedded systems
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

#ifndef EINA_SCHED_H_
#define EINA_SCHED_H_

#include "eina_types.h"


/**
 * @brief Lower priority of current thread.
 *
 * It's used by worker threads so they use up background cpu and do not stall
 * the main thread If current thread is running with real-time priority, we
 * decrease our priority by @c RTNICENESS. This is done in a portable way.
 *
 * Otherwise (we are running with SCHED_OTHER policy) there's no portable way to
 * set the nice level on current thread. In Linux, it does work and it's the
 * only one that is implemented as of now. In this case the nice level is
 * incremented on this thread by @c NICENESS.
 */
EAPI void eina_sched_prio_drop(void);

#endif /* EINA_SCHED_H_ */
