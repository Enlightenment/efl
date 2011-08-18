#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* TODO: add no_display check, as we might want only displayable items */

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

#include <fnmatch.h>

#include <Ecore_File.h>

/* define macros and variable for using the eina logging system  */
#define EFREET_MODULE_LOG_DOM _efreet_utils_log_dom
static int _efreet_utils_log_dom = -1;

#include "Efreet.h"
#include "efreet_private.h"

static char *efreet_util_path_in_default(const char *section, const char *path);

static int  efreet_util_glob_match(const char *str, const char *glob);

static Eina_List *efreet_util_menus_find_helper(Eina_List *menus, const char *config_dir);

static Efreet_Desktop *efreet_util_cache_find(const char *search, const char *what1, const char *what2);
static Eina_List *efreet_util_cache_list(const char *search, const char *what);
static Eina_List *efreet_util_cache_glob_list(const char *search, const char *what);

static Eina_Hash *file_id_by_desktop_path = NULL;

static int init = 0;

int
efreet_util_init(void)
{
    if (init++) return init;
    _efreet_utils_log_dom = eina_log_domain_register
      ("efreet_util", EFREET_DEFAULT_LOG_COLOR);
    if (_efreet_utils_log_dom < 0)
    {
        EINA_LOG_ERR("Efreet: Could not create a log domain for efreet_util");
        return 0;
    }

    file_id_by_desktop_path = eina_hash_string_superfast_new(EINA_FREE_CB(eina_stringshare_del));

    return init;
}

int
efreet_util_shutdown(void)
{
    if (--init) return init;

    eina_log_domain_unregister(_efreet_utils_log_dom);
    _efreet_utils_log_dom = -1;
    IF_FREE_HASH(file_id_by_desktop_path);

    return init;
}

static char *
efreet_util_path_in_default(const char *section, const char *path)
{
    Eina_List *dirs;
    char *ret = NULL;
    char *dir;

    dirs = efreet_default_dirs_get(efreet_data_home_get(), efreet_data_dirs_get(),
                                   section);

    EINA_LIST_FREE(dirs, dir)
    {
        if (!strncmp(path, dir, strlen(dir)))
            ret = dir;
        else
            eina_stringshare_del(dir);
    }

    return ret;
}

EAPI const char *
efreet_util_path_to_file_id(const char *path)
{
    size_t len, len2;
    char *tmp, *p;
    char *base;
    const char *file_id;

    if (!path) return NULL;
    file_id = eina_hash_find(file_id_by_desktop_path, path);
    if (file_id) return file_id;

    base = efreet_util_path_in_default("applications", path);
    if (!base) return NULL;

    len = strlen(base);
    if (strlen(path) <= len)
    {
        eina_stringshare_del(base);
        return NULL;
    }
    if (strncmp(path, base, len))
    {
        eina_stringshare_del(base);
        return NULL;
    }

    len2 = strlen(path + len + 1) + 1;
    tmp = alloca(len2);
    memcpy(tmp, path + len + 1, len2);
    p = tmp;
    while (*p)
    {
        if (*p == '/') *p = '-';
        p++;
    }
    eina_stringshare_del(base);
    file_id = eina_stringshare_add(tmp);
    eina_hash_add(file_id_by_desktop_path, path, (void *)file_id);
    return file_id;
}

EAPI Eina_List *
efreet_util_desktop_mime_list(const char *mime)
{
    return efreet_util_cache_list("mime_types", mime);
}

EAPI Efreet_Desktop *
efreet_util_desktop_wm_class_find(const char *wmname, const char *wmclass)
{
    return efreet_util_cache_find("startup_wm_class", wmname, wmclass);
}

EAPI Efreet_Desktop *
efreet_util_desktop_file_id_find(const char *file_id)
{
    Efreet_Cache_Hash *hash;
    Efreet_Desktop *ret = NULL;
    const char *str;

    if (!file_id) return NULL;

    hash = efreet_cache_util_hash_string("file_id");
    if (!hash) return NULL;
    str = eina_hash_find(hash->hash, file_id);
    if (str)
        ret = efreet_desktop_get(str);
    return ret;
}

