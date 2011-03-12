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

#include "eeze_udev_private.h"
#include "eeze_disk_private.h"

/**
 * @addtogroup disk Disk
 * @{
 */

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
   { NULL, 0, 0 }
};
typedef struct libmnt_table libmnt_table;
typedef struct libmnt_lock libmnt_lock;
typedef struct libmnt_fs libmnt_fs;
static Ecore_File_Monitor *_mtab_mon = NULL;
static Ecore_File_Monitor *_fstab_mon = NULL;
static Eina_Bool _watching = EINA_FALSE;
static Eina_Bool _mtab_scan_active = EINA_FALSE;
static Eina_Bool _fstab_scan_active = EINA_FALSE;
static libmnt_table *_eeze_mount_mtab = NULL;
static libmnt_table *_eeze_mount_fstab = NULL;
static libmnt_lock *_eeze_mtab_lock = NULL;

static libmnt_table *_eeze_mount_tab_parse(const char *filename);
static void _eeze_mount_tab_watcher(void *data, Ecore_File_Monitor *mon __UNUSED__, Ecore_File_Event event __UNUSED__, const char *path);

static Eina_Bool
_eeze_mount_lock_mtab(void)
{
//    DBG("Locking mlock: %s", mnt_lock_get_linkfile(_eeze_mtab_lock));
    if (mnt_lock_file(_eeze_mtab_lock))
     {
        ERR("Couldn't lock mtab!");
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static void
_eeze_mount_unlock_mtab(void)
{
//   DBG("Unlocking mlock: %s", mnt_lock_get_linkfile(_eeze_mtab_lock));
   mnt_unlock_file(_eeze_mtab_lock);
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

   mnt_free_table(bak);
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
   if (!_eeze_mtab_lock)
     return;

   mnt_unlock_file(_eeze_mtab_lock);
   mnt_free_lock(_eeze_mtab_lock);
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
     return EINA_FALSE;

   eina_stringshare_replace(&disk->mount_point, mnt_fs_get_target(mnt));
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

/**
 * @brief Begin watching mtab and fstab
 * @return #EINA_TRUE if watching was started, else #EINA_FALSE
 *
 * This function creates inotify watches on /etc/mtab and /etc/fstab and watches
 * them for changes.  This function should be used when expecting a lot of disk
 * mounting/unmounting while you need disk data since it will automatically update
 * certain necessary data instead of waiting.
 * @see eeze_mount_mtab_scan, eeze_mount_fstab_scan
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

/**
 * @brief Stop watching /etc/fstab and /etc/mtab
 *
 * This function stops watching fstab and mtab.  Data obtained previously will be saved.
 */
EAPI void
eeze_mount_tabs_unwatch(void)
{
   if (!_watching)
     return;

   ecore_file_monitor_del(_mtab_mon);
   ecore_file_monitor_del(_fstab_mon);
}

/**
 * @brief Scan /etc/mtab a single time
 * @return #EINA_TRUE if mtab could be scanned, else #EINA_FALSE
 *
 * This function is used to perform a single scan on /etc/mtab.  It is used to gather
 * information about mounted filesystems which can then be used with your #Eeze_Disk objects
 * where appropriate.  These files will automatically be scanned any time a mount point or mount state
 * is requested unless eeze_mount_tabs_watch has been called previously, in which case data is stored for
 * use.
 * If this function is called after eeze_mount_tabs_watch, #EINA_TRUE will be returned.
 * @see eeze_mount_tabs_watch, eeze_mount_fstab_scan
 */
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
     mnt_free_table(_eeze_mount_mtab);
   _eeze_mount_mtab = bak;
   return EINA_TRUE;

error:
   return EINA_FALSE;
}

/**
 * @brief Scan /etc/fstab a single time
 * @return #EINA_TRUE if mtab could be scanned, else #EINA_FALSE
 *
 * This function is used to perform a single scan on /etc/fstab.  It is used to gather
 * information about mounted filesystems which can then be used with your #Eeze_Disk objects
 * where appropriate.  These files will automatically be scanned any time a mount point or mount state
 * is requested unless eeze_mount_tabs_watch has been called previously, in which case data is stored for
 * use.
 * If this function is called after eeze_mount_tabs_watch, #EINA_TRUE will be returned.
 * @see eeze_mount_tabs_watch, eeze_mount_mtab_scan
 */
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
     mnt_free_table(_eeze_mount_fstab);
   _eeze_mount_fstab = bak;

   return EINA_TRUE;

error:
   return EINA_FALSE;
}

/** @} */
