#ifndef E_UDEV_H
#define E_UDEV_H

#include <Ecore.h>
#include <libudev.h>

#define LIBUDEV_I_KNOW_THE_API_IS_SUBJECT_TO_CHANGE 1

#ifdef EAPI
#undef EAPI
#endif
#ifndef _MSC_VER
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

/**
 * @defgroup Subsystem_Types Subsystem_Types
 *
 * ac97
 * acpi
 * bdi
 * block
 * bsg
 * dmi
 * graphics
 * hid
 * hwmon
 * i2c
 * input
 * mem
 * misc
 * net
 * pci
 * pci_bus
 * pci_express
 * platform
 * pnp
 * rtc
 * scsi
 * scsi_device
 * scsi_disk
 * scsi_generic
 * scsi_host
 * serio
 * sound
 * thermal
 * tty
 * usb
 * usb_device
 * vc
 * vtconsole
 *
 * @ingroup udev
 */

/**
 * @defgroup Device_Types Device_Types
 * 
 * atapi
 * audio
 * block
 * cd
 * char
 * disk
 * floppy
 * generic
 * hid
 * hub
 * media
 * optical
 * printer
 * rbc
 * scsi
 * storage
 * tape
 * video
 *
 * @ingroup udev
 */

/*FIXME: these probably need to be bitmasks with categories*/
typedef enum
{
   EUDEV_TYPE_NONE,
   EUDEV_TYPE_KEYBOARD,
   EUDEV_TYPE_MOUSE,
   EUDEV_TYPE_TOUCHPAD,
   EUDEV_TYPE_DRIVE_MOUNTABLE,
   EUDEV_TYPE_DRIVE_INTERNAL,
   EUDEV_TYPE_DRIVE_REMOVABLE,
   EUDEV_TYPE_DRIVE_CDROM,
   EUDEV_TYPE_POWER_AC,
   EUDEV_TYPE_POWER_BAT
/*   EUDEV_TYPE_ANDROID */
} Eudev_Type;

struct Eudev_Watch;
typedef struct Eudev_Watch Eudev_Watch;

#ifdef __cplusplus
extern "C" {
#endif

   EAPI int             e_udev_init(void);
   EAPI int             e_udev_shutdown(void);

   EAPI Eina_List       *e_udev_find_by_type(const Eudev_Type type, const char *name);
   EAPI Eina_List       *e_udev_find_by_filter(const char *subsystem, const char *type, const char *name);
   
   EAPI const char      *e_udev_syspath_rootdev_get(const char *syspath);
   EAPI const char      *e_udev_syspath_get_devpath(const char *syspath);
   EAPI const char      *e_udev_syspath_get_subsystem(const char *syspath);
   EAPI const char      *e_udev_syspath_get_property(const char *syspath, const char *property);
   
   EAPI const char      *e_udev_devpath_get_syspath(const char *devpath);
   EAPI const char      *e_udev_devpath_get_subsystem(const char *devpath);
   
   EAPI Eina_Bool       e_udev_syspath_is_mouse(const char *syspath);
   EAPI Eina_Bool       e_udev_syspath_is_kbd(const char *syspath);
   EAPI Eina_Bool       e_udev_syspath_is_touchpad(const char *syspath);

   EAPI Eudev_Watch     *e_udev_watch_add(Eudev_Type type, void(*func)(const char *, const char *, void *, Eudev_Watch *), void *user_data);
   EAPI void            *e_udev_watch_del(Eudev_Watch *watch);

#ifdef __cplusplus
}
#endif

#endif
