#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <limits.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <Eina.h>
#include <Eet.h>
#include <Ecore.h>
#include <Ecore_File.h>

#include "Efreet.h"
#include "efreet_private.h"
#include "efreet_cache_private.h"

static Eet_Data_Descriptor *edd = NULL;
static Eet_File *ef = NULL;
static Eet_File *util_ef = NULL;

static Eina_Hash *file_ids = NULL;
static Eina_Hash *paths = NULL;

static int verbose = 0;

static int
strcmplen(const void *data1, const void *data2)
{
    return strncmp(data1, data2, eina_stringshare_strlen(data2));
}

static int
cache_add(const char *path, const char *file_id, int priority __UNUSED__, int *changed)
{
    Efreet_Desktop *desk;
    char *ext;

    if (verbose)
    {
        printf("FOUND: %s\n", path);
        if (file_id) printf(" (id): %s\n", file_id);
    }
    ext = strrchr(path, '.');
    if (!ext || (strcmp(ext, ".desktop") && strcmp(ext, ".directory"))) return 1;
    desk = efreet_desktop_new(path);
    if (verbose)
    {
        if (desk) printf("  OK\n");
        else      printf("  FAIL\n");
    }
    if (!desk) return 1;
    if (!desk->eet)
    {
        /* This file isn't in cache */
        *changed = 1;
        if (verbose)
        {
            printf("  NEW\n");
        }
    }
    else if (ecore_file_mod_time(desk->orig_path) != desk->load_time)
    {
        efreet_desktop_free(desk);
        *changed = 1;
        desk = efreet_desktop_uncached_new(path);
        if (verbose)
        {
            if (desk) printf("  CHANGED\n");
            else      printf("  NO UNCACHED\n");
        }
    }
    if (!desk) return 1;
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
        int id;
        char key[PATH_MAX];
        char *data;
        int i = 0;
        Eina_List *l;

        id = eina_hash_population(file_ids);
        i = 0;
        EINA_LIST_FOREACH(desk->mime_types, l, data)
        {
            snprintf(key, sizeof(key), "%d::%d::m", id, i++);
            if (!eet_write(util_ef, key, data, strlen(data) + 1, 0)) return 0;
        }
        i = 0;
        EINA_LIST_FOREACH(desk->categories, l, data)
        {
            snprintf(key, sizeof(key), "%d::%d::ca", id, i++);
            if (!eet_write(util_ef, key, data, strlen(data) + 1, 0)) return 0;
        }
        if (desk->startup_wm_class)
        {
            data = desk->startup_wm_class;
            snprintf(key, sizeof(key), "%d::swc", id);
            if (!eet_write(util_ef, key, data, strlen(data) + 1, 0)) return 0;
        }
        if (desk->name)
        {
            data = desk->name;
            snprintf(key, sizeof(key), "%d::n", id);
            if (!eet_write(util_ef, key, data, strlen(data) + 1, 0)) return 0;
        }
        if (desk->generic_name)
        {
            data = desk->generic_name;
            snprintf(key, sizeof(key), "%d::gn", id);
            if (!eet_write(util_ef, key, data, strlen(data) + 1, 0)) return 0;
        }
        if (desk->comment)
        {
            data = desk->comment;
            snprintf(key, sizeof(key), "%d::co", id);
            if (!eet_write(util_ef, key, data, strlen(data) + 1, 0)) return 0;
        }
        if (desk->exec)
        {
            data = desk->exec;
            snprintf(key, sizeof(key), "%d::e", id);
            if (!eet_write(util_ef, key, data, strlen(data) + 1, 0)) return 0;
        }
        if (desk->orig_path)
        {
            data = desk->orig_path;
            snprintf(key, sizeof(key), "%d::op", id);
            if (!eet_write(util_ef, key, data, strlen(data) + 1, 0)) return 0;
        }
        snprintf(key, sizeof(key), "%d::fi", id);
        if (!eet_write(util_ef, key, file_id, strlen(file_id) + 1, 0)) return 0;

        eina_hash_add(file_ids, file_id, (void *)1);
    }
    efreet_desktop_free(desk);
    return 1;
}


