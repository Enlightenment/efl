/*
 * Copyright © 2008 Kristian Høgsberg
 * Copyright © 2012-2013 Collabora, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "ecore_wl2_private.h"

struct _dnd_task
{
   void *data;
   Ecore_Fd_Cb cb;
};

struct _dnd_read_ctx
{
   int epoll_fd;
   struct epoll_event *ep;
};

static void
_offer_cb_offer(void *data, struct wl_data_offer *offer EINA_UNUSED, const char *type)
{
   Ecore_Wl2_Dnd_Source *source;
   char **t;

   source = data;
   if ((!source) || (!type)) return;

   t = wl_array_add(&source->types, sizeof(*t));
   if (t) *t = strdup(type);
}

static const struct wl_data_offer_listener _offer_listener =
{
   _offer_cb_offer
};

static void
_source_cb_target_free(void *data EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Data_Source_Target *ev;

   ev = event;
   if (!ev) return;

   free(ev->type);
   free(ev);
}

static void
_source_cb_target(void *data, struct wl_data_source *source EINA_UNUSED, const char *mime_type)
{
   Ecore_Wl2_Input *input;
   Ecore_Wl2_Event_Data_Source_Target *ev;

   input = data;
   if (!input) return;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Data_Source_Target));
   if (!ev) return;

   if (mime_type) ev->type = strdup(mime_type);

   ecore_event_add(ECORE_WL2_EVENT_DATA_SOURCE_TARGET, ev,
                   _source_cb_target_free, NULL);
}

static void
_source_cb_send_free(void *data EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Data_Source_Send *ev;

   ev = event;
   if (!ev) return;

   free(ev->type);
   free(ev);
}

static void
_source_cb_send(void *data, struct wl_data_source *source EINA_UNUSED, const char *mime_type, int32_t fd)
{
   Ecore_Wl2_Input *input;
   Ecore_Wl2_Event_Data_Source_Send *ev;

   input = data;
   if (!input) return;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Data_Source_Send));
   if (!ev) return;

   ev->fd = fd;
   ev->type = strdup(mime_type);

   ecore_event_add(ECORE_WL2_EVENT_DATA_SOURCE_SEND, ev,
                   _source_cb_send_free, NULL);
}

static void
_source_cb_cancelled(void *data, struct wl_data_source *source)
{
   Ecore_Wl2_Input *input;
   Ecore_Wl2_Event_Data_Source_Cancelled *ev;

   input = data;
   if (!input) return;

   if (input->data.source == source) input->data.source = NULL;
   wl_data_source_destroy(source);

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Data_Source_Cancelled));
   if (!ev) return;

   if (input->focus.pointer)
     ev->win = input->focus.pointer->id;
   if (input->focus.keyboard)
     ev->source = input->focus.keyboard->id;

   if (!ev->win) ev->win = ev->source;

   ecore_event_add(ECORE_WL2_EVENT_DATA_SOURCE_CANCELLED, ev, NULL, NULL);
}

static const struct wl_data_source_listener _source_listener =
{
   _source_cb_target,
   _source_cb_send,
   _source_cb_cancelled
};

static void
_selection_data_ready_cb_free(void *data EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Selection_Data_Ready *ev;

   ev = event;
   if (!ev) return;

   free(ev->data);
   free(ev);
}

static Eina_Bool
_selection_data_read(void *data, Ecore_Fd_Handler *hdlr EINA_UNUSED)
{
   int len;
   char buffer[PATH_MAX];
   Ecore_Wl2_Dnd_Source *source;
   Ecore_Wl2_Event_Selection_Data_Ready *event;
   Eina_Bool ret;

   source = data;

   len = read(source->fd, buffer, sizeof buffer);

   event = calloc(1, sizeof(Ecore_Wl2_Event_Selection_Data_Ready));
   if (!event) return ECORE_CALLBACK_CANCEL;

   if (len <= 0)
     {
        close(source->fd);
        _ecore_wl2_dnd_del(source);
        event->done = EINA_TRUE;
        event->data = NULL;
        event->len = 0;
        ret = ECORE_CALLBACK_CANCEL;
     }
   else
     {
        event->data = malloc(len);
        if (!event->data)
          {
             free(event);
             return ECORE_CALLBACK_CANCEL;
          }
        memcpy(event->data, buffer, len);
        event->len = len;
        event->done = EINA_FALSE;
        ret = ECORE_CALLBACK_RENEW;
     }

   ecore_event_add(ECORE_WL2_EVENT_SELECTION_DATA_READY, event,
                   _selection_data_ready_cb_free, NULL);

   return ret;
}

static Eina_Bool
_selection_cb_idle(void *data)
{
   struct _dnd_read_ctx *ctx;
   struct _dnd_task *task;
   int count, i;

   ctx = data;
   count = epoll_wait(ctx->epoll_fd, ctx->ep, 1, 0);
   for (i = 0; i < count; i++)
     {
        task = ctx->ep->data.ptr;
        if (task->cb(task->data, NULL) == ECORE_CALLBACK_CANCEL)
          {
             free(ctx->ep);
             free(task);
             free(ctx);
             return ECORE_CALLBACK_CANCEL;
          }
     }
   return ECORE_CALLBACK_RENEW;
}

static void
_selection_data_receive(Ecore_Wl2_Dnd_Source *source, const char *type)
{
   int epoll_fd;
   struct epoll_event *ep = NULL;
   struct _dnd_task *task = NULL;
   struct _dnd_read_ctx *read_ctx = NULL;
   int p[2];

   if (pipe2(p, O_CLOEXEC) == -1)
     return;

   wl_data_offer_receive(source->offer, type, p[1]);
   close(p[1]);

   /* Due to http://trac.enlightenment.org/e/ticket/1208,
    * use epoll and idle handler instead of ecore_main_fd_handler_add() */

   ep = calloc(1, sizeof(struct epoll_event));
   if (!ep) goto err;

   task = calloc(1, sizeof(struct _dnd_task));
   if (!task) goto err;

   read_ctx = calloc(1, sizeof(struct _dnd_read_ctx));
   if (!read_ctx) goto err;

   epoll_fd  = epoll_create1(0);
   if (epoll_fd < 0) goto err;

   task->data = source;
   task->cb = _selection_data_read;
   ep->events = EPOLLIN;
   ep->data.ptr = task;

   if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, p[0], ep) < 0) goto err;

   read_ctx->epoll_fd = epoll_fd;
   read_ctx->ep = ep;

   if (!ecore_idler_add(_selection_cb_idle, read_ctx)) goto err;

   source->refcount++;
   source->fd = p[0];

   return;

