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
#include "Eina.h"
#include "Ecore_File.h"
#include <sys/types.h>
#include <sys/event.h>
#include <sys/stat.h>

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

#define KEVENT_NUM_EVENTS 5

typedef struct _Eio_File_Info
{
  unsigned int      file_off;
  unsigned int      mode;
  unsigned long int ino, mtime;
} Eio_File_Info;

typedef struct _Eio_Ls
{
  Eio_File_Info *info;
  char          *strings;
} Eio_Ls;

struct _Eio_Monitor_Backend
{
  Eio_Monitor *parent;
  Eio_Ls       prev_ls;
  int          fd;
};

static Ecore_Fd_Handler *_kqueue_fd = NULL;
static Eina_Hash *_kevent_monitors = NULL;

static void
_eio_kevent_ls_free(Eio_Ls ls)
{
  free(ls.info);
  free(ls.strings);
  ls.info = NULL;
  ls.strings = NULL;
}

static void
_eio_kevent_del(void *data)
{
  Eio_Monitor_Backend *emb = data;

  _eio_kevent_ls_free(emb->prev_ls);
  if (emb->fd >= 0) close(emb->fd);
  free(emb);
}

static Eio_Ls
_eio_kevent_ls(const char *directory)
{
  Eina_Iterator *it;
  Eina_File_Direct_Info *info;
  Eio_Ls ls = { NULL, NULL }, ls2;
  unsigned int file_num = 0;
  unsigned int strings_size = 0;
  unsigned int strings_size2 = 0;

  it = eina_file_direct_ls(directory);
  if (!it) return ls;

  ls.info = calloc(1, sizeof(Eio_File_Info));
  if (!ls.info) goto done;
  file_num = 1;
  ls.strings = calloc(1, 1);
  if (!ls.strings)
    {
      free(ls.info);
      goto done;
    }
  strings_size = 1;
  EINA_ITERATOR_FOREACH(it, info)
    {
      Eina_Stat st;
      const char *file = info->path + info->name_start;

      if (eina_file_statat(eina_iterator_container_get(it), info, &st))
        continue;
      file_num++;
      strings_size2 = strings_size + strlen(file) + 1;
      ls2.info = realloc(ls.info, file_num * sizeof(Eio_File_Info));
      ls2.strings = realloc(ls.strings, strings_size2);
      if ((strings_size2 >= (2UL * 1024 * 1024 * 1024)/*2GB*/) || // too big
          (!ls2.info) || (!ls2.strings))
        {
          if      (ls2.info)    free(ls2.info);
          else if (ls.info)     free(ls.info);
          if      (ls2.strings) free(ls2.strings);
          else if (ls.strings)  free(ls.strings);
          ls.info    = NULL;
          ls.strings = NULL;
          goto done;
        }
      ls.info = ls2.info;
      ls.strings = ls2.strings;
      ls.info[file_num - 2].file_off = strings_size;
      ls.info[file_num - 2].mode = st.mode;
      ls.info[file_num - 2].ino = st.ino;
      ls.info[file_num - 2].mtime = st.mtime;
      ls.info[file_num - 1].file_off = 0; // item with file_off == 0 == end
      ls.info[file_num - 1].mode = 0;
      ls.info[file_num - 1].ino = 0;
      ls.info[file_num - 1].mtime = 0;
      strcpy(ls.strings + strings_size, file); // already allocd right size
      strings_size = strings_size2;
    }
done:
  eina_iterator_free(it);
  return ls;
}

static void
_eio_kevent_event_find(Eio_Monitor_Backend *backend)
{
  Eio_File_Info *file, *file2;
  Eio_Ls next_ls = _eio_kevent_ls(backend->parent->path);
  char buf[PATH_MAX];

  if (backend->prev_ls.info)
    {
      for (file = backend->prev_ls.info; file->file_off; file++)
        {
          Eina_Bool exists = EINA_FALSE;
          if (next_ls.info)
            {
              for (file2 = next_ls.info; file2->file_off; file2++)
                {
                  if (file->ino == file2->ino)
                    {
                      if (!strcmp(backend->prev_ls.strings + file->file_off,
                                  next_ls.strings + file2->file_off))
                        exists = EINA_TRUE;
                      if (file->mtime != file2->mtime)
                        {
                          snprintf(buf, sizeof(buf), "%s/%s",
                                   backend->parent->path,
                                   backend->prev_ls.strings + file->file_off);
                          if (S_ISDIR(file->mode))
                            _eio_monitor_send(backend->parent, buf,
                                              EIO_MONITOR_DIRECTORY_MODIFIED);
                          else
                            _eio_monitor_send(backend->parent, buf,
                                              EIO_MONITOR_FILE_MODIFIED);
                        }
                    }
                }
            }
          if (!exists)
            {
              snprintf(buf, sizeof(buf), "%s/%s",
                       backend->parent->path,
                       backend->prev_ls.strings + file->file_off);
              if (S_ISDIR(file->mode))
                _eio_monitor_send(backend->parent, buf,
                                  EIO_MONITOR_DIRECTORY_DELETED);
              else
                _eio_monitor_send(backend->parent, buf,
                                  EIO_MONITOR_FILE_DELETED);
            }
        }
    }

  if (next_ls.info)
    {
      for (file2 = next_ls.info; file2->file_off; file2++)
        {
          Eina_Bool exists = EINA_FALSE;

          if (backend->prev_ls.info)
            {
              for (file = backend->prev_ls.info; file->file_off; file++)
                {
                  if ((!strcmp(backend->prev_ls.strings + file->file_off,
                               next_ls.strings + file2->file_off)) &&
                      (file->ino == file2->ino))
                    {
                      exists = EINA_TRUE;
                      break;
                    }
                }
            }
          if (!exists)
            {
              snprintf(buf, sizeof(buf), "%s/%s",
                       backend->parent->path,
                       next_ls.strings + file2->file_off);
              if (S_ISDIR(file2->mode))
                _eio_monitor_send(backend->parent, buf,
                                  EIO_MONITOR_DIRECTORY_CREATED);
              else
                _eio_monitor_send(backend->parent, buf,
                                  EIO_MONITOR_FILE_CREATED);
            }
        }
    }

   _eio_kevent_ls_free(backend->prev_ls);
   backend->prev_ls = next_ls;
}

