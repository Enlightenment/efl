/**
 * @defgroup Config Elementary Config
 * @ingroup Elementary
 *
 * Elementary configuration is formed by a set options bounded to a
 * given @ref Profile profile, like @ref Theme theme, @ref Fingers
 * "finger size", etc. These are functions with which one synchronizes
 * changes made to those values to the configuration storing files, de
 * facto. You most probably don't want to use the functions in this
 * group unless you're writing an elementary configuration manager.
 *
 * @{
 */

/**
 * Save back Elementary's configuration, so that it will persist on
 * future sessions.
 *
 * @return @c EINA_TRUE, when successful. @c EINA_FALSE, otherwise.
 * @ingroup Config
 *
 * This function will take effect -- thus, do I/O -- immediately. Use
 * it when you want to save all configuration changes at once. The
 * current configuration set will get saved onto the current profile
 * configuration file.
 *
 */
EAPI Eina_Bool elm_config_save(void);

/**
 * Reload Elementary's configuration, bounded to current selected
 * profile.
 *
 * @return @c EINA_TRUE, when successful. @c EINA_FALSE, otherwise.
 * @ingroup Config
 *
 * Useful when you want to force reloading of configuration values for
 * a profile. If one removes user custom configuration directories,
 * for example, it will force a reload with system values instead.
 *
 */
EAPI void      elm_config_reload(void);

/**
 * Flush all config settings then apply those settings to all applications
 * using elementary on the current display.
 *
 * @ingroup Config
 */
EAPI void      elm_config_all_flush(void);

/**
 * @}
 */

/**
 * @defgroup Profile Elementary Profile
 * @ingroup Elementary
 *
 * Profiles are pre-set options that affect the whole look-and-feel of
 * Elementary-based applications. There are, for example, profiles
 * aimed at desktop computer applications and others aimed at mobile,
 * touchscreen-based ones. You most probably don't want to use the
 * functions in this group unless you're writing an elementary
 * configuration manager.
 *
 * @{
 */

/**
 * Get Elementary's profile in use.
 *
 * This gets the global profile that is applied to all Elementary
 * applications.
 *
 * @return The profile's name
 * @ingroup Profile
 */
EAPI const char *elm_config_profile_get(void);

/**
 * Get an Elementary's profile directory path in the filesystem. One
 * may want to fetch a system profile's dir or a user one (fetched
 * inside $HOME).
 *
 * @param profile The profile's name
 * @param is_user Whether to lookup for a user profile (@c EINA_TRUE)
 *                or a system one (@c EINA_FALSE)
 * @return The profile's directory path.
 * @ingroup Profile
 *
 * @note You must free it with elm_config_profile_dir_free().
 */
EAPI const char *elm_config_profile_dir_get(const char *profile, Eina_Bool is_user);

/**
 * Free an Elementary's profile directory path, as returned by
 * elm_config_profile_dir_get().
 *
 * @param p_dir The profile's path
 * @ingroup Profile
 *
 */
EAPI void        elm_config_profile_dir_free(const char *p_dir);

/**
 * Get Elementary's list of available profiles.
 *
 * @return The profiles list. List node data are the profile name
 *         strings.
 * @ingroup Profile
 *
 * @note One must free this list, after usage, with the function
 *       elm_config_profile_list_free().
 */
EAPI Eina_List  *elm_config_profile_list_get(void);

/**
 * Free Elementary's list of available profiles.
 *
 * @param l The profiles list, as returned by elm_config_profile_list_get().
 * @ingroup Profile
 *
 */
EAPI void        elm_config_profile_list_free(Eina_List *l);

/**
 * Set Elementary's profile.
 *
 * This sets the global profile that is applied to Elementary
 * applications. Just the process the call comes from will be
 * affected.
 *
 * @param profile The profile's name
 * @ingroup Profile
 *
 */
EAPI void        elm_config_profile_set(const char *profile);

/**
 * @}
 */

/**
 * @defgroup Scrolling Elementary Scrolling
 * @ingroup Elementary
 *
 * These are functions setting how scrollable views in Elementary
 * widgets should behave on user interaction.
 *
 * @{
 */

/**
 * Get whether scrollers should bounce when they reach their
 * viewport's edge during a scroll.
 *
 * @return the thumb scroll bouncing state
 *
 * This is the default behavior for touch screens, in general.
 * @ingroup Scrolling
 */
EAPI Eina_Bool    elm_config_scroll_bounce_enabled_get(void);

/**
 * Set whether scrollers should bounce when they reach their
 * viewport's edge during a scroll.
 *
 * @param enabled the thumb scroll bouncing state
 *
 * @see elm_config_scroll_bounce_enabled_get()
 * @ingroup Scrolling
 */
EAPI void         elm_config_scroll_bounce_enabled_set(Eina_Bool enabled);

/**
 * Get the amount of inertia a scroller will impose at bounce
 * animations.
 *
 * @return the thumb scroll bounce friction
 *
 * @ingroup Scrolling
 */
EAPI double       elm_config_scroll_bounce_friction_get(void);

/**
 * Set the amount of inertia a scroller will impose at bounce
 * animations.
 *
 * @param friction the thumb scroll bounce friction
 *
 * @see elm_config_scroll_bounce_friction_get()
 * @ingroup Scrolling
 */
EAPI void         elm_config_scroll_bounce_friction_set(double friction);

/**
 * Get the amount of inertia a <b>paged</b> scroller will impose at
 * page fitting animations.
 *
 * @return the page scroll friction
 *
 * @ingroup Scrolling
 */
EAPI double       elm_config_scroll_page_scroll_friction_get(void);

/**
 * Set the amount of inertia a <b>paged</b> scroller will impose at
 * page fitting animations.
 *
 * @param friction the page scroll friction
 *
 * @see elm_config_scroll_page_scroll_friction_get()
 * @ingroup Scrolling
 */
EAPI void         elm_config_scroll_page_scroll_friction_set(double friction);

/**
 * Get the amount of inertia a scroller will impose at region bring
 * animations.
 *
 * @return the bring in scroll friction
 *
 * @ingroup Scrolling
 */
EAPI double       elm_config_scroll_bring_in_scroll_friction_get(void);

