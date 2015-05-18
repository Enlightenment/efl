#ifndef TIZEN_EXTENSION_CLIENT_PROTOCOL_H
#define TIZEN_EXTENSION_CLIENT_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-client.h"

struct wl_client;
struct wl_resource;

struct tizen_resource;
struct tizen_surface_extension;
struct tizen_transient_for;

extern const struct wl_interface tizen_resource_interface;
extern const struct wl_interface tizen_surface_extension_interface;
extern const struct wl_interface tizen_transient_for_interface;

struct tizen_resource_listener {
	/**
	 * resource_id - (none)
	 * @id: (none)
	 */
	void (*resource_id)(void *data,
			    struct tizen_resource *tizen_resource,
			    uint32_t id);
};

static inline int
tizen_resource_add_listener(struct tizen_resource *tizen_resource,
			    const struct tizen_resource_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) tizen_resource,
				     (void (**)(void)) listener, data);
}

#define TIZEN_RESOURCE_DESTROY	0

static inline void
tizen_resource_set_user_data(struct tizen_resource *tizen_resource, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) tizen_resource, user_data);
}

static inline void *
tizen_resource_get_user_data(struct tizen_resource *tizen_resource)
{
	return wl_proxy_get_user_data((struct wl_proxy *) tizen_resource);
}

static inline void
tizen_resource_destroy(struct tizen_resource *tizen_resource)
{
	wl_proxy_marshal((struct wl_proxy *) tizen_resource,
			 TIZEN_RESOURCE_DESTROY);

	wl_proxy_destroy((struct wl_proxy *) tizen_resource);
}

#define TIZEN_SURFACE_EXTENSION_GET_TIZEN_RESOURCE	0

static inline void
tizen_surface_extension_set_user_data(struct tizen_surface_extension *tizen_surface_extension, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) tizen_surface_extension, user_data);
}

static inline void *
tizen_surface_extension_get_user_data(struct tizen_surface_extension *tizen_surface_extension)
{
	return wl_proxy_get_user_data((struct wl_proxy *) tizen_surface_extension);
}

static inline void
tizen_surface_extension_destroy(struct tizen_surface_extension *tizen_surface_extension)
{
	wl_proxy_destroy((struct wl_proxy *) tizen_surface_extension);
}

static inline struct tizen_resource *
tizen_surface_extension_get_tizen_resource(struct tizen_surface_extension *tizen_surface_extension, struct wl_surface *surface)
{
	struct wl_proxy *id;

	id = wl_proxy_marshal_constructor((struct wl_proxy *) tizen_surface_extension,
			 TIZEN_SURFACE_EXTENSION_GET_TIZEN_RESOURCE, &tizen_resource_interface, NULL, surface);

	return (struct tizen_resource *) id;
}

struct tizen_transient_for_listener {
	/**
	 * done - (none)
	 * @child_id: (none)
	 */
	void (*done)(void *data,
		     struct tizen_transient_for *tizen_transient_for,
		     uint32_t child_id);
};

static inline int
tizen_transient_for_add_listener(struct tizen_transient_for *tizen_transient_for,
				 const struct tizen_transient_for_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) tizen_transient_for,
				     (void (**)(void)) listener, data);
}

#define TIZEN_TRANSIENT_FOR_SET	0

static inline void
tizen_transient_for_set_user_data(struct tizen_transient_for *tizen_transient_for, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) tizen_transient_for, user_data);
}

static inline void *
tizen_transient_for_get_user_data(struct tizen_transient_for *tizen_transient_for)
{
	return wl_proxy_get_user_data((struct wl_proxy *) tizen_transient_for);
}

static inline void
tizen_transient_for_destroy(struct tizen_transient_for *tizen_transient_for)
{
	wl_proxy_destroy((struct wl_proxy *) tizen_transient_for);
}

static inline void
tizen_transient_for_set(struct tizen_transient_for *tizen_transient_for, uint32_t child_id, uint32_t parent_id)
{
	wl_proxy_marshal((struct wl_proxy *) tizen_transient_for,
			 TIZEN_TRANSIENT_FOR_SET, child_id, parent_id);
}

#ifdef  __cplusplus
}
#endif

#endif
