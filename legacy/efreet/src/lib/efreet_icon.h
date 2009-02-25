/* vim: set sw=4 ts=4 sts=4 et: */
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
 * The possible contexts for an icon directory
 */
enum Efreet_Icon_Theme_Context
{
    EFREET_ICON_THEME_CONTEXT_NONE,
    EFREET_ICON_THEME_CONTEXT_ACTIONS,
    EFREET_ICON_THEME_CONTEXT_DEVICES,
    EFREET_ICON_THEME_CONTEXT_FILESYSTEMS,
    EFREET_ICON_THEME_CONTEXT_MIMETYPES
};

/**
 * Efreet_icon_Theme_Context
 */
typedef enum Efreet_Icon_Theme_Context Efreet_Icon_Theme_Context;

/**
 * The possible size types for an icon directory
 */
enum Efreet_Icon_Size_Type
{
    EFREET_ICON_SIZE_TYPE_NONE,
    EFREET_ICON_SIZE_TYPE_FIXED,
    EFREET_ICON_SIZE_TYPE_SCALABLE,
    EFREET_ICON_SIZE_TYPE_THRESHOLD
};

/**
 * Efreet_Icon_Size_Type
 */
typedef enum Efreet_Icon_Size_Type Efreet_Icon_Size_Type;

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

    char *comment;        /**< String describing the theme */
    char *example_icon;   /**< Icon to use as an example of the theme */

    /* An icon theme can have multiple directories that store it's icons. We
     * need to be able to find a search each one. */

    Eina_List *paths;          /**< The paths */
    Eina_List *inherits;       /**< Icon themes we inherit from */
    Eina_List *directories;    /**< List of subdirectories for this theme */

    double last_cache_check;    /**< Last time the cache was checked */

    unsigned char hidden:1;     /**< Should this theme be hidden from users */
    unsigned char valid:1;      /**< Have we seen an index for this theme */
    unsigned char fake:1;       /**< This isnt' a real theme but the user has
                                        tried to query from it. We create the
                                        fake one to give us the theme cache. */
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
    char *name;               /**< The directory name */
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
    char *path;       /**< Full path to the icon */
    char *name;       /**< Translated UTF8 string that can
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
    unsigned char has_embedded_text_rectangle:1; /**< Was the embedded
                                                        rectangle set */
};

/**
 * Efreet_Point
 */
typedef struct Efreet_Icon_Point Efreet_Icon_Point;

/**
 * Efreet_Point
 * @brief Stores an x, y point.
 */
struct Efreet_Icon_Point
{
    int x;          /**< x coord */
    int y;          /**< y coord */
};

EAPI const char        *efreet_icon_user_dir_get(void);
EAPI void               efreet_icon_extension_add(const char *ext);

EAPI Eina_List         *efreet_icon_extra_list_get(void);
EAPI Eina_List         *efreet_icon_theme_list_get(void);
EAPI Efreet_Icon_Theme *efreet_icon_theme_find(const char *theme_name);
EAPI Efreet_Icon       *efreet_icon_find(const char *theme_name,
                                            const char *icon,
                                            unsigned int size);
EAPI char              *efreet_icon_list_find(const char *theme_name,
                                                Eina_List *icons,
                                                unsigned int size);
EAPI char              *efreet_icon_path_find(const char *theme_name,
                                                const char *icon,
                                                unsigned int size);
EAPI void               efreet_icon_free(Efreet_Icon *icon);

/**
 * @}
 */

#endif
