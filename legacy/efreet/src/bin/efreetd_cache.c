#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_File.h>

#include "efreetd.h"
#include "efreetd_dbus.h"

#include "Efreet.h"
#define EFREET_MODULE_LOG_DOM efreetd_log_dom
#include "efreet_private.h"
#include "efreetd_cache.h"

static Eina_Hash *change_monitors = NULL;

static Ecore_Event_Handler *cache_exe_del_handler = NULL;
static Ecore_Event_Handler *cache_exe_data_handler = NULL;
static Ecore_Exe           *icon_cache_exe = NULL;
static Ecore_Exe           *desktop_cache_exe = NULL;
static Ecore_Timer         *icon_cache_timer = NULL;
static Ecore_Timer         *desktop_cache_timer = NULL;

static Eina_Bool  desktop_exists = EINA_FALSE;

static Eina_List *desktop_system_dirs = NULL;
static Eina_List *desktop_extra_dirs = NULL;
static Eina_List *icon_extra_dirs = NULL;
static Eina_List *icon_exts = NULL;
static Eina_Bool  icon_flush = EINA_FALSE;

static Eina_Bool desktop_queue = EINA_FALSE;
static Eina_Bool icon_queue = EINA_FALSE;

static void desktop_changes_monitor_add(const char *path);

/* internal */
static Eina_Bool
icon_cache_update_cache_cb(void *data __UNUSED__)
{
   char file[PATH_MAX];
   int prio;

   icon_cache_timer = NULL;

   if (icon_cache_exe)
     {
        icon_queue = EINA_TRUE;
        return ECORE_CALLBACK_CANCEL;
     }
   icon_queue = EINA_FALSE;
   if ((!icon_flush) && (!icon_exts)) return ECORE_CALLBACK_CANCEL;

   /* TODO: Queue if already running */
   prio = ecore_exe_run_priority_get();
   ecore_exe_run_priority_set(19);
   // XXX: use eina_prefix, not hard-coded prefixes
   eina_strlcpy(file, PACKAGE_LIB_DIR "/efreet/efreet_icon_cache_create", sizeof(file));
   if (icon_extra_dirs)
     {
        Eina_List *ll;
        char *p;

        eina_strlcat(file, " -d", sizeof(file));
        EINA_LIST_FOREACH(icon_extra_dirs, ll, p)
          {
             eina_strlcat(file, " ", sizeof(file));
             eina_strlcat(file, p, sizeof(file));
          }
     }
   if (icon_exts)
     {
        Eina_List *ll;
        char *p;

        eina_strlcat(file, " -e", sizeof(file));
        EINA_LIST_FOREACH(icon_exts, ll, p)
          {
             eina_strlcat(file, " ", sizeof(file));
             eina_strlcat(file, p, sizeof(file));
          }
     }
   if (icon_flush)
     eina_strlcat(file, " -f", sizeof(file));
   icon_flush = EINA_FALSE;
   icon_cache_exe =
      ecore_exe_pipe_run(file, ECORE_EXE_PIPE_READ|ECORE_EXE_PIPE_READ_LINE_BUFFERED, NULL);
   ecore_exe_run_priority_set(prio);

   return ECORE_CALLBACK_CANCEL;
}

static void
cache_icon_update(Eina_Bool flush)
{
   if (icon_cache_timer)
     ecore_timer_del(icon_cache_timer);
   if (flush)
     icon_flush = flush;
   icon_cache_timer = ecore_timer_add(1.0, icon_cache_update_cache_cb, NULL);
}

