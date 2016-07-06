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
data_offer_offer(void *data, struct wl_data_offer *wl_data_offer EINA_UNUSED, const char *type)
{
   Ecore_Wl2_Dnd_Source *source = data;
   char **p;

   p = wl_array_add(&source->types, sizeof *p);
   *p = strdup(type);
}

static void
data_offer_source_actions(void *data, struct wl_data_offer *wl_data_offer EINA_UNUSED, uint32_t source_actions)
{
   Ecore_Wl2_Dnd_Source *source = data;

   source->source_actions = source_actions;
}

static void
data_offer_action(void *data, struct wl_data_offer *wl_data_offer EINA_UNUSED, uint32_t dnd_action)
{
   Ecore_Wl2_Dnd_Source *source = data;

   source->dnd_action = dnd_action;
}

static const struct wl_data_offer_listener _offer_listener =
{
   data_offer_offer,
   data_offer_source_actions,
   data_offer_action
};

static void
data_source_target_free(void *data EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Data_Source_Target *ev;

   ev = event;
   if (!ev) return;

   free(ev->type);
   free(ev);
}

static void
data_source_target(void *data, struct wl_data_source *source EINA_UNUSED, const char *mime_type)
{
   Ecore_Wl2_Input *input;
   Ecore_Wl2_Event_Data_Source_Target *ev;

   input = data;
   if (!input) return;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Data_Source_Target));
   if (!ev) return;

   if (mime_type) ev->type = strdup(mime_type);

   ecore_event_add(ECORE_WL2_EVENT_DATA_SOURCE_TARGET, ev,
                   data_source_target_free, NULL);
}

static void
data_source_send_free(void *data EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Data_Source_Send *ev;

   ev = event;
   if (!ev) return;

   free(ev->type);
   free(ev);
}

static void
data_source_send(void *data, struct wl_data_source *source EINA_UNUSED, const char *mime_type, int32_t fd)
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
                   data_source_send_free, NULL);
}

static void
data_source_event_emit(Ecore_Wl2_Input *input, int event)
{
   Ecore_Wl2_Event_Data_Source_End *ev;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Data_Source_End));
   if (!ev) return;

   if (input->focus.pointer)
     ev->win = input->focus.pointer->id;
   else if (input->focus.prev_pointer)
     ev->win = input->focus.prev_pointer->id;
   if (input->focus.keyboard)
     ev->source = input->focus.keyboard->id;

   if (!ev->win) ev->win = ev->source;
   if (input->drag.source)
     ev->action = input->drag.source->dnd_action;

   ecore_event_add(event, ev, NULL, NULL);
}

static void
data_source_cancelled(void *data, struct wl_data_source *source)
{
   Ecore_Wl2_Input *input = data;

   if (input->drag.source)
     input->drag.source->dnd_action = 0;
   if (input->data.source == source) input->data.source = NULL;
   wl_data_source_destroy(source);
   data_source_event_emit(input, ECORE_WL2_EVENT_DATA_SOURCE_END);
}

static void
data_source_dnd_drop_performed(void *data, struct wl_data_source *source EINA_UNUSED)
{
   Ecore_Wl2_Input *input = data;
   data_source_event_emit(input, ECORE_WL2_EVENT_DATA_SOURCE_DROP);
}

static void
data_source_dnd_finished(void *data, struct wl_data_source *source)
{
   Ecore_Wl2_Input *input = data;

   if (input->data.source == source) input->data.source = NULL;
   wl_data_source_destroy(source);
   data_source_event_emit(input, ECORE_WL2_EVENT_DATA_SOURCE_END);
}

static void
data_source_action(void *data, struct wl_data_source *source EINA_UNUSED, uint32_t dnd_action)
{
   Ecore_Wl2_Input *input = data;

   if (input->drag.source)
     input->drag.source->dnd_action = dnd_action;
   data_source_event_emit(input, ECORE_WL2_EVENT_DATA_SOURCE_ACTION);
}

