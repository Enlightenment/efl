#include <Ecore.h>
#include <Ecore_Desktop.h>
#include <stdio.h>
#include "comp.h"

int
main(int argc, char ** argv)
{
    int i = 0, k;
    const char *path;

    ecore_init();
    ecore_desktop_init();

    for (k = 0; k < LOOPS; k++)
    {
        for (i = 0; icons[i] != NULL; i++)
        {
            path = ecore_desktop_icon_find(icons[i], SIZE, THEME);
//            printf("%s: %s\n", icons[i], (path ? path : "NOT FOUND"));
        }
    }

    ecore_desktop_shutdown();
    ecore_shutdown();

    return 0;
}

