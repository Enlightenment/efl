#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <Ecore.h>
#include <Eeze.h>
#include <Eeze_Disk.h>

#include "eeze_udev_private.h"
#include "eeze_disk_private.h"

#define EEZE_MOUNT_DEFAULT_OPTS "noexec,nosuid,utf8"

EAPI int EEZE_EVENT_DISK_MOUNT = 0;
EAPI int EEZE_EVENT_DISK_UNMOUNT = 0;
EAPI int EEZE_EVENT_DISK_EJECT = 0;
EAPI int EEZE_EVENT_DISK_ERROR = 0;
static Ecore_Event_Handler *_mount_handler = NULL;
Eina_List *eeze_events = NULL;

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
   /* FIXME: placeholder since currently there are only mount-type errors */
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

   if ((!disk) || (!eeze_events) || (!(l = eina_list_data_find_list(eeze_events, disk))))
     return ECORE_CALLBACK_RENEW;

   eeze_events = eina_list_remove_list(eeze_events, l);
   if (!disk->mounter) /* killed */
     {
        disk->mount_status = EEZE_DISK_NULL;
        return ECORE_CALLBACK_RENEW;
     }
   if (disk->mount_status == EEZE_DISK_MOUNTING)
     {
        disk->mounter = NULL;
        if (!ev->exit_code)
          {
              e = malloc(sizeof(Eeze_Event_Disk_Mount));
              EINA_SAFETY_ON_NULL_RETURN_VAL(e, ECORE_CALLBACK_RENEW);
              e->disk = disk;
              ecore_event_add(EEZE_EVENT_DISK_MOUNT, e, NULL, NULL);
          }
        else if (ev->exit_code & 2)
           _eeze_disk_mount_error_handler(disk, "system error (out of memory, cannot fork, no more loop devices)");
        else if (ev->exit_code & 4)
           _eeze_disk_mount_error_handler(disk, "internal mount bug");
        else if (ev->exit_code & 8)
           _eeze_disk_mount_error_handler(disk, "user interrupt");
        else if (ev->exit_code & 16)
           _eeze_disk_mount_error_handler(disk, "problems writing or locking /etc/mtab");
        else if (ev->exit_code & 32)
           _eeze_disk_mount_error_handler(disk, "mount failure");
        else if (ev->exit_code & 64)
           _eeze_disk_mount_error_handler(disk, "some mount succeeded");
        else
           _eeze_disk_mount_error_handler(disk, "incorrect invocation or permissions");
     }
   else if (disk->mount_status == EEZE_DISK_UNMOUNTING)
     switch (ev->exit_code)
       {
        case 0:
          e = malloc(sizeof(Eeze_Event_Disk_Unmount));
          EINA_SAFETY_ON_NULL_RETURN_VAL(e, ECORE_CALLBACK_RENEW);
          e->disk = disk;
          disk->mounter = NULL;
          disk->mounted = EINA_FALSE;
          ecore_event_add(EEZE_EVENT_DISK_UNMOUNT, e, NULL, NULL);
          break;

        default:
          INF("Could not unmount disk, retrying");
          disk->mounter = ecore_exe_run(eina_strbuf_string_get(disk->unmount_cmd), disk);
          eeze_events = eina_list_append(eeze_events, disk);
          return ECORE_CALLBACK_RENEW;
       }
     else
       switch (ev->exit_code)
         {
          case 0:
            e = malloc(sizeof(Eeze_Event_Disk_Eject));
            EINA_SAFETY_ON_NULL_RETURN_VAL(e, ECORE_CALLBACK_RENEW);
            e->disk = disk;
            disk->mounter = NULL;
            if (disk->mount_status & EEZE_DISK_UNMOUNTING)
              {
                 disk->mount_status |= EEZE_DISK_UNMOUNTING;
                 disk->mounted = EINA_FALSE;
                 ecore_event_add(EEZE_EVENT_DISK_UNMOUNT, e, NULL, NULL);
                 eeze_disk_eject(disk);
              }
            else
              ecore_event_add(EEZE_EVENT_DISK_EJECT, e, NULL, NULL);
            break;

          default:
            INF("Could not eject disk, retrying");
            if (disk->mount_status & EEZE_DISK_UNMOUNTING)
              disk->mounter = ecore_exe_run(eina_strbuf_string_get(disk->unmount_cmd), disk);
            else
              disk->mounter = ecore_exe_run(eina_strbuf_string_get(disk->eject_cmd), disk);
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
   EEZE_EVENT_DISK_EJECT = ecore_event_type_new();
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

EAPI Eina_Bool
eeze_disk_mounted_get(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, EINA_FALSE);

   return eeze_disk_libmount_mounted_get(disk);
}

