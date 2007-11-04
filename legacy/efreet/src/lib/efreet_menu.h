/* vim: set sw=4 ts=4 sts=4 et: */
#ifndef EFREET_MENU_H
#define EFREET_MENU_H

/**
 * @file efreet_menu.h
 * @brief Contains the structures and methods to support the Desktop
 * Menu Specification.
 * @addtogroup Efreet_Menu Efreet_Menu: The FDO Desktop Menu Specification
 * functions and structures
 *
 * @{
 */

/**
 * The type of entry
 */
enum Efreet_Menu_Entry_Type
{
    EFREET_MENU_ENTRY_MENU,
    EFREET_MENU_ENTRY_DESKTOP,
    EFREET_MENU_ENTRY_SEPARATOR,
    EFREET_MENU_ENTRY_HEADER
};

/**
 * Efreet_Menu_Entry_Type
 */
typedef enum Efreet_Menu_Entry_Type Efreet_Menu_Entry_Type;

/**
 * Efreet_Menu
 */
typedef struct Efreet_Menu Efreet_Menu;

/**
 * Efreet_Menu
 * Stores information on a entry in the menu
 */
struct Efreet_Menu
{
    Efreet_Menu_Entry_Type type;
    const char *id;   /**< File-id for desktop and relative name for menu */

    const char *name; /**< Name this entry should show */
    const char *icon; /**< Icon for this entry */

    Efreet_Desktop *desktop;   /**< The desktop we refer too */
    Ecore_List     *entries;   /**< The menu items */
};

EAPI int              efreet_menu_kde_legacy_init(void);

EAPI Efreet_Menu     *efreet_menu_new(void);
EAPI Efreet_Menu     *efreet_menu_get(void);
EAPI Efreet_Menu     *efreet_menu_parse(const char *path);
EAPI int              efreet_menu_save(Efreet_Menu *menu, const char *path);
EAPI void             efreet_menu_free(Efreet_Menu *menu);

EAPI int              efreet_menu_desktop_insert(Efreet_Menu *menu,
                                                    Efreet_Desktop *desktop,
                                                    int pos);
EAPI int              efreet_menu_desktop_remove(Efreet_Menu *menu,
                                                    Efreet_Desktop *desktop);

EAPI void             efreet_menu_dump(Efreet_Menu *menu, const char *indent);

/**
 * @}
 */

#endif
