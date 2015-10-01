/*
 * Copyright © 2012-2013 Collabora, Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this
 * software and its documentation for any purpose is hereby granted
 * without fee, provided that the above copyright notice appear in
 * all copies and that both that copyright notice and this permission
 * notice appear in supporting documentation, and that the name of
 * the copyright holders not be used in advertising or publicity
 * pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */

#ifndef SUBSURFACE_CLIENT_PROTOCOL_H
#define SUBSURFACE_CLIENT_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-client.h"

struct wl_client;
struct wl_resource;

struct wl_subcompositor;
struct wl_subsurface;

extern const struct wl_interface wl_subcompositor_interface;
extern const struct wl_interface wl_subsurface_interface;

#ifndef WL_SUBCOMPOSITOR_ERROR_ENUM
#define WL_SUBCOMPOSITOR_ERROR_ENUM
enum wl_subcompositor_error {
        WL_SUBCOMPOSITOR_ERROR_BAD_SURFACE = 0,
};
#endif /* WL_SUBCOMPOSITOR_ERROR_ENUM */

#define WL_SUBCOMPOSITOR_DESTROY        0
#define WL_SUBCOMPOSITOR_GET_SUBSURFACE 1

static inline void
wl_subcompositor_set_user_data(struct wl_subcompositor *wl_subcompositor, void *user_data)
{
        wl_proxy_set_user_data((struct wl_proxy *) wl_subcompositor, user_data);
}

static inline void *
wl_subcompositor_get_user_data(struct wl_subcompositor *wl_subcompositor)
{
        return wl_proxy_get_user_data((struct wl_proxy *) wl_subcompositor);
}

static inline void
wl_subcompositor_destroy(struct wl_subcompositor *wl_subcompositor)
{
        wl_proxy_marshal((struct wl_proxy *) wl_subcompositor,
                         WL_SUBCOMPOSITOR_DESTROY);

        wl_proxy_destroy((struct wl_proxy *) wl_subcompositor);
}

static inline struct wl_subsurface *
wl_subcompositor_get_subsurface(struct wl_subcompositor *wl_subcompositor, struct wl_surface *surface, struct wl_surface *parent)
{
        struct wl_proxy *id;

        id = wl_proxy_create((struct wl_proxy *) wl_subcompositor,
                             &wl_subsurface_interface);
        if (!id)
                return NULL;

        wl_proxy_marshal((struct wl_proxy *) wl_subcompositor,
                         WL_SUBCOMPOSITOR_GET_SUBSURFACE, id, surface, parent);

        return (struct wl_subsurface *) id;
}

#ifndef WL_SUBSURFACE_ERROR_ENUM
#define WL_SUBSURFACE_ERROR_ENUM
enum wl_subsurface_error {
        WL_SUBSURFACE_ERROR_BAD_SURFACE = 0,
};
#endif /* WL_SUBSURFACE_ERROR_ENUM */

#define WL_SUBSURFACE_DESTROY   0
#define WL_SUBSURFACE_SET_POSITION      1
#define WL_SUBSURFACE_PLACE_ABOVE       2
#define WL_SUBSURFACE_PLACE_BELOW       3
#define WL_SUBSURFACE_SET_SYNC  4
#define WL_SUBSURFACE_SET_DESYNC        5

static inline void
wl_subsurface_set_user_data(struct wl_subsurface *wl_subsurface, void *user_data)
{
        wl_proxy_set_user_data((struct wl_proxy *) wl_subsurface, user_data);
}

static inline void *
wl_subsurface_get_user_data(struct wl_subsurface *wl_subsurface)
{
        return wl_proxy_get_user_data((struct wl_proxy *) wl_subsurface);
}

static inline void
wl_subsurface_destroy(struct wl_subsurface *wl_subsurface)
{
        wl_proxy_marshal((struct wl_proxy *) wl_subsurface,
                         WL_SUBSURFACE_DESTROY);

        wl_proxy_destroy((struct wl_proxy *) wl_subsurface);
}

static inline void
wl_subsurface_set_position(struct wl_subsurface *wl_subsurface, int32_t x, int32_t y)
{
        wl_proxy_marshal((struct wl_proxy *) wl_subsurface,
                         WL_SUBSURFACE_SET_POSITION, x, y);
}

static inline void
wl_subsurface_place_above(struct wl_subsurface *wl_subsurface, struct wl_surface *sibling)
{
        wl_proxy_marshal((struct wl_proxy *) wl_subsurface,
                         WL_SUBSURFACE_PLACE_ABOVE, sibling);
}

static inline void
wl_subsurface_place_below(struct wl_subsurface *wl_subsurface, struct wl_surface *sibling)
{
        wl_proxy_marshal((struct wl_proxy *) wl_subsurface,
                         WL_SUBSURFACE_PLACE_BELOW, sibling);
}

static inline void
wl_subsurface_set_sync(struct wl_subsurface *wl_subsurface)
{
        wl_proxy_marshal((struct wl_proxy *) wl_subsurface,
                         WL_SUBSURFACE_SET_SYNC);
}

static inline void
wl_subsurface_set_desync(struct wl_subsurface *wl_subsurface)
{
        wl_proxy_marshal((struct wl_proxy *) wl_subsurface,
                         WL_SUBSURFACE_SET_DESYNC);
}

#ifdef  __cplusplus
}
#endif

#endif
