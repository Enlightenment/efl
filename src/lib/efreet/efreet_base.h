#ifndef EFREET_BASE_H
#define EFREET_BASE_H
/**
 * @file efreet_base.h
 * @brief Contains the methods used to support the FDO base directory
 * specification.
 * @addtogroup Efreet_Base Efreet_Base: The XDG Base Directory Specification
 * functions
 * @ingroup Efreet
 *
 * @see http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html
 * @see http://0pointer.de/blog/projects/tmp.html
 *
 * @{
 */


/**
 * @return Returns the XDG Data Home directory
 * @brief Retrieves the XDG Data Home directory
 *
 * This is where user-specific data files should be written
 * ($XDG_DATA_HOME or $HOME/.local/share).
 */
EAPI const char *efreet_data_home_get(void);

/**
 * @return Returns the Eina_List of preference ordered extra data directories
 * @brief Returns the Eina_List of preference ordered extra data directories
 *
 * Ordered base directories relative to which data files should be
 * searched ($XDG_DATA_DIRS or /usr/local/share/:/usr/share/)
 *
 * @note The returned list is static inside Efreet. If you add/remove from the
 * list then the next call to efreet_data_dirs_get() will return your
 * modified values. DO NOT free this list.
 */
EAPI Eina_List *efreet_data_dirs_get(void);


/**
 * @return Returns the XDG Config Home directory
 * @brief Retrieves the XDG Config Home directory
 *
 * This is where user-specific configuration files should be
 * written ($XDG_CONFIG_HOME or $HOME/.config).
 */
EAPI const char *efreet_config_home_get(void);

/**
 * @return Returns the XDG Desktop directory
 * @brief Retrieves the XDG Desktop directory
 *
 * This is where user-specific desktop files should be located. It's
 * localized (translated) to current user locale
 * (~/.config/user-dirs.locale and ~/.config/user-dirs.dirs).
 *
 * @since 1.3
 */
EAPI const char *efreet_desktop_dir_get(void);

/**
 * @return Returns the XDG Download directory
 * @brief Retrieves the XDG Download directory
 *
 * This is where user-specific download files should be located. It's
 * localized (translated) to current user locale
 * (~/.config/user-dirs.locale and ~/.config/user-dirs.dirs).
 *
 * It's meant for large download or in-progress downloads, it's
 * private for the user but may be shared among different
 * machines. It's not automatically cleaned up.
 *
 * @see efreet_cache_home_get()
 * @see efreet_runtime_dir_get()
 * @see http://0pointer.de/blog/projects/tmp.html
 *
 * @since 1.8
 */
EAPI const char *efreet_download_dir_get(void);

/**
 * @return Returns the XDG Templates directory
 * @brief Retrieves the XDG Templates directory
 *
 * This is where user-specific template files should be located. It's
 * localized (translated) to current user locale
 * (~/.config/user-dirs.locale and ~/.config/user-dirs.dirs).
 *
 * @see efreet_documents_dir_get()
 * @see efreet_music_dir_get()
 * @see efreet_pictures_dir_get()
 * @see efreet_videos_dir_get()
 *
 * @since 1.8
 */
EAPI const char *efreet_templates_dir_get(void);

/**
 * @return Returns the XDG Public Share directory
 * @brief Retrieves the XDG Public Share directory
 *
 * This is where user-specific public shareable files should be
 * located. It's localized (translated) to current user locale
 * (~/.config/user-dirs.locale and ~/.config/user-dirs.dirs).
 *
 * Usually local file servers should look here (Apache, Samba, FTP).
 *
 * @since 1.8
 */
EAPI const char *efreet_public_share_dir_get(void);

/**
 * @return Returns the XDG Documents directory
 * @brief Retrieves the XDG Documents directory
 *
 * This is where user-specific documents files should be located. It's
 * localized (translated) to current user locale
 * (~/.config/user-dirs.locale and ~/.config/user-dirs.dirs).
 *
 * @see efreet_templates_dir_get()
 * @see efreet_music_dir_get()
 * @see efreet_pictures_dir_get()
 * @see efreet_videos_dir_get()
 *
 * @since 1.8
 */
EAPI const char *efreet_documents_dir_get(void);

