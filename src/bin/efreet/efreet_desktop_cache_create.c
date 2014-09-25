#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/time.h>
#include <sys/resource.h>
#endif

#include <Eina.h>
#include <Eet.h>
#include <Ecore.h>
#include <Ecore_File.h>

#define EFREET_MODULE_LOG_DOM _efreet_desktop_cache_log_dom
static int _efreet_desktop_cache_log_dom = -1;

#include "Efreet.h"
#include "efreet_private.h"
#include "efreet_cache_private.h"

static Eet_Data_Descriptor *edd = NULL;
static Eet_File *ef = NULL;
static Eet_File *util_ef = NULL;

static Eina_Hash *desktops = NULL;

static Eina_Hash         *file_ids = NULL;
static Efreet_Cache_Hash *old_file_ids = NULL;
static Eina_Hash         *paths = NULL;

static Eina_Hash *mime_types = NULL;
static Eina_Hash *categories = NULL;
static Eina_Hash *startup_wm_class = NULL;
static Eina_Hash *name = NULL;
static Eina_Hash *generic_name = NULL;
static Eina_Hash *comment = NULL;
static Eina_Hash *exec = NULL;
static Eina_Hash *environments = NULL;
static Eina_Hash *keywords = NULL;

static int
cache_add(const char *path, const char *file_id, int priority EINA_UNUSED, int *changed)
{
    Efreet_Desktop *desk;
    char *ext;

    INF("FOUND: %s", path);
    if (file_id) INF(" (id): %s", file_id);
    ext = strrchr(path, '.');
    if (!ext || (strcmp(ext, ".desktop") && strcmp(ext, ".directory"))) return 1;
    desk = efreet_desktop_new(path);
    if (desk) INF("  OK");
    else      INF("  FAIL");
    if (!desk) return 1;
    if (!desk->eet)
    {
        /* This file isn't in cache */
        *changed = 1;
        INF("  NEW");
    }
    else if (ecore_file_mod_time(desk->orig_path) != desk->load_time)
    {
        efreet_desktop_free(desk);
        *changed = 1;
        desk = efreet_desktop_uncached_new(path);
        if (desk) INF("  CHANGED");
        else      INF("  NO UNCACHED");
    }
    if (!desk) return 1;
    if (file_id && old_file_ids && !eina_hash_find(old_file_ids->hash, file_id))
    {
        *changed = 1;
        INF("  NOT IN UTILS");
    }
    if (!eina_hash_find(paths, desk->orig_path))
    {
        if (!eet_data_write(ef, edd, desk->orig_path, desk, 0))
            return 0;
        eina_hash_add(paths, desk->orig_path, (void *)1);
    }
    /* TODO: We should check priority, and not just hope we search in right order */
    /* TODO: We need to find out if prioritized file id has changed because of
     * changed search order. */
    if (!desk->hidden && desk->type == EFREET_DESKTOP_TYPE_APPLICATION &&
        file_id && !eina_hash_find(file_ids, file_id))
    {
        Eina_List *l;
        char *data;
        Efreet_Cache_Array_String *array;

#define ADD_LIST(list, hash) \
        EINA_LIST_FOREACH((list), l, data) \
        { \
            array = eina_hash_find((hash), data); \
            if (!array) \
                array = NEW(Efreet_Cache_Array_String, 1); \
            array->array = realloc(array->array, sizeof (char *) * (array->array_count + 1)); \
            array->array[array->array_count++] = desk->orig_path; \
            eina_hash_set((hash), data, array); \
        }
#define ADD_ELEM(elem, hash) \
        if ((elem)) \
        { \
            data = (elem); \
            array = eina_hash_find((hash), data); \
            if (!array) \
                array = NEW(Efreet_Cache_Array_String, 1); \
            array->array = realloc(array->array, sizeof (char *) * (array->array_count + 1)); \
            array->array[array->array_count++] = desk->orig_path; \
            eina_hash_set((hash), data, array); \
        }
        /* Desktop Spec 1.0 */
        ADD_LIST(desk->mime_types, mime_types);
        ADD_LIST(desk->categories, categories);
        ADD_ELEM(desk->startup_wm_class, startup_wm_class);
        ADD_ELEM(desk->name, name);
        ADD_ELEM(desk->generic_name, generic_name);
        ADD_ELEM(desk->comment, comment);
        ADD_ELEM(desk->exec, exec);
        ADD_LIST(desk->only_show_in, environments);
        ADD_LIST(desk->not_show_in, environments);
        eina_hash_add(file_ids, file_id, desk->orig_path);
        eina_hash_add(desktops, desk->orig_path, desk);
        /* Desktop Spec 1.1 */
        ADD_LIST(desk->keywords, keywords);
    }
    else
        efreet_desktop_free(desk);
    return 1;
}

