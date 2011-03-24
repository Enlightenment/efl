#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Ecore.h>
#include <Eeze.h>
#include <Eeze_Disk.h>

#include "eeze_udev_private.h"
#include "eeze_disk_private.h"

/**
 * @addtogroup disk Disk
 * @{
 */

static Eeze_Disk_Type
_eeze_disk_type_find(Eeze_Disk *disk)
{
   const char *bus;
   bus = udev_device_get_property_value(disk->device, "ID_BUS");
   if (!bus)
     return EEZE_DISK_TYPE_UNKNOWN;

   if (!strcmp(bus, "ata") || !strcmp(bus, "scsi"))
     { /* FIXME: I think some other types of devices fall into this, check later */
        if (udev_device_get_property_value(disk->device, "ID_CDROM"))
          return EEZE_DISK_TYPE_CDROM;
        else
          return EEZE_DISK_TYPE_INTERNAL;
     }
   else if (!strcmp(bus, "usb"))
     return EEZE_DISK_TYPE_USB;

   return EEZE_DISK_TYPE_UNKNOWN;
}

static _udev_device *
_eeze_disk_device_from_property(const char *prop, Eina_Bool uuid)
{
   _udev_enumerate *en;
   _udev_list_entry *devs, *cur;
   _udev_device *device;
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

/**
 * @brief Use this function to determine whether your eeze is disk-capable
 *
 * Since applications will die if they run against a function that doesn't exist,
 * if your application successfully runs this function then you have eeze_disk.
 */
EAPI void
eeze_disk_function(void)
{
}

/**
 * @brief Create a new disk object from a /sys/ path or /dev/ path
 * @param path The /sys/ or /dev path of the disk; CANNOT be #NULL
 * @return The new disk object
 *
 * This function creates a new #Eeze_Disk from @p path.  Note that this function
 * does the minimal amount of work in order to save memory, and udev info about the disk
 * is not retrieved in this call.
 */
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

   return disk;
}

/**
 * @brief Create a new disk object from a mount point
 * @param mount_point The mount point of the disk; CANNOT be #NULL
 * @return The new disk object
 *
 * This function creates a new #Eeze_Disk from @p mount_point.  Note that this function
 * does the minimal amount of work in order to save memory, and udev info about the disk
 * is not retrieved in this call.  If the disk is not currently mounted, it must have an entry
 * in /etc/fstab.
 */
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

/**
 * @brief Frees a disk object
 * @param disk The disk object to free
 *
 * This call frees an #Eeze_Disk.  Once freed, the disk can no longer be used.
 */
EAPI void
eeze_disk_free(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN(disk);


   udev_device_unref(disk->device);
   if (disk->mount_cmd)
     eina_strbuf_free(disk->mount_cmd);
   if (disk->unmount_cmd)
     eina_strbuf_free(disk->unmount_cmd);
   free(disk);
}

/**
 * @brief Retrieve all disk information
 * @param disk
 *
 * Use this function to retrieve all of a disk's information at once, then use
 * a "get" function to retrieve the value.  Data retrieved in this call is cached,
 * meaning that subsequent calls will return immediately without performing any work.
 */
EAPI void
eeze_disk_scan(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN(disk);
   /* never rescan; if these values change then something is seriously wrong */
   if (disk->cache.filled) return;

   if (!disk->cache.vendor)
     disk->cache.vendor = udev_device_get_property_value(disk->device, "ID_VENDOR");
   if (!disk->cache.model)
     disk->cache.model = udev_device_get_property_value(disk->device, "ID_MODEL");
   if (!disk->cache.serial)
     disk->cache.serial = udev_device_get_property_value(disk->device, "ID_SERIAL_SHORT");
   if (!disk->cache.uuid)
     disk->cache.uuid = udev_device_get_property_value(disk->device, "ID_FS_UUID");
   if (!disk->cache.type)
     disk->cache.type = _eeze_disk_type_find(disk);
   if (!disk->cache.label)
     disk->cache.label = udev_device_get_property_value(disk->device, "ID_FS_LABEL");
   disk->cache.removable = !!strtol(udev_device_get_sysattr_value(disk->device, "removable"), NULL, 10);

   disk->cache.filled = EINA_TRUE;
}

/**
 * @brief Associate data with a disk
 * @param disk The disk
 * @param data The data
 *
 * Data can be associated with @p disk with this function.
 * @see eeze_disk_data_get
 */
