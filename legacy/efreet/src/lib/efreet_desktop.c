#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <dirent.h>

#ifdef _WIN32
# include <winsock2.h>
#endif

#include <Ecore.h>
#include <Ecore_File.h>

#include "Efreet.h"
#include "efreet_private.h"

#define DESKTOP_VERSION "1.0"

typedef struct _Efreet_Old_Cache Efreet_Old_Cache;

struct _Efreet_Old_Cache
{
    Eina_Hash *desktop_cache;
    Eet_File *cache;
};

/**
 * The current desktop environment (e.g. "Enlightenment" or "Gnome")
 */
static const char *desktop_environment = NULL;

/**
 * A cache of all loaded desktops, hashed by file name.
 * Values are Efreet_Desktop structures
 */
static Eina_Hash *efreet_desktop_cache = NULL;

/**
 * A list of the desktop types available
 */
static Eina_List *efreet_desktop_types = NULL;

/**
 * A cache of all unknown desktop dirs
 */
static Eina_List *efreet_desktop_dirs = NULL;

/**
 * A job pointer for cache updates
 */
static Ecore_Job           *efreet_desktop_job = NULL;
static Ecore_Exe           *efreet_desktop_exe = NULL;
static int                  efreet_desktop_exe_lock = -1;
static Ecore_Event_Handler *efreet_desktop_exe_handler = NULL;

/**
 * Data for cache files
 */
static const char          *cache_dirs = NULL;
static const char          *cache_file = NULL;
static Eet_File            *cache = NULL;
static Eet_Data_Descriptor *desktop_edd = NULL;
static Ecore_File_Monitor  *cache_monitor = NULL;

static Eina_List           *old_caches = NULL;

static Eina_Hash           *change_monitors = NULL;

#ifdef EFREET_MODULE_LOG_DOM
#undef EFREET_MODULE_LOG_DOM
#endif

#define EFREET_MODULE_LOG_DOM _efreet_desktop_log_dom
int _efreet_desktop_log_dom = -1;

EAPI int EFREET_DESKTOP_TYPE_APPLICATION = 0;
EAPI int EFREET_DESKTOP_TYPE_LINK = 0;
EAPI int EFREET_DESKTOP_TYPE_DIRECTORY = 0;

EAPI int EFREET_EVENT_DESKTOP_CACHE_UPDATE = 0;

/**
 * @internal
 * Information about custom types
 */
typedef struct Efreet_Desktop_Type_Info Efreet_Desktop_Type_Info;
struct Efreet_Desktop_Type_Info
{
    int id;
    const char *type;
    Efreet_Desktop_Type_Parse_Cb parse_func;
    Efreet_Desktop_Type_Save_Cb save_func;
    Efreet_Desktop_Type_Free_Cb free_func;
};

static int efreet_desktop_cache_check(Efreet_Desktop *desktop);
static int efreet_desktop_read(Efreet_Desktop *desktop);
static Efreet_Desktop_Type_Info *efreet_desktop_type_parse(const char *type_str);
static void efreet_desktop_type_info_free(Efreet_Desktop_Type_Info *info);
static void *efreet_desktop_application_fields_parse(Efreet_Desktop *desktop,
                                                    Efreet_Ini *ini);
static void efreet_desktop_application_fields_save(Efreet_Desktop *desktop,
                                                    Efreet_Ini *ini);
static void *efreet_desktop_link_fields_parse(Efreet_Desktop *desktop,
                                                Efreet_Ini *ini);
static void efreet_desktop_link_fields_save(Efreet_Desktop *desktop,
                                                Efreet_Ini *ini);
static int efreet_desktop_generic_fields_parse(Efreet_Desktop *desktop,
                                                Efreet_Ini *ini);
static void efreet_desktop_generic_fields_save(Efreet_Desktop *desktop,
                                                Efreet_Ini *ini);
static Eina_Bool efreet_desktop_x_fields_parse(const Eina_Hash *hash,
                                                const void *key,
                                                void *data,
                                                void *fdata);
static Eina_Bool efreet_desktop_x_fields_save(const Eina_Hash *hash,
                                                const void *key,
                                                void *value,
                                                void *fdata);
static int efreet_desktop_environment_check(Efreet_Ini *ini);

static int efreet_desktop_write_cache_dirs_file(void);

static void efreet_desktop_cache_update_cb(void *data, Ecore_File_Monitor *em,
                                           Ecore_File_Event event, const char *path);
static void efreet_desktop_cache_update_free(void *data, void *ev);

static void efreet_desktop_update_cache(void);
static void efreet_desktop_update_cache_job(void *data);
static Eina_Bool efreet_desktop_exe_cb(void *data, int type, void *event);

static void efreet_desktop_changes_listen(void);
static void efreet_desktop_changes_listen_recursive(const char *path);
static void efreet_desktop_changes_monitor_add(const char *path);
static void efreet_desktop_changes_cb(void *data, Ecore_File_Monitor *em,
                                             Ecore_File_Event event, const char *path);

/**
 * @internal
 * @return Returns > 0 on success or 0 on failure
 * @brief Initialize the Desktop parser subsystem
 */
int
efreet_desktop_init(void)
{
    char buf[PATH_MAX];

    _efreet_desktop_log_dom = eina_log_domain_register("Efreet_desktop", EFREET_DEFAULT_LOG_COLOR);
    if (_efreet_desktop_log_dom < 0)
    {
        ERROR("Efreet: Could not create a log domain for Efreet_desktop");
        return 0;
    }
    if (!ecore_file_init())
        goto ecore_error;
    desktop_edd = efreet_desktop_edd_init();
    if (!desktop_edd)
        goto edd_error;

    efreet_desktop_cache = eina_hash_string_superfast_new(NULL);
    efreet_desktop_types = NULL;

    EFREET_DESKTOP_TYPE_APPLICATION = efreet_desktop_type_add("Application",
                                        efreet_desktop_application_fields_parse,
                                        efreet_desktop_application_fields_save,
                                        NULL);
    EFREET_DESKTOP_TYPE_LINK = efreet_desktop_type_add("Link",
                                    efreet_desktop_link_fields_parse,
                                    efreet_desktop_link_fields_save, NULL);
    EFREET_DESKTOP_TYPE_DIRECTORY = efreet_desktop_type_add("Directory", NULL,
                                                                NULL, NULL);

    EFREET_EVENT_DESKTOP_CACHE_UPDATE = ecore_event_type_new();

    snprintf(buf, sizeof(buf), "%s/.efreet", efreet_home_dir_get());
    if (!ecore_file_mkpath(buf)) goto cache_error;

    if (efreet_cache_update)
    {
        efreet_desktop_exe_handler = ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
                                                             efreet_desktop_exe_cb, NULL);
        if (!efreet_desktop_exe_handler) goto cache_error;

        cache_monitor = ecore_file_monitor_add(buf,
                                               efreet_desktop_cache_update_cb,
                                               NULL);
        if (!cache_monitor) goto handler_error;

        efreet_desktop_changes_listen();

        ecore_exe_run(PACKAGE_LIB_DIR "/efreet/efreet_desktop_cache_create", NULL);

    }

    /* TODO: Defer eet open until we actually need it open. */
    cache = eet_open(efreet_desktop_cache_file(), EET_FILE_MODE_READ);

    return 1;

