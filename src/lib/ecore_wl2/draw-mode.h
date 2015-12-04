#ifndef ZWP_DRAW_MODES_CLIENT_PROTOCOL_H
#define ZWP_DRAW_MODES_CLIENT_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-client.h"

struct wl_client;
struct wl_resource;

struct draw_modes;

extern const struct wl_interface draw_modes_interface;

#ifndef DRAW_MODES_STATE_ENUM
#define DRAW_MODES_STATE_ENUM
/**
 * draw_modes_state - the surface has CSD without dropshadow
 * @DRAW_MODES_STATE_DRAW_NOSHADOW: CSD with no dropshadow
 *
 * The surface contains a CSD region which does not include a dropshadow.
 */
enum draw_modes_state {
	DRAW_MODES_STATE_DRAW_NOSHADOW = 0x2000,
};
#endif /* DRAW_MODES_STATE_ENUM */

#define DRAW_MODES_SET_AVAILABLE_DRAW_MODES	0

static inline void
draw_modes_set_user_data(struct draw_modes *draw_modes, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) draw_modes, user_data);
}

static inline void *
draw_modes_get_user_data(struct draw_modes *draw_modes)
{
	return wl_proxy_get_user_data((struct wl_proxy *) draw_modes);
}

static inline void
draw_modes_destroy(struct draw_modes *draw_modes)
{
	wl_proxy_destroy((struct wl_proxy *) draw_modes);
}

static inline void
draw_modes_set_available_draw_modes(struct draw_modes *draw_modes, struct xdg_surface *surface, struct wl_array *states)
{
	wl_proxy_marshal((struct wl_proxy *) draw_modes,
			 DRAW_MODES_SET_AVAILABLE_DRAW_MODES, surface, states);
}

#ifdef  __cplusplus
}
#endif

#endif
