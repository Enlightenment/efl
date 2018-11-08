#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eeze.h>
#include <Eeze_Disk.h>
#include <Ecore.h>
#include <Ecore_File.h>
#include <Ecore_Getopt.h>
#include <stdio.h>

/** This app can be used as a "dumb" replacement for mount. Just don't try anything fancy yet! */
static const Ecore_Getopt opts =
{
   "eeze_mount",
   "eeze_mount /dev/sdb1 /media/disk",
   "1.0",
   "(C) 2010 Mike Blumenkrantz",
   "LGPL",
   "Mount a disk using either its /sys/ path or its /dev/ path\n\n",
   1,
   {
      ECORE_GETOPT_STORE_TRUE('v', "verbose", "Enable debug output"),
      ECORE_GETOPT_VERSION('V', "version"),
      ECORE_GETOPT_COPYRIGHT('R', "copyright"),
      ECORE_GETOPT_LICENSE('L', "license"),
      ECORE_GETOPT_HELP('h', "help"),
      ECORE_GETOPT_SENTINEL
   }
};

static Eina_Bool
_mount_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *info)
{
   Eeze_Event_Disk_Mount *e = info;
   printf("Success!\n");
   eeze_disk_free(e->disk);
   ecore_main_loop_quit();
   return EINA_TRUE;
}

static Eina_Bool
_error_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *info)
{
   Eeze_Event_Disk_Error *de = info;
   printf("Could not mount disk with /dev/ path: %s!\n", eeze_disk_devpath_get(de->disk));
   eeze_disk_free(de->disk);
   ecore_main_loop_quit();
   return EINA_TRUE;
}

int
main(int argc, char *argv[])
{
   int args;
   const char *dev, *mount_point = NULL;
   Eina_Bool verbose = EINA_FALSE, exit_option = EINA_FALSE;
   Eeze_Disk *disk;

   Ecore_Getopt_Value values[] =
   {
      ECORE_GETOPT_VALUE_BOOL(verbose),
      ECORE_GETOPT_VALUE_BOOL(exit_option),
      ECORE_GETOPT_VALUE_BOOL(exit_option),
      ECORE_GETOPT_VALUE_BOOL(exit_option),
      ECORE_GETOPT_VALUE_BOOL(exit_option)
   };

   if (argc < 2)
     {
        printf("Insufficient args specified!\n");
        ecore_getopt_help(stderr, &opts);
        exit(1);
     }

   ecore_init();
   eeze_init();
   ecore_app_args_set(argc, (const char **)argv);
   args = ecore_getopt_parse(&opts, values, argc, argv);

   if (exit_option)
     return 0;

   if (args < 0)
     {
        printf("No args specified!\n");
        ecore_getopt_help(stderr, &opts);
        exit(1);
     }
   if (verbose) eina_log_domain_level_set("eeze_disk", EINA_LOG_LEVEL_DBG);
   dev = argv[args];
   if (args + 1 < argc)
     mount_point = argv[args + 1];
   if ((!strncmp(dev, "/sys/", 5)) || (!strncmp(dev, "/dev/", 5)))
     disk = eeze_disk_new(dev);
   else if ((args == argc - 1) && (ecore_file_is_dir(dev)))
     disk = eeze_disk_new_from_mount(dev);
   else
     {
        printf("[Device] must be either a /dev/ path or a /sys/ path!\n");
        ecore_getopt_help(stderr, &opts);
        exit(1);
     }
   if (eeze_disk_mounted_get(disk))
     {
        printf("[%s] is already mounted!", dev);
        exit(1);
     }
   if (argc - args > 1)
     {
        eeze_disk_mount_point_set(disk, mount_point);
        if (eina_str_has_extension(dev, "iso"))
          {
             int f;
             f = eeze_disk_mountopts_get(disk);
             eeze_disk_mountopts_set(disk, f | EEZE_DISK_MOUNTOPT_LOOP);
          }
     }
   ecore_event_handler_add(EEZE_EVENT_DISK_MOUNT, _mount_cb, NULL);
   ecore_event_handler_add(EEZE_EVENT_DISK_ERROR, _error_cb, NULL);
   eeze_disk_mountopts_get(disk);
   if (!eeze_disk_mount(disk))
     {
        const char *mp;

        mp = eeze_disk_mount_point_get(disk);
        if (!mp) fprintf(stderr, "No mount point passed!\n");
        else fprintf(stderr, "Mount operation could not be started!\n");
        exit(1);
     }
   ecore_main_loop_begin();

   return 0;
}
