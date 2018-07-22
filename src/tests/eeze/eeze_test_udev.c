#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include <Eeze.h>

#include "eeze_suite.h"
/* #include "eeze_udev_private.h" */

static void
catch_events(const char *device EINA_UNUSED, Eeze_Udev_Event  event EINA_UNUSED,
             void *data EINA_UNUSED, Eeze_Udev_Watch *watch EINA_UNUSED)
{
}

#if 0
// FIXME split udev tests into pieces here
EFL_START_TEST(eeze_test_udev_types)
{
   Eina_List *type;

   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_NONE, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_KEYBOARD, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_MOUSE, NULL);
   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_TOUCHPAD, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_DRIVE_MOUNTABLE, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_DRIVE_INTERNAL, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_DRIVE_REMOVABLE, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_DRIVE_CDROM, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_POWER_AC, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_POWER_BAT, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_IS_IT_HOT_OR_IS_IT_COLD_SENSOR, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_NET, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_V4L, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_BLUETOOTH, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_JOYSTICK, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   type = eeze_udev_find_by_type(0, NULL);
   type = eeze_udev_find_unlisted_similar(type);

}
EFL_END_TEST
#endif

EFL_START_TEST(eeze_test_udev_watch)
{
   Eeze_Udev_Watch *watch;

   /* watch never gets triggered as this is run without user interaction */
   watch = eeze_udev_watch_add(EEZE_UDEV_TYPE_KEYBOARD, (EEZE_UDEV_EVENT_ADD | EEZE_UDEV_EVENT_REMOVE), catch_events, NULL);
   watch = eeze_udev_watch_add(EEZE_UDEV_TYPE_DRIVE_INTERNAL, (EEZE_UDEV_EVENT_ADD | EEZE_UDEV_EVENT_REMOVE), catch_events, NULL);
   watch = eeze_udev_watch_add(EEZE_UDEV_TYPE_POWER_AC, (EEZE_UDEV_EVENT_ADD | EEZE_UDEV_EVENT_REMOVE), catch_events, NULL);
   watch = eeze_udev_watch_add(EEZE_UDEV_TYPE_NET, (EEZE_UDEV_EVENT_ADD | EEZE_UDEV_EVENT_REMOVE), catch_events, NULL);
   watch = eeze_udev_watch_add(EEZE_UDEV_TYPE_IS_IT_HOT_OR_IS_IT_COLD_SENSOR, (EEZE_UDEV_EVENT_ADD | EEZE_UDEV_EVENT_REMOVE), catch_events, NULL);
   watch = eeze_udev_watch_add(EEZE_UDEV_TYPE_V4L, (EEZE_UDEV_EVENT_ADD | EEZE_UDEV_EVENT_REMOVE), catch_events, NULL);
   watch = eeze_udev_watch_add(EEZE_UDEV_TYPE_BLUETOOTH, (EEZE_UDEV_EVENT_ADD | EEZE_UDEV_EVENT_REMOVE), catch_events, NULL);
   watch = eeze_udev_watch_add(EEZE_UDEV_TYPE_GPIO, (EEZE_UDEV_EVENT_ADD | EEZE_UDEV_EVENT_REMOVE), catch_events, NULL);
   watch = eeze_udev_watch_add(EEZE_UDEV_TYPE_NONE, (EEZE_UDEV_EVENT_ADD | EEZE_UDEV_EVENT_REMOVE), catch_events, NULL);
   eeze_udev_watch_del(watch);
}
EFL_END_TEST

