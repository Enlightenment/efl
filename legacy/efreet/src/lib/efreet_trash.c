/* vim: set sw=4 ts=4 sts=4 et: */
#include <errno.h>
#include <time.h>
#include "Efreet.h"
#include "Efreet_Trash.h"
#include "efreet_private.h"

static const char *efreet_trash_dir = NULL;

/**
 * @return Returns 1 on success or 0 on failure
 * @brief Initializes the efreet trash system
 */
EAPI int
efreet_trash_init(void)
{
    return 1;
}

/**
 * @return Returns no value
 * @brief Cleans up the efreet trash system
 */
EAPI void
efreet_trash_shutdown(void)
{
    IF_RELEASE(efreet_trash_dir);
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
    efreet_trash_dir = ecore_string_instance(buf);

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
    snprintf(dest, PATH_MAX, "%s/files/%s", efreet_trash_dir_get(), fname);

    /* search for a free filename */
    while (ecore_file_exists(dest))
        snprintf(dest, PATH_MAX, "%s/files/%s$%d",
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
                printf("EFREET TRASH ERROR: Can't delete file.\n");
                return 0; 
            }
        }
        else
        {
            printf("EFREET TRASH ERROR: Can't move file to trash.\n");
            return 0;
        }
    }
   
      
    /* create info file */
    snprintf(dest, PATH_MAX, "%s/info/%s.trashinfo",
             efreet_trash_dir_get(), fname);

    if (f = fopen(dest, "w"))
    {
        fputs("[Trash Info]\n", f); //TODO is '\n' right?? (or \r\c??)

        fputs("Path=", f);
        escaped = efreet_uri_escape(uri);
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
        printf("EFREET TRASH ERROR: Can't create trash info file.\n");
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
    snprintf(buf, PATH_MAX, "%s/files", efreet_trash_dir_get());
   
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

    snprintf(buf, PATH_MAX, "%s/info", efreet_trash_dir_get());
    if (!ecore_file_recursive_rm(buf)) return 0;
    ecore_file_mkdir(buf);

    snprintf(buf, PATH_MAX, "%s/files", efreet_trash_dir_get());
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
EAPI Ecore_List*
efreet_trash_ls(void)
{
    char *infofile;
    char buf[PATH_MAX];
    Ecore_List *files;

    // NOTE THIS FUNCTION NOW IS NOT COMPLETE AS I DON'T NEED IT
    // TODO read the name from the infofile instead of the filename

    snprintf(buf, PATH_MAX, "%s/files", efreet_trash_dir_get());
    files = ecore_file_ls(buf);

    while (infofile = ecore_list_next(files))
    {
        printf("FILE: %s\n", infofile);
    }

    return files;
}


/**
 * @param val: a valid uri string to parse
 * @return Return The corresponding Efreet_Uri structure. Or NULL on errors.
 * @brief Parse a single uri and return an Efreet_Uri struct. If there's no
 * hostname in the uri then the hostname parameter is NULL. All the uri escaped
 * chars will be converted back.
 */
EAPI Efreet_Uri *
efreet_uri_parse(const char *val)
{
    Efreet_Uri *uri;
    const char *p;
    char protocol[64], hostname[_POSIX_HOST_NAME_MAX], path[PATH_MAX];
    int i = 0;

    /* An uri should be in the form <protocol>://<hostname>/<path> */
    p = strstr(val, "://");
    if (!p) return NULL;

    memset(protocol, 0, 64);
    memset(hostname, 0, _POSIX_HOST_NAME_MAX);
    memset(path, 0, PATH_MAX);

    /* parse protocol */
    p = val;
    for (i = 0; *p != ':' && *p != '\0' && i < 64; p++, i++)
         protocol[i] = *p;
    protocol[i] = '\0';

    /* parse hostname */
    p += 3;
    if (*p != '/')
    {
        for (i = 0; *p != '/' && *p != '\0' && i < _POSIX_HOST_NAME_MAX; p++, i++)
            hostname[i] = *p;
        hostname[i] = '\0';
    }
    else
        hostname[0] = '\0';

    /* parse path */
    /* See http://www.faqs.org/rfcs/rfc1738.html for the escaped chars */
    for (i = 0; *p != '\0' && i < PATH_MAX; i++, p++)
    {
        if (*p == '%')
        {
            path[i] = *(++p);
            path[i + 1] = *(++p);
            path[i] = (char)strtol(&(path[i]), NULL, 16);
            path[i + 1] = '\0';
        }
        else
            path[i] = *p;
    }

    uri = NEW(Efreet_Uri, 1);
    if (!uri) return NULL;

    uri->protocol = ecore_string_instance(protocol);
    uri->hostname = ecore_string_instance(hostname);
    uri->path = ecore_string_instance(path);

    return uri;
}

/**
 * @param uri: The uri structure to escape
 * @return The string rapresentation of an uri (ex: 'file:///home/my%20name')
 * @brief Get the string rapresentation of the given uri struct escaping
 * illegal caracters. The resulting string will contain the protocol but not the
 * hostname, as many apps doesn't handle it.
 */
EAPI const char *
efreet_uri_escape(Efreet_Uri *uri)
{
    char dest[PATH_MAX * 3 + 4];
    const char *p;
    int i;

    if (!uri || !uri->path || !uri->protocol) return NULL;
    memset(dest, 0, PATH_MAX * 3 + 4);
    snprintf(dest, strlen(uri->protocol) + 4, "%s://", uri->protocol);


    /* Most app doesn't handle the hostname in the uri so it's put to NULL */
    for (i = strlen(uri->protocol) + 3, p = uri->path; *p != '\0'; p++, i++)
    {
        if (isalnum(*p) || strchr("/$-_.+!*'()", *p))
            dest[i] = *p;
        else
        {
            snprintf(&(dest[i]), 4, "%%%02X", *p);
            i += 2;
        }
    }

    return ecore_string_instance(dest);
}

/**
 * @param uri: The uri to free
 * @brief Free the given uri structure.
 */
EAPI void
efreet_uri_free(Efreet_Uri *uri)
{
    if (!uri) return;
    IF_RELEASE(uri->protocol);
    IF_RELEASE(uri->path);
    IF_RELEASE(uri->hostname);
    free(uri);
    uri = NULL;
}
