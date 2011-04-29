/* EINA - EFL data type library
 * Copyright (C) 2011 Vincent Torri
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

#ifndef EINA_LOCK_H_
#define EINA_LOCK_H_

#include "eina_config.h"

/**
 * @addtogroup Eina_Tools_Group Tools
 *
 * @{
 */

/**
 * @defgroup Eina_File_Group File
 *
 * @{
 */

#ifdef EINA_HAVE_THREADS
# ifdef _WIN32_WCE
#  include "eina_inline_lock_wince.x"
# elif defined(_WIN32)
#  include "eina_inline_lock_win32.x"
# else
#  include "eina_inline_lock_posix.x"
# endif
#else
# include "eina_inline_lock_void.x"
#endif

/**
 * @}
 */

/**
 * @}
 */

#endif
