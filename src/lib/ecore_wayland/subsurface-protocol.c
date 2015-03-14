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

#include <stdlib.h>
#include <stdint.h>
#include "wayland-util.h"

extern const struct wl_interface wl_subsurface_interface;
extern const struct wl_interface wl_surface_interface;
extern const struct wl_interface wl_surface_interface;
extern const struct wl_interface wl_surface_interface;
extern const struct wl_interface wl_surface_interface;

static const struct wl_interface *types[] = {
        NULL,
        NULL,
        &wl_subsurface_interface,
        &wl_surface_interface,
        &wl_surface_interface,
        &wl_surface_interface,
        &wl_surface_interface,
};

static const struct wl_message wl_subcompositor_requests[] = {
        { "destroy", "", types + 0 },
        { "get_subsurface", "noo", types + 2 },
};

WL_EXPORT const struct wl_interface wl_subcompositor_interface = {
        "wl_subcompositor", 1,
        2, wl_subcompositor_requests,
        0, NULL,
};

static const struct wl_message wl_subsurface_requests[] = {
        { "destroy", "", types + 0 },
        { "set_position", "ii", types + 0 },
        { "place_above", "o", types + 5 },
        { "place_below", "o", types + 6 },
        { "set_sync", "", types + 0 },
        { "set_desync", "", types + 0 },
};

WL_EXPORT const struct wl_interface wl_subsurface_interface = {
        "wl_subsurface", 1,
        6, wl_subsurface_requests,
        0, NULL,
};
