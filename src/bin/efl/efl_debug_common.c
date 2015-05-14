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

#include "efl_debug_common.h"

void
_protocol_collect(unsigned char **buf, unsigned int *buf_size,
                  void *data, int size)
{
   // no buffer yet - duplicate it as out only data
   if (!*buf)
     {
        *buf = malloc(size);
        if (*buf)
          {
             *buf_size = size;
             memcpy(*buf, data, size);
          }
     }
   // we have data - append to the buffer and reallocate it as needed
   else
     {
        unsigned char *b = realloc(*buf, *buf_size + size);
        if (b)
          {
             *buf = b;
             memcpy(*buf + *buf_size, data, size);
             *buf_size += size;
          }
     }
}

int
_proto_read(unsigned char **buf, unsigned int *buf_size,
            char *op, unsigned char **data)
{
   unsigned int size, new_buf_size;
   unsigned char *b;

   // we have no data yet, or not enough - minimum 8 bytes
   if (!*buf) return -1;
   if (*buf_size < 8) return -1;
   // get size of total message
   memcpy(&size, *buf, 4);
   // if size is invalid < 4 bytes - no message there
   if (size < 4) return -1;
   // if our total message buffer size is not big enough yet - no message
   if (*buf_size < (size + 4)) return -1;

   // copy out 4 byte opcode and nul byet terminate it
   memcpy(op, *buf + 4, 4);
   op[4] = 0;

   // take off opcode header of 4 bytes
   size -= 4;
   // the new buffer size once we remove header+payload is...
   new_buf_size = *buf_size - (size + 8);
   if (size == 0)
     {
        *data = NULL;
        size = 0;
     }
   else
     {
        // allocate new space for payload
        *data = malloc(size);
        if (!*data)
          {
             // allocation faild - no message
             return -1;
          }
        memcpy(*data, *buf + 8, size);
     }
   // if new shrunk buffer size is empty -= just simply free buffer
   if (new_buf_size == 0)
     {
        free(*buf);
        *buf = NULL;
     }
   else
     {
        // allocate newly shrunk buffer
        b = malloc(new_buf_size);
        if (!b)
          {
             // alloc failure - bad. fail proto read then
             free(*data);
             return -1;
          }
        // copy data to new smaller buffer and free old, storing new buffer
        memcpy(b, *buf + size + 8, new_buf_size);
        free(*buf);
        *buf = b;
     }
   // store new buffer size
   *buf_size = new_buf_size;
   return (int)size;
}
