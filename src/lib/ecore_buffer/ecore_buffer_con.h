#ifndef _ECORE_BUFFER_con_H_
#define _ECORE_BUFFER_con_H_

#include <stdio.h>
#include <wayland-client.h>

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Buffer.h>

#include "bq_mgr_protocol.h"
#include "ecore_buffer_private.h"

Eina_Bool             _ecore_buffer_con_init(void);
void                  _ecore_buffer_con_shutdown(void);
void                  _ecore_buffer_con_init_wait(void);
struct bq_provider   *_ecore_buffer_con_provider_create(const char *name);
struct bq_consumer   *_ecore_buffer_con_consumer_create(const char *name, int queue_size, int w, int h);

#endif