static int
stat_cmp(const void *a, const void *b)
{
   const struct stat *st1 = a;
   const struct stat *st2 = b;

   if ((st2->st_dev == st1->st_dev) && (st2->st_ino == st1->st_ino))
     return 0;
   return 1;
}

static int
cache_scan(Eina_Inarray *stack, const char *path, const char *base_id,
           int priority, int recurse, int *changed)
{
    char *file_id = NULL;
    char id[PATH_MAX];
    Eina_Iterator *it;
    Eina_File_Direct_Info *info;
    struct stat st;
    int ret = 1;

    if (!ecore_file_is_dir(path)) return 1;
    if (stat(path, &st) == -1) return 1;
    if (eina_inarray_search(stack, &st, stat_cmp) >= 0) return 1;
    eina_inarray_push(stack, &st);

    it = eina_file_stat_ls(path);
    if (!it) goto end;
    id[0] = '\0';
    EINA_ITERATOR_FOREACH(it, info)
    {
        const char *fname = info->path + info->name_start;

        if (info->path[info->name_start] == '.') continue;
        if (base_id)
        {
            if (*base_id) snprintf(id, sizeof(id), "%s-%s", base_id, fname);
            else
            {
                strncpy(id, fname, PATH_MAX);
                id[PATH_MAX - 1] = 0;
            }
            file_id = id;
        }

        if (((info->type == EINA_FILE_LNK) && (ecore_file_is_dir(info->path))) ||
            (info->type == EINA_FILE_DIR))
        {
           if (recurse)
             {
                ret = cache_scan(stack, info->path, file_id, priority, recurse, changed);
                if (!ret) break;
             }
        }
        else
        {
           ret = cache_add(info->path, file_id, priority, changed);
           if (!ret) break;
        }
    }
    eina_iterator_free(it);
end:
    eina_inarray_pop(stack);
    return ret;
}