static const struct wl_data_source_listener _source_listener =
{
   data_source_target,
   data_source_send,
   data_source_cancelled,
   data_source_dnd_drop_performed,
   data_source_dnd_finished,
   data_source_action,
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
_selection_data_read(void *data, Ecore_Fd_Handler *fdh)
{
   int len = 0, fd;
   char buffer[PATH_MAX];
   Ecore_Wl2_Dnd_Source *source = data;
   Ecore_Wl2_Event_Selection_Data_Ready *event;

   fd = ecore_main_fd_handler_fd_get(fdh);
   if (fd >= 0)
     len = read(fd, buffer, sizeof buffer);
   else
     return ECORE_CALLBACK_RENEW;

   event = calloc(1, sizeof(Ecore_Wl2_Event_Selection_Data_Ready));
   if (!event) return ECORE_CALLBACK_CANCEL;

   event->sel_type = source->sel_type;
   if (len <= 0)
     {
        if (source->input->drag.source)
          {
             if (source->input->display->wl.data_device_manager_version >=
                 WL_DATA_OFFER_FINISH_SINCE_VERSION)
               wl_data_offer_finish(source->offer);
               wl_data_offer_destroy(source->offer);
          }

        fd = ecore_main_fd_handler_fd_get(source->fdh);
        if (fd >= 0) close(fd);
        ecore_main_fd_handler_del(source->fdh);
        source->fdh = NULL;

        event->data = source->read_data;
        event->len = source->len;
        ecore_event_add(ECORE_WL2_EVENT_SELECTION_DATA_READY, event,
                        _selection_data_ready_cb_free, NULL);

        return ECORE_CALLBACK_CANCEL;
     }
   else
     {
        int old_len = source->len;

        if (!source->read_data)
          {
             source->read_data = malloc(len);
             source->len = len;
          }
        else
          {
             source->len += len;
               source->read_data = realloc(source->read_data, source->len);
            }

        memcpy(((char*)source->read_data) + old_len, buffer, len);
        return ECORE_CALLBACK_RENEW;
     }
}

static void
_selection_data_receive(Ecore_Wl2_Dnd_Source *source, const char *type)
{
   int p[2];

   source->active_read = EINA_TRUE;

   if (pipe2(p, O_CLOEXEC) == -1)
     return;

   wl_data_offer_receive(source->offer, type, p[1]);
   close(p[1]);

   source->fdh =
     ecore_main_fd_handler_file_add(p[0], ECORE_FD_READ | ECORE_FD_ERROR,
                                    _selection_data_read, source, NULL, NULL);
}

void
_ecore_wl2_dnd_add(Ecore_Wl2_Input *input, struct wl_data_offer *offer)
{
   Ecore_Wl2_Dnd_Source *source;

   source = calloc(1, sizeof(Ecore_Wl2_Dnd_Source));
   if (!source) return;

   wl_array_init(&source->types);
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
        input->drag.source->dnd_action =
          WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE;
        num = (input->drag.source->types.size / sizeof(char *));
        types = input->drag.source->types.data;
        if (input->display->wl.data_device_manager_version >=
            WL_DATA_OFFER_SET_ACTIONS_SINCE_VERSION)
          wl_data_offer_set_actions(offer,
               WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY |
               WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE,
               WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE);
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

static void
_delay_offer_destroy(void *user_data, void *event)
{
   Ecore_Wl2_Dnd_Source *source;

   source = user_data;

   if (source && source->offer
       && !source->active_read)
     {
        wl_data_offer_destroy(source->offer);
        source->offer = NULL;
     }

   free(event);
}

void
_ecore_wl2_dnd_leave(Ecore_Wl2_Input *input)
{
   Ecore_Wl2_Event_Dnd_Leave *ev;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Dnd_Leave));
   if (!ev) return;

   if (input->focus.pointer)
     ev->win = input->focus.pointer->id;
   else if (input->focus.prev_pointer)
     ev->win = input->focus.prev_pointer->id;
   if (input->focus.keyboard)
     ev->source = input->focus.keyboard->id;

   if (!ev->win) ev->win = ev->source;

   ecore_event_add(ECORE_WL2_EVENT_DND_LEAVE, ev, _delay_offer_destroy, input->drag.source);
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
   else if (input->focus.prev_pointer)
     ev->win = input->focus.prev_pointer->id;
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
        else if (input->focus.prev_pointer)
          ev->win = input->focus.prev_pointer->id;
        if (input->focus.keyboard)
          ev->source = input->focus.keyboard->id;

        if (!ev->win) ev->win = ev->source;
     }

   ev->x = input->pointer.sx;
   ev->y = input->pointer.sy;

   ecore_event_add(ECORE_WL2_EVENT_DND_DROP, ev, _delay_offer_destroy, input->drag.source);
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
   if (source->fdh)
     {
        int fd;

        fd = ecore_main_fd_handler_fd_get(source->fdh);
        if (fd >= 0)
          close(ecore_main_fd_handler_fd_get(source->fdh));
        ecore_main_fd_handler_del(source->fdh);
     }
   wl_data_offer_destroy(source->offer);
   wl_array_release(&source->types);
   free(source);
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
        if (input->display->wl.data_device_manager_version >= WL_DATA_SOURCE_SET_ACTIONS_SINCE_VERSION)
          wl_data_source_set_actions(input->data.source,
            WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE | WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY);

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

   input->drag.source->sel_type = ECORE_WL2_SELECTION_DND;
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

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Dnd_End));
   if (!ev) return;

   if (input->focus.pointer)
     ev->win = input->focus.pointer->id;
   else if (input->focus.prev_pointer)
     ev->win = input->focus.prev_pointer->id;
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

   input->selection.source->sel_type = ECORE_WL2_SELECTION_CNP;
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
