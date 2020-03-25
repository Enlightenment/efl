#ifndef _ECORE_WL2_INTERNAL_H
# define _ECORE_WL2_INTERNAL_H

# ifdef EAPI
#  undef EAPI
# endif

# ifdef _WIN32
#  ifdef EFL_BUILD
#   ifdef DLL_EXPORT
#    define EAPI __declspec(dllexport)
#   else
#    define EAPI
#   endif
#  else
#   define EAPI __declspec(dllimport)
#  endif
# else
#  ifdef __GNUC__
#   if __GNUC__ >= 4
#    define EAPI __attribute__ ((visibility("default")))
#   else
#    define EAPI
#   endif
#  else
#   define EAPI
#  endif
# endif

EAPI extern int ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_PREPARE; /** @since 1.20 */
EAPI extern int ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_PREPARE_DONE; /** @since 1.20 */
EAPI extern int ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_REQUEST; /** @since 1.20 */
EAPI extern int ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_DONE; /** @since 1.20 */

EAPI void ecore_wl2_window_rotation_change_prepare_send(Ecore_Wl2_Window *window, int rot, int w, int h, Eina_Bool resize);
EAPI void ecore_wl2_window_rotation_change_prepare_done_send(Ecore_Wl2_Window *window, int rot);
EAPI void ecore_wl2_window_rotation_change_request_send(Ecore_Wl2_Window *window, int rot);
EAPI void ecore_wl2_window_rotation_change_done_send(Ecore_Wl2_Window *window, int rot, int w, int h);

EAPI void ecore_wl2_window_false_commit(Ecore_Wl2_Window *window);
EAPI Eina_Bool ecore_wl2_buffer_fit(Ecore_Wl2_Buffer *b, int w, int h);

EAPI void ecore_wl2_window_damage(Ecore_Wl2_Window *window, Eina_Rectangle *rects, unsigned int count);
EAPI Eina_Bool ecore_wl2_buffer_init(Ecore_Wl2_Display *ewd, Ecore_Wl2_Buffer_Type types);
EAPI Ecore_Wl2_Buffer *ecore_wl2_buffer_create(Ecore_Wl2_Display *ewd, int w, int h, Eina_Bool alpha);
EAPI void ecore_wl2_buffer_destroy(Ecore_Wl2_Buffer *b);
EAPI struct wl_buffer *ecore_wl2_buffer_wl_buffer_get(Ecore_Wl2_Buffer *buf);
EAPI void *ecore_wl2_buffer_map(Ecore_Wl2_Buffer *buf, int *w, int *h, int *stride);
EAPI void ecore_wl2_buffer_unmap(Ecore_Wl2_Buffer *buf);
EAPI void ecore_wl2_buffer_discard(Ecore_Wl2_Buffer *buf);
EAPI void ecore_wl2_buffer_lock(Ecore_Wl2_Buffer *b);
EAPI void ecore_wl2_buffer_unlock(Ecore_Wl2_Buffer *b);
EAPI void ecore_wl2_buffer_destroy(Ecore_Wl2_Buffer *b);
EAPI Eina_Bool ecore_wl2_buffer_busy_get(Ecore_Wl2_Buffer *buffer);
EAPI void ecore_wl2_buffer_busy_set(Ecore_Wl2_Buffer *buffer);
EAPI int ecore_wl2_buffer_age_get(Ecore_Wl2_Buffer *buffer);
EAPI void ecore_wl2_buffer_age_set(Ecore_Wl2_Buffer *buffer, int age);
EAPI void ecore_wl2_buffer_age_inc(Ecore_Wl2_Buffer *buffer);

