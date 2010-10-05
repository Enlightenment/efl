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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <pthread.h>

#include "eio_private.h"

#include "Eio.h"

static int _eio_count = 0;

/* Progress pool */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static Eina_Trash *trash = NULL;
static int trash_count = 0;

EAPI int
eio_init(void)
{
   _eio_count++;

   if (_eio_count > 1) return _eio_count;

   eina_init();
   ecore_init();

   return _eio_count;
}

EAPI int
eio_shutdown(void)
{
   Eio_Progress *progress;

   _eio_count--;

   if (_eio_count > 0) return _eio_count;

   /* Cleanup progress pool */
   EINA_TRASH_CLEAN(&trash, progress)
     free(progress);
   trash_count = 0;

   ecore_shutdown();
   eina_shutdown();
   return _eio_count;
}

Eio_Progress *
eio_progress_malloc(void)
{
   Eio_Progress *progress;

   pthread_mutex_lock(&lock);
   progress = eina_trash_pop(&trash);
   if (progress) trash_count--;
   pthread_mutex_unlock(&lock);

   if (!progress) progress = malloc(sizeof (Eio_Progress));
   return progress;
}

void
eio_progress_free(Eio_Progress *progress)
{
   eina_stringshare_del(progress->source);
   eina_stringshare_del(progress->dest);

   if (trash_count >= EIO_PROGRESS_LIMIT)
     {
        free(progress);
     }
   else
     {
        pthread_mutex_lock(&lock);
        eina_trash_push(&trash, progress);
        trash_count++;
        pthread_mutex_unlock(&lock);
     }
}

void
eio_progress_send(Ecore_Thread *thread, Eio_File_Progress *op, off_t current, off_t max)
{
   Eio_Progress *progress;

   if (op->progress_cb == NULL)
     return ;

   progress = eio_progress_malloc();
   if (!progress) return ;

   progress->current = current;
   progress->max = max;
   progress->percent = (float) current * 100.0 / (float) max;
   progress->source = eina_stringshare_ref(op->source);
   progress->dest = eina_stringshare_ref(op->dest);

   ecore_thread_feedback(thread, progress);
}

