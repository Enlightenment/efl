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
#define send_cli(cli, op, data, size) \
   do { \
      unsigned char head[8]; \
      char *op2 = op; \
      int size2 = size + 4; \
      memcpy(head + 0, &size2, 4); \
      memcpy(head + 4, op2, 4); \
      ecore_con_client_send(cli, head, 8); \
      if (size > 0) ecore_con_client_send(cli, data, size); \
   } while (0)

#endif
