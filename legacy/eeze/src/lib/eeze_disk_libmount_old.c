#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef USE_UNSTABLE_LIBMOUNT_API
# define USE_UNSTABLE_LIBMOUNT_API 1
#endif

#include <Ecore.h>
#include <Eeze.h>
#include <Eeze_Disk.h>
#include <mount/mount.h>

#include "eeze_udev_private.h"
#include "eeze_disk_private.h"
/*
 *
 * PRIVATE
 *
 */
static Ecore_File_Monitor *_mtab_mon = NULL;
static Ecore_File_Monitor *_fstab_mon = NULL;
static Eina_Bool _watching = EINA_FALSE;
static Eina_Bool _mtab_scan_active = EINA_FALSE;
static Eina_Bool _fstab_scan_active = EINA_FALSE;
static mnt_tab *_eeze_mount_mtab = NULL;
static mnt_tab *_eeze_mount_fstab = NULL;
static mnt_lock *_eeze_mtab_lock = NULL;

static mnt_tab *_eeze_mount_tab_parse(const char *filename);
static void _eeze_mount_tab_watcher(void *data, Ecore_File_Monitor *mon __UNUSED__, Ecore_File_Event event __UNUSED__, const char *path);

static Eina_Bool
_eeze_mount_lock_mtab(void)
{
    DBG("Locking mlock: %s", mnt_lock_get_linkfile(_eeze_mtab_lock));
#if 0
#warning this code is broken with current libmount!
    if (mnt_lock_file(_eeze_mtab_lock))
     {
        ERR("Couldn't lock mtab!");
        return EINA_FALSE;
     }
#endif
   return EINA_TRUE;
}

static void
_eeze_mount_unlock_mtab(void)
{
   DBG("Unlocking mlock: %s", mnt_lock_get_linkfile(_eeze_mtab_lock));
   mnt_unlock_file(_eeze_mtab_lock);
}

/*
 * I could use mnt_new_tab_from_file() but this way gives much more detailed output
 * on failure so why not
 */
static mnt_tab *
_eeze_mount_tab_parse(const char *filename)
{
   mnt_tab *tab;

   if (!(tab = mnt_new_tab(filename)))
     return NULL;
   if (!mnt_tab_parse_file(tab))
     return tab;

   if (mnt_tab_get_nerrs(tab))
     {  /* parse error */
        char buf[1024];

        mnt_tab_strerror(tab, buf, sizeof(buf));
        ERR("%s", buf);
     }
   else
     /* system error */
     ERR("%s", mnt_tab_get_name(tab));
   mnt_free_tab(tab);
   return NULL;
}

static void
_eeze_mount_tab_watcher(void *data, Ecore_File_Monitor *mon __UNUSED__, Ecore_File_Event event __UNUSED__, const char *path)
{
   mnt_tab *bak;

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

   mnt_free_tab(bak);
   return;

error:
   mnt_free_tab(_eeze_mount_mtab);
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
   if (!(_eeze_mtab_lock = mnt_new_lock(NULL, 0)))
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
   _eeze_mtab_lock = NULL;
}

/*
 * helper function to return whether a disk is mounted
 */
Eina_Bool
eeze_disk_libmount_mounted_get(Eeze_Disk *disk)
{
   mnt_fs *mnt;

   if (!disk)
     return EINA_FALSE;

   if (!eeze_mount_mtab_scan() || !eeze_mount_fstab_scan())
     return EINA_FALSE;

   mnt = mnt_tab_find_srcpath(_eeze_mount_mtab, eeze_disk_devpath_get(disk), MNT_ITER_BACKWARD);
   if (!mnt)
     return EINA_FALSE;

   disk->mount_point = eina_stringshare_add(mnt_fs_get_target(mnt));
   return EINA_TRUE;
}


/*
 * helper function to return the device that is mounted at a mount point
 */
const char *
eeze_disk_libmount_mp_find_source(const char *mount_point)
{
   mnt_fs *mnt;

   if (!mount_point)
     return NULL;

   if (!eeze_mount_mtab_scan() || !eeze_mount_fstab_scan())
     return NULL;

   mnt = mnt_tab_find_target(_eeze_mount_mtab, mount_point, MNT_ITER_BACKWARD);
   if (!mnt)
     mnt = mnt_tab_find_target(_eeze_mount_fstab, mount_point, MNT_ITER_BACKWARD);

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
   mnt_fs *mnt;

   if (!uuid)
     return NULL;

   if (!eeze_mount_mtab_scan() || !eeze_mount_fstab_scan())
     return NULL;

   mnt = mnt_tab_find_tag(_eeze_mount_fstab, "UUID", uuid, MNT_ITER_BACKWARD);

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
   mnt_fs *mnt;

   if (!label)
     return NULL;

   if (!eeze_mount_mtab_scan() || !eeze_mount_fstab_scan())
     return NULL;

   mnt = mnt_tab_find_tag(_eeze_mount_fstab, "LABEL", label, MNT_ITER_BACKWARD);

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
   mnt_fs *mnt;

   if (!devpath)
     return NULL;

   if (!eeze_mount_mtab_scan() || !eeze_mount_fstab_scan())
     return NULL;

   mnt = mnt_tab_find_srcpath(_eeze_mount_mtab, devpath, MNT_ITER_BACKWARD);
   if (!mnt)
     mnt = mnt_tab_find_srcpath(_eeze_mount_fstab, devpath, MNT_ITER_BACKWARD);

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
   mnt_tab *bak;

   if (_watching)
     return EINA_TRUE;

   if (!_eeze_mount_lock_mtab())
     return EINA_FALSE;

   bak = _eeze_mount_tab_parse("/etc/mtab");
   _eeze_mount_unlock_mtab();
   if (!bak)
     goto error;

   mnt_free_tab(_eeze_mount_mtab);
   _eeze_mount_mtab = bak;
   if (!(bak = _eeze_mount_tab_parse("/etc/fstab")))
     goto error;

   mnt_free_tab(_eeze_mount_fstab);
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
        mnt_free_tab(_eeze_mount_mtab);
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
   mnt_tab *bak;

   if (_watching)
     return EINA_TRUE;

   if (!_eeze_mount_lock_mtab())
     return EINA_FALSE;
   bak = _eeze_mount_tab_parse("/etc/mtab");
   _eeze_mount_unlock_mtab();
   if (!bak)
     goto error;
   if (_eeze_mount_mtab)
     mnt_free_tab(_eeze_mount_mtab);
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
   mnt_tab *bak;
   if (_watching)
     return EINA_TRUE;

   bak = _eeze_mount_tab_parse("/etc/fstab");
   if (!bak)
     goto error;
   if (_eeze_mount_fstab)
     mnt_free_tab(_eeze_mount_fstab);
   _eeze_mount_fstab = bak;

   return EINA_TRUE;

error:
   return EINA_FALSE;
}
