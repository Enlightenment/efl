#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef USE_UNSTABLE_LIBMOUNT_API
# define USE_UNSTABLE_LIBMOUNT_API 1
#endif

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <Ecore.h>
#include <Eeze.h>
#include <Eeze_Disk.h>
#ifdef HAVE_EEZE_MOUNT
# include <libmount.h>
#endif
#include "eeze_udev_private.h"
#include "eeze_disk_private.h"

/*
 *
 * PRIVATE
 *
 */

static struct libmnt_optmap eeze_optmap[] =
{
   { "loop[=]", EEZE_DISK_MOUNTOPT_LOOP, 0 },
   { "utf8", EEZE_DISK_MOUNTOPT_UTF8, 0 },
   { "noexec", EEZE_DISK_MOUNTOPT_NOEXEC, 0 },
   { "nosuid", EEZE_DISK_MOUNTOPT_NOSUID, 0 },
   { "remount", EEZE_DISK_MOUNTOPT_REMOUNT, 0 },
   { "uid[=]", EEZE_DISK_MOUNTOPT_UID, 0 },
   { "nodev", EEZE_DISK_MOUNTOPT_NODEV, 0 },
   { NULL, 0, 0 }
};
typedef struct libmnt_table libmnt_table;
typedef struct libmnt_fs libmnt_fs;
typedef struct libmnt_cache libmnt_cache;
static Ecore_File_Monitor *_fstab_mon = NULL;
static Eina_Bool _watching = EINA_FALSE;
static Eina_Bool _fstab_scan_active = EINA_FALSE;
static libmnt_cache *_eeze_mount_mtab_cache = NULL;
static libmnt_cache *_eeze_mount_fstab_cache = NULL;
static libmnt_table *_eeze_mount_mtab = NULL;
static libmnt_table *_eeze_mount_fstab = NULL;
extern Eina_List *_eeze_disks;

static Ecore_Fd_Handler *_mountinfo_fdh = NULL;
static int _mountinfo = -1;

static libmnt_table *_eeze_mount_tab_parse(const char *filename);
static void _eeze_mount_tab_watcher(void *data, Ecore_File_Monitor *mon EINA_UNUSED, Ecore_File_Event event EINA_UNUSED, const char *path);

static int
_eeze_mount_tab_parse_errcb(libmnt_table *tab EINA_UNUSED, const char *filename, int line)
{
   ERR("%s:%d: could not parse line!", filename, line); /* most worthless error reporting ever. */
   return -1;
}

/*
 * I could use mnt_new_table_from_file() but this way gives much more detailed output
 * on failure so why not
 */
static libmnt_table *
_eeze_mount_tab_parse(const char *filename)
{
   libmnt_table *tab;

   if (!(tab = mnt_new_table())) return NULL;
   if (mnt_table_set_parser_errcb(tab, _eeze_mount_tab_parse_errcb))
     {
        ERR("Alloc!");
        mnt_free_table(tab);
        return NULL;
     }

   if (!mnt_table_parse_file(tab, filename))
     return tab;

   mnt_free_table(tab);
   return NULL;
}

static void
_eeze_mount_tab_watcher(void *data, Ecore_File_Monitor *mon EINA_UNUSED, Ecore_File_Event event EINA_UNUSED, const char *path)
{
   libmnt_table *bak;

   if (_fstab_scan_active)
     /* prevent scans from triggering a scan */
     return;

   bak = _eeze_mount_mtab;
   _eeze_mount_mtab = _eeze_mount_tab_parse(path);
   if (!_eeze_mount_mtab)
     {
        ERR("Could not parse %s! keeping old tab...", path);
        goto error;
     }

   mnt_free_table(bak);
   if (data)
     {
        mnt_free_cache(_eeze_mount_mtab_cache);
        _eeze_mount_mtab_cache = mnt_new_cache();
        mnt_table_set_cache(_eeze_mount_mtab, _eeze_mount_mtab_cache);
     }
   else
     {
        mnt_free_cache(_eeze_mount_fstab_cache);
        _eeze_mount_fstab_cache = mnt_new_cache();
        mnt_table_set_cache(_eeze_mount_fstab, _eeze_mount_fstab_cache);
     }
   return;

error:
   mnt_free_table(_eeze_mount_mtab);
   _eeze_mount_mtab = bak;
}

/* on tab change, check differences
 * based on code from findmnt
 */
