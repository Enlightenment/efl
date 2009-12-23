/* vim: set sw=4 ts=4 sts=4 et: */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <time.h>

#include <Ecore_File.h>

#include "Efreet.h"
#include "Efreet_Trash.h"
#include "efreet_private.h"

static unsigned int _efreet_trash_init_count = 0;
static const char *efreet_trash_dir = NULL;

/* define macros and variable for using the eina logging system  */

#ifdef EFREET_MODULE_LOG_DOM 
#undef EFREET_MODULE_LOG_DOM
#endif
#define EFREET_MODULE_LOG_DOM _efreet_trash_log_dom
static int _efreet_trash_log_dom = -1;

/**
 * @return Returns 1 on success or 0 on failure
 * @brief Initializes the efreet trash system
 */
EAPI int
efreet_trash_init(void)
{
    if (++_efreet_trash_init_count != 1)
        return _efreet_trash_init_count;

    if (!eina_init())
        return --_efreet_trash_init_count;

    _efreet_trash_log_dom = eina_log_domain_register("Efreet_trash", EFREET_DEFAULT_LOG_COLOR);
    if (_efreet_trash_log_dom < 0)
    {
	ERROR("Efreet: Could not create a log domain for Efreet_trash");
        eina_shutdown();
        return --_efreet_trash_init_count;
      }
    return _efreet_trash_init_count;
}

/**
 * @return Returns no value
 * @brief Cleans up the efreet trash system
 */
EAPI int
efreet_trash_shutdown(void)
{
    if (--_efreet_trash_init_count != 0)
        return _efreet_trash_init_count;

    IF_RELEASE(efreet_trash_dir);
    eina_log_domain_unregister(_efreet_trash_log_dom);
    eina_shutdown();

    return _efreet_trash_init_count;
}

/**
 * @return Returns the XDG Trash local directory or NULL on errors
 * @brief Retrieves the XDG Trash local directory
 */
EAPI const char*
efreet_trash_dir_get(void)
{
    char buf[PATH_MAX];

    if (efreet_trash_dir && ecore_file_exists(efreet_trash_dir))
        return efreet_trash_dir;

    snprintf(buf, sizeof(buf), "%s/Trash", efreet_data_home_get());
    if (!ecore_file_exists(buf) && !ecore_file_mkpath(buf))
        return NULL;

    IF_RELEASE(efreet_trash_dir);
    efreet_trash_dir = eina_stringshare_add(buf);

    snprintf(buf, sizeof(buf), "%s/files", efreet_trash_dir);
    if (!ecore_file_exists(buf) && !ecore_file_mkpath(buf))
        return NULL;

    snprintf(buf, sizeof(buf), "%s/info", efreet_trash_dir);
    if (!ecore_file_exists(buf) && !ecore_file_mkpath(buf))
        return NULL;

    return efreet_trash_dir;
}

/**
 * @param uri: The local uri to move in the trash
 * @param force_delete: If you set this to 1 than files on different filesystems
 * will be deleted permanently
 * @return Return 1 on success, 0 on failure or -1 in case the uri is not on the
 * same filesystem and force_delete is not set.
 * @brief This function try to move the given uri to the trash. Files on 
 * different filesystem can't be moved to trash. If force_delete
 * is 0 than non-local files will be ignored and -1 is returned, if you set
 * force_delete to 1 non-local files will be deleted without asking.
 */
EAPI int
efreet_trash_delete_uri(Efreet_Uri *uri, int force_delete)
{
    char dest[PATH_MAX];
    char times[64];
    const char *fname;
    const char *escaped;
    int i = 1;
    time_t now;
    FILE *f;

    if (!uri || !uri->path || !ecore_file_can_write(uri->path)) return 0;

    fname = ecore_file_file_get(uri->path);
    snprintf(dest, sizeof(dest), "%s/files/%s", efreet_trash_dir_get(), fname);

    /* search for a free filename */
    while (ecore_file_exists(dest))
        snprintf(dest, sizeof(dest), "%s/files/%s$%d",
                 efreet_trash_dir_get(), fname, i++);
    fname = ecore_file_file_get(dest);

    /* move file to trash dir */
    if (rename(uri->path, dest))
    {
        if (errno == EXDEV)
        {
            if (!force_delete) return -1;
            if (!ecore_file_recursive_rm(uri->path))
            {
                ERR("EFREET TRASH ERROR: Can't delete file.");
                return 0; 
            }
        }
        else
        {
            ERR("EFREET TRASH ERROR: Can't move file to trash.");
            return 0;
        }
    }

    /* create info file */
    snprintf(dest, sizeof(dest), "%s/info/%s.trashinfo",
             efreet_trash_dir_get(), fname);

    if ((f = fopen(dest, "w")))
    {
        fputs("[Trash Info]\n", f); //TODO is '\n' right?? (or \r\c??)

        fputs("Path=", f);
        escaped = efreet_uri_encode(uri);
        fputs(escaped + 7, f); // +7 == don't write 'file://'
        IF_RELEASE(escaped);

        time(&now);
        strftime(times, sizeof(times), "%Y-%m-%dT%H:%M:%S", localtime(&now));
        fputs("\nDeletionDate=", f);
        fputs(times, f);
        fputs("\n", f);
        fclose(f);
    }
    else
    {
        ERR("EFREET TRASH ERROR: Can't create trash info file.");
        return 0;
    }

    return 1;
}

/**
 * @return Return 1 if the trash is empty or 0 if some file are in.
 * @brief Check if the trash is currently empty
 */
EAPI int
efreet_trash_is_empty(void)
{
    char buf[PATH_MAX];

    snprintf(buf, sizeof(buf), "%s/files", efreet_trash_dir_get());

    /* TODO Check also trash in other filesystems */
    return ecore_file_dir_is_empty(buf);
}

/**
 * @return Return 1 on success or 0 on failure
 * @brief Delete all the files inside the trash.
 */
EAPI int
efreet_trash_empty_trash(void)
{
    char buf[PATH_MAX];

    snprintf(buf, sizeof(buf), "%s/info", efreet_trash_dir_get());
    if (!ecore_file_recursive_rm(buf)) return 0;
    ecore_file_mkdir(buf);

    snprintf(buf, sizeof(buf), "%s/files", efreet_trash_dir_get());
    if (!ecore_file_recursive_rm(buf)) return 0;
    ecore_file_mkdir(buf);

    /* TODO Empty also trash in other filesystems */
    return 1;
}

/**
 * @return Return a list of strings with filename (remember to free the list
 * when you don't need anymore)
 * @brief List all the files and directory currently inside the trash.
 */
EAPI Eina_List*
efreet_trash_ls(void)
{
    char *infofile;
    char buf[PATH_MAX];
    Eina_List *files, *l;

    // NOTE THIS FUNCTION NOW IS NOT COMPLETE AS I DON'T NEED IT
    // TODO read the name from the infofile instead of the filename

    snprintf(buf, sizeof(buf), "%s/files", efreet_trash_dir_get());
    files = ecore_file_ls(buf);

    EINA_LIST_FOREACH(files, l, infofile)
        printf("FILE: %s\n", infofile);

    return files;
}

