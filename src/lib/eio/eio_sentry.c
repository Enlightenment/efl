/* EIO - EFL data type library
 * Copyright (C) 2016 Enlightenment Developers:
 *           Lauro Moura <lauromoura@expertisesolutions.com.br>
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EIO_SENTRY_BETA 1

#include <Eo.h>
#include "Ecore.h"
#include "Eio.h"
#include "eio_sentry_private.h"


static const Efl_Event_Description*
_translate_event(int input_event)
{
   if (input_event == EIO_MONITOR_FILE_CREATED)
     return EIO_SENTRY_EVENT_FILE_CREATED;
   else if (input_event == EIO_MONITOR_FILE_DELETED)
     return EIO_SENTRY_EVENT_FILE_DELETED;
   else if (input_event == EIO_MONITOR_FILE_MODIFIED)
     return EIO_SENTRY_EVENT_FILE_MODIFIED;
   else if (input_event == EIO_MONITOR_FILE_CLOSED)
     return EIO_SENTRY_EVENT_FILE_CLOSED;
   else if (input_event == EIO_MONITOR_DIRECTORY_CREATED)
     return EIO_SENTRY_EVENT_DIRECTORY_CREATED;
   else if (input_event == EIO_MONITOR_DIRECTORY_DELETED)
     return EIO_SENTRY_EVENT_DIRECTORY_DELETED;
   else if (input_event == EIO_MONITOR_DIRECTORY_MODIFIED)
     return EIO_SENTRY_EVENT_DIRECTORY_MODIFIED;
   else if (input_event == EIO_MONITOR_DIRECTORY_CLOSED)
     return EIO_SENTRY_EVENT_DIRECTORY_CLOSED;
   else if (input_event == EIO_MONITOR_SELF_RENAME)
     return EIO_SENTRY_EVENT_SELF_RENAME;
   else if (input_event == EIO_MONITOR_SELF_DELETED)
     return EIO_SENTRY_EVENT_SELF_DELETED;
   else if (input_event == EIO_MONITOR_ERROR)
     return EIO_SENTRY_EVENT_ERROR;
   else
     return NULL;
}

static unsigned char
_handle_event(void *data, int type, void *event)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, ECORE_CALLBACK_PASS_ON);
   EINA_SAFETY_ON_NULL_RETURN_VAL(event, ECORE_CALLBACK_PASS_ON);

   const Efl_Event_Description* translated_event = _translate_event(type);
   Eio_Sentry_Data *pd = (Eio_Sentry_Data *)data;
   Eio_Monitor_Event *monitor_event = (Eio_Monitor_Event *)event;

   Eio_Sentry_Event *event_info = malloc(sizeof(Eio_Sentry_Event));
   EINA_SAFETY_ON_NULL_RETURN_VAL(event_info, ECORE_CALLBACK_PASS_ON);

   event_info->source = eio_monitor_path_get(monitor_event->monitor);
   event_info->trigger = monitor_event->filename;

   efl_event_callback_call(pd->object, translated_event, event_info);

   // If event was error, we must delete the monitor.
   if (type == EIO_MONITOR_ERROR)
     eina_hash_del(pd->targets, event_info->source, NULL);

   free(event_info);

   return ECORE_CALLBACK_PASS_ON;
}

static void
_initialize_handlers(Eio_Sentry_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN(pd);

   ecore_event_handler_add(EIO_MONITOR_FILE_CREATED, _handle_event, pd);
   ecore_event_handler_add(EIO_MONITOR_FILE_DELETED, _handle_event, pd);
   ecore_event_handler_add(EIO_MONITOR_FILE_MODIFIED, _handle_event, pd);
   ecore_event_handler_add(EIO_MONITOR_FILE_CLOSED, _handle_event, pd);

   ecore_event_handler_add(EIO_MONITOR_DIRECTORY_CREATED, _handle_event, pd);
   ecore_event_handler_add(EIO_MONITOR_DIRECTORY_DELETED, _handle_event, pd);
   ecore_event_handler_add(EIO_MONITOR_DIRECTORY_MODIFIED, _handle_event, pd);
   ecore_event_handler_add(EIO_MONITOR_DIRECTORY_CLOSED, _handle_event, pd);

   ecore_event_handler_add(EIO_MONITOR_SELF_RENAME, _handle_event, pd);
   ecore_event_handler_add(EIO_MONITOR_SELF_DELETED, _handle_event, pd);
   ecore_event_handler_add(EIO_MONITOR_ERROR, _handle_event, pd);

   pd->handlers_initialized = EINA_TRUE;
}

Eina_Bool
_eio_sentry_add(Eo *obj EINA_UNUSED, Eio_Sentry_Data *pd, const char *path)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(path, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, EINA_FALSE);

   if (!pd->handlers_initialized)
     _initialize_handlers(pd);

   if (eina_hash_find(pd->targets, path))
     return EINA_TRUE;

   Eio_Monitor *monitor = eio_monitor_add(path);

   if (!monitor)
     {
        EINA_LOG_ERR("Failed to create monitor.");
        return EINA_FALSE;
     }

   if (!eina_hash_add(pd->targets, path, monitor))
     {
        EINA_LOG_ERR("Failed to register monitor.");
        eio_monitor_del(monitor);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

void
_eio_sentry_remove(Eo *obj EINA_UNUSED, Eio_Sentry_Data *pd, const char *path)
{
   EINA_SAFETY_ON_NULL_RETURN(path);
   EINA_SAFETY_ON_NULL_RETURN(pd);

   eina_hash_del(pd->targets, path, NULL);
}

Eina_Bool
_eio_sentry_fallback_check(const Eo *obj EINA_UNUSED, Eio_Sentry_Data *pd, const char *path)
{
   Eio_Monitor *monitor;

   EINA_SAFETY_ON_NULL_RETURN_VAL(path, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, EINA_FALSE);

   monitor = eina_hash_find(pd->targets, path);
   EINA_SAFETY_ON_NULL_RETURN_VAL(monitor, EINA_FALSE);
   return eio_monitor_fallback_check(monitor);
}

Efl_Object * _eio_sentry_efl_object_constructor(Eo *obj, Eio_Sentry_Data *pd)
{
   obj = efl_constructor(efl_super(obj, EIO_SENTRY_CLASS));

   pd->object = obj;
   pd->targets = eina_hash_string_small_new((Eina_Free_Cb)&eio_monitor_del);
   pd->handlers_initialized = EINA_FALSE;

   return obj;
}

void _eio_sentry_efl_object_destructor(Eo *obj, Eio_Sentry_Data *pd)
{
   eina_hash_free(pd->targets);

   efl_destructor(efl_super(obj, EIO_SENTRY_CLASS));
}

#include "eio_sentry.eo.c"
