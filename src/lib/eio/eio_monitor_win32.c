/* EIO - EFL data type library
 * Copyright (C) 2011 Enlightenment Developers:
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
#include "Eio.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

typedef struct _Eio_Monitor_Win32_Watcher Eio_Monitor_Win32_Watcher;

/* 4096 = 256 * sizeof(FILE_NOTIFY_INFORMATION) */
# define EIO_MONITOR_WIN32_BUFFER_SIZE 4096

struct _Eio_Monitor_Win32_Watcher
{
   char                 buffer[EIO_MONITOR_WIN32_BUFFER_SIZE];
   OVERLAPPED           overlapped;
   HANDLE               handle;
   HANDLE               event;
   Eio_Monitor         *monitor;
   Ecore_Win32_Handler *h;
   Eina_Stringshare    *current;
   Eina_Stringshare    *file;
   DWORD                buf_length;
   Eina_Bool            monitor_file : 1;
   Eina_Bool            monitor_parent : 1;
};

struct _Eio_Monitor_Backend
{
   Eio_Monitor               *parent;
   Eio_Monitor_Win32_Watcher *watcher_file;
   Eio_Monitor_Win32_Watcher *watcher_dir;
   Eio_Monitor_Win32_Watcher *watcher_parent;
};

static Eina_Bool _eio_monitor_win32_native = EINA_FALSE;

static Eina_Bool
_eio_monitor_win32_cb(void *data, Ecore_Win32_Handler *wh EINA_UNUSED)
{
   PFILE_NOTIFY_INFORMATION   fni;
   Eio_Monitor_Win32_Watcher *w;
   wchar_t                   *wname;
   char                      *name;
   DWORD                      filter;
   DWORD                      offset;
   DWORD                      buf_length;
   int                        event;

   w = (Eio_Monitor_Win32_Watcher *)data;

   if (!GetOverlappedResult(w->handle, &w->overlapped, &buf_length, TRUE))
     return ECORE_CALLBACK_RENEW;

   fni = (PFILE_NOTIFY_INFORMATION)w->buffer;
   do {
      if (!fni)
        break;
      offset = fni->NextEntryOffset;

      wname = (wchar_t *)malloc(fni->FileNameLength + sizeof(wchar_t));
      if (!wname)
        return 0;

      memcpy(wname, fni->FileName, fni->FileNameLength);
      wname[fni->FileNameLength / sizeof(wchar_t)] = 0;
      name = evil_wchar_to_char(wname);
      free(wname);
      if (!name)
        return ECORE_CALLBACK_CANCEL;

      event = -1;
      switch (fni->Action)
        {
        case FILE_ACTION_ADDED:
          if (!w->monitor_parent)
            {
               if (w->monitor_file)
                 event = EIO_MONITOR_FILE_CREATED;
               else
                 event = EIO_MONITOR_DIRECTORY_CREATED;
            }
          break;
        case FILE_ACTION_REMOVED:
          if (w->monitor_parent)
            {
               char path[MAX_PATH];
               char *res;

               res = _fullpath(path, name, MAX_PATH);
               if (res && (strcmp(res, w->current) == 0))
                 event = EIO_MONITOR_SELF_DELETED;
            }
          else
            {
               if (w->monitor_file)
                 event = EIO_MONITOR_FILE_DELETED;
               else
                 event = EIO_MONITOR_DIRECTORY_DELETED;
            }
          break;
        case FILE_ACTION_MODIFIED:
          if (!w->monitor_parent)
            {
               if (w->monitor_file)
                 event = EIO_MONITOR_FILE_MODIFIED;
               else
                 event = EIO_MONITOR_DIRECTORY_MODIFIED;
            }
          break;
        case FILE_ACTION_RENAMED_OLD_NAME:
          if (!w->monitor_parent)
            {
               if (w->monitor_file)
                 event = EIO_MONITOR_FILE_DELETED;
               else
                 event = EIO_MONITOR_DIRECTORY_DELETED;
            }
          break;
        case FILE_ACTION_RENAMED_NEW_NAME:
          if (!w->monitor_parent)
            {
               if (w->monitor_file)
                 event = EIO_MONITOR_FILE_CREATED;
               else
                 event = EIO_MONITOR_DIRECTORY_CREATED;
            }
          break;
        default:
          ERR("unknown event");
          event = EIO_MONITOR_ERROR;
          break;
        }

      if (event >= 0)
        _eio_monitor_send(w->monitor, name, event);

      free(name);

      fni = (PFILE_NOTIFY_INFORMATION)((LPBYTE)fni + offset);
   } while (offset);

   filter =
     FILE_NOTIFY_CHANGE_ATTRIBUTES |
     FILE_NOTIFY_CHANGE_SIZE |
     FILE_NOTIFY_CHANGE_LAST_WRITE |
     FILE_NOTIFY_CHANGE_LAST_ACCESS |
     FILE_NOTIFY_CHANGE_CREATION |
     FILE_NOTIFY_CHANGE_SECURITY;
   if (w->monitor_file)
     filter |= FILE_NOTIFY_CHANGE_FILE_NAME;
   else
     filter |= FILE_NOTIFY_CHANGE_DIR_NAME;

    ReadDirectoryChangesW(w->handle,
                          (LPVOID)w->buffer,
                          EIO_MONITOR_WIN32_BUFFER_SIZE,
                          FALSE,
                          filter,
                          &w->buf_length,
                          &w->overlapped,
                          NULL);

   return ECORE_CALLBACK_RENEW;
}