EAPI Ecore_Wl2_Surface *ecore_wl2_surface_create(Ecore_Wl2_Window *win, Eina_Bool alpha);
EAPI void ecore_wl2_surface_destroy(Ecore_Wl2_Surface *surface);
EAPI void ecore_wl2_surface_reconfigure(Ecore_Wl2_Surface *surface, int w, int h, uint32_t flags, Eina_Bool alpha);
EAPI void *ecore_wl2_surface_data_get(Ecore_Wl2_Surface *surface, int *w, int *h);
EAPI int  ecore_wl2_surface_assign(Ecore_Wl2_Surface *surface);
EAPI void ecore_wl2_surface_post(Ecore_Wl2_Surface *surface, Eina_Rectangle *rects, unsigned int count);
EAPI void ecore_wl2_surface_flush(Ecore_Wl2_Surface *surface, Eina_Bool purge);
EAPI void ecore_wl2_window_surface_flush(Ecore_Wl2_Window *window, Eina_Bool purge);
EAPI Ecore_Wl2_Buffer *ecore_wl2_surface_buffer_create(Ecore_Wl2_Surface *surface);
EAPI int ecore_wl2_surface_manager_add(Ecore_Wl2_Surface_Interface *intf);
EAPI void ecore_wl2_surface_manager_del(Ecore_Wl2_Surface_Interface *intf);
EAPI Ecore_Wl2_Window *ecore_wl2_surface_window_get(Ecore_Wl2_Surface *surface);
EAPI Eina_Bool ecore_wl2_surface_alpha_get(Ecore_Wl2_Surface *surface);

/**
 * Attach a buffer to a window
 *
 * Note that the GL stack my attach buffers to a surface - we should call this
 * function at that time (with a NULL buffer) to track whether a surface
 * has a valid buffer.  That is, call with implicit true and buffer NULL at
 * the time of glSwapBuffers.
 *
 * @param window The target window
 * @param buffer The buffer to attach
 * @param x X offset from corner
 * @param y Y offset from corner
 * @param implicit True if an external library is doing the actual attaching
 *
 * @since 1.21
 */
EAPI void ecore_wl2_window_buffer_attach(Ecore_Wl2_Window *win, void *buffer, int x, int y, Eina_Bool implicit);

/**
 * Set a buffer transform on a given window
 *
 * @param window The window on which to set the buffer transform
 * @param transform The buffer transform being requested
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.20
 */
EAPI void ecore_wl2_window_buffer_transform_set(Ecore_Wl2_Window *window, int transform);

/**
 * Iconify a window
 *
 * @param window The window to iconifiy
 * @param iconified The new iconified state to set
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.17
 */
EAPI void ecore_wl2_window_iconified_set(Ecore_Wl2_Window *window, Eina_Bool iconified);

/**
 * Check if a wayland window's surface is in the pending state.
 *
 * A surface is pending if it's been commit but we haven't received a
 * frame callback for it yet.  This mean's we're not ready to draw yet.
 *
 * @param window The window whose surface we want to check
 *
 * @return whether the window's surface is pending or not.
 *
 * @since 1.21
 */
EAPI Eina_Bool ecore_wl2_window_pending_get(Ecore_Wl2_Window *window);

/**
 * @defgroup Ecore_Wl2_Dnd_Group Wayland Library Drag-n-Drop Functions
 * @ingroup Ecore_Wl2_Group
 *
 * Functions that deal with creating, destroying, or interacting with
 * Wayland Drag-n-Drop
 */

/**
 * Set the types which are supported by a possible drag and drop operation.
 * This call initializes a data source and offeres the given mimetypes
 *
 * @param input the input where to add on the data source
 * @param types a null-terminated array of mimetypes
 *
 * @ingroup Ecore_Wl2_Dnd_Group
 * @since 1.17
 */
EAPI void ecore_wl2_dnd_drag_types_set(Ecore_Wl2_Input *input, const char **types);

/**
 * Start a drag on the given input
 *
 * @param input the input to use
 * @param window the window which is the origin of the drag operation
 * @param drag_window the window which is used as window of the visible hint.
 *
 * @return The serial for the start_drag request
 *
 * @ingroup Ecore_Wl2_Dnd_Group
 * @since 1.17
 */
EAPI uint32_t ecore_wl2_dnd_drag_start(Ecore_Wl2_Input *input, Ecore_Wl2_Window *window, Ecore_Wl2_Window *drag_window);

/**
 * Call wl_data_source.set_actions on an existing source
 *
 * @param input the input to use
 *
 * @see ecore_wl2_dnd_drag_start for a more useful function.
 *
 * @ingroup Ecore_Wl2_Dnd_Group
 * @since 1.20
 */
EAPI void ecore_wl2_dnd_set_actions(Ecore_Wl2_Input *input);

/**
 * End a drag started by a call to ecore_wl2_dnd_drag_start
 *
 * @param input the input object on which the drag was started
 *
 * @ingroup Ecore_Wl2_Dnd_Group
 * @since 1.17
 */
EAPI void ecore_wl2_dnd_drag_end(Ecore_Wl2_Input *input);

