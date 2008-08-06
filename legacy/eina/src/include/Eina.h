/* EINA - EFL data type library
 * Copyright (C) 2008 Jorge Luis Zapata Muga, Cedric Bail
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

#ifndef EINA_H_
#define EINA_H_

/**
 * @mainpage Eina
 * @file Eina.h
 * @brief Contains list, hash, debugging and tree functions.
 * TODO add debug functions, magic functions, etc
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "eina_types.h"
#include "eina_f16p16.h"
#include "eina_rectangle.h"
#include "eina_inlist.h"
#include "eina_file.h"
#include "eina_list.h"
#include "eina_hash.h"
#include "eina_lalloc.h"
#include "eina_module.h"
#include "eina_mempool.h"
#include "eina_error.h"
#include "eina_array.h"
#include "eina_stringshare.h"
#include "eina_magic.h"
#include "eina_counter.h"

EAPI int eina_init(void);
EAPI int eina_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* EINA_H */