/**
 * Set the amount of inertia a scroller will impose at region bring
 * animations.
 *
 * @param friction the bring in scroll friction
 *
 * @see elm_config_scroll_bring_in_scroll_friction_get()
 * @ingroup Scrolling
 */
EAPI void         elm_config_scroll_bring_in_scroll_friction_set(double friction);

/**
 * Get the amount of inertia scrollers will impose at animations
 * triggered by Elementary widgets' zooming API.
 *
 * @return the zoom friction
 *
 * @ingroup Scrolling
 */
EAPI double       elm_config_scroll_zoom_friction_get(void);

/**
 * Set the amount of inertia scrollers will impose at animations
 * triggered by Elementary widgets' zooming API.
 *
 * @param friction the zoom friction
 *
 * @see elm_config_scroll_zoom_friction_get()
 * @ingroup Scrolling
 */
EAPI void         elm_config_scroll_zoom_friction_set(double friction);

/**
 * Get whether scrollers should be draggable from any point in their
 * views.
 *
 * @return the thumb scroll state
 *
 * @note This is the default behavior for touch screens, in general.
 * @note All other functions namespaced with "thumbscroll" will only
 *       have effect if this mode is enabled.
 *
 * @ingroup Scrolling
 */
EAPI Eina_Bool    elm_config_scroll_thumbscroll_enabled_get(void);

/**
 * Set whether scrollers should be draggable from any point in their
 * views.
 *
 * @param enabled the thumb scroll state
 *
 * @see elm_config_scroll_thumbscroll_enabled_get()
 * @ingroup Scrolling
 */
EAPI void         elm_config_scroll_thumbscroll_enabled_set(Eina_Bool enabled);

/**
 * Get the number of pixels one should travel while dragging a
 * scroller's view to actually trigger scrolling.
 *
 * @return the thumb scroll threshold
 *
 * One would use higher values for touch screens, in general, because
 * of their inherent imprecision.
 * @ingroup Scrolling
 */
EAPI unsigned int elm_config_scroll_thumbscroll_threshold_get(void);

/**
 * Set the number of pixels one should travel while dragging a
 * scroller's view to actually trigger scrolling.
 *
 * @param threshold the thumb scroll threshold
 *
 * @see elm_config_thumbscroll_threshold_get()
 * @ingroup Scrolling
 */
EAPI void         elm_config_scroll_thumbscroll_threshold_set(unsigned int threshold);

/**
 * Get the number of pixels the range which can be scrolled,
 * while the scroller is holded.
 *
 * @return the thumb scroll hold threshold
 *
 * @ingroup Scrolling
 */
EAPI unsigned int elm_config_scroll_thumbscroll_hold_threshold_get(void);

/**
 * Set the number of pixels the range which can be scrolled,
 * while the scroller is holded.
 *
 * @param threshold the thumb scroll hold threshold
 *
 * @see elm_config_thumbscroll_hold_threshold_get()
 * @ingroup Scrolling
 */
EAPI void         elm_config_scroll_thumbscroll_hold_threshold_set(unsigned int threshold);

/**
 * Get the minimum speed of mouse cursor movement which will trigger
 * list self scrolling animation after a mouse up event
 * (pixels/second).
 *
 * @return the thumb scroll momentum threshold
 *
 * @ingroup Scrolling
 */
EAPI double       elm_config_scroll_thumbscroll_momentum_threshold_get(void);

/**
 * Set the minimum speed of mouse cursor movement which will trigger
 * list self scrolling animation after a mouse up event
 * (pixels/second).
 *
 * @param threshold the thumb scroll momentum threshold
 *
 * @see elm_config_thumbscroll_momentum_threshold_get()
 * @ingroup Scrolling
 */
EAPI void         elm_config_scroll_thumbscroll_momentum_threshold_set(double threshold);

/**
 * Get the number of pixels the maximum distance which can be flicked.
 * If it is flicked more than this,
 * the flick distance is same with maximum distance.
 *
 * @return the thumb scroll maximum flick distance
 *
 * @ingroup Scrolling
 */
EAPI unsigned int elm_config_scroll_thumbscroll_flick_distance_tolerance_get(void);

/**
 * Set the number of pixels the maximum distance which can be flicked.
 * If it is flicked more than this,
 * the flick distance is same with maximum distance.
 *
 * @param distance the thumb scroll maximum flick distance
 *
 * @see elm_config_thumbscroll_flick_distance_tolerance_get()
 * @ingroup Scrolling
 */
EAPI void         elm_config_scroll_thumbscroll_flick_distance_tolerance_set(unsigned int distance);

/**
 * Get the amount of inertia a scroller will impose at self scrolling
 * animations.
 *
 * @return the thumb scroll friction
 *
 * @ingroup Scrolling
 */
EAPI double       elm_config_scroll_thumbscroll_friction_get(void);

/**
 * Set the amount of inertia a scroller will impose at self scrolling
 * animations.
 *
 * @param friction the thumb scroll friction
 *
 * @see elm_config_thumbscroll_friction_get()
 * @ingroup Scrolling
 */
EAPI void         elm_config_scroll_thumbscroll_friction_set(double friction);

/**
 * Get the min amount of inertia a scroller will impose at self scrolling
 * animations.
 *
 * @return the thumb scroll min friction
 *
 * @ingroup Scrolling
 */
EAPI double       elm_config_scroll_thumbscroll_min_friction_get(void);

/**
 * Set the min amount of inertia a scroller will impose at self scrolling
 * animations.
 *
 * @param friction the thumb scroll min friction
 *
 * @see elm_config_thumbscroll_min_friction_get()
 * @ingroup Scrolling
 */
EAPI void         elm_config_scroll_thumbscroll_min_friction_set(double friction);

/**
 * Get the standard velocity of the scroller. The scroll animation time is
 * same with thumbscroll friction, if the velocity is same with standard
 * velocity.
 *
 * @return the thumb scroll friction
 *
 * @ingroup Scrolling
 */
EAPI double       elm_config_scroll_thumbscroll_friction_standard_get(void);

/**
 * Set the standard velocity of the scroller. The scroll animation time is
 * same with thumbscroll friction, if the velocity is same with standard
 * velocity.
 *
 * @param friction the thumb scroll friction standard
 *
 * @see elm_config_thumbscroll_friction_standard_get()
 * @ingroup Scrolling
 */