/**
 * Get the offer which is currently resposible for the clipboard
 *
 * @param input the input object to use
 *
 * @ingroup Ecore_Wl2_Dnd_Group
 * @since 1.19
 */
EAPI Ecore_Wl2_Offer* ecore_wl2_dnd_selection_get(Ecore_Wl2_Input *input);

/**
 * Set the types which are available from this client
 * Later the event ECORE_WL2_EVENT_DATA_SOURCE_SEND is emitted,
 * where the caller of this api must write the data (encoded in the given mimetype) to the fd
 *
 * @param input the input to provice this types on
 * @param types a null-terminated array of mimetypes supported by the client
 *
 * @return serial of request on success, 0 on failure
 *
 * @ingroup Ecore_Wl2_Dnd_Group
 * @since 1.17
 */
EAPI uint32_t ecore_wl2_dnd_selection_set(Ecore_Wl2_Input *input, const char **types);

/**
 * Clear the selection currently setted on this input.
 *
 * @param input the input to clear
 *
 * @return serial of request on success, 0 on failure
 *
 * @ingroup Ecore_Wl2_Dnd_Group
 * @since 1.17
 */
EAPI uint32_t ecore_wl2_dnd_selection_clear(Ecore_Wl2_Input *input);

/**
 * Get the actions available from the data source
 *
 * @param offer Offer object to use
 *
 * @return or´ed values from Ecore_Wl2_Drag_Action which are describing the available actions
 *
 * @ingroup Ecore_Wl2_Dnd_Group
 * @since 1.19
 */
EAPI Ecore_Wl2_Drag_Action ecore_wl2_offer_actions_get(Ecore_Wl2_Offer *offer);

/**
 * Set the actions which are supported by you
 *
 * @param offer Offer object to use
 * @param actions A or´ed value of mutliple Ecore_Wl2_Drag_Action values
 * @param action the preferred action out of the actions
 *
 * @ingroup Ecore_Wl2_Dnd_Group
 * @since 1.19
 */
EAPI void ecore_wl2_offer_actions_set(Ecore_Wl2_Offer *offer, Ecore_Wl2_Drag_Action actions, Ecore_Wl2_Drag_Action action);

/**
 * Get action which is set by either the data source or in the last call of actions_set
 *
 * @param offer Offer object to use
 *
 * @return the preferred action
 *
 * @ingroup Ecore_Wl2_Dnd_Group
 * @since 1.19
 */
EAPI Ecore_Wl2_Drag_Action ecore_wl2_offer_action_get(Ecore_Wl2_Offer *offer);

/**
 * Get the mime types which are given by the source
 *
 * @param offer the offer to query
 *
 * @return a eina array of strdup´ed strings, this array must NOT be changed or freed
 *
 * @ingroup Ecore_Wl2_Dnd_Group
 * @since 1.19
 */
EAPI Eina_Array* ecore_wl2_offer_mimes_get(Ecore_Wl2_Offer *offer);

/**
 * Set mimetypes you are accepting under this offer
 *
 * @param offer the offer to use
 *
 * @ingroup Ecore_Wl2_Dnd_Group
 * @since 1.19
 */
EAPI void ecore_wl2_offer_mimes_set(Ecore_Wl2_Offer *offer, Eina_Array *mimes);

/**
 * Accept a single mime type for an offer
 *
 * @param offer the offer to use
 * @param mime_type the mime type
 *
 * @ingroup Ecore_Wl2_Dnd_Group
 * @since 1.20
 */
EAPI void ecore_wl2_offer_accept(Ecore_Wl2_Offer *offer, const char *mime_type);

/**
 * Request the data from this offer.
 * The event ECORE_WL2_EVENT_OFFER_DATA_READY is called when the data is available.
 * There offer will be not destroyed as long as requested data is not emitted by the event.
 *
 * @param offer the offer to use
 * @param mime the mimetype to receive
 *
 * @ingroup Ecore_Wl2_Dnd_Group
 * @since 1.19
 */
EAPI void ecore_wl2_offer_receive(Ecore_Wl2_Offer *offer, char *mime);

/**
 * Request the data from this offer on an externally managed fd.
 * The event ECORE_WL2_EVENT_OFFER_DATA_READY is called when the data is available.
 * There offer will be not destroyed as long as requested data is not emitted by the event.
 *
 * @param offer the offer to use
 * @param mime the mimetype to receive
 * @param fd the fd to pass for receiving
 *
 * @ingroup Ecore_Wl2_Dnd_Group
 * @since 1.20
 */
