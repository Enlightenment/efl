#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_File.h>
#include <Eio.h>
#include <Eet.h>
#include "efreetd.h"
#include "efreetd_ipc.h"

#include "Efreet.h"
#define EFREET_MODULE_LOG_DOM efreetd_log_dom
#include "efreet_private.h"
#include "efreetd_cache.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static Eina_Hash *icon_change_monitors = NULL;
static Eina_Hash *icon_change_monitors_mon = NULL;
static Eina_Hash *desktop_change_monitors = NULL;
static Eina_Hash *desktop_change_monitors_mon = NULL;

static Ecore_Event_Handler *cache_exe_del_handler = NULL;
static Ecore_Event_Handler *cache_exe_data_handler = NULL;
static Ecore_Exe           *icon_cache_exe = NULL;
static Ecore_Exe           *desktop_cache_exe = NULL;
static Ecore_Timer         *icon_cache_timer = NULL;
static Ecore_Timer         *desktop_cache_timer = NULL;
static Eina_Prefix         *pfx = NULL;

static Eina_Bool  desktop_exists = EINA_FALSE;

static Eina_List *desktop_system_dirs = NULL;
static Eina_List *desktop_extra_dirs = NULL;
static Eina_List *icon_extra_dirs = NULL;
static Eina_List *icon_exts = NULL;
static Eina_Bool  icon_flush = EINA_FALSE;

static Eina_Bool desktop_queue = EINA_FALSE;
static Eina_Bool icon_queue = EINA_FALSE;

static Eina_List *_handlers = NULL;

static void icon_changes_listen(void);
static void desktop_changes_listen(void);

/* internal */
typedef struct _Subdir_Cache Subdir_Cache;
typedef struct _Subdir_Cache_Dir Subdir_Cache_Dir;

struct _Subdir_Cache
{
   Eina_Hash *dirs;
};

struct _Subdir_Cache_Dir
{
   unsigned long long dev;
   unsigned long long ino;
   unsigned long long mode;
   unsigned long long uid;
   unsigned long long gid;
   unsigned long long size;
   unsigned long long mtim;
   unsigned long long ctim;
   const char **dirs;
   unsigned int dirs_count;
};

static Eet_Data_Descriptor *subdir_edd = NULL;
static Eet_Data_Descriptor *subdir_dir_edd = NULL;
static Subdir_Cache        *subdir_cache = NULL;
static Eina_Bool            subdir_need_save = EINA_FALSE;

static Eina_Hash *mime_monitors = NULL;
static Eina_Hash *mime_monitors_mon = NULL;
static Ecore_Timer *mime_update_timer = NULL;
static Ecore_Exe *mime_cache_exe = NULL;

static void mime_cache_init(void);
static void mime_cache_shutdown(void);
static Eina_Bool mime_update_cache_cb(void *data EINA_UNUSED);

static void
subdir_cache_dir_free(Subdir_Cache_Dir *cd)
{
   unsigned int i;
   if (!cd) return;
   if (cd->dirs)
     {
        for (i = 0; i < cd->dirs_count; i++)
          eina_stringshare_del(cd->dirs[i]);
        free(cd->dirs);
     }
   free(cd);
}

static void *
subdir_cache_hash_add(void *hash, const char *key, void *data)
{
   if (!hash) hash = eina_hash_string_superfast_new(EINA_FREE_CB(subdir_cache_dir_free));
   if (!hash) return NULL;
   eina_hash_add(hash, key, data);
   return hash;
}