handler_error:
    if (efreet_desktop_exe_handler) ecore_event_handler_del(efreet_desktop_exe_handler);
cache_error:
    if (efreet_desktop_cache) eina_hash_free(efreet_desktop_cache);
edd_error:
    ecore_file_shutdown();
ecore_error:
    eina_log_domain_unregister(_efreet_desktop_log_dom);
    return 0;
}

/**
 * @internal
 * @returns the number of initializations left for this system
 * @brief Attempts to shut down the subsystem if nothing else is using it
 */
void
efreet_desktop_shutdown(void)
{
    Efreet_Desktop_Type_Info *info;
    char *dir;

    if (efreet_desktop_exe_handler) ecore_event_handler_del(efreet_desktop_exe_handler);
    IF_RELEASE(desktop_environment);
    IF_FREE_HASH(efreet_desktop_cache);
    EINA_LIST_FREE(efreet_desktop_types, info)
        efreet_desktop_type_info_free(info);
    EINA_LIST_FREE(efreet_desktop_dirs, dir)
        eina_stringshare_del(dir);
    if (cache_monitor) ecore_file_monitor_del(cache_monitor);
    if (change_monitors) eina_hash_free(change_monitors);
    if (cache) eet_close(cache);
    efreet_desktop_edd_shutdown(desktop_edd);
    ecore_file_shutdown();
    eina_log_domain_unregister(_efreet_desktop_log_dom);
    IF_RELEASE(cache_file);
    IF_RELEASE(cache_dirs);
    if (efreet_desktop_job)
      {
         ecore_job_del(efreet_desktop_job);
         efreet_desktop_job = NULL;
      }
}

/**
 * @param file: The file to get the Efreet_Desktop from
 * @return Returns a reference to a cached Efreet_Desktop on success, NULL
 * on failure. This reference should not be freed.
 * @brief Gets a reference to an Efreet_Desktop structure representing the
 * contents of @a file or NULL if @a file is not a valid .desktop file.
 *
 * By using efreet_desktop_get the Efreet_Desktop will be saved in an internal
 * cache, and changes will be signalled by events.
 *
 * Efreet will also try to save all files fetched by efreet_desktop_get in a
 * cache to speed up further requests.
 */
EAPI Efreet_Desktop *
efreet_desktop_get(const char *file)
{
    /* TODO: Check if we need to differentiate between desktop_new and desktop_get */
    Efreet_Desktop *desktop;

    if (!file) return NULL;
    if (efreet_desktop_cache)
    {
        char rp[PATH_MAX];

        if (!realpath(file, rp)) return NULL;
        desktop = eina_hash_find(efreet_desktop_cache, rp);
        if (desktop)
        {
            if (efreet_desktop_cache_check(desktop))
            {
                desktop->ref++;
                return desktop;
            }

            desktop->cached = 0;
            eina_hash_del_by_key(efreet_desktop_cache, rp);
        }
    }

    desktop = efreet_desktop_new(file);
    if (!desktop) return NULL;

    if (!desktop->eet)
    {
        char buf[PATH_MAX];
        char *p;

        /*
         * Read file from disk, save path in cache so it will be included in next
         * cache update
         */
        strncpy(buf, desktop->orig_path, PATH_MAX);
        buf[PATH_MAX - 1] = '\0';
        p = dirname(buf);
        if (!eina_list_search_unsorted(efreet_desktop_dirs, EINA_COMPARE_CB(strcmp), p))
            efreet_desktop_dirs = eina_list_append(efreet_desktop_dirs, eina_stringshare_add(p));
        efreet_desktop_update_cache();
    }

    if (efreet_desktop_cache) eina_hash_add(efreet_desktop_cache, desktop->orig_path, desktop);
    desktop->cached = 1;
    return desktop;
}

/**
 * @param desktop: The Efreet_Desktop to ref
 * @return Returns the new reference count
 */
EAPI int
efreet_desktop_ref(Efreet_Desktop *desktop)
{
    if (!desktop) return 0;
    desktop->ref++;
    return desktop->ref;
}

/**
 * @param file: The file to create the Efreet_Desktop from
 * @return Returns a new empty_Efreet_Desktop on success, NULL on failure
 * @brief Creates a new empty Efreet_Desktop structure or NULL on failure
 */
EAPI Efreet_Desktop *
efreet_desktop_empty_new(const char *file)
{
    Efreet_Desktop *desktop;

    desktop = NEW(Efreet_Desktop, 1);
    if (!desktop) return NULL;

    desktop->orig_path = strdup(file);
    desktop->load_time = ecore_file_mod_time(file);

    desktop->ref = 1;

    return desktop;
}

/**
 * @param file: The file to create the Efreet_Desktop from
 * @return Returns a new Efreet_Desktop on success, NULL on failure
 * @brief Creates a new Efreet_Desktop structure initialized from the
 * contents of @a file or NULL on failure
 *
 * By using efreet_desktop_new the caller will get a unique copy of a
 * Efreet_Desktop. The Efreet_Desktop should immidiatly after use be free'd,
 * as there is no guarantee how long the pointers will be valid.
 */
EAPI Efreet_Desktop *
efreet_desktop_new(const char *file)
{
    Efreet_Desktop *desktop = NULL;

    if (!file) return NULL;
    if (cache)
    {
        char rp[PATH_MAX];
        if (!realpath(file, rp)) return NULL;

        desktop = eet_data_read(cache, desktop_edd, rp);
        if (desktop && desktop->load_time == ecore_file_mod_time(desktop->orig_path))
        {
            desktop->ref = 1;
            desktop->eet = 1;
            return desktop;
        }
    }
    return efreet_desktop_uncached_new(file);
}

