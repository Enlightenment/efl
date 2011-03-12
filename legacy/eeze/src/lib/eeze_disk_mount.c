#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Ecore.h>
#include <Eeze.h>
#include <Eeze_Disk.h>

#include "eeze_udev_private.h"
#include "eeze_disk_private.h"

#define EEZE_MOUNT_DEFAULT_OPTS "noexec,nosuid,utf8"

/**
 * @addtogroup disk Disk
 * @{
 */

EAPI int EEZE_EVENT_DISK_MOUNT = 0;
EAPI int EEZE_EVENT_DISK_UNMOUNT = 0;
EAPI int EEZE_EVENT_DISK_ERROR = 0;
static Ecore_Event_Handler *_mount_handler = NULL;
static Eina_List *eeze_events = NULL;

/*
 *
 * PRIVATE
 *
 */

static void
_eeze_disk_mount_error_free(void *data __UNUSED__, Eeze_Event_Disk_Error *de)
{
   if (!de)
     return;

   eina_stringshare_del(de->message);
   free(de);
}

static void
_eeze_disk_mount_error_handler(Eeze_Disk *disk, const char *error)
{
   Eeze_Event_Disk_Error *de;

   ERR("%s", error);
   if (!(de = calloc(1, sizeof(Eeze_Event_Disk_Error))))
     return;

   de->disk = disk;
   de->message = eina_stringshare_add(error);
   /* FIXME: this is a placeholder since currently there are only mount-type errors */
   de->type.mount = EINA_TRUE;
   ecore_event_add(EEZE_EVENT_DISK_ERROR, de, (Ecore_End_Cb)_eeze_disk_mount_error_free, NULL);
}

static Eina_Bool
_eeze_disk_mount_result_handler(void *data __UNUSED__, int type __UNUSED__, Ecore_Exe_Event_Del *ev)
{
   Eeze_Disk *disk;
   Eina_List *l;
   Eeze_Event_Disk_Mount *e;

   if ((!ev) || (!ev->exe))
     return ECORE_CALLBACK_RENEW;
   disk = ecore_exe_data_get(ev->exe);

   if ((!disk) || (!eeze_events) || (!(l = eina_list_data_find(eeze_events, disk))))
     return ECORE_CALLBACK_RENEW;

   eeze_events = eina_list_remove_list(eeze_events, l);
   if (disk->mount_status == EEZE_DISK_MOUNTING)
     switch (ev->exit_code)
       {
         case 1:
           _eeze_disk_mount_error_handler(disk, "incorrect invocation or permissions");
           break;

         case 2:
           _eeze_disk_mount_error_handler(disk, "system error (out of memory, cannot fork, no more loop devices)");
           break;

         case 4:
           _eeze_disk_mount_error_handler(disk, "internal mount bug");
           break;

         case 8:
           _eeze_disk_mount_error_handler(disk, "user interrupt");
           break;

         case 16:
           _eeze_disk_mount_error_handler(disk, "problems writing or locking /etc/mtab");
           break;

         case 32:
           _eeze_disk_mount_error_handler(disk, "mount failure");
           break;

         case 64:
           _eeze_disk_mount_error_handler(disk, "some mount succeeded");
           break;

         default:
           e = malloc(sizeof(Eeze_Event_Disk_Mount));
           EINA_SAFETY_ON_NULL_RETURN_VAL(e, ECORE_CALLBACK_RENEW);
           e->disk = disk;
           ecore_event_add(EEZE_EVENT_DISK_MOUNT, e, NULL, NULL);
        }
    else
      switch (ev->exit_code)
        {
         case 0:
           e = malloc(sizeof(Eeze_Event_Disk_Unmount));
           EINA_SAFETY_ON_NULL_RETURN_VAL(e, ECORE_CALLBACK_RENEW);
           e->disk = disk;
           ecore_event_add(EEZE_EVENT_DISK_UNMOUNT, e, NULL, NULL);
           break;

         default:
           INF("Could not unmount disk, retrying");
           disk->mounter = ecore_exe_pipe_run(eina_strbuf_string_get(disk->unmount_cmd), 0, disk);
           eeze_events = eina_list_append(eeze_events, disk);
           return ECORE_CALLBACK_RENEW;
        }

   return ECORE_CALLBACK_RENEW;
}

/*
 *
 * INVISIBLE
 *
 */

Eina_Bool
eeze_mount_init(void)
{
   EEZE_EVENT_DISK_MOUNT = ecore_event_type_new();
   EEZE_EVENT_DISK_UNMOUNT = ecore_event_type_new();
   EEZE_EVENT_DISK_ERROR = ecore_event_type_new();
   _mount_handler = ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
                                           (Ecore_Event_Handler_Cb)_eeze_disk_mount_result_handler, NULL);
   return eeze_libmount_init();
}