static Eina_Bool
_eio_kevent_handler(void *data EINA_UNUSED, Ecore_Fd_Handler *fdh)
{
   Eio_Monitor_Backend *backend;
   struct kevent evs[KEVENT_NUM_EVENTS];
   int event_code = 0;
   const struct timespec timeout = { 0, 0 };
   int res = kevent(ecore_main_fd_handler_fd_get(fdh), 0, 0, evs,
                    KEVENT_NUM_EVENTS, &timeout);

   for (int i = 0; i < res; i++)
     {
        backend = eina_hash_find(_kevent_monitors, &evs[i].ident);
        if (evs[i].fflags & NOTE_DELETE)
          {
             event_code = EIO_MONITOR_SELF_DELETED;
             _eio_monitor_send(backend->parent, backend->parent->path, event_code);
          }
        if (evs[i].fflags & NOTE_WRITE || evs[i].fflags & NOTE_ATTRIB)
          {
             /* Handle directory/file creation and deletion */
             if (ecore_file_is_dir(backend->parent->path))
               _eio_kevent_event_find(backend);
             else
               {
                  event_code = EIO_MONITOR_FILE_MODIFIED;
                  _eio_monitor_send(backend->parent, backend->parent->path, event_code);
               }
          }
     }

   return ECORE_CALLBACK_RENEW;
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

void
eio_monitor_backend_init(void)
{
  int fd;

  if (_kqueue_fd) return; // already initialized

  fd = kqueue();
  if (fd < 0) return;

  _kqueue_fd = ecore_main_fd_handler_add(fd, ECORE_FD_READ, _eio_kevent_handler, NULL, NULL, NULL);
  if (!_kqueue_fd)
    {
      close(fd);
      return;
    }

  _kevent_monitors = eina_hash_int32_new(_eio_kevent_del);
}

void
eio_monitor_backend_shutdown(void)
{
  int fd;

  if (!_kqueue_fd) return;

  eina_hash_free(_kevent_monitors);
  fd = ecore_main_fd_handler_fd_get(_kqueue_fd);
  ecore_main_fd_handler_del(_kqueue_fd);
  _kqueue_fd = NULL;

  if (fd < 0) return;

  close(fd);
}

void
eio_monitor_backend_add(Eio_Monitor *monitor)
{
  struct kevent e;
  struct stat st;
  Eio_Monitor_Backend* backend;
  int fd, res = 0;

  if (!_kqueue_fd) return;

  backend = calloc(1, sizeof (Eio_Monitor_Backend));
  if (!backend) return;

  res = stat(monitor->path, &st);
  if (res) goto error;

  fd = open(monitor->path, O_RDONLY);
  if (fd < 0) goto error;

  eina_file_close_on_exec(fd, EINA_TRUE);
  backend->fd = fd;
  backend->parent = monitor;
  monitor->backend = backend;

  if (ecore_file_is_dir(backend->parent->path))
    backend->prev_ls = _eio_kevent_ls(backend->parent->path);

  eina_hash_direct_add(_kevent_monitors, &backend->fd, backend);

  EV_SET(&e, fd, EVFILT_VNODE, EV_ADD | EV_CLEAR,
         NOTE_DELETE | NOTE_WRITE | NOTE_ATTRIB, 0, NULL);
  res = kevent(ecore_main_fd_handler_fd_get(_kqueue_fd), &e, 1, 0, 0, 0);
  if (res) eina_hash_del(_kevent_monitors, &backend->fd, backend);
  return;

error:
   free(backend);
}

void
eio_monitor_backend_del(Eio_Monitor *monitor)
{
  Eio_Monitor_Backend *backend;

  backend = monitor->backend;
  monitor->backend = NULL;

  eina_hash_del(_kevent_monitors, &backend->fd, backend);
}

Eina_Bool
eio_monitor_context_check(const Eio_Monitor *monitor, const char *path)
{
  Eio_Monitor_Backend *backend = monitor->backend;
  Eio_File_Info *file;
  const char *filename;

  if (!path) return EINA_FALSE;
  filename = strrchr(path, '/');
  if (!filename) return EINA_FALSE;
  filename++;
  for (file = backend->prev_ls.info; file->file_off; file++)
    {
      if (!strcmp(backend->prev_ls.strings + file->file_off, filename))
        return EINA_TRUE;
    }
  return EINA_FALSE;
}



/*============================================================================*
 *                                   API                                      *
 *============================================================================*/