static Eina_Bool
_eeze_mount_fdh(void *d EINA_UNUSED, Ecore_Fd_Handler *fdh EINA_UNUSED)
{
   libmnt_table *tb_new;
   libmnt_fs *old, *new;
   int change;
   struct libmnt_iter *itr = NULL;
   struct libmnt_tabdiff *diff = NULL;

   tb_new = mnt_new_table();
   EINA_SAFETY_ON_NULL_RETURN_VAL(tb_new, ECORE_CALLBACK_RENEW);
   EINA_SAFETY_ON_TRUE_GOTO(mnt_table_set_parser_errcb(tb_new, _eeze_mount_tab_parse_errcb), err);
   itr = mnt_new_iter(MNT_ITER_BACKWARD);
   EINA_SAFETY_ON_NULL_GOTO(itr, err);
   diff = mnt_new_tabdiff();
   EINA_SAFETY_ON_NULL_GOTO(diff, err);
   if (mnt_table_parse_file(tb_new, "/proc/self/mountinfo"))
     {
        ERR("PARSING FAILED FOR /proc/self/mountinfo! THIS IS WEIRD!");
        goto err;
     }
   change = mnt_diff_tables(diff, _eeze_mount_mtab, tb_new);
   if (change < 0)
     {
        ERR("DIFFING FAILED FOR /proc/self/mountinfo! THIS IS ALSO WEIRD!");
        goto err;
     }
   if (!change) goto err;
   while (!mnt_tabdiff_next_change(diff, itr, &old, &new, &change))
     {
        const char *src;
        Eeze_Disk *disk;
        Eina_Bool found =  EINA_FALSE;
        Eeze_Event_Disk_Mount *e;
        Eina_List *l;

        src = mnt_fs_get_source(new);
        if (!src) continue;
        EINA_LIST_FOREACH(_eeze_disks, l, disk)
          {
             if (!strcmp(src, eeze_disk_devpath_get(disk)))
               {
                  found = EINA_TRUE;
                  break;
               }
          }
        if (!found) continue;
        switch (change)
          {
           case MNT_TABDIFF_MOUNT:
             disk->mounted = EINA_TRUE;
             eina_stringshare_replace(&disk->mount_point, mnt_fs_get_target(new));
             if (disk->mount_status) break;
             e = malloc(sizeof(Eeze_Event_Disk_Mount));
             if (e)
               {
                  e->disk = disk;
                  ecore_event_add(EEZE_EVENT_DISK_MOUNT, e, NULL, NULL);
               }
             break;
           case MNT_TABDIFF_UMOUNT:
             if (!mnt_fs_get_target(new))
               disk->mounted = EINA_FALSE;
             eina_stringshare_replace(&disk->mount_point, NULL);
             if (disk->mount_status) break;
             e = malloc(sizeof(Eeze_Event_Disk_Mount));
             if (e)
               {
                  e->disk = disk;
                  ecore_event_add(EEZE_EVENT_DISK_UNMOUNT, e, NULL, NULL);
               }
             break;
           /* anything could have happened here, send both events to flush */
           case MNT_TABDIFF_REMOUNT:
           case MNT_TABDIFF_MOVE:
             if (!mnt_fs_get_target(new))
               disk->mounted = EINA_FALSE;
             eina_stringshare_replace(&disk->mount_point, mnt_fs_get_target(new));
             if (disk->mount_status) break;
             e = malloc(sizeof(Eeze_Event_Disk_Mount));
             if (e)
               {
                  e->disk = disk;
                  ecore_event_add(EEZE_EVENT_DISK_UNMOUNT, e, NULL, NULL);
               }
             e = malloc(sizeof(Eeze_Event_Disk_Mount));
             if (e)
               {
                  e->disk = disk;
                  ecore_event_add(EEZE_EVENT_DISK_MOUNT, e, NULL, NULL);
               }
           default:
             break;
          }
     }

   mnt_free_cache(_eeze_mount_mtab_cache);
   _eeze_mount_mtab_cache = mnt_new_cache();
   mnt_table_set_cache(_eeze_mount_mtab, _eeze_mount_mtab_cache);
   mnt_free_table(_eeze_mount_mtab);
   _eeze_mount_mtab = tb_new;
   return ECORE_CALLBACK_RENEW;
err:
   if (tb_new) mnt_free_table(tb_new);
   if (itr) mnt_free_iter(itr);
   if (diff) mnt_free_tabdiff(diff);
   return ECORE_CALLBACK_RENEW;
}