void
eeze_mount_shutdown(void)
{
   eeze_libmount_shutdown();
   ecore_event_handler_del(_mount_handler);
   _mount_handler = NULL;
}

/*
 *
 * API
 *
 */

/**
 * @brief Return the mount state of a disk
 * @param disk The disk
 * @return The mount state
 *
 * This returns the mounted state of the disk.  #EINA_TRUE if mounted, else #EINA_FALSE.
 */
EAPI Eina_Bool
eeze_disk_mounted_get(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, EINA_FALSE);

   return eeze_disk_libmount_mounted_get(disk);
}

/**
 * @brief Set the mount options using flags
 * @param disk The disk
 * @param opts An ORed set of #Eeze_Mount_Opts
 * @return EINA_TRUE on success, else EINA_FALSE
 *
 * This function replaces the current mount opts of a disk with the ones in @p opts.
 */
EAPI Eina_Bool
eeze_disk_mountopts_set(Eeze_Disk *disk, unsigned long opts)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, EINA_FALSE);
   if (opts != disk->mount_opts)
     disk->mount_cmd_changed = EINA_TRUE;
   disk->mount_opts = opts;
   return EINA_TRUE;
}

/**
 * @brief Get the flags of a disk's current mount options
 * @param disk The disk
 * @return An ORed set of #Eeze_Mount_Opts, 0 on failure
 *
 * This function returns the current mount opts of a disk.
 */
EAPI unsigned long
eeze_disk_mountopts_get(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, 0);
#ifndef OLD_LIBMOUNT
   if (!disk->mount_opts)
     disk->mount_opts = eeze_disk_libmount_opts_get(disk);
#endif
   return disk->mount_opts;
}

/**
 * @brief Begin a mount operation on the disk
 * @param disk The disk
 * @return #EINA_TRUE if the operation was started, else #EINA_FALSE
 *
 * This call is used to begin a mount operation on @p disk.  The operation will
 * run asynchronously in a pipe, emitting an EEZE_EVENT_DISK_MOUNT event with the disk object
 * as its event on completion.  If any errors are encountered, they will automatically logged
 * to the eeze_disk domain and an EEZE_EVENT_DISK_ERROR event will be generated with an #Eeze_Event_Disk_Error
 * struct as its event.
 *
 * NOTE: The return value of this function does not in any way reflect the mount state of a disk.
 */
EAPI Eina_Bool
eeze_disk_mount(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, EINA_FALSE);

   if ((!disk->mount_point) && eeze_disk_libmount_mounted_get(disk))
     return EINA_FALSE;

   if (!disk->mount_cmd)
     disk->mount_cmd = eina_strbuf_new();

   if (disk->mount_cmd_changed)
     {
        eina_strbuf_string_free(disk->mount_cmd);
        if (!disk->cache.uuid)
          disk->cache.uuid = udev_device_get_property_value(disk->device, "ID_FS_UUID");

        if (!disk->mount_point)
          {
             const char *mp;
             /* here we attempt to guess the mount point using libmount */
             mp = eeze_disk_libmount_mp_lookup_by_uuid(disk->cache.uuid);
             if (!mp)
               {
                  const char *label;

                  label = eeze_disk_label_get(disk);
                  if (label)
                    {
                       mp = eeze_disk_libmount_mp_lookup_by_label(label);
                       eina_stringshare_del(label);
                    }
                  if (!mp)
                    {
                       const char *devpath;

                       devpath = eeze_disk_devpath_get(disk);
                       if (devpath)
                         {
                            mp = eeze_disk_libmount_mp_lookup_by_devpath(devpath);
                            eina_stringshare_del(devpath);
                         }
                    }
               }
             if (!eeze_disk_mount_point_set(disk, mp))
               /* sometimes we fail */
               return EINA_FALSE;
          }

        if ((!disk->mount_point) || (!disk->mount_point[0])) return EINA_FALSE;
        if (disk->mount_opts == EEZE_DISK_MOUNTOPT_DEFAULTS)
          eina_strbuf_append_printf(disk->mount_cmd, "%s -o %s UUID=%s %s", EEZE_MOUNT_BIN, EEZE_MOUNT_DEFAULT_OPTS, disk->cache.uuid, disk->mount_point);
        else if (!disk->mount_opts)
          eina_strbuf_append_printf(disk->mount_cmd, "%s UUID=%s %s", EEZE_MOUNT_BIN, disk->cache.uuid, disk->mount_point);
        else
          {
             eina_strbuf_append_printf(disk->mount_cmd, "%s -o ", EEZE_MOUNT_BIN);
             /* trailing commas are okay */
             if (disk->mount_opts & EEZE_DISK_MOUNTOPT_LOOP)
               eina_strbuf_append(disk->mount_cmd, "loop,");
             if (disk->mount_opts & EEZE_DISK_MOUNTOPT_UTF8)
               eina_strbuf_append(disk->mount_cmd, "utf8,");
             if (disk->mount_opts & EEZE_DISK_MOUNTOPT_NOEXEC)
               eina_strbuf_append(disk->mount_cmd, "noexec,");
             if (disk->mount_opts & EEZE_DISK_MOUNTOPT_NOSUID)
               eina_strbuf_append(disk->mount_cmd, "nosuid,");
             if (disk->mount_opts & EEZE_DISK_MOUNTOPT_REMOUNT)
               eina_strbuf_append(disk->mount_cmd, "remount,");
             eina_strbuf_append_printf(disk->mount_cmd, " UUID=%s %s", disk->cache.uuid, disk->mount_point);
          }
        disk->mount_cmd_changed = EINA_FALSE;
     }

   INF("Mounting: %s", eina_strbuf_string_get(disk->mount_cmd));
   disk->mounter = ecore_exe_pipe_run(eina_strbuf_string_get(disk->mount_cmd), 0, disk);
   if (!disk->mounter)
     return EINA_FALSE;
   eeze_events = eina_list_append(eeze_events, disk);
   disk->mount_status = EEZE_DISK_MOUNTING;

   return EINA_TRUE;
}

