#include <Eeze.h>
#include <Ecore.h>
#include <stdio.h>

/**
 * This demo program shows how to use some eeze_udev functions.  It roughly
 * 1kb as of now, TODO is to fix this but I'm too lazy now and it's only
 * a demo.
 */

typedef struct kbdmouse
{
   Eina_List *kbds;
   Eina_List *mice;
   Eina_Hash *hash;
} kbdmouse;

static void
/* event will always be a syspath starting with /sys */
catch_events(const char      *device,
             Eeze_Udev_Event  event,
             void            *data,
             Eeze_Udev_Watch *watch)
{
   kbdmouse *akbdmouse = data;
   Eina_List *l;
   const char *name, *dev, *type;

   /* the device that comes through will be prefixed by "/sys"
    * but the saved name will not, so we check for the saved name
    * inside the device name
    */
   EINA_LIST_FOREACH(akbdmouse->kbds, l, name)
     if (!strncmp(device + 5, name, strlen(device + 5) - 8)) goto end;
   EINA_LIST_FOREACH(akbdmouse->mice, l, name)
     if (!strncmp(device + 5, name, strlen(device + 5) - 8)) goto end;

   /* check to see if the device was just plugged in */
   if (eeze_udev_syspath_is_kbd(device) || eeze_udev_syspath_is_mouse(device))
     goto end;
   /* if we reach here, the device is neither a keyboard nor a mouse that we saw
    * previously, so we print a moderately amusing message and bail
    */
   printf("Sneaky sneaky!  But %s is not a keyboard or a mouse!!\n", device);
   return;

end:
   /* we stored the devpaths for all the syspaths previously so that
    * we can retrieve them now even though the device has been removed and
    * is inaccessible to udev
    */
   if ((event & EEZE_UDEV_EVENT_ADD) == EEZE_UDEV_EVENT_ADD)
     {
        dev = eeze_udev_syspath_get_devpath(device);
        type = "plugged in";
     }
   else
     {
        dev = eina_hash_find(akbdmouse->hash, name);
        type = "unplugged";
     }
   printf("You %s %s!\n", type, dev);
   printf("All tests completed, exiting successfully!\n");
   /* and the hash */
   eina_hash_free(akbdmouse->hash);
   /* now we free the lists */
   eina_list_free(akbdmouse->kbds);
   eina_list_free(akbdmouse->mice);
   /* and the random storage struct */
   free(akbdmouse);
   /* and delete the watch */
   eeze_udev_watch_del(watch);
   /* and shut down eudev */
   eeze_shutdown();
   /* and quit the main loop */
   ecore_main_loop_quit();
}

static void
hash_free(void *data)
{
   eina_stringshare_del(data);
}

