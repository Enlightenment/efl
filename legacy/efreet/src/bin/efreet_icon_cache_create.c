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
#include <Ecore.h>
#include <Ecore_File.h>

#include "Efreet.h"
#include "efreet_private.h"

static Eet_Data_Descriptor *edd = NULL;
static Eet_Data_Descriptor *fallback_edd = NULL;

static Eina_List *extensions;

static int verbose = 0;

static char file[PATH_MAX] = { '\0' };

static void
term_handler (int sig, siginfo_t * info, void *data)
{
    if (file[0]) unlink(file);
    if (verbose) printf("EXIT\n");
    exit(1);
}

static int
cache_fallback_scan_dir(Eet_File *ef, Eina_Hash *dirs, const char *dir, int *changed)
{
    Eina_Iterator *it;
    const char *ext, *ent;

    if (eina_hash_find(dirs, dir)) return 1;
    eina_hash_add(dirs, dir, (void *)-1);

    it = eina_file_ls(dir);
    if (!it) return 1;
    EINA_ITERATOR_FOREACH(it, ent)
    {
        Efreet_Cache_Icon *icon;
        char *name, *tmp;

        ext = strrchr(ent, '.');
        if (!ext) continue;
        ext = eina_stringshare_add(ext);
        if (!eina_list_data_find(extensions, ext))
        {
            eina_stringshare_del(ext);
            continue;
        }
        /* icon with known extension */
        name = strdup(ecore_file_file_get(ent));
        tmp = strrchr(name, '.');
        if (tmp) *tmp = '\0';
        icon = eet_data_read(ef, fallback_edd, name);
        if (!icon)
        {
            icon = NEW(Efreet_Cache_Icon, 1);
            icon->free = 1;
            icon->fallback = 1;
            icon->theme = NULL;
        }

        icon->icons = eina_list_append(icon->icons, eina_stringshare_ref(ent));
        if (!eet_data_write(ef, fallback_edd, name, icon, 1))
        {
            free(name);
            break;
        }
        efreet_cache_icon_free(icon);
        free(name);
    }
    eina_iterator_free(it);
    return 1;
}

static int
cache_fallback_scan(Eet_File *ef, int *changed)
{
    Eina_List *xdg_dirs, *l;
    const char *dir;
    char path[PATH_MAX];
    Eina_Hash *dirs;

    dirs = eina_hash_string_superfast_new(NULL);
    cache_fallback_scan_dir(ef, dirs, efreet_icon_deprecated_user_dir_get(), changed);
    cache_fallback_scan_dir(ef, dirs, efreet_icon_user_dir_get(), changed);

    xdg_dirs = efreet_data_dirs_get();
    EINA_LIST_FOREACH(xdg_dirs, l, dir)
    {
        snprintf(path, sizeof(path), "%s/icons", dir);
        cache_fallback_scan_dir(ef, dirs, path, changed);
    }

#ifndef STRICT_SPEC
    EINA_LIST_FOREACH(xdg_dirs, l, dir)
    {
        snprintf(path, sizeof(path), "%s/pixmaps", dir);
        cache_fallback_scan_dir(ef, dirs, path, changed);
    }
#endif

    cache_fallback_scan_dir(ef, dirs, "/usr/share/pixmaps", changed);
    eina_hash_free(dirs);
    return 1;
}

