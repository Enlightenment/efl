#include "Efreet.h"
#include "efreet_private.h"

static void
ef_menu_desktop_exec(Efreet_Menu *menu)
{
#if 0
    if (menu->entries)
    {
        Efreet_Desktop *desktop;

        ecore_list_goto_first(menu->entries);
        while ((desktop = ecore_list_next(menu->entries)))
            efreet_desktop_exec(desktop, NULL);
    }
    if (menu->sub_menus)
    {
        Efreet_Menu *sub_menu;

        ecore_list_goto_first(menu->sub_menus);
        while ((sub_menu = ecore_list_next(menu->sub_menus)))
            ef_menu_desktop_exec(sub_menu);
    }
#endif
}

int
ef_cb_menu_get(void)
{
    Efreet_Menu *menu;

    menu = efreet_menu_get();
//    menu = efreet_menu_parse(PACKAGE_DATA_DIR"/efreet/test/test.menu");
    if (!menu)
    {
        printf("efreet_menu_get() returned NULL\n");
        return 0;
    }
#if 0
    if (strcmp(menu->name.internal, "Applications"))
    {
        printf("menu name didn't match\n");
        return 0;
    }

    if (!menu->moves || ecore_list_nodes(menu->moves) != 2)
    {
        printf("Missing moves\n");
        return 0;
    }

    if (menu->current_move)
    {
        printf("Current move still set\n");
        return 0;
    }

    if (menu->filters)
    {
        printf("Have filters when we shouldn't\n");
        return 0;
    }
    ef_menu_desktop_exec(menu);
#endif
    printf("\n");
    efreet_menu_dump(menu, "");
#if 0
    unlink("/tmp/test.menu");
    efreet_menu_save(menu, "/tmp/test.menu");
#endif
    efreet_menu_free(menu);

    return 1;
}

