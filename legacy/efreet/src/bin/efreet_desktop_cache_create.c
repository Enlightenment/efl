/* vim: set sw=4 ts=4 sts=4 et: */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <limits.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <Eina.h>
#include <Ecore_File.h>

#include "Efreet.h"
#include "efreet_private.h"

static Eet_Data_Descriptor *edd = NULL;
static Eet_File *ef = NULL;
static Eet_File *util_ef = NULL;

static Eina_Hash *file_ids = NULL;
static Eina_Hash *paths = NULL;

static int
cache_add(const char *path, const char *file_id, int priority __UNUSED__)
{
    Efreet_Desktop *desk;
    char *ext;

    ext = strrchr(path, '.');
    if (!ext || strcmp(ext, ".desktop")) return 1;
    desk = efreet_desktop_new(path);

    if (!desk || desk->type != EFREET_DESKTOP_TYPE_APPLICATION)
    {
        if (desk) efreet_desktop_free(desk);
        return 1;
    }
    if (!eina_hash_find(paths, desk->orig_path))
    {
        if (!eet_data_write(ef, edd, desk->orig_path, desk, 0))
            return 0;
        eina_hash_add(paths, desk->orig_path, (void *)1);
    }
    /* TODO: We should check priority, and not just hope we search in right order */
    if (file_id && !eina_hash_find(file_ids, file_id))
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
cache_scan(const char *path, const char *base_id, int priority)
{
    char file_id[PATH_MAX];
    char buf[PATH_MAX];
    DIR *files;
    struct dirent *file;

    if (!ecore_file_is_dir(path)) return 1;

    files = opendir(path);
    file_id[0] = '\0';
    while ((file = readdir(files)))
    {
        if (!file) break;
        if (!strcmp(file->d_name, ".") || !strcmp(file->d_name, "..")) continue;

        snprintf(buf, sizeof(buf), "%s/%s", path, file->d_name);
        if (*base_id)
            snprintf(file_id, sizeof(file_id), "%s-%s", base_id, file->d_name);
        else
            strcpy(file_id, file->d_name);

        if (ecore_file_is_dir(buf))
        {
            cache_scan(buf, file_id, priority);
        }
        else
        {
            if (!cache_add(buf, file_id, priority)) return 0;
        }
    }
    closedir(files);
    return 1;
}

int
main()
{
    /* TODO:
     * - Add file monitor on files, so that we catch changes on files
     *   during whilst this program runs.
     * - Only update cache, don't recreate everything.
     * - Use return value to signal calling process wheter cache was
     *   updated or not.
     */
    char file[PATH_MAX];
    char util_file[PATH_MAX];
    Eina_List *dirs;
    int priority = 0;
    char *dir = NULL;
    int fd = -1, tmpfd;

    /* init external subsystems */
    if (!eet_init()) goto eet_error;
    if (!eina_init()) goto eina_error;

    /* create homedir */
    snprintf(file, sizeof(file), "%s/.efreet", efreet_home_dir_get());
    if (!ecore_file_mkpath(file)) goto efreet_error;

    /* lock process, so that we only run one copy of this program */
    snprintf(file, sizeof(file), "%s/.efreet/lock", efreet_home_dir_get());
    fd = open(file, O_CREAT | O_TRUNC | O_RDONLY, S_IRUSR | S_IWUSR);
    if (fd < 0) goto efreet_error;
    if (flock(fd, LOCK_EX | LOCK_NB) < 0) goto efreet_error;

    /* create dir for desktop cache */
    dir = ecore_file_dir_get(efreet_desktop_cache_file());
    if (!ecore_file_mkpath(dir)) goto efreet_error;
    free(dir);
    /* unlink old cache file */
    if (unlink(efreet_desktop_cache_file()) < 0)
    {
        if (errno != ENOENT) goto efreet_error;
    }
    /* create desktop cache file, so that efreet_init wont run another instance of this program */
    fd = open(efreet_desktop_cache_file(), O_CREAT | O_EXCL | O_RDONLY, S_IRUSR | S_IWUSR);
    if (fd < 0) goto efreet_error;
    close(fd);

    /* create dir for util cache */
    dir = ecore_file_dir_get(efreet_desktop_util_cache_file());
    if (!ecore_file_mkpath(dir)) goto efreet_error;
    free(dir);
    /* unlink old cache file */
    if (unlink(efreet_desktop_util_cache_file()) < 0)
    {
        if (errno != ENOENT) goto efreet_error;
    }
    /* create util cache file */
    fd = open(efreet_desktop_util_cache_file(), O_CREAT | O_EXCL | O_RDONLY, S_IRUSR | S_IWUSR);
    if (fd < 0) goto efreet_error;
    close(fd);


    /* finish efreet init */
    if (!efreet_init()) goto efreet_error;
    edd = efreet_desktop_edd_init();
    if (!edd) goto edd_error;

    /* create cache */
    snprintf(file, sizeof(file), "%s.XXXXXX", efreet_desktop_cache_file());
    tmpfd = mkstemp(file);
    if (tmpfd < 0) goto error;
    close(tmpfd);
    ef = eet_open(file, EET_FILE_MODE_WRITE);
    if (!ef) goto error;

    snprintf(util_file, sizeof(util_file), "%s.XXXXXX", efreet_desktop_util_cache_file());
    tmpfd = mkstemp(util_file);
    if (tmpfd < 0) goto error;
    close(tmpfd);
    util_ef = eet_open(util_file, EET_FILE_MODE_WRITE);
    if (!util_ef) goto error;

    file_ids = eina_hash_string_superfast_new(NULL);
    if (!file_ids) goto error;
    paths = eina_hash_string_superfast_new(NULL);
    if (!paths) goto error;

    dirs = efreet_default_dirs_get(efreet_data_home_get(), efreet_data_dirs_get(),
                                                                    "applications");
    /* TODO:
     * - read dirs cache
     * - skip dirs/subdirs in default dirs
     */
    if (!dirs) goto error;
    while (dirs)
    {
        char file_id[PATH_MAX] = { '\0' };
        char *path;

        path = eina_list_data_get(dirs);
        if (path)
        {
            if (!cache_scan(path, file_id, priority++)) goto error;
            free(path);
        }
        dirs = eina_list_remove_list(dirs, dirs);
    }
    eina_hash_free(file_ids);
    eina_hash_free(paths);
 
    /* cleanup */
    eet_close(util_ef);
    eet_close(ef);

    /* rename tmp files to real files */
    if (rename(file, efreet_desktop_cache_file()) < 0) goto error;
    if (rename(util_file, efreet_desktop_util_cache_file()) < 0) goto error;

    efreet_desktop_edd_shutdown(edd);
    efreet_shutdown();
    eet_shutdown();
    eina_shutdown();
    close(fd);
    return 0;
error:
    IF_FREE(dir);
    efreet_desktop_edd_shutdown(edd);
edd_error:
    efreet_shutdown();
efreet_error:
    eina_shutdown();
eina_error:
    eet_shutdown();
eet_error:
    if (fd > 0) close(fd);
    return 1;
}