EAPI void         elm_config_scroll_thumbscroll_friction_standard_set(double standard);

/**
 * Get the amount of lag between your actual mouse cursor dragging
 * movement and a scroller's view movement itself, while pushing it
 * into bounce state manually.
 *
 * @return the thumb scroll border friction
 *
 * @ingroup Scrolling
 */
EAPI double       elm_config_scroll_thumbscroll_border_friction_get(void);

/**
 * Set the amount of lag between your actual mouse cursor dragging
 * movement and a scroller's view movement itself, while pushing it
 * into bounce state manually.
 *
 * @param friction the thumb scroll border friction. @c 0.0 for
 *        perfect synchrony between two movements, @c 1.0 for maximum
 *        lag.
 *
 * @see elm_config_thumbscroll_border_friction_get()
 * @note parameter value will get bound to 0.0 - 1.0 interval, always
 *
 * @ingroup Scrolling
 */
EAPI void         elm_config_scroll_thumbscroll_border_friction_set(double friction);

/**
 * Get the sensitivity amount which is be multiplied by the length of
 * mouse dragging.
 *
 * @return the thumb scroll sensitivity friction
 *
 * @ingroup Scrolling
 */
EAPI double       elm_config_scroll_thumbscroll_sensitivity_friction_get(void);

/**
 * Set the sensitivity amount which is be multiplied by the length of
 * mouse dragging.
 *
 * @param friction the thumb scroll sensitivity friction. @c 0.1 for
 *        minimum sensitivity, @c 1.0 for maximum sensitivity. 0.25
 *        is proper.
 *
 * @see elm_config_thumbscroll_sensitivity_friction_get()
 * @note parameter value will get bound to 0.1 - 1.0 interval, always
 *
 * @ingroup Scrolling
 */
EAPI void         elm_config_scroll_thumbscroll_sensitivity_friction_set(double friction);

/**
 * Get the minimum speed of mouse cursor movement which will accelerate
 * scrolling velocity after a mouse up event
 * (pixels/second).
 *
 * @return the thumb scroll acceleration threshold
 *
 * @ingroup Scrolling
 */
EAPI double       elm_config_scroll_thumbscroll_acceleration_threshold_get(void);

/**
 * Set the minimum speed of mouse cursor movement which will accelerate
 * scrolling velocity after a mouse up event
 * (pixels/second).
 *
 * @param threshold the thumb scroll acceleration threshold
 *
 * @see elm_config_thumbscroll_acceleration_threshold_get()
 * @ingroup Scrolling
 */
EAPI void         elm_config_scroll_thumbscroll_acceleration_threshold_set(double threshold);

/**
 * Get the time limit for accelerating velocity.
 *
 * @return the thumb scroll acceleration time limit
 *
 * @ingroup Scrolling
 */
EAPI double       elm_config_scroll_thumbscroll_acceleration_time_limit_get(void);

/**
 * Set the time limit for accelerating velocity.
 *
 * @param threshold the thumb scroll acceleration time limit
 *
 * @see elm_config_thumbscroll_acceleration_time_limit_get()
 * @ingroup Scrolling
 */
EAPI void         elm_config_scroll_thumbscroll_acceleration_time_limit_set(double time_limit);

/**
 * Get the weight for the acceleration.
 *
 * @return the thumb scroll acceleration weight
 *
 * @ingroup Scrolling
 */
EAPI double       elm_config_scroll_thumbscroll_acceleration_weight_get(void);

/**
 * Set the weight for the acceleration.
 *
 * @param threshold the thumb scroll acceleration weight
 *
 * @see elm_config_thumbscroll_acceleration_weight_get()
 * @ingroup Scrolling
 */
EAPI void         elm_config_scroll_thumbscroll_acceleration_weight_set(double weight);

/**
 * Focus Autoscroll Mode
 *
 * @since 1.10
 * @ingroup Focus
 */
typedef enum
{
   ELM_FOCUS_AUTOSCROLL_MODE_SHOW, /**< directly show the focused region or item automatically */
   ELM_FOCUS_AUTOSCROLL_MODE_NONE, /**< do not show the focused region or item automatically */
   ELM_FOCUS_AUTOSCROLL_MODE_BRING_IN /**< bring_in the focused region or item automatically which might invole the scrolling */
} Elm_Focus_Autoscroll_Mode;

/**
 * Get focus auto scroll mode.
 *
 * When a region or an item is focused and it resides inside any scroller,
 * elementary will automatically scroll the focused area to the visible
 * viewport.
 *
 * @see elm_config_focus_autoscroll_mode_set()
 * @ingroup Focus
 * @since 1.10
 */
EAPI Elm_Focus_Autoscroll_Mode elm_config_focus_autoscroll_mode_get(void);

/**
 * Set focus auto scroll mode.
 *
 * @param mode focus auto scroll mode. This can be one of the
 * Elm_Focus_Autoscroll_Mode enum values.
 *
 * When a region or an item is focused and it resides inside any scroller,
 * elementary will automatically scroll the focused area to the visible
 * viewport.
 * Focus auto scroll mode is set to @c ELM_FOCUS_AUTOSCROLL_MODE_SHOW by
 * default historically.
 *
 * @see elm_config_focus_autoscroll_mode_get()
 * @ingroup Focus
 * @since 1.10
 */
EAPI void         elm_config_focus_autoscroll_mode_set(Elm_Focus_Autoscroll_Mode mode);

/**
 * @}
 */

/**
 * Get the duration for occurring long press event.
 *
 * @return Timeout for long press event
 * @ingroup Longpress
 */
EAPI double       elm_config_longpress_timeout_get(void);

/**
 * Set the duration for occurring long press event.
 *
 * @param lonpress_timeout Timeout for long press event
 * @ingroup Longpress
 */
EAPI void         elm_config_longpress_timeout_set(double longpress_timeout);

typedef enum _Elm_Softcursor_Mode
{
   ELM_SOFTCURSOR_MODE_AUTO, /**< Auto-detect if a software cursor should be used (default) */
   ELM_SOFTCURSOR_MODE_ON, /**< Always use a softcursor */
   ELM_SOFTCURSOR_MODE_OFF /**< Never use a softcursor */
} Elm_Softcursor_Mode; /**< @since 1.7 */

