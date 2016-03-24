#ifndef ZWP_WWW_CLIENT_PROTOCOL_H
#define ZWP_WWW_CLIENT_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-client.h"

struct wl_client;
struct wl_resource;

struct wl_surface;
struct www;
struct www_surface;

extern const struct wl_interface www_interface;
extern const struct wl_interface www_surface_interface;

#define WWW_CREATE	0

#define WWW_CREATE_SINCE_VERSION	1

static inline void
www_set_user_data(struct www *www, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) www, user_data);
}

static inline void *
www_get_user_data(struct www *www)
{
	return wl_proxy_get_user_data((struct wl_proxy *) www);
}

static inline uint32_t
www_get_version(struct www *www)
{
	return wl_proxy_get_version((struct wl_proxy *) www);
}

static inline void
www_destroy(struct www *www)
{
	wl_proxy_destroy((struct wl_proxy *) www);
}

static inline struct www_surface *
www_create(struct www *www, struct wl_surface *surface)
{
	struct wl_proxy *id;

	id = wl_proxy_marshal_constructor((struct wl_proxy *) www,
			 WWW_CREATE, &www_surface_interface, NULL, surface);

	return (struct www_surface *) id;
}

struct www_surface_listener {
	/**
	 * status - Status update on a www_surface
	 * @x_rel: (none)
	 * @y_rel: (none)
	 * @timestamp: (none)
	 *
	 * 
	 */
	void (*status)(void *data,
		       struct www_surface *www_surface,
		       int32_t x_rel,
		       int32_t y_rel,
		       uint32_t timestamp);
	/**
	 * start_drag - Drag has started
	 *
	 * 
	 */
	void (*start_drag)(void *data,
			   struct www_surface *www_surface);
	/**
	 * end_drag - Drag has ended
	 *
	 * 
	 */
	void (*end_drag)(void *data,
			 struct www_surface *www_surface);
};

static inline int
www_surface_add_listener(struct www_surface *www_surface,
			 const struct www_surface_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) www_surface,
				     (void (**)(void)) listener, data);
}

#define WWW_SURFACE_DESTROY	0

#define WWW_SURFACE_DESTROY_SINCE_VERSION	1

static inline void
www_surface_set_user_data(struct www_surface *www_surface, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) www_surface, user_data);
}

static inline void *
www_surface_get_user_data(struct www_surface *www_surface)
{
	return wl_proxy_get_user_data((struct wl_proxy *) www_surface);
}

static inline uint32_t
www_surface_get_version(struct www_surface *www_surface)
{
	return wl_proxy_get_version((struct wl_proxy *) www_surface);
}

static inline void
www_surface_destroy(struct www_surface *www_surface)
{
	wl_proxy_marshal((struct wl_proxy *) www_surface,
			 WWW_SURFACE_DESTROY);

	wl_proxy_destroy((struct wl_proxy *) www_surface);
}

#ifdef  __cplusplus
}
#endif

#endif