err:
   if (ep) free(ep);
   if (task) free(task);
   if (read_ctx) free(read_ctx);
   close(p[0]);
   return;
}

void
_ecore_wl2_dnd_add(Ecore_Wl2_Input *input, struct wl_data_offer *offer)
{
   Ecore_Wl2_Dnd_Source *source;

   source = calloc(1, sizeof(Ecore_Wl2_Dnd_Source));
   if (!source) return;

   wl_array_init(&source->types);
   source->refcount = 1;
   source->input = input;
   source->offer = offer;

   wl_data_offer_add_listener(source->offer, &_offer_listener, source);
}

void
_ecore_wl2_dnd_enter(Ecore_Wl2_Input *input, struct wl_data_offer *offer, struct wl_surface *surface, int x, int y, uint32_t serial)
{
   Ecore_Wl2_Window *window;
   Ecore_Wl2_Event_Dnd_Enter *ev;
   char **types;
   int num = 0;

   window = _ecore_wl2_display_window_surface_find(input->display, surface);
   if (!window) return;

   if (offer)
     {
        input->drag.source = wl_data_offer_get_user_data(offer);
        num = (input->drag.source->types.size / sizeof(char *));
        types = input->drag.source->types.data;
     }
   else
     {
        input->drag.source = NULL;
        types = NULL;
     }

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Dnd_Enter));
   if (!ev) return;

   ev->win = window->id;

   if (input->focus.keyboard)
     ev->source = input->focus.keyboard->id;

   if (!ev->win) ev->win = ev->source;

   ev->x = x;
   ev->y = y;
   ev->offer = offer;
   ev->serial = serial;
   ev->num_types = num;
   ev->types = types;

   ecore_event_add(ECORE_WL2_EVENT_DND_ENTER, ev, NULL, NULL);
}

