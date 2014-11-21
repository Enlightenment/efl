#ifndef EINA_MMAP_H_
#define EINA_MMAP_H_

/**
 * @internal
 * @defgroup Eina_Mmap_Group Mmap Group
 * @ingroup Eina_Core_Group
 *
 * @brief This group discusses the functions that provide helpers for safe mmap handling.
 *
 * @{
 *
 * @since 1.1.0
 */

/**
 * @brief Enables or disables safe mmaped IO handling.
 *
 * @details This enables (if possible on your platform) a signal handler for
 *          SIGBUS, that replaces the "bad page" with a page of 0's (from /dev/zero)
 *          if a SIGBUS occurs. This allows for safe mmap() of files that may truncate
 *          or from files on devices with IO errors. Normally these cases result
 *          in a SIGBUS being delivered (and termination of your process), but
 *          when "mmap safety" is enabled, this does not occur. Instead a page of
 *          bytes of the value 0 replaces the "bad page", allowing the process
 *          to continue and allow its own parsing error detection to safely abort
 *          the operation without the process falling apart.
 *
 * @since 1.1.0
 *
 * @since_tizen 2.3
 *
 * @remarks If you disable mmap safety, the SIGBUS handler is restored to its
 *          default handler. Note that eina_file_map_all() and eina_file_map_new()
 *          automatically enable mmap safety as they provide an mmaped file IO
 *          layer, and rely on mmap to not fail for any part of the file.
 *
 * @remarks If you set up your own SIGBUS handler, then this effectively disables
 *          the safe mmap handling and makes you liable to crashes on IO to or from
 *          such "damaged files" that would take down your process.
 *
 * @param[in] enabled The enabled state (to enable, pass @c EINA_TRUE)
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 */
EAPI Eina_Bool
eina_mmap_safety_enabled_set(Eina_Bool enabled);

/**
 * @brief Gets the enabled state of mmap safety.
 *
 * @details This returns the mmap safety state set by eina_mmap_safety_enabled_set().
 *
 * @since 1.1.0
 *
 * @since_tizen 2.3
 *
 * @return The safety state (@c EINA_TRUE if enabled)
 * 
 * @see eina_mmap_safety_enabled_set()
 */
EAPI Eina_Bool
eina_mmap_safety_enabled_get(void);

/**
 * @}
 */
#endif
