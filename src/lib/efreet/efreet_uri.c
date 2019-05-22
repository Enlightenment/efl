#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <ctype.h>

#ifndef _POSIX_HOST_NAME_MAX
#define _POSIX_HOST_NAME_MAX 255
#endif

/* define macros and variable for using the eina logging system  */
#define EFREET_MODULE_LOG_DOM /* no logging in this file */

#include "Efreet.h"
#include "efreet_private.h"

/* The URI standard is at http://tools.ietf.org/html/std66 */

EAPI Efreet_Uri *
efreet_uri_decode(const char *full_uri)
{
    Efreet_Uri *uri;
    const char *p;
    char scheme[64], authority[_POSIX_HOST_NAME_MAX], path[PATH_MAX];
    int i = 0;

    EINA_SAFETY_ON_NULL_RETURN_VAL(full_uri, NULL);

    /* An uri should be in the form <scheme>:[<authority>][<path>][<query>][<fragment>] */
    if (!strstr(full_uri, ":")) return NULL;


    memset(scheme, 0, 64);
    memset(authority, 0, _POSIX_HOST_NAME_MAX);
    memset(path, 0, PATH_MAX);

    /* parse scheme */
    p = full_uri;
    for (i = 0; *p != ':' && *p != '\0' && i < (64 - 1); p++, i++)
         scheme[i] = *p;
    if (i == 0) return NULL; /* scheme is required */
    scheme[i] = '\0';

    /* parse authority */
    p++;
    if (*p == '/')
    {
        p++;
        if (*p == '/')
        {
            p++;
            for (i = 0; *p != '/' && *p != '?' && *p != '#' && *p != '\0' && i < (_POSIX_HOST_NAME_MAX - 1); p++, i++)
                authority[i] = *p;
            authority[i] = '\0';
        }
        else /* It's a path, let path parser handle the leading slash */
            p--;
    }
    else
        authority[0] = '\0';

    /* parse path */
    /* See http://www.faqs.org/rfcs/rfc1738.html for the escaped chars */
    for (i = 0; *p != '\0' && i < (PATH_MAX - 1); i++, p++)
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

    uri->protocol = eina_stringshare_add(scheme);
    uri->hostname = eina_stringshare_add(authority);
    uri->path = eina_stringshare_add(path);

    return uri;
}

EAPI const char *
efreet_uri_encode(Efreet_Uri *uri)
{
    char dest[PATH_MAX * 3 + 4];
    const char *p;
    int i;

    EINA_SAFETY_ON_NULL_RETURN_VAL(uri, NULL);
    EINA_SAFETY_ON_NULL_RETURN_VAL(uri->path, NULL);
    EINA_SAFETY_ON_NULL_RETURN_VAL(uri->protocol, NULL);

    memset(dest, 0, PATH_MAX * 3 + 4);
    snprintf(dest, strlen(uri->protocol) + 4, "%s://", uri->protocol);

    /* Most app doesn't handle the hostname in the uri so it's put to NULL */
    for (i = strlen(uri->protocol) + 3, p = uri->path; *p != '\0'; p++, i++)
    {
        if (isalnum(*p) || strchr("/$-_.+!*'()", *p))
            dest[i] = *p;
        else
        {
            snprintf(&(dest[i]), 4, "%%%02X", (unsigned char) *p);
            i += 2;
        }
    }

    return eina_stringshare_add(dest);
}

EAPI void
efreet_uri_free(Efreet_Uri *uri)
{
    if (!uri) return;

    IF_RELEASE(uri->protocol);
    IF_RELEASE(uri->path);
    IF_RELEASE(uri->hostname);
    FREE(uri);
}
