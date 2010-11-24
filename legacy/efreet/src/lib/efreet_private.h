#ifndef EFREET_PRIVATE_H
#define EFREET_PRIVATE_H

#include <Eet.h>
#include <Ecore.h>
#include <Ecore_File.h>

#undef alloca
#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# include <stddef.h>
# ifdef  __cplusplus
extern "C"
# endif
void *alloca (size_t);
#endif


/**
 * @file efreet_private.h
 * @brief Contains methods and defines that are private to the Efreet
 * implementaion
 * @addtogroup Efreet_Private Efreet_Private: Private methods and defines
 *
 * @{
 */

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
#define IF_FREE_LIST(list, free_cb) do { \
    void *_data; \
    EINA_LIST_FREE(list, _data) \
        free_cb(_data); \
    list = NULL; \
} while (0)

/**
 * @def IF_FREE_HASH(x)
 * If x is a valid pointer destroy x and set to NULL
 */
#define IF_FREE_HASH(x) do { \
    if (x) { \
        Eina_Hash *__tmp; __tmp = (x); (x) = NULL; eina_hash_free(__tmp); \
    } \
    (x) = NULL; \
} while (0)

/**
 * @def _efree_log_domain_global
 * global log domain for efreet (see eina_log module)
 */

extern int _efreet_log_dom_global;
#ifdef EFREET_DEFAULT_LOG_COLOR
#undef EFREET_DEFAULT_LOG_COLOR
#endif
#define EFREET_DEFAULT_LOG_COLOR "\033[36m"

#define EFREET_MODULE_LOG_DOM _efreet_log_dom_global /*default log domain for each module. It can redefined inside each module */
#ifdef ERROR
#undef ERROR
#endif
#define ERROR(...) EINA_LOG_DOM_ERR(EFREET_MODULE_LOG_DOM, __VA_ARGS__)
#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG(...) EINA_LOG_DOM_DBG(EFREET_MODULE_LOG_DOM, __VA_ARGS__)
#ifdef INFO
#undef INFO
#endif
#define INFO(...) EINA_LOG_DOM_INFO(EFREET_MODULE_LOG_DOM, __VA_ARGS__)
#ifdef WARN
#undef WARN
#endif
#define WARN(...) EINA_LOG_DOM_WARN(EFREET_MODULE_LOG_DOM, __VA_ARGS__)

/**
 * macros that are used all around the code for message processing
 * four macros are defined ERR, WRN, DGB, INF. 
 * EFREET_MODULE_LOG_DOM should be defined individually for each module
 */
#define EFREET_MODULE_LOG_DOM _efreet_log_dom_global /*default log domain for each module. It can redefined inside each module */
#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(EFREET_MODULE_LOG_DOM, __VA_ARGS__)
#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(EFREET_MODULE_LOG_DOM, __VA_ARGS__)
#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(EFREET_MODULE_LOG_DOM, __VA_ARGS__)
#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(EFREET_MODULE_LOG_DOM, __VA_ARGS__)

extern Eina_Hash *efreet_desktop_cache;

#ifdef ICON_CACHE
typedef struct Efreet_Cache_Icon Efreet_Cache_Icon;
struct Efreet_Cache_Icon
{
    const char *theme;
    Eina_List  *icons;
    unsigned char fallback;
    unsigned char free:1;
};

typedef struct Efreet_Cache_Icon_Element Efreet_Cache_Icon_Element;
struct Efreet_Cache_Icon_Element
{
    int        type;  /* size type of icon */
    Eina_List *paths; /* possible paths for icon */
    struct
    {
        unsigned short normal;        /* The size for this icon */
        unsigned short min;           /* The minimum size for this icon */
        unsigned short max;           /* The maximum size for this icon */
    } size;
};
#endif

int efreet_base_init(void);
void efreet_base_shutdown(void);

int efreet_cache_init(void);
void efreet_cache_shutdown(void);

int efreet_icon_init(void);
void efreet_icon_shutdown(void);

int efreet_menu_init(void);
void efreet_menu_shutdown(void);
EAPI Eina_List *efreet_default_dirs_get(const char *user_dir,
                                        Eina_List *system_dirs,
                                        const char *suffix);

int efreet_ini_init(void);
void efreet_ini_shutdown(void);

int efreet_desktop_init(void);
void efreet_desktop_shutdown(void);

int efreet_util_init(void);
int efreet_util_shutdown(void);

EAPI const char *efreet_home_dir_get(void);

const char *efreet_lang_get(void);
const char *efreet_lang_country_get(void);
const char *efreet_lang_modifier_get(void);

size_t efreet_array_cat(char *buffer, size_t size, const char *strs[]);

const char *efreet_desktop_environment_get(void);

EAPI Eet_Data_Descriptor *efreet_desktop_edd_init(void);

void efreet_util_desktop_cache_reload(void);
EAPI const char *efreet_desktop_util_cache_file(void);
EAPI const char *efreet_desktop_cache_file(void);
EAPI const char *efreet_desktop_cache_dirs(void);
int efreet_desktop_write_cache_dirs_file(void);

void efreet_cache_desktop_update(void);
#ifdef ICON_CACHE
void efreet_cache_icon_update(void);
#endif
void efreet_cache_desktop_free(Efreet_Desktop *desktop);
Efreet_Desktop *efreet_cache_desktop_find(const char *file);

#ifdef ICON_CACHE
EAPI const char *efreet_icon_cache_file(const char *theme);

EAPI Eet_Data_Descriptor *efreet_icon_edd_init(void);
EAPI Eet_Data_Descriptor *efreet_icon_fallback_edd_init(void);
EAPI void efreet_cache_icon_free(Efreet_Cache_Icon *icon);
Efreet_Cache_Icon *efreet_cache_icon_find(Efreet_Icon_Theme *theme, const char *icon);
Efreet_Cache_Icon *efreet_cache_icon_fallback_find(const char *icon);
#endif

#define NON_EXISTING (void *)-1

EAPI extern int efreet_cache_update;

/**
 * @}
 */

#endif