/**
 * @param file: The file to create the Efreet_Desktop from
 * @return Returns a new Efreet_Desktop on success, NULL on failure
 * @brief Creates a new Efreet_Desktop structure initialized from the
 * contents of @a file or NULL on failure
 *
 * By using efreet_desktop_uncached_new the Efreet_Desktop structure will be
 * read from disk, and not from any cache.
 */
EAPI Efreet_Desktop *
efreet_desktop_uncached_new(const char *file)
{
    Efreet_Desktop *desktop = NULL;
    char rp[PATH_MAX];

    if (!file) return NULL;
    if (!realpath(file, rp)) return NULL;
    if (!ecore_file_exists(rp)) return NULL;

    desktop = NEW(Efreet_Desktop, 1);
    if (!desktop) return NULL;
    desktop->orig_path = strdup(rp);
    desktop->ref = 1;
    if (!efreet_desktop_read(desktop))
    {
        efreet_desktop_free(desktop);
        return NULL;
    }

    return desktop;
}

/**
 * @param desktop: The desktop file to save
 * @return Returns 1 on success or 0 on failure
 * @brief Saves any changes made to @a desktop back to the file on the
 * filesystem
 */
EAPI int
efreet_desktop_save(Efreet_Desktop *desktop)
{
    Efreet_Desktop_Type_Info *info;
    Efreet_Ini *ini;
    int ok = 1;

    ini = efreet_ini_new(NULL);
    if (!ini) return 0;
    efreet_ini_section_add(ini, "Desktop Entry");
    efreet_ini_section_set(ini, "Desktop Entry");

    info = eina_list_nth(efreet_desktop_types, desktop->type);
    if (info)
    {
        efreet_ini_string_set(ini, "Type", info->type);
        if (info->save_func) info->save_func(desktop, ini);
    }
    else
        ok = 0;

    if (ok)
    {
        char *val;

        if (desktop->only_show_in)
        {
            val = efreet_desktop_string_list_join(desktop->only_show_in);
            if (val)
            {
                efreet_ini_string_set(ini, "OnlyShowIn", val);
                FREE(val);
            }
        }
        if (desktop->not_show_in)
        {
            val = efreet_desktop_string_list_join(desktop->not_show_in);
            if (val)
            {
                efreet_ini_string_set(ini, "NotShowIn", val);
                FREE(val);
            }
        }
        efreet_desktop_generic_fields_save(desktop, ini);
        /* When we save the file, it should be updated to the
         * latest version that we support! */
        efreet_ini_string_set(ini, "Version", DESKTOP_VERSION);

        if (!efreet_ini_save(ini, desktop->orig_path)) ok = 0;
    }
    efreet_ini_free(ini);
    return ok;
}

/**
 * @param desktop: The desktop file to save
 * @param file: The filename to save as
 * @return Returns 1 on success or 0 on failure
 * @brief Saves @a desktop to @a file
 */
EAPI int
efreet_desktop_save_as(Efreet_Desktop *desktop, const char *file)
{
    if (desktop->cached && efreet_desktop_cache &&
        desktop == eina_hash_find(efreet_desktop_cache, desktop->orig_path))
    {
        desktop->cached = 0;
        eina_hash_del_by_key(efreet_desktop_cache, desktop->orig_path);
    }
    FREE(desktop->orig_path);
    desktop->orig_path = strdup(file);
    return efreet_desktop_save(desktop);
}

/**
 * @internal
 * @param desktop: The Efreet_Desktop to work with
 * @return Returns no value
 * @brief Frees the Efreet_Desktop structure and all of it's data
 */
EAPI void
efreet_desktop_free(Efreet_Desktop *desktop)
{
    if (!desktop) return;

    desktop->ref--;
    if (desktop->ref > 0) return;

    if (desktop->cached)
    {
       if (efreet_desktop_cache &&
           desktop == eina_hash_find(efreet_desktop_cache, desktop->orig_path))
       {
           eina_hash_del_by_key(efreet_desktop_cache, desktop->orig_path);
       }
       else if (old_caches)
       {
           Efreet_Old_Cache *d;
           Efreet_Desktop *curr;
           Eina_List *l;

           EINA_LIST_FOREACH(old_caches, l, d)
           {
               curr = eina_hash_find(d->desktop_cache, desktop->orig_path);
               if (curr && curr == desktop)
               {
                   eina_hash_del_by_key(d->desktop_cache, desktop->orig_path);
                   break;
               }
           }
       }
    }

    if (desktop->eet)
    {
        eina_list_free(desktop->only_show_in);
        eina_list_free(desktop->not_show_in);
        eina_list_free(desktop->categories);
        eina_list_free(desktop->mime_types);
        IF_FREE_HASH(desktop->x);
    }
    else
    {
        char *str;

        IF_FREE(desktop->orig_path);

        IF_FREE(desktop->version);
        IF_FREE(desktop->name);
        IF_FREE(desktop->generic_name);
        IF_FREE(desktop->comment);
        IF_FREE(desktop->icon);
        IF_FREE(desktop->url);

        IF_FREE(desktop->try_exec);
        IF_FREE(desktop->exec);
        IF_FREE(desktop->path);
        IF_FREE(desktop->startup_wm_class);

        IF_FREE_LIST(desktop->only_show_in, free);
        IF_FREE_LIST(desktop->not_show_in, free);

        EINA_LIST_FREE(desktop->categories, str)
            eina_stringshare_del(str);
        EINA_LIST_FREE(desktop->mime_types, str)
            eina_stringshare_del(str);

        IF_FREE_HASH(desktop->x);

        if (desktop->type_data)
        {
            Efreet_Desktop_Type_Info *info;
            info = eina_list_nth(efreet_desktop_types, desktop->type);
            if (info->free_func)
                info->free_func(desktop->type_data);
        }
    }
    FREE(desktop);
}

/**
 * @param environment: the environment name
 * @brief sets the global desktop environment name
 */
EAPI void
efreet_desktop_environment_set(const char *environment)
{
    if (desktop_environment) eina_stringshare_del(desktop_environment);
    if (environment) desktop_environment = eina_stringshare_add(environment);
    else desktop_environment = NULL;
}

/**
 * @return environment: the environment name
 * @brief sets the global desktop environment name
 */
EAPI const char *
efreet_desktop_environment_get(void)
{
    return desktop_environment;
}

/**
 * @param desktop: The desktop to work with
 * @return Returns the number of categories assigned to this desktop
 * @brief Retrieves the number of categories the given @a desktop belongs
 * too
 */
