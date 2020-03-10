#ifndef BQ_MGR_CLIENT_PROTOCOL_H
#define BQ_MGR_CLIENT_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-client.h"

struct wl_client;
struct wl_resource;

struct bq_mgr;
struct bq_consumer;
struct bq_provider;
struct bq_buffer;

extern const struct wl_interface bq_mgr_interface;
extern const struct wl_interface bq_consumer_interface;
extern const struct wl_interface bq_provider_interface;
extern const struct wl_interface bq_buffer_interface;

#ifndef BQ_MGR_ERROR_ENUM
#define BQ_MGR_ERROR_ENUM
enum bq_mgr_error {
	BQ_MGR_ERROR_INVALID_PERMISSION = 0,
	BQ_MGR_ERROR_INVALID_NAME = 1,
	BQ_MGR_ERROR_ALREADY_USED = 2,
};
#endif /* BQ_MGR_ERROR_ENUM */

#define BQ_MGR_CREATE_CONSUMER	0
#define BQ_MGR_CREATE_PROVIDER	1

static inline void
bq_mgr_set_user_data(struct bq_mgr *bq_mgr, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) bq_mgr, user_data);
}

static inline void *
bq_mgr_get_user_data(struct bq_mgr *bq_mgr)
{
	return wl_proxy_get_user_data((struct wl_proxy *) bq_mgr);
}

static inline void
bq_mgr_destroy(struct bq_mgr *bq_mgr)
{
	wl_proxy_destroy((struct wl_proxy *) bq_mgr);
}

static inline struct bq_consumer *
bq_mgr_create_consumer(struct bq_mgr *bq_mgr, const char *name, int32_t queue_size, int32_t width, int32_t height)
{
	struct wl_proxy *id;

	id = wl_proxy_marshal_constructor((struct wl_proxy *) bq_mgr,
			 BQ_MGR_CREATE_CONSUMER, &bq_consumer_interface, NULL, name, queue_size, width, height);

	return (struct bq_consumer *) id;
}

static inline struct bq_provider *
bq_mgr_create_provider(struct bq_mgr *bq_mgr, const char *name)
{
	struct wl_proxy *id;

	id = wl_proxy_marshal_constructor((struct wl_proxy *) bq_mgr,
			 BQ_MGR_CREATE_PROVIDER, &bq_provider_interface, NULL, name);

	return (struct bq_provider *) id;
}

struct bq_consumer_listener {
	void (*connected)(void *data,
			  struct bq_consumer *bq_consumer);
	void (*disconnected)(void *data,
			     struct bq_consumer *bq_consumer);
	void (*buffer_attached)(void *data,
				struct bq_consumer *bq_consumer,
				struct bq_buffer *buffer,
				const char *engine,
				int32_t width,
				int32_t height,
				int32_t format,
				uint32_t flags);
	void (*set_buffer_id)(void *data,
			      struct bq_consumer *bq_consumer,
			      struct bq_buffer *buffer,
			      int32_t id,
			      int32_t offset0,
			      int32_t stride0,
			      int32_t offset1,
			      int32_t stride1,
			      int32_t offset2,
			      int32_t stride2);
	void (*set_buffer_fd)(void *data,
			      struct bq_consumer *bq_consumer,
			      struct bq_buffer *buffer,
			      int32_t fd,
			      int32_t offset0,
			      int32_t stride0,
			      int32_t offset1,
			      int32_t stride1,
			      int32_t offset2,
			      int32_t stride2);
	void (*buffer_detached)(void *data,
				struct bq_consumer *bq_consumer,
				struct bq_buffer *buffer);
	void (*add_buffer)(void *data,
			   struct bq_consumer *bq_consumer,
			   struct bq_buffer *buffer,
			   uint32_t serial);
};

static inline int
bq_consumer_add_listener(struct bq_consumer *bq_consumer,
			 const struct bq_consumer_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) bq_consumer,
				     (void (**)(void)) listener, data);
}

#define BQ_CONSUMER_RELEASE_BUFFER	0

static inline void
bq_consumer_set_user_data(struct bq_consumer *bq_consumer, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) bq_consumer, user_data);
}

static inline void *
bq_consumer_get_user_data(struct bq_consumer *bq_consumer)
{
	return wl_proxy_get_user_data((struct wl_proxy *) bq_consumer);
}

static inline void
bq_consumer_destroy(struct bq_consumer *bq_consumer)
{
	wl_proxy_destroy((struct wl_proxy *) bq_consumer);
}

