/* vim: set sw=4 ts=4 sts=4 et: */
#include "Efreet.h"
#include "efreet_private.h"


/**
 * @param full_uri: a valid uri string to parse
 * @return Return The corresponding Efreet_Uri structure. Or NULL on errors.
 * @brief Read a single uri and return an Efreet_Uri struct. If there's no
 * hostname in the uri then the hostname parameter will be NULL. All the uri
 * escaped chars will be converted to normal.
 */
EAPI Efreet_Uri *
efreet_uri_decode(const char *full_uri)
{
    Efreet_Uri *uri;
    const char *p;
    char protocol[64], hostname[_POSIX_HOST_NAME_MAX], path[PATH_MAX];
    int i = 0;

    /* An uri should be in the form <protocol>://<hostname>/<path> */
    if (!strstr(full_uri, "://")) return NULL;

    memset(protocol, 0, 64);
    memset(hostname, 0, _POSIX_HOST_NAME_MAX);
    memset(path, 0, PATH_MAX);

    /* parse protocol */
    p = full_uri;
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
 * @param uri: Create an URI string from an Efreet_Uri struct
 * @return The string rapresentation of uri (ex: 'file:///home/my%20name')
 * @brief Get the string rapresentation of the given uri struct escaping
 * illegal caracters. Remember to free the string with ecore_string_release()
 * when you don't need it anymore.
 * @note The resulting string will contain the protocol and the path but not
 * the hostname, as many apps doesn't handle it.
 */
EAPI const char *
efreet_uri_encode(Efreet_Uri *uri)
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
    FREE(uri);
}
