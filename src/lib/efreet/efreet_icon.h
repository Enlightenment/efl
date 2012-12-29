#ifndef EFREET_ICON_H
#define EFREET_ICON_H

/**
 * @file efreet_icon.h
 * @brief Contains the structures and methods used to support the FDO icon
 * theme specificiation.
 * @addtogroup Efreet_Icon Efreet_Icon: The FDO Icon Theme
 *                  Specification functions and structures
 *
 * @ingroup Efreet
 * @{
 */


/**
 * Event id for cache update.
 */
EAPI extern int EFREET_EVENT_ICON_CACHE_UPDATE;

/**
 * The possible contexts for an icon directory
 */
typedef enum Efreet_Icon_Theme_Context
{
    EFREET_ICON_THEME_CONTEXT_NONE,
    EFREET_ICON_THEME_CONTEXT_ACTIONS,
    EFREET_ICON_THEME_CONTEXT_DEVICES,
    EFREET_ICON_THEME_CONTEXT_FILESYSTEMS,
    EFREET_ICON_THEME_CONTEXT_MIMETYPES
} Efreet_Icon_Theme_Context;

/**
 * The possible size types for an icon directory
 */
typedef enum Efreet_Icon_Size_Type
{
    EFREET_ICON_SIZE_TYPE_NONE,
    EFREET_ICON_SIZE_TYPE_FIXED,
    EFREET_ICON_SIZE_TYPE_SCALABLE,
    EFREET_ICON_SIZE_TYPE_THRESHOLD
} Efreet_Icon_Size_Type;

/**
 * Efreet_Icon_Theme
 */
typedef struct Efreet_Icon_Theme Efreet_Icon_Theme;

/**
 * Efreet_Icon_Theme
 * @brief contains all of the known information about a given theme
 */
struct Efreet_Icon_Theme
{
    struct
    {
        const char *internal;   /**< The internal theme name */
        const char *name;       /**< The user visible name */
    } name;                     /**< The different names for the theme */

    const char *comment;        /**< String describing the theme */
    const char *example_icon;   /**< Icon to use as an example of the theme */

    /* An icon theme can have multiple directories that store it's icons. We
     * need to be able to find a search each one. */

    Eina_List *paths;           /**< The paths */
    Eina_List *inherits;        /**< Icon themes we inherit from */
    Eina_List *directories;     /**< List of subdirectories for this theme */
};

/**
 * Efreet_Icon_Theme_Directory
 */
typedef struct Efreet_Icon_Theme_Directory Efreet_Icon_Theme_Directory;

/**
 * Efreet_Icon_Theme_Directory
 * @brief Contains all the information about a sub-directory of a theme
 */
struct Efreet_Icon_Theme_Directory
{
    const char *name;               /**< The directory name */
    Efreet_Icon_Theme_Context context;  /**< The type of icons in the dir */
    Efreet_Icon_Size_Type type;     /**< The size type for the icons */

    struct
    {
        unsigned int normal;        /**< The size for this directory */
        unsigned int min;           /**< The minimum size for this directory */
        unsigned int max;           /**< The maximum size for this directory */
        unsigned int threshold;     /**< Size difference threshold */
    } size;                         /**< The size settings for the icon theme */
};

/**
 * Efreet_Icon
 */
typedef struct Efreet_Icon Efreet_Icon;

/**
 * Efreet_Icon
 * @brief Contains all the information about a given icon
 */
struct Efreet_Icon
{
    const char *path;       /**< Full path to the icon */
    const char *name;       /**< Translated UTF8 string that can
                                    be used for the icon name */

    struct
    {
        int x0,             /**< x0 position */
            y0,             /**< y0 position */
            x1,             /**< x1 position */
            y1;             /**< y1 position */
    } embedded_text_rectangle;  /**< Rectangle where text can
                                        be displayed on the icon */

    Eina_List *attach_points; /**< List of points to be used as anchor
                                        points for emblems/overlays */