/**
 * @return Returns the XDG Music directory
 * @brief Retrieves the XDG Music directory
 *
 * This is where user-specific music files should be located. It's
 * localized (translated) to current user locale
 * (~/.config/user-dirs.locale and ~/.config/user-dirs.dirs).
 *
 * @see efreet_templates_dir_get()
 * @see efreet_download_dir_get()
 * @see efreet_pictures_dir_get()
 * @see efreet_videos_dir_get()
 *
 * @since 1.8
 */
EAPI const char *efreet_music_dir_get(void);

/**
 * @return Returns the XDG Pictures directory
 * @brief Retrieves the XDG Pictures directory
 *
 * This is where user-specific pictures files should be located. It's
 * localized (translated) to current user locale
 * (~/.config/user-dirs.locale and ~/.config/user-dirs.dirs).
 *
 * @see efreet_templates_dir_get()
 * @see efreet_documents_dir_get()
 * @see efreet_music_dir_get()
 * @see efreet_videos_dir_get()
 *
 * @since 1.8
 */
EAPI const char *efreet_pictures_dir_get(void);

/**
 * @return Returns the XDG Videos directory
 * @brief Retrieves the XDG Videos directory
 *
 * This is where user-specific video files should be located. It's
 * localized (translated) to current user locale
 * (~/.config/user-dirs.locale and ~/.config/user-dirs.dirs).
 *
 * @see efreet_templates_dir_get()
 * @see efreet_documents_dir_get()
 * @see efreet_music_dir_get()
 * @see efreet_pictures_dir_get()
 *
 * @since 1.8
 */
EAPI const char *efreet_videos_dir_get(void);

/**
 * @return Returns the Eina_List of preference ordered extra config directories
 * @brief Returns the Eina_List of preference ordered extra config
 * directories
 *
 * Ordered base directories relative to which configuration files
 * should be searched ($XDG_CONFIG_DIRS or /etc/xdg)
 *
 * @note The returned list is static inside Efreet. If you add/remove from the
 * list then the next call to efreet_config_dirs_get() will return your
 * modified values. DO NOT free this list.
 */
EAPI Eina_List *efreet_config_dirs_get(void);


/**
 * @return Returns the XDG Cache Home directory
 * @brief Retrieves the XDG Cache Home directory
 *
 * This is the base directory relative to which user specific
 * non-essential data files should be stored ($XDG_CACHE_HOME or
 * $HOME/.cache).
 *
 * @see efreet_runtime_dir_get()
 * @see efreet_download_dir_get()
 * @see efreet_config_home_get()
 */
EAPI const char *efreet_cache_home_get(void);

/**
 * @return Returns the XDG User Runtime directory.
 * @brief Retrieves the XDG User Runtime directory.
 *
 * This is the base directory relative to which user-specific
 * non-essential runtime files and other file objects (such as
 * sockets, named pipes, ...) should be stored. The directory @b must
 * be owned by the user, and he @b must be the only one having read
 * and write access to it. Its Unix access mode @b must be 0700.
 *
 * The lifetime of this directory @b must be bound to the user being
 * logged in. It @b must be created when the user first logs in and if
 * the user fully logs out the directory @b must be removed. If the
 * user logs in more than once he should get pointed to the same
 * directory, and it is mandatory that the directory continues to
 * exist from his first login to his last logout on the system, and
 * not removed in between. Files in the directory @b must not survive
 * reboot or a full logout/login cycle.
 *
 * The directory @b must be on a local file system and not shared with
 * any other system. The directory @b must by fully-featured by the
 * standards of the operating system. More specifically, on Unix-like
 * operating systems AF_UNIX sockets, symbolic links, hard links,
 * proper permissions, file locking, sparse files, memory mapping,
 * file change notifications, a reliable hard link count must be
 * supported, and no restrictions on the file name character set
 * should be imposed. Files in this directory @b may be subjected to
 * periodic clean-up. To ensure that your files are not removed, they
 * should have their access time timestamp modified at least once
 * every 6 hours of monotonic time or the 'sticky' bit should be set
 * on the file.
 *
 * If @c NULL applications should fall back to a replacement directory
 * with similar capabilities and print a warning message. Applications
 * should use this directory for communication and synchronization
 * purposes and should not place larger files in it, since it might
 * reside in runtime memory and cannot necessarily be swapped out to
 * disk.
 *
 * @note this directory is similar to @c /run and is often created in
 * tmpfs (memory-only/RAM filesystem). It is created, managed and
 * cleaned automatically by systemd.
 *
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
    int error;
};

/**
 * @}
 */

#endif