EAPI Eina_Bool
eeze_disk_mountopts_set(Eeze_Disk *disk, unsigned long opts)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, EINA_FALSE);
   if (opts != disk->mount_opts)
     disk->mount_cmd_changed = EINA_TRUE;
   disk->mount_opts = opts;
   if (opts & EEZE_DISK_MOUNTOPT_UID)
     disk->uid = getuid();
   return EINA_TRUE;
}

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

EAPI Eina_Bool
eeze_disk_mount_wrapper_set(Eeze_Disk *disk, const char *wrapper)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, EINA_FALSE);
   if (wrapper) EINA_SAFETY_ON_TRUE_RETURN_VAL(!*wrapper, EINA_FALSE);
   else
     {
        eina_stringshare_del(disk->mount_wrapper);
        disk->mount_wrapper = NULL;
        return EINA_TRUE;
     }
   eina_stringshare_replace(&disk->mount_wrapper, wrapper);
   return EINA_TRUE;
}

EAPI const char *
eeze_disk_mount_wrapper_get(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, NULL);
   return disk->mount_wrapper;
}

EAPI Eina_Bool
eeze_disk_mount(Eeze_Disk *disk)
{
   struct stat st;
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
        if (disk->mount_wrapper)
          eina_strbuf_append_printf(disk->mount_cmd, "%s ", disk->mount_wrapper);
        if (disk->mount_opts == EEZE_DISK_MOUNTOPT_DEFAULTS)
          eina_strbuf_append_printf(disk->mount_cmd, EEZE_MOUNT_BIN" -o "EEZE_MOUNT_DEFAULT_OPTS" UUID=%s %s", disk->cache.uuid, disk->mount_point);
        else if (!disk->mount_opts)
          eina_strbuf_append_printf(disk->mount_cmd, EEZE_MOUNT_BIN" UUID=%s %s", disk->cache.uuid, disk->mount_point);
        else
          {
             eina_strbuf_append(disk->mount_cmd, EEZE_MOUNT_BIN" -o ");
             /* trailing commas are okay */
             if (disk->mount_opts & EEZE_DISK_MOUNTOPT_LOOP)
               eina_strbuf_append(disk->mount_cmd, "loop,");
             if (disk->mount_opts & EEZE_DISK_MOUNTOPT_UTF8)
               {
                  const char *fstype;
                  eina_strbuf_append(disk->mount_cmd, "utf8,");
                  fstype = eeze_disk_fstype_get(disk);
                  if (fstype && (!strcmp(fstype, "jfs")))
                    eina_strbuf_append(disk->mount_cmd, "iocharset=utf8,");
               }
             if (disk->mount_opts & EEZE_DISK_MOUNTOPT_NOEXEC)
               eina_strbuf_append(disk->mount_cmd, "noexec,");
             if (disk->mount_opts & EEZE_DISK_MOUNTOPT_NOSUID)
               eina_strbuf_append(disk->mount_cmd, "nosuid,");
             if (disk->mount_opts & EEZE_DISK_MOUNTOPT_REMOUNT)
               eina_strbuf_append(disk->mount_cmd, "remount,");
             if (disk->mount_opts & EEZE_DISK_MOUNTOPT_UID)
               eina_strbuf_append_printf(disk->mount_cmd, "uid=%i,", (int)disk->uid);
             eina_strbuf_append_printf(disk->mount_cmd, " UUID=%s %s", disk->cache.uuid, disk->mount_point);
          }
        disk->mount_cmd_changed = EINA_FALSE;
     }

   if (stat(disk->mount_point, &st))
     {
        INF("Creating not-existing mount point directory '%s'", disk->mount_point);
        if (mkdir(disk->mount_point, S_IROTH | S_IWOTH | S_IXOTH))
          {
             ERR("Could not create directory: %s", strerror(errno));
             return EINA_FALSE;
          }
     }
   else if (!S_ISDIR(st.st_mode))
     {
        ERR("%s is not a directory!", disk->mount_point);
        return EINA_FALSE;
     }
   INF("Mounting: %s", eina_strbuf_string_get(disk->mount_cmd));
   disk->mounter = ecore_exe_run(eina_strbuf_string_get(disk->mount_cmd), disk);
   if (!disk->mounter)
     return EINA_FALSE;
   eeze_events = eina_list_append(eeze_events, disk);
   disk->mount_status = EEZE_DISK_MOUNTING;

   return EINA_TRUE;
}

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
        if (disk->mount_wrapper)
          eina_strbuf_append_printf(disk->unmount_cmd, "%s ", disk->mount_wrapper);
        eina_strbuf_append_printf(disk->unmount_cmd, EEZE_UNMOUNT_BIN" %s", disk->mount_point);
        disk->unmount_cmd_changed = EINA_FALSE;
     }

   INF("Unmounting: %s", eina_strbuf_string_get(disk->unmount_cmd));
   disk->mounter = ecore_exe_run(eina_strbuf_string_get(disk->unmount_cmd), disk);
   if (!disk->mounter)
     return EINA_FALSE;

   eeze_events = eina_list_append(eeze_events, disk);
   disk->mount_status = EEZE_DISK_UNMOUNTING;
   return EINA_TRUE;
}

