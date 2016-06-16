/* EIO - EFL data type library
 * Copyright (C) 2010 Enlightenment Developers:
 *           Cedric Bail <cedric.bail@free.fr>
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
#include "eio_private.h"

#include "efl_uri_mapper_file.eo.h"

typedef struct _Efl_Uri_Mapper_File_Data Efl_Uri_Mapper_File_Data;
struct _Efl_Uri_Mapper_File_Data
{
};

static Eina_Promise *
_efl_uri_mapper_file_efl_uri_mapper_fetch(Eo *obj, Efl_Uri_Mapper_File_Data *pd EINA_UNUSED, const Eina_Stringshare *file)
{
   Efl_Io_Manager *iom;
   unsigned int offset = 0;

   iom = eo_provider_find(obj, EFL_IO_MANAGER_CLASS);
   if (!iom) return NULL;

   if (!strncmp("file://", file, 7))
     offset += 7;

   return efl_io_manager_file_open(iom, file + offset, EINA_FALSE);
}

#include "efl_uri_mapper_file.eo.c"
