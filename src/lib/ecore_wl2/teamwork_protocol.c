#include <stdlib.h>
#include <stdint.h>
#include "wayland-util.h"

extern const struct wl_interface wl_surface_interface;

static const struct wl_interface *types[] = {
	&wl_surface_interface,
	NULL,
	&wl_surface_interface,
	NULL,
	NULL,
	NULL,
	&wl_surface_interface,
	NULL,
	&wl_surface_interface,
	NULL,
	&wl_surface_interface,
	NULL,
	&wl_surface_interface,
	NULL,
	NULL,
	&wl_surface_interface,
	NULL,
	NULL,
};

static const struct wl_message zwp_teamwork_requests[] = {
	{ "preload_uri", "os", types + 0 },
	{ "activate_uri", "osff", types + 2 },
	{ "deactivate_uri", "os", types + 6 },
	{ "open_uri", "os", types + 8 },
};

static const struct wl_message zwp_teamwork_events[] = {
	{ "fetching_uri", "os", types + 10 },
	{ "completed_uri", "osi", types + 12 },
	{ "fetch_info", "osu", types + 15 },
};

WL_EXPORT const struct wl_interface zwp_teamwork_interface = {
	"zwp_teamwork", 2,
	4, zwp_teamwork_requests,
	3, zwp_teamwork_events,
};

