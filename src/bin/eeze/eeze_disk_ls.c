#include <stdio.h>
#include <Ecore.h>
#include <Eeze.h>
#include <Eeze_Disk.h>

/* simple app to print disks and their mount points */

int
main(void)
{
   Eina_List *disks;
   const char *syspath;

   eeze_init();
   eeze_disk_function();

   disks = eeze_udev_find_by_type(EEZE_UDEV_TYPE_DRIVE_MOUNTABLE, NULL);
   printf("Found the following mountable disks:\n");
   EINA_LIST_FREE(disks, syspath)
     {
        Eeze_Disk *disk;

        disk = eeze_disk_new(syspath);
        printf("\t%s - %s:%s\n", syspath, eeze_disk_devpath_get(disk), eeze_disk_mount_point_get(disk));
        eeze_disk_free(disk);
        eina_stringshare_del(syspath);
     }

   disks = eeze_udev_find_by_type(EEZE_UDEV_TYPE_DRIVE_REMOVABLE, NULL);
   printf("Found the following removable drives:\n");
   EINA_LIST_FREE(disks, syspath)
     {
        Eeze_Disk *disk;

        disk = eeze_disk_new(syspath);
        printf("\t%s - %s:%s\n", syspath, eeze_disk_devpath_get(disk), eeze_disk_mount_point_get(disk));
        eeze_disk_free(disk);
        eina_stringshare_del(syspath);
     }

   disks = eeze_udev_find_by_type(EEZE_UDEV_TYPE_DRIVE_INTERNAL, NULL);
   printf("Found the following internal drives:\n");
   EINA_LIST_FREE(disks, syspath)
     {
        Eeze_Disk *disk;

        disk = eeze_disk_new(syspath);
        printf("\t%s - %s\n", syspath, eeze_disk_devpath_get(disk));
        eeze_disk_free(disk);
        eina_stringshare_del(syspath);
     }
   return 0;
}