/**
 * @brief Begin an unmount operation on the disk
 * @param disk The disk
 * @return #EINA_TRUE if the operation was started, else #EINA_FALSE
 *
 * This call is used to begin an unmount operation on @p disk.  The operation will
 * run asynchronously in a pipe, emitting an EEZE_EVENT_DISK_MOUNT event with the disk object
 * as its event on completion.  If any errors are encountered, they will automatically logged
 * to the eeze_disk domain and an EEZE_EVENT_DISK_ERROR event will be generated with
 * an #Eeze_Event_Disk_Error struct as its event.
 *
 * NOTE: The return value of this function does not in any way reflect the mount state of a disk.
 */
EAPI Eina_Bool
eeze_disk_unmount(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, EINA_FALSE);

   if (!eeze_disk_libmount_mounted_get(disk))
     return EINA_TRUE;

   if (!disk->unmount_cmd)
     disk->unmount_cmd = eina_strbuf_new();

   if (disk->unmount_cmd_changed)
     {
        eina_strbuf_string_free(disk->unmount_cmd);
        eina_strbuf_append_printf(disk->unmount_cmd, "%s %s", EEZE_UNMOUNT_BIN, disk->mount_point);
        disk->unmount_cmd_changed = EINA_FALSE;
     }

   INF("Unmounting: %s", eina_strbuf_string_get(disk->unmount_cmd));
   disk->mounter = ecore_exe_pipe_run(eina_strbuf_string_get(disk->unmount_cmd), 0, disk);
   if (!disk->mounter)
     return EINA_FALSE;

   eeze_events = eina_list_append(eeze_events, disk);
   disk->mount_status = EEZE_DISK_UNMOUNTING;
   return EINA_TRUE;
}

/**
 * @brief Return the mount point of a disk
 * @param disk The disk
 * @return The mount point
 *
 * This function returns the mount point associated with @p disk.
 * Note that to determine whether the disk is actually mounted, eeze_disk_mounted_get should be used.
 */
EAPI const char *
eeze_disk_mount_point_get(Eeze_Disk *disk)
{
   const char *mp;
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, NULL);

   if (disk->mount_point)
     return disk->mount_point;

   mp = eeze_disk_libmount_mp_lookup_by_devpath(eeze_disk_devpath_get(disk));
   if (mp)
     {
        disk->mount_point = eina_stringshare_add(mp);
        return disk->mount_point;
     }
   mp = eeze_disk_libmount_mp_lookup_by_uuid(eeze_disk_uuid_get(disk));
   if (mp)
     {
        disk->mount_point = eina_stringshare_add(mp);
        return disk->mount_point;
     }
   mp = eeze_disk_libmount_mp_lookup_by_label(eeze_disk_label_get(disk));
   if (mp)
     {
        disk->mount_point = eina_stringshare_add(mp);
        return disk->mount_point;
     }
   return NULL;
}

/**
 * @brief Set the mount point of a disk
 * @param disk The disk
 * @param mount_point The mount point
 * @return EINA_TRUE on success, else EINA_FALSE
 *
 * This function sets the mount point associated with @p disk.
 * Note that to determine whether the disk is actually mounted, eeze_disk_mounted_get should be used.
 * Also note that this function cannot be used while the disk is mounted to avoid losing the current mount point.
 */
EAPI Eina_Bool
eeze_disk_mount_point_set(Eeze_Disk *disk, const char *mount_point)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, EINA_FALSE);

   eina_stringshare_replace(&disk->mount_point, mount_point);
   disk->mount_cmd_changed = EINA_TRUE;
   disk->unmount_cmd_changed = EINA_TRUE;
   return EINA_TRUE;
}

/** @} */
