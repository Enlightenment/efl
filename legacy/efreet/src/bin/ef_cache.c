#include "Efreet.h"
#include <stdio.h>

static int
timer(void *data)
{
    Efreet_Desktop *desktop;
    double start;

    start = ecore_time_get();
    desktop = efreet_util_desktop_name_find("Mozilla Firefox");
    printf("Mozilla Firefox: %p %.3f\n", desktop, (ecore_time_get() - start));
    start = ecore_time_get();
    desktop = efreet_util_desktop_generic_name_find("Mail/News");
    printf("Mail/News: %p %.3f\n", desktop, (ecore_time_get() - start));

    return 0;
}

int
main(int argc, char **argv)
{
    if (!efreet_init()) return 1;
    ecore_timer_add(1.0, timer, NULL);
    ecore_main_loop_begin();
    efreet_shutdown();
    return 0;
}
