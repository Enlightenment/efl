#include <stdlib.h>
#include <stdint.h>
#include "wayland-util.h"

extern const struct wl_interface wl_surface_interface;

static const struct wl_interface *types[] = {
	&wl_surface_interface,
	&wl_surface_interface,
	NULL,
	&wl_surface_interface,
	NULL,
	&wl_surface_interface,
	NULL,
};

static const struct wl_message zwp_e_session_recovery_requests[] = {
	{ "get_uuid", "o", types + 0 },
	{ "set_uuid", "os", types + 1 },
	{ "destroy_uuid", "os", types + 3 },
};

static const struct wl_message zwp_e_session_recovery_events[] = {
	{ "create_uuid", "os", types + 5 },
};

WL_EXPORT const struct wl_interface zwp_e_session_recovery_interface = {
	"zwp_e_session_recovery", 1,
	3, zwp_e_session_recovery_requests,
	1, zwp_e_session_recovery_events,
};

