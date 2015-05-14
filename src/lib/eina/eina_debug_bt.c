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

#include "eina_debug.h"

#ifdef EINA_HAVE_DEBUG

void
_eina_debug_dump_fhandle_bt(FILE *f, void **bt, int btlen)
{
   int i;
   Dl_info info;
   const char *file;
   unsigned long long offset, base;

   for (i = 0; i < btlen; i++)
     {
        file = NULL;
        offset = base = 0;
        // we have little choice but to hgope/assume dladdr() doesn't alloc
        // anything here
        if ((dladdr(bt[i], &info)) && (info.dli_fname[0]))
          {
             offset = (unsigned long long)bt[i];
             base = (unsigned long long)info.dli_fbase;
             file = _eina_debug_file_get(info.dli_fname);
          }
        // rely on normal libc buffering for file ops to avoid syscalls.
        // may or may not be a good idea. good enough for now.
        if (file) fprintf(f, "%s\t 0x%llx 0x%llx\n", file, offset, base);
        else fprintf(f, "??\t -\n");
     }
}
#endif
