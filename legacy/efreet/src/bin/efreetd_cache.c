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

static Eina_Hash *change_monitors = NULL;

static Ecore_Event_Handler *cache_exe_del_handler = NULL;
static Ecore_Event_Handler *cache_exe_data_handler = NULL;
static Ecore_Exe           *icon_cache_exe = NULL;
static Ecore_Exe           *desktop_cache_exe = NULL;
static Ecore_Timer         *icon_cache_timer = NULL;
static Ecore_Timer         *desktop_cache_timer = NULL;

static Eina_List *desktop_extra_dirs = NULL;
static Eina_List *icon_extra_dirs = NULL;
static Eina_List *icon_extra_exts = NULL;

static void desktop_changes_monitor_add(const char *path);

/* internal */
static Eina_Bool
icon_cache_update_cache_cb(void *data __UNUSED__)
{
   char file[PATH_MAX];
   int prio;

   icon_cache_timer = NULL;

   /* TODO: Queue if already running */
   prio = ecore_exe_run_priority_get();
   ecore_exe_run_priority_set(19);
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
   if (icon_extra_exts)
     {
        Eina_List *ll;
        char *p;

        eina_strlcat(file, " -e", sizeof(file));
        EINA_LIST_FOREACH(icon_extra_exts, ll, p)
          {
             eina_strlcat(file, " ", sizeof(file));
             eina_strlcat(file, p, sizeof(file));
          }
     }
   icon_cache_exe = ecore_exe_run(file, NULL);
   ecore_exe_run_priority_set(prio);

   return ECORE_CALLBACK_CANCEL;
}

static void
cache_icon_update(void)
{
   if (icon_cache_timer)
     ecore_timer_del(icon_cache_timer);
   icon_cache_timer = ecore_timer_add(0.2, icon_cache_update_cache_cb, NULL);
}

static Eina_Bool
desktop_cache_update_cache_cb(void *data __UNUSED__)
{
   char file[PATH_MAX];
   int prio;

   desktop_cache_timer = NULL;

   /* TODO: Queue if already running */
   prio = ecore_exe_run_priority_get();
   ecore_exe_run_priority_set(19);
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
   desktop_cache_exe = ecore_exe_run(file, NULL);
   ecore_exe_run_priority_set(prio);

   return ECORE_CALLBACK_CANCEL;
}

static void
cache_desktop_update(void)
{
   if (desktop_cache_timer)
     ecore_timer_del(desktop_cache_timer);
   desktop_cache_timer = ecore_timer_add(0.2, desktop_cache_update_cache_cb, NULL);
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
     }
   else if (ev->exe == icon_cache_exe)
     {
        icon_cache_exe = NULL;
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
         cache_icon_update();
         break;

      case ECORE_FILE_EVENT_DELETED_SELF:
         eina_hash_del_by_key(change_monitors, path);
         cache_icon_update();
         break;
     }
}

static void
icon_changes_monitor_add(const char *path)
{
   Eina_Iterator *it;
   Eina_File_Direct_Info *info;

   if (!ecore_file_is_dir(path)) return;
   if (eina_hash_find(change_monitors, path)) return;
   eina_hash_add(change_monitors, path,
                 ecore_file_monitor_add(path,
                                        icon_changes_cb,
                                        NULL));

   it = eina_file_stat_ls(path);
   if (!it) return;
   EINA_ITERATOR_FOREACH(it, info)
     {
        if (info->type != EINA_FILE_DIR) continue;
        eina_hash_add(change_monitors, info->path,
                      ecore_file_monitor_add(info->path,
                                             icon_changes_cb,
                                             NULL));
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

   icon_changes_monitor_add(efreet_icon_deprecated_user_dir_get());
   icon_changes_monitor_add(efreet_icon_user_dir_get());
   EINA_LIST_FOREACH(icon_extra_dirs, l, dir)
      icon_changes_monitor_add(dir);

   xdg_dirs = efreet_data_dirs_get();
   EINA_LIST_FOREACH(xdg_dirs, l, dir)
     {
        snprintf(buf, sizeof(buf), "%s/icons", dir);
        icon_changes_monitor_add(buf);
     }

#ifndef STRICT_SPEC
   EINA_LIST_FOREACH(xdg_dirs, l, dir)
     {
        snprintf(buf, sizeof(buf), "%s/pixmaps", dir);
        icon_changes_monitor_add(buf);
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
   if (eina_hash_find(change_monitors, path)) return;
   eina_hash_add(change_monitors, path,
                 ecore_file_monitor_add(path,
                                        desktop_changes_cb,
                                        NULL));
}

static void
desktop_changes_listen_recursive(const char *path)
{
   Eina_Iterator *it;
   Eina_File_Direct_Info *info;

   desktop_changes_monitor_add(path);

   it = eina_file_stat_ls(path);
   if (!it) return;
   EINA_ITERATOR_FOREACH(it, info)
     {
        if (info->type != EINA_FILE_DIR) continue;
        desktop_changes_listen_recursive(info->path);
     }
   eina_iterator_free(it);
}

static void
desktop_changes_listen(void)
{
   Eina_List *dirs, *l;
   const char *path;

   dirs = efreet_default_dirs_get(efreet_data_home_get(),
                                  efreet_data_dirs_get(), "applications");

   EINA_LIST_FREE(dirs, path)
     {
        if (ecore_file_is_dir(path))
          desktop_changes_listen_recursive(path);
        eina_stringshare_del(path);
     }

   EINA_LIST_FOREACH(desktop_extra_dirs, l, path)
      desktop_changes_monitor_add(path);
}

static void
fill_extra(const char *file, Eina_List **l)
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
        printf("fill: %s\n", (const char *)(*l)->data);
     }
   eina_iterator_free(it);
error:
   eina_file_close(f);
}