static void
subdir_cache_init(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_File *ef;
   Eina_Strbuf *buf = eina_strbuf_new();
   if (!buf) return;

   // set up data codecs for subdirs in memory
   eet_eina_stream_data_descriptor_class_set(&eddc, sizeof(Subdir_Cache_Dir), "D", sizeof(Subdir_Cache_Dir));
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Subdir_Cache_Dir);
   subdir_dir_edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(subdir_dir_edd, Subdir_Cache_Dir, "0", dev, EET_T_ULONG_LONG);
   EET_DATA_DESCRIPTOR_ADD_BASIC(subdir_dir_edd, Subdir_Cache_Dir, "1", ino, EET_T_ULONG_LONG);
   EET_DATA_DESCRIPTOR_ADD_BASIC(subdir_dir_edd, Subdir_Cache_Dir, "2", mode, EET_T_ULONG_LONG);
   EET_DATA_DESCRIPTOR_ADD_BASIC(subdir_dir_edd, Subdir_Cache_Dir, "3", uid, EET_T_ULONG_LONG);
   EET_DATA_DESCRIPTOR_ADD_BASIC(subdir_dir_edd, Subdir_Cache_Dir, "4", gid, EET_T_ULONG_LONG);
   EET_DATA_DESCRIPTOR_ADD_BASIC(subdir_dir_edd, Subdir_Cache_Dir, "5", size, EET_T_ULONG_LONG);
   EET_DATA_DESCRIPTOR_ADD_BASIC(subdir_dir_edd, Subdir_Cache_Dir, "6", mtim, EET_T_ULONG_LONG);
   EET_DATA_DESCRIPTOR_ADD_BASIC(subdir_dir_edd, Subdir_Cache_Dir, "7", ctim, EET_T_ULONG_LONG);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY_STRING(subdir_dir_edd, Subdir_Cache_Dir, "d", dirs);

   eet_eina_stream_data_descriptor_class_set(&eddc, sizeof(Subdir_Cache), "C", sizeof(Subdir_Cache));
   eddc.func.hash_add = subdir_cache_hash_add;
   subdir_edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_HASH(subdir_edd, Subdir_Cache, "dirs", dirs, subdir_dir_edd);

   // load subdirs from the cache file
   eina_strbuf_append_printf(buf, "%s/efreet/subdirs_%s.eet",
                             efreet_cache_home_get(), efreet_hostname_get());
   ef = eet_open(eina_strbuf_string_get(buf), EET_FILE_MODE_READ);
   if (ef)
     {
        subdir_cache = eet_data_read(ef, subdir_edd, "subdirs");
        eet_close(ef);
     }
   eina_strbuf_free(buf);

   // if we don't have a decoded subdir cache - allocate one
   if (!subdir_cache) subdir_cache = calloc(1, sizeof(Subdir_Cache));
   if (!subdir_cache)
     {
        ERR("Cannot allocate subdir cache in memory");
        return;
     }

   // if we don't have a hash in the subdir cache - allocate it
   if (!subdir_cache->dirs)
     subdir_cache->dirs = eina_hash_string_superfast_new(EINA_FREE_CB(subdir_cache_dir_free));
}

static void
subdir_cache_shutdown(void)
{
   // free up in-memory subdir scan info - don't need it anymore
   if (subdir_cache)
     {
        if (subdir_cache->dirs) eina_hash_free(subdir_cache->dirs);
        free(subdir_cache);
     }
   eet_data_descriptor_free(subdir_dir_edd);
   eet_data_descriptor_free(subdir_edd);
   subdir_cache = NULL;
   subdir_dir_edd = NULL;
   subdir_edd = NULL;
}

static void
subdir_cache_save(void)
{
   Eina_Strbuf *buf;
   Eet_File *ef;
   Eina_Tmpstr *tmpstr = NULL;
   int tmpfd;

   // only if subdirs need saving... and we have subdirs.
   if (!subdir_need_save) return;
   if (!subdir_cache) return;
   if (!subdir_cache->dirs) return;

   buf = eina_strbuf_new();
   if (!buf) return;

   // save to tmp file first
   eina_strbuf_append_printf(buf, "%s/efreet/subdirs_%s.eet.XXXXXX.cache",
                             efreet_cache_home_get(), efreet_hostname_get());

   tmpfd = eina_file_mkstemp(eina_strbuf_string_get(buf), &tmpstr);
   if (tmpfd < 0)
     {
        eina_strbuf_free(buf);
        return;
     }

   eina_strbuf_reset(buf);

   // write out eet file to tmp file
   ef = eet_open(tmpstr, EET_FILE_MODE_WRITE);
   eet_data_write(ef, subdir_edd, "subdirs", subdir_cache, EET_COMPRESSION_SUPERFAST);
   eet_close(ef);

   /*
    * On Windows, buf2 has one remaining ref, hence it can not be renamed below.
    * Stupid NTFS... So we close it first. "Magically", on Windows, this
    * temporary file is not deleted...
    */
#ifdef _WIN32
   close(tmpfd);
#endif

   // atomically rename subdirs file on top from tmp file
   eina_strbuf_append_printf(buf, "%s/efreet/subdirs_%s.eet",
                             efreet_cache_home_get(), efreet_hostname_get());

   if (rename(tmpstr, eina_strbuf_string_get(buf)) < 0)
     {
        unlink(tmpstr);
        ERR("Can't save subdir cache %s", eina_strbuf_string_get(buf));
     }
   // we dont need saving anymore - we just did
   subdir_need_save = EINA_FALSE;
   eina_tmpstr_del(tmpstr);
   eina_strbuf_free(buf);
}

