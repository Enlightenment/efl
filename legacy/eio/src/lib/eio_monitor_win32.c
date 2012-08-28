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
   DWORD                buf_length;
   int                  is_dir;
};

struct _Eio_Monitor_Backend
{
   Eio_Monitor               *parent;

   Eio_Monitor_Win32_Watcher *file;
   Eio_Monitor_Win32_Watcher *dir;
};

static Eina_Bool _eio_monitor_win32_native = EINA_FALSE;

static Eina_Bool
_eio_monitor_win32_cb(void *data, Ecore_Win32_Handler *wh __UNUSED__)
{
   char                       filename[PATH_MAX];
   PFILE_NOTIFY_INFORMATION   fni;
   Eio_Monitor_Win32_Watcher *w;
   wchar_t                   *wname;
   char                      *name;
   DWORD                      filter;
   DWORD                      offset;
   DWORD                      buf_length;
   int                        event = EIO_MONITOR_ERROR;

   w = (Eio_Monitor_Win32_Watcher *)data;

   if (!GetOverlappedResult(w->handle, &w->overlapped, &buf_length, TRUE))
     return ECORE_CALLBACK_RENEW;

   fni = (PFILE_NOTIFY_INFORMATION)w->buffer;
   do {
      if (!fni)
        break;
      offset = fni->NextEntryOffset;

      wname = (wchar_t *)malloc(sizeof(wchar_t) * (fni->FileNameLength + 1));
      if (!wname)
        return 0;

      memcpy(wname, fni->FileName, fni->FileNameLength);
      wname[fni->FileNameLength]='\0';
      name = evil_wchar_to_char(wname);
      free(wname);
      if (!name)
        return ECORE_CALLBACK_CANCEL;

      _snprintf(filename, PATH_MAX, "%s\\%s", w->monitor->path, name);
      free(name);

      name = filename;
      while (*name)
        {
          if (*name == '/') *name = '\\';
          name++;
        }

      switch (fni->Action)
        {
        case FILE_ACTION_ADDED:
          if (w->is_dir)
            event = EIO_MONITOR_DIRECTORY_CREATED;
          else
            event = EIO_MONITOR_FILE_CREATED;
          break;
        case FILE_ACTION_REMOVED:
          if (w->is_dir)
            event = EIO_MONITOR_DIRECTORY_DELETED;
          else
            event = EIO_MONITOR_FILE_DELETED;
          break;
        case FILE_ACTION_MODIFIED:
          if (!w->is_dir)
            event = EIO_MONITOR_FILE_MODIFIED;
          break;
        case FILE_ACTION_RENAMED_OLD_NAME:
          if (w->is_dir)
            event = EIO_MONITOR_DIRECTORY_DELETED;
          else
            event = EIO_MONITOR_FILE_DELETED;
          break;
        case FILE_ACTION_RENAMED_NEW_NAME:
          if (w->is_dir)
            event = EIO_MONITOR_DIRECTORY_CREATED;
          else
            event = EIO_MONITOR_FILE_CREATED;
          break;
        default:
          fprintf(stderr, "unknown event\n");
          event = EIO_MONITOR_ERROR;
          break;
        }
      if (event != EIO_MONITOR_ERROR)
        _eio_monitor_send(w->monitor, filename, event);

      fni = (PFILE_NOTIFY_INFORMATION)((LPBYTE)fni + offset);
   } while (offset);

   filter = (w->is_dir == 0) ? FILE_NOTIFY_CHANGE_FILE_NAME : FILE_NOTIFY_CHANGE_DIR_NAME;
   filter |=
     FILE_NOTIFY_CHANGE_ATTRIBUTES |
     FILE_NOTIFY_CHANGE_SIZE |
     FILE_NOTIFY_CHANGE_LAST_WRITE |
     FILE_NOTIFY_CHANGE_LAST_ACCESS |
     FILE_NOTIFY_CHANGE_CREATION |
     FILE_NOTIFY_CHANGE_SECURITY;

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
_eio_monitor_win32_watcher_new(Eio_Monitor *monitor, unsigned char is_dir)
{
   char path[PATH_MAX];
   Eio_Monitor_Win32_Watcher *w;
   DWORD                      filter;

   w = (Eio_Monitor_Win32_Watcher *)calloc(1, sizeof(Eio_Monitor_Win32_Watcher));
   if (!w) return NULL;

   realpath(monitor->path, path);
   w->handle = CreateFile(path,
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

   filter = (is_dir == 0) ? FILE_NOTIFY_CHANGE_FILE_NAME : FILE_NOTIFY_CHANGE_DIR_NAME;
   filter |=
     FILE_NOTIFY_CHANGE_ATTRIBUTES |
     FILE_NOTIFY_CHANGE_SIZE |
     FILE_NOTIFY_CHANGE_LAST_WRITE |
     FILE_NOTIFY_CHANGE_LAST_ACCESS |
     FILE_NOTIFY_CHANGE_CREATION |
     FILE_NOTIFY_CHANGE_SECURITY;

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
             ERR("%s\n", msg);
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
   w->is_dir = is_dir;

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
   Eio_Monitor_Backend *backend;

   backend = calloc(1, sizeof (Eio_Monitor_Backend));
   if (!backend)
     {
        eio_monitor_fallback_add(monitor);
        return;
     }

   backend->parent = monitor;
   backend->file = _eio_monitor_win32_watcher_new(monitor, 0);
   if (!backend->file)
     {
        INF("falling back to poll monitoring");
        free(backend);
        eio_monitor_fallback_add(monitor);
        return;
     }

   backend->dir = _eio_monitor_win32_watcher_new(monitor, 1);
   if (!backend->dir)
     {
        INF("falling back to poll monitoring");
        _eio_monitor_win32_watcher_free(backend->file);
        free(backend);
        eio_monitor_fallback_add(monitor);
        return;
     }

   _eio_monitor_win32_native = EINA_TRUE;
   monitor->backend = backend;
}

void eio_monitor_backend_del(Eio_Monitor *monitor)
{
   if (!_eio_monitor_win32_native)
     {
        eio_monitor_fallback_del(monitor);
        return ;
     }

   _eio_monitor_win32_watcher_free(monitor->backend->file);
   _eio_monitor_win32_watcher_free(monitor->backend->dir);
   free(monitor->backend);
   monitor->backend = NULL;
}


/**
 * @endcond
 */


/*============================================================================*
 *                                   API                                      *
 *============================================================================*/