EAPI unsigned int
efreet_desktop_category_count_get(Efreet_Desktop *desktop)
{
    if (!desktop || !desktop->categories) return 0;
    return eina_list_count(desktop->categories);
}

/**
 * @param desktop: the desktop
 * @param category: the category name
 * @brief add a category to a desktop
 */
EAPI void
efreet_desktop_category_add(Efreet_Desktop *desktop, const char *category)
{
    if (!desktop) return;

    if (eina_list_search_unsorted(desktop->categories,
                                  EINA_COMPARE_CB(strcmp), category)) return;

    desktop->categories = eina_list_append(desktop->categories,
                        (void *)eina_stringshare_add(category));
}

/**
 * @param desktop: the desktop
 * @param category: the category name
 * @brief removes a category from a desktop
 * @return 1 if the desktop had his category listed, 0 otherwise
 */
EAPI int
efreet_desktop_category_del(Efreet_Desktop *desktop, const char *category)
{
    char *found = NULL;

    if (!desktop || !desktop->categories) return 0;

    if ((found = eina_list_search_unsorted(desktop->categories,
                                           EINA_COMPARE_CB(strcmp), category)))
    {
        eina_stringshare_del(found);
        desktop->categories = eina_list_remove(desktop->categories, found);

        return 1;
    }

    return 0;
}

/**
 * @param type: The type to add to the list of matching types
 * @param parse_func: a function to parse out custom fields
 * @param save_func: a function to save data returned from @a parse_func
 * @param free_func: a function to free data returned from @a parse_func
 * @return Returns the id of the new type
 * @brief Adds the given type to the list of types in the system
 */
EAPI int
efreet_desktop_type_add(const char *type, Efreet_Desktop_Type_Parse_Cb parse_func,
                        Efreet_Desktop_Type_Save_Cb save_func,
                        Efreet_Desktop_Type_Free_Cb free_func)
{
    int id;
    Efreet_Desktop_Type_Info *info;

    info = NEW(Efreet_Desktop_Type_Info, 1);
    if (!info) return 0;

    id = eina_list_count(efreet_desktop_types);

    info->id = id;
    info->type = eina_stringshare_add(type);
    info->parse_func = parse_func;
    info->save_func = save_func;
    info->free_func = free_func;

    efreet_desktop_types = eina_list_append(efreet_desktop_types, info);

    return id;
}

/**
 * @brief Add an alias for an existing desktop type.
 * @param from_type the type to alias (e.g. EFREE_DESKTOP_TYPE_APPLICATION)
 * @param alias the alias
 * @return the new type id, or -1 if @p from_type was not valid
 *
 * This allows applications to add non-standard types that behave exactly as standard types.
 */
EAPI int
efreet_desktop_type_alias(int from_type, const char *alias)
{
    Efreet_Desktop_Type_Info *info;
    info = eina_list_nth(efreet_desktop_types, from_type);
    if (!info) return -1;

    return efreet_desktop_type_add(alias, info->parse_func, info->save_func, info->free_func);
}

EAPI Eina_Bool
efreet_desktop_x_field_set(Efreet_Desktop *desktop, const char *key, const char *data)
{
    if (!desktop || strncmp(key, "X-", 2))
        return EINA_FALSE;

    if (!desktop->x)
        desktop->x = eina_hash_string_superfast_new(EINA_FREE_CB(eina_stringshare_del));

    eina_hash_del_by_key(desktop->x, key);
    eina_hash_add(desktop->x, key, eina_stringshare_add(data));

    return EINA_TRUE;
}

EAPI const char *
efreet_desktop_x_field_get(Efreet_Desktop *desktop, const char *key)
{
    const char *ret;

    if (!desktop || strncmp(key, "X-", 2))
        return NULL;

    if (!desktop->x)
        return NULL;

    ret = eina_hash_find(desktop->x, key);
    if (!ret)
        return NULL;

    return eina_stringshare_add(ret);
}

EAPI Eina_Bool
efreet_desktop_x_field_del(Efreet_Desktop *desktop, const char *key)
{
    if (!desktop || strncmp(key, "X-", 2))
        return EINA_FALSE;

    if (!desktop->x)
        return EINA_FALSE;

    return eina_hash_del_by_key(desktop->x, key);
}

/**
 * @brief get type specific data for custom desktop types
 * @param desktop the desktop
 * @return type specific data, or NULL if there is none
 */
EAPI void *
efreet_desktop_type_data_get(Efreet_Desktop *desktop)
{
    return desktop->type_data;
}

/**
 * @param string: the raw string list
 * @return an Eina_List of ecore string's
 * @brief Parse ';' separate list of strings according to the desktop spec
 */
EAPI Eina_List *
efreet_desktop_string_list_parse(const char *string)
{
    Eina_List *list = NULL;
    char *tmp;
    char *s, *p;
    size_t len;

    if (!string) return NULL;

    len = strlen(string) + 1;
    tmp = alloca(len);
    memcpy(tmp, string, len);
    s = tmp;

    while ((p = strchr(s, ';')))
    {
        if (p > tmp && *(p-1) == '\\') continue;
        *p = '\0';
        list = eina_list_append(list, (void *)eina_stringshare_add(s));
        s = p + 1;
    }
    /* If this is true, the .desktop file does not follow the standard */
    if (*s)
    {
#ifdef STRICT_SPEC
        WRN("[Efreet]: Found a string list without ';' "
                "at the end: %s", string);
#endif
        list = eina_list_append(list, (void *)eina_stringshare_add(s));
    }

    return list;
}

/**
 * @param list: Eina_List with strings
 * @return a raw string list
 * @brief Create a ';' separate list of strings according to the desktop spec
 */
EAPI char *
efreet_desktop_string_list_join(Eina_List *list)
{
    Eina_List *l;
    const char *elem;
    char *string;
    size_t size, pos, len;

    if (!list) return strdup("");

    size = 1024;
    string = malloc(size);
    if (!string) return NULL;
    pos = 0;

    EINA_LIST_FOREACH(list, l, elem)
    {
        len = strlen(elem);
        /* +1 for ';' */
        if ((len + pos + 1) >= size)
        {
            char *tmp;
            size = len + pos + 1024;
            tmp = realloc(string, size);
            if (!tmp)
            {
                free(string);
                return NULL;
            }
            string = tmp;
        }
        strcpy(string + pos, elem);
        pos += len;
        strcpy(string + pos, ";");
        pos += 1;
    }
    return string;
}

/*
 * Needs EAPI because of helper binaries
 */