EAPI void
eeze_disk_data_set(Eeze_Disk *disk, void *data)
{
   EINA_SAFETY_ON_NULL_RETURN(disk);

   disk->data = data;
}

/**
 * @brief Retrieve data previously associated with a disk
 * @param disk The disk
 * @return The data
 *
 * Data that has been previously associated with @p disk
 * is returned with this function.
 * @see eeze_disk_data_set
 */
EAPI void *
eeze_disk_data_get(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, NULL);

   return disk->data;
}

/**
 * @brief Return the /sys/ path of a disk
 * @param disk The disk
 * @return The /sys/ path
 *
 * This retrieves the /sys/ path that udev associates with @p disk.
 */
EAPI const char *
eeze_disk_syspath_get(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, NULL);

   return disk->syspath;
}

/**
 * @brief Return the /dev/ path of a disk
 * @param disk The disk
 * @return The /dev/ path
 *
 * This retrieves the /dev/ path that udev has created a device node at for @p disk.
 */
EAPI const char *
eeze_disk_devpath_get(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, NULL);

   if (disk->devpath)
     return disk->devpath;
   disk->devpath = udev_device_get_devnode(disk->device);
   return disk->devpath;
}

/**
 * @brief Return the filesystem of the disk (if known)
 * @param disk The disk
 * @return The filesystem type
 *
 * This retrieves the filesystem that the disk is using, or #NULL if unknown.
 */
EAPI const char *
eeze_disk_fstype_get(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, NULL);

   return disk->fstype;
}

/**
 * @brief Return the manufacturing vendor of the disk
 * @param disk The disk
 * @return The vendor
 *
 * This retrieves the vendor which manufactured the disk, or #NULL if unknown.
 */
EAPI const char *
eeze_disk_vendor_get(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, NULL);

   if (disk->cache.vendor)
     return disk->cache.vendor;

   disk->cache.vendor = udev_device_get_property_value(disk->device, "ID_VENDOR");
   return disk->cache.vendor;
}

/**
 * @brief Return the model of the disk
 * @param disk The disk
 * @return The model
 *
 * This retrieves the model of the disk, or #NULL if unknown.
 */
EAPI const char *
eeze_disk_model_get(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, NULL);

   if (disk->cache.model)
     return disk->cache.model;

   disk->cache.model = udev_device_get_property_value(disk->device, "ID_MODEL");
   return disk->cache.model;
}

/**
 * @brief Return the serial number of the disk
 * @param disk The disk
 * @return The serial number
 *
 * This retrieves the serial number the disk, or #NULL if unknown.
 */
EAPI const char *
eeze_disk_serial_get(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, NULL);

   if (disk->cache.serial)
     return disk->cache.serial;
   disk->cache.serial = udev_device_get_property_value(disk->device, "ID_SERIAL_SHORT");
   return disk->cache.serial;
}

/**
 * @brief Return the UUID of the disk
 * @param disk The disk
 * @return The UUID
 *
 * This retrieves the UUID of the disk, or #NULL if unknown.
 * A UUID is a 36 character (hopefully) unique identifier which can
 * be used to store persistent information about a disk.
 */
EAPI const char *
eeze_disk_uuid_get(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, NULL);

   if (disk->cache.uuid)
     return disk->cache.uuid;
   disk->cache.uuid = udev_device_get_property_value(disk->device, "ID_FS_UUID");
   return disk->cache.uuid;
}

/**
 * @brief Return the label of the disk
 * @param disk The disk
 * @return The label
 *
 * This retrieves the label (name) of the disk, or #NULL if unknown.
 */
EAPI const char *
eeze_disk_label_get(Eeze_Disk *disk)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, NULL);

   if (disk->cache.label)
     return disk->cache.label;
   disk->cache.label = udev_device_get_property_value(disk->device, "ID_FS_LABEL");
   return disk->cache.label;
}

/**
 * @brief Return the #Eeze_Disk_Type of the disk
 * @param disk The disk
 * @return The type
 *
 * This retrieves the #Eeze_Disk_Type of the disk.  This call is useful for determining
 * the bus that the disk is connected through.
 */
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
   EINA_SAFETY_ON_NULL_RETURN_VAL(disk, EINA_FALSE);

   if (disk->cache.filled)
     return disk->cache.removable;

   disk->cache.removable = !!strtol(udev_device_get_sysattr_value(disk->device, "removable"), NULL, 10);
   return disk->cache.removable;
}

/** @} */
