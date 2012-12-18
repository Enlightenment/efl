#ifndef EINA_MMAP_H_
#define EINA_MMAP_H_

/**
 * @addtogroup Eina_Mmap_Group Mmap Group
 * @ingroup Eina
 *
 * @brief These functions provide helpers for safe mmap handling
 *
 * @{
 *
 * @since 1.1.0
 */

/**
 * @brief Enable or disable safe mmaped IO handling
 *
 * @param enabled The enabled state (to enable, pass #EINA_TRUE)
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This enables (if possible on your platform) a signal handler for
 * SIGBUS, that replaces the "bad page" with a page of 0's (from /dev/zero)
 * if a SIGBUS occurs. This allows for safe mmap() of files that may truncate
 * or from files on devices with IO errors. Normally these cases will result
 * in a SIGBUS being delivered (and termination of your process), but
 * when "mmap safety" is enabled, this will not occur. Instead a page of
 * bytes of the value 0 will replace the "bad page", allowing the process
 * to continue and allow its own parsing error detection to safely abort
 * the operation without the process falling apart.
 *
 * If you disable mmap safety, the SIGBUS handler will be restored to its
 * default handler. Note that eina_file_map_all() and eina_file_map_new()
 * will automatically enable mmap safety as they provide an mmaped file IO
 * layer, and rely on mmap to not fail for any part of the file.
 *
 * If you set up your own SIGBUS handler, then this will effectively disable
 * the safe mmap handling and make you liable to crashes on IO to or from
 * such "damaged files" that would take down your process.
 *
 * @since 1.1.0
 */
EAPI Eina_Bool
eina_mmap_safety_enabled_set(Eina_Bool enabled);

/**
 * @brief Get the enabled state of mmap safety.
 *
 * @return The safety state (#EINA_TRUE if enabled)
 *
 * This returns the mmap safety state set by eina_mmap_safety_enabled_set().
 * See eina_mmap_safety_enabled_set() for more information.
 *
 * @since 1.1.0
 */
EAPI Eina_Bool
eina_mmap_safety_enabled_get(void);

/**
 * @}
 */
#endif