EAPI const char *
efreet_desktop_cache_file(void)
{
    char tmp[PATH_MAX] = { '\0' };
    const char *home, *lang, *country, *modifier;

    if (cache_file) return cache_file;

    home = efreet_home_dir_get();
    lang = efreet_lang_get();
    country = efreet_lang_country_get();
    modifier = efreet_lang_modifier_get();

    if (lang && country && modifier)
        snprintf(tmp, sizeof(tmp), "%s/.efreet/desktop_%s_%s@%s.cache", home, lang, country, modifier);
    else if (lang && country)
        snprintf(tmp, sizeof(tmp), "%s/.efreet/desktop_%s_%s.cache", home, lang, country);
    else if (lang)
        snprintf(tmp, sizeof(tmp), "%s/.efreet/desktop_%s.cache", home, lang);
    else
        snprintf(tmp, sizeof(tmp), "%s/.efreet/desktop.cache", home);

    cache_file = eina_stringshare_add(tmp);
    return cache_file;
}

/*
 * Needs EAPI because of helper binaries
 */
EAPI const char *
efreet_desktop_cache_dirs(void)
{
    char tmp[PATH_MAX] = { '\0' };

    if (cache_dirs) return cache_dirs;

    snprintf(tmp, sizeof(tmp), "%s/.efreet/desktop_dirs.cache", efreet_home_dir_get());

    cache_dirs = eina_stringshare_add(tmp);
    return cache_dirs;
}

/**
 * @internal
 * @param desktop: The desktop to check
 * @return Returns 1 if the cache is still valid, 0 otherwise
 * @brief This will check if the desktop cache is still valid.
 */
static int
efreet_desktop_cache_check(Efreet_Desktop *desktop)
{
    if (!desktop) return 0;

    /* have we modified this file since we last read it in? */
    if (ecore_file_mod_time(desktop->orig_path) != desktop->load_time)
        return 0;

    return 1;
}

/**
 * @internal
 * @param desktop: The desktop to fill
 * @return Returns 1 on success, 0 on failure
 * @brief initialize an Efreet_Desktop from the contents of @a file
 */
static int
efreet_desktop_read(Efreet_Desktop *desktop)
{
    Efreet_Ini *ini;
    int error = 0;
    int ok;

    ini = efreet_ini_new(desktop->orig_path);
    if (!ini) return 0;
    if (!ini->data)
    {
        efreet_ini_free(ini);
        return 0;
    }

    ok = efreet_ini_section_set(ini, "Desktop Entry");
    if (!ok) ok = efreet_ini_section_set(ini, "KDE Desktop Entry");
    if (!ok)
    {
        ERR("efreet_desktop_new error: no Desktop Entry section");
        error = 1;
    }

    if (!error)
    {
        Efreet_Desktop_Type_Info *info;

        info = efreet_desktop_type_parse(efreet_ini_string_get(ini, "Type"));
        if (info)
        {
            const char *val;

            desktop->type = info->id;
            val = efreet_ini_string_get(ini, "Version");
            if (val) desktop->version = strdup(val);

            if (info->parse_func)
                desktop->type_data = info->parse_func(desktop, ini);
        }
        else
            error = 1;
    }

    if (!error && !efreet_desktop_environment_check(ini)) error = 1;
    if (!error && !efreet_desktop_generic_fields_parse(desktop, ini)) error = 1;
    if (!error)
        eina_hash_foreach(ini->section, efreet_desktop_x_fields_parse, desktop);

    efreet_ini_free(ini);

    desktop->load_time = ecore_file_mod_time(desktop->orig_path);

    if (error) return 0;

    return 1;
}

/**
 * @internal
 * @param type_str: the type as a string
 * @return the parsed type
 * @brief parse the type string into an Efreet_Desktop_Type
 */
static Efreet_Desktop_Type_Info *
efreet_desktop_type_parse(const char *type_str)
{
    Efreet_Desktop_Type_Info *info;
    Eina_List *l;

    if (!type_str) return NULL;

    EINA_LIST_FOREACH(efreet_desktop_types, l, info)
    {
        if (!strcmp(info->type, type_str))
            return info;
    }

    return NULL;
}

/**
 * @internal
 * @brief Free an Efreet Desktop_Type_Info struct
 */
static void
efreet_desktop_type_info_free(Efreet_Desktop_Type_Info *info)
{
    if (!info) return;
    IF_RELEASE(info->type);
    free(info);
}

/**
 * @internal
 * @param desktop: the Efreet_Desktop to store parsed fields in
 * @param ini: the Efreet_Ini to parse fields from
 * @return No value
 * @brief Parse application specific desktop fields
 */
static void *
efreet_desktop_application_fields_parse(Efreet_Desktop *desktop, Efreet_Ini *ini)
{
    const char *val;

    val = efreet_ini_string_get(ini, "TryExec");
    if (val) desktop->try_exec = strdup(val);

    val = efreet_ini_string_get(ini, "Exec");
    if (val) desktop->exec = strdup(val);

    val = efreet_ini_string_get(ini, "Path");
    if (val) desktop->path = strdup(val);

    val = efreet_ini_string_get(ini, "StartupWMClass");
    if (val) desktop->startup_wm_class = strdup(val);

    desktop->categories = efreet_desktop_string_list_parse(
                                efreet_ini_string_get(ini, "Categories"));
    desktop->mime_types = efreet_desktop_string_list_parse(
                                efreet_ini_string_get(ini, "MimeType"));

    desktop->terminal = efreet_ini_boolean_get(ini, "Terminal");
    desktop->startup_notify = efreet_ini_boolean_get(ini, "StartupNotify");

    return NULL;
}

/**
 * @internal
 * @param desktop: the Efreet_Desktop to save fields from
 * @param ini: the Efreet_Ini to save fields to
 * @return Returns no value
 * @brief Save application specific desktop fields
 */
static void
efreet_desktop_application_fields_save(Efreet_Desktop *desktop, Efreet_Ini *ini)
{
    char *val;

    if (desktop->try_exec)
        efreet_ini_string_set(ini, "TryExec", desktop->try_exec);

    if (desktop->exec)
        efreet_ini_string_set(ini, "Exec", desktop->exec);

    if (desktop->path)
        efreet_ini_string_set(ini, "Path", desktop->path);

    if (desktop->startup_wm_class)
        efreet_ini_string_set(ini, "StartupWMClass", desktop->startup_wm_class);

    if (desktop->categories)
    {
        val = efreet_desktop_string_list_join(desktop->categories);
        if (val)
        {
            efreet_ini_string_set(ini, "Categories", val);
            FREE(val);
        }
    }

    if (desktop->mime_types)
    {
        val = efreet_desktop_string_list_join(desktop->mime_types);
        if (val)
        {
           efreet_ini_string_set(ini, "MimeType", val);
           FREE(val);
        }
    }

    efreet_ini_boolean_set(ini, "Terminal", desktop->terminal);
    efreet_ini_boolean_set(ini, "StartupNotify", desktop->startup_notify);
}

