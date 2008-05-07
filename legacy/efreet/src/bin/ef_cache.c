#include "Efreet.h"
#include <stdio.h>
#include "ef_test.h"

static int
timer(void *data __UNUSED__)
{
    Ecore_List *list;
    Efreet_Desktop *desktop;
    double start;

    start = ecore_time_get();
    desktop = efreet_util_desktop_name_find("Mozilla Firefox");
    printf("Mozilla Firefox: %p %.3f\n", desktop, (ecore_time_get() - start));
    start = ecore_time_get();
    desktop = efreet_util_desktop_generic_name_find("Mail/News");
    printf("Mail/News: %p %.3f\n", desktop, (ecore_time_get() - start));
    start = ecore_time_get();
    desktop = efreet_util_desktop_wm_class_find("Mozilla-firefox-bin", NULL);
    printf("Mozilla-firefox-bin: %p %.3f\n", desktop, (ecore_time_get() - start));

    list = efreet_util_desktop_mime_list("application/ogg");
    if (list)
    {
        ecore_list_first_goto(list);
        while ((desktop = ecore_list_next(list)))
        {
            printf("application/ogg: %s\n", desktop->name);
        }
        ecore_list_destroy(list);
    }

    return 0;
}

int
main(int argc __UNUSED__, char **argv __UNUSED__)
{
    if (!efreet_init()) return 1;
    if (!efreet_util_init()) return 1;
    ecore_timer_add(2.0, timer, NULL);
    ecore_main_loop_begin();
    efreet_util_shutdown();
    efreet_shutdown();
    return 0;
}