/**
 * Set the mode used for software provided mouse cursors inline in the window
 * canvas.
 *
 * A software rendered cursor can be provided for rendering inline inside the
 * canvas windows in the event the native display system does not provide one
 * or the native one is not wanted.
 *
 * @param lonpress_timeout Timeout for long press event
 * @ingroup Softcursor
 *
 * @see elm_config_softcursor_mode_get()
 * @since 1.7
 */
EAPI void         elm_config_softcursor_mode_set(Elm_Softcursor_Mode mode);

/**
 * Get the software cursor mode
 *
 * @return The mode used for software cursors
 * @ingroup Softcursor
 *
 * @see elm_config_softcursor_mode_set()
 * @since 1.7
 */
EAPI Elm_Softcursor_Mode elm_config_softcursor_mode_get(void);

/**
 * Get the duration after which tooltip will be shown.
 *
 * @return Duration after which tooltip will be shown.
 */
EAPI double      elm_config_tooltip_delay_get(void);

/**
 * Set the duration after which tooltip will be shown.
 *
 * @return @c EINA_TRUE if value is set.
 */
EAPI void        elm_config_tooltip_delay_set(double delay);

/**
 * Get the configured cursor engine only usage
 *
 * This gets the globally configured exclusive usage of engine cursors.
 *
 * @return 1 if only engine cursors should be used
 * @ingroup Cursors
 */
EAPI Eina_Bool   elm_config_cursor_engine_only_get(void);

/**
 * Set the configured cursor engine only usage
 *
 * This sets the globally configured exclusive usage of engine cursors.
 * It won't affect cursors set before changing this value.
 *
 * @param engine_only If 1 only engine cursors will be enabled, if 0 will
 * look for them on theme before.
 * @ingroup Cursors
 */
EAPI void        elm_config_cursor_engine_only_set(Eina_Bool engine_only);

/**
 * Get the global scaling factor
 *
 * This gets the globally configured scaling factor that is applied to all
 * objects.
 *
 * @return The scaling factor
 * @ingroup Scaling
 */
EAPI double elm_config_scale_get(void);

/**
 * Set the global scaling factor
 *
 * This sets the globally configured scaling factor that is applied to all
 * objects.
 *
 * @param scale The scaling factor to set
 * @ingroup Scaling
 */
EAPI void   elm_config_scale_set(double scale);

/**
 * @defgroup Password_last_show Password show last
 * @ingroup Elementary
 *
 * Show last feature of password mode enables user to view
 * the last input entered for few seconds before masking it.
 * These functions allow to set this feature in password mode
 * of entry widget and also allow to manipulate the duration
 * for which the input has to be visible.
 *
 * @{
 */

/**
 * Get the "show last" setting of password mode.
 *
 * This gets the "show last" setting of password mode which might be
 * enabled or disabled.
 *
 * @return @c EINA_TRUE, if the "show last" setting is enabled,
 * @c EINA_FALSE  if it's disabled.
 *
 * @ingroup Password_last_show
 */
EAPI Eina_Bool elm_config_password_show_last_get(void);

/**
 * Set show last setting in password mode.
 *
 * This enables or disables show last setting of password mode.
 *
 * @param password_show_last If @c EINA_TRUE, enables "show last" in password mode.
 * @see elm_config_password_show_last_timeout_set()
 * @ingroup Password_last_show
 */
EAPI void      elm_config_password_show_last_set(Eina_Bool password_show_last);

/**
 * Get the timeout value in "show last" password mode.
 *
 * This gets the time out value for which the last input entered in password
 * mode will be visible.
 *
 * @return The timeout value of "show last" password mode.
 * @ingroup Password_last_show
 */
EAPI double    elm_config_password_show_last_timeout_get(void);

/**
 * Set's the timeout value in "show last" password mode.
 *
 * This sets the time out value for which the last input entered in password
 * mode will be visible.
 *
 * @param password_show_last_timeout The timeout value.
 * @see elm_config_password_show_last_set()
 * @ingroup Password_last_show
 */
EAPI void      elm_config_password_show_last_timeout_set(double password_show_last_timeout);

/**
 * @}
 */

/**
 * @defgroup Engine Elementary Engine
 * @ingroup Elementary
 *
 * These are functions setting and querying which rendering engine
 * Elementary will use for drawing its windows' pixels.
 *
 * The following are the available engines:
 * @li "software_x11"
 * @li "fb"
 * @li "opengl_x11"
 * @li "software_gdi"
 * @li "sdl"
 * @li "opengl_sdl"
 * @li "buffer"
 * @li "ews"
 * @li "opengl_cocoa"
 * @li "psl1ght"
 * @li "wayland_shm"
 * @li "wayland_egl"
 * @li "drm"
 *
 * @deprecated Please use elm_config_accel_preference_override_set() instead
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
 * @see elm_config_engine_set()
 * 
 * @deprecated Please use elm_config_accel_preference_override_get() instead
 */
EINA_DEPRECATED EAPI const char *elm_config_engine_get(void);

/**
 * @brief Set Elementary's rendering engine for use.
 *
 * @param engine The rendering engine's name
 *
 * Note that it will take effect only to Elementary windows created after
 * this is called.
 *
 * @see elm_win_add()
 * 
 * @deprecated Please use elm_config_accel_preference_override_set() instead
 */
EINA_DEPRECATED EAPI void        elm_config_engine_set(const char *engine);

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
 * @see elm_config_preferred_engine_set()
 * 
 * @deprecated Please use elm_config_accel_preference_override_get() instead
 */
EINA_DEPRECATED EAPI const char *elm_config_preferred_engine_get(void);

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
 * 
 * @deprecated Please use elm_config_accel_preference_override_set() instead
 */
EINA_DEPRECATED EAPI void        elm_config_preferred_engine_set(const char *engine);

/**
 * @brief Get Elementary's preferred engine to use.
 *
 * @return The acceleration preference hint string
 * @note there's no need to free the returned string, here.
 *
 * See elm_config_accel_preference_set() for more information, but this simply
 * returns what was set by this call, nothing more.
 * 
 * @see elm_config_accel_preference_set()
 * @since 1.10
 */
