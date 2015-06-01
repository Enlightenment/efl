
#include <stdlib.h>
#include <stdint.h>
#include "wayland-util.h"

extern const struct wl_interface wl_surface_interface;

static const struct wl_interface *types[] = {
   &wl_surface_interface,
   NULL,
   NULL,
   &wl_surface_interface,
   NULL,
   &wl_surface_interface,
   NULL,
   &wl_surface_interface,
   NULL,
   NULL,
   NULL,
};

static const struct wl_message wl_keyrouter_requests[] = {
   { "set_keygrab", "?ouu", types + 0 },
   { "unset_keygrab", "?ou", types + 3 },
   { "get_keygrab_status", "?ou", types + 5 },
};

static const struct wl_message wl_keyrouter_events[] = {
   { "keygrab_notify", "?ouuu", types + 7 },
};

WL_EXPORT const struct wl_interface wl_keyrouter_interface = {
   "wl_keyrouter", 1,
   3, wl_keyrouter_requests,
   1, wl_keyrouter_events,
};

