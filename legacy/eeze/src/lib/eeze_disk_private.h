#ifndef EEZE_DISK_PRIVATE_H
#define EEZE_DISK_PRIVATE_H
#include <Eeze.h>
#include <Ecore_File.h>

#ifndef EEZE_DISK_COLOR_DEFAULT
#define EEZE_DISK_COLOR_DEFAULT EINA_COLOR_LIGHTBLUE
#endif
extern int _eeze_disk_log_dom;
#ifdef CRI
#undef CRI
#endif

#ifdef ERR
#undef ERR
#endif
#ifdef INF
#undef INF
#endif
#ifdef WARN
#undef WARN
#endif
#ifdef DBG
#undef DBG
#endif

#define CRI(...)   EINA_LOG_DOM_CRIT(_eeze_disk_log_dom, __VA_ARGS__)
#define DBG(...)   EINA_LOG_DOM_DBG(_eeze_disk_log_dom, __VA_ARGS__)
#define INF(...)    EINA_LOG_DOM_INFO(_eeze_disk_log_dom, __VA_ARGS__)
#define WARN(...) EINA_LOG_DOM_WARN(_eeze_disk_log_dom, __VA_ARGS__)
#define ERR(...)   EINA_LOG_DOM_ERR(_eeze_disk_log_dom, __VA_ARGS__)

typedef enum
{
   EEZE_DISK_NULL = 0,
   EEZE_DISK_MOUNTING = 1,
   EEZE_DISK_UNMOUNTING = 2,
   EEZE_DISK_EJECTING = 4
} Eeze_Disk_Status;

struct _Eeze_Disk
{
   _udev_device *device;
   void *data;

   int mount_status;
   Eina_Strbuf *mount_cmd;
   Eina_Strbuf *unmount_cmd;
   Eina_Strbuf *eject_cmd;
   Eina_Bool mount_cmd_changed : 1;
   Eina_Bool unmount_cmd_changed : 1;
   Eina_Bool mounted : 1;
   Ecore_Exe *mounter;

   const char *syspath;
   const char *devpath;
   const char *fstype;
   const char *mount_point;
   const char *mount_wrapper;
   unsigned long mount_opts;
   uid_t uid;

   struct
     {
        Eeze_Disk_Type type;
        Eina_Bool removable : 1;
        const char *vendor;
        const char *model;
        const char *serial;
        const char *uuid;
        const char *label;
        Eina_Bool filled : 1;
     } cache;
};

Eina_Bool eeze_disk_init(void);
void eeze_disk_shutdown(void);

Eina_Bool eeze_mount_init(void);
void eeze_mount_shutdown(void);

Eina_Bool eeze_libmount_init(void);
void eeze_libmount_shutdown(void);
Eina_Bool eeze_disk_libmount_mounted_get(Eeze_Disk *disk);
unsigned long eeze_disk_libmount_opts_get(Eeze_Disk *disk);
const char *eeze_disk_libmount_mp_find_source(const char *mount_point);

const char *eeze_disk_libmount_mp_lookup_by_uuid(const char *uuid);
const char *eeze_disk_libmount_mp_lookup_by_label(const char *label);
const char *eeze_disk_libmount_mp_lookup_by_devpath(const char *devpath);

#endif