static const Subdir_Cache_Dir *
subdir_cache_get(const struct stat *st, const char *path)
{
   Eina_Iterator *it;
   Eina_File_Direct_Info *info;
   Subdir_Cache_Dir *cd;
   Eina_List *files = NULL;
   int i = 0;
   const char *file;

   // if no subdir cache at all - return null
   if (!subdir_cache) return NULL;
   if (!subdir_cache->dirs) return NULL;

   // if found but something invalid in stored stat info...
   cd = eina_hash_find(subdir_cache->dirs, path);
   if ((cd) &&
       ((cd->dev != (unsigned long long)st->st_dev) ||
        (cd->ino != (unsigned long long)st->st_ino) ||
        (cd->mode != (unsigned long long)st->st_mode) ||
        (cd->uid != (unsigned long long)st->st_uid) ||
        (cd->gid != (unsigned long long)st->st_gid) ||
        (cd->size != (unsigned long long)st->st_size) ||
        (cd->mtim != (unsigned long long)st->st_mtime) ||
        (cd->ctim != (unsigned long long)st->st_ctime)))
     {
        // delete old node and prepare to scan a new one
        eina_hash_del(subdir_cache->dirs, path, cd);
        cd = NULL;
     }
   // if cached dir is ok by now - return it
   if (cd) return cd;

   // we need a new node (fesh or invalid)
   cd = calloc(1, sizeof(Subdir_Cache_Dir));
   if (!cd) return NULL;

   // store stat info
   cd->dev = (unsigned long long)st->st_dev;
   cd->ino = (unsigned long long)st->st_ino;
   cd->mode = (unsigned long long)st->st_mode;
   cd->uid = (unsigned long long)st->st_uid;
   cd->gid = (unsigned long long)st->st_gid;
   cd->size = (unsigned long long)st->st_size;
   cd->mtim = (unsigned long long)st->st_mtime;
   cd->ctim = (unsigned long long)st->st_ctime;

   // go through content finding directories
   it = eina_file_stat_ls(path);
   if (!it) return cd;
   
   EINA_ITERATOR_FOREACH(it, info)
     {
        // if ., .. or other "hidden" dot files - ignore
        if (info->path[info->name_start] == '.') continue;
        // if it's a dir or link to a dir - store it.
        if (((info->type == EINA_FILE_LNK) && (ecore_file_is_dir(info->path))) ||
            (info->type == EINA_FILE_DIR))
          {
             // store just the name, not the full path
             files = eina_list_append
               (files, eina_stringshare_add(info->path + info->name_start));
          }
     }
   eina_iterator_free(it);
   
   // now convert our temporary list into an array of stringshare strings
   cd->dirs_count = eina_list_count(files);
   if (cd->dirs_count > 0)
     {
        cd->dirs = malloc(cd->dirs_count * sizeof(char *));
        EINA_LIST_FREE(files, file)
          {
             cd->dirs[i] = file;
             i++;
          }
     }
   // add cache dir to hash with full path as key
   eina_hash_add(subdir_cache->dirs, path, cd);
   // mark subdirs as needing a save - something changed
   subdir_need_save = EINA_TRUE;
   return cd;
}

