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

#ifdef _WIN32
# include <evil_private.h> /* mkdir */
#endif

#include "eio_private.h"
#include "Eio.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

static void
_eio_file_mkdir(void *data, Ecore_Thread *thread)
{
   Eio_File_Mkdir *m = data;

   if (mkdir(m->path, m->mode) != 0)
     eio_file_thread_error(&m->common, thread);
}

static void
_eio_mkdir_free(Eio_File_Mkdir *m)
{
   eina_stringshare_del(m->path);
   eio_file_free(&m->common);
}

static void
_eio_file_mkdir_done(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_File_Mkdir *m = data;

   if (m->common.done_cb)
     m->common.done_cb((void*) m->common.data, &m->common);

   _eio_mkdir_free(m);
}

static void
_eio_file_mkdir_error(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_File_Mkdir *m = data;

   eio_file_error(&m->common);
   _eio_mkdir_free(m);
}

static void
_eio_file_unlink(void *data, Ecore_Thread *thread)
{
   Eio_File_Unlink *l = data;

   if (unlink(l->path) != 0)
     eio_file_thread_error(&l->common, thread);
}

static void
_eio_unlink_free(Eio_File_Unlink *l)
{
   eina_stringshare_del(l->path);
   eio_file_free(&l->common);
}

static void
_eio_file_unlink_done(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_File_Unlink *l = data;

   if (l->common.done_cb)
     l->common.done_cb((void*) l->common.data, &l->common);

   _eio_unlink_free(l);
}

static void
_eio_file_unlink_error(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_File_Unlink *l = data;

   eio_file_error(&l->common);
   _eio_unlink_free(l);
}

static void
_eio_file_struct_2_eina(Eina_Stat *es, _eio_stat_t *st)
{
   es->dev = st->st_dev;
   es->ino = st->st_ino;
   es->mode = st->st_mode;
   es->nlink = st->st_nlink;
   es->uid = st->st_uid;
   es->gid = st->st_gid;
   es->rdev = st->st_rdev;
   es->size = st->st_size;
#ifdef _WIN32
   es->blksize = 0;
   es->blocks = 0;
#else
   es->blksize = st->st_blksize;
   es->blocks = st->st_blocks;
#endif
   es->atime = st->st_atime;
   es->mtime = st->st_mtime;
   es->ctime = st->st_ctime;
#ifdef _STAT_VER_LINUX
# if (defined __USE_MISC && defined st_mtime)
   es->atimensec = st->st_atim.tv_nsec;
   es->mtimensec = st->st_mtim.tv_nsec;
   es->ctimensec = st->st_ctim.tv_nsec;
# else
   es->atimensec = st->st_atimensec;
   es->mtimensec = st->st_mtimensec;
   es->ctimensec = st->st_ctimensec;
# endif
#else
   es->atimensec = 0;
   es->mtimensec = 0;
   es->ctimensec = 0;
#endif
}

static void
_eio_file_stat(void *data, Ecore_Thread *thread)
{
   Eio_File_Stat *s = data;
   _eio_stat_t buf;

   if (_eio_stat(s->path, &buf) != 0)
     eio_file_thread_error(&s->common, thread);

   _eio_file_struct_2_eina(&s->buffer, &buf);
}

static void
_eio_file_lstat(void *data, Ecore_Thread *thread)
{
   Eio_File_Stat *s = data;
   _eio_stat_t buf;

   if (_eio_lstat(s->path, &buf) != 0)
     eio_file_thread_error(&s->common, thread);

   _eio_file_struct_2_eina(&s->buffer, &buf);
}

static void
_eio_stat_free(Eio_File_Stat *s)
{
   eina_stringshare_del(s->path);
   eio_file_free(&s->common);
}

static void
_eio_file_stat_done(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_File_Stat *s = data;

   if (s->done_cb)
     s->done_cb((void*) s->common.data, &s->common, &s->buffer);

   _eio_stat_free(s);
}

static void
_eio_file_stat_error(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_File_Stat *s = data;

   eio_file_error(&s->common);
   _eio_stat_free(s);
}

static void
_eio_file_chmod(void *data, Ecore_Thread *thread)
{
   Eio_File_Chmod *ch = data;

   if (chmod(ch->path, ch->mode) != 0)
     eio_file_thread_error(&ch->common, thread);
}

