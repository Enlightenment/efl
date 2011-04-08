#ifndef EFREET_BASE_H
#define EFREET_BASE_H
/**
 * @file efreet_base.h
 * @brief Contains the methods used to support the FDO base directory
 * specification.
 * @addtogroup Efreet_Base Efreet_Base: The XDG Base Directory Specification
 * functions
 *
 * @{
 */


/**
 * @return Returns the XDG Data Home directory
 * @brief Retrieves the XDG Data Home directory
 */
EAPI const char *efreet_data_home_get(void);

/**
 * @return Returns the Eina_List of preference ordered extra data directories
 * @brief Returns the Eina_List of prefernece oredred extra data
 * directories
 *
 * @note The returned list is static inside Efreet. If you add/remove from the
 * list then the next call to efreet_data_dirs_get() will return your
 * modified values. DO NOT free this list.
 */
EAPI Eina_List *efreet_data_dirs_get(void);


/**
 * @return Returns the XDG Config Home directory
 * @brief Retrieves the XDG Config Home directory
 */
EAPI const char *efreet_config_home_get(void);

/**
 * @return Returns the Eina_List of preference ordered extra config directories
 * @brief Returns the Eina_List of prefernece ordered extra config
 * directories
 *
 * @note The returned list is static inside Efreet. If you add/remove from the
 * list then the next call to efreet_config_dirs_get() will return your
 * modified values. DO NOT free this list.
 */
EAPI Eina_List *efreet_config_dirs_get(void);


/**
 * @return Returns the XDG Cache Home directory
 * @brief Retrieves the XDG Cache Home directory
 */
EAPI const char *efreet_cache_home_get(void);

/**
 * @return Returns the current hostname
 * @brief Returns the current hostname or empty string if not found
 */
EAPI const char *efreet_hostname_get(void);

/**
 * Efreet_Event_Cache_Update
 */
typedef struct _Efreet_Event_Cache_Update Efreet_Event_Cache_Update;

/**
 * Efreet_Event_Cache_Update
 * @brief event struct sent with EFREET_EVENT_*_CACHE_UPDATE
 */
struct _Efreet_Event_Cache_Update
{
    int dummy;
};

/**
 * @}
 */

#endif
