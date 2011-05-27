#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "ecore_file_private.h"

#ifdef HAVE_POLL

/*
 * TODO:
 * - Implement recursive as an option!
 * - Keep whole path or just name of file? (Memory or CPU...)
 * - Remove requests without files?
 * - Change poll time
 */

typedef struct _Ecore_File_Monitor_Poll Ecore_File_Monitor_Poll;

#define ECORE_FILE_MONITOR_POLL(x) ((Ecore_File_Monitor_Poll *)(x))

struct _Ecore_File_Monitor_Poll
{
   Ecore_File_Monitor  monitor;
   int                 mtime;
   unsigned char       deleted;
};

#define ECORE_FILE_INTERVAL_MIN  1.0
#define ECORE_FILE_INTERVAL_STEP 0.5
#define ECORE_FILE_INTERVAL_MAX  5.0

static double         _interval = ECORE_FILE_INTERVAL_MIN;
static Ecore_Timer   *_timer = NULL;
static Ecore_File_Monitor *_monitors = NULL;
static int          _lock = 0;

static Eina_Bool   _ecore_file_monitor_poll_handler(void *data);
static void        _ecore_file_monitor_poll_check(Ecore_File_Monitor *em);
static int         _ecore_file_monitor_poll_checking(Ecore_File_Monitor *em, char *name);

int
ecore_file_monitor_poll_init(void)
{
   return 1;
}

int
ecore_file_monitor_poll_shutdown(void)
{
   while(_monitors)
        ecore_file_monitor_poll_del(_monitors);

   if (_timer)
     {
        ecore_timer_del(_timer);
        _timer = NULL;
     }
   return 1;
}

Ecore_File_Monitor *
ecore_file_monitor_poll_add(const char *path,
                            void (*func) (void *data, Ecore_File_Monitor *em,
                                          Ecore_File_Event event,
                                          const char *path),
                            void *data)
{
   Ecore_File_Monitor *em;
   size_t len;

   if (!path) return NULL;
   if (!func) return NULL;

   em = calloc(1, sizeof(Ecore_File_Monitor_Poll));
   if (!em) return NULL;

   if (!_timer)
     _timer = ecore_timer_add(_interval, _ecore_file_monitor_poll_handler, NULL);
   else
     ecore_timer_interval_set(_timer, ECORE_FILE_INTERVAL_MIN);

   em->path = strdup(path);
   len = strlen(em->path);
   if (em->path[len - 1] == '/' && strcmp(em->path, "/"))
     em->path[len - 1] = 0;

   em->func = func;
   em->data = data;

   ECORE_FILE_MONITOR_POLL(em)->mtime = ecore_file_mod_time(em->path);
   _monitors = ECORE_FILE_MONITOR(eina_inlist_append(EINA_INLIST_GET(_monitors), EINA_INLIST_GET(em)));

   if (ecore_file_exists(em->path))
     {
        if (ecore_file_is_dir(em->path))
          {
             /* Check for subdirs */
             Eina_List *files;
             char *file;

             files = ecore_file_ls(em->path);
             EINA_LIST_FREE(files, file)
                    {
                       Ecore_File *f;
                       char buf[PATH_MAX];

                       f = calloc(1, sizeof(Ecore_File));
                       if (!f)
                    {
                       free(file);
                         continue;
                    }

                       snprintf(buf, sizeof(buf), "%s/%s", em->path, file);
                       f->name = file;
                       f->mtime = ecore_file_mod_time(buf);
                       f->is_dir = ecore_file_is_dir(buf);
                       em->files = (Ecore_File *) eina_inlist_append(EINA_INLIST_GET(em->files), EINA_INLIST_GET(f));
                    }
          }
     }
   else
     {
        ecore_file_monitor_poll_del(em);
        return NULL;
     }

   return em;
}

void
ecore_file_monitor_poll_del(Ecore_File_Monitor *em)
{
   Ecore_File *l;

   if (_lock)
     {
        ECORE_FILE_MONITOR_POLL(em)->deleted = 1;
        return;
     }

   /* Remove files */
   /*It's possible there weren't any files to monitor, so check if the list is init*/
   if (em->files)
     {
        for (l = em->files; l;)
          {
             Ecore_File *file = l;

             l = (Ecore_File *) EINA_INLIST_GET(l)->next;
             free(file->name);
             free(file);
          }
     }

   if (_monitors)
     _monitors = ECORE_FILE_MONITOR(eina_inlist_remove(EINA_INLIST_GET(_monitors), EINA_INLIST_GET(em)));

   free(em->path);
   free(em);

   if (_timer)
     {
        if (!_monitors)
          {
             ecore_timer_del(_timer);
             _timer = NULL;
          }
        else
          ecore_timer_interval_set(_timer, ECORE_FILE_INTERVAL_MIN);
     }
}