EAPI const char *elm_config_accel_preference_get(void);

/**
 * @brief Set Elementary's acceleration preferences for new windows.
 *
 * @param pref The preference desired as a normal C string
 *
 * Note that it will take effect only to Elementary windows created after
 * this is called. The @p pref string is a freeform C string that indicates
 * what kind of acceleration is preferred. This may or may not be honored,
 * but a best attempt will be made. Known strings are as follows:
 * 
 * "gl", "opengl" - try use opengl.
 * "3d" - try and use a 3d acceleration unit.
 * "hw", "hardware", "accel" - try any acceleration unit (best possible)
 * 
 * This takes precedence over engine preferences set with
 * elm_config_preferred_engine_set().
 * 
 * @see elm_win_add()
 * @since 1.10
 */
EAPI void        elm_config_accel_preference_set(const char *pref);


typedef struct _Elm_Text_Class
{
   const char *name;
   const char *desc;
} Elm_Text_Class;

typedef struct _Elm_Font_Overlay
{
   const char    *text_class;
   const char    *font;
   Evas_Font_Size size;
} Elm_Font_Overlay;

/**
 * Get Elementary's list of supported text classes.
 *
 * @return The text classes list, with @c Elm_Text_Class blobs as data.
 * @ingroup Fonts
 *
 * Release the list with elm_text_classes_list_free().
 */
EAPI Eina_List *elm_config_text_classes_list_get(void);

/**
 * Free Elementary's list of supported text classes.
 *
 * @ingroup Fonts
 *
 * @see elm_config_text_classes_list_get().
 */
EAPI void elm_config_text_classes_list_free(Eina_List *list);

/**
 * Get Elementary's list of font overlays, set with
 * elm_config_font_overlay_set().
 *
 * @return The font overlays list, with @c Elm_Font_Overlay blobs as
 * data.
 *
 * @ingroup Fonts
 *
 * For each text class, one can set a <b>font overlay</b> for it,
 * overriding the default font properties for that class coming from
 * the theme in use. There is no need to free this list.
 *
 * @see elm_config_font_overlay_set() and elm_config_font_overlay_unset().
 */
EAPI const Eina_List *elm_config_font_overlay_list_get(void);

/**
 * Set a font overlay for a given Elementary text class.
 *
 * @param text_class Text class name
 * @param font Font name and style string
 * @param size Font size.
 *
 * @note If the @p size is lower than zero, the value will be the amount of the size percentage. ex) -50: half of the current size, -100: current size, -10: 1/10 size.
 *
 * @ingroup Fonts
 *
 * @p font has to be in the format returned by elm_font_fontconfig_name_get().
 * @see elm_config_font_overlay_list_get()
 * @see elm_config_font_overlay_unset()
 * @see edje_object_text_class_set()
 */
EAPI void             elm_config_font_overlay_set(const char *text_class, const char *font, Evas_Font_Size size);

/**
 * Get access mode
 *
 * @return the access mode bouncing state
 *
 * @since 1.7
 *
 * @ingroup Access
 *
 * @see elm_config_access_set()
 */
EAPI Eina_Bool        elm_config_access_get(void);

/**
 * Set access mode
 *
 * @param is_accesss If @c EINA_TRUE, enables access mode
 *
 * @note Elementary objects may have information (e.g. label on the elm_button)
 * to be read. This information is read by access module when an object
 * receives EVAS_CALLBACK_MOUSE_IN event
 *
 * @since 1.7
 *
 * @ingroup Access
 *
 * @see elm_config_access_get()
 */
EAPI void             elm_config_access_set(Eina_Bool is_access);

/**
 * Get whether selection should be cleared when entry widget is unfocused.
 *
 * @return if the selection would be cleared on unfocus.
 *
 * @since 1.7
 *
 * @ingroup Selection
 *
 * @see elm_config_selection_unfocused_clear_set()
 */
EAPI Eina_Bool        elm_config_selection_unfocused_clear_get(void);

/**
 * Set whether selection should be cleared when entry widget is unfocused.
 *
 * @param enabled If @c EINA_TRUE, clear selection when unfocus,
 * otherwise does not clear selection when unfocus.
 *
 * @since 1.7
 *
 * @ingroup Selection
 *
 * @see elm_config_selection_unfocused_clear_get()
 */
EAPI void             elm_config_selection_unfocused_clear_set(Eina_Bool enabled);

/**
 * Unset a font overlay for a given Elementary text class.
 *
 * @param text_class Text class name
 *
 * @ingroup Fonts
 *
 * This will bring back text elements belonging to text class
 * @p text_class back to their default font settings.
 */
EAPI void             elm_config_font_overlay_unset(const char *text_class);

/**
 * Apply the changes made with elm_config_font_overlay_set() and
 * elm_config_font_overlay_unset() on the current Elementary window.
 *
 * @ingroup Fonts
 *
 * This applies all font overlays set to all objects in the UI.
 */
EAPI void             elm_config_font_overlay_apply(void);

/**
 * Get the configured "finger size"
 *
 * @return The finger size
 *
 * This gets the globally configured finger size, <b>in pixels</b>
 *
 * @ingroup Fingers
 */
EAPI Evas_Coord elm_config_finger_size_get(void);

/**
 * Set the configured finger size
 *
 * This sets the globally configured finger size in pixels
 *
 * @param size The finger size
 * @ingroup Fingers
 */
EAPI void       elm_config_finger_size_set(Evas_Coord size);

/**
 * Get the configured cache flush interval time
 *
 * This gets the globally configured cache flush interval time, in
 * ticks
 *
 * @return The cache flush interval time
 * @ingroup Caches
 *
 * @see elm_cache_all_flush()
 */
EAPI int       elm_config_cache_flush_interval_get(void);

/**
 * Set the configured cache flush interval time
 *
 * This sets the globally configured cache flush interval time, in ticks
 *
 * @param size The cache flush interval time
 *
 * @note The @p size must be greater than 0. if not, the cache flush will be
 *       ignored.
 *
 * @ingroup Caches
 *
 * @see elm_cache_all_flush()
 */