static void
_eio_file_chown(void *data, Ecore_Thread *thread)
{
#ifdef _WIN32
  /* FIXME:
   * look at http://wwwthep.physik.uni-mainz.de/~frink/chown/readme.html
   */
  (void)data;
  (void)thread;
#else
   Eio_File_Chown *own = data;
   char *tmp;
   uid_t owner = -1;
   gid_t group = -1;

   own->common.error = 0;

   if (own->user)
     {
        owner = strtol(own->user, &tmp, 10);

        if (*tmp != '\0')
          {
             struct passwd *pw = NULL;

             own->common.error = EIO_FILE_GETPWNAM;

             pw = getpwnam(own->user);
             if (!pw) goto on_error;

             owner = pw->pw_uid;
          }
     }

   if (own->group)
     {
        group = strtol(own->group, &tmp, 10);

        if (*tmp != '\0')
          {
             struct group *grp = NULL;

             own->common.error = EIO_FILE_GETGRNAM;

             grp = getgrnam(own->group);
             if (!grp) goto on_error;

             group = grp->gr_gid;
          }
     }

   if (owner == (uid_t) -1 && group == (gid_t) -1)
     goto on_error;

   if (chown(own->path, owner, group) != 0)
     eio_file_thread_error(&own->common, thread);

   return;

 on_error:
   ecore_thread_cancel(thread);
   return;
#endif
}

static void
_eio_chown_free(Eio_File_Chown *ch)
{
   if (ch->user) eina_stringshare_del(ch->user);
   if (ch->group) eina_stringshare_del(ch->group);
   eina_stringshare_del(ch->path);
   eio_file_free(&ch->common);
}

static void
_eio_file_chown_done(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_File_Chown *ch = data;

   if (ch->common.done_cb)
     ch->common.done_cb((void*) ch->common.data, &ch->common);

   _eio_chown_free(ch);
}