static int
cache_scan_path_dir(Efreet_Icon_Theme *theme, const char *path, Efreet_Icon_Theme_Directory *dir, Eet_File *ef, int *changed)
{
    Eina_Iterator *it;
    char buf[PATH_MAX];
    const char *ext, *ent;

    snprintf(buf, sizeof(buf), "%s/%s", path, dir->name);
    it = eina_file_ls(buf);
    if (!it) return 1;
    EINA_ITERATOR_FOREACH(it, ent)
    {
        Eina_List *l;
        Efreet_Cache_Icon *icon;
        Efreet_Cache_Icon_Element *elem = NULL, *oelem = NULL;
        char *name, *tmp;

        ext = strrchr(ent, '.');
        if (!ext) continue;
        ext = eina_stringshare_add(ext);
        if (!eina_list_data_find(extensions, ext))
        {
            eina_stringshare_del(ext);
            continue;
        }
        /* icon with known extension */
        name = strdup(ecore_file_file_get(ent));
        tmp = strrchr(name, '.');
        if (tmp) *tmp = '\0';
        icon = eet_data_read(ef, edd, name);
        if (!icon)
        {
            icon = NEW(Efreet_Cache_Icon, 1);
            icon->free = 1;
            icon->theme = eina_stringshare_add(theme->name.internal);
        }
        else if (strcmp(icon->theme, theme->name.internal))
        {
            /* We got this icon from a parent theme */
            free(name);
            continue;
        }

        /* find if we have the same icon in another type */
        EINA_LIST_FOREACH(icon->icons, l, oelem)
        {
            if ((oelem->type == dir->type) &&
                (oelem->size.normal == dir->size.normal) &&
                (oelem->size.max == dir->size.max) &&
                (oelem->size.min == dir->size.min))
            {
                elem = oelem;
                break;
            }
        }
        if (elem)
        {
            elem->paths = eina_list_append(elem->paths, eina_stringshare_ref(ent));
        }
        else
        {
            elem = NEW(Efreet_Cache_Icon_Element, 1);
            elem->paths = eina_list_append(elem->paths, eina_stringshare_ref(ent));
            elem->type = dir->type;
            elem->size.normal = dir->size.normal;
            elem->size.min = dir->size.min;
            elem->size.max = dir->size.max;
            icon->icons = eina_list_append(icon->icons, elem);
        }
        if (!eet_data_write(ef, edd, name, icon, 1))
        {
            free(name);
            break;
        }
        efreet_cache_icon_free(icon);
        free(name);
    }
    eina_iterator_free(it);
    return 1;
}

static int
cache_scan_path(Efreet_Icon_Theme *theme, const char *path, Eet_File *ef, int *changed)
{
    Eina_List *l;
    Efreet_Icon_Theme_Directory *dir;

    EINA_LIST_FOREACH(theme->directories, l, dir)
        if (!cache_scan_path_dir(theme, path, dir, ef, changed)) return 0;
    return 1;
}

static int
cache_scan(Efreet_Icon_Theme *theme, Eina_Hash *themes, Eet_File *ef, int *changed)
{
    Eina_List *l;
    const char *path;
    const char *name;

    if (!theme) return 1;
    if (eina_hash_find(themes, theme->name.internal)) return 1;
    eina_hash_direct_add(themes, theme->name.internal, theme);
    /* TODO: Maybe always read entry from eet, so when can check changed */

    /* scan theme */
    EINA_LIST_FOREACH(theme->paths, l, path)
        if (!cache_scan_path(theme, path, ef, changed)) return 0;

    /* scan inherits */
    if (theme->inherits)
    {
        EINA_LIST_FOREACH(theme->inherits, l, name)
        {
            theme = efreet_icon_theme_find(name);
            if (!cache_scan(theme, themes, ef, changed)) return 0;
        }
    }
    else if (strcmp(theme->name.internal, "hicolor"))
    {
        theme = efreet_icon_theme_find("hicolor");
        if (!cache_scan(theme, themes, ef, changed)) return 0;
    }
    return 1;
}