/**
 * @internal
 * @param desktop: the Efreet_Desktop to store parsed fields in
 * @param ini: the Efreet_Ini to parse fields from
 * @return Returns no value
 * @brief Parse link specific desktop fields
 */
static void *
efreet_desktop_link_fields_parse(Efreet_Desktop *desktop, Efreet_Ini *ini)
{
    const char *val;

    val = efreet_ini_string_get(ini, "URL");
    if (val) desktop->url = strdup(val);
    return NULL;
}

/**
 * @internal
 * @param desktop: the Efreet_Desktop to save fields from
 * @param ini: the Efreet_Ini to save fields in
 * @return Returns no value
 * @brief Save link specific desktop fields
 */
static void
efreet_desktop_link_fields_save(Efreet_Desktop *desktop, Efreet_Ini *ini)
{
    if (desktop->url) efreet_ini_string_set(ini, "URL", desktop->url);
}

/**
 * @internal
 * @param desktop: the Efreet_Desktop to store parsed fields in
 * @param ini: the Efreet_Ini to parse fields from
 * @return 1 if parsed successfully, 0 otherwise
 * @brief Parse desktop fields that all types can include
 */
static int
efreet_desktop_generic_fields_parse(Efreet_Desktop *desktop, Efreet_Ini *ini)
{
    const char *val;

    val = efreet_ini_localestring_get(ini, "Name");
    if (val) desktop->name = strdup(val);
    else
    {
        ERR("efreet_desktop_generic_fields_parse error: no Name");
        return 0;
    }

    val = efreet_ini_localestring_get(ini, "GenericName");
    if (val) desktop->generic_name = strdup(val);

    val = efreet_ini_localestring_get(ini, "Comment");
    if (val) desktop->comment = strdup(val);

    val = efreet_ini_localestring_get(ini, "Icon");
    if (val) desktop->icon = strdup(val);

    desktop->no_display = efreet_ini_boolean_get(ini, "NoDisplay");
    desktop->hidden = efreet_ini_boolean_get(ini, "Hidden");

    return 1;
}

/**
 * @internal
 * @param desktop: the Efreet_Desktop to save fields from
 * @param ini: the Efreet_Ini to save fields to
 * @return Returns nothing
 * @brief Save desktop fields that all types can include
 */
static void
efreet_desktop_generic_fields_save(Efreet_Desktop *desktop, Efreet_Ini *ini)
{
    const char *val;

    if (desktop->name)
    {
        efreet_ini_localestring_set(ini, "Name", desktop->name);
        val = efreet_ini_string_get(ini, "Name");
        if (!val)
            efreet_ini_string_set(ini, "Name", desktop->name);
    }
    if (desktop->generic_name)
    {
        efreet_ini_localestring_set(ini, "GenericName", desktop->generic_name);
        val = efreet_ini_string_get(ini, "GenericName");
        if (!val)
            efreet_ini_string_set(ini, "GenericName", desktop->generic_name);
    }
    if (desktop->comment)
    {
        efreet_ini_localestring_set(ini, "Comment", desktop->comment);
        val = efreet_ini_string_get(ini, "Comment");
        if (!val)
            efreet_ini_string_set(ini, "Comment", desktop->comment);
    }
    if (desktop->icon)
    {
        efreet_ini_localestring_set(ini, "Icon", desktop->icon);
        val = efreet_ini_string_get(ini, "Icon");
        if (!val)
            efreet_ini_string_set(ini, "Icon", desktop->icon);
    }

    efreet_ini_boolean_set(ini, "NoDisplay", desktop->no_display);
    efreet_ini_boolean_set(ini, "Hidden", desktop->hidden);

    if (desktop->x) eina_hash_foreach(desktop->x, efreet_desktop_x_fields_save,
                                        ini);
}

/**
 * @internal
 * @param node: The node to work with
 * @param desktop: The desktop file to work with
 * @return Returns always true, to be used in eina_hash_foreach()
 * @brief Parses out an X- key from @a node and stores in @a desktop
 */
static Eina_Bool
efreet_desktop_x_fields_parse(const Eina_Hash *hash __UNUSED__, const void *key, void *value, void *fdata)
{
    Efreet_Desktop * desktop = fdata;

    if (!desktop) return EINA_TRUE;
    if (strncmp(key, "X-", 2)) return EINA_TRUE;

    if (!desktop->x)
        desktop->x = eina_hash_string_superfast_new(EINA_FREE_CB(eina_stringshare_del));
    eina_hash_del_by_key(desktop->x, key);
    eina_hash_add(desktop->x, key, (void *)eina_stringshare_add(value));

    return EINA_TRUE;
}

/**
 * @internal
 * @param node: The node to work with
 * @param ini: The ini file to work with
 * @return Returns no value
 * @brief Stores an X- key from @a node and stores in @a ini
 */
static Eina_Bool
efreet_desktop_x_fields_save(const Eina_Hash *hash __UNUSED__, const void *key, void *value, void *fdata)
{
    Efreet_Ini *ini = fdata;
    efreet_ini_string_set(ini, key, value);

    return EINA_TRUE;
}


/**
 * @internal
 * @param ini: The Efreet_Ini to parse values from
 * @return 1 if desktop should be included in current environement, 0 otherwise
 * @brief Determines if a desktop should be included in the current environment,
 * based on the values of the OnlyShowIn and NotShowIn fields
 */
static int
efreet_desktop_environment_check(Efreet_Ini *ini)
{
    Eina_List *list;
    int found = 0;
    char *val;

    if (!desktop_environment)
        return 1;

    list = efreet_desktop_string_list_parse(efreet_ini_string_get(ini, "OnlyShowIn"));
    if (list)
    {
        EINA_LIST_FREE(list, val)
        {
            if (!strcmp(val, desktop_environment))
                found = 1;
            eina_stringshare_del(val);
        }

        return found;
    }

    list = efreet_desktop_string_list_parse(efreet_ini_string_get(ini, "NotShowIn"));
    EINA_LIST_FREE(list, val)
    {
        if (!strcmp(val, desktop_environment))
            found = 1;
        eina_stringshare_del(val);
    }

    return !found;
}

