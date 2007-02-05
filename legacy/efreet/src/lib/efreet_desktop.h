/* vim: set sw=4 ts=4 sts=4 et: */
#ifndef EFREET_DESKTOP_H
#define EFREET_DESKTOP_H

#include "efreet_ini.h"

/**
 * @file efreet_desktop.h
 * @brief Contains the structures and methods used to support the 
 *        FDO desktop entry specificiation.
 * @addtogroup Efreet_Desktop Efreet_Desktop: The FDO Desktop Entry
 *                  Specification functions and structures 
 *
 * @{
 */

extern int EFREET_DESKTOP_TYPE_APPLICATION;
extern int EFREET_DESKTOP_TYPE_LINK;
extern int EFREET_DESKTOP_TYPE_DIRECTORY;

/**
 * Efreet_Desktop_Type
 */
typedef enum Efreet_Desktop_Type Efreet_Desktop_Type;

/**
 * Efreet_Desktop
 */
typedef struct Efreet_Desktop Efreet_Desktop;

/**
 * A callback used with efreet_desktop_command_get()
 */
typedef void (*Efreet_Desktop_Command_Cb) (void *data, Efreet_Desktop *desktop, 
                                            char *command, int remaining);

/**
 * A callback used to get download progress of remote uris
 */
typedef int (*Efreet_Desktop_Progress_Cb) (void *data, Efreet_Desktop *desktop, 
                                            char *uri, long int total, long int current);

/**
 * A callback used to parse data for custom types
 */
typedef void *(*Efreet_Desktop_Type_Parse_Cb) (Efreet_Desktop *desktop, Efreet_Ini *ini);

/**
 * A callback used to save data for custom types
 */
typedef void (*Efreet_Desktop_Type_Save_Cb) (Efreet_Desktop *desktop, Efreet_Ini *ini);

/**
 * A callback used to free data for custom types
 */
typedef void *(*Efreet_Desktop_Type_Free_Cb) (void *data);

/**
 * Efreet_Desktop
 * @brief a parsed representation of a .desktop file
 */
struct Efreet_Desktop
{
    int type;               /**< type of desktop file */

    double version;         /**< version of spec file conforms to */

    char *orig_path;        /**< original path to .desktop file */
    double load_time;       /**< when the .desktop was loaded from disk */ 

    char *name;             /**< Specific name of the application */
    char *generic_name;     /**< Generic name of the application */
    char *comment;          /**< Tooltip for the entry */
    char *icon;             /**< Icon to display in file manager, menus, etc */
    char *try_exec;         /**< Binary to determine if app is installed */
    char *exec;             /**< Program to execute */
    char *path;             /**< Working directory to run app in */
    char *startup_wm_class; /**< If specified will map at least one window with
                                the given string as it's WM class or WM name */
    char *url;              /**< URL to access if type is EFREET_TYPE_LINK */

    Ecore_List *only_show_in;   /**< list of environments that should 
                                                    display the icon */
    Ecore_List *not_show_in;    /**< list of environments that shoudn't 
                                                    display the icon */
    Ecore_List *categories;     /**< Categories in which item should be shown */
    Ecore_List *mime_types;     /**< The mime types supppored by this app */

    unsigned char no_display:1;        /**< Don't display this application in menus */
    unsigned char hidden:1;            /**< User delete the item */
    unsigned char terminal:1;          /**< Does the program run in a terminal */
    unsigned char startup_notify:1;    /**< The starup notify settings of the app */

    Ecore_Hash *x; /**< Keep track of all user extensions, keys that begin with X- */
    void *type_data; /**< Type specific data for custom types */
};

Efreet_Desktop   *efreet_desktop_get(const char *file);
Efreet_Desktop   *efreet_desktop_empty_new(const char *file);
void              efreet_desktop_free(Efreet_Desktop *desktop);

int               efreet_desktop_save(Efreet_Desktop *desktop);
int               efreet_desktop_save_as(Efreet_Desktop *desktop, 
                                                const char *file);

void              efreet_desktop_exec(Efreet_Desktop *desktop, 
                                      Ecore_List *files, void *data);

void              efreet_desktop_environment_set(const char *environment);
int               efreet_desktop_command_progress_get(Efreet_Desktop *desktop,
                                         Ecore_List *files,
                                         Efreet_Desktop_Command_Cb cb_command,
                                         Efreet_Desktop_Progress_Cb cb_prog,
                                         void *data);
int               efreet_desktop_command_get(Efreet_Desktop *desktop,
                                         Ecore_List *files,
                                         Efreet_Desktop_Command_Cb func,
                                         void *data);

unsigned int      efreet_desktop_category_count_get(Efreet_Desktop *desktop);
void              efreet_desktop_category_add(Efreet_Desktop *desktop,
                                              const char *category);
int               efreet_desktop_category_del(Efreet_Desktop *desktop,
                                              const char *category);

int               efreet_desktop_type_add(const char *type, 
                                    Efreet_Desktop_Type_Parse_Cb parse_func, 
                                    Efreet_Desktop_Type_Save_Cb save_func, 
                                    Efreet_Desktop_Type_Free_Cb free_func);

/** 
 * @}
 */

#endif