EAPI void ecore_wl2_offer_proxy_receive(Ecore_Wl2_Offer *offer, const char *mime, int fd);

/**
 * End the use of a proxy received offer. This may invalidate the offer object
 *
 * @param offer the offer
 *
 * @ingroup Ecore_Wl2_Dnd_Group
 * @since 1.20
 */
EAPI void ecore_wl2_offer_proxy_receive_end(Ecore_Wl2_Offer *offer);

/**
 * Check if the given offer supports the given mimetype
 *
 * @param offer the offer to use
 * @param mime the mimetype to check
 *
 * @return Returns true if the mimetype is supported by this offer, false if not
 *
 * @ingroup Ecore_Wl2_Dnd_Group
 * @since 1.19
 */
EAPI Eina_Bool ecore_wl2_offer_supports_mime(Ecore_Wl2_Offer *offer, const char *mime);

/**
 * Mark this offer as finished
 * This will call the dnd_finished event on the source of the sender.
 *
 * @param offer the offer to use
 *
 * @ingroup Ecore_Wl2_Dnd_Group
 * @since 1.19
 */
EAPI void ecore_wl2_offer_finish(Ecore_Wl2_Offer *offer);

/**
 * @defgroup Ecore_Wl2_Subsurface_Group Functions to manipulate subsurfaces.
 * @ingroup Ecore_Wl2_Group
 *
 * Functions to manipulate wayland subsurfaces, using Ecore_Wl2_Subsurface.
 *
 * This API is intended to expose Wayland subsurface functionality, although it
 * should not be necessary for most applications to use it, as soon as we have
 * means to make Evas automatically switch Evas images to use subsurfaces.
 *
 * It can/should be used, for instance, when subsurfaces are needed to be not
 * in sync with the main window surface.
 */

/**
 * Create and return a new subsurface.
 *
 * Create a new surface (and subsurface interface), with the parent surface
 * being the one associated with the given window.
 *
 * @param window The window. It must be visible, otherwise there will be no surface created
 * for it yet.
 *
 * @return the allocated and initialized Ecore_Wl2_Subsurface object, or
 * NULL on failure
 *
 * @ingroup Ecore_Wl2_Subsurface_Group
 * @since 1.17
 */
EAPI Ecore_Wl2_Subsurface *ecore_wl2_subsurface_new(Ecore_Wl2_Window *window);

/**
 * Destroy the given subsurface, as well as the surface associated with it.
 *
 * @param subsurface the subsurface
 *
 * @ingroup Ecore_Wl2_Subsurface_Group
 * @since 1.17
 */
EAPI void ecore_wl2_subsurface_del(Ecore_Wl2_Subsurface *subsurface);

/**
 * Get the wl_surface for this subsurface
 *
 * @param subsurface the subsurface
 *
 * @return the wl_surface associated with this subsurface, or NULL on failure
 *
 * @ingroup Ecore_Wl2_Subsurface_Group
 * @since 1.17
 */
EAPI struct wl_surface *ecore_wl2_subsurface_surface_get(Ecore_Wl2_Subsurface *subsurface);

/**
 * Set the position of this subsurface, relative to its parent surface.
 * If subsurface is defined and the x, y coordinates differ from the currently
 * tracked position, this also schedules a sub-surface position change.
 *
 * @param subsurface the subsurface
 * @param x coordinate in the parent surface
 * @param y coordinate in the parent surface
 *
 * @ingroup Ecore_Wl2_Subsurface_Group
 * @since 1.17
 */
EAPI void ecore_wl2_subsurface_position_set(Ecore_Wl2_Subsurface *subsurface, int x, int y);

/**
 * Get the position of this subsurface, relative to its parent surface.
 * Coordinates will be returned in x and y if non-NULL.
 *
 * @param subsurface the subsurface
 * @param x coordinate in the parent surface, or NULL to ignore
 * @param y coordinate in the parent surface, or NULL to ignore
 *
 * @ingroup Ecore_Wl2_Subsurface_Group
 * @since 1.17
 */
EAPI void ecore_wl2_subsurface_position_get(Ecore_Wl2_Subsurface *subsurface, int *x, int *y);