int
main(int argc, char **argv)
{
    /* TODO:
     * - Add file monitor on files, so that we catch changes on files
     *   during whilst this program runs.
     * - Maybe linger for a while to reduce number of cache re-creates.
     * - pass extra dirs to binary, and read them
     * - make sure programs with different extra dirs all work together
     */
    Eet_File *ef;
    Eina_List *l = NULL;
    Efreet_Icon_Theme *theme;
    char *dir = NULL;
    int lockfd = -1, tmpfd;
    int changed = 0;
    int i;
    struct flock fl;
    const char *exts[] = { ".png", ".xpm", ".svg", NULL };
    struct sigaction act;

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

    // Trap SIGTERM for clean shutdown
    act.sa_sigaction = term_handler;
    act.sa_flags = SA_RESTART | SA_SIGINFO;
    sigemptyset(&act.sa_mask);

    if (sigaction(SIGTERM, &act, NULL) < 0)
    {
        perror("sigaction");
        goto efreet_error;
    }

    for (i = 0; exts[i]; i++)
        extensions = eina_list_append(extensions, eina_stringshare_add(exts[i]));

    efreet_cache_update = 0;

    /* create homedir */
    snprintf(file, sizeof(file), "%s/.efreet", efreet_home_dir_get());
    if (!ecore_file_mkpath(file)) goto efreet_error;

    /* lock process, so that we only run one copy of this program */
    snprintf(file, sizeof(file), "%s/.efreet/icon_data.lock", efreet_home_dir_get());
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

    /* create dir for icon cache */
    dir = ecore_file_dir_get(efreet_icon_cache_file(""));
    if (!ecore_file_mkpath(dir)) goto efreet_error;
    free(dir);

    /* finish efreet init */
    if (!efreet_init()) goto efreet_error;
    edd = efreet_icon_edd_init();
    if (!edd) goto edd_error;
    fallback_edd = efreet_icon_fallback_edd_init();
    if (!edd) goto edd_error;

    if (argc > 1)
    {
        for (i = 1; i < argc; i++)
        {
            theme = efreet_icon_theme_find(argv[i]);
            if (theme) l = eina_list_append(l, theme);
        }
    }
    if (!l)
        l = efreet_icon_theme_list_get();
    EINA_LIST_FREE(l, theme)
    {
        Eina_Hash *themes;

        changed = 0;
        /* create cache */
        /* TODO: Copy old cache to temp file, so we can check whether something has changed */
        snprintf(file, sizeof(file), "%s.XXXXXX", efreet_icon_cache_file(theme->name.internal));
        tmpfd = mkstemp(file);
        if (tmpfd < 0) goto error;
        close(tmpfd);
        ef = eet_open(file, EET_FILE_MODE_READ_WRITE);
        if (!ef) goto error;

        themes = eina_hash_string_superfast_new(NULL);
        if (!cache_scan(theme, themes, ef, &changed))
        {
            eet_close(ef);
            eina_hash_free(themes);
            goto error;
        }

        changed = 1;
        /* check if old and new caches contain the same number of entries */
#if 0
        if (!changed)
        {
            Eet_File *old;

            old = eet_open(efreet_icon_cache_file(), EET_FILE_MODE_READ);
            if (!old || eet_num_entries(old) != eet_num_entries(ef)) changed = 1;
            if (old) eet_close(old);

        }
#endif

        /* cleanup */
        eet_close(ef);
        eina_hash_free(themes);

        /* unlink old cache files */
        if (changed)
        {
            if (unlink(efreet_icon_cache_file(theme->name.internal)) < 0)
            {
                if (errno != ENOENT) goto error;
            }
            /* rename tmp files to real files */
            if (rename(file, efreet_icon_cache_file(theme->name.internal)) < 0) goto error;
        }
        else
        {
            unlink(file);
        }

        eet_clearcache();
    }

    /* fallback */
    changed = 0;
    /* TODO: Copy old cache to temp file, so we can check whether something has changed */
    snprintf(file, sizeof(file), "%s.XXXXXX", efreet_icon_cache_file("_fallback"));
    tmpfd = mkstemp(file);
    if (tmpfd < 0) goto error;
    close(tmpfd);
    ef = eet_open(file, EET_FILE_MODE_READ_WRITE);
    if (!ef) goto error;

    if (!cache_fallback_scan(ef, &changed))
    {
        eet_close(ef);
        goto error;
    }

    changed = 1;
    /* check if old and new caches contain the same number of entries */
#if 0
    if (!changed)
    {
        Eet_File *old;

        old = eet_open(efreet_icon_cache_file(), EET_FILE_MODE_READ);
        if (!old || eet_num_entries(old) != eet_num_entries(ef)) changed = 1;
        if (old) eet_close(old);

    }
#endif

    /* cleanup */
    eet_close(ef);

    /* unlink old cache files */
    if (changed)
    {
        if (unlink(efreet_icon_cache_file("_fallback")) < 0)
        {
            if (errno != ENOENT) goto error;
        }
        /* rename tmp files to real files */
        if (rename(file, efreet_icon_cache_file("_fallback")) < 0) goto error;
    }
    else
    {
        unlink(file);
    }

    /* Remove signal handler, no need to exit now */
    act.sa_sigaction = SIG_IGN;
    act.sa_flags = SA_RESTART | SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    sigaction(SIGTERM, &act, NULL);

    eina_list_free(extensions);

    /* touch update file */
    snprintf(file, sizeof(file), "%s/.efreet/icon_data.update", efreet_home_dir_get());
    tmpfd = open(file, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    if (tmpfd >= 0)
    {
        write(tmpfd, "a", 1);
        close(tmpfd);
    }
    efreet_shutdown();
    ecore_shutdown();
    eet_shutdown();
    eina_shutdown();
    close(lockfd);
    return 0;
error:
    printf("error\n");
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
