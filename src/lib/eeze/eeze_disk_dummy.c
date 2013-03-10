#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Eeze.h>
#include <Eeze_Disk.h>

#include "eeze_udev_private.h"
#include "eeze_disk_private.h"

Eina_Bool
eeze_libmount_init(void)
{
   return EINA_TRUE;
}

void
eeze_libmount_shutdown(void)
{
}

unsigned long
eeze_disk_libmount_opts_get(Eeze_Disk *disk EINA_UNUSED)
{
   return 0;
}

Eina_Bool
eeze_disk_libmount_mounted_get(Eeze_Disk *disk EINA_UNUSED)
{
   return EINA_FALSE;
}

const char *
eeze_disk_libmount_mp_find_source(const char *mount_point EINA_UNUSED)
{
   return NULL;
}

const char *
eeze_disk_libmount_mp_lookup_by_uuid(const char *uuid EINA_UNUSED)
{
   return NULL;
}

const char *
eeze_disk_libmount_mp_lookup_by_label(const char *label EINA_UNUSED)
{
   return NULL;
}

const char *
eeze_disk_libmount_mp_lookup_by_devpath(const char *devpath EINA_UNUSED)
{
   return NULL;
}

EAPI Eina_Bool
eeze_mount_tabs_watch(void)
{
   ERR("Dummy backend no watching code provided !");
   return EINA_FALSE;
}

EAPI void
eeze_mount_tabs_unwatch(void)
{
}

EAPI Eina_Bool
eeze_mount_mtab_scan(void)
{
   return EINA_FALSE;
}

EAPI Eina_Bool
eeze_mount_fstab_scan(void)
{
   return EINA_FALSE;
}

