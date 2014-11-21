#ifndef EFREET_URI_H
#define EFREET_URI_H

/**
 * @file efreet_uri.h
 * @brief Contains the methods used to support the FDO URI specification.
 *
 * @internal
 * @defgroup Efreet_Uri_Group Efreet_Uri: The FDO URI Specification functions
 * @ingroup Efreet_Group
 *
 * @{
 */


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



/**
 * @brief Get the string rapresentation of the given uri struct escaping
 * illegal caracters. Remember to free the string with eina_stringshare_del()
 * when you don't need it anymore.
 *
 * @param[in] uri Create an URI string from an Efreet_Uri struct
 * @return The string rapresentation of uri (ex: 'file:///home/my%20name')
 * @note The resulting string will contain the protocol and the path but not
 * the hostname, as many apps doesn't handle it.
 */
EAPI const char *efreet_uri_encode(Efreet_Uri *uri);

/**
 * @brief Read a single uri and return an Efreet_Uri struct. If there's no
 * hostname in the uri then the hostname parameter will be NULL. All the uri
 * escaped chars will be converted to normal.
 *
 * @param[in] val a valid uri string to parse
 * @return Return The corresponding Efreet_Uri structure. Or NULL on errors.
 */
EAPI Efreet_Uri *efreet_uri_decode(const char *val);

/**
 * @brief Free the given uri structure.
 *
 * @param[in] uri The uri to free
 */
EAPI void        efreet_uri_free(Efreet_Uri *uri);


/**
 * @}
 */

#endif
