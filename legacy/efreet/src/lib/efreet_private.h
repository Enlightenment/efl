/* vim: set sw=4 ts=4 sts=4 et: */
#ifndef EFREET_PRIVATE_H
#define EFREET_PRIVATE_H

/**
 * @file efreet_private.h
 * @brief Contains methods and defines that are private to the Efreet
 * implementaion
 * @addtogroup Efreet_Private Efreet_Private: Private methods and defines
 *
 * @{
 */

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fnmatch.h>
#include <limits.h>

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_File.h>
#include <Ecore_Str.h>

#include "efreet_xml.h"
#include "efreet_ini.h"

/**
 * @def NEW(x, c)
 * Allocate and zero out c structures of type x
 */
#define NEW(x, c) calloc(c, sizeof(x))

/**
 * @def FREE(x)
 * Free x and set to NULL
 */
#define FREE(x) do { free(x); x = NULL; } while (0)

/**
 * @def IF_FREE(x)
 * If x is set, free x and set to NULL
 */
#define IF_FREE(x) do { if (x) FREE(x); } while (0)

/**
 * @def IF_RELEASE(x)
 * If x is set, eina_stringshare_del x and set to NULL
 */
#define IF_RELEASE(x) do { \
    if (x) { \
        const char *__tmp; __tmp = (x); (x) = NULL; eina_stringshare_del(__tmp); \
    } \
    (x) = NULL; \
} while (0)

/**
 * @def IF_FREE_LIST(x)
 * If x is a valid pointer destroy x and set to NULL
 */
#define IF_FREE_LIST(x) do { \
    if (x) { \
        Ecore_List *__tmp; __tmp = (x); (x) = NULL; ecore_list_destroy(__tmp); \
    } \
    (x) = NULL; \
} while (0)

/**
 * @def IF_FREE_DLIST(x)
 * If x is a valid pointer destroy x and set to NULL
 */
#define IF_FREE_DLIST(x) do { \
    if (x) { \
        Ecore_DList *__tmp; __tmp = (x); (x) = NULL; ecore_dlist_destroy(__tmp); \
    } \
    (x) = NULL; \
} while (0)

/**
 * @def IF_FREE_HASH(x)
 * If x is a valid pointer destroy x and set to NULL
 */
#define IF_FREE_HASH(x) do { \
    if (x) { \
        Ecore_Hash *__tmp; __tmp = (x); (x) = NULL; ecore_hash_destroy(__tmp); \
    } \
    (x) = NULL; \
} while (0)

#ifndef PATH_MAX
/**
 * @def PATH_MAX
 * Convenience define to set the maximim path length
 */
#define PATH_MAX 4096
#endif

/**
 * @internal
 * The different types of commands in an Exec entry
 */
enum Efreet_Desktop_Command_Flag
{
    EFREET_DESKTOP_EXEC_FLAG_FULLPATH = 0x0001,
    EFREET_DESKTOP_EXEC_FLAG_URI      = 0x0002,
    EFREET_DESKTOP_EXEC_FLAG_DIR      = 0x0004,
    EFREET_DESKTOP_EXEC_FLAG_FILE     = 0x0008
};

/**
 * @internal
 * Efreet_Desktop_Command_Flag
 */
typedef enum Efreet_Desktop_Command_Flag Efreet_Desktop_Command_Flag;

/**
 * @internal
 * Efreet_Desktop_Command
 */
typedef struct Efreet_Desktop_Command Efreet_Desktop_Command;

/**
 * @internal
 * Holds information on a desktop Exec command entry
 */
struct Efreet_Desktop_Command
{
  Efreet_Desktop *desktop;
  int num_pending;

  Efreet_Desktop_Command_Flag flags;

  Efreet_Desktop_Command_Cb cb_command;
  Efreet_Desktop_Progress_Cb cb_progress;
  void *data;

  Ecore_List *files; /**< list of Efreet_Desktop_Command_File */
};

/**
 * @internal
 * Efreet_Desktop_Command_File
 */
typedef struct Efreet_Desktop_Command_File Efreet_Desktop_Command_File;

/**
 * @internal
 * Stores information on a file passed to the desktop Exec command
 */
struct Efreet_Desktop_Command_File
{
  Efreet_Desktop_Command *command;
  char *dir;
  char *file;
  char *fullpath;
  char *uri;

  int pending;
};

int efreet_base_init(void);
void efreet_base_shutdown(void);

int efreet_icon_init(void);
void efreet_icon_shutdown(void);

int efreet_menu_init(void);
void efreet_menu_shutdown(void);
Ecore_List *efreet_default_dirs_get(const char *user_dir,
                                    Ecore_List *system_dirs,
                                    const char *suffix);

int efreet_ini_init(void);
int efreet_ini_shutdown(void);

int efreet_desktop_init(void);
int efreet_desktop_shutdown(void);

const char *efreet_home_dir_get(void);

EAPI const char *efreet_lang_get(void);
EAPI const char *efreet_lang_country_get(void);
EAPI const char *efreet_lang_modifier_get(void);

size_t efreet_array_cat(char *buffer, size_t size, const char *strs[]);

const char *efreet_desktop_environment_get(void);

/**
 * @}
 */

#endif