void
_ecore_wl2_dnd_leave(Ecore_Wl2_Input *input)
{
   Ecore_Wl2_Event_Dnd_Leave *ev;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Dnd_Leave));
   if (!ev) return;

   if (input->focus.pointer)
     ev->win = input->focus.pointer->id;
   if (input->focus.keyboard)
     ev->source = input->focus.keyboard->id;

   if (!ev->win) ev->win = ev->source;

   ecore_event_add(ECORE_WL2_EVENT_DND_LEAVE, ev, NULL, NULL);
}

void
_ecore_wl2_dnd_motion(Ecore_Wl2_Input *input, int x, int y, uint32_t serial)
{
   Ecore_Wl2_Event_Dnd_Motion *ev;

   input->pointer.sx = x;
   input->pointer.sy = y;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Dnd_Motion));
   if (!ev) return;

   if (input->focus.pointer)
     ev->win = input->focus.pointer->id;
   if (input->focus.keyboard)
     ev->source = input->focus.keyboard->id;

   if (!ev->win) ev->win = ev->source;

   ev->x = x;
   ev->y = y;
   ev->serial = serial;

   ecore_event_add(ECORE_WL2_EVENT_DND_MOTION, ev, NULL, NULL);
}

void
_ecore_wl2_dnd_drop(Ecore_Wl2_Input *input)
{
   Ecore_Wl2_Event_Dnd_Drop *ev;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Dnd_Drop));
   if (!ev) return;

   if (input->drag.source)
     {
        if (input->focus.pointer)
          ev->win = input->focus.pointer->id;
        if (input->focus.keyboard)
          ev->source = input->focus.keyboard->id;

        if (!ev->win) ev->win = ev->source;
     }

   ev->x = input->pointer.sx;
   ev->y = input->pointer.sy;

   ecore_event_add(ECORE_WL2_EVENT_DND_DROP, ev, NULL, NULL);
}

void
_ecore_wl2_dnd_selection(Ecore_Wl2_Input *input, struct wl_data_offer *offer)
{
   if (input->selection.source) _ecore_wl2_dnd_del(input->selection.source);
   input->selection.source = NULL;

   if (offer)
     {
        char **t;

        input->selection.source = wl_data_offer_get_user_data(offer);
        t = wl_array_add(&input->selection.source->types, sizeof(*t));
        *t = NULL;
     }
}

void
_ecore_wl2_dnd_del(Ecore_Wl2_Dnd_Source *source)
{
   if (!source) return;
   source->refcount--;
   if (source->refcount == 0)
     {
        wl_data_offer_destroy(source->offer);
        wl_array_release(&source->types);
        free(source);
     }
}

EAPI void
ecore_wl2_dnd_drag_types_set(Ecore_Wl2_Input *input, const char **types)
{
   struct wl_data_device_manager *manager;
   const char **type;
   char **t;

   EINA_SAFETY_ON_NULL_RETURN(input);
   EINA_SAFETY_ON_NULL_RETURN(input->display);

   manager = input->display->wl.data_device_manager;

   if (input->data.types.data)
     {
        wl_array_for_each(t, &input->data.types)
          free(*t);
        wl_array_release(&input->data.types);
        wl_array_init(&input->data.types);
     }

   if (input->data.source) wl_data_source_destroy(input->data.source);
   input->data.source = NULL;

   input->data.source = wl_data_device_manager_create_data_source(manager);
   if (!input->data.source)
     {
        ERR("Could not create data source");
        return;
     }

   for (type = types; *type; type++)
     {
        if (!*type) continue;
        t = wl_array_add(&input->data.types, sizeof(*t));
        if (t)
          {
             *t = strdup(*type);
             wl_data_source_offer(input->data.source, *t);
          }
     }
}