static inline void
bq_consumer_release_buffer(struct bq_consumer *bq_consumer, struct bq_buffer *buffer)
{
	wl_proxy_marshal((struct wl_proxy *) bq_consumer,
			 BQ_CONSUMER_RELEASE_BUFFER, buffer);
}

#ifndef BQ_PROVIDER_ERROR_ENUM
#define BQ_PROVIDER_ERROR_ENUM
enum bq_provider_error {
	BQ_PROVIDER_ERROR_OVERFLOW_QUEUE_SIZE = 0,
	BQ_PROVIDER_ERROR_CONNECTION = 1,
};
#endif /* BQ_PROVIDER_ERROR_ENUM */

struct bq_provider_listener {
	void (*connected)(void *data,
			  struct bq_provider *bq_provider,
			  int32_t queue_size,
			  int32_t width,
			  int32_t height);
	void (*disconnected)(void *data,
			     struct bq_provider *bq_provider);
	void (*add_buffer)(void *data,
			   struct bq_provider *bq_provider,
			   struct bq_buffer *buffer,
			   uint32_t serial);
};

static inline int
bq_provider_add_listener(struct bq_provider *bq_provider,
			 const struct bq_provider_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) bq_provider,
				     (void (**)(void)) listener, data);
}

#define BQ_PROVIDER_ATTACH_BUFFER	0
#define BQ_PROVIDER_SET_BUFFER_ID	1
#define BQ_PROVIDER_SET_BUFFER_FD	2
#define BQ_PROVIDER_DETACH_BUFFER	3
#define BQ_PROVIDER_ENQUEUE_BUFFER	4

static inline void
bq_provider_set_user_data(struct bq_provider *bq_provider, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) bq_provider, user_data);
}

static inline void *
bq_provider_get_user_data(struct bq_provider *bq_provider)
{
	return wl_proxy_get_user_data((struct wl_proxy *) bq_provider);
}

static inline void
bq_provider_destroy(struct bq_provider *bq_provider)
{
	wl_proxy_destroy((struct wl_proxy *) bq_provider);
}

static inline struct bq_buffer *
bq_provider_attach_buffer(struct bq_provider *bq_provider, const char *engine, int32_t width, int32_t height, int32_t format, uint32_t flags)
{
	struct wl_proxy *buffer;

	buffer = wl_proxy_marshal_constructor((struct wl_proxy *) bq_provider,
			 BQ_PROVIDER_ATTACH_BUFFER, &bq_buffer_interface, NULL, engine, width, height, format, flags);

	return (struct bq_buffer *) buffer;
}

static inline void
bq_provider_set_buffer_id(struct bq_provider *bq_provider, struct bq_buffer *buffer, int32_t id, int32_t offset0, int32_t stride0, int32_t offset1, int32_t stride1, int32_t offset2, int32_t stride2)
{
	wl_proxy_marshal((struct wl_proxy *) bq_provider,
			 BQ_PROVIDER_SET_BUFFER_ID, buffer, id, offset0, stride0, offset1, stride1, offset2, stride2);
}

static inline void
bq_provider_set_buffer_fd(struct bq_provider *bq_provider, struct bq_buffer *buffer, int32_t fd, int32_t offset0, int32_t stride0, int32_t offset1, int32_t stride1, int32_t offset2, int32_t stride2)
{
	wl_proxy_marshal((struct wl_proxy *) bq_provider,
			 BQ_PROVIDER_SET_BUFFER_FD, buffer, fd, offset0, stride0, offset1, stride1, offset2, stride2);
}

static inline void
bq_provider_detach_buffer(struct bq_provider *bq_provider, struct bq_buffer *buffer)
{
	wl_proxy_marshal((struct wl_proxy *) bq_provider,
			 BQ_PROVIDER_DETACH_BUFFER, buffer);
}

static inline void
bq_provider_enqueue_buffer(struct bq_provider *bq_provider, struct bq_buffer *buffer, uint32_t serial)
{
	wl_proxy_marshal((struct wl_proxy *) bq_provider,
			 BQ_PROVIDER_ENQUEUE_BUFFER, buffer, serial);
}

static inline void
bq_buffer_set_user_data(struct bq_buffer *bq_buffer, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) bq_buffer, user_data);
}

static inline void *
bq_buffer_get_user_data(struct bq_buffer *bq_buffer)
{
	return wl_proxy_get_user_data((struct wl_proxy *) bq_buffer);
}

static inline void
bq_buffer_destroy(struct bq_buffer *bq_buffer)
{
	wl_proxy_destroy((struct wl_proxy *) bq_buffer);
}

#ifdef  __cplusplus
}
#endif

#endif
