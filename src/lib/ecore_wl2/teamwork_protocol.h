#ifndef TEAMWORK_CLIENT_PROTOCOL_H
#define TEAMWORK_CLIENT_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-client.h"

struct wl_client;
struct wl_resource;

struct wl_surface;
struct zwp_teamwork;

extern const struct wl_interface zwp_teamwork_interface;

struct zwp_teamwork_listener {
	/**
	 * fetching_uri - (none)
	 * @surface: (none)
	 * @uri: (none)
	 */
	void (*fetching_uri)(void *data,
			     struct zwp_teamwork *zwp_teamwork,
			     struct wl_surface *surface,
			     const char *uri);
	/**
	 * completed_uri - (none)
	 * @surface: (none)
	 * @uri: (none)
	 * @valid: 1 if uri can be displayed, else 0
	 */
	void (*completed_uri)(void *data,
			      struct zwp_teamwork *zwp_teamwork,
			      struct wl_surface *surface,
			      const char *uri,
			      int32_t valid);
	/**
	 * fetch_info - (none)
	 * @surface: (none)
	 * @uri: (none)
	 * @progress: percentage of download
	 */
	void (*fetch_info)(void *data,
			   struct zwp_teamwork *zwp_teamwork,
			   struct wl_surface *surface,
			   const char *uri,
			   uint32_t progress);
};

static inline int
zwp_teamwork_add_listener(struct zwp_teamwork *zwp_teamwork,
			  const struct zwp_teamwork_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) zwp_teamwork,
				     (void (**)(void)) listener, data);
}

#define ZWP_TEAMWORK_PRELOAD_URI	0
#define ZWP_TEAMWORK_ACTIVATE_URI	1
#define ZWP_TEAMWORK_DEACTIVATE_URI	2
#define ZWP_TEAMWORK_OPEN_URI	3

#define ZWP_TEAMWORK_PRELOAD_URI_SINCE_VERSION	1
#define ZWP_TEAMWORK_ACTIVATE_URI_SINCE_VERSION	1
#define ZWP_TEAMWORK_DEACTIVATE_URI_SINCE_VERSION	1
#define ZWP_TEAMWORK_OPEN_URI_SINCE_VERSION	1

static inline void
zwp_teamwork_set_user_data(struct zwp_teamwork *zwp_teamwork, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) zwp_teamwork, user_data);
}

static inline void *
zwp_teamwork_get_user_data(struct zwp_teamwork *zwp_teamwork)
{
	return wl_proxy_get_user_data((struct wl_proxy *) zwp_teamwork);
}

static inline uint32_t
zwp_teamwork_get_version(struct zwp_teamwork *zwp_teamwork)
{
	return wl_proxy_get_version((struct wl_proxy *) zwp_teamwork);
}

static inline void
zwp_teamwork_destroy(struct zwp_teamwork *zwp_teamwork)
{
	wl_proxy_destroy((struct wl_proxy *) zwp_teamwork);
}

static inline void
zwp_teamwork_preload_uri(struct zwp_teamwork *zwp_teamwork, struct wl_surface *surface, const char *uri)
{
	wl_proxy_marshal((struct wl_proxy *) zwp_teamwork,
			 ZWP_TEAMWORK_PRELOAD_URI, surface, uri);
}

static inline void
zwp_teamwork_activate_uri(struct zwp_teamwork *zwp_teamwork, struct wl_surface *surface, const char *uri, wl_fixed_t x, wl_fixed_t y)
{
	wl_proxy_marshal((struct wl_proxy *) zwp_teamwork,
			 ZWP_TEAMWORK_ACTIVATE_URI, surface, uri, x, y);
}

static inline void
zwp_teamwork_deactivate_uri(struct zwp_teamwork *zwp_teamwork, struct wl_surface *surface, const char *uri)
{
	wl_proxy_marshal((struct wl_proxy *) zwp_teamwork,
			 ZWP_TEAMWORK_DEACTIVATE_URI, surface, uri);
}

static inline void
zwp_teamwork_open_uri(struct zwp_teamwork *zwp_teamwork, struct wl_surface *surface, const char *uri)
{
	wl_proxy_marshal((struct wl_proxy *) zwp_teamwork,
			 ZWP_TEAMWORK_OPEN_URI, surface, uri);
}

#ifdef  __cplusplus
}
#endif

#endif
