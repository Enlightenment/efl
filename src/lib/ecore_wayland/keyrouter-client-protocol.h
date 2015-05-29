
#ifndef KEYROUTER_CLIENT_PROTOCOL_H
#define KEYROUTER_CLIENT_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-client.h"

struct wl_client;
struct wl_resource;

struct wl_keyrouter;

extern const struct wl_interface wl_keyrouter_interface;

#ifndef WL_KEYROUTER_ERROR_ENUM
#define WL_KEYROUTER_ERROR_ENUM
enum wl_keyrouter_error {
   WL_KEYROUTER_ERROR_NONE = 0,
   WL_KEYROUTER_ERROR_INVALID_SURFACE = 1,
   WL_KEYROUTER_ERROR_INVALID_KEY = 2,
   WL_KEYROUTER_ERROR_INVALID_MODE = 3,
   WL_KEYROUTER_ERROR_GRABBED_ALREADY = 4,
   WL_KEYROUTER_ERROR_NO_PERMISSION = 5,
   WL_KEYROUTER_ERROR_NO_SYSTEM_RESOURCES = 6,
};
#endif /* WL_KEYROUTER_ERROR_ENUM */

#ifndef WL_KEYROUTER_MODE_ENUM
#define WL_KEYROUTER_MODE_ENUM
/**
 * wl_keyrouter_mode - mode for a key grab
 * @WL_KEYROUTER_MODE_NONE: none
 * @WL_KEYROUTER_MODE_SHARED: mode to get a key grab with the other
 *  client surfaces when the focused client surface gets the key
 * @WL_KEYROUTER_MODE_TOPMOST: mode to get a key grab when the client
 *  surface is the top most surface
 * @WL_KEYROUTER_MODE_OVERRIDABLE_EXCLUSIVE: mode to get a key grab
 *  exclusively, overridably regardless of the order in the surface stack
 * @WL_KEYROUTER_MODE_EXCLUSIVE: mode to get a key grab exclusively
 *  regardless of the order in surface stack
 *
 * This value is used to set a mode for a key grab. With this mode and
 * the order of the surface between surfaces' stack, the compositor will
 * determine the destination client surface.
 */
enum wl_keyrouter_mode {
   WL_KEYROUTER_MODE_NONE = 0,
   WL_KEYROUTER_MODE_SHARED = 1,
   WL_KEYROUTER_MODE_TOPMOST = 2,
   WL_KEYROUTER_MODE_OVERRIDABLE_EXCLUSIVE = 3,
   WL_KEYROUTER_MODE_EXCLUSIVE = 4,
};
#endif /* WL_KEYROUTER_MODE_ENUM */

/**
 * wl_keyrouter - an interface to set each focus for each key
 * @keygrab_notify: (none)
 *
 * In tradition, all the keys in a keyboard and a device on which some
 * keys are attached will be sent to focus surface by default. Currently
 * it's possible to set up each focus for each key in a keyboard and a
 * device. Therefore, by setting a key grab for a surface, the owner of the
 * surface will get the key event when it has the key grab for the key.
 */
struct wl_keyrouter_listener {
   /**
    * keygrab_notify - (none)
    * @surface: (none)
    * @key: (none)
    * @mode: (none)
    * @error: (none)
    */
   void (*keygrab_notify)(void *data,
                   struct wl_keyrouter *wl_keyrouter,
                   struct wl_surface *surface,
                   uint32_t key,
                   uint32_t mode,
                   uint32_t error);
};

static inline int
wl_keyrouter_add_listener(struct wl_keyrouter *wl_keyrouter,
                          const struct wl_keyrouter_listener *listener, void *data)
{
   return wl_proxy_add_listener((struct wl_proxy *) wl_keyrouter,
                                (void (**)(void)) listener, data);
}

#define WL_KEYROUTER_SET_KEYGRAB	0
#define WL_KEYROUTER_UNSET_KEYGRAB	1
#define WL_KEYROUTER_GET_KEYGRAB_STATUS	2

static inline void
wl_keyrouter_set_user_data(struct wl_keyrouter *wl_keyrouter, void *user_data)
{
   wl_proxy_set_user_data((struct wl_proxy *) wl_keyrouter, user_data);
}

static inline void *
wl_keyrouter_get_user_data(struct wl_keyrouter *wl_keyrouter)
{
   return wl_proxy_get_user_data((struct wl_proxy *) wl_keyrouter);
}

static inline void
wl_keyrouter_destroy(struct wl_keyrouter *wl_keyrouter)
{
   wl_proxy_destroy((struct wl_proxy *) wl_keyrouter);
}

static inline void
wl_keyrouter_set_keygrab(struct wl_keyrouter *wl_keyrouter, struct wl_surface *surface, uint32_t key, uint32_t mode)
{
   wl_proxy_marshal((struct wl_proxy *) wl_keyrouter,
                    WL_KEYROUTER_SET_KEYGRAB, surface, key, mode);
}

static inline void
wl_keyrouter_unset_keygrab(struct wl_keyrouter *wl_keyrouter, struct wl_surface *surface, uint32_t key)
{
   wl_proxy_marshal((struct wl_proxy *) wl_keyrouter,
                    WL_KEYROUTER_UNSET_KEYGRAB, surface, key);
}

static inline void
wl_keyrouter_get_keygrab_status(struct wl_keyrouter *wl_keyrouter, struct wl_surface *surface, uint32_t key)
{
   wl_proxy_marshal((struct wl_proxy *) wl_keyrouter,
                    WL_KEYROUTER_GET_KEYGRAB_STATUS, surface, key);
}

#ifdef  __cplusplus
}
#endif

#endif