static Eina_Bool
icon_cache_update_cache_cb(void *data EINA_UNUSED)
{
   Eina_Strbuf *file = eina_strbuf_new();
   if (!file) return EINA_FALSE;

   icon_cache_timer = NULL;

   if (icon_cache_exe)
     {
        icon_queue = EINA_TRUE;
        eina_strbuf_free(file);
        return ECORE_CALLBACK_CANCEL;
     }
   icon_queue = EINA_FALSE;
   if ((!icon_flush) && (!icon_exts))
     {
        eina_strbuf_free(file);
        return ECORE_CALLBACK_CANCEL;
     }

   if (icon_change_monitors) eina_hash_free(icon_change_monitors);
   if (icon_change_monitors_mon) eina_hash_free(icon_change_monitors_mon);
   icon_change_monitors = eina_hash_string_superfast_new
     (EINA_FREE_CB(eio_monitor_del));
   icon_change_monitors_mon = eina_hash_pointer_new(NULL);
   icon_changes_listen();
   subdir_cache_save();

   /* TODO: Queue if already running */
   eina_strbuf_append_printf(file, "%s/efreet/" MODULE_ARCH "/efreet_icon_cache_create",
                             eina_prefix_lib_get(pfx));
   if (icon_extra_dirs)
     {
        Eina_List *ll;
        char *p;

        eina_strbuf_append(file, " -d");
        EINA_LIST_FOREACH(icon_extra_dirs, ll, p)
          {
             eina_strbuf_append(file, " ");
             eina_strbuf_append(file, p);
          }
     }
   if (icon_exts)
     {
        Eina_List *ll;
        char *p;

        eina_strbuf_append(file, " -e");
        EINA_LIST_FOREACH(icon_exts, ll, p)
          {
             eina_strbuf_append(file, " ");
             eina_strbuf_append(file, p);
          }
     }
   if (icon_flush)
     eina_strbuf_append(file, " -f");
   icon_flush = EINA_FALSE;
   icon_cache_exe =
      ecore_exe_pipe_run(eina_strbuf_string_get(file), ECORE_EXE_PIPE_READ |
                                                       ECORE_EXE_PIPE_READ_LINE_BUFFERED,
                                                       NULL);

   eina_strbuf_free(file);

   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
desktop_cache_update_cache_cb(void *data EINA_UNUSED)
{
   Eina_Strbuf *file;

   desktop_cache_timer = NULL;

   if (desktop_cache_exe)
     {
        desktop_queue = EINA_TRUE;
        return ECORE_CALLBACK_CANCEL;
     }
   desktop_queue = EINA_FALSE;
   file = eina_strbuf_new();

   if (desktop_change_monitors) eina_hash_free(desktop_change_monitors);
   if (desktop_change_monitors_mon) eina_hash_free(desktop_change_monitors_mon);
   desktop_change_monitors = eina_hash_string_superfast_new
     (EINA_FREE_CB(eio_monitor_del));
   desktop_change_monitors_mon = eina_hash_pointer_new(NULL);
   desktop_changes_listen();
   subdir_cache_save();

   eina_strbuf_append_printf(file, "%s/efreet/" MODULE_ARCH "/efreet_desktop_cache_create",
                            eina_prefix_lib_get(pfx));
   if (desktop_extra_dirs)
     {
        Eina_List *ll;
        const char *str;

        eina_strbuf_append(file, " -d");
        EINA_LIST_FOREACH(desktop_extra_dirs, ll, str)
          {
             eina_strbuf_append(file, " ");
             eina_strbuf_append(file, str);
          }
     }
   INF("Run desktop cache creation: %s", eina_strbuf_string_get(file));
   desktop_cache_exe = ecore_exe_pipe_run
     (eina_strbuf_string_get(file), ECORE_EXE_PIPE_READ |
                                    ECORE_EXE_PIPE_READ_LINE_BUFFERED,
                                    NULL);

   eina_strbuf_free(file);

   return ECORE_CALLBACK_CANCEL;
}

static void
cache_icon_update(Eina_Bool flush)
{
   if (icon_cache_timer) ecore_timer_del(icon_cache_timer);
   if (flush) icon_flush = flush;
   icon_cache_timer = ecore_timer_add(0.2, icon_cache_update_cache_cb, NULL);
}

void
cache_desktop_update(void)
{
   if (desktop_cache_timer) ecore_timer_del(desktop_cache_timer);
   desktop_cache_timer = ecore_timer_add(0.2, desktop_cache_update_cache_cb, NULL);
}

static Eina_Bool
_cb_monitor_event(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Eio_Monitor_Event *ev = event;

   // if it's an icon
   if (eina_hash_find(icon_change_monitors_mon, &(ev->monitor)))
     {
        cache_icon_update(EINA_FALSE);
     }
   // if it's a desktop
   else if (eina_hash_find(desktop_change_monitors_mon, &(ev->monitor)))
     {
        cache_desktop_update();
     }
   // if it's a mime file
   else if (eina_hash_find(mime_monitors_mon, &(ev->monitor)))
     {
        if ((!strcmp("/etc/mime.types", ev->filename)) ||
            (!strcmp("globs", ecore_file_file_get(ev->filename))))
          {
             mime_cache_shutdown();
             mime_cache_init();
             if (mime_update_timer) ecore_timer_del(mime_update_timer);
             mime_update_timer = ecore_timer_add(0.2, mime_update_cache_cb, NULL);
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}

static void
icon_changes_monitor_add(const struct stat *st, const char *path)
{
   Eio_Monitor *mon;
   char *realp = NULL;
   const char *monpath = path;

   if (eina_hash_find(icon_change_monitors, path)) return;
#ifndef _WIN32
   if (S_ISLNK(st->st_mode))
     {
        realp = ecore_file_realpath(path);
        if (!realp) return;
        monpath = realp;
     }
#endif
   if (ecore_file_is_dir(monpath))
     {
        mon = eio_monitor_add(monpath);
        if (mon)
          {
             eina_hash_add(icon_change_monitors, path, mon);
             eina_hash_add(icon_change_monitors_mon, &mon, mon);
          }
     }
   free(realp);
}

static void
desktop_changes_monitor_add(const struct stat *st, const char *path)
{
   Eio_Monitor *mon;
   char *realp = NULL;
   const char *monpath = path;

   if (eina_hash_find(desktop_change_monitors, path)) return;
#ifndef _WIN32
   if (S_ISLNK(st->st_mode))
     {
        realp = ecore_file_realpath(path);
        if (!realp) return;
        monpath = realp;
     }
#endif
   if (ecore_file_is_dir(monpath))
     {
        mon = eio_monitor_add(monpath);
        if (mon)
          {
             eina_hash_add(desktop_change_monitors, path, mon);
             eina_hash_add(desktop_change_monitors_mon, &mon, mon);
          }
     }
   free(realp);
}

static int
stat_cmp(const void *a, const void *b)
{
   const struct stat *st1 = a;
   const struct stat *st2 = b;

   if ((st2->st_dev == st1->st_dev) && (st2->st_ino == st1->st_ino))
     return 0;
   return 1;
}

static Eina_Bool
_check_recurse_monitor_sanity(Eina_Inarray *stack, const char *path, unsigned int stack_limit)
{
   const char *home = eina_environment_home_get();

   // protect against too deep recursion even if it's valid.
   if (eina_inarray_count(stack) >= stack_limit)
     {
        ERR("Recursing too far. Level %i. Stopping at %s\n", stack_limit, path);
        return EINA_FALSE;
     }
   // detect if we start recursing at $HOME - a sign of something wrong
   if ((home) && (!strcmp(home, path)))
     {
        ERR("Recursively monitor homedir! Ignore.");
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static void
icon_changes_listen_recursive(Eina_Inarray *stack, const char *path, Eina_Bool base)
{
   struct stat *st = eina_mempool_malloc(efreetd_mp_stat, sizeof(struct stat));
   if (!st) return;

   if (stat(path, st) == -1) return;
   if (eina_inarray_search(stack, st, stat_cmp) >= 0) return;
   if (!_check_recurse_monitor_sanity(stack, path, 10)) return;
   eina_inarray_push(stack, st);

   if ((!S_ISDIR(st->st_mode)) && (base))
     {
        // XXX: if it doesn't exist... walk the parent dirs back down
        // to this path until we find one that doesn't exist, then
        // monitor its parent, and treat it specially as it needs
        // to look for JUST the creation of this specific child
        // and when this child is created, replace this monitor with
        // monitoring the next specific child dir down until we are
        // monitoring the original path again.
     }
   if (S_ISDIR(st->st_mode))
     {
        unsigned int i;
        const Subdir_Cache_Dir *cd = subdir_cache_get(st, path);
        icon_changes_monitor_add(st, path);
        if (cd)
          {
             Eina_Strbuf *buf = eina_strbuf_new();
             if (!buf) return;
             for (i = 0; i < cd->dirs_count; i++)
               {

                  eina_strbuf_append_printf(buf,  "%s/%s", path, cd->dirs[i]);
                  icon_changes_listen_recursive(stack, eina_strbuf_string_get(buf), EINA_FALSE);
                  eina_strbuf_reset(buf);
               }
             eina_strbuf_free(buf);
          }
     }
   eina_inarray_pop(stack);
   eina_mempool_free(efreetd_mp_stat, st);
}

static void
desktop_changes_listen_recursive(Eina_Inarray *stack, const char *path, Eina_Bool base)
{
   struct stat *st = eina_mempool_malloc(efreetd_mp_stat, sizeof(struct stat));
   if (!st) return;

   if (stat(path, st) == -1) return;
   if (eina_inarray_search(stack, st, stat_cmp) >= 0) return;
   if (!_check_recurse_monitor_sanity(stack, path, 10)) return;
   eina_inarray_push(stack, st);

   if ((!S_ISDIR(st->st_mode)) && (base))
     {
        // XXX: if it doesn't exist... walk the parent dirs back down
        // to this path until we find one that doesn't exist, then
        // monitor its parent, and treat it specially as it needs
        // to look for JUST the creation of this specific child
        // and when this child is created, replace this monitor with
        // monitoring the next specific child dir down until we are
        // monitoring the original path again.
     }
   if (S_ISDIR(st->st_mode))
     {
        unsigned int i;
        const Subdir_Cache_Dir *cd = subdir_cache_get(st, path);
        desktop_changes_monitor_add(st, path);
        if (cd)
          {
             Eina_Strbuf *buf = eina_strbuf_new();
             if (!buf) return;
             for (i = 0; i < cd->dirs_count; i++)
               {
                  eina_strbuf_append_printf(buf, "%s/%s", path, cd->dirs[i]);
                  desktop_changes_listen_recursive(stack, eina_strbuf_string_get(buf), EINA_FALSE);
                  eina_strbuf_reset(buf);
               }
             eina_strbuf_free(buf);
          }
     }
   eina_inarray_pop(stack);
   eina_mempool_free(efreetd_mp_stat, st);
}

static void
icon_changes_listen(void)
{
   Eina_List *l;
   Eina_List *xdg_dirs;
   const char *dir;
   Eina_Inarray *stack;
   Eina_Strbuf *buf = eina_strbuf_new();
   if (!buf) return;

   stack = eina_inarray_new(sizeof(struct stat), 16);
   if (!stack)
     {
        eina_strbuf_free(buf);
        return;
     }
   icon_changes_listen_recursive(stack, efreet_icon_deprecated_user_dir_get(), EINA_TRUE);
   eina_inarray_flush(stack);
   icon_changes_listen_recursive(stack, efreet_icon_user_dir_get(), EINA_TRUE);
   EINA_LIST_FOREACH(icon_extra_dirs, l, dir)
     {
        if (!strcmp(dir, "/")) continue;
        eina_inarray_flush(stack);
        icon_changes_listen_recursive(stack, dir, EINA_TRUE);
     }

   xdg_dirs = efreet_data_dirs_get();
   EINA_LIST_FOREACH(xdg_dirs, l, dir)
     {
        eina_strbuf_append_printf(buf, "%s/icons", dir);
        eina_inarray_flush(stack);
        icon_changes_listen_recursive(stack, eina_strbuf_string_get(buf), EINA_TRUE);
        eina_strbuf_reset(buf);
     }

#ifndef STRICT_SPEC
   EINA_LIST_FOREACH(xdg_dirs, l, dir)
     {
        eina_strbuf_append_printf(buf, "%s/pixmaps", dir);
        eina_inarray_flush(stack);
        icon_changes_listen_recursive(stack, eina_strbuf_string_get(buf), EINA_TRUE);
        eina_strbuf_reset(buf);
     }
#endif
   eina_inarray_flush(stack);
   icon_changes_listen_recursive(stack, "/usr/share/pixmaps", EINA_TRUE);
   eina_inarray_free(stack);
   eina_strbuf_free(buf);
}

static void
desktop_changes_listen(void)
{
   Eina_List *l;
   const char *path;
   Eina_Inarray *stack;

   stack = eina_inarray_new(sizeof(struct stat), 16);
   if (!stack) return;
   EINA_LIST_FOREACH(desktop_system_dirs, l, path)
     {
        eina_inarray_flush(stack);
        desktop_changes_listen_recursive(stack, path, EINA_TRUE);
     }
   EINA_LIST_FOREACH(desktop_extra_dirs, l, path)
     {
        eina_inarray_flush(stack);
        desktop_changes_listen_recursive(stack, path, EINA_TRUE);
     }
   eina_inarray_free(stack);
}

static void
fill_list(const char *file, Eina_List **l)
{
   Eina_File *f = NULL;
   Eina_Iterator *it = NULL;
   Eina_File_Line *line = NULL;
   Eina_Strbuf *buf = eina_strbuf_new();
   if (!buf) return;

   eina_strbuf_append_printf(buf, "%s/efreet/%s", efreet_cache_home_get(), file);
   f = eina_file_open(eina_strbuf_string_get(buf), EINA_FALSE);
   if (!f) goto error_buf;
   it = eina_file_map_lines(f);
   if (!it) goto error;
   EINA_ITERATOR_FOREACH(it, line)
     {
        if (line->end > line->start)
          {
             const char *s = eina_stringshare_add_length(line->start, line->end - line->start);
             if (s) *l = eina_list_append(*l, s);
          }
     }
   eina_iterator_free(it);
error:
   eina_file_close(f);
error_buf:
   eina_strbuf_free(buf);
}

static void
read_lists(void)
{
// dont use extra dirs as the only way to get extra dirs is by loading a
// specific desktop file at a specific path, and this is wrong
//   fill_list("extra_desktops.dirs", &desktop_extra_dirs);
   fill_list("extra_icons.dirs", &icon_extra_dirs);
   fill_list("icons.exts", &icon_exts);
}

static void
save_list(const char *file, Eina_List *l)
{
   FILE *f;
   Eina_List *ll;
   const char *path;
   Eina_Strbuf *buf = eina_strbuf_new();
   if (!buf) return;

   eina_strbuf_append_printf(buf, "%s/efreet/%s", efreet_cache_home_get(), file);
   f = fopen(eina_strbuf_string_get(buf), "wb");
   if (!f)
     {
        eina_strbuf_free(buf);
        return;
     }
   EINA_LIST_FOREACH(l, ll, path)
      fprintf(f, "%s\n", path);
   fclose(f);
   eina_strbuf_free(buf);
}

static int
strcmplen(const void *data1, const void *data2)
{
   return strncmp(data1, data2, eina_stringshare_strlen(data1));
}

static Eina_Bool
cache_exe_data_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Exe_Event_Data *ev = event;

   if (ev->exe == desktop_cache_exe)
     {
        Eina_Bool update = EINA_FALSE;

        if ((ev->lines) && (*ev->lines->line == 'c')) update = EINA_TRUE;
        if (!desktop_exists)
          send_signal_desktop_cache_build();
        desktop_exists = EINA_TRUE;
        send_signal_desktop_cache_update(update);
     }
   else if (ev->exe == icon_cache_exe)
     {
        Eina_Bool update = EINA_FALSE;

        if ((ev->lines) && (*ev->lines->line == 'c')) update = EINA_TRUE;
        send_signal_icon_cache_update(update);
     }
   else if (ev->exe == mime_cache_exe)
     {
        // XXX: ZZZ: handle stdout here from cache updater... if needed
     }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
cache_exe_del_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Exe_Event_Del *ev = event;

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
   else if (ev->exe == mime_cache_exe)
     {
        mime_cache_exe = NULL;
        send_signal_mime_cache_build();
     }
   return ECORE_CALLBACK_RENEW;
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
        if (!eina_hash_find(desktop_change_monitors, path))
          cache_desktop_update();
     }
   else if (!eina_list_search_unsorted_list(desktop_extra_dirs, EINA_COMPARE_CB(strcmp), san))
     {
        /* Not a registered path */
        desktop_extra_dirs = eina_list_append(desktop_extra_dirs, eina_stringshare_add(san));
        save_list("extra_desktops.dirs", desktop_extra_dirs);
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
        if (!strcmp(san, "/")) goto out;
        icon_extra_dirs = eina_list_append(icon_extra_dirs, eina_stringshare_add(san));
        save_list("extra_icons.dirs", icon_extra_dirs);
        cache_icon_update(EINA_TRUE);
     }
out:
   free(san);
}

void
cache_icon_ext_add(const char *ext)
{
   if (!eina_list_search_unsorted_list(icon_exts, EINA_COMPARE_CB(strcmp), ext))
     {
        icon_exts = eina_list_append(icon_exts, eina_stringshare_add(ext));
        save_list("icons.exts", icon_exts);
        cache_icon_update(EINA_TRUE);
     }
}

Eina_Bool
cache_desktop_exists(void)
{
   return desktop_exists;
}

static void
mime_update_launch(void)
{
   Eina_Strbuf *file = eina_strbuf_new();
   if (!file) return;

   eina_strbuf_append_printf(file,
            "%s/efreet/" MODULE_ARCH "/efreet_mime_cache_create",
            eina_prefix_lib_get(pfx));
   mime_cache_exe = ecore_exe_pipe_run(eina_strbuf_string_get(file),
                                       ECORE_EXE_PIPE_READ |
                                       ECORE_EXE_PIPE_READ_LINE_BUFFERED,
                                       NULL);
   eina_strbuf_free(file);
}

static Eina_Bool
mime_update_cache_cb(void *data EINA_UNUSED)
{
   mime_update_timer = NULL;
   if (mime_cache_exe)
     {
        ecore_exe_kill(mime_cache_exe);
        ecore_exe_free(mime_cache_exe);
     }
   mime_update_launch();
   return EINA_FALSE;
}

static void
mime_cache_init(void)
{
   Eio_Monitor *mon;
   Eina_List *datadirs, *l;
   const char *s;
   Eina_Strbuf *buf = eina_strbuf_new();
   if (!buf) return;

   mime_monitors = eina_hash_string_superfast_new
     (EINA_FREE_CB(eio_monitor_del));
   mime_monitors_mon = eina_hash_pointer_new(NULL);

   if (ecore_file_is_dir("/etc"))
     {
        mon = eio_monitor_add("/etc"); // specifically look at /etc/mime.types
        if (mon)
          {
             eina_hash_add(mime_monitors, "/etc", mon);
             eina_hash_add(mime_monitors_mon, &mon, mon);
          }
     }
   if (ecore_file_is_dir("/usr/share/mime"))
     {
        mon = eio_monitor_add("/usr/share/mime"); // specifically look at /usr/share/mime/globs
        if (mon)
          {
             eina_hash_add(mime_monitors, "/usr/share/mime", mon);
             eina_hash_add(mime_monitors_mon, &mon, mon);
          }
     }

   datadirs = efreet_data_dirs_get();
   EINA_LIST_FOREACH(datadirs, l, s)
     {
        eina_strbuf_append_printf(buf, "%s/mime", s); // specifically lok at XXX/mime/globs
        if (ecore_file_is_dir(eina_strbuf_string_get(buf)))
          {
             if (!eina_hash_find(mime_monitors, eina_strbuf_string_get(buf)))
               {
                  mon = eio_monitor_add(eina_strbuf_string_get(buf));
                  if (mon)
                    {
                       eina_hash_add(mime_monitors, eina_strbuf_string_get(buf), mon);
                       eina_hash_add(mime_monitors_mon, &mon, mon);
                    }
               }
          }
     }
   eina_strbuf_free(buf);
}

static void
mime_cache_shutdown(void)
{
   if (mime_update_timer)
     {
        ecore_timer_del(mime_update_timer);
        mime_update_timer = NULL;
     }
   if (mime_monitors)
     {
        eina_hash_free(mime_monitors);
        mime_monitors = NULL;
     }
   if (mime_monitors_mon)
     {
        eina_hash_free(mime_monitors_mon);
        mime_monitors_mon = NULL;
     }
}

Eina_Bool
cache_init(void)
{
   char **argv;

   ecore_app_args_get(NULL, &argv);

   pfx = eina_prefix_new(argv[0], cache_init,
                         "EFREET", "efreet", "checkme",
                         PACKAGE_BIN_DIR,
                         PACKAGE_LIB_DIR,
                         PACKAGE_DATA_DIR,
                         PACKAGE_DATA_DIR);

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

   icon_change_monitors = eina_hash_string_superfast_new
     (EINA_FREE_CB(eio_monitor_del));
   icon_change_monitors_mon = eina_hash_pointer_new(NULL);
   desktop_change_monitors = eina_hash_string_superfast_new
     (EINA_FREE_CB(eio_monitor_del));
   desktop_change_monitors_mon = eina_hash_pointer_new(NULL);

   efreet_cache_update = 0;
   if (!efreet_init()) goto error;
   eio_init();

#define MONITOR_EVENT(ev, fn) \
_handlers = eina_list_append(_handlers, ecore_event_handler_add(ev, fn, NULL))
   MONITOR_EVENT(EIO_MONITOR_FILE_CREATED,       _cb_monitor_event);
   MONITOR_EVENT(EIO_MONITOR_FILE_DELETED,       _cb_monitor_event);
   MONITOR_EVENT(EIO_MONITOR_FILE_MODIFIED,      _cb_monitor_event);
   MONITOR_EVENT(EIO_MONITOR_DIRECTORY_CREATED,  _cb_monitor_event);
   MONITOR_EVENT(EIO_MONITOR_DIRECTORY_DELETED,  _cb_monitor_event);
   MONITOR_EVENT(EIO_MONITOR_DIRECTORY_MODIFIED, _cb_monitor_event);
   MONITOR_EVENT(EIO_MONITOR_SELF_RENAME,        _cb_monitor_event);
   MONITOR_EVENT(EIO_MONITOR_SELF_DELETED,       _cb_monitor_event);

   subdir_cache_init();
   mime_cache_init();
   mime_update_launch();
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
   subdir_cache_save();

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
   Ecore_Event_Handler *handler;

   eina_prefix_free(pfx);
   pfx = NULL;

   mime_cache_shutdown();
   subdir_cache_shutdown();
   efreet_shutdown();

   if (cache_exe_del_handler) ecore_event_handler_del(cache_exe_del_handler);
   cache_exe_del_handler = NULL;
   if (cache_exe_data_handler) ecore_event_handler_del(cache_exe_data_handler);
   cache_exe_data_handler = NULL;

   if (icon_change_monitors) eina_hash_free(icon_change_monitors);
   icon_change_monitors = NULL;
   if (icon_change_monitors_mon) eina_hash_free(icon_change_monitors_mon);
   icon_change_monitors_mon = NULL;
   if (desktop_change_monitors) eina_hash_free(desktop_change_monitors);
   desktop_change_monitors = NULL;
   if (desktop_change_monitors_mon) eina_hash_free(desktop_change_monitors_mon);
   desktop_change_monitors_mon = NULL;
   EINA_LIST_FREE(desktop_system_dirs, data)
      eina_stringshare_del(data);
   EINA_LIST_FREE(desktop_extra_dirs, data)
      eina_stringshare_del(data);
   EINA_LIST_FREE(icon_extra_dirs, data)
      eina_stringshare_del(data);
   EINA_LIST_FREE(icon_exts, data)
      eina_stringshare_del(data);
   EINA_LIST_FREE(_handlers, handler)
      ecore_event_handler_del(handler);
   eio_shutdown();
   return EINA_TRUE;
}