/*
 * Needs EAPI because of helper binaries
 */
EAPI Eet_Data_Descriptor *
efreet_desktop_edd_init(void)
{
    Eet_Data_Descriptor *edd;

    Eet_Data_Descriptor_Class eddc;
    if (!eet_eina_file_data_descriptor_class_set(&eddc, sizeof (eddc), "cache", sizeof(Efreet_Desktop))) return NULL;
    edd = eet_data_descriptor_file_new(&eddc);
    if (!edd) return NULL;
    EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Efreet_Desktop, "type", type, EET_T_INT);
    EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Efreet_Desktop, "version", version, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Efreet_Desktop, "orig_path", orig_path, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Efreet_Desktop, "load_time", load_time, EET_T_LONG_LONG);
    EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Efreet_Desktop, "name", name, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Efreet_Desktop, "generic_name", generic_name, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Efreet_Desktop, "comment", comment, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Efreet_Desktop, "icon", icon, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Efreet_Desktop, "try_exec", try_exec, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Efreet_Desktop, "exec", exec, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Efreet_Desktop, "path", path, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Efreet_Desktop, "startup_wm_class", startup_wm_class, EET_T_STRING);
    EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Efreet_Desktop, "url", url, EET_T_STRING);
    eet_data_descriptor_element_add(edd, "only_show_in", EET_T_STRING, EET_G_LIST, offsetof(Efreet_Desktop, only_show_in), 0, NULL, NULL);
    eet_data_descriptor_element_add(edd, "not_show_in", EET_T_STRING, EET_G_LIST, offsetof(Efreet_Desktop, not_show_in), 0, NULL, NULL);
    eet_data_descriptor_element_add(edd, "categories", EET_T_STRING, EET_G_LIST, offsetof(Efreet_Desktop, categories), 0, NULL, NULL);
    eet_data_descriptor_element_add(edd, "mime_types", EET_T_STRING, EET_G_LIST, offsetof(Efreet_Desktop, mime_types), 0, NULL, NULL);
    eet_data_descriptor_element_add(edd, "x", EET_T_STRING, EET_G_HASH, offsetof(Efreet_Desktop, x), 0, NULL, NULL);
    EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Efreet_Desktop, "no_display", no_display, EET_T_UCHAR);
    EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Efreet_Desktop, "hidden", hidden, EET_T_UCHAR);
    EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Efreet_Desktop, "terminal", terminal, EET_T_UCHAR);
    EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Efreet_Desktop, "startup_notify", startup_notify, EET_T_UCHAR);
    return edd;
}

/*
 * Needs EAPI because of helper binaries
 */
EAPI void
efreet_desktop_edd_shutdown(Eet_Data_Descriptor *edd)
{
    eet_data_descriptor_free(edd);
}

static int
efreet_desktop_write_cache_dirs_file(void)
{
    char file[PATH_MAX];
    int fd = -1;
    int cachefd = -1;
    char *dir;
    struct stat st;
    struct flock fl;

    if (!efreet_desktop_dirs) return 1;

    snprintf(file, sizeof(file), "%s/.efreet/desktop_data.lock", efreet_home_dir_get());
    fd = open(file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) return 0;
    /* TODO: Retry update cache later */
    memset(&fl, 0, sizeof(struct flock));
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    if (fcntl(fd, F_SETLK, &fl) < 0) goto error;

    cachefd = open(efreet_desktop_cache_dirs(), O_CREAT | O_APPEND | O_RDWR, S_IRUSR | S_IWUSR);
    if (cachefd < 0) goto error;
    if (fstat(cachefd, &st) < 0) goto error;
    if (st.st_size > 0)
    {
        Eina_List *l, *ln;
        char *p;
        char *map;

        map = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, cachefd, 0);
        if (map == MAP_FAILED) goto error;
        p = map;
        while (p < map + st.st_size)
        {
            unsigned int size = *(unsigned int *)p;
            p += sizeof(unsigned int);
            EINA_LIST_FOREACH_SAFE(efreet_desktop_dirs, l, ln, dir)
            {
                if (!strcmp(dir, p))
                {
                    efreet_desktop_dirs = eina_list_remove_list(efreet_desktop_dirs, l);
                    eina_stringshare_del(dir);
                    break;
                }
            }
            p += size;
        }
        munmap(map, st.st_size);
    }
    EINA_LIST_FREE(efreet_desktop_dirs, dir)
    {
        unsigned int size = strlen(dir) + 1;
        size_t count;

        count = write(cachefd, &size, sizeof(int));
        count += write(cachefd, dir, size);

        if (count != sizeof(int) + size)
            DBG("Didn't write all data on cachefd");

        efreet_desktop_changes_monitor_add(dir);
        eina_stringshare_del(dir);
    }
    efreet_desktop_dirs = NULL;
    if (fd >= 0) close(fd);
    if (cachefd >= 0) close(cachefd);
    return 1;

error:
    if (fd >= 0) close(fd);
    if (cachefd >= 0) close(cachefd);
    return 0;
}

static void
efreet_desktop_cache_update_cb(void *data __UNUSED__, Ecore_File_Monitor *em __UNUSED__,
                               Ecore_File_Event event, const char *path)
{
    Eet_File *tmp;
    Efreet_Event_Cache_Update *ev = NULL;
    Efreet_Old_Cache *d = NULL;

    if (strcmp(path, efreet_desktop_cache_file())) return;
    if (event != ECORE_FILE_EVENT_CREATED_FILE &&
        event != ECORE_FILE_EVENT_MODIFIED) return;

    tmp = eet_open(efreet_desktop_cache_file(), EET_FILE_MODE_READ);
    if (!tmp) return;
    ev = NEW(Efreet_Event_Cache_Update, 1);
    if (!ev) goto error;
    d = NEW(Efreet_Old_Cache, 1);
    if (!d) goto error;

    d->desktop_cache = efreet_desktop_cache;
    d->cache = cache;
    old_caches = eina_list_append(old_caches, d);

    efreet_desktop_cache = eina_hash_string_superfast_new(NULL);
    cache = tmp;

    efreet_util_desktop_cache_reload();
    ecore_event_add(EFREET_EVENT_DESKTOP_CACHE_UPDATE, ev, efreet_desktop_cache_update_free, d);
    return;
error:
    IF_FREE(ev);
    IF_FREE(d);
    eet_close(tmp);
}

