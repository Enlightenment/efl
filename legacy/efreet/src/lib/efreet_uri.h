/* vim: set sw=4 ts=4 sts=4 et: */
#ifndef EFREET_URI_H
#define EFREET_URI_H

/**
 * @file efreet_uri.h
 * @brief Contains the methods used to support the FDO URI specification.
 * @addtogroup Efreet_Uri Efreet_Uri: The FDO URI Specification functions
 * @{
 */

#include <Ecore.h>


/**
 * Efreet_Uri
 */
typedef struct Efreet_Uri Efreet_Uri;

/**
 * Efreet_Uri
 * @brief Contains a simple rappresentation of an uri. The string don't have 
 * special chars escaped.
 */
struct Efreet_Uri
{
    const char *protocol;   /**< The protocol used (usually 'file')*/
    const char *hostname;   /**< The name of the host if any, or NULL */
    const char *path;       /**< The full file path whitout protocol nor host*/
};


EAPI const char *efreet_uri_encode(Efreet_Uri *uri);
EAPI Efreet_Uri *efreet_uri_decode(const char *val);
EAPI void        efreet_uri_free(Efreet_Uri *uri);


/**
 * @}
 */

#endif
