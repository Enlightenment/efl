/* vim: set sw=4 ts=4 sts=4 et: */
#include "Efreet.h"
#include "config.h"
#include <stdio.h>
#include <unistd.h>

#if 0
static void
ef_menu_desktop_exec(Efreet_Menu *menu)
{
    if (menu->entries)
    {
        Efreet_Desktop *desktop;

        ecore_list_first_goto(menu->entries);
        while ((desktop = ecore_list_next(menu->entries)))
            efreet_desktop_exec(desktop, NULL);
    }
    if (menu->sub_menus)
    {
        Efreet_Menu *sub_menu;

        ecore_list_first_goto(menu->sub_menus);
        while ((sub_menu = ecore_list_next(menu->sub_menus)))
            ef_menu_desktop_exec(sub_menu);
    }
}
#endif

int
ef_cb_menu_get(void)
{
    Efreet_Menu *menu;

    menu = efreet_menu_get();
//    menu = efreet_menu_parse(PACKAGE_DATA_DIR"/test/test.menu");
    if (!menu)
    {
        printf("efreet_menu_get() returned NULL\n");
        return 0;
    }
    printf("\n");
    efreet_menu_dump(menu, "");
    efreet_menu_free(menu);

    return 1;
}

int
ef_cb_menu_with_slashes(void)
{
    Efreet_Menu *menu;

    menu = efreet_menu_parse(PACKAGE_DATA_DIR"/test/test_menu_slash_bad.menu");
    if (menu)
    {
        printf("efreet_menu_get() didn't return NULL\n");
        return 0;
    }

    return 1;
}

int
ef_cb_menu_save(void)
{
    Efreet_Menu *menu;
    int ret;

//    menu = efreet_menu_get();
    menu = efreet_menu_parse(PACKAGE_DATA_DIR"/test/test.menu");
    if (!menu)
    {
        printf("efreet_menu_get() returned NULL\n");
        return 0;
    }
    unlink("/tmp/test.menu");
    ret = efreet_menu_save(menu, "/tmp/test.menu");
    efreet_menu_free(menu);
    return ret;
}

int
ef_cb_menu_edit(void)
{
    Efreet_Menu *menu, *entry;
    Efreet_Desktop *desktop;

//    menu = efreet_menu_get();
    menu = efreet_menu_parse(PACKAGE_DATA_DIR"/test/test.menu");
    if (!menu)
    {
        printf("efreet_menu_get() returned NULL\n");
        return 0;
    }
#if 0
    printf("\n");
    efreet_menu_dump(menu, "");
    printf("\n");
#endif

    desktop = efreet_desktop_get(PACKAGE_DATA_DIR"/test/test.desktop");
    if (!desktop)
    {
        efreet_menu_free(menu);
        printf("No desktop found.\n");
        return 0;
    }

    efreet_menu_desktop_insert(menu, desktop, 0);
#if 0
    printf("\n");
    efreet_menu_dump(menu, "");
    printf("\n");
#endif
    ecore_list_first_goto(menu->entries);
    entry = ecore_list_current(menu->entries);
    if (desktop != entry->desktop)
    {
        efreet_menu_free(menu);
        return 0;
    }

    efreet_menu_desktop_insert(menu, desktop, 2);
#if 0
    printf("\n");
    efreet_menu_dump(menu, "");
    printf("\n");
#endif
    ecore_list_index_goto(menu->entries, 2);
    entry = ecore_list_current(menu->entries);
    if (desktop != entry->desktop)
    {
        efreet_menu_free(menu);
        return 0;
    }

    efreet_menu_desktop_insert(menu, desktop, -1);
#if 0
    printf("\n");
    efreet_menu_dump(menu, "");
    printf("\n");
#endif
    ecore_list_last_goto(menu->entries);
    entry = ecore_list_current(menu->entries);
    if (desktop != entry->desktop)
    {
        efreet_menu_free(menu);
        return 0;
    }

    efreet_menu_free(menu);
    return 1;
}