/*
 *
 * INVISIBLE
 *
 */

Eina_Bool
eeze_libmount_init(void)
{
   /* placeholder */
   return EINA_TRUE;
}

void
eeze_libmount_shutdown(void)
{
   if (_eeze_mount_fstab)
     {
        mnt_free_table(_eeze_mount_fstab);
        mnt_free_cache(_eeze_mount_fstab_cache);
     }
   if (_eeze_mount_mtab)
     {
        mnt_free_table(_eeze_mount_mtab);
        mnt_free_cache(_eeze_mount_mtab_cache);
     }
   eeze_mount_tabs_unwatch();
}

unsigned long
eeze_disk_libmount_opts_get(Eeze_Disk *disk)
{
   libmnt_fs *mnt;
   const char *opts;
   unsigned long f = 0;

   if (!eeze_mount_mtab_scan() || !eeze_mount_fstab_scan())
     return 0;

   mnt = mnt_table_find_tag(_eeze_mount_mtab, "UUID", eeze_disk_uuid_get(disk), MNT_ITER_BACKWARD);
   if (!mnt)
     mnt = mnt_table_find_tag(_eeze_mount_fstab, "UUID", eeze_disk_uuid_get(disk), MNT_ITER_BACKWARD);

   if (!mnt) return 0;

   opts = mnt_fs_get_fs_options(mnt);
   if (!opts) return 0;
   if (!mnt_optstr_get_flags(opts, &f, eeze_optmap)) return 0;
   return f;
}

/*
 * helper function to return whether a disk is mounted
 */
Eina_Bool
eeze_disk_libmount_mounted_get(Eeze_Disk *disk)
{
   libmnt_fs *mnt;

   if (!disk)
     return EINA_FALSE;

   if (!eeze_mount_mtab_scan() || !eeze_mount_fstab_scan())
     return EINA_FALSE;

   mnt = mnt_table_find_source(_eeze_mount_mtab, eeze_disk_devpath_get(disk), MNT_ITER_BACKWARD);
   if (!mnt)
     {
        disk->mounted = EINA_FALSE;
        return EINA_FALSE;
     }

   eina_stringshare_replace(&disk->mount_point, mnt_fs_get_target(mnt));
   disk->mounted = EINA_TRUE;
   return EINA_TRUE;
}


/*
 * helper function to return the device that is mounted at a mount point
 */
const char *
eeze_disk_libmount_mp_find_source(const char *mount_point)
{
   libmnt_fs *mnt;

   if (!mount_point)
     return NULL;

   if (!eeze_mount_mtab_scan() || !eeze_mount_fstab_scan())
     return NULL;

   mnt = mnt_table_find_target(_eeze_mount_mtab, mount_point, MNT_ITER_BACKWARD);
   if (!mnt)
     mnt = mnt_table_find_target(_eeze_mount_fstab, mount_point, MNT_ITER_BACKWARD);

   if (!mnt)
     return NULL;

   return mnt_fs_get_source(mnt);
}

/*
 * helper function to return a mount point from a uuid
 */
const char *
eeze_disk_libmount_mp_lookup_by_uuid(const char *uuid)
{
   libmnt_fs *mnt;

   if (!uuid)
     return NULL;

   if (!eeze_mount_mtab_scan() || !eeze_mount_fstab_scan())
     return NULL;

   mnt = mnt_table_find_tag(_eeze_mount_fstab, "UUID", uuid, MNT_ITER_BACKWARD);

   if (!mnt)
     return NULL;

   return mnt_fs_get_target(mnt);
}

/*
 * helper function to return a mount point from a label
 */
const char *
eeze_disk_libmount_mp_lookup_by_label(const char *label)
{
   libmnt_fs *mnt;

   if (!label)
     return NULL;

   if (!eeze_mount_mtab_scan() || !eeze_mount_fstab_scan())
     return NULL;

   mnt = mnt_table_find_tag(_eeze_mount_fstab, "LABEL", label, MNT_ITER_BACKWARD);

   if (!mnt)
     return NULL;

   return mnt_fs_get_target(mnt);
}

/*
 * helper function to return a mount point from a /dev/ path
 */