static void
efreet_desktop_cache_update_free(void *data, void *ev)
{
   Efreet_Old_Cache *d;
   int dangling = 0;

   d = data;
   /*
    * All users should now had the chance to update their pointers, so we can now
    * free the old cache
    */
   if (d->desktop_cache)
     {
        Eina_Iterator *it;
        Eina_Hash_Tuple *tuple;

        it = eina_hash_iterator_tuple_new(d->desktop_cache);
        EINA_ITERATOR_FOREACH(it, tuple)
          {
             printf("Efreet: %d:%s still in cache on cache close!\n",
                    ((Efreet_Desktop *)tuple->data)->ref, (char *)tuple->key);
             dangling++;
          }
        eina_iterator_free(it);

        eina_hash_free(d->desktop_cache);
     }
   /*
    * If there are dangling references the eet file won't be closed - to
    * avoid crashes, but this will leak instead.
    */
   if (dangling == 0)
     {
        if (d->cache) eet_close(d->cache);
     }
   else
     {
        printf("Efreet: ERROR. There are still %i desktop files with old\n"
               "dangling references to desktop files. This application\n"
               "has not handled the EFREET_EVENT_DESKTOP_CACHE_UPDATE\n"
               "fully and released its references. Please fix the application\n"
               "so it does this.\n",
               dangling);
     }
   old_caches = eina_list_remove(old_caches, d);
   free(d);
   free(ev);
}

static void
efreet_desktop_update_cache(void)
{
    if (!efreet_cache_update) return;

    /* TODO: Make sure we don't create a lot of execs, maybe use a timer? */
    if (efreet_desktop_job) ecore_job_del(efreet_desktop_job);
    efreet_desktop_job = ecore_job_add(efreet_desktop_update_cache_job, NULL);
}

static void
efreet_desktop_update_cache_job(void *data __UNUSED__)
{
    char file[PATH_MAX];
    struct flock fl;

    efreet_desktop_job = NULL;

    /* TODO: Retry update cache later */
    if (efreet_desktop_exe_lock > 0) return;

    if (!efreet_desktop_write_cache_dirs_file()) return;

    snprintf(file, sizeof(file), "%s/.efreet/desktop_exec.lock", efreet_home_dir_get());

    efreet_desktop_exe_lock = open(file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (efreet_desktop_exe_lock < 0) return;
    memset(&fl, 0, sizeof(struct flock));
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    if (fcntl(efreet_desktop_exe_lock, F_SETLK, &fl) < 0) goto error;
    efreet_desktop_exe = ecore_exe_run(PACKAGE_LIB_DIR "/efreet/efreet_desktop_cache_create", NULL);
    if (!efreet_desktop_exe) goto error;

    return;

error:
    if (efreet_desktop_exe_lock > 0)
    {
        close(efreet_desktop_exe_lock);
        efreet_desktop_exe_lock = -1;
    }
}

static Eina_Bool
efreet_desktop_exe_cb(void *data __UNUSED__, int type __UNUSED__, void *event)
{
    Ecore_Exe_Event_Del *ev;

    ev = event;
    if (ev->exe != efreet_desktop_exe) return ECORE_CALLBACK_RENEW;
    if (efreet_desktop_exe_lock > 0)
    {
        close(efreet_desktop_exe_lock);
        efreet_desktop_exe_lock = -1;
    }
    return ECORE_CALLBACK_RENEW;
}

static void
efreet_desktop_changes_listen(void)
{
    int dirsfd = -1;
    Eina_List *dirs;
    char *path;
    struct stat st;

    change_monitors = eina_hash_string_superfast_new(EINA_FREE_CB(ecore_file_monitor_del));
    if (!change_monitors) return;

    dirs = efreet_default_dirs_get(efreet_data_home_get(),
                                   efreet_data_dirs_get(), "applications");

    EINA_LIST_FREE(dirs, path)
    {
        efreet_desktop_changes_listen_recursive(path);
        eina_stringshare_del(path);
    }

    dirsfd = open(efreet_desktop_cache_dirs(), O_RDONLY, S_IRUSR | S_IWUSR);
    if (dirsfd >= 0)
    {
        if ((fstat(dirsfd, &st) == 0) && (st.st_size > 0))
        {
            char *p;
            char *map;

            map = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, dirsfd, 0);
            if (map == MAP_FAILED) goto error;
            p = map;
            while (p < map + st.st_size)
            {
                unsigned int size = *(unsigned int *)p;
                p += sizeof(unsigned int);
                efreet_desktop_changes_monitor_add(p);
                p += size;
            }
            munmap(map, st.st_size);
        }
        close(dirsfd);
    }

    return;
error:
    if (dirsfd >= 0) close(dirsfd);
}

static void
efreet_desktop_changes_listen_recursive(const char *path)
{
    char buf[PATH_MAX];
    DIR *files;
    struct dirent *file;

    efreet_desktop_changes_monitor_add(path);

    files = opendir(path);
    if (!files) return;
    while ((file = readdir(files)))
    {
        if (!file) break;
        if (!strcmp(file->d_name, ".") || !strcmp(file->d_name, "..")) continue;

        snprintf(buf, sizeof(buf), "%s/%s", path, file->d_name);
        if (ecore_file_is_dir(buf)) efreet_desktop_changes_listen_recursive(buf);
    }
    closedir(files);
}

static void
efreet_desktop_changes_monitor_add(const char *path)
{
    char rp[PATH_MAX];

    if (!realpath(path, rp)) return;
    if (eina_hash_find(change_monitors, rp)) return;
    eina_hash_add(change_monitors, rp,
                  ecore_file_monitor_add(rp,
                                         efreet_desktop_changes_cb,
                                         NULL));
}

static void
efreet_desktop_changes_cb(void *data __UNUSED__, Ecore_File_Monitor *em __UNUSED__,
                                 Ecore_File_Event event, const char *path)
{
    switch (event)
    {
        case ECORE_FILE_EVENT_NONE:
            /* noop */
            break;

        case ECORE_FILE_EVENT_CREATED_FILE:
        case ECORE_FILE_EVENT_DELETED_FILE:
        case ECORE_FILE_EVENT_MODIFIED:
            efreet_desktop_update_cache();
            break;

        case ECORE_FILE_EVENT_DELETED_SELF:
        case ECORE_FILE_EVENT_DELETED_DIRECTORY:
            eina_hash_del_by_key(change_monitors, path);
            efreet_desktop_update_cache();
            break;

        case ECORE_FILE_EVENT_CREATED_DIRECTORY:
            efreet_desktop_changes_monitor_add(path);
            efreet_desktop_update_cache();
            break;
    }
}
