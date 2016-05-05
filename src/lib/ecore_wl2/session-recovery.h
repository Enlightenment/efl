#ifndef E_SESSION_RECOVERY_CLIENT_PROTOCOL_H
#define E_SESSION_RECOVERY_CLIENT_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-client.h"

struct wl_client;
struct wl_resource;

struct wl_surface;
struct zwp_e_session_recovery;

extern const struct wl_interface zwp_e_session_recovery_interface;

struct zwp_e_session_recovery_listener {
	/**
	 * create_uuid - (none)
	 * @surface: (none)
	 * @uuid: (none)
	 */
	void (*create_uuid)(void *data,
			    struct zwp_e_session_recovery *zwp_e_session_recovery,
			    struct wl_surface *surface,
			    const char *uuid);
};

static inline int
zwp_e_session_recovery_add_listener(struct zwp_e_session_recovery *zwp_e_session_recovery,
				    const struct zwp_e_session_recovery_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) zwp_e_session_recovery,
				     (void (**)(void)) listener, data);
}

#define ZWP_E_SESSION_RECOVERY_GET_UUID	0
#define ZWP_E_SESSION_RECOVERY_SET_UUID	1
#define ZWP_E_SESSION_RECOVERY_DESTROY_UUID	2

#define ZWP_E_SESSION_RECOVERY_GET_UUID_SINCE_VERSION	1
#define ZWP_E_SESSION_RECOVERY_SET_UUID_SINCE_VERSION	1
#define ZWP_E_SESSION_RECOVERY_DESTROY_UUID_SINCE_VERSION	1

static inline void
zwp_e_session_recovery_set_user_data(struct zwp_e_session_recovery *zwp_e_session_recovery, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) zwp_e_session_recovery, user_data);
}

static inline void *
zwp_e_session_recovery_get_user_data(struct zwp_e_session_recovery *zwp_e_session_recovery)
{
	return wl_proxy_get_user_data((struct wl_proxy *) zwp_e_session_recovery);
}

static inline uint32_t
zwp_e_session_recovery_get_version(struct zwp_e_session_recovery *zwp_e_session_recovery)
{
	return wl_proxy_get_version((struct wl_proxy *) zwp_e_session_recovery);
}

static inline void
zwp_e_session_recovery_destroy(struct zwp_e_session_recovery *zwp_e_session_recovery)
{
	wl_proxy_destroy((struct wl_proxy *) zwp_e_session_recovery);
}

static inline void
zwp_e_session_recovery_get_uuid(struct zwp_e_session_recovery *zwp_e_session_recovery, struct wl_surface *surface)
{
	wl_proxy_marshal((struct wl_proxy *) zwp_e_session_recovery,
			 ZWP_E_SESSION_RECOVERY_GET_UUID, surface);
}

static inline void
zwp_e_session_recovery_set_uuid(struct zwp_e_session_recovery *zwp_e_session_recovery, struct wl_surface *surface, const char *uuid)
{
	wl_proxy_marshal((struct wl_proxy *) zwp_e_session_recovery,
			 ZWP_E_SESSION_RECOVERY_SET_UUID, surface, uuid);
}

static inline void
zwp_e_session_recovery_destroy_uuid(struct zwp_e_session_recovery *zwp_e_session_recovery, struct wl_surface *surface, const char *uuid)
{
	wl_proxy_marshal((struct wl_proxy *) zwp_e_session_recovery,
			 ZWP_E_SESSION_RECOVERY_DESTROY_UUID, surface, uuid);
}

#ifdef  __cplusplus
}
#endif

#endif