EAPI Eina_Bool
eeze_disk_eject(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, EINA_FALSE);

   if (!disk->eject_cmd)
     {
        disk->eject_cmd = eina_strbuf_new();
        if (disk->mount_wrapper)
          eina_strbuf_append_printf(disk->eject_cmd, "%s ", disk->mount_wrapper);
        eina_strbuf_append_printf(disk->eject_cmd, EEZE_EJECT_BIN" %s", eeze_disk_devpath_get(disk));
     }

   INF("Ejecting: %s", eina_strbuf_string_get(disk->eject_cmd));
   if (eeze_disk_libmount_mounted_get(disk))
     {
        Eina_Bool ret;

        ret = eeze_disk_unmount(disk);
        if (ret) disk->mount_status |= EEZE_DISK_EJECTING;
        return ret;
     }
   disk->mounter = ecore_exe_run(eina_strbuf_string_get(disk->eject_cmd), disk);
   if (!disk->mounter)
     return EINA_FALSE;

   eeze_events = eina_list_append(eeze_events, disk);
   disk->mount_status = EEZE_DISK_EJECTING;
   return EINA_TRUE;
}

EAPI void
eeze_disk_cancel(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN(disk);
   if ((!disk->mount_status) || (!disk->mounter)) return;
   disk->mount_status = EEZE_DISK_NULL;
   ecore_exe_kill(disk->mounter);
   disk->mounter = NULL;
}

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

EAPI Eina_Bool
eeze_disk_mount_point_set(Eeze_Disk *disk, const char *mount_point)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, EINA_FALSE);

   eina_stringshare_replace(&disk->mount_point, mount_point);
   disk->mount_cmd_changed = EINA_TRUE;
   disk->unmount_cmd_changed = EINA_TRUE;
   return EINA_TRUE;
}
