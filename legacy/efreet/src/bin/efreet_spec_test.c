#include <Efreet.h>
#include <stdio.h>
#include "ef_test.h"

#define PATH_MAX 4096

static void dump(Efreet_Menu *menu, const char *path);

int
main(int argc __UNUSED__, char **argv __UNUSED__)
{
    Efreet_Menu *menu;

    if (!efreet_init())
    {
        fprintf(stderr, "Failed to init Efreet\n");
        return 1;
    }

    menu = efreet_menu_get();
    if (!menu)
    {
        fprintf(stderr, "Failed to read menu\n");
        return 1;
    }

    dump(menu, "");

    efreet_menu_free(menu);
    efreet_shutdown();
    return 0;
}

static void
dump(Efreet_Menu *menu, const char *path)
{
    Efreet_Menu *entry;
    Eina_List *l;

    if (!menu || !menu->entries) return;

    EINA_LIST_FOREACH(menu->entries, l, entry)
    {
        if (entry->type == EFREET_MENU_ENTRY_DESKTOP)
        {
            if (!path || !*path) path = "/";
            printf("%s\t%s\t%s\n", path, entry->id,
                                    entry->desktop->orig_path);
        }
        else if (entry->type == EFREET_MENU_ENTRY_MENU)
        {
            char new_path[PATH_MAX];

            snprintf(new_path, PATH_MAX, "%s%s/", path, entry->name);
            dump(entry, new_path);
        }
    }
}
