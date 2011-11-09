#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Ecore.h>
#include <Eeze.h>
#include <Eeze_Disk.h>

#include "eeze_udev_private.h"
#include "eeze_disk_private.h"

int _eeze_disk_log_dom = -1;
Eina_List *_eeze_disks = NULL;

static Eeze_Disk_Type
_eeze_disk_type_find(Eeze_Disk *disk)
{
   const char *test;
   Eeze_Disk_Type ret;
   Eina_Bool filesystem = EINA_FALSE; /* this will have no children */

   if (udev_device_get_property_value(disk->device, "ID_CDROM"))
     return EEZE_DISK_TYPE_CDROM;
   test = udev_device_get_property_value(disk->device, "ID_FS_USAGE");
   if ((!test) || strcmp(test, "filesystem"))
     {
        test = _walk_children_get_attr(disk->syspath, "ID_CDROM", "block", EINA_TRUE);
        if (test)
          {
             eina_stringshare_del(test);
             return EEZE_DISK_TYPE_CDROM;
          }
     }
   else
     filesystem = EINA_TRUE;
   if (udev_device_get_property_value(disk->device, "ID_ATA"))
     return EEZE_DISK_TYPE_INTERNAL;
   if (!filesystem)
     {
        test = _walk_children_get_attr(disk->syspath, "ID_ATA", "block", EINA_TRUE);
        if (test)
          {
             eina_stringshare_del(test);
             return EEZE_DISK_TYPE_INTERNAL;
          }
     }
   test = udev_device_get_property_value(disk->device, "ID_BUS");
   if (test)
     {
        if (!strcmp(test, "ata")) return EEZE_DISK_TYPE_INTERNAL;
        if (!strcmp(test, "usb")) return EEZE_DISK_TYPE_USB;
        return EEZE_DISK_TYPE_UNKNOWN; /* FIXME */
     }
   if ((!test) && (!filesystem))
     test = _walk_children_get_attr(disk->syspath, "ID_BUS", "block", EINA_TRUE);
   if (!test)
     {
        _udev_device *dev;

        for (dev = udev_device_get_parent(disk->device); dev; dev = udev_device_get_parent(dev))
          {
             test = udev_device_get_subsystem(dev);
             if (!test) return EEZE_DISK_TYPE_UNKNOWN;
             if (!strcmp(test, "block")) continue;
             if (!strcmp(test, "mmc")) return EEZE_DISK_TYPE_FLASH;
             break;
          }
        return EEZE_DISK_TYPE_UNKNOWN;  /* FIXME */
     }

   if (!strcmp(test, "ata")) ret = EEZE_DISK_TYPE_INTERNAL;
   else if (!strcmp(test, "usb")) ret = EEZE_DISK_TYPE_USB;
   else ret = EEZE_DISK_TYPE_UNKNOWN; /* FIXME */

   eina_stringshare_del(test);

   return ret;
}

static _udev_device *
_eeze_disk_device_from_property(const char *prop, Eina_Bool uuid)
{
   _udev_enumerate *en;
   _udev_list_entry *devs, *cur;
   _udev_device *device = NULL;
   const char *devname;

   en = udev_enumerate_new(udev);

   if (!en)
     return NULL;

   if (uuid)
     udev_enumerate_add_match_property(en, "ID_FS_UUID", prop);
   else
     udev_enumerate_add_match_property(en, "ID_FS_LABEL", prop);
   udev_enumerate_scan_devices(en);
   devs = udev_enumerate_get_list_entry(en);
   udev_list_entry_foreach(cur, devs)
     {
        devname = udev_list_entry_get_name(cur);
        device = udev_device_new_from_syspath(udev, devname);
        break;
     }
   udev_enumerate_unref(en);
   return device;

}

void
eeze_disk_shutdown(void)
{
   eeze_mount_shutdown();
   ecore_file_shutdown();
   eina_log_domain_unregister(_eeze_disk_log_dom);
   _eeze_disk_log_dom = -1;
}

Eina_Bool
eeze_disk_init(void)
{
   _eeze_disk_log_dom = eina_log_domain_register("eeze_disk", EINA_COLOR_LIGHTBLUE);

   if (_eeze_disk_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register 'eeze_disk' log domain.");
        goto disk_fail;
     }

   if (!ecore_file_init())
     goto disk_fail;
   if  (!eeze_mount_init())
     goto ecore_file_fail;

   return EINA_TRUE;

ecore_file_fail:
   ecore_file_shutdown();
disk_fail:
   eina_log_domain_unregister(_eeze_disk_log_dom);
   _eeze_disk_log_dom = -1;
   return EINA_FALSE;
}

