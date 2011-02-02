#ifndef EFREET_ICON_H
#define EFREET_ICON_H

/**
 * @file efreet_icon.h
 * @brief Contains the structures and methods used to support the FDO icon
 * theme specificiation.
 * @addtogroup Efreet_Icon Efreet_Icon: The FDO Icon Theme
 *                  Specification functions and structures
 *
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

EAPI const char        *efreet_icon_user_dir_get(void);
EAPI const char        *efreet_icon_deprecated_user_dir_get(void);
EAPI void               efreet_icon_extension_add(const char *ext);

EAPI Eina_List        **efreet_icon_extra_list_get(void);
EAPI Eina_List         *efreet_icon_extensions_list_get(void);
EAPI Eina_List         *efreet_icon_theme_list_get(void);
EAPI Efreet_Icon_Theme *efreet_icon_theme_find(const char *theme_name);
EAPI Efreet_Icon       *efreet_icon_find(const char *theme_name,
                                            const char *icon,
                                            unsigned int size);
EAPI const char        *efreet_icon_list_find(const char *theme_name,
                                                Eina_List *icons,
                                                unsigned int size);
EAPI const char        *efreet_icon_path_find(const char *theme_name,
                                                const char *icon,
                                                unsigned int size);
EAPI void               efreet_icon_free(Efreet_Icon *icon);

/**
 * @}
 */

#endif
