#include <Ecore.h>
#include <Ecore_Desktop.h>
#include <Efreet.h>
#include <stdio.h>
#include <string.h>
#include "comp.h"

int
main(int argc, char ** argv)
{
    int i;
    const char *ef, *ed;

    ecore_init();
    ecore_desktop_init();
    efreet_init();

    efreet_icon_extension_add(".svg");

    for (i = 0; icons[i] != NULL; i++)
    {
        ef = efreet_icon_path_find(THEME, icons[i], SIZE);
        ed = ecore_desktop_icon_find(icons[i], SIZE, THEME);

        if (!ef && !ed) continue;

        if (!ef && ed)
            printf("%s matched ecore (%s) but not efreet\n", icons[i], ed);

        else if (ef && !ed)
            printf("%s matched efreet (%s) but not ecore\n", icons[i], ef);

        else if (strcmp(ef, ed))
            printf("%s didn't match ef(%s) vs ed(%s)\n", icons[i], ef, ed);
    }

    efreet_shutdown();
    ecore_desktop_shutdown();
    ecore_shutdown();

    return 0;
}