/**
 * Place subsurface on layer above a reference surface
 *
 * Moves the subsurface to just above the reference surface,
 * changing the z-order.  The reference surface must
 * be either a sibling or parent surface, else a protocol error will
 * be generated.
 *
 * @param subsurface the subsurface
 * @param surface the sibling reference surface
 *
 * @ingroup Ecore_Wl2_Subsurface_Group
 * @since 1.17
 */
EAPI void ecore_wl2_subsurface_place_above(Ecore_Wl2_Subsurface *subsurface, struct wl_surface *surface);

/**
 * Place subsurface on layer below a reference surface
 *
 * See ecore_wl2_subsurface_place_above.
 *
 * @param subsurface the subsurface
 * @param surface the sibling reference surface
 *
 * @ingroup Ecore_Wl2_Subsurface_Group
 * @since 1.17
 */
EAPI void ecore_wl2_subsurface_place_below(Ecore_Wl2_Subsurface *subsurface, struct wl_surface *surface);

/**
 * Enables or disables sub-surface synchronization
 *
 * When synchronization is enabled, surface commits on the subsurface
 * will be cached and only applied when the parent surface's state is
 * applied.  This ensures atomic updates of the parent and all of its
 * synchronized sub-surfaces.
 *
 * When synchronization is disabled, commits will apply to the pending
 * state directly without caching, just like a normal wl_surface.  If
 * there are already cached events when this is set, those events are
 * applied simultaneously with the desync event.
 *
 * Attempting to enable synchronization when the subsurface already
 * thinks it's sync'd, or desync when it believes its desync'd, will
 * be trivially ignored and will not generate a Wayland event.
 *
 * See Wayland's set_desync documentation for further details and
 * exceptional cases.
 *
 * @param subsurface the subsurface
 * @param sync true to enable synchronization, false to desynchronize
 *
 * @ingroup Ecore_Wl2_Subsurface_Group
 * @since 1.17
 */
EAPI void ecore_wl2_subsurface_sync_set(Ecore_Wl2_Subsurface *subsurface, Eina_Bool sync);

/**
 * Set an opaque region for the given subsurface.
 *
 * This is an optimization hint to the compositor to allow it avoid
 * redrawing content unnecessarily.  Note that marking transparent
 * content as opaque will cause repaint artifacts.
 *
 * Use a 0x0 region size to unset the opaque region.
 *
 * @param subsurface the subsurface
 * @param x coordinate in the parent surface
 * @param y coordinate in the parent surface
 * @param w width to set as opaque
 * @param h height to set as opaque
 *
 * @ingroup Ecore_Wl2_Subsurface_Group
 * @since 1.17
 */
EAPI void ecore_wl2_subsurface_opaque_region_set(Ecore_Wl2_Subsurface *subsurface, int x, int y, int w, int h);

/**
 * Get list of supported auxiliary window hints
 *
 * @param window
 *
 * @return An Eina_List of supported auxiliary hints, or NULL otherwise
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.20
 */
EAPI Eina_List *ecore_wl2_window_aux_hints_supported_get(Ecore_Wl2_Window *window);

/**
 * Add a supported auxiliary hint to a given window
 *
 * @param window
 * @param id
 * @param hint
 * @param val
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.20
 */
EAPI void ecore_wl2_window_aux_hint_add(Ecore_Wl2_Window *window, int id, const char *hint, const char *val);

/**
 * Change an auxiliary hint on a given window
 *
 * @param window
 * @param id
 * @param val
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.20
 */
EAPI void ecore_wl2_window_aux_hint_change(Ecore_Wl2_Window *window, int id, const char *val);

/**
 * Delete an auxiliary hint on a given window
 *
 * @param window
 * @param id
 *
 * @ingroup Ecore_Wl2_Window_Group
 * @since 1.20
 */
EAPI void ecore_wl2_window_aux_hint_del(Ecore_Wl2_Window *window, int id);

/**
 * Terminate a Wayland display's main loop
 *
 * @brief This function is typically used by servers to terminate the
 * Wayland display main loop. This is usually only called when a server
 * encounters an error.
 *
 * @param display The Ecore_Wl2_Display to terminate
 *
 * @ingroup Ecore_Wl2_Display_Group
 * @since 1.17
 */
EAPI void ecore_wl2_display_terminate(Ecore_Wl2_Display *display);

# undef EAPI
# define EAPI

#endif
