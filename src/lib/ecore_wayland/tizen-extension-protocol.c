#include <stdlib.h>
#include <stdint.h>
#include "wayland-util.h"

extern const struct wl_interface tizen_resource_interface;
extern const struct wl_interface wl_surface_interface;

static const struct wl_interface *types[] = {
	NULL,
	&tizen_resource_interface,
	&wl_surface_interface,
};

static const struct wl_message tizen_resource_requests[] = {
	{ "destroy", "", types + 0 },
};

static const struct wl_message tizen_resource_events[] = {
	{ "resource_id", "u", types + 0 },
};

WL_EXPORT const struct wl_interface tizen_resource_interface = {
	"tizen_resource", 1,
	1, tizen_resource_requests,
	1, tizen_resource_events,
};

static const struct wl_message tizen_surface_extension_requests[] = {
	{ "get_tizen_resource", "no", types + 1 },
};

WL_EXPORT const struct wl_interface tizen_surface_extension_interface = {
	"tizen_surface_extension", 1,
	1, tizen_surface_extension_requests,
	0, NULL,
};

