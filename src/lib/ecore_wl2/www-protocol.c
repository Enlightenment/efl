#include <stdlib.h>
#include <stdint.h>
#include "wayland-util.h"

extern const struct wl_interface wl_surface_interface;
extern const struct wl_interface www_surface_interface;

static const struct wl_interface *types[] = {
	NULL,
	NULL,
	NULL,
	&www_surface_interface,
	&wl_surface_interface,
};

static const struct wl_message www_requests[] = {
	{ "create", "no", types + 3 },
};

WL_EXPORT const struct wl_interface www_interface = {
	"www", 1,
	1, www_requests,
	0, NULL,
};

static const struct wl_message www_surface_requests[] = {
	{ "destroy", "", types + 0 },
};

static const struct wl_message www_surface_events[] = {
	{ "status", "iiu", types + 0 },
	{ "start_drag", "", types + 0 },
	{ "end_drag", "", types + 0 },
};

WL_EXPORT const struct wl_interface www_surface_interface = {
	"www_surface", 1,
	1, www_surface_requests,
	3, www_surface_events,
};

