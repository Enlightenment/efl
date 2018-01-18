#ifndef EFREET_PRIVATE_H
#define EFREET_PRIVATE_H

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

#ifdef ENABLE_NLS
# include <libintl.h>
# define _(str) dgettext(PACKAGE, str)
#else
# define _(str) (str)
#endif

/**
 * @file efreet_private.h
 * @brief Contains methods and defines that are private to the Efreet
 * implementaion
 * @addtogroup Efreet_Private Efreet_Private: Private methods and defines
 * @ingroup Efreet
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
#define IF_RELEASE(x) eina_stringshare_replace(&(x), NULL)

/**
 * @def IF_FREE_LIST(x)
 * If x is a valid pointer destroy x and set to NULL
 */
#define IF_FREE_LIST(list, free_cb) \
  do                                     \
    {                                    \
       void *_data;                      \
       EINA_LIST_FREE(list, _data)       \
         free_cb(_data);                 \
    }                                    \
  while (0)

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
 * @def IF_FREE_HASH_CB(x, cb)
 * If x is a valid pointer destroy x with cb and set to NULL
 */
#define IF_FREE_HASH_CB(x, cb) do { \
    if (x) { \
        Eina_Hash *__tmp; __tmp = (x); (x) = NULL; efreet_hash_free(__tmp, cb); \
    } \
    (x) = NULL; \
} while (0)

#ifdef EFREET_DEFAULT_LOG_COLOR
#undef EFREET_DEFAULT_LOG_COLOR
#endif
#define EFREET_DEFAULT_LOG_COLOR "\033[36m"

#ifndef EFREET_MODULE_LOG_DOM
#error "Need to define a log domain"
#endif

/**
 * macros that are used all around the code for message processing
 * four macros are defined ERR, WRN, DGB, INF. 
 * EFREET_MODULE_LOG_DOM should be defined individually for each module
 */
#ifdef CRI
#undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(EFREET_MODULE_LOG_DOM, __VA_ARGS__)
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

typedef struct _Efreet_Cache_Icon Efreet_Cache_Icon;
typedef struct _Efreet_Cache_Icon_Element Efreet_Cache_Icon_Element;
typedef struct _Efreet_Cache_Fallback_Icon Efreet_Cache_Fallback_Icon;

struct _Efreet_Cache_Icon
{
    const char *theme;

    Efreet_Cache_Icon_Element **icons;
    unsigned int icons_count;
};

struct _Efreet_Cache_Icon_Element
{
    const char **paths;          /* possible paths for icon */
    unsigned int paths_count;

    unsigned short type;         /* size type of icon */

    unsigned short normal;       /* The size for this icon */
    unsigned short min;          /* The minimum size for this icon */
    unsigned short max;          /* The maximum size for this icon */
};

struct _Efreet_Cache_Fallback_Icon
{
    const char *theme;
    const char **icons;
    unsigned int icons_count;
};

typedef struct _Efreet_Cache_Version Efreet_Cache_Version;
struct _Efreet_Cache_Version
{
    unsigned char major;
    unsigned char minor;
};

typedef struct _Efreet_Cache_Hash Efreet_Cache_Hash;
struct _Efreet_Cache_Hash
{
    Eina_Hash *hash;
};

typedef struct _Efreet_Cache_Array_String Efreet_Cache_Array_String;
struct _Efreet_Cache_Array_String
{
    const char **array;
    unsigned int array_count;
};

int efreet_base_init(void);
void efreet_base_shutdown(void);

int efreet_cache_init(void);
void efreet_cache_shutdown(void);

int efreet_icon_init(void);
void efreet_icon_shutdown(void);
void efreet_icon_extensions_refresh(void);

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

const char *efreet_home_dir_get(void);
void        efreet_dirs_reset(void);

EAPI const char *efreet_lang_get(void);
EAPI const char *efreet_lang_country_get(void);
EAPI const char *efreet_lang_modifier_get(void);
EAPI const char *efreet_language_get(void);

size_t efreet_array_cat(char *buffer, size_t size, const char *strs[]);

void efreet_cache_desktop_close(void);
void efreet_cache_desktop_build(void);

Efreet_Desktop *efreet_cache_desktop_find(const char *file);
void efreet_cache_desktop_free(Efreet_Desktop *desktop);
void efreet_cache_desktop_add(Efreet_Desktop *desktop);
void efreet_cache_icon_exts_add(Eina_List *exts);
void efreet_cache_icon_dirs_add(Eina_List *dirs);
Efreet_Cache_Array_String *efreet_cache_desktop_dirs(void);

Efreet_Cache_Icon *efreet_cache_icon_find(Efreet_Icon_Theme *theme, const char *icon);
Efreet_Cache_Fallback_Icon *efreet_cache_icon_fallback_find(const char *icon);
Efreet_Icon_Theme *efreet_cache_icon_theme_find(const char *theme);
Eina_List *efreet_cache_icon_theme_list(void);

Efreet_Cache_Hash *efreet_cache_util_hash_string(const char *key);
Efreet_Cache_Hash *efreet_cache_util_hash_array_string(const char *key);
Efreet_Cache_Array_String *efreet_cache_util_names(const char *key);

EAPI void efreet_cache_array_string_free(Efreet_Cache_Array_String *array);

EAPI void efreet_hash_free(Eina_Hash *hash, Eina_Free_Cb free_cb);
EAPI void efreet_setowner(const char *path);
EAPI void efreet_fsetowner(int fd);

EAPI extern int efreet_cache_update;

EAPI extern void (*_efreet_mime_update_func) (void);

#undef EAPI
#define EAPI

/**
 * @}
 */

#endif