static Eina_Bool
_ecore_file_monitor_poll_handler(void *data __UNUSED__)
{
   Ecore_File_Monitor *l;

   _interval += ECORE_FILE_INTERVAL_STEP;

   _lock = 1;
   EINA_INLIST_FOREACH(_monitors, l)
        _ecore_file_monitor_poll_check(l);
   _lock = 0;

   if (_interval > ECORE_FILE_INTERVAL_MAX)
     _interval = ECORE_FILE_INTERVAL_MAX;
   ecore_timer_interval_set(_timer, _interval);

   for (l = _monitors; l;)
     {
        Ecore_File_Monitor *em = l;

        l = ECORE_FILE_MONITOR(EINA_INLIST_GET(l)->next);
        if (ECORE_FILE_MONITOR_POLL(em)->deleted)
          ecore_file_monitor_del(em);
     }
   return ECORE_CALLBACK_RENEW;
}

static void
_ecore_file_monitor_poll_check(Ecore_File_Monitor *em)
{
   int mtime;

   mtime = ecore_file_mod_time(em->path);
   if (mtime < ECORE_FILE_MONITOR_POLL(em)->mtime)
     {
        Ecore_File *l;
        Ecore_File_Event event;

        /* Notify all files deleted */
        for (l = em->files; l;)
          {
             Ecore_File *f = l;
             char buf[PATH_MAX];

             l = (Ecore_File *) EINA_INLIST_GET(l)->next;

             snprintf(buf, sizeof(buf), "%s/%s", em->path, f->name);
             if (f->is_dir)
               event = ECORE_FILE_EVENT_DELETED_DIRECTORY;
             else
               event = ECORE_FILE_EVENT_DELETED_FILE;
             em->func(em->data, em, event, buf);
             free(f->name);
             free(f);
          }
        em->files = NULL;
        em->func(em->data, em, ECORE_FILE_EVENT_DELETED_SELF, em->path);
        _interval = ECORE_FILE_INTERVAL_MIN;
     }
   else
     {
        Ecore_File *l;

        /* Check for changed files */
        for (l = em->files; l;)
          {
             Ecore_File *f = l;
             char buf[PATH_MAX];
             int mt;
             Ecore_File_Event event;

             l = (Ecore_File *) EINA_INLIST_GET(l)->next;

             snprintf(buf, sizeof(buf), "%s/%s", em->path, f->name);
             mt = ecore_file_mod_time(buf);
             if (mt < f->mtime)
               {
                  if (f->is_dir)
                    event = ECORE_FILE_EVENT_DELETED_DIRECTORY;
                  else
                    event = ECORE_FILE_EVENT_DELETED_FILE;

                  em->func(em->data, em, event, buf);
                  em->files = (Ecore_File *) eina_inlist_remove(EINA_INLIST_GET(em->files), EINA_INLIST_GET(f));
                  free(f->name);
                  free(f);
                  _interval = ECORE_FILE_INTERVAL_MIN;
               }
             else if ((mt > f->mtime) && !(f->is_dir))
               {
                  em->func(em->data, em, ECORE_FILE_EVENT_MODIFIED, buf);
                  _interval = ECORE_FILE_INTERVAL_MIN;
                  f->mtime = mt;
               }
             else
               f->mtime = mt;
          }

        /* Check for new files */
        if (ECORE_FILE_MONITOR_POLL(em)->mtime < mtime)
          {
             Eina_List *files;
             Eina_List *fl;
             char *file;

             /* Files have been added or removed */
             files = ecore_file_ls(em->path);
             if (files)
               {
                  /* Are we a directory? We should check first, rather than rely on null here*/
                  EINA_LIST_FOREACH(files, fl, file)
                    {
                       Ecore_File *f;
                       char buf[PATH_MAX];
                       Ecore_File_Event event;

                       if (_ecore_file_monitor_poll_checking(em, file))
                         continue;

                       snprintf(buf, sizeof(buf), "%s/%s", em->path, file);
                       f = calloc(1, sizeof(Ecore_File));
                       if (!f)
                         continue;

                       f->name = strdup(file);
                       f->mtime = ecore_file_mod_time(buf);
                       f->is_dir = ecore_file_is_dir(buf);
                       if (f->is_dir)
                         event = ECORE_FILE_EVENT_CREATED_DIRECTORY;
                       else
                         event = ECORE_FILE_EVENT_CREATED_FILE;
                       em->func(em->data, em, event, buf);
                       em->files = (Ecore_File *) eina_inlist_append(EINA_INLIST_GET(em->files), EINA_INLIST_GET(f));
                    }
                  while (files)
                    {
                       file = eina_list_data_get(files);
                       free(file);
                       files = eina_list_remove_list(files, files);
                    }
               }

             if (!ecore_file_is_dir(em->path))
               em->func(em->data, em, ECORE_FILE_EVENT_MODIFIED, em->path);
             _interval = ECORE_FILE_INTERVAL_MIN;
          }
     }
   ECORE_FILE_MONITOR_POLL(em)->mtime = mtime;
}

static int
_ecore_file_monitor_poll_checking(Ecore_File_Monitor *em, char *name)
{
   Ecore_File *l;

   EINA_INLIST_FOREACH(em->files, l)
     {
        if (!strcmp(l->name, name))
          return 1;
     }
   return 0;
}
#endif
