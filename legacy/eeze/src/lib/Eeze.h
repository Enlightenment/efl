#ifndef EEZE_UDEV_H
#define EEZE_UDEV_H

#include <Eina.h>

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
 * Eeze is a library for manipulating devices through udev with a simple and fast
 * api. It interfaces directly with libudev, avoiding such middleman daemons as
 * udisks/upower or hal, to immediately gather device information the instant it
 * becomes known to the system.  This can be used to determine such things as:
 * @li If a cdrom has a disk inserted
 * @li The temperature of a cpu core
 * @li The remaining power left in a battery
 * @li The current power consumption of various parts
 * @li Monitor in realtime the status of peripheral devices
 * Each of the above examples can be performed by using only a single eeze
 * function, as one of the primary focuses of the library is to reduce the
 * complexity of managing devices.
 *
 *
 * For udev functions, see @ref udev.
 */
/**
 * @addtogroup udev
 *
 * These are the device subsystems of udev:
 * @li ac97
 * @li acpi
 * @li bdi
 * @li block
 * @li bsg
 * @li dmi
 * @li graphics
 * @li hid
 * @li hwmon
 * @li i2c
 * @li input
 * @li mem
 * @li misc
 * @li net
 * @li pci
 * @li pci_bus
 * @li pci_express
 * @li platform
 * @li pnp
 * @li rtc
 * @li scsi
 * @li scsi_device
 * @li scsi_disk
 * @li scsi_generic
 * @li scsi_host
 * @li serio
 * @li sound
 * @li thermal
 * @li tty
 * @li usb
 * @li usb_device
 * @li vc
 * @li vtconsole
 *
 * These are the devtypes of udev.
 * @li atapi
 * @li audio
 * @li block
 * @li cd
 * @li char
 * @li disk
 * @li floppy
 * @li generic
 * @li hid
 * @li hub
 * @li media
 * @li optical
 * @li printer
 * @li rbc
 * @li scsi
 * @li storage
 * @li tape
 * @li video
 */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup udev
 * @typedef Eeze_Udev_Event
 * @enum Eeze_Udev_Event
 * @brief Flags for watch events
 *
 * These events are used to specify the events to watch in a
 * #Eeze_Udev_Watch.  They can be ORed together.
 *@{
 */
typedef enum
{
    /** - No event specified */
    EEZE_UDEV_EVENT_NONE = 0xf0,
    /** - Device added */
    EEZE_UDEV_EVENT_ADD = (1 << 1),
    /** - Device removed */
    EEZE_UDEV_EVENT_REMOVE = (1 << 2),
    /** - Device changed */
    EEZE_UDEV_EVENT_CHANGE = (1 << 3),
    /** - Device has come online */
    EEZE_UDEV_EVENT_ONLINE = (1 << 4),
    /** - Device has gone offline */
    EEZE_UDEV_EVENT_OFFLINE = (1 << 5)
} Eeze_Udev_Event;
/** @} */

/**
 * @addtogroup udev udev
 * @typedef Eeze_Udev_Type Eeze_Udev_Type
 * @enum Eeze_Udev_Type
 * @brief Convenience types to simplify udev access.
 *
 * These types allow easy access to certain udev device types.  They
 * may only be used in specified functions.
 *
 * @{
 */
/*FIXME: these probably need to be bitmasks with categories*/
typedef enum
{
   /** - No type */
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
   EEZE_UDEV_TYPE_IS_IT_HOT_OR_IS_IT_COLD_SENSOR,
   /** - Network devices */
   EEZE_UDEV_TYPE_NET
} Eeze_Udev_Type;
/**@}*/

struct Eeze_Udev_Watch;
typedef struct Eeze_Udev_Watch Eeze_Udev_Watch;

#define EEZE_VERSION_MAJOR 1
#define EEZE_VERSION_MINOR 0

   typedef struct _Eeze_Version
     {
        int major;
        int minor;
        int micro;
        int revision;
     } Eeze_Version;

   EAPI extern Eeze_Version *eeze_version;

/**
 * @addtogroup watch
 * @brief Callback type for use with #Eeze_Udev_Watch
 */
typedef void(*Eeze_Udev_Watch_Cb)(const char *, Eeze_Udev_Event, void *, Eeze_Udev_Watch *);

   EAPI int             eeze_init(void);
   EAPI int             eeze_shutdown(void);

   EAPI Eina_List       *eeze_udev_find_similar_from_syspath(const char *syspath);
   EAPI Eina_List       *eeze_udev_find_unlisted_similar(Eina_List *list);
   EAPI Eina_List       *eeze_udev_find_by_sysattr(const char *sysattr, const char *value);
   EAPI Eina_List       *eeze_udev_find_by_type(Eeze_Udev_Type type, const char *name);
   EAPI Eina_List       *eeze_udev_find_by_filter(const char *subsystem, const char *type, const char *name);

   EAPI const char      *eeze_udev_devpath_get_syspath(const char *devpath);
   EAPI const char      *eeze_udev_syspath_get_parent(const char *syspath);
   EAPI Eina_List       *eeze_udev_syspath_get_parents(const char *syspath);
   EAPI const char      *eeze_udev_syspath_get_devpath(const char *syspath);
   EAPI const char      *eeze_udev_syspath_get_devname(const char *syspath);
   EAPI const char      *eeze_udev_syspath_get_subsystem(const char *syspath);
   EAPI const char      *eeze_udev_syspath_get_property(const char *syspath, const char *property);
   EAPI const char      *eeze_udev_syspath_get_sysattr(const char *syspath, const char *sysattr);

   EAPI Eina_Bool       eeze_udev_syspath_is_mouse(const char *syspath);
   EAPI Eina_Bool       eeze_udev_syspath_is_kbd(const char *syspath);
   EAPI Eina_Bool       eeze_udev_syspath_is_touchpad(const char *syspath);

   EAPI Eina_Bool       eeze_udev_walk_check_sysattr(const char *syspath, const char *sysattr, const char *value);
   EAPI const char     *eeze_udev_walk_get_sysattr(const char *syspath, const char *sysattr);

   EAPI Eeze_Udev_Watch *eeze_udev_watch_add(Eeze_Udev_Type type, int event, Eeze_Udev_Watch_Cb cb, void *user_data);
   EAPI void            *eeze_udev_watch_del(Eeze_Udev_Watch *watch);

#ifdef __cplusplus
}
#endif

#endif
