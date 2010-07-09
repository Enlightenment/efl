#ifndef EEZE_UDEV_H
#define EEZE_UDEV_H

#include <Ecore.h>

#define LIBUDEV_I_KNOW_THE_API_IS_SUBJECT_TO_CHANGE 1
#include <libudev.h>

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

/**
 * @file Eeze.h
 * @brief Easy device manipulation.
 * 
 * This header provides Eeze device handling functions.  Included are 
 * types and defines for accessing udev, mounting devices, and others.
 */

/**
 * @defgroup subsystems Subsystems
 * @brief These are the device subsystems of udev.
 * 
 * - ac97
 * - acpi
 * - bdi
 * - block
 * - bsg
 * - dmi
 * - graphics
 * - hid
 * - hwmon
 * - i2c
 * - input
 * - mem
 * - misc
 * - net
 * - pci
 * - pci_bus
 * - pci_express
 * - platform
 * - pnp
 * - rtc
 * - scsi
 * - scsi_device
 * - scsi_disk
 * - scsi_generic
 * - scsi_host
 * - serio
 * - sound
 * - thermal
 * - tty
 * - usb
 * - usb_device
 * - vc
 * - vtconsole
 *
 * @ingroup udev
 */

/**
 * @defgroup devices Device_Types
 * @brief These are the devtypes of udev.
 * 
 * - atapi
 * - audio
 * - block
 * - cd
 * - char
 * - disk
 * - floppy
 * - generic
 * - hid
 * - hub
 * - media
 * - optical
 * - printer
 * - rbc
 * - scsi
 * - storage
 * - tape
 * - video
 *
 * @ingroup udev
 */

/**
 * @defgroup event EEZE_UDEV_EVENT
 * @brief These are event types used in udev functions.
 * @ingroup udev
 * 
 * @{
 */

/** - No event specified */
#define EEZE_UDEV_EVENT_NONE 0x0000
/** - Device added */
#define EEZE_UDEV_EVENT_ADD 0x0001
/** - Device removed */
#define EEZE_UDEV_EVENT_REMOVE 0x0002
/** - Device changed */
#define EEZE_UDEV_EVENT_CHANGE 0x0004
/** - Device has come online */
#define EEZE_UDEV_EVENT_ONLINE 0x0008
/** - Device has gone offline */
#define EEZE_UDEV_EVENT_OFFLINE 0x0010
/** @} */

/**
 * @defgroup type Eeze_Udev_Type
 * @brief Convenience types to simplify udev access.
 * 
 * These types allow easy access to certain udev device types.  They
 * may only be used in specified functions.
 * 
 * @ingroup udev
 * @{
 */
/*FIXME: these probably need to be bitmasks with categories*/
typedef enum
{  /** - No type */
   EEZE_UDEV_TYPE_NONE,
   /** - Keyboard device */
   EEZE_UDEV_TYPE_KEYBOARD,
   /** - Mouse device */
   EEZE_UDEV_TYPE_MOUSE,
   /** - Touchpad device */
   EEZE_UDEV_TYPE_TOUCHPAD,
   /** - Mountable drive */
   EEZE_UDEV_TYPE_DRIVE_MOUNTABLE,
   /** - Internal drive */
   EEZE_UDEV_TYPE_DRIVE_INTERNAL,
   /** - Removable drive */
   EEZE_UDEV_TYPE_DRIVE_REMOVABLE,
   /** - cd drive */
   EEZE_UDEV_TYPE_DRIVE_CDROM,
   /** - AC adapter */
   EEZE_UDEV_TYPE_POWER_AC,
   /** - Battery */
   EEZE_UDEV_TYPE_POWER_BAT,
   /** - Temperature sensor */
   EEZE_UDEV_TYPE_IS_IT_HOT_OR_IS_IT_COLD_SENSOR
} Eeze_Udev_Type;
/**@}*/

struct Eeze_Udev_Watch;
typedef struct Eeze_Udev_Watch Eeze_Udev_Watch;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Eeze_Version
  {
     int major;
     int minor;
     int micro;
     int revision;
  } Eeze_Version;

   EAPI extern Eeze_Version *eeze_version;

   EAPI int             eeze_init(void);
   EAPI int             eeze_shutdown(void);

   EAPI Eina_List       *eeze_udev_find_similar_from_syspath(const char *syspath);
   EAPI void             eeze_udev_find_unlisted_similar(Eina_List *list);
   EAPI Eina_List       *eeze_udev_find_by_sysattr(const char *sysattr, const char *value);
   EAPI Eina_List       *eeze_udev_find_by_type(const Eeze_Udev_Type type, const char *name);
   EAPI Eina_List       *eeze_udev_find_by_filter(const char *subsystem, const char *type, const char *name);
   
   EAPI const char      *eeze_udev_devpath_get_syspath(const char *devpath);
   EAPI const char      *eeze_udev_syspath_get_parent(const char *syspath);
   EAPI Eina_List       *eeze_udev_syspath_get_parents(const char *syspath);
   EAPI const char      *eeze_udev_syspath_get_devpath(const char *syspath);
   EAPI const char      *eeze_udev_syspath_get_subsystem(const char *syspath);
   EAPI const char      *eeze_udev_syspath_get_property(const char *syspath, const char *property);
   EAPI const char      *eeze_udev_syspath_get_sysattr(const char *syspath, const char *sysattr);
   
   EAPI Eina_Bool       eeze_udev_syspath_is_mouse(const char *syspath);
   EAPI Eina_Bool       eeze_udev_syspath_is_kbd(const char *syspath);
   EAPI Eina_Bool       eeze_udev_syspath_is_touchpad(const char *syspath);
   
   EAPI Eina_Bool       eeze_udev_walk_check_sysattr(const char *syspath, const char *sysattr, const char *value);
   EAPI const char     *eeze_udev_walk_get_sysattr(const char *syspath, const char *sysattr);

   EAPI Eeze_Udev_Watch *eeze_udev_watch_add(Eeze_Udev_Type type, int event, void(*func)(const char *, int, void *, Eeze_Udev_Watch *), void *user_data);
   EAPI void            *eeze_udev_watch_del(Eeze_Udev_Watch *watch);

#ifdef __cplusplus
}
#endif

#endif