static int
cache_lock_file(void)
{
    char file[PATH_MAX];
    struct flock fl;
    int lockfd;

    snprintf(file, sizeof(file), "%s/efreet/desktop_data.lock", efreet_cache_home_get());
    lockfd = open(file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (lockfd < 0) return -1;
    efreet_fsetowner(lockfd);

    memset(&fl, 0, sizeof(struct flock));
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    if (fcntl(lockfd, F_SETLK, &fl) < 0)
    {
        INF("LOCKED! You may want to delete %s if this persists", file);
        close(lockfd);
        return -1;
    }

    return lockfd;
}

/* check if old and new caches contain the same number of entries,
 * and are the same version */
static int
check_changed(const char *old_name, Eet_File *new_ef,
              unsigned char major, unsigned char minor)
{
   Eet_File *old_ef;
   Efreet_Cache_Version *old_version = NULL;

   old_ef = eet_open(old_name, EET_FILE_MODE_READ);
   if (!old_ef) return 1;
   if (eet_num_entries(old_ef) != eet_num_entries(new_ef))
     goto changed;

   old_version = eet_data_read(old_ef, efreet_version_edd(), EFREET_CACHE_VERSION);
   if (!old_version)
     goto changed;

   if ((old_version->major != major) ||
       (old_version->minor != minor))
     goto changed;

   free(old_version);
   eet_close(old_ef);
   return 0;

changed:
   free(old_version);
   eet_close(old_ef);
   return 1;
}

int
main(int argc, char **argv)
{
    /* TODO:
     * - Add file monitor on files, so that we catch changes on files
     *   during whilst this program runs.
     * - Maybe linger for a while to reduce number of cache re-creates.
     */
    Efreet_Cache_Hash hash;
    Efreet_Cache_Version version;
    Eina_List *dirs = NULL;
    Eina_List *systemdirs = NULL;
    Eina_List *extra_dirs = NULL;
    Eina_List *l = NULL;
    Eina_Inarray *stack = NULL;
    int priority = 0;
    char *dir = NULL;
    char *path;
    int lockfd = -1, tmpfd;
    int changed = 0;
    int i;
    char file[PATH_MAX] = { '\0' };
    char util_file[PATH_MAX] = { '\0' };
    mode_t um;

    if (!eina_init()) goto eina_error;
    _efreet_desktop_cache_log_dom =
        eina_log_domain_register("efreet_desktop_cache", EFREET_DEFAULT_LOG_COLOR);
    if (_efreet_desktop_cache_log_dom < 0)
    {
        EINA_LOG_ERR("Efreet: Could not create a log domain for efreet_desktop_cache.");
        return -1;
    }

    for (i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-v"))
            eina_log_domain_level_set("efreet_desktop_cache", EINA_LOG_LEVEL_DBG);
        else if ((!strcmp(argv[i], "-h")) ||
                 (!strcmp(argv[i], "-help")) ||
                 (!strcmp(argv[i], "--h")) ||
                 (!strcmp(argv[i], "--help")))
        {
            printf("Options:\n");
            printf("  -v              Verbose mode\n");
            printf("  -d dir1 dir2    Extra dirs\n");
            exit(0);
        }
        else if (!strcmp(argv[i], "-d"))
        {
            while ((i < (argc - 1)) && (argv[(i + 1)][0] != '-'))
                extra_dirs = eina_list_append(extra_dirs, argv[++i]);
        }
    }
    extra_dirs = eina_list_sort(extra_dirs, -1, EINA_COMPARE_CB(strcmp));

#ifdef HAVE_SYS_RESOURCE_H
    setpriority(PRIO_PROCESS, 0, 19);
#elif _WIN32
    SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
#endif

    /* init external subsystems */
    if (!eet_init()) goto eet_error;
    if (!ecore_init()) goto ecore_error;

    efreet_cache_update = 0;
    /* finish efreet init */
    if (!efreet_init()) goto efreet_error;

    /* create homedir */
    snprintf(file, sizeof(file), "%s/efreet", efreet_cache_home_get());
    if (!ecore_file_exists(file))
    {
        if (!ecore_file_mkpath(file)) goto efreet_error;
        efreet_setowner(file);
    }

    /* lock process, so that we only run one copy of this program */
    lockfd = cache_lock_file();
    if (lockfd == -1) goto efreet_error;

    edd = efreet_desktop_edd();
    if (!edd) goto edd_error;

    /* read user dirs from old cache */
    ef = eet_open(efreet_desktop_util_cache_file(), EET_FILE_MODE_READ);
    if (ef)
    {
        old_file_ids = eet_data_read(ef, efreet_hash_string_edd(), "file_id");
        eet_close(ef);
    }

    /* create cache */
    snprintf(file, sizeof(file), "%s.XXXXXX", efreet_desktop_cache_file());
    /* set secure umask for temporary files */
    um = umask(0077);
    tmpfd = mkstemp(file);
    umask(um);
    if (tmpfd < 0) goto error;
    close(tmpfd);
    ef = eet_open(file, EET_FILE_MODE_READ_WRITE);
    if (!ef) goto error;

    snprintf(util_file, sizeof(util_file), "%s.XXXXXX", efreet_desktop_util_cache_file());
    /* set secure umask for temporary files */
    um = umask(0077);
    tmpfd = mkstemp(util_file);
    umask(um);
    if (tmpfd < 0) goto error;
    close(tmpfd);
    util_ef = eet_open(util_file, EET_FILE_MODE_READ_WRITE);
    if (!util_ef) goto error;

    /* write cache version */
    version.major = EFREET_DESKTOP_UTILS_CACHE_MAJOR;
    version.minor = EFREET_DESKTOP_UTILS_CACHE_MINOR;
    eet_data_write(util_ef, efreet_version_edd(), EFREET_CACHE_VERSION, &version, 1);
    version.major = EFREET_DESKTOP_CACHE_MAJOR;
    version.minor = EFREET_DESKTOP_CACHE_MINOR;
    eet_data_write(ef, efreet_version_edd(), EFREET_CACHE_VERSION, &version, 1);

    desktops = eina_hash_string_superfast_new(EINA_FREE_CB(efreet_desktop_free));

    file_ids = eina_hash_string_superfast_new(NULL);
    paths = eina_hash_string_superfast_new(NULL);

    mime_types = eina_hash_string_superfast_new(EINA_FREE_CB(efreet_cache_array_string_free));
    categories = eina_hash_string_superfast_new(EINA_FREE_CB(efreet_cache_array_string_free));
    startup_wm_class = eina_hash_string_superfast_new(EINA_FREE_CB(efreet_cache_array_string_free));
    name = eina_hash_string_superfast_new(EINA_FREE_CB(efreet_cache_array_string_free));
    generic_name = eina_hash_string_superfast_new(EINA_FREE_CB(efreet_cache_array_string_free));
    comment = eina_hash_string_superfast_new(EINA_FREE_CB(efreet_cache_array_string_free));
    exec = eina_hash_string_superfast_new(EINA_FREE_CB(efreet_cache_array_string_free));
    environments = eina_hash_string_superfast_new(EINA_FREE_CB(efreet_cache_array_string_free));
    keywords = eina_hash_string_superfast_new(EINA_FREE_CB(efreet_cache_array_string_free));

    dirs = efreet_default_dirs_get(efreet_data_home_get(), efreet_data_dirs_get(),
                                                                    "applications");
    if (!dirs) goto error;
    stack = eina_inarray_new(sizeof(struct stat), 16);
    if (!stack) goto error;

    EINA_LIST_FREE(dirs, path)
     {
        char file_id[PATH_MAX] = { '\0' };

        eina_inarray_flush(stack);
        if (!cache_scan(stack, path, file_id, priority++, 1, &changed))
          goto error;
        systemdirs = eina_list_append(systemdirs, path);
     }

    EINA_LIST_FOREACH(extra_dirs, l, path)
    {
        eina_inarray_flush(stack);
        if (!cache_scan(stack, path, NULL, priority, 0, &changed)) goto error;
    }

    /* store util */
#define STORE_HASH_ARRAY(_hash) \
    if (eina_hash_population((_hash)) > 0) \
    { \
        Eina_Iterator *it;   \
        Efreet_Cache_Array_String array; \
        const char *str;     \
                             \
        hash.hash = (_hash); \
        eet_data_write(util_ef, efreet_hash_array_string_edd(), #_hash "_hash", &hash, 1); \
        array.array_count = 0; \
        array.array = malloc(eina_hash_population(hash.hash) * sizeof(char *)); \
        it = eina_hash_iterator_key_new(hash.hash); \
        EINA_ITERATOR_FOREACH(it, str) \
            array.array[array.array_count++] = str; \
        eina_iterator_free(it); \
        eet_data_write(util_ef, efreet_array_string_edd(), #_hash "_list", &array, 1); \
        free(array.array); \
    }
    STORE_HASH_ARRAY(mime_types);
    STORE_HASH_ARRAY(categories);
    STORE_HASH_ARRAY(startup_wm_class);
    STORE_HASH_ARRAY(name);
    STORE_HASH_ARRAY(generic_name);
    STORE_HASH_ARRAY(comment);
    STORE_HASH_ARRAY(exec);
    STORE_HASH_ARRAY(environments);
    STORE_HASH_ARRAY(keywords);
    if (eina_hash_population(file_ids) > 0)
    {
        hash.hash = file_ids;
        eet_data_write(util_ef, efreet_hash_string_edd(), "file_id", &hash, 1);
    }

    eina_hash_free(mime_types);
    eina_hash_free(categories);
    eina_hash_free(startup_wm_class);
    eina_hash_free(name);
    eina_hash_free(generic_name);
    eina_hash_free(comment);
    eina_hash_free(exec);
    eina_hash_free(environments);
    eina_hash_free(keywords);

    if (old_file_ids)
    {
        eina_hash_free(old_file_ids->hash);
        free(old_file_ids);
    }

    eina_hash_free(file_ids);
    eina_hash_free(paths);

    eina_hash_free(desktops);

    /* check if old and new caches are equal */
    if (!changed)
    {
        if (check_changed(efreet_desktop_cache_file(), ef,
                          EFREET_DESKTOP_CACHE_MAJOR, EFREET_DESKTOP_CACHE_MINOR))
            changed = 1;
    }
    if (!changed)
    {
        if (check_changed(efreet_desktop_util_cache_file(), util_ef,
                          EFREET_DESKTOP_UTILS_CACHE_MAJOR, EFREET_DESKTOP_UTILS_CACHE_MINOR))
            changed = 1;

    }

    /* cleanup */
    eet_close(util_ef);
    eet_close(ef);

    /* unlink old cache files */
    if (changed)
    {
        if (unlink(efreet_desktop_cache_file()) < 0)
        {
            if (errno != ENOENT) goto error;
        }
        if (unlink(efreet_desktop_util_cache_file()) < 0)
        {
            if (errno != ENOENT) goto error;
        }
        /* rename tmp files to real files */
        if (rename(util_file, efreet_desktop_util_cache_file()) < 0) goto error;
        efreet_setowner(efreet_desktop_util_cache_file());
        if (rename(file, efreet_desktop_cache_file()) < 0) goto error;
        efreet_setowner(efreet_desktop_cache_file());
    }
    else
    {
        unlink(util_file);
        unlink(file);
    }

    {
        char c = 'n';

        if (changed) c = 'c';
        printf("%c\n", c);
    }

    EINA_LIST_FREE(systemdirs, dir)
        eina_stringshare_del(dir);
    eina_list_free(extra_dirs);
    eina_inarray_free(stack);
    efreet_shutdown();
    ecore_shutdown();
    eet_shutdown();
    eina_log_domain_unregister(_efreet_desktop_cache_log_dom);
    eina_shutdown();
    close(lockfd);
    return 0;
error:
    if (stack) eina_inarray_free(stack);
    IF_FREE(dir);
edd_error:
    if (old_file_ids)
    {
        eina_hash_free(old_file_ids->hash);
        free(old_file_ids);
    }
    efreet_shutdown();
efreet_error:
    ecore_shutdown();
ecore_error:
    eet_shutdown();
eet_error:
    EINA_LIST_FREE(systemdirs, dir)
        eina_stringshare_del(dir);
    eina_list_free(extra_dirs);
    eina_log_domain_unregister(_efreet_desktop_cache_log_dom);
    eina_shutdown();
eina_error:
    if (lockfd >= 0) close(lockfd);
    return 1;
}
