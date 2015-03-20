#include <stdlib.h>
#include <stdint.h>
#include "wayland-util.h"


static const struct wl_interface *types[] = {
	NULL,
};

static const struct wl_message session_recovery_requests[] = {
	{ "provide_uuid", "s", types + 0 },
};

static const struct wl_message session_recovery_events[] = {
	{ "uuid", "s", types + 0 },
};

WL_EXPORT const struct wl_interface session_recovery_interface = {
	"session_recovery", 1,
	1, session_recovery_requests,
	1, session_recovery_events,
};

