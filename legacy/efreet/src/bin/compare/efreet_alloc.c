#include <Efreet.h>
#include <stdio.h>
#include <stdlib.h>
#include "comp.h"

int
main(int argc, char **argv)
{
    int i = 0, k;
    char *path;

    efreet_init();

    for (k = 0; k < LOOPS; k++)
    {
        for (i = 0; icons[i] != NULL; i++)
        {
            path = efreet_icon_path_find(THEME, icons[i], SIZE);
//            printf("%s: %s\n", icons[i], (path ? path : "NOT FOUND"));
            if (path) free(path);
        }
    }

    efreet_shutdown();

    return 0;
}

