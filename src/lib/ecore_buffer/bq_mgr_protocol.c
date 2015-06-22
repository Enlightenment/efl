#include <stdlib.h>
#include <stdint.h>
#include "wayland-util.h"

extern const struct wl_interface bq_buffer_interface;
extern const struct wl_interface bq_consumer_interface;
extern const struct wl_interface bq_provider_interface;

static const struct wl_interface *types[] = {
	NULL,
	NULL,
	NULL,
	&bq_consumer_interface,
	NULL,
	NULL,
	NULL,
	NULL,
	&bq_provider_interface,
	NULL,
	&bq_buffer_interface,
	&bq_buffer_interface,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&bq_buffer_interface,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&bq_buffer_interface,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&bq_buffer_interface,
	&bq_buffer_interface,
	NULL,
	&bq_buffer_interface,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&bq_buffer_interface,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&bq_buffer_interface,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&bq_buffer_interface,
	&bq_buffer_interface,
	NULL,
	&bq_buffer_interface,
	NULL,
};

static const struct wl_message bq_mgr_requests[] = {
	{ "create_consumer", "nsiii", types + 3 },
	{ "create_provider", "ns", types + 8 },
};

WL_EXPORT const struct wl_interface bq_mgr_interface = {
	"bq_mgr", 1,
	2, bq_mgr_requests,
	0, NULL,
};

static const struct wl_message bq_consumer_requests[] = {
	{ "release_buffer", "o", types + 10 },
};

static const struct wl_message bq_consumer_events[] = {
	{ "connected", "", types + 0 },
	{ "disconnected", "", types + 0 },
	{ "buffer_attached", "nsiiiu", types + 11 },
	{ "set_buffer_id", "oiiiiiii", types + 17 },
	{ "set_buffer_fd", "ohiiiiii", types + 25 },
	{ "buffer_detached", "o", types + 33 },
	{ "add_buffer", "ou", types + 34 },
};

WL_EXPORT const struct wl_interface bq_consumer_interface = {
	"bq_consumer", 1,
	1, bq_consumer_requests,
	7, bq_consumer_events,
};

static const struct wl_message bq_provider_requests[] = {
	{ "attach_buffer", "nsiiiu", types + 36 },
	{ "set_buffer_id", "oiiiiiii", types + 42 },
	{ "set_buffer_fd", "ohiiiiii", types + 50 },
	{ "detach_buffer", "o", types + 58 },
	{ "enqueue_buffer", "ou", types + 59 },
};

static const struct wl_message bq_provider_events[] = {
	{ "connected", "iii", types + 0 },
	{ "disconnected", "", types + 0 },
	{ "add_buffer", "ou", types + 61 },
};

WL_EXPORT const struct wl_interface bq_provider_interface = {
	"bq_provider", 1,
	5, bq_provider_requests,
	3, bq_provider_events,
};

WL_EXPORT const struct wl_interface bq_buffer_interface = {
	"bq_buffer", 1,
	0, NULL,
	0, NULL,
};