int
main()
{
   Eina_List *type, *l;
   const char *name, *check, *check2;
   kbdmouse *akbdmouse;
   Eina_Hash *hash;

   ecore_init();
   eeze_init();

   hash = eina_hash_stringshared_new(hash_free);
   akbdmouse = malloc(sizeof(kbdmouse));
   akbdmouse->hash = hash;

   printf("For my first trick, I will find all of your keyboards and return their syspaths.\n");
   /* find all keyboards using type EEZE_UDEV_TYPE_KEYBOARD */
   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_KEYBOARD, NULL);
   /* add all "link" devices that aren't explicitly found, but are still
    * part of the device chain
    */
   type = eeze_udev_find_unlisted_similar(type);
   EINA_LIST_FOREACH(type, l, name)
     {
        /* add the devpath to the hash for use in the cb later */
         if ((check = eeze_udev_syspath_get_devpath(name)))
           eina_hash_direct_add(hash, name, check);
         printf("Found keyboard: %s\n", name);
     }
   /* we save this list for later, because once a device is unplugged it can
    * no longer be detected by udev, and any related properties are unusable unless
    * they have been previously stored
    */
   akbdmouse->kbds = type;

   printf("\nNext, I will find all of your mice and print the corresponding manufacturer.\n");
   /* find all mice using type EEZE_UDEV_TYPE_MOUSE */
   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_MOUSE, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   EINA_LIST_FOREACH(type, l, name)
     {  /* add the devpath to the hash for use in the cb later */
        if ((check = eeze_udev_syspath_get_devpath(name)))
          eina_hash_direct_add(hash, name, check);  /* get a property using the device's syspath */
        printf("Found mouse %s with vendor: %s\n", name, eeze_udev_walk_get_sysattr(name, "manufacturer"));
     }
   /* we save this list for later, because once a device is unplugged it can
    * no longer be detected by udev, and any related properties are unusable unless
    * they have been previously stored
    */
   akbdmouse->mice = type;

   printf("\nNow let's try something a little more difficult.  Mountable filesystems!\n");
   /* find all mountable drives using type EEZE_UDEV_TYPE_DRIVE_MOUNTABLE */
   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_DRIVE_MOUNTABLE, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   EINA_LIST_FREE(type, name)
     {
        printf("Found device: %s\n", name); /* get a property using the device's syspath */
        if ((check = eeze_udev_syspath_get_property(name, "DEVNAME")))
          {
             printf("\tYou probably know it better as %s\n", check);
             eina_stringshare_del(check);
          }
        if ((check = eeze_udev_syspath_get_property(name, "ID_FS_TYPE")))
          {
             printf("\tIt's formatted as %s", check);
             eina_stringshare_del(check);
             check = eeze_udev_syspath_get_property(name, "FSTAB_DIR");
             if (check)
               {
                  printf(", and gets mounted at %s", check);
                  eina_stringshare_del(check);
               }
             printf("!\n");
          }
        eina_stringshare_del(name);
     }

   printf("\nNetwork devices!\n");
   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_NET, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   EINA_LIST_FREE(type, name)
     {
        printf("Found device: %s\n", name); /* get a property using the device's syspath */
        if ((check = eeze_udev_syspath_get_property(name, "INTERFACE")))
          {
             printf("\tYou probably know it better as %s\n", check);
             eina_stringshare_del(check);
          }
        eina_stringshare_del(name);
     }

   printf("\nInternal drives, anyone?  With serial numbers?\n");
   /* find all internal drives using type EEZE_UDEV_TYPE_DRIVE_INTERNAL */
   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_DRIVE_INTERNAL, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   EINA_LIST_FREE(type, name) /* get a property using the device's syspath */
     {
        if ((check = eeze_udev_syspath_get_property(name, "ID_SERIAL")))
          {
             printf("%s: %s\n", name, check);
             eina_stringshare_del(check);
          }
        eina_stringshare_del(name);
     }

   printf("\nGot any removables?  I'm gonna find em!\n");
   /* find all removable media using type EEZE_UDEV_TYPE_DRIVE_REMOVABLE */
   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_DRIVE_REMOVABLE, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   EINA_LIST_FREE(type, name)  /* get a property using the device's syspath */
     {
        if ((check = eeze_udev_syspath_get_sysattr(name, "model")))
          {
             check2 = eeze_udev_syspath_get_subsystem(name);
             printf("\tOoh, a %s attached to the %s subsytem!\n", check, check2);
             eina_stringshare_del(check);
             eina_stringshare_del(check2);
          }
        eina_stringshare_del(name);
     }

   printf("\nGot any v4l device ?\n");
   /* find all V4L device, may be a webcam or anything that can get a video
    * stream from the real worl in a numerical form */
   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_V4L, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   EINA_LIST_FREE(type, name)  /* get a device name using the device's syspath */
     {
       if ((check = eeze_udev_syspath_get_property(name, "DEVNAME")))
	 {
	   if ((check2 = eeze_udev_syspath_get_sysattr(name, "name")))
	     {
	       printf("%s: '%s' [%s]\n", name, check2, check);
	       eina_stringshare_del(check2);
	     }
	   eina_stringshare_del(check);
	 }
       eina_stringshare_del(name);
     }

   /* set a udev watch, grab all events because no EEZE_UDEV_TYPE filter is specified,
    * set the events to be sent to callback function catch_events(), and attach
    * kbdmouse to the watch as associated data
    */
   eeze_udev_watch_add(EEZE_UDEV_TYPE_NONE, (EEZE_UDEV_EVENT_ADD | EEZE_UDEV_EVENT_REMOVE), catch_events, akbdmouse);
   printf("\nAnd now for something more complicated.  Plug or unplug your keyboard or mouse for me.\n");

   /* main loop must be started to use ecore fd polling */
   ecore_main_loop_begin();

   return 0;
}

