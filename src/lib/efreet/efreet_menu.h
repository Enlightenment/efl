#ifndef EFREET_MENU_H
#define EFREET_MENU_H

/**
 * @file efreet_menu.h
 * @brief Contains the structures and methods to support the Desktop
 * Menu Specification.
 *
 * @internal
 * @defgroup Efreet_Menu_Group Efreet_Menu: The FDO Desktop Menu Specification
 *           functions and structures
 * @ingroup Efreet_Group
 *
 * @{
 */

/**
 * The type of entry
 */
typedef enum Efreet_Menu_Entry_Type
{
    EFREET_MENU_ENTRY_MENU,
    EFREET_MENU_ENTRY_DESKTOP,
    EFREET_MENU_ENTRY_SEPARATOR,
    EFREET_MENU_ENTRY_HEADER
} Efreet_Menu_Entry_Type;

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
    Eina_List      *entries;   /**< The menu items */
};


/**
 * @brief Initialize legacy kde support. This function blocks while
 * the kde-config script is run.
 *
 * @return Returns no value
 */
EAPI int              efreet_menu_kde_legacy_init(void);

/**
 * @brief Creates a new menu
 *
 * @param[in] name The internal name of the menu
 * @return Returns the Efreet_Menu on success or
 * NULL on failure
 */
EAPI Efreet_Menu     *efreet_menu_new(const char *name);

/**
 * @brief Override which file is used for menu creation
 *
 * @param[in] file The file to use for menu creation
 *
 * This file is only used if it exists, else the standard files will be used
 * for the menu.
 */
EAPI void             efreet_menu_file_set(const char *file);

/**
 * @brief Creates the default menu representation
 *
 * @return Returns the Efreet_Menu_Internal representation of the default menu or
 * NULL if none found
 */
EAPI Efreet_Menu     *efreet_menu_get(void);

/**
 * @brief Parses the given .menu file and creates the menu representation
 *
 * @param[in] path The path of the menu to load
 * @return Returns the Efreet_Menu_Internal representation on success or NULL on
 * failure
 */
EAPI Efreet_Menu     *efreet_menu_parse(const char *path);

/**
 * @brief Saves the menu to file
 *
 * @param[in] menu The menu to work with
 * @param[in] path The path where the menu should be saved
 * @return Returns 1 on success, 0 on failure
 */
EAPI int              efreet_menu_save(Efreet_Menu *menu, const char *path);

/**
 * @brief Frees the given structure
 *
 * @param[in] menu The Efreet_Menu to free
 * @return Returns no value
 */
EAPI void             efreet_menu_free(Efreet_Menu *menu);


/**
 * @brief Insert a desktop element in a menu structure. Only accepts desktop files
 * in default directories.
 *
 * @param[in] menu The menu to work with
 * @param[in] desktop The desktop to insert
 * @param[in] pos The position to place the new desktop
 * @return Returns 1 on success, 0 on failure
 */
EAPI int              efreet_menu_desktop_insert(Efreet_Menu *menu,
                                                    Efreet_Desktop *desktop,
                                                    int pos);

/**
 * @brief Remove a desktop element in a menu structure. Only accepts desktop files
 * in default directories.
 *
 * @param[in] menu The menu to work with
 * @param[in] desktop The desktop to remove
 * @return Returns 1 on success, 0 on failure
 */
EAPI int              efreet_menu_desktop_remove(Efreet_Menu *menu,
                                                    Efreet_Desktop *desktop);


/**
 * @brief Dumps the contents of the menu to the command line
 *
 * @param[in] menu The menu to work with
 * @param[in] menu The menu to work with
 * @param[in] indent The indent level to print the menu at
 * @return Returns no value
 */
EAPI void             efreet_menu_dump(Efreet_Menu *menu, const char *indent);

/**
 * @}
 */

#endif