EAPI void
ecore_wl2_dnd_drag_start(Ecore_Wl2_Input *input, Ecore_Wl2_Window *window, Ecore_Wl2_Window *drag_window)
{
   struct wl_surface *dsurface, *osurface;

   EINA_SAFETY_ON_NULL_RETURN(input);
   EINA_SAFETY_ON_NULL_RETURN(input->data.source);
   EINA_SAFETY_ON_NULL_RETURN(drag_window);

   dsurface = ecore_wl2_window_surface_get(drag_window);

   _ecore_wl2_input_ungrab(input);

   wl_data_source_add_listener(input->data.source, &_source_listener, input);

   osurface = ecore_wl2_window_surface_get(window);
   if (osurface)
     {
        wl_data_device_start_drag(input->data.device, input->data.source,
                                  osurface, dsurface, input->display->serial);

        ecore_wl2_window_cursor_from_name_set(window, "move");
     }
}

EAPI Eina_Bool
ecore_wl2_dnd_drag_get(Ecore_Wl2_Input *input, const char *type)
{
   char **t;

   EINA_SAFETY_ON_NULL_RETURN_VAL(input, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(input->drag.source, EINA_FALSE);

   wl_array_for_each(t, &input->drag.source->types)
     if (!strcmp(type, *t)) break;

   if (!*t) return EINA_FALSE;

   _selection_data_receive(input->drag.source, type);

   return EINA_TRUE;
}

EAPI void
ecore_wl2_dnd_drag_end(Ecore_Wl2_Input *input)
{
   Ecore_Wl2_Event_Dnd_End *ev;

   EINA_SAFETY_ON_NULL_RETURN(input);

   if (input->data.types.data)
     {
        char **t;

        wl_array_for_each(t, &input->data.types)
          free(*t);
        wl_array_release(&input->data.types);
        wl_array_init(&input->data.types);
     }

   if (input->data.source) wl_data_source_destroy(input->data.source);
   input->data.source = NULL;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Dnd_End));
   if (!ev) return;

   if (input->focus.pointer)
     ev->win = input->focus.pointer->id;
   if (input->focus.keyboard)
     ev->source = input->focus.keyboard->id;

   if (!ev->win) ev->win = ev->source;

   ecore_event_add(ECORE_WL2_EVENT_DND_END, ev, NULL, NULL);
}

EAPI Eina_Bool
ecore_wl2_dnd_selection_owner_has(Ecore_Wl2_Input *input)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(input, EINA_FALSE);

   return (input->selection.source != NULL);
}

EAPI Eina_Bool
ecore_wl2_dnd_selection_set(Ecore_Wl2_Input *input, const char **types)
{
   struct wl_data_device_manager *manager;
   const char **type;
   char **t;

   EINA_SAFETY_ON_NULL_RETURN_VAL(input, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(input->display, EINA_FALSE);

   manager = input->display->wl.data_device_manager;

   if (input->data.types.data)
     {
        wl_array_for_each(t, &input->data.types)
          free(*t);
        wl_array_release(&input->data.types);
        wl_array_init(&input->data.types);
     }

   input->data.source = NULL;

   if (!types[0]) return EINA_FALSE;

   input->data.source = wl_data_device_manager_create_data_source(manager);
   if (!input->data.source)
     {
        ERR("Could not create data source");
        return EINA_FALSE;
     }

   for (type = types; *type; type++)
     {
        if (!*type) continue;
        t = wl_array_add(&input->data.types, sizeof(*t));
        if (t)
          {
             *t = strdup(*type);
             wl_data_source_offer(input->data.source, *t);
          }
     }

   wl_data_source_add_listener(input->data.source, &_source_listener, input);

   wl_data_device_set_selection(input->data.device, input->data.source,
                                input->display->serial);

   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_wl2_dnd_selection_get(Ecore_Wl2_Input *input, const char *type)
{
   char **t;

   EINA_SAFETY_ON_NULL_RETURN_VAL(input, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(input->selection.source, EINA_FALSE);

   for (t = input->selection.source->types.data; *t; t++)
     if (!strcmp(type, *t)) break;

   if (!*t) return EINA_FALSE;

   _selection_data_receive(input->selection.source, type);

   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_wl2_dnd_selection_clear(Ecore_Wl2_Input *input)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(input, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(input->data.device, EINA_FALSE);

   wl_data_device_set_selection(input->data.device,
                                NULL, input->display->serial);

   return EINA_TRUE;
}
