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

Eina_Bool
received_data(Eo *sock, void (*handle)(void *data, const char op[static 4], const Eina_Slice payload), const void *data)
{
   Eina_Slice slice, payload;
   Efl_Debug_Message_Header msgheader;

   slice = efl_io_buffered_stream_slice_get(sock);
   if (slice.len < sizeof(msgheader))
     return EINA_TRUE;

   memcpy(&msgheader, slice.mem, sizeof(msgheader));
   if (msgheader.size < 4) /* must contain at last 4 byte opcode */
     {
        fprintf(stderr, "ERROR: invalid message header, size=%u\n", msgheader.size);
        return EINA_FALSE;
     }

   if (msgheader.size + 4 > slice.len)
     return EINA_TRUE;

   payload.bytes = slice.bytes + sizeof(msgheader);
   payload.len = msgheader.size - 4;

   handle((void *)data, msgheader.op, payload);

   efl_io_buffered_stream_discard(sock, sizeof(msgheader) + payload.len);
   return EINA_TRUE;
}

Eina_Bool
send_data(Eo *sock, const char op[static 4], const void *data, unsigned int len)
{
   Eina_Error err;
   Efl_Debug_Message_Header msghdr = {
     .size = 4 + len,
   };
   Eina_Slice s, r;

   memcpy(msghdr.op, op, 4);

   s.mem = &msghdr;
   s.len = sizeof(msghdr);

   err = efl_io_writer_write(sock, &s, &r);
   if (err || r.len) goto end;

   if (!len) goto end;

   s.mem = data;
   s.len = len;
   err = efl_io_writer_write(sock, &s, &r);

 end:
   if (err)
     {
        fprintf(stderr, "ERROR: could not queue message '%.4s': %s\n", op, eina_error_msg_get(err));
        return EINA_FALSE;
     }

   if (r.len)
     {
        fprintf(stderr, "ERROR: could not queue message '%.4s': out of memory\n", op);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}