    unsigned int ref_count;    /**< References to this icon */
    unsigned char has_embedded_text_rectangle:1; /**< Has the embedded
                                                        rectangle set */
};

/**
 * Efreet_Icon_Point
 */
typedef struct Efreet_Icon_Point Efreet_Icon_Point;

/**
 * Efreet_Icon_Point
 * @brief Stores an x, y point.
 */
struct Efreet_Icon_Point
{
    int x;          /**< x coord */
    int y;          /**< y coord */
};

/**
 * @return Returns the user icon directory
 * @brief Returns the user icon directory
 */
EAPI const char        *efreet_icon_user_dir_get(void);

/**
 * @return Returns the deprecated user icon directory
 * @brief Returns the deprecated user icon directory
 */
EAPI const char        *efreet_icon_deprecated_user_dir_get(void);

/**
 * @param ext The extension to add to the list of checked extensions
 * @return Returns no value.
 * @brief Adds the given extension to the list of possible icon extensions
 */
EAPI void               efreet_icon_extension_add(const char *ext);


/**
 * @return Returns a list of strings that are paths to other icon directories
 * @brief Gets the list of all extra directories to look for icons. These
 * directories are used to look for icons after looking in the user icon dir
 * and before looking in standard system directories. The order of search is
 * from first to last directory in this list. the strings in the list should
 * be created with eina_stringshare_add().
 */
EAPI Eina_List        **efreet_icon_extra_list_get(void);

/**
 * @return Returns a list of strings that are icon extensions to look for
 * @brief Gets the list of all icon extensions to look for
 */
EAPI Eina_List         *efreet_icon_extensions_list_get(void);

/**
 * @return Returns a list of Efreet_Icon structs for all the non-hidden icon
 * themes
 * @brief Retrieves all of the non-hidden icon themes available on the system.
 * The returned list must be freed. Do not free the list data.
 */
EAPI Eina_List         *efreet_icon_theme_list_get(void);

/**
 * @param theme_name The theme to look for
 * @return Returns the icon theme related to the given theme name or NULL if
 * none exists.
 * @brief Tries to get the icon theme structure for the given theme name
 */
EAPI Efreet_Icon_Theme *efreet_icon_theme_find(const char *theme_name);

/**
 * @param theme_name The icon theme to look for
 * @param icon The icon to look for
 * @param size The icon size to look for
 * @return Returns the Efreet_Icon structure representing this icon or NULL
 * if the icon is not found
 * @brief Retrieves all of the information about the given icon.
 */
EAPI Efreet_Icon       *efreet_icon_find(const char *theme_name,
                                            const char *icon,
                                            unsigned int size);

/**
 * @param theme_name The icon theme to look for
 * @param icons List of icons to look for
 * @param size; The icon size to look for
 * @return Returns the path representing first found icon or
 * NULL if none of the icons are found
 * @brief Retrieves all of the information about the first found icon in
 * the list.
 * @note This function will search the given theme for all icons before falling
 * back. This is useful when searching for mimetype icons.
 *
 * There is no guarantee for how long the pointer to the path will be valid.
 * If the pointer is to be kept, the user must create a copy of the path.
 */
EAPI const char        *efreet_icon_list_find(const char *theme_name,
                                                Eina_List *icons,
                                                unsigned int size);

/**
 * @param theme_name The icon theme to look for
 * @param icon The icon to look for
 * @param size; The icon size to look for
 * @return Returns the path to the given icon or NULL if none found
 * @brief Retrives the path to the given icon.
 *
 * There is no guarantee for how long the pointer to the path will be valid.
 * If the pointer is to be kept, the user must create a copy of the path.
 */
EAPI const char        *efreet_icon_path_find(const char *theme_name,
                                                const char *icon,
                                                unsigned int size);

/**
 * @param icon The Efreet_Icon to cleanup
 * @return Returns no value.
 * @brief Free's the given icon and all its internal data.
 */
EAPI void               efreet_icon_free(Efreet_Icon *icon);

/**
 * @}
 */

#endif