EAPI void
eeze_disk_function(void)
{
}

EAPI Eeze_Disk *
eeze_disk_new(const char *path)
{
   Eeze_Disk *disk;
   _udev_device *dev;
   const char *syspath = NULL;
   Eina_Bool is_dev = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);

   if (!strncmp(path, "/dev/", 5))
     {
        is_dev = EINA_TRUE;
        syspath = eeze_udev_devpath_get_syspath(path);
        if (!syspath)
          return NULL;

        if (!(dev = _new_device(syspath)))
          {
             eina_stringshare_del(syspath);
             return NULL;
          }
     }
   else if (!(dev = _new_device(path)))
     return NULL;


   if (!(disk = calloc(1, sizeof(Eeze_Disk))))
     return NULL;


   if (is_dev)
     {
        disk->devpath = eina_stringshare_add(path);
        disk->syspath = syspath;
     }
   else
     disk->syspath = eina_stringshare_add(udev_device_get_syspath(dev));


   disk->device = dev;
   disk->mount_opts = EEZE_DISK_MOUNTOPT_DEFAULTS;
   disk->mount_cmd_changed = EINA_TRUE;
   disk->unmount_cmd_changed = EINA_TRUE;

   _eeze_disks = eina_list_append(_eeze_disks, disk);

   return disk;
}

EAPI Eeze_Disk *
eeze_disk_new_from_mount(const char *mount_point)
{
   Eeze_Disk *disk = NULL;
   _udev_device *dev = NULL;
   const char *syspath = NULL, *source, *uuid = NULL, *label = NULL, *devpath = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(mount_point, NULL);

   if (!(source = eeze_disk_libmount_mp_find_source(mount_point)))
     return NULL;

   if (source[4] == '=')
     {
        source += 5;
        uuid = eina_stringshare_add(source);
        dev = _eeze_disk_device_from_property(uuid, EINA_TRUE);
     }
   else if (source[5] == '=')
     {
        source += 6;
        label = eina_stringshare_add(source);
        dev = _eeze_disk_device_from_property(label, EINA_FALSE);
     }
   else
     {
        const char *spath;

        devpath = eina_stringshare_add(source);
        spath = eeze_udev_devpath_get_syspath(devpath);
        dev = _new_device(spath);
        eina_stringshare_del(spath);
     }

   if (!dev)
     goto error;

   if (!(disk = calloc(1, sizeof(Eeze_Disk))))
     goto error;

   disk->syspath = udev_device_get_syspath(dev);

   disk->device = dev;
   disk->mount_cmd_changed = EINA_TRUE;
   disk->unmount_cmd_changed = EINA_TRUE;
   if (uuid)
     disk->cache.uuid = uuid;
   else if (label)
     disk->cache.label = label;
   else
     disk->devpath = devpath;
   disk->mount_point = eina_stringshare_add(mount_point);

   _eeze_disks = eina_list_append(_eeze_disks, disk);

   return disk;
error:
   if (uuid)
     eina_stringshare_del(uuid);
   else if (label)
     eina_stringshare_del(label);
   else if (devpath)
     eina_stringshare_del(devpath);
   if (syspath)
     eina_stringshare_del(syspath);
   if (dev)
     udev_device_unref(dev);
   return NULL;
}

EAPI void
eeze_disk_free(Eeze_Disk *disk)
{
   extern Eina_List *eeze_events;
   EINA_SAFETY_ON_NULL_RETURN(disk);

   udev_device_unref(disk->device);
   if (disk->mount_cmd)
     eina_strbuf_free(disk->mount_cmd);
   if (disk->unmount_cmd)
     eina_strbuf_free(disk->unmount_cmd);
   if (disk->eject_cmd)
     eina_strbuf_free(disk->eject_cmd);
   if (disk->mounter) ecore_exe_kill(disk->mounter);
   _eeze_disks = eina_list_remove(_eeze_disks, disk);
   eeze_events = eina_list_remove(eeze_events, disk);
   free(disk);
}