EAPI void      elm_config_cache_flush_interval_set(int size);

/**
 * Get the configured cache flush enabled state
 *
 * This gets the globally configured cache flush state - if it is enabled
 * or not. When cache flushing is enabled, elementary will regularly
 * (see elm_config_cache_flush_interval_get() ) flush caches and dump data out of
 * memory and allow usage to re-seed caches and data in memory where it
 * can do so. An idle application will thus minimize its memory usage as
 * data will be freed from memory and not be re-loaded as it is idle and
 * not rendering or doing anything graphically right now.
 *
 * @return The cache flush state
 * @ingroup Caches
 *
 * @see elm_cache_all_flush()
 */
EAPI Eina_Bool elm_config_cache_flush_enabled_get(void);

/**
 * Set the configured cache flush enabled state
 *
 * This sets the globally configured cache flush enabled state.
 *
 * @param enabled The cache flush enabled state
 * @ingroup Caches
 *
 * @see elm_cache_all_flush()
 */
EAPI void      elm_config_cache_flush_enabled_set(Eina_Bool enabled);

/**
 * Get the configured font cache size
 *
 * This gets the globally configured font cache size, in kilo bytes.
 *
 * @return The font cache size
 * @ingroup Caches
 */
EAPI int       elm_config_cache_font_cache_size_get(void);

/**
 * Set the configured font cache size
 *
 * This sets the globally configured font cache size, in kilo bytes
 *
 * @param size The font cache size
 * @ingroup Caches
 */
EAPI void      elm_config_cache_font_cache_size_set(int size);

/**
 * Get the configured image cache size
 *
 * This gets the globally configured image cache size, in kilo bytes
 *
 * @return The image cache size
 * @ingroup Caches
 */
EAPI int       elm_config_cache_image_cache_size_get(void);

/**
 * Set the configured image cache size
 *
 * This sets the globally configured image cache size, in kilo bytes
 *
 * @param size The image cache size
 * @ingroup Caches
 */
EAPI void       elm_config_cache_image_cache_size_set(int size);

/**
 * Get the configured edje file cache size.
 *
 * This gets the globally configured edje file cache size, in number
 * of files.
 *
 * @return The edje file cache size
 * @ingroup Caches
 */
EAPI int       elm_config_cache_edje_file_cache_size_get(void);

/**
 * Set the configured edje file cache size
 *
 * This sets the globally configured edje file cache size, in number
 * of files.
 *
 * @param size The edje file cache size
 * @ingroup Caches
 */
EAPI void       elm_config_cache_edje_file_cache_size_set(int size);

/**
 * Get the configured edje collections (groups) cache size.
 *
 * This gets the globally configured edje collections cache size, in
 * number of collections.
 *
 * @return The edje collections cache size
 * @ingroup Caches
 */
EAPI int       elm_config_cache_edje_collection_cache_size_get(void);

/**
 * Set the configured edje collections (groups) cache size
 *
 * This sets the globally configured edje collections cache size, in
 * number of collections.
 *
 * @param size The edje collections cache size
 * @ingroup Caches
 */
EAPI void       elm_config_cache_edje_collection_cache_size_set(int size);

/**
 * Get the configured vsync flag
 *
 * This gets the globally configured vsync flag that asks some backend
 * engines to use vsync display if possible.
 *
 * @return If vsync is enabled
 * 
 * @since 1.11
 */
EAPI Eina_Bool  elm_config_vsync_get(void);

/**
 * Set the configured vsync flag
 *
 * This sets the globally configured vsync flag that asks some backend
 * engines to use vsync display if possible.
 *
 * @param enabled This should be @c EINA_TRUE if enabled, or @c EINA_FALSE if
 * not.
 *
 * @since 1.11
 */
EAPI void       elm_config_vsync_set(Eina_Bool enabled);

/**
 * Get the acceleration override preference flag
 *
 * This gets the acceleration override preference. This is a flag that
 * has the global system acceleration preference configuration forcibly
 * override whatever acceleration preference the application may want.
 *
 * @return If acceleration override is enabled
 *
 * @since 1.11
 */
EAPI Eina_Bool  elm_config_accel_preference_override_get(void);

/**
 * Set the acceleration override preference flag
 *
 * This sets the acceleration override preference. This is a flag that
 * has the global system acceleration preference configuration forcibly
 * override whatever acceleration preference the application may want.
 *
 * @param enabled This should be @c EINA_TRUE if enabled, or @c EINA_FALSE if
 * not.
 *
 * @since 1.11
 */
EAPI void       elm_config_accel_preference_override_set(Eina_Bool enabled);

/**
 * Get the enable status of the focus highlight
 *
 * This gets whether the highlight on focused objects is enabled or not
 *
 * @return enable @c EINA_TRUE if the focus highlight is enabled, @c EINA_FALSE
 * otherwise.
 *
 * @see elm_config_focus_highlight_enabled_set()
 * @ingroup Focus
 */
EAPI Eina_Bool            elm_config_focus_highlight_enabled_get(void);

/**
 * Set the enable status of the focus highlight
 *
 * @param enable Enable highlight if @c EINA_TRUE, disable otherwise
 *
 * Set whether to show or not the highlight on focused objects
 *
 * Note that it will take effect only to Elementary windows created after
 * this is called.
 *
 * @see elm_config_focus_highlight_enabled_get()
 * @ingroup Focus
 */
EAPI void                 elm_config_focus_highlight_enabled_set(Eina_Bool enable);

/**
 * Get the enable status of the focus highlight animation
 *
 * @return animate @c EINA_TRUE if the focus highlight animation is enabled, @c
 * EINA_FALSE otherwise.
 *
 * Get whether the focus highlight, if enabled, will animate its switch from
 * one object to the next
 *
 * @see elm_config_focus_highlight_animate_set()
 * @ingroup Focus
 */
EAPI Eina_Bool            elm_config_focus_highlight_animate_get(void);

/**
 * Set the enable status of the highlight animation
 *
 * @param animate Enable animation if @c EINA_TRUE, disable otherwise
 *
 * Set whether the focus highlight, if enabled, will animate its switch from
 * one object to the next
 *
 * Note that it will take effect only to Elementary windows created after
 * this is called.
 *
 * @see elm_config_focus_highlight_animate_get()
 * @ingroup Focus
 */
