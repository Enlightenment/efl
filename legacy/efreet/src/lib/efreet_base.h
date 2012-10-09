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
 * @brief Returns the Eina_List of preference ordered extra data directories
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
 * @return Returns the XDG Desktop directory
 * @brief Retrieves the XDG Desktop directory
 * @since 1.3
 */
EAPI const char *efreet_desktop_dir_get(void);

/**
 * @return Returns the XDG Download directory
 * @brief Retrieves the XDG Download directory
 * @since 1.8
 */
EAPI const char *efreet_download_dir_get(void);

/**
 * @return Returns the XDG Templates directory
 * @brief Retrieves the XDG Templates directory
 * @since 1.8
 */
EAPI const char *efreet_templates_dir_get(void);

/**
 * @return Returns the XDG Public Share directory
 * @brief Retrieves the XDG Public Share directory
 * @since 1.8
 */
EAPI const char *efreet_public_share_dir_get(void);

/**
 * @return Returns the XDG Documents directory
 * @brief Retrieves the XDG Documents directory
 * @since 1.8
 */
EAPI const char *efreet_documents_dir_get(void);

/**
 * @return Returns the XDG Music directory
 * @brief Retrieves the XDG Music directory
 * @since 1.8
 */
EAPI const char *efreet_music_dir_get(void);

/**
 * @return Returns the XDG Pictures directory
 * @brief Retrieves the XDG Pictures directory
 * @since 1.8
 */
EAPI const char *efreet_pictures_dir_get(void);

/**
 * @return Returns the XDG Videos directory
 * @brief Retrieves the XDG Videos directory
 * @since 1.8
 */
EAPI const char *efreet_videos_dir_get(void);

/**
 * @return Returns the Eina_List of preference ordered extra config directories
 * @brief Returns the Eina_List of preference ordered extra config
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
 * @return Returns the XDG User Runtime directory.
 * @brief Retrieves the XDG User Runtime directory.
 * @since 1.8
 */
EAPI const char *efreet_runtime_dir_get(void);

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
