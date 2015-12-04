#include <stdlib.h>
#include <stdint.h>
#include "wayland-util.h"

extern const struct wl_interface xdg_surface_interface;

static const struct wl_interface *types[] = {
	&xdg_surface_interface,
	NULL,
};

static const struct wl_message draw_modes_requests[] = {
	{ "set_available_draw_modes", "oa", types + 0 },
};

WL_EXPORT const struct wl_interface draw_modes_interface = {
	"draw_modes", 1,
	1, draw_modes_requests,
	0, NULL,
};