EAPI Efreet_Desktop *
efreet_util_desktop_exec_find(const char *exec)
{
    Efreet_Cache_Hash *hash = NULL;
    Efreet_Desktop *ret = NULL;
    Efreet_Cache_Array_String *names = NULL;
    unsigned int i;

    if (!exec) return NULL;

    names = efreet_cache_util_names("exec_list");
    if (!names) return NULL;
    for (i = 0; i < names->array_count; i++)
    {
        const char *file;
        char *exe;
        unsigned int j;
        Efreet_Cache_Array_String *array;

        exe = ecore_file_app_exe_get(names->array[i]);
        if (!exe) continue;
        file = ecore_file_file_get(exe);
        if (!file) continue;
        if (strcmp(exec, exe) && strcmp(exec, file))
        {
            free(exe);
            continue;
        }
        free(exe);

        if (!hash)
            hash = efreet_cache_util_hash_array_string("exec_hash");
        if (!hash) return NULL;
        array = eina_hash_find(hash->hash, names->array[i]);
        if (!array) continue;
        for (j = 0; j < array->array_count; j++)
        {
            ret = efreet_desktop_get(array->array[j]);
            if (ret) break;
        }
        if (ret) break;
    }
    return ret;
}

EAPI Efreet_Desktop *
efreet_util_desktop_name_find(const char *name)
{
    return efreet_util_cache_find("name", name, NULL);
}

EAPI Efreet_Desktop *
efreet_util_desktop_generic_name_find(const char *generic_name)
{
    return efreet_util_cache_find("generic_name", generic_name, NULL);
}

EAPI Eina_List *
efreet_util_desktop_name_glob_list(const char *glob)
{
    return efreet_util_cache_glob_list("name", glob);
}

EAPI Eina_List *
efreet_util_desktop_exec_glob_list(const char *glob)
{
    Efreet_Cache_Hash *hash = NULL;
    Eina_List *ret = NULL;
    Efreet_Cache_Array_String *names = NULL;
    unsigned int i;

    if (!glob) return NULL;
    if (!strcmp(glob, "*"))
        glob = NULL;

    names = efreet_cache_util_names("exec_list");
    if (!names) return NULL;
    for (i = 0; i < names->array_count; i++)
    {
        Efreet_Cache_Array_String *array;
        unsigned int j;
        char *exe;
        Efreet_Desktop *desk;

        exe = ecore_file_app_exe_get(names->array[i]);
        if (!exe) continue;
        if (glob && !efreet_util_glob_match(exe, glob))
        {
            free(exe);
            continue;
        }
        free(exe);

        if (!hash)
            hash = efreet_cache_util_hash_array_string("exec_hash");
        if (!hash) return NULL;

        array = eina_hash_find(hash->hash, names->array[i]);
        if (!array) continue;
        for (j = 0; j < array->array_count; j++)
        {
            desk = efreet_desktop_get(array->array[j]);
            if (desk)
                ret = eina_list_append(ret, desk);
        }
    }
    return ret;
}

EAPI Eina_List *
efreet_util_desktop_generic_name_glob_list(const char *glob)
{
    return efreet_util_cache_glob_list("generic_name", glob);
}

EAPI Eina_List *
efreet_util_desktop_comment_glob_list(const char *glob)
{
    return efreet_util_cache_glob_list("comment", glob);
}

EAPI Eina_List *
efreet_util_desktop_categories_list(void)
{
    Efreet_Cache_Array_String *array;
    Eina_List *ret = NULL;
    unsigned int i;

    array = efreet_cache_util_names("categories_list");
    if (!array) return NULL;
    for (i = 0; i < array->array_count; i++)
        ret = eina_list_append(ret, array->array[i]);
    return ret;
}

EAPI Eina_List *
efreet_util_desktop_category_list(const char *category)
{
    return efreet_util_cache_list("categories", category);
}

static int
efreet_util_glob_match(const char *str, const char *glob)
{
    if (!str || !glob)
        return 0;
    if (glob[0] == '\0')
    {
        if (str[0] == '\0') return 1;
        return 0;
    }
    if (!strcmp(glob, "*")) return 1;
    if (!fnmatch(glob, str, 0)) return 1;
    return 0;
}

