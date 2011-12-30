/**
 * @defgroup Caches Caches
 *
 * These are functions which let one fine-tune some cache values for
 * Elementary applications, thus allowing for performance adjustments.
 *
 * @{
 */

/**
 * @brief Flush all caches.
 *
 * Frees all data that was in cache and is not currently being used to reduce
 * memory usage. This frees Edje's, Evas' and Eet's cache. This is equivalent
 * to calling all of the following functions:
 * @li edje_file_cache_flush()
 * @li edje_collection_cache_flush()
 * @li eet_clearcache()
 * @li evas_image_cache_flush()
 * @li evas_font_cache_flush()
 * @li evas_render_dump()
 * @note Evas caches are flushed for every canvas associated with a window.
 *
 * @ingroup Caches
 */
// XXX: rename: elm_cache_all_flush()
EAPI void      elm_all_flush(void);

/**
 * Get the configured cache flush interval time
 *
 * This gets the globally configured cache flush interval time, in
 * ticks
 *
 * @return The cache flush interval time
 * @ingroup Caches
 *
 * @see elm_all_flush()
 */
EAPI int       elm_cache_flush_interval_get(void);

/**
 * Set the configured cache flush interval time
 *
 * This sets the globally configured cache flush interval time, in ticks
 *
 * @param size The cache flush interval time
 * @ingroup Caches
 *
 * @see elm_all_flush()
 */
EAPI void      elm_cache_flush_interval_set(int size);

/**
 * Set the configured cache flush interval time for all applications on the
 * display
 *
 * This sets the globally configured cache flush interval time -- in ticks
 * -- for all applications on the display.
 *
 * @param size The cache flush interval time
 * @ingroup Caches
 */
// XXX: deprecate and replace with elm_config_all_flush()
EAPI void      elm_cache_flush_interval_all_set(int size);

/**
 * Get the configured cache flush enabled state
 *
 * This gets the globally configured cache flush state - if it is enabled
 * or not. When cache flushing is enabled, elementary will regularly
 * (see elm_cache_flush_interval_get() ) flush caches and dump data out of
 * memory and allow usage to re-seed caches and data in memory where it
 * can do so. An idle application will thus minimise its memory usage as
 * data will be freed from memory and not be re-loaded as it is idle and
 * not rendering or doing anything graphically right now.
 *
 * @return The cache flush state
 * @ingroup Caches
 *
 * @see elm_all_flush()
 */
EAPI Eina_Bool elm_cache_flush_enabled_get(void);

/**
 * Set the configured cache flush enabled state
 *
 * This sets the globally configured cache flush enabled state.
 *
 * @param size The cache flush enabled state
 * @ingroup Caches
 *
 * @see elm_all_flush()
 */
EAPI void      elm_cache_flush_enabled_set(Eina_Bool enabled);

/**
 * Set the configured cache flush enabled state for all applications on the
 * display
 *
 * This sets the globally configured cache flush enabled state for all
 * applications on the display.
 *
 * @param size The cache flush enabled state
 * @ingroup Caches
 */
// XXX: deprecate and replace with elm_config_all_flush()
EAPI void      elm_cache_flush_enabled_all_set(Eina_Bool enabled);

/**
 * Get the configured font cache size
 *
 * This gets the globally configured font cache size, in bytes.
 *
 * @return The font cache size
 * @ingroup Caches
 */
EAPI int       elm_font_cache_get(void);

/**
 * Set the configured font cache size
 *
 * This sets the globally configured font cache size, in bytes
 *
 * @param size The font cache size
 * @ingroup Caches
 */
EAPI void      elm_font_cache_set(int size);

/**
 * Set the configured font cache size for all applications on the
 * display
 *
 * This sets the globally configured font cache size -- in bytes
 * -- for all applications on the display.
 *
 * @param size The font cache size
 * @ingroup Caches
 */
// XXX: deprecate and replace with elm_config_all_flush()
EAPI void      elm_font_cache_all_set(int size);

/**
 * Get the configured image cache size
 *
 * This gets the globally configured image cache size, in bytes
 *
 * @return The image cache size
 * @ingroup Caches
 */
EAPI int       elm_image_cache_get(void);

/**
 * Set the configured image cache size
 *
 * This sets the globally configured image cache size, in bytes
 *
 * @param size The image cache size
 * @ingroup Caches
 */
EAPI void      elm_image_cache_set(int size);

/**
 * Set the configured image cache size for all applications on the
 * display
 *
 * This sets the globally configured image cache size -- in bytes
 * -- for all applications on the display.
 *
 * @param size The image cache size
 * @ingroup Caches
 */
// XXX: deprecate and replace with elm_config_all_flush()
EAPI void      elm_image_cache_all_set(int size);

/**
 * Get the configured edje file cache size.
 *
 * This gets the globally configured edje file cache size, in number
 * of files.
 *
 * @return The edje file cache size
 * @ingroup Caches
 */
EAPI int       elm_edje_file_cache_get(void);

/**
 * Set the configured edje file cache size
 *
 * This sets the globally configured edje file cache size, in number
 * of files.
 *
 * @param size The edje file cache size
 * @ingroup Caches
 */
EAPI void      elm_edje_file_cache_set(int size);

/**
 * Set the configured edje file cache size for all applications on the
 * display
 *
 * This sets the globally configured edje file cache size -- in number
 * of files -- for all applications on the display.
 *
 * @param size The edje file cache size
 * @ingroup Caches
 */
// XXX: deprecate and replace with elm_config_all_flush()
EAPI void      elm_edje_file_cache_all_set(int size);

/**
 * Get the configured edje collections (groups) cache size.
 *
 * This gets the globally configured edje collections cache size, in
 * number of collections.
 *
 * @return The edje collections cache size
 * @ingroup Caches
 */
EAPI int       elm_edje_collection_cache_get(void);

/**
 * Set the configured edje collections (groups) cache size
 *
 * This sets the globally configured edje collections cache size, in
 * number of collections.
 *
 * @param size The edje collections cache size
 * @ingroup Caches
 */
EAPI void      elm_edje_collection_cache_set(int size);

/**
 * Set the configured edje collections (groups) cache size for all
 * applications on the display
 *
 * This sets the globally configured edje collections cache size -- in
 * number of collections -- for all applications on the display.
 *
 * @param size The edje collections cache size
 * @ingroup Caches
 */
// XXX: deprecate and replace with elm_config_all_flush()
EAPI void      elm_edje_collection_cache_all_set(int size);

/**
 * @}
 */
