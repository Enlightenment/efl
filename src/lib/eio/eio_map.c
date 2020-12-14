/* EIO - EFL data type library
 * Copyright (C) 2010 Enlightenment Developers:
 *           Cedric Bail <cedric.bail@free.fr>
 *           Vincent "caro" Torri  <vtorri at univ-evry dot fr>
 *           Stephen "okra" Houston <UnixTitan@gmail.com>
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
#include "Eio.h"


/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

static void
_eio_file_open_job(void *data, Ecore_Thread *thread)
{
   Eio_File_Map *map = data;

   map->result = eina_file_open(map->name, map->shared);
   if (!map->result) eio_file_thread_error(&map->common, thread);
}

static void
_eio_file_open_free(Eio_File_Map *map)
{
   if (map->name) eina_stringshare_del(map->name);
   eio_file_free((Eio_File*)map);
}

static void
_eio_file_open_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_File_Map *map = data;

   map->open_cb((void*) map->common.data, &map->common, map->result);
   _eio_file_open_free(map);
}

static void
_eio_file_open_cancel(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_File_Map *map = data;

   eio_file_error(&map->common);
   _eio_file_open_free(map);
}

static void
_eio_file_close_job(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_File_Map *map = data;

   map->common.length = eina_file_size_get(map->result);
   eina_file_close(map->result);
}

static void
_eio_file_close_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_File_Map *map = data;

   map->common.done_cb((void*) map->common.data, &map->common);
   _eio_file_open_free(map);
}

static void
_eio_file_close_cancel(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_File_Map *map = data;

   eio_file_error(&map->common);
   _eio_file_open_free(map);
}

static void
_eio_file_map_all_job(void *data, Ecore_Thread *thread)
{
   Eio_File_Map_Rule *map = data;

   eio_file_container_set(&map->common, map->file);
   map->result = eina_file_map_all(map->common.container, map->rule);
   if (map->result && map->filter_cb)
     {
        if (!map->filter_cb((void*) map->common.data,
                            &map->common,
                            map->result,
			    map->length))
          {
             eina_file_map_free(map->common.container, map->result);
             map->result = NULL;
          }
     }

   if (!map->result)
     eio_file_thread_error(&map->common, thread);
}

static void
_eio_file_map_new_job(void *data, Ecore_Thread *thread)
{
   Eio_File_Map_Rule *map = data;

   eio_file_container_set(&map->common, map->file);
   map->result = eina_file_map_new(map->common.container, map->rule,
                                   map->offset, map->length);
   if (map->result && map->filter_cb)
     {
        if (!map->filter_cb((void*) map->common.data,
                            &map->common,
                            map->result,
			    map->length))
          {
             eina_file_map_free(map->common.container, map->result);
             map->result = NULL;
          }
     }

   if (!map->result)
     eio_file_thread_error(&map->common, thread);
}

static void
_eio_file_map_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_File_Map_Rule *map = data;

   map->map_cb((void*) map->common.data, &map->common, map->result, map->length);
   eio_file_free((Eio_File*)map);
}

static void
_eio_file_map_cancel(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_File_Map_Rule *map = data;

   eio_file_error(&map->common);
   eio_file_free((Eio_File*)map);
}

/**
 * @endcond
 */


/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */


/**
 * @endcond
 */

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

EIO_API Eio_File *
eio_file_open(const char *name, Eina_Bool shared,
	      Eio_Open_Cb open_cb,
	      Eio_Error_Cb error_cb,
	      const void *data)
{
   Eio_File_Map *map;

   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(open_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   map = malloc(sizeof (Eio_File_Map));
   EINA_SAFETY_ON_NULL_RETURN_VAL(map, NULL);

   map->open_cb = open_cb;
   map->name = eina_stringshare_add(name);
   map->shared = shared;
   map->result = NULL;

   if (!eio_file_set(&map->common,
                     NULL,
                     error_cb,
                     data,
                     _eio_file_open_job,
                     _eio_file_open_end,
                     _eio_file_open_cancel))
     return NULL;

   return &map->common;
}

EIO_API Eio_File *
eio_file_close(Eina_File *f,
               Eio_Done_Cb done_cb,
               Eio_Error_Cb error_cb,
               const void *data)
{
   Eio_File_Map *map;

   EINA_SAFETY_ON_NULL_RETURN_VAL(f, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   map = malloc(sizeof (Eio_File_Map));
   EINA_SAFETY_ON_NULL_RETURN_VAL(map, NULL);

   map->name = NULL;
   map->result = f;

   if (!eio_file_set(&map->common,
                     done_cb,
                     error_cb,
                     data,
                     _eio_file_close_job,
                     _eio_file_close_end,
                     _eio_file_close_cancel))
     return NULL;

   return &map->common;
}

EIO_API Eio_File *
eio_file_map_all(Eina_File *f,
                 Eina_File_Populate rule,
                 Eio_Filter_Map_Cb filter_cb,
                 Eio_Map_Cb map_cb,
                 Eio_Error_Cb error_cb,
                 const void *data)
{
   Eio_File_Map_Rule *map;

   EINA_SAFETY_ON_NULL_RETURN_VAL(f, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(map_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   map = malloc(sizeof (Eio_File_Map_Rule));
   EINA_SAFETY_ON_NULL_RETURN_VAL(map, NULL);

   map->file = f;
   map->filter_cb = filter_cb;
   map->map_cb = map_cb;
   map->rule = rule;
   map->result = NULL;
   map->length = eina_file_size_get(f);

   if (!eio_file_set(&map->common,
                     NULL,
                     error_cb,
                     data,
                     _eio_file_map_all_job,
                     _eio_file_map_end,
                     _eio_file_map_cancel))
     return NULL;

   return &map->common;
}

EIO_API Eio_File *
eio_file_map_new(Eina_File *f,
                 Eina_File_Populate rule,
                 unsigned long int offset,
                 unsigned long int length,
                 Eio_Filter_Map_Cb filter_cb,
                 Eio_Map_Cb map_cb,
                 Eio_Error_Cb error_cb,
                 const void *data)
{
   Eio_File_Map_Rule *map;

   EINA_SAFETY_ON_NULL_RETURN_VAL(f, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(map_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   map = malloc(sizeof (Eio_File_Map_Rule));
   EINA_SAFETY_ON_NULL_RETURN_VAL(map, NULL);

   map->file = f;
   map->filter_cb = filter_cb;
   map->map_cb = map_cb;
   map->rule = rule;
   map->result = NULL;
   map->offset = offset;
   map->length = length;

   if (!eio_file_set(&map->common,
                     NULL,
                     error_cb,
                     data,
                     _eio_file_map_new_job,
                     _eio_file_map_end,
                     _eio_file_map_cancel))
     return NULL;

   return &map->common;
}
