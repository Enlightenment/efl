#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef USE_UNSTABLE_LIBMOUNT_API
# define USE_UNSTABLE_LIBMOUNT_API 1
#endif

#include <Ecore.h>
#include <Eeze.h>
#include <Eeze_Disk.h>
#include <libmount.h>
#include <unistd.h>

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
   { NULL, 0, 0 }
};
typedef struct libmnt_table libmnt_table;
typedef struct libmnt_lock libmnt_lock;
typedef struct libmnt_fs libmnt_fs;
typedef struct libmnt_cache libmnt_cache;
static Ecore_File_Monitor *_mtab_mon = NULL;
static Ecore_File_Monitor *_fstab_mon = NULL;
static Eina_Bool _watching = EINA_FALSE;
static Eina_Bool _mtab_scan_active = EINA_FALSE;
static Eina_Bool _mtab_locked = EINA_FALSE;
static Eina_Bool _fstab_scan_active = EINA_FALSE;
static libmnt_cache *_eeze_mount_mtab_cache = NULL;
static libmnt_cache *_eeze_mount_fstab_cache = NULL;
static libmnt_table *_eeze_mount_mtab = NULL;
static libmnt_table *_eeze_mount_fstab = NULL;
static libmnt_lock *_eeze_mtab_lock = NULL;
extern Eina_List *_eeze_disks;

static libmnt_table *_eeze_mount_tab_parse(const char *filename);
static void _eeze_mount_tab_watcher(void *data, Ecore_File_Monitor *mon __UNUSED__, Ecore_File_Event event __UNUSED__, const char *path);

static Eina_Bool
_eeze_mount_lock_mtab(void)
{
//    DBG("Locking mlock: %s", mnt_lock_get_linkfile(_eeze_mtab_lock));
    if (EINA_LIKELY(access("/etc/mtab", W_OK)))
      {
         INF("Insufficient privs for mtab lock, continuing without lock");
         return EINA_TRUE;
      }
    if (mnt_lock_file(_eeze_mtab_lock))
     {
        ERR("Couldn't lock mtab!");
        return EINA_FALSE;
     }
   _mtab_locked = EINA_TRUE;
   return EINA_TRUE;
}

static void
_eeze_mount_unlock_mtab(void)
{
//   DBG("Unlocking mlock: %s", mnt_lock_get_linkfile(_eeze_mtab_lock));
   if (_mtab_locked) mnt_unlock_file(_eeze_mtab_lock);
   _mtab_locked = EINA_FALSE;
}


static int
_eeze_mount_tab_parse_errcb(libmnt_table *tab __UNUSED__, const char *filename, int line)
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
_eeze_mount_tab_watcher(void *data, Ecore_File_Monitor *mon __UNUSED__, Ecore_File_Event event __UNUSED__, const char *path)
{
   libmnt_table *bak;

   if (
       ((_mtab_scan_active) && (data)) || /* mtab has non-null data to avoid needing strcmp */
       ((_fstab_scan_active) && (!data))
      )
     /* prevent scans from triggering a scan */
     return;

   bak = _eeze_mount_mtab;
   if (data)
     if (!_eeze_mount_lock_mtab())
       {  /* FIXME: maybe queue job here? */
          ERR("Losing events...");
          return;
       }
   _eeze_mount_mtab = _eeze_mount_tab_parse(path);
   if (data)
     _eeze_mount_unlock_mtab();
   if (!_eeze_mount_mtab)
     {
        ERR("Could not parse %s! keeping old tab...", path);
        goto error;
     }
   if (data)
     {
        Eina_List *l;
        Eeze_Disk *disk;

        /* catch externally initiated mounts on existing disks by comparing known mount state to current state */
        EINA_LIST_FOREACH(_eeze_disks, l, disk)
          {
             Eina_Bool mounted;

             mounted = disk->mounted;

             if ((eeze_disk_libmount_mounted_get(disk) != mounted) && (!disk->mount_status))
               {
                  if (!mounted)
                    {
                        Eeze_Event_Disk_Mount *e;
                        e = malloc(sizeof(Eeze_Event_Disk_Mount));
                        if (e)
                          {
                             e->disk = disk;
                             ecore_event_add(EEZE_EVENT_DISK_MOUNT, e, NULL, NULL);
                          }
                    }
                  else
                    {
                       Eeze_Event_Disk_Unmount *e;
                       e = malloc(sizeof(Eeze_Event_Disk_Unmount));
                       if (e)
                         {
                            e->disk = disk;
                            ecore_event_add(EEZE_EVENT_DISK_UNMOUNT, e, NULL, NULL);
                         }
                    }
               }
          }
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

/*
 *
 * INVISIBLE
 *
 */

Eina_Bool
eeze_libmount_init(void)
{
   if (_eeze_mtab_lock)
     return EINA_TRUE;
   if (!(_eeze_mtab_lock = mnt_new_lock("/etc/mtab", 0)))
     return EINA_FALSE;
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
   if (!_eeze_mtab_lock)
     return;

   mnt_unlock_file(_eeze_mtab_lock);
   mnt_free_lock(_eeze_mtab_lock);
   _eeze_mtab_lock = NULL;
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

   mnt = mnt_table_find_srcpath(_eeze_mount_mtab, eeze_disk_devpath_get(disk), MNT_ITER_BACKWARD);
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

   if (!_eeze_mount_lock_mtab())
     return EINA_FALSE;

   bak = _eeze_mount_tab_parse("/etc/mtab");
   _eeze_mount_unlock_mtab();
   if (!bak)
     goto error;

   mnt_free_table(_eeze_mount_mtab);
   _eeze_mount_mtab = bak;
   if (!(bak = _eeze_mount_tab_parse("/etc/fstab")))
     goto error;

   mnt_free_table(_eeze_mount_fstab);
   _eeze_mount_fstab = bak;

   _eeze_mount_mtab_cache = mnt_new_cache();
   mnt_table_set_cache(_eeze_mount_mtab, _eeze_mount_mtab_cache);

   _eeze_mount_fstab_cache = mnt_new_cache();
   mnt_table_set_cache(_eeze_mount_fstab, _eeze_mount_fstab_cache);

   _mtab_mon = ecore_file_monitor_add("/etc/mtab", _eeze_mount_tab_watcher, (void*)1);
   _fstab_mon = ecore_file_monitor_add("/etc/fstab", _eeze_mount_tab_watcher, NULL);
   _watching = EINA_TRUE;

  return EINA_TRUE;

error:
   if (!_eeze_mount_mtab)
     ERR("Could not parse /etc/mtab!");
   else
     {
        ERR("Could not parse /etc/fstab!");
        mnt_free_table(_eeze_mount_mtab);
     }
   return EINA_FALSE;
}

EAPI void
eeze_mount_tabs_unwatch(void)
{
   if (!_watching)
     return;

   ecore_file_monitor_del(_mtab_mon);
   _mtab_mon = NULL;
   ecore_file_monitor_del(_fstab_mon);
   _fstab_mon = NULL;
   _watching = EINA_FALSE;
}

EAPI Eina_Bool
eeze_mount_mtab_scan(void)
{
   libmnt_table *bak;

   if (_watching)
     return EINA_TRUE;

   if (!_eeze_mount_lock_mtab())
     return EINA_FALSE;
   bak = _eeze_mount_tab_parse("/etc/mtab");
   _eeze_mount_unlock_mtab();
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