static void
_eio_file_chown_error(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_File_Chown *ch = data;

   eio_file_error(&ch->common);
   _eio_chown_free(ch);
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

void
eio_file_error(Eio_File *common)
{
   if (common->error_cb)
     common->error_cb((void*) common->data, common, common->error);
   common->thread = NULL;
}

void
eio_file_thread_error(Eio_File *common, Ecore_Thread *thread)
{
   common->error = errno;
   ecore_thread_cancel(thread);
}

void
eio_file_free(Eio_File *common)
{
   if (common->worker.associated)
     eina_hash_free(common->worker.associated);
   if (common->main.associated)
     eina_hash_free(common->main.associated);
   eio_file_unregister(common);
   eio_common_free(common);
}

Eina_Bool
eio_long_file_set(Eio_File *common,
		  Eio_Done_Cb done_cb,
		  Eio_Error_Cb error_cb,
		  const void *data,
		  Ecore_Thread_Cb heavy_cb,
		  Ecore_Thread_Notify_Cb notify_cb,
		  Ecore_Thread_Cb end_cb,
		  Ecore_Thread_Cb cancel_cb)
{
   Ecore_Thread *thread;

   common->done_cb = done_cb;
   common->error_cb = error_cb;
   common->data = data;
   common->error = 0;
   common->length = 0;
   common->thread = NULL;
   common->container = NULL;
   common->worker.associated = NULL;
   common->main.associated = NULL;

   /* Be aware that ecore_thread_feedback_run could call cancel_cb if something goes wrong.
      This means that common would be destroyed if thread == NULL.
    */
   thread = ecore_thread_feedback_run(heavy_cb,
                                      notify_cb,
                                      end_cb,
                                      cancel_cb,
                                      common,
                                      EINA_FALSE);
   if (thread)
     {
        common->thread = thread;
        eio_file_register(common);
     }
   return !!thread;
}

Eina_Bool
eio_file_set(Eio_File *common,
	     Eio_Done_Cb done_cb,
	     Eio_Error_Cb error_cb,
	     const void *data,
	     Ecore_Thread_Cb job_cb,
	     Ecore_Thread_Cb end_cb,
	     Ecore_Thread_Cb cancel_cb)
{
   Ecore_Thread *thread;

   common->done_cb = done_cb;
   common->error_cb = error_cb;
   common->data = data;
   common->error = 0;
   common->length = 0;
   common->thread = NULL;
   common->container = NULL;
   common->worker.associated = NULL;
   common->main.associated = NULL;

   /* Be aware that ecore_thread_run could call cancel_cb if something goes wrong.
      This means that common would be destroyed if thread == NULL.
   */
   thread = ecore_thread_run(job_cb, end_cb, cancel_cb, common);

   if (thread)
     {
        common->thread = thread;
        eio_file_register(common);
     }
   return !!thread;
}

void
eio_file_container_set(Eio_File *common, void *container)
{
   common->container = container;
}

/**
 * @endcond
 */


/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

EAPI Eio_File *
eio_file_direct_stat(const char *path,
		     Eio_Stat_Cb done_cb,
		     Eio_Error_Cb error_cb,
		     const void *data)
{
   Eio_File_Stat *s = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   s = eio_common_alloc(sizeof (Eio_File_Stat));
   if (!s) return NULL;

   s->path = eina_stringshare_add(path);
   s->done_cb = done_cb;

   if (!eio_file_set(&s->common,
		      NULL,
		      error_cb,
		      data,
		      _eio_file_stat,
		      _eio_file_stat_done,
		      _eio_file_stat_error))
     /* THERE IS NO MEMLEAK HERE, ECORE_THREAD CANCEL CALLBACK HAS BEEN ALREADY CALLED
	AND s HAS BEEN FREED, SAME FOR ALL CALL TO EIO_FILE_SET ! */
     return NULL;

   return &s->common;
}

EAPI Eio_File *
eio_file_direct_lstat(const char *path,
		      Eio_Stat_Cb done_cb,
		      Eio_Error_Cb error_cb,
		      const void *data)
{
   Eio_File_Stat *s = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   s = eio_common_alloc(sizeof (Eio_File_Stat));
   if (!s) return NULL;

   s->path = eina_stringshare_add(path);
   s->done_cb = done_cb;

   if (!eio_file_set(&s->common,
		      NULL,
		      error_cb,
		      data,
		      _eio_file_lstat,
		      _eio_file_stat_done,
		      _eio_file_stat_error))
     return NULL;

   return &s->common;
}

EAPI Eio_File *
eio_file_unlink(const char *path,
		Eio_Done_Cb done_cb,
		Eio_Error_Cb error_cb,
		const void *data)
{
   Eio_File_Unlink *l = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   l = eio_common_alloc(sizeof (Eio_File_Unlink));
   if (!l) return NULL;

   l->path = eina_stringshare_add(path);

   if (!eio_file_set(&l->common,
		      done_cb,
		      error_cb,
		      data,
		      _eio_file_unlink,
		      _eio_file_unlink_done,
		      _eio_file_unlink_error))
     return NULL;

   return &l->common;
}

EAPI Eio_File *
eio_file_mkdir(const char *path,
	       mode_t mode,
	       Eio_Done_Cb done_cb,
	       Eio_Error_Cb error_cb,
	       const void *data)
{
   Eio_File_Mkdir *r = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   r = eio_common_alloc(sizeof (Eio_File_Mkdir));
   if (!r) return NULL;

   r->path = eina_stringshare_add(path);
   r->mode = mode;

   if (!eio_file_set(&r->common,
		     done_cb,
		     error_cb,
		      data,
		     _eio_file_mkdir,
		     _eio_file_mkdir_done,
		     _eio_file_mkdir_error))
     return NULL;

   return &r->common;
}

EAPI Eio_File *
eio_file_chmod(const char *path,
	       mode_t mode,
	       Eio_Done_Cb done_cb,
	       Eio_Error_Cb error_cb,
	       const void *data)
{
   Eio_File_Mkdir *r = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   r = eio_common_alloc(sizeof (Eio_File_Mkdir));
   if (!r) return NULL;

   r->path = eina_stringshare_add(path);
   r->mode = mode;

   if (!eio_file_set(&r->common,
		     done_cb,
		     error_cb,
		      data,
		     _eio_file_chmod,
		     _eio_file_mkdir_done,
		     _eio_file_mkdir_error))
     return NULL;

   return &r->common;
}

EAPI Eio_File *
eio_file_chown(const char *path,
	       const char *user,
	       const char *group,
	       Eio_Done_Cb done_cb,
	       Eio_Error_Cb error_cb,
	       const void *data)
{
   Eio_File_Chown *c = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   c = eio_common_alloc(sizeof (Eio_File_Chown));
   if (!c) return NULL;

   c->path = eina_stringshare_add(path);
   c->user = eina_stringshare_add(user);
   c->group = eina_stringshare_add(group);

   if (!eio_file_set(&c->common,
		     done_cb,
		     error_cb,
		      data,
		     _eio_file_chown,
		     _eio_file_chown_done,
		     _eio_file_chown_error))
     return NULL;

   return &c->common;
}