static Eio_Monitor_Win32_Watcher *
_eio_monitor_win32_watcher_new(Eio_Monitor      *monitor,
                               Eina_Stringshare *current,
                               Eina_Stringshare *file,
                               Eina_Bool         monitor_file,
                               Eina_Bool         monitor_parent)
{
   Eio_Monitor_Win32_Watcher *w;
   char                      *monitored;
   DWORD                      filter;

   w = (Eio_Monitor_Win32_Watcher *)calloc(1, sizeof(Eio_Monitor_Win32_Watcher));
   if (!w) return NULL;

   if (!monitor_parent)
     monitored = (char *)current;
   else
     {
        char *tmp;

        tmp = strrchr(current, '\\');
        monitored = (char *)alloca((tmp - current) + 1);
        memcpy(monitored, current, tmp - current);
        monitored[tmp - current] = '\0';
     }

   w->handle = CreateFile(monitored,
                          FILE_LIST_DIRECTORY,
                          FILE_SHARE_READ |
                          FILE_SHARE_WRITE,
                          NULL,
                          OPEN_EXISTING,
                          FILE_FLAG_BACKUP_SEMANTICS |
                          FILE_FLAG_OVERLAPPED,
                          NULL);
   if (w->handle == INVALID_HANDLE_VALUE)
     goto free_w;

   w->event = CreateEvent(NULL, FALSE, FALSE, NULL);
   if (!w->event)
     goto close_handle;

   ZeroMemory (&w->overlapped, sizeof(w->overlapped));
   w->overlapped.hEvent = w->event;

   filter =
     FILE_NOTIFY_CHANGE_ATTRIBUTES |
     FILE_NOTIFY_CHANGE_SIZE |
     FILE_NOTIFY_CHANGE_LAST_WRITE |
     FILE_NOTIFY_CHANGE_LAST_ACCESS |
     FILE_NOTIFY_CHANGE_CREATION |
     FILE_NOTIFY_CHANGE_SECURITY;
   if (monitor_file)
     filter |= FILE_NOTIFY_CHANGE_FILE_NAME;
   else
     filter |= FILE_NOTIFY_CHANGE_DIR_NAME;

   if (!ReadDirectoryChangesW(w->handle,
                              (LPVOID)w->buffer,
                              EIO_MONITOR_WIN32_BUFFER_SIZE,
                              FALSE,
                              filter,
                              &w->buf_length,
                              &w->overlapped,
                              NULL))
     {
        char *msg;

        msg = evil_last_error_get();
        if (msg)
          {
             ERR("%s", msg);
             free(msg);
          }
        goto close_event;
     }

   w->h = ecore_main_win32_handler_add(w->event,
                                       _eio_monitor_win32_cb,
                                       w);
   if (!w->h)
     goto close_event;

   w->monitor = monitor;
   w->monitor_file = monitor_file;
   w->monitor_parent = monitor_parent;
   w->file = eina_stringshare_ref(file);
   w->current = eina_stringshare_ref(current);

   return w;

 close_event:
   CloseHandle(w->event);
 close_handle:
   CloseHandle(w->handle);
 free_w:
   free(w);

   return NULL;
}