static int
cache_scan(const char *path, const char *base_id, int priority, int recurse, int *changed)
{
    char *file_id = NULL;
    char id[PATH_MAX];
    char buf[PATH_MAX];
    DIR *files;
    struct dirent *ent;

    if (!ecore_file_is_dir(path)) return 1;

    files = opendir(path);
    if (!files) return 1;
    id[0] = '\0';
    while ((ent = readdir(files)))
    {
        if (!ent) break;
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) continue;

        if (base_id)
        {
            if (*base_id)
                snprintf(id, sizeof(id), "%s-%s", base_id, ent->d_name);
            else
                strcpy(id, ent->d_name);
            file_id = id;
        }

        snprintf(buf, sizeof(buf), "%s/%s", path, ent->d_name);
        if (ecore_file_is_dir(buf))
        {
            if (recurse)
                cache_scan(buf, file_id, priority, recurse, changed);
        }
        else
        {
            if (!cache_add(buf, file_id, priority, changed))
            {
                closedir(files);
                return 0;
            }
        }
    }
    closedir(files);
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
    Efreet_Cache_Version version;
    Eina_List *dirs = NULL, *user_dirs = NULL;
    int priority = 0;
    char *dir = NULL;
    char *path;
    int lockfd = -1, tmpfd, dirsfd = -1;
    struct stat st;
    int changed = 0;
    int i;
    struct flock fl;
    char file[PATH_MAX] = { '\0' };
    char util_file[PATH_MAX] = { '\0' };


    for (i = 1; i < argc; i++)
    {
        if      (!strcmp(argv[i], "-v")) verbose = 1;
        else if ((!strcmp(argv[i], "-h")) ||
                 (!strcmp(argv[i], "-help")) ||
                 (!strcmp(argv[i], "--h")) ||
                 (!strcmp(argv[i], "--help")))
        {
            printf("Options:\n");
            printf("  -v     Verbose mode\n");
            exit(0);
        }
    }
    /* init external subsystems */
    if (!eina_init()) goto eina_error;
    if (!eet_init()) goto eet_error;
    if (!ecore_init()) goto ecore_error;

    efreet_cache_update = 0;

    /* create homedir */
    snprintf(file, sizeof(file), "%s/efreet", efreet_cache_home_get());
    if (!ecore_file_mkpath(file)) goto efreet_error;

    /* lock process, so that we only run one copy of this program */
    snprintf(file, sizeof(file), "%s/efreet/desktop_data.lock", efreet_cache_home_get());
    lockfd = open(file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (lockfd < 0) goto efreet_error;
    memset(&fl, 0, sizeof(struct flock));
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    if (fcntl(lockfd, F_SETLK, &fl) < 0)
    {
        if (verbose)
        {
            printf("LOCKED! You may want to delete %s if this persists\n", file);
        }
        goto efreet_error;
    }

    /* create dir for desktop cache */
    dir = ecore_file_dir_get(efreet_desktop_cache_file());
    if (!ecore_file_mkpath(dir)) goto efreet_error;
    free(dir);

    /* create dir for util cache */
    dir = ecore_file_dir_get(efreet_desktop_util_cache_file());
    if (!ecore_file_mkpath(dir)) goto efreet_error;
    free(dir);

    /* finish efreet init */
    if (!efreet_init()) goto efreet_error;
    edd = efreet_desktop_edd();
    if (!edd) goto edd_error;

    /* create cache */
    snprintf(file, sizeof(file), "%s.XXXXXX", efreet_desktop_cache_file());
    tmpfd = mkstemp(file);
    if (tmpfd < 0) goto error;
    close(tmpfd);
    ef = eet_open(file, EET_FILE_MODE_READ_WRITE);
    if (!ef) goto error;

    snprintf(util_file, sizeof(util_file), "%s.XXXXXX", efreet_desktop_util_cache_file());
    tmpfd = mkstemp(util_file);
    if (tmpfd < 0) goto error;
    close(tmpfd);
    util_ef = eet_open(util_file, EET_FILE_MODE_READ_WRITE);
    if (!util_ef) goto error;

    file_ids = eina_hash_string_superfast_new(NULL);
    if (!file_ids) goto error;
    paths = eina_hash_string_superfast_new(NULL);
    if (!paths) goto error;

    dirs = efreet_default_dirs_get(efreet_data_home_get(), efreet_data_dirs_get(),
                                                                    "applications");
    if (!dirs) goto error;

    dirsfd = open(efreet_desktop_cache_dirs(), O_APPEND | O_RDWR, S_IRUSR | S_IWUSR);
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
                user_dirs = eina_list_append(user_dirs, eina_stringshare_add(p));
                p += size;
            }
            munmap(map, st.st_size);
        }
        close(dirsfd);
        dirsfd = -1;
        unlink(efreet_desktop_cache_dirs());
    }

    EINA_LIST_FREE(dirs, path)
    {
        char file_id[PATH_MAX] = { '\0' };
        Eina_List *l;

        if (!cache_scan(path, file_id, priority++, 1, &changed)) goto error;
        l = eina_list_search_unsorted_list(user_dirs, strcmplen, path);
        if (l)
        {
            eina_stringshare_del(eina_list_data_get(l));
            user_dirs = eina_list_remove_list(user_dirs, l);
        }
        eina_stringshare_del(path);
    }
    if (user_dirs)
    {
        dirsfd = open(efreet_desktop_cache_dirs(), O_CREAT | O_APPEND | O_WRONLY, S_IRUSR | S_IWUSR);
        if (dirsfd < 0) goto error;
        EINA_LIST_FREE(user_dirs, dir)
        {
            unsigned int size = strlen(dir) + 1;
            size_t count;

            count = write(dirsfd, &size, sizeof(int));
            count += write(dirsfd, dir, size);

            if (count != sizeof (int) + size)
                printf("Didn't write all data on dirsfd");

            if (!cache_scan(dir, NULL, priority, 0, &changed)) goto error;
            eina_stringshare_del(dir);
        }
        close(dirsfd);
        dirsfd = -1;
    }
    eina_hash_free(file_ids);
    eina_hash_free(paths);

    /* check if old and new caches contain the same number of entries */
    if (!changed)
    {
        Eet_File *old;

        old = eet_open(efreet_desktop_cache_file(), EET_FILE_MODE_READ);
        if (!old || eet_num_entries(old) != eet_num_entries(ef)) changed = 1;
        if (old) eet_close(old);
        old = eet_open(efreet_desktop_util_cache_file(), EET_FILE_MODE_READ);
        if (!old || eet_num_entries(old) != eet_num_entries(util_ef)) changed = 1;
        if (old) eet_close(old);

    }

    /* cleanup */
    version.major = EFREET_DESKTOP_UTILS_CACHE_MAJOR;
    version.minor = EFREET_DESKTOP_UTILS_CACHE_MINOR;
    eet_data_write(util_ef, efreet_version_edd(), EFREET_CACHE_VERSION, &version, 1);
    version.major = EFREET_DESKTOP_CACHE_MAJOR;
    version.minor = EFREET_DESKTOP_CACHE_MINOR;
    eet_data_write(ef, efreet_version_edd(), EFREET_CACHE_VERSION, &version, 1);
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
        if (rename(file, efreet_desktop_cache_file()) < 0) goto error;
    }
    else
    {
        unlink(util_file);
        unlink(file);
    }

    /* touch update file */
    snprintf(file, sizeof(file), "%s/efreet/desktop_data.update", efreet_cache_home_get());
    tmpfd = open(file, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    if (tmpfd >= 0)
    {
        if (write(tmpfd, "a", 1) != 1) perror("write");
        close(tmpfd);
    }
    efreet_shutdown();
    ecore_shutdown();
    eet_shutdown();
    eina_shutdown();
    close(lockfd);
    return 0;
error:
    if (dirsfd >= 0) close(dirsfd);
    IF_FREE(dir);
edd_error:
    efreet_shutdown();
efreet_error:
    ecore_shutdown();
ecore_error:
    eet_shutdown();
eet_error:
    eina_shutdown();
eina_error:
    if (lockfd >= 0) close(lockfd);
    return 1;
}
