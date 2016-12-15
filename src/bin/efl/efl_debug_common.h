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

#ifndef EFL_DEBUG_COMMON_H
#define EFL_DEBUG_COMMON_H 1

#define EFL_BETA_API_SUPPORT 1
#define EFL_EO_API_SUPPORT 1

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Con.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct _Efl_Debug_Message_Header {
   unsigned int size;
   char op[4];
} Efl_Debug_Message_Header;

void _protocol_collect(unsigned char **buf, unsigned int *buf_size,
                       void *data, int size);
int _proto_read(unsigned char **buf, unsigned int *buf_size,
                char *op, unsigned char **data);

#define fetch_val(dst, buf, off) \
   memcpy(&dst, ((unsigned char *)buf) + off, sizeof(dst))
#define store_val(buf, off, src) \
   memcpy(buf + off, &src, sizeof(src))

#define IS_OP(x) memcmp(op, OP_ ## x, 4) == 0

#define DECLARE_OP(x) static char OP_ ## x[4] = #x
#ifdef DECLARE_OPS
DECLARE_OP(LIST);
DECLARE_OP(CLST);
DECLARE_OP(PLON);
DECLARE_OP(PLOF);
DECLARE_OP(EVON);
DECLARE_OP(EVOF);
DECLARE_OP(EVLG);
DECLARE_OP(HELO);
#endif

Eina_Bool send_data(Eo *sock, const char op[static 4], const void *data, unsigned int len);
Eina_Bool received_data(Eo *sock, void (*handle)(void *data, const char op[static 4], const Eina_Slice payload), const void *data);

#endif
