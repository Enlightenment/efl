/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric BAIL, Vincent Torri
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

#ifndef EINA_CONVERT_H_
#define EINA_CONVERT_H_

#include "eina_types.h"
#include "eina_error.h"

EAPI extern Eina_Error EINA_ERROR_CONVERT_P_NOT_FOUND;
EAPI extern Eina_Error EINA_ERROR_CONVERT_0X_NOT_FOUND;
EAPI extern Eina_Error EINA_ERROR_CONVERT_OUTRUN_STRING_LENGTH;

EAPI int eina_convert_init(void);
EAPI int eina_convert_shutdown(void);

EAPI int eina_convert_itoa(int n, char *s);
EAPI int eina_convert_xtoa(unsigned int n, char *s);
EAPI int eina_convert_dtoa(double d, char *des);
EAPI Eina_Bool eina_convert_atod(const char *src, int length, long long *m, long *e);

#endif /* EINA_CONVERT_H_ */

