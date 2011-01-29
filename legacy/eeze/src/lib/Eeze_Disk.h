#ifndef EEZE_DISK_H
#define EEZE_DISK_H

#ifdef EAPI
# undef EAPI
#endif

#ifdef __GNUC__
# if __GNUC__ >= 4
#  define EAPI __attribute__ ((visibility("default")))
# else
#  define EAPI
# endif
#else
# define EAPI
#endif

#include <Eina.h>
#include <Ecore.h>

/**
 * @file Eeze_Disk.h
 * @brief Disk manipulation
 *
 * Eeze disk functions allow you to quickly and efficiently manipulate disks
 * through simple function calls.
 *
 * @addtogroup disk Disk
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
   EEZE_DISK_TYPE_UNKNOWN = 0, /**< type could not be determined */
   EEZE_DISK_TYPE_INTERNAL = 1, /**< internal drive */
   EEZE_DISK_TYPE_CDROM = 2, /**< cdrom drive */
   EEZE_DISK_TYPE_USB = 4 /**< usb drive */
} Eeze_Disk_Type;

typedef enum
{
   EEZE_DISK_MOUNTOPT_DEFAULTS = 0xf0,
   EEZE_DISK_MOUNTOPT_LOOP = (1 << 1)
} Eeze_Mount_Opts;

EAPI extern int EEZE_EVENT_DISK_MOUNT;
EAPI extern int EEZE_EVENT_DISK_UNMOUNT;
EAPI extern int EEZE_EVENT_DISK_ERROR;

typedef struct _Eeze_Event_Disk_Mount Eeze_Event_Disk_Mount;
typedef struct _Eeze_Event_Disk_Unmount Eeze_Event_Disk_Unmount;
typedef struct _Eeze_Disk Eeze_Disk;

struct _Eeze_Event_Disk_Mount
{
   Eeze_Disk *disk;
};

struct _Eeze_Event_Disk_Unmount
{
   Eeze_Disk *disk;
};


typedef struct _Eeze_Event_Disk_Error Eeze_Event_Disk_Error;

struct _Eeze_Event_Disk_Error
{
   Eeze_Disk *disk;
   const char *message;
   struct
   { /* probably switching this to enum */
      Eina_Bool mount : 1;
      Eina_Bool unmount :1;
   } type;
};

EAPI void           eeze_disk_function(void);
EAPI Eeze_Disk     *eeze_disk_new(const char *path);
EAPI Eeze_Disk     *eeze_disk_new_from_mount(const char *mount_point);
EAPI void           eeze_disk_free(Eeze_Disk *disk);
EAPI void           eeze_disk_scan(Eeze_Disk *disk);
EAPI void           eeze_disk_data_set(Eeze_Disk *disk, void *data);
EAPI void          *eeze_disk_data_get(Eeze_Disk *disk);
EAPI const char    *eeze_disk_syspath_get(Eeze_Disk *disk);
EAPI const char    *eeze_disk_devpath_get(Eeze_Disk *disk);
EAPI const char    *eeze_disk_fstype_get(Eeze_Disk *disk);
EAPI const char    *eeze_disk_vendor_get(Eeze_Disk *disk);
EAPI const char    *eeze_disk_model_get(Eeze_Disk *disk);
EAPI const char    *eeze_disk_serial_get(Eeze_Disk *disk);
EAPI const char    *eeze_disk_uuid_get(Eeze_Disk *disk);
EAPI const char    *eeze_disk_label_get(Eeze_Disk *disk);
EAPI Eeze_Disk_Type eeze_disk_type_get(Eeze_Disk *disk);
EAPI Eina_Bool      eeze_disk_removable_get(Eeze_Disk *disk);

EAPI Eina_Bool      eeze_disk_mounted_get(Eeze_Disk *disk);
EAPI Eina_Bool      eeze_disk_mount(Eeze_Disk *disk);
EAPI Eina_Bool      eeze_disk_unmount(Eeze_Disk *disk);
EAPI const char    *eeze_disk_mount_point_get(Eeze_Disk *disk);
EAPI Eina_Bool      eeze_disk_mount_point_set(Eeze_Disk *disk, const char *mount_point);

EAPI Eina_Bool      eeze_mount_tabs_watch(void);
EAPI void           eeze_mount_tabs_unwatch(void);
EAPI Eina_Bool      eeze_mount_mtab_scan(void);
EAPI Eina_Bool      eeze_mount_fstab_scan(void);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
