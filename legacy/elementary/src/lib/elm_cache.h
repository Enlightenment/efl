/**
 * @defgroup Caches Caches
 * @ingroup Elementary
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
EAPI void      elm_cache_all_flush(void);

/**
 * @}
 */
