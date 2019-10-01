/* EIO - EFL data type library
 * Copyright (C) 2015 Enlightenment Developers:
 *           Pierre Lamot <pierre.lamot@openwide.fr>
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

#import <CoreServices/CoreServices.h>

static CFTimeInterval _latency  = 0.1;

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

struct _Eio_Monitor_Backend
{
   Eio_Monitor *parent;
   ///the monitored path
   char *mon_path;
   ///the actual file path
   char *real_path;
};

typedef struct _FSEvent_Info FSEvent_Info;

struct _FSEvent_Info {
   char *path;
   FSEventStreamEventFlags flags;

};

typedef struct _Eio_FSEvent_Table Eio_FSEvent_Table;

struct _Eio_FSEvent_Table
{
   int mask;
   int *ev_file_code;
   int *ev_dir_code;
};

#define EIO_FSEVENT_LINE(FSe, Ef, Ed)		\
  { kFSEventStreamEventFlag##FSe, &EIO_MONITOR_##Ef, &EIO_MONITOR_##Ed }

static const Eio_FSEvent_Table match[] = {
  EIO_FSEVENT_LINE(ItemChangeOwner, FILE_MODIFIED, DIRECTORY_MODIFIED),
  EIO_FSEVENT_LINE(ItemInodeMetaMod, FILE_MODIFIED, DIRECTORY_MODIFIED),
  EIO_FSEVENT_LINE(ItemXattrMod, FILE_MODIFIED, DIRECTORY_MODIFIED),
  EIO_FSEVENT_LINE(ItemModified, FILE_MODIFIED, DIRECTORY_MODIFIED),
  EIO_FSEVENT_LINE(ItemRemoved, FILE_DELETED, DIRECTORY_DELETED),
  EIO_FSEVENT_LINE(ItemCreated, FILE_CREATED, DIRECTORY_CREATED),
  EIO_FSEVENT_LINE(RootChanged, SELF_DELETED, SELF_DELETED)
};

static FSEventStreamRef _stream = NULL;
static Eina_Hash *_fsevent_monitors = NULL;
static CFMutableArrayRef _paths_to_watch = NULL;
static dispatch_queue_t _dispatch_queue;

static Eina_Bool
_handle_fsevent_with_monitor(const Eina_Hash *hash EINA_UNUSED,
                             const void *key EINA_UNUSED,
                             void *data,
                             void *fdata)
{
   FSEvent_Info *event_info = (FSEvent_Info*)fdata;
   Eio_Monitor_Backend *backend = (Eio_Monitor_Backend*)data;
   FSEventStreamEventFlags flags = event_info->flags;
   unsigned int i;
   Eina_Bool is_dir;
   unsigned int length, tmp_length;

   char *tmp = NULL;

   if (backend->parent->delete_me)
     return 1;

   if (!eina_str_has_prefix(event_info->path, backend->real_path))
     {
        return 1;
     }

   length = strlen(event_info->path) - strlen(backend->real_path);
   if (length == 0)
     {
        tmp = strdup(backend->parent->path);
     }
   else
     {
        tmp_length =
          eina_stringshare_strlen(backend->parent->path) + length + 2;

        tmp = malloc(sizeof(char) * tmp_length);
        snprintf(tmp, tmp_length, "%s/%s",
                 backend->parent->path,
                 &(event_info->path[strlen(backend->real_path) + 1]));
     }

   is_dir = !!(flags & kFSEventStreamEventFlagItemIsDir);

   for (i = 0; i < sizeof (match) / sizeof (Eio_FSEvent_Table); ++i)
     if (match[i].mask & flags)
       {
          DBG("send event from %s with event %X\n", event_info->path, flags);
          _eio_monitor_send(backend->parent,
                            tmp,
                            is_dir ? *match[i].ev_dir_code : *match[i].ev_file_code);
       }

   free(tmp);
   //we have found the right event, no need to continue
   return 0;
}


static void
_main_loop_send_event(void *data)
{
   FSEvent_Info *info = (FSEvent_Info*)data;

   if (!_stream)
     {
        //this can happen, when eio_shutdown is called
        goto cleanup;
     }

   if ((info->flags & kFSEventStreamEventFlagKernelDropped) != 0)
     {
        _eio_monitor_send(NULL, "", EIO_MONITOR_ERROR);
        goto cleanup;
     }

   eina_hash_foreach(_fsevent_monitors,
                     _handle_fsevent_with_monitor,
                     info);

 cleanup:
   free(info->path);
   free(info);
}


static void
_eio_fsevent_cb(ConstFSEventStreamRef stream_ref EINA_UNUSED,
                void *ctx EINA_UNUSED,
                size_t count,
                void *event_paths,
                const FSEventStreamEventFlags event_flags[],
                const FSEventStreamEventId event_ids[] EINA_UNUSED
                )
{
   size_t i;
   FSEvent_Info *event_info;

   for (i = 0; i < count; i++)
     {
        event_info = malloc(sizeof(FSEvent_Info));
        event_info->path = strdup(((char**)event_paths)[i]);
        event_info->flags = event_flags[i];

        ecore_main_loop_thread_safe_call_async(_main_loop_send_event,
                                               event_info);
     }
}

static void
_eio_fsevent_del(void *data)
{
   Eio_Monitor_Backend *backend = (Eio_Monitor_Backend *)data;
   free(backend);
}

static void
_eio_get_monitor_path(const char *path, char **monpath, char **fullpath)
{
   char realPath[PATH_MAX];
   char *realPathOk;
   char *dname = NULL;
   struct stat sb;

   realPathOk = realpath(path, realPath);
   if (realPathOk == NULL)
     {
        dname = dirname((char*)path);
        if (strcmp(dname, ".") == 0)
          {
             realPathOk = realpath("./", realPath);
          }
        else
          {
             realPathOk = realpath(dname, realPath);
          }

        if (realPathOk == NULL)
          return;
     }

   if (stat(realPath, &sb) < 0)
     {
        return;
     }

   if (S_ISDIR(sb.st_mode))
     {
        if (fullpath)
          *fullpath = strdup(realPath);
        if (monpath)
          *monpath = strdup(realPath);
     }
   else
     {
        //not a directory, monitor parent
        if (fullpath)
          *fullpath = strdup(realPath);
        dname = dirname(realPath);
        if (monpath)
          *monpath = strdup(dname);
     }
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

void eio_monitor_backend_init(void)
{
   _dispatch_queue = dispatch_queue_create("org.elf.fseventqueue", NULL);
   _fsevent_monitors = eina_hash_string_small_new(_eio_fsevent_del);
   _paths_to_watch = CFArrayCreateMutable(kCFAllocatorDefault,
                                          0,
                                          &kCFTypeArrayCallBacks);
}

void eio_monitor_backend_shutdown(void)
{
   if (_stream)
     {
        FSEventStreamStop(_stream);
        FSEventStreamInvalidate(_stream);
        FSEventStreamRelease(_stream);
        _stream = NULL;
     }
   dispatch_release(_dispatch_queue);
   eina_hash_free(_fsevent_monitors);
   CFRelease(_paths_to_watch);
}


void eio_monitor_backend_add(Eio_Monitor *monitor)
{
   Eio_Monitor_Backend *backend;

   FSEventStreamEventId eventid;

   CFStringRef path = NULL;

   //the path we should monitor
   char *monitor_path = NULL;
   //the real file path
   char *real_path = NULL;

   _eio_get_monitor_path(monitor->path, &monitor_path, &real_path);

   backend = calloc(1, sizeof (Eio_Monitor_Backend));
   if (!backend)
     {
        free(monitor_path);
        eio_monitor_fallback_add(monitor);
        return;
     }

   path = CFStringCreateWithCString(NULL,
                                    monitor_path,
                                    kCFStringEncodingUTF8);

   CFArrayAppendValue(_paths_to_watch, path);

   if (_stream)
     {
        eventid = FSEventStreamGetLatestEventId(_stream);
        FSEventStreamRelease(_stream);
        _stream = NULL;
     }
   else
     {
        eventid = kFSEventStreamEventIdSinceNow;
     }

   _stream = FSEventStreamCreate(NULL,
                                 _eio_fsevent_cb,
                                 NULL,
                                 _paths_to_watch,
                                 eventid,
                                 _latency,
                                 kFSEventStreamCreateFlagFileEvents
                                 | kFSEventStreamCreateFlagNoDefer
                                );

   if (!_stream)
     {
        free(monitor_path);
        free(backend);
        eio_monitor_fallback_add(monitor);
        return;
     }

   backend->parent = monitor;
   backend->mon_path = monitor_path;
   backend->real_path = real_path;
   monitor->backend = backend;

   eina_hash_direct_add(_fsevent_monitors, monitor->path, backend);

   FSEventStreamSetDispatchQueue(_stream, _dispatch_queue);
   FSEventStreamStart(_stream);


}

void eio_monitor_backend_del(Eio_Monitor *monitor)
{
   Eio_Monitor_Backend *backend;
   CFStringRef path = NULL;
   FSEventStreamEventId eventid;
   char *monitor_path;

   if (!_stream)
     {
        eio_monitor_fallback_del(monitor);
        return;
     }

   _eio_get_monitor_path(monitor->path, &monitor_path, NULL);

   eventid = FSEventStreamGetLatestEventId(_stream);
   FSEventStreamRelease(_stream);
   _stream = NULL;

   path = CFStringCreateWithCString(NULL,
                                    monitor_path,
                                    kCFStringEncodingUTF8);

   CFIndex idx =
     CFArrayGetFirstIndexOfValue(_paths_to_watch,
                                 CFRangeMake(0,
                                             CFArrayGetCount(_paths_to_watch)
                                             ),
                                 path);

   if (idx != -1)
     {
        CFArrayRemoveValueAtIndex(_paths_to_watch, idx);
     }

   if (CFArrayGetCount(_paths_to_watch) > 0)
     {
        _stream = FSEventStreamCreate(NULL,
                                      _eio_fsevent_cb,
                                      NULL,
                                      _paths_to_watch,
                                      eventid,
                                      _latency,
                                      kFSEventStreamCreateFlagFileEvents
                                      | kFSEventStreamCreateFlagNoDefer
                                      );
     }
   backend = monitor->backend;
   monitor->backend = NULL;
   if (!backend) return;

   eina_hash_del(_fsevent_monitors, monitor->path, backend);
}

Eina_Bool eio_monitor_context_check(const Eio_Monitor *monitor, const char *path)
{
   return EINA_TRUE;
}

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/