const char *
eeze_disk_libmount_mp_lookup_by_devpath(const char *devpath)
{
   libmnt_fs *mnt;

   if (!devpath)
     return NULL;

   if (!eeze_mount_mtab_scan() || !eeze_mount_fstab_scan())
     return NULL;

   mnt = mnt_table_find_srcpath(_eeze_mount_mtab, devpath, MNT_ITER_BACKWARD);
   if (!mnt)
     mnt = mnt_table_find_srcpath(_eeze_mount_fstab, devpath, MNT_ITER_BACKWARD);

   if (!mnt)
     return NULL;

   return mnt_fs_get_target(mnt);
}

/*
 *
 * API
 *
 */

EAPI Eina_Bool
eeze_mount_tabs_watch(void)
{
   libmnt_table *bak;

   if (_watching)
     return EINA_TRUE;

   bak = _eeze_mount_tab_parse("/proc/self/mountinfo");
   EINA_SAFETY_ON_NULL_GOTO(bak, error);

   mnt_free_table(_eeze_mount_mtab);
   _eeze_mount_mtab = bak;
   bak = _eeze_mount_tab_parse("/etc/fstab");
   EINA_SAFETY_ON_NULL_GOTO(bak, error);

   mnt_free_table(_eeze_mount_fstab);
   _eeze_mount_fstab = bak;

   _eeze_mount_mtab_cache = mnt_new_cache();
   mnt_table_set_cache(_eeze_mount_mtab, _eeze_mount_mtab_cache);

   _eeze_mount_fstab_cache = mnt_new_cache();
   mnt_table_set_cache(_eeze_mount_fstab, _eeze_mount_fstab_cache);

   _mountinfo = open("/proc/self/mountinfo", O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
   if (_mountinfo < 0) goto error;
   if (fcntl(_mountinfo, F_SETFL, O_NONBLOCK) < 0) goto error;

   _mountinfo_fdh = ecore_main_fd_handler_file_add(_mountinfo, ECORE_FD_ERROR, _eeze_mount_fdh, NULL, NULL, NULL);
   if (!_mountinfo_fdh) goto error;
   _fstab_mon = ecore_file_monitor_add("/etc/fstab", _eeze_mount_tab_watcher, NULL);
   _watching = EINA_TRUE;

  return EINA_TRUE;

error:
   if (_mountinfo >= 0) close(_mountinfo);
   _mountinfo = -1;
   if (!_eeze_mount_mtab)
     ERR("Could not parse /proc/self/mountinfo!");
   else
     {
        ERR("Could not parse /etc/fstab!");
        mnt_free_table(_eeze_mount_mtab);
     }
   _eeze_mount_mtab = _eeze_mount_fstab = NULL;
   return EINA_FALSE;
}

EAPI void
eeze_mount_tabs_unwatch(void)
{
   if (!_watching)
     return;

   ecore_main_fd_handler_del(_mountinfo_fdh);
   ecore_file_monitor_del(_fstab_mon);
   close(_mountinfo);
   _mountinfo = -1;
   _fstab_mon = NULL;
   _watching = EINA_FALSE;
}

EAPI Eina_Bool
eeze_mount_mtab_scan(void)
{
   libmnt_table *bak;

   if (_watching)
     return EINA_TRUE;

   bak = _eeze_mount_tab_parse("/proc/self/mountinfo");
   if (!bak)
     goto error;
   if (_eeze_mount_mtab)
     {
        mnt_free_table(_eeze_mount_mtab);
        mnt_free_cache(_eeze_mount_mtab_cache);
     }
   _eeze_mount_mtab = bak;
   _eeze_mount_mtab_cache = mnt_new_cache();
   mnt_table_set_cache(_eeze_mount_mtab, _eeze_mount_mtab_cache);

   return EINA_TRUE;

error:
   return EINA_FALSE;
}

EAPI Eina_Bool
eeze_mount_fstab_scan(void)
{
   libmnt_table *bak;
   if (_watching)
     return EINA_TRUE;

   bak = _eeze_mount_tab_parse("/etc/fstab");
   if (!bak)
     goto error;
   if (_eeze_mount_fstab)
     {
        mnt_free_table(_eeze_mount_fstab);
        mnt_free_cache(_eeze_mount_fstab_cache);
     }
   _eeze_mount_fstab = bak;
   _eeze_mount_fstab_cache = mnt_new_cache();
   mnt_table_set_cache(_eeze_mount_fstab, _eeze_mount_fstab_cache);

   return EINA_TRUE;

error:
   return EINA_FALSE;
}
