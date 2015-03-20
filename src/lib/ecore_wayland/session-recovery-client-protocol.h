#ifndef SESSION_RECOVERY_CLIENT_PROTOCOL_H
#define SESSION_RECOVERY_CLIENT_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-client.h"

struct wl_client;
struct wl_resource;

struct session_recovery;

extern const struct wl_interface session_recovery_interface;

struct session_recovery_listener {
	/**
	 * uuid - (none)
	 * @uuid: (none)
	 */
	void (*uuid)(void *data,
		     struct session_recovery *session_recovery,
		     const char *uuid);
};

static inline int
session_recovery_add_listener(struct session_recovery *session_recovery,
			      const struct session_recovery_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) session_recovery,
				     (void (**)(void)) listener, data);
}

#define SESSION_RECOVERY_PROVIDE_UUID	0

static inline void
session_recovery_set_user_data(struct session_recovery *session_recovery, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) session_recovery, user_data);
}

static inline void *
session_recovery_get_user_data(struct session_recovery *session_recovery)
{
	return wl_proxy_get_user_data((struct wl_proxy *) session_recovery);
}

static inline void
session_recovery_destroy(struct session_recovery *session_recovery)
{
	wl_proxy_destroy((struct wl_proxy *) session_recovery);
}

static inline void
session_recovery_provide_uuid(struct session_recovery *session_recovery, const char *uuid)
{
	wl_proxy_marshal((struct wl_proxy *) session_recovery,
			 SESSION_RECOVERY_PROVIDE_UUID, uuid);
}

#ifdef  __cplusplus
}
#endif

#endif
