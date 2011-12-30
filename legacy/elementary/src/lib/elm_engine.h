/**
 * @defgroup Engine Elementary Engine
 *
 * These are functions setting and querying which rendering engine
 * Elementary will use for drawing its windows' pixels.
 *
 * The following are the available engines:
 * @li "software_x11"
 * @li "fb"
 * @li "directfb"
 * @li "software_16_x11"
 * @li "software_8_x11"
 * @li "xrender_x11"
 * @li "opengl_x11"
 * @li "software_gdi"
 * @li "software_16_wince_gdi"
 * @li "sdl"
 * @li "software_16_sdl"
 * @li "opengl_sdl"
 * @li "buffer"
 * @li "ews"
 * @li "opengl_cocoa"
 * @li "psl1ght"
 *
 * @{
 */

/**
 * @brief Get Elementary's rendering engine in use.
 *
 * @return The rendering engine's name
 * @note there's no need to free the returned string, here.
 *
 * This gets the global rendering engine that is applied to all Elementary
 * applications.
 *
 * @see elm_engine_set()
 */
// XXX: rename to elm_engine_get()
EAPI const char *elm_engine_current_get(void);

/**
 * @brief Set Elementary's rendering engine for use.
 *
 * @param engine The rendering engine's name
 *
 * This sets global rendering engine that is applied to all Elementary
 * applications. Note that it will take effect only to Elementary windows
 * created after this is called.
 *
 * @see elm_win_add()
 */
EAPI void        elm_engine_set(const char *engine);

/**
 * @}
 */