EAPI void                 elm_config_focus_highlight_animate_set(Eina_Bool animate);

/**
 * Get the disable status of the focus highlight clip feature.
 *
 * @return The focus highlight clip disable status
 *
 * Get whether the focus highlight clip feature is disabled. If disabled return
 * @c EINA_TRUE, else return @c EINA_FALSE. If the return is @c EINA_TRUE, focus
 * highlight clip feature is not disabled so the focus highlight can be clipped.
 *
 * @see elm_config_focus_highlight_clip_disabled_set()
 * @since 1.10
 * @ingroup Focus
 */
EAPI Eina_Bool elm_config_focus_highlight_clip_disabled_get(void);

/**
 * Set the disable status of the focus highlight clip feature.
 *
 * @param disable Disable focus highlight clip feature if @c EINA_TRUE, enable
 * it otherwise.
 *
 * @see elm_config_focus_highlight_clip_disabled_get()
 * @since 1.10
 * @ingroup Focus
 */
EAPI void elm_config_focus_highlight_clip_disabled_set(Eina_Bool disable);

/**
 * Focus Movement Policy
 *
 * @since 1.10
 * @ingroup Focus
 */
typedef enum
{
   ELM_FOCUS_MOVE_POLICY_CLICK,
   ELM_FOCUS_MOVE_POLICY_IN
} Elm_Focus_Move_Policy;

/**
 * Get the focus movement policy
 *
 * @return The focus movement policy
 *
 * Get how the focus is moved to another object. It can be @c
 * ELM_FOCUS_MOVE_POLICY_CLICK or @c ELM_FOCUS_MOVE_POLICY_IN. The first means
 * elementary focus is moved on elementary object click. The second means
 * elementary focus is moved on elementary object mouse in.
 *
 * @see elm_config_focus_move_policy_set()
 * @since 1.10
 * @ingroup Focus
 */
EAPI Elm_Focus_Move_Policy elm_config_focus_move_policy_get(void);

/**
 * Set elementary focus movement policy
 *
 * @param policy A policy to apply for the focus movement
 *
 * @see elm_config_focus_move_policy_get()
 * @since 1.10
 * @ingroup Focus
 */
EAPI void elm_config_focus_move_policy_set(Elm_Focus_Move_Policy policy);

/**
 * Get disable status of item select on focus feature.
 *
 * @return The item select on focus disable status
 *
 * @see elm_config_item_select_on_focus_disabled_set
 * @since 1.10
 * @ingroup Focus
 */
EAPI Eina_Bool elm_config_item_select_on_focus_disabled_get(void);

/**
 * Set the disable status of the item select on focus feature.
 *
 * @param disable Disable item select on focus if @c EINA_TRUE, enable otherwise
 *
 * @see elm_config_item_select_on_focus_disabled_get
 * @since 1.10
 * @ingroup Focus
 */
EAPI void elm_config_item_select_on_focus_disabled_set(Eina_Bool enabled);

/**
 * Get status of first item focus on first focusin feature.
 *
 * @return The first item focus on first focusin status
 *
 * @see elm_config_first_item_focus_on_first_focusin_set
 * @since 1.11
 * @ingroup Focus
 */
EAPI Eina_Bool elm_config_first_item_focus_on_first_focusin_get(void);

/**
 * Set the first item focus on first focusin feature.
 *
 * @param enable first_item_focus_on_first_focusin if @c EINA_TRUE, enable otherwise
 *
 * @see elm_config_first_item_focus_on_first_focusin_get
 * @since 1.11
 * @ingroup Focus
 */
EAPI void elm_config_first_item_focus_on_first_focusin_set(Eina_Bool enabled);

/**
 * Get the system mirrored mode. This determines the default mirrored mode
 * of widgets.
 *
 * @return @c EINA_TRUE if mirrored is set, @c EINA_FALSE otherwise
 */
EAPI Eina_Bool elm_config_mirrored_get(void);

/**
 * Set the system mirrored mode. This determines the default mirrored mode
 * of widgets.
 *
 * @param mirrored @c EINA_TRUE to set mirrored mode, @c EINA_FALSE to unset it.
 */
EAPI void      elm_config_mirrored_set(Eina_Bool mirrored);

/**
 * Get the clouseau state. @c EINA_TRUE if clouseau was tried to be run.
 *
 * @since 1.8
 * @return @c EINA_TRUE if clouseau was tried to run, @c EINA_FALSE otherwise
 */
EAPI Eina_Bool elm_config_clouseau_enabled_get(void);

/**
 * Get the clouseau state. @c EINA_TRUE if clouseau should be attempted to be run.
 *
 * @since 1.8
 * @param enabled @c EINA_TRUE to try and run clouseau, @c EINA_FALSE otherwise.
 */
EAPI void      elm_config_clouseau_enabled_set(Eina_Bool enabled);

/**
 * Get the indicator service name according to the rotation degree.
 *
 * @param rotation The rotation which related with the indicator service name,
 * in degrees (0-360),
 *
 * @return The indicator service name according to the rotation degree. The
 * indicator service name can be either @c "elm_indicator_portrait" or
 * @c "elm_indicator_landscape".
 *
 * @note Do not free the return string.
 */
EAPI const char *elm_config_indicator_service_get(int rotation);

/**
 * Get the duration for occurring long tap event of gesture layer.
 *
 * @return Timeout for long tap event of gesture layer.
 * @ingroup Elm_Gesture_Layer
 * @since 1.8
 */
EAPI double   elm_config_glayer_long_tap_start_timeout_get(void);

/**
 * Set the duration for occurring long tap event of gesture layer.
 *
 * @param long_tap_timeout Timeout for long tap event of gesture layer.
 * @ingroup Elm_Gesture_Layer
 * @since 1.8
 */
EAPI void   elm_config_glayer_long_tap_start_timeout_set(double long_tap_timeout);

/**
 * Get the duration for occurring double tap event of gesture layer.
 *
 * @return Timeout for double tap event of gesture layer.
 * @ingroup Elm_Gesture_Layer
 * @since 1.8
 */
EAPI double   elm_config_glayer_double_tap_timeout_get(void);