EFL_START_TEST(eeze_test_udev_syspath)
{
   Eina_List *type, *l;
   const char *name;
   Eina_Bool r;

   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_KEYBOARD, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   EINA_LIST_FOREACH(type, l, name)
     {
         eeze_udev_syspath_get_devpath(name);
         eeze_udev_find_similar_from_syspath(name);
         eeze_udev_find_similar_from_syspath(NULL);
         eeze_udev_syspath_get_parent(name);
         eeze_udev_syspath_get_parent(NULL);
         eeze_udev_syspath_get_parents(name);
         eeze_udev_syspath_get_parents(NULL);
         eeze_udev_syspath_get_devname(name);
         eeze_udev_syspath_get_devname(NULL);
         eeze_udev_syspath_get_subsystem(name);
         eeze_udev_syspath_get_sysattr(name, "manufacturer");
         eeze_udev_syspath_is_mouse(name);
         eeze_udev_syspath_is_kbd(name);
         eeze_udev_syspath_is_touchpad(name);
         eeze_udev_syspath_is_joystick(name);
         eeze_udev_walk_get_sysattr(name, "manufacturer");
         eeze_udev_find_by_sysattr("manufacturer", NULL);
         eeze_udev_find_by_sysattr(NULL, NULL);
         eeze_udev_walk_check_sysattr(name, "manufacturer", NULL);
         eeze_udev_walk_check_sysattr(name, "manufacturer", "foo");
         eeze_udev_walk_get_sysattr(NULL, "manufacturer");
         eeze_udev_walk_get_sysattr(name, NULL);
     }

   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_MOUSE, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   EINA_LIST_FOREACH(type, l, name)
     {
        r = eeze_udev_syspath_is_mouse(name);
        fail_unless(r);

        eeze_udev_syspath_is_touchpad(name);
        eeze_udev_syspath_is_joystick(name);
     }

   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_TOUCHPAD, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   EINA_LIST_FOREACH(type, l, name)
     {
        r = eeze_udev_syspath_is_touchpad(name);
        fail_unless(r);
     }

   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_JOYSTICK, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   EINA_LIST_FOREACH(type, l, name)
     {
        r = eeze_udev_syspath_is_joystick(name);
        fail_unless(r);
     }

   eeze_udev_devpath_get_syspath("/dev/null");
}
EFL_END_TEST

EFL_START_TEST(eeze_test_udev_attr)
{
   Eina_List *type;
   const char *name, *check, *check2;

   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_DRIVE_INTERNAL, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   eeze_udev_find_by_filter("backlight", NULL, NULL);

   eeze_udev_find_by_filter(NULL, NULL, NULL);

   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_NET, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   EINA_LIST_FREE(type, name)
     {
        check = eeze_udev_syspath_get_property(name, "INTERFACE");
     }

   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_DRIVE_INTERNAL, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   EINA_LIST_FREE(type, name)
     {
        check = eeze_udev_syspath_get_property(name, "ID_SERIAL");
     }

   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_DRIVE_REMOVABLE, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   EINA_LIST_FREE(type, name)
     {
        if ((check = eeze_udev_syspath_get_sysattr(name, "model")))
          {
             check2 = eeze_udev_syspath_get_subsystem(name);
          }
     }

   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_V4L, NULL);
   type = eeze_udev_find_unlisted_similar(type);
   EINA_LIST_FREE(type, name)
     {
       if ((check = eeze_udev_syspath_get_property(name, "DEVNAME")))
	 {
	   if ((check2 = eeze_udev_syspath_get_sysattr(name, "name")))
	     {
	       eina_stringshare_del(check2);
	     }
	   eina_stringshare_del(check);
	 }
       eina_stringshare_del(name);
     }
}
EFL_END_TEST

/*
EFL_START_TEST(eeze_test_udev_device)
{
   Eina_List *type, *l;
   _udev_device *device, *device2;
   const char *name;

   type = eeze_udev_find_by_type(EEZE_UDEV_TYPE_DRIVE_INTERNAL, NULL);
   fail_if(type == NULL);
   type = eeze_udev_find_unlisted_similar(type);
   fail_if(type == NULL);
   EINA_LIST_FOREACH(type, l, name)
     {
         device = _new_device(name);
         fail_if(device == NULL);
         _walk_parents_get_attr(device, "FOO", EINA_FALSE);
         _walk_parents_get_attr(device, "FOO", EINA_TRUE);
         _walk_children_get_attr(name, "FOO", "BAR", EINA_FALSE);
         _walk_children_get_attr(name, "FOO", "BAR", EINA_TRUE);
     }

   device = _new_device("Wrong path");
   fail_if(device != NULL);

   device2 = _copy_device(device);
   fail_if(device2 != NULL);
}
EFL_END_TEST
*/

void eeze_test_udev(TCase *tc)
{
   //FIXME This also fails all the time. Enable this once we verify it's not completely useless
   //tcase_add_test(tc, eeze_test_udev_types);
   tcase_add_test(tc, eeze_test_udev_watch);
   tcase_add_test(tc, eeze_test_udev_syspath);
   tcase_add_test(tc, eeze_test_udev_attr);
   //FIXME  Tested functions here are hidden (not EAPI) and thus can not be tested like this with
   //-fvisibility=hidden turned on.
   //tcase_add_test(tc, eeze_test_udev_device);
}