static void
_eio_monitor_win32_watcher_free(Eio_Monitor_Win32_Watcher *w)
{
   if (!w) return;

   eina_stringshare_del(w->file);
   eina_stringshare_del(w->current);
   CloseHandle(w->event);
   CloseHandle (w->handle);
   free (w);
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

void eio_monitor_backend_init(void)
{
}

void eio_monitor_backend_shutdown(void)
{
}

void eio_monitor_backend_add(Eio_Monitor *monitor)
{
   char path[PATH_MAX];
   struct _stat s;
   char *res;
   Eina_Stringshare *current;
   Eina_Stringshare *file = NULL;
   Eio_Monitor_Backend *backend;
   int ret;

   res = _fullpath(path, monitor->path, MAX_PATH);
   if (!res)
     goto fallback;

   ret = _stat(res, &s);
   if (ret != 0)
     goto fallback;

   if (_S_IFDIR & s.st_mode)
     {
        current = eina_stringshare_add(path);
        if (!current)
          goto fallback;
     }
   else if (_S_IFREG & s.st_mode)
     {
        char *tmp;

        tmp = strrchr(path, '\\');
        file = eina_stringshare_add(tmp + 1);
        if (!file)
          goto fallback;

        *tmp = '\0';
        current = eina_stringshare_add(path);
        if (!current)
          {
             eina_stringshare_del(file);
             goto fallback;
          }
     }
   else
     goto fallback;

   backend = calloc(1, sizeof (Eio_Monitor_Backend));
   if (!backend)
     goto fallback;

   backend->parent = monitor;

   backend->watcher_file = _eio_monitor_win32_watcher_new(monitor, current, file, EINA_TRUE, EINA_FALSE);
   if (!backend->watcher_file)
     goto free_backend;

   backend->watcher_dir = _eio_monitor_win32_watcher_new(monitor, current, file, EINA_FALSE, EINA_FALSE);
   if (!backend->watcher_dir)
     goto free_backend_file;

   backend->watcher_parent = _eio_monitor_win32_watcher_new(monitor, current, file, EINA_FALSE, EINA_TRUE);
   if (!backend->watcher_parent)
     goto free_backend_dir;

   _eio_monitor_win32_native = EINA_TRUE;
   monitor->backend = backend;

   eina_stringshare_del(current);
   eina_stringshare_del(file);

   return;

 free_backend_dir:
   _eio_monitor_win32_watcher_free(backend->watcher_dir);
 free_backend_file:
   _eio_monitor_win32_watcher_free(backend->watcher_file);
 free_backend:
   free(backend);
 fallback:
   INF("falling back to poll monitoring");
   eio_monitor_fallback_add(monitor);
}

void eio_monitor_backend_del(Eio_Monitor *monitor)
{
   if (!_eio_monitor_win32_native)
     {
        eio_monitor_fallback_del(monitor);
        return;
     }

   _eio_monitor_win32_watcher_free(monitor->backend->watcher_parent);
   _eio_monitor_win32_watcher_free(monitor->backend->watcher_dir);
   _eio_monitor_win32_watcher_free(monitor->backend->watcher_file);
   free(monitor->backend);
   monitor->backend = NULL;
}


/**
 * @endcond
 */


/*============================================================================*
 *                                   API                                      *
 *============================================================================*/
