#include <stdlib.h>
#include <stdint.h>
#include "wayland-util.h"

extern const struct wl_interface tizen_visibility_interface;
extern const struct wl_interface wl_surface_interface;

static const struct wl_interface *types[] = {
	NULL,
	&tizen_visibility_interface,
	&wl_surface_interface,
};

static const struct wl_message tizen_policy_requests[] = {
	{ "get_visibility", "no", types + 1 },
};

WL_EXPORT const struct wl_interface tizen_policy_interface = {
	"tizen_policy", 1,
	1, tizen_policy_requests,
	0, NULL,
};

static const struct wl_message tizen_visibility_requests[] = {
	{ "destroy", "", types + 0 },
};

static const struct wl_message tizen_visibility_events[] = {
	{ "notify", "u", types + 0 },
};

WL_EXPORT const struct wl_interface tizen_visibility_interface = {
	"tizen_visibility", 1,
	1, tizen_visibility_requests,
	1, tizen_visibility_events,
};