EAPI void
eeze_disk_scan(Eeze_Disk *disk)
{
   const char *test;
   EINA_SAFETY_ON_NULL_RETURN(disk);
   /* never rescan; if these values change then something is seriously wrong */
   if (disk->cache.filled) return;

   if (!disk->cache.vendor)
     disk->cache.vendor = udev_device_get_property_value(disk->device, "ID_VENDOR");
   if (!disk->cache.vendor)
     if (!disk->cache.vendor) disk->cache.vendor = udev_device_get_sysattr_value(disk->device, "vendor");
   if (!disk->cache.model)
     disk->cache.model = udev_device_get_property_value(disk->device, "ID_MODEL");
   if (!disk->cache.model)
     if (!disk->cache.model) disk->cache.model = udev_device_get_sysattr_value(disk->device, "model");
   if (!disk->cache.serial)
     disk->cache.serial = udev_device_get_property_value(disk->device, "ID_SERIAL_SHORT");
   if (!disk->cache.uuid)
     disk->cache.uuid = udev_device_get_property_value(disk->device, "ID_FS_UUID");
   if (!disk->cache.type)
     disk->cache.type = _eeze_disk_type_find(disk);
   if (!disk->cache.label)
     disk->cache.label = udev_device_get_property_value(disk->device, "ID_FS_LABEL");
   test = udev_device_get_sysattr_value(disk->device, "removable");
   if (test) disk->cache.removable = !!strtol(test, NULL, 10);
   else
     test = _walk_children_get_attr(disk->syspath, "removable", "block", EINA_FALSE);
   if (test)
     {
        disk->cache.removable = !!strtol(test, NULL, 10);
        eina_stringshare_del(test);
     }

   disk->cache.filled = EINA_TRUE;
}

EAPI void
eeze_disk_data_set(Eeze_Disk *disk, void *data)
{
   EINA_SAFETY_ON_NULL_RETURN(disk);

   disk->data = data;
}

EAPI void *
eeze_disk_data_get(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, NULL);

   return disk->data;
}

EAPI const char *
eeze_disk_syspath_get(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, NULL);

   return disk->syspath;
}

EAPI const char *
eeze_disk_devpath_get(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, NULL);

   if (disk->devpath)
     return disk->devpath;
   disk->devpath = udev_device_get_devnode(disk->device);
   return disk->devpath;
}

EAPI const char *
eeze_disk_fstype_get(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, NULL);

   return disk->fstype;
}

EAPI const char *
eeze_disk_vendor_get(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, NULL);

   if (disk->cache.vendor)
     return disk->cache.vendor;

   disk->cache.vendor = udev_device_get_property_value(disk->device, "ID_VENDOR");
   if (!disk->cache.vendor) disk->cache.vendor = udev_device_get_sysattr_value(disk->device, "vendor");
   return disk->cache.vendor;
}

EAPI const char *
eeze_disk_model_get(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, NULL);

   if (disk->cache.model)
     return disk->cache.model;

   disk->cache.model = udev_device_get_property_value(disk->device, "ID_MODEL");
   if (!disk->cache.model) disk->cache.model = udev_device_get_sysattr_value(disk->device, "model");
   return disk->cache.model;
}

EAPI const char *
eeze_disk_serial_get(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, NULL);

   if (disk->cache.serial)
     return disk->cache.serial;
   disk->cache.serial = udev_device_get_property_value(disk->device, "ID_SERIAL_SHORT");
   return disk->cache.serial;
}

EAPI const char *
eeze_disk_uuid_get(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, NULL);

   if (disk->cache.uuid)
     return disk->cache.uuid;
   disk->cache.uuid = udev_device_get_property_value(disk->device, "ID_FS_UUID");
   return disk->cache.uuid;
}

EAPI const char *
eeze_disk_label_get(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, NULL);

   if (disk->cache.label)
     return disk->cache.label;
   disk->cache.label = udev_device_get_property_value(disk->device, "ID_FS_LABEL");
   return disk->cache.label;
}

EAPI Eeze_Disk_Type
eeze_disk_type_get(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, EEZE_DISK_TYPE_UNKNOWN);

   if (disk->cache.type)
     return disk->cache.type;
   disk->cache.type = _eeze_disk_type_find(disk);
   return disk->cache.type;
}

EAPI Eina_Bool
eeze_disk_removable_get(Eeze_Disk *disk)
{
   const char *test;
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, EINA_FALSE);

   if (disk->cache.filled)
     return disk->cache.removable;

   test = udev_device_get_sysattr_value(disk->device, "removable");
   if (test) disk->cache.removable = !!strtol(test, NULL, 10);
   else
     test = _walk_children_get_attr(disk->syspath, "removable", "block", EINA_FALSE);
   if (test)
     {
        disk->cache.removable = !!strtol(test, NULL, 10);
        eina_stringshare_del(test);
     }
   return disk->cache.removable;
}
