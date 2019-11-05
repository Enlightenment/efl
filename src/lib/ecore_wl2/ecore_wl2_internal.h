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

# undef EAPI
# define EAPI

#endif