static void
read_extra(void)
{
   fill_extra("extra_desktop.dirs", &desktop_extra_dirs);
   fill_extra("extra_icon.dirs", &icon_extra_dirs);
   fill_extra("extra_icon.exts", &icon_extra_exts);
}

static void
save_extra(const char *file, Eina_List *l)
{
   FILE *f;
   char buf[PATH_MAX];
   Eina_List *ll;
   const char *path;

   snprintf(buf, sizeof(buf), "%s/efreet/%s", efreet_cache_home_get(), file);
   f = fopen("wb", buf);
   if (!f) return;
   EINA_LIST_FOREACH(l, ll, path)
      fprintf(f, "%s\n", path);
   fclose(f);
}

/* external */
void
cache_desktop_dir_add(const char *dir)
{
   char *san;

   san = eina_file_path_sanitize(dir);
   if (!san) return;
   if (!eina_list_search_unsorted_list(desktop_extra_dirs, EINA_COMPARE_CB(strcmp), san))
     {
        desktop_extra_dirs = eina_list_append(desktop_extra_dirs, eina_stringshare_add(san));
        save_extra("extra_desktop.dirs", desktop_extra_dirs);
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
        save_extra("extra_icon.dirs", icon_extra_dirs);
        cache_icon_update();
     }
   free(san);
}

void
cache_icon_ext_add(const char *ext)
{
   if (!eina_list_search_unsorted_list(icon_extra_exts, EINA_COMPARE_CB(strcmp), ext))
     {
        icon_extra_exts = eina_list_append(icon_extra_exts, eina_stringshare_add(ext));
        save_extra("extra_icon.exts", icon_extra_exts);
        cache_icon_update();
     }
}

Eina_Bool
cache_init(void)
{
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%s/efreet", efreet_cache_home_get());
   if (!ecore_file_mkpath(buf))
     {
        ERR("Failed to create directory '%s'\n", buf);
        goto error;
     }

   cache_exe_del_handler = ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
                                               cache_exe_del_cb, NULL);
   if (!cache_exe_del_handler)
     {
        ERR("Failed to add exe del handler\n");
        goto error;
     }
   cache_exe_data_handler = ecore_event_handler_add(ECORE_EXE_EVENT_DATA,
                                               cache_exe_data_cb, NULL);
   if (!cache_exe_data_handler)
     {
        ERR("Failed to add exe del handler\n");
        goto error;
     }

   change_monitors = eina_hash_string_superfast_new(EINA_FREE_CB(ecore_file_monitor_del));

   efreet_cache_update = 0;
   if (!efreet_init()) goto error;

   read_extra();
   icon_changes_listen();
   desktop_changes_listen();
   cache_icon_update();
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
   EINA_LIST_FREE(desktop_extra_dirs, data)
      eina_stringshare_del(data);
   EINA_LIST_FREE(icon_extra_dirs, data)
      eina_stringshare_del(data);
   EINA_LIST_FREE(icon_extra_exts, data)
      eina_stringshare_del(data);
   return EINA_TRUE;
}
