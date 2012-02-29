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
EAPI const char *elm_engine_get(void);

/**
 * @brief Set Elementary's rendering engine for use.
 *
 * @param engine The rendering engine's name
 *
 * Note that it will take effect only to Elementary windows created after
 * this is called.
 *
 * @see elm_win_add()
 */
EAPI void        elm_engine_set(const char *engine);

/**
 * @brief Get Elementary's preferred engine to use.
 *
 * @return The rendering engine's name
 * @note there's no need to free the returned string, here.
 *
 * This gets the global rendering engine that is applied to all Elementary
 * applications and is PREFERRED by the application. This can (and will)
 * override the engine configured for all applications which.
 *
 * @see elm_preferred_engine_set()
 */
EAPI const char *elm_preferred_engine_get(void);

/**
 * @brief Set Elementary's preferred rendering engine for use.
 *
 * @param engine The rendering engine's name
 *
 * Note that it will take effect only to Elementary windows created after
 * this is called. This overrides the engine set by configuration at
 * application startup. Note that it is a hint and may not be honored.
 *
 * @see elm_win_add()
 */
EAPI void        elm_preferred_engine_set(const char *engine);

/**
 * @}
 */
