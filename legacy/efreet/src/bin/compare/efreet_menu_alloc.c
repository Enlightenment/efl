#include <Efreet.h>
#include <stdio.h>
#include "comp.h"

int
main(int argc, char **argv)
{
    int k;

    efreet_init();

    for (k = 0; k < LOOPS; k++)
    {
        Efreet_Menu *menu;
        menu = efreet_menu_get();
        efreet_menu_free(menu);
    }

    efreet_shutdown();

    return 0;
}