/**
 * Set the duration for occurring double tap event of gesture layer.
 *
 * @param long_tap_timeout Timeout for double tap event of gesture layer.
 * @ingroup Elm_Gesture_Layer
 * @since 1.8
 */
EAPI void   elm_config_glayer_double_tap_timeout_set(double double_tap_timeout);

typedef struct _Elm_Color_Class
{
   const char *name;
   const char *desc;
} Elm_Color_Class;

typedef struct _Elm_Color_Overlay
{
   const char *color_class;
   struct {
      unsigned char r, g, b, a;
   } color, outline, shadow;
} Elm_Color_Overlay;

/**
 * Get Elementary's list of supported color classes.
 *
 * @return The color classes list, with @c Elm_Color_Class blobs as data.
 * @ingroup Colors
 * @since 1.10
 *
 * Release the list with elm_color_classes_list_free().
 */
EAPI Eina_List *elm_config_color_classes_list_get(void);

/**
 * Free Elementary's list of supported color classes.
 *
 * @ingroup Colors
 * @since 1.10
 *
 * @see elm_config_color_classes_list_get().
 */
EAPI void      elm_config_color_classes_list_free(Eina_List *list);

/**
 * Get Elementary's list of color overlays, set with
 * elm_config_color_overlay_set().
 *
 * @return The color overlays list, with @c Elm_Color_Overlay blobs as
 * data.
 *
 * @ingroup Colors
 * @since 1.10
 *
 * For each color class, one can set a <b>color overlay</b> for it,
 * overriding the default color properties for that class coming from
 * the theme in use. There is no need to free this list.
 *
 * @see elm_config_color_overlay_set()
 * @see elm_config_color_overlay_unset().
 */
EAPI const Eina_List *elm_config_color_overlay_list_get(void);

/**
 * Set a color overlay for a given Elementary color class.
 *
 * @param color_class Color class name
 * @param r Object Red value
 * @param g Object Green value
 * @param b Object Blue value
 * @param a Object Alpha value
 * @param r2 Text outline Red value
 * @param g2 Text outline Green value
 * @param b2 Text outline Blue value
 * @param a2 Text outline Alpha value
 * @param r3 Text shadow Red value
 * @param g3 Text shadow Green value
 * @param b3 Text shadow Blue value
 * @param a3 Text shadow Alpha value
 *
 * @ingroup Colors
 * @since 1.10

 * The first color is for the object itself, the second color is for the text
 * outline, and the third color is for the text shadow.
 *
 * @note The second two color are only for texts.

 * Setting color emits a signal "color_class,set" with source being
 * the given color class in all edje objects.
 *
 * @see elm_config_color_overlay_list_get()
 * @see elm_config_color_overlay_unset()
 * @see edje_color_class_set()

 * @note unlike Evas, Edje colors are @b not pre-multiplied. That is,
 *       half-transparent white is 255 255 255 128.
 */
EAPI void      elm_config_color_overlay_set(const char *color_class,
                                            int r, int g, int b, int a,
                                            int r2, int g2, int b2, int a2,
                                            int r3, int g3, int b3, int a3);

/**
 * Unset a color overlay for a given Elementary color class.
 *
 * @param color_class Color class name
 *
 * @ingroup Colors
 * @since 1.10
 *
 * This will bring back color elements belonging to color class
 * @p color_class back to their default color settings.
 */
EAPI void      elm_config_color_overlay_unset(const char *color_class);

/**
 * Apply the changes made with elm_config_color_overlay_set() and
 * elm_config_color_overlay_unset() on the current Elementary window.
 *
 * @ingroup Colors
 * @since 1.10
 *
 * This applies all color overlays set to all objects in the UI.
 */
EAPI void      elm_config_color_overlay_apply(void);

/**
 * Get the magnifier enabled state for entries
 *
 * @return The enabled state for magnifier
 * @since 1.9
 */
EAPI Eina_Bool elm_config_magnifier_enable_get(void);

/**
 * Set the magnifier enabled state for entires
 *
 * @param enable The magnifier config state
 * @since 1.9
 */
EAPI void      elm_config_magnifier_enable_set(Eina_Bool enable);

/**
 * Get the amount of scaling the magnifer does
 *
 * @return The scaling amount (1.0 is none, 2.0 is twice as big etc.)
 * @since 1.9
 */
EAPI double    elm_config_magnifier_scale_get(void);

/**
 * Set the amount of scaling the magnifier does
 *
 * @param scale The scaling amount for magnifiers
 * @since 1.9
 */
EAPI void      elm_config_magnifier_scale_set(double scale);

/**
 * Get the mute state of an audio channel for effects
 *
 * @param channel The channel to get the mute state of
 * @return The mute state
 * @since 1.9
 */
EAPI Eina_Bool elm_config_audio_mute_get(Edje_Channel channel);

/**
 * Set the mute state of the specified channel
 *
 * @param channel The channel to set the mute state of
 * @param mute The mute state to set
 * @since 1.9
 */
EAPI void      elm_config_audio_mute_set(Edje_Channel channel, Eina_Bool mute);

/**
 * @defgroup ATSPI AT-SPI2 Accessibility
 * @ingroup Elementary
 *
 * Elementary widgets support Linux Accessibility standard. For more
 * information please visit:
 * http://www.linuxfoundation.org/collaborate/workgroups/accessibility/atk/at-spi/at-spi_on_d-bus
 *
 * @{
 */

/**
 * Gets ATSPI mode
 *
 * @return the ATSPI mode
 *
 * @since 1.10
 *
 * @ingroup ATSPI
 *
 * @see elm_config_atspi_mode_set()
 */
EAPI Eina_Bool        elm_config_atspi_mode_get(void);

/**
 * Sets ATSPI mode
 *
 * @param is_atspi If @c EINA_TRUE, enables ATSPI2 mode
 *
 * @note Enables Linux Accessibility support for Elementary widgets.
 *
 * @since 1.10
 *
 * @ingroup ATSPI
 *
 * @see elm_config_atspi_mode_get()
 */
EAPI void             elm_config_atspi_mode_set(Eina_Bool is_atspi);

/**
 * @}
 */

/**
 * @}
 */