EAPI Eina_List *
efreet_util_menus_find(void)
{
    Eina_List *menus = NULL;
    Eina_List *dirs, *l;
    const char *dir;

    menus = efreet_util_menus_find_helper(menus, efreet_config_home_get());

    dirs = efreet_config_dirs_get();
    EINA_LIST_FOREACH(dirs, l, dir)
        menus = efreet_util_menus_find_helper(menus, dir);

    return menus;
}

static Eina_List *
efreet_util_menus_find_helper(Eina_List *menus, const char *config_dir)
{
    DIR *files = NULL;
    struct dirent *file = NULL;
    char dbuf[PATH_MAX], fbuf[PATH_MAX];

    snprintf(dbuf, sizeof(dbuf), "%s/menus", config_dir);
    files = opendir(dbuf);
    if (!files) return menus;
    while ((file = readdir(files))) {
        const char *exten;
        exten = strrchr(file->d_name, '.');
        if (!exten) continue;
        if (strcmp(".menu", exten)) continue;

        snprintf(fbuf, sizeof(fbuf), "%s/%s", dbuf, file->d_name);
        if (ecore_file_is_dir(fbuf)) continue;

        menus = eina_list_append(menus, strdup(fbuf));
    }
    closedir(files);
    return menus;
}

static Efreet_Desktop *
efreet_util_cache_find(const char *search, const char *what1, const char *what2)
{
    Efreet_Cache_Hash *hash;
    Efreet_Desktop *ret = NULL;
    Efreet_Cache_Array_String *array = NULL;
    char key[256];

    if ((!what1) && (!what2)) return NULL;

    snprintf(key, sizeof(key), "%s_hash", search);
    hash = efreet_cache_util_hash_array_string(key);
    if (!hash) return NULL;
    if (what1)
        array = eina_hash_find(hash->hash, what1);
    if (!array && what2) array = eina_hash_find(hash->hash, what2);
    if (array)
    {
        unsigned int i;

        for (i = 0; i < array->array_count; i++)
        {
            ret = efreet_desktop_get(array->array[i]);
            if (ret) break;
        }
    }
    return ret;
}

static Eina_List *
efreet_util_cache_list(const char *search, const char *what)
{
    Efreet_Cache_Hash *hash;
    Efreet_Cache_Array_String *array;
    Eina_List *ret = NULL;
    char key[256];

    if (!what) return NULL;

    snprintf(key, sizeof(key), "%s_hash", search);
    hash = efreet_cache_util_hash_array_string(key);
    if (!hash) return NULL;
    array = eina_hash_find(hash->hash, what);
    if (array)
    {
        unsigned int i;
        Efreet_Desktop *desk;

        for (i = 0; i < array->array_count; i++)
        {
            desk = efreet_desktop_get(array->array[i]);
            if (desk)
                ret = eina_list_append(ret, desk);
        }
    }
    return ret;
}

static Eina_List *
efreet_util_cache_glob_list(const char *search, const char *what)
{
    Efreet_Cache_Hash *hash = NULL;
    Eina_List *ret = NULL;
    Efreet_Cache_Array_String *names = NULL;
    char key[256];
    unsigned int i;

    if (!what) return NULL;
    if (!strcmp(what, "*"))
        what = NULL;

    snprintf(key, sizeof(key), "%s_list", search);
    names = efreet_cache_util_names(key);
    if (!names) return NULL;
    for (i = 0; i < names->array_count; i++)
    {
        Efreet_Cache_Array_String *array;
        unsigned int j;
        Efreet_Desktop *desk;

        if (what && !efreet_util_glob_match(names->array[i], what)) continue;

        if (!hash)
        {
            snprintf(key, sizeof(key), "%s_hash", search);
            hash = efreet_cache_util_hash_array_string(key);
        }
        if (!hash) return NULL;

        array = eina_hash_find(hash->hash, names->array[i]);
        if (!array) continue;
        for (j = 0; j < array->array_count; j++)
        {
            desk = efreet_desktop_get(array->array[j]);
            if (desk)
                ret = eina_list_append(ret, desk);
        }
    }
    return ret;
}

/*
 * Needs EAPI because of helper binaries
 */
EAPI void
efreet_hash_free(Eina_Hash *hash, Eina_Free_Cb free_cb)
{
    eina_hash_free_cb_set(hash, free_cb);
    eina_hash_free(hash);
}