static Eina_Bool
desktop_cache_update_cache_cb(void *data __UNUSED__)
{
   char file[PATH_MAX];
   int prio;

   desktop_cache_timer = NULL;

   if (desktop_cache_exe)
     {
        desktop_queue = EINA_TRUE;
        return ECORE_CALLBACK_CANCEL;
     }
   desktop_queue = EINA_FALSE;
   prio = ecore_exe_run_priority_get();
   ecore_exe_run_priority_set(19);
   // XXX: use eina_prefix, not hard-coded prefixes
   eina_strlcpy(file, PACKAGE_LIB_DIR "/efreet/efreet_desktop_cache_create", sizeof(file));
   if (desktop_extra_dirs)
     {
        Eina_List *ll;
        const char *str;

        eina_strlcat(file, " -d", sizeof(file));
        EINA_LIST_FOREACH(desktop_extra_dirs, ll, str)
          {
             eina_strlcat(file, " ", sizeof(file));
             eina_strlcat(file, str, sizeof(file));
          }
     }
   INF("Run desktop cache creation: %s", file);
   desktop_cache_exe =
      ecore_exe_pipe_run(file, ECORE_EXE_PIPE_READ|ECORE_EXE_PIPE_READ_LINE_BUFFERED, NULL);
   ecore_exe_run_priority_set(prio);

   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
cache_exe_data_cb(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Exe_Event_Data *ev;

   ev = event;
   if (ev->exe == desktop_cache_exe)
     {
        Eina_Bool update = EINA_FALSE;

        if ((ev->lines) && (*ev->lines->line == 'c'))
          update = EINA_TRUE;

        desktop_exists = EINA_TRUE;
        send_signal_desktop_cache_update(update);
     }
   else if (ev->exe == icon_cache_exe)
     {
        Eina_Bool update = EINA_FALSE;

        if ((ev->lines) && (*ev->lines->line == 'c'))
          update = EINA_TRUE;

        send_signal_icon_cache_update(update);
     }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
cache_exe_del_cb(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Exe_Event_Del *ev;

   ev = event;
   if (ev->exe == desktop_cache_exe)
     {
        desktop_cache_exe = NULL;
        if (desktop_queue) cache_desktop_update();
     }
   else if (ev->exe == icon_cache_exe)
     {
        icon_cache_exe = NULL;
        if (icon_queue) cache_icon_update(EINA_FALSE);
     }
   return ECORE_CALLBACK_RENEW;
}

static void
icon_changes_cb(void *data __UNUSED__, Ecore_File_Monitor *em __UNUSED__,
                Ecore_File_Event event, const char *path)
{
   /* TODO: If we get a stale symlink, we need to rerun cache creation */
   switch (event)
     {
      case ECORE_FILE_EVENT_NONE:
         /* noop */
         break;

      case ECORE_FILE_EVENT_CREATED_FILE:
      case ECORE_FILE_EVENT_DELETED_FILE:
      case ECORE_FILE_EVENT_MODIFIED:
      case ECORE_FILE_EVENT_CLOSED:
      case ECORE_FILE_EVENT_DELETED_DIRECTORY:
      case ECORE_FILE_EVENT_CREATED_DIRECTORY:
         cache_icon_update(EINA_FALSE);
         break;

      case ECORE_FILE_EVENT_DELETED_SELF:
         eina_hash_del_by_key(change_monitors, path);
         cache_icon_update(EINA_FALSE);
         break;
     }
}

static void
icon_changes_monitor_add(const char *path)
{
   Ecore_File_Monitor *mon;

   if (eina_hash_find(change_monitors, path)) return;
   /* TODO: Check for symlink and monitor the real path */
   mon = ecore_file_monitor_add(path,
                                icon_changes_cb,
                                NULL);
   if (mon)
     eina_hash_add(change_monitors, path, mon);
}

static void
icon_changes_listen_recursive(const char *path, Eina_Bool base)
{
   Eina_Iterator *it;
   Eina_File_Direct_Info *info;

   if ((!ecore_file_is_dir(path)) && (base))
     {
        // XXX: if it doesn't exist... walk the parent dirs back down
        // to this path until we find one that doesn't exist, then
        // monitor its parent, and treat it specially as it needs
        // to look for JUST the creation of this specific child
        // and when this child is created, replace this monitor with
        // monitoring the next specific child dir down until we are
        // monitoring the original path again.
     }
   icon_changes_monitor_add(path);
   it = eina_file_stat_ls(path);
   if (!it) return;
   EINA_ITERATOR_FOREACH(it, info)
     {
        if (((info->type == EINA_FILE_LNK) && (ecore_file_is_dir(info->path))) ||
            (info->type == EINA_FILE_DIR))
          icon_changes_monitor_add(info->path);
     }
   eina_iterator_free(it);
}

static void
icon_changes_listen(void)
{
   Eina_List *l;
   Eina_List *xdg_dirs;
   char buf[PATH_MAX];
   const char *dir;

   icon_changes_listen_recursive(efreet_icon_deprecated_user_dir_get(), EINA_TRUE);
   icon_changes_listen_recursive(efreet_icon_user_dir_get(), EINA_TRUE);
   EINA_LIST_FOREACH(icon_extra_dirs, l, dir)
     {
        icon_changes_listen_recursive(dir, EINA_TRUE);
     }

   xdg_dirs = efreet_data_dirs_get();
   EINA_LIST_FOREACH(xdg_dirs, l, dir)
     {
        snprintf(buf, sizeof(buf), "%s/icons", dir);
        icon_changes_listen_recursive(buf, EINA_TRUE);
     }

#ifndef STRICT_SPEC
   EINA_LIST_FOREACH(xdg_dirs, l, dir)
     {
        snprintf(buf, sizeof(buf), "%s/pixmaps", dir);
        icon_changes_listen_recursive(buf, EINA_TRUE);
     }
#endif

   icon_changes_monitor_add("/usr/share/pixmaps");
}

static void
desktop_changes_cb(void *data __UNUSED__, Ecore_File_Monitor *em __UNUSED__,
                   Ecore_File_Event event, const char *path)
{
   const char *ext;

   /* TODO: If we get a stale symlink, we need to rerun cache creation */
   /* TODO: Check for desktop*.cache, as this will be created when app is installed */
   /* TODO: Do efreet_cache_icon_update() when app is installed, as it has the same
    *       symlink problem */
   switch (event)
     {
      case ECORE_FILE_EVENT_NONE:
         /* noop */
         break;

      case ECORE_FILE_EVENT_CREATED_FILE:
      case ECORE_FILE_EVENT_DELETED_FILE:
      case ECORE_FILE_EVENT_MODIFIED:
      case ECORE_FILE_EVENT_CLOSED:
         ext = strrchr(path, '.');
         if (ext && (!strcmp(ext, ".desktop") || !strcmp(ext, ".directory")))
           cache_desktop_update();
         break;

      case ECORE_FILE_EVENT_DELETED_SELF:
      case ECORE_FILE_EVENT_DELETED_DIRECTORY:
         eina_hash_del_by_key(change_monitors, path);
         cache_desktop_update();
         break;

      case ECORE_FILE_EVENT_CREATED_DIRECTORY:
         desktop_changes_monitor_add(path);
         cache_desktop_update();
         break;
     }
}

static void
desktop_changes_monitor_add(const char *path)
{
   Ecore_File_Monitor *mon;

   if (eina_hash_find(change_monitors, path)) return;
   /* TODO: Check for symlink and monitor the real path */
   mon = ecore_file_monitor_add(path,
                                desktop_changes_cb,
                                NULL);
   if (mon)
     eina_hash_add(change_monitors, path, mon);
}

static void
desktop_changes_listen_recursive(const char *path)
{
   Eina_Iterator *it;
   Eina_File_Direct_Info *info;

   if (!ecore_file_is_dir(path)) return;
   desktop_changes_monitor_add(path);
   it = eina_file_stat_ls(path);
   if (!it) return;
   EINA_ITERATOR_FOREACH(it, info)
     {
        if (((info->type == EINA_FILE_LNK) && (ecore_file_is_dir(info->path))) ||
            (info->type == EINA_FILE_DIR))
          desktop_changes_listen_recursive(info->path);
     }
   eina_iterator_free(it);
}

static void
desktop_changes_listen(void)
{
   Eina_List *l;
   const char *path;

   EINA_LIST_FOREACH(desktop_system_dirs, l, path)
      desktop_changes_listen_recursive(path);

   EINA_LIST_FOREACH(desktop_extra_dirs, l, path)
      desktop_changes_listen_recursive(path);
}

static void
fill_list(const char *file, Eina_List **l)
{
   Eina_File *f = NULL;
   Eina_Iterator *it = NULL;
   Eina_File_Line *line = NULL;
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%s/efreet/%s", efreet_cache_home_get(), file);
   f = eina_file_open(buf, EINA_FALSE);
   if (!f) return;
   it = eina_file_map_lines(f);
   if (!it) goto error;
   EINA_ITERATOR_FOREACH(it, line)
     {
        const char *end;
        end = line->end - 1;
        *l = eina_list_append(*l, eina_stringshare_add_length(line->start, end - line->start));
     }
   eina_iterator_free(it);
error:
   eina_file_close(f);
}

static void
read_lists(void)
{
   fill_list("extra_desktop.dirs", &desktop_extra_dirs);
   fill_list("extra_icon.dirs", &icon_extra_dirs);
   fill_list("icon.exts", &icon_exts);
}

static void
save_list(const char *file, Eina_List *l)
{
   FILE *f;
   char buf[PATH_MAX];
   Eina_List *ll;
   const char *path;

   snprintf(buf, sizeof(buf), "%s/efreet/%s", efreet_cache_home_get(), file);
   f = fopen(buf, "wb");
   if (!f) return;
   EINA_LIST_FOREACH(l, ll, path)
      fprintf(f, "%s\n", path);
   fclose(f);
}

static int
strcmplen(const void *data1, const void *data2)
{
    return strncmp(data1, data2, eina_stringshare_strlen(data1));
}

/* external */
void
cache_desktop_dir_add(const char *dir)
{
   char *san;
   Eina_List *l;

   san = eina_file_path_sanitize(dir);
   if (!san) return;
   if ((l = eina_list_search_unsorted_list(desktop_system_dirs, strcmplen, san)))
     {
        /* Path is registered, but maybe not monitored */
        const char *path = eina_list_data_get(l);
        if (!eina_hash_find(change_monitors, path))
          cache_desktop_update();
     }
   else if (!eina_list_search_unsorted_list(desktop_extra_dirs, EINA_COMPARE_CB(strcmp), san))
     {
        /* Not a registered path */
        desktop_extra_dirs = eina_list_append(desktop_extra_dirs, eina_stringshare_add(san));
        save_list("extra_desktop.dirs", desktop_extra_dirs);
        cache_desktop_update();
     }
   free(san);
}

void
cache_icon_dir_add(const char *dir)
{
   char *san;

   san = eina_file_path_sanitize(dir);
   if (!san) return;
   if (!eina_list_search_unsorted_list(icon_extra_dirs, EINA_COMPARE_CB(strcmp), san))
     {
        icon_extra_dirs = eina_list_append(icon_extra_dirs, eina_stringshare_add(san));
        save_list("extra_icon.dirs", icon_extra_dirs);
        cache_icon_update(EINA_TRUE);
     }
   free(san);
}

void
cache_icon_ext_add(const char *ext)
{
   if (!eina_list_search_unsorted_list(icon_exts, EINA_COMPARE_CB(strcmp), ext))
     {
        icon_exts = eina_list_append(icon_exts, eina_stringshare_add(ext));
        save_list("icon.exts", icon_exts);
        cache_icon_update(EINA_TRUE);
     }
}

void
cache_desktop_update(void)
{
   if (desktop_cache_timer)
     ecore_timer_del(desktop_cache_timer);
   desktop_cache_timer = ecore_timer_add(1.0, desktop_cache_update_cache_cb, NULL);
}

Eina_Bool
cache_desktop_exists(void)
{
   return desktop_exists;
}

Eina_Bool
cache_init(void)
{
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%s/efreet", efreet_cache_home_get());
   if (!ecore_file_mkpath(buf))
     {
        ERR("Failed to create directory '%s'", buf);
        goto error;
     }

   cache_exe_del_handler = ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
                                                   cache_exe_del_cb, NULL);
   if (!cache_exe_del_handler)
     {
        ERR("Failed to add exe del handler");
        goto error;
     }
   cache_exe_data_handler = ecore_event_handler_add(ECORE_EXE_EVENT_DATA,
                                                    cache_exe_data_cb, NULL);
   if (!cache_exe_data_handler)
     {
        ERR("Failed to add exe data handler");
        goto error;
     }

   change_monitors = eina_hash_string_superfast_new(EINA_FREE_CB(ecore_file_monitor_del));

   efreet_cache_update = 0;
   if (!efreet_init()) goto error;

   read_lists();
   /* TODO: Should check if system dirs has changed and handles extra_dirs */
   desktop_system_dirs = efreet_default_dirs_get(efreet_data_home_get(),
                                                 efreet_data_dirs_get(), "applications");
   desktop_system_dirs =
      eina_list_merge(
         desktop_system_dirs, efreet_default_dirs_get(efreet_data_home_get(),
                                                      efreet_data_dirs_get(), "desktop-directories"));
   icon_changes_listen();
   desktop_changes_listen();
   cache_icon_update(EINA_FALSE);
   cache_desktop_update();

   return EINA_TRUE;
error:
   if (cache_exe_del_handler) ecore_event_handler_del(cache_exe_del_handler);
   cache_exe_del_handler = NULL;
   if (cache_exe_data_handler) ecore_event_handler_del(cache_exe_data_handler);
   cache_exe_data_handler = NULL;
   return EINA_FALSE;
}

Eina_Bool
cache_shutdown(void)
{
   const char *data;

   efreet_shutdown();

   if (cache_exe_del_handler) ecore_event_handler_del(cache_exe_del_handler);
   cache_exe_del_handler = NULL;
   if (cache_exe_data_handler) ecore_event_handler_del(cache_exe_data_handler);
   cache_exe_data_handler = NULL;

   if (change_monitors)
     eina_hash_free(change_monitors);
   change_monitors = NULL;
   EINA_LIST_FREE(desktop_system_dirs, data)
      eina_stringshare_del(data);
   EINA_LIST_FREE(desktop_extra_dirs, data)
      eina_stringshare_del(data);
   EINA_LIST_FREE(icon_extra_dirs, data)
      eina_stringshare_del(data);
   EINA_LIST_FREE(icon_exts, data)
      eina_stringshare_del(data);
   return EINA_TRUE;
}
