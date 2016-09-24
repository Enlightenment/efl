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

struct _Ecore_Wl2_Offer
{
   Ecore_Wl2_Input *input;
   struct wl_data_offer *offer;
   Eina_Array *mimetypes;
   Ecore_Wl2_Drag_Action actions;
   Ecore_Wl2_Drag_Action action;
   uint32_t serial;
   Ecore_Fd_Handler *read;
   int ref;
   unsigned int window_id;
};

static int
_win_id_get(Ecore_Wl2_Input *input)
{
   int win = 0;

   if (input->focus.pointer)
     win = input->focus.pointer->id;
   else if (input->focus.prev_pointer)
     win = input->focus.prev_pointer->id;
   else if (input->focus.keyboard)
     win = input->focus.keyboard->id;

   if (win == 0)
     ERR("Failed to fetch window id");

   return win;
}

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

   if (input->focus.keyboard)
     ev->source = input->focus.keyboard->id;

   ev->win = _win_id_get(input);
   ev->action = input->data.action;

   ecore_event_add(event, ev, NULL, NULL);
}

static void
data_source_cancelled(void *data, struct wl_data_source *source)
{
   Ecore_Wl2_Input *input = data;

   if (input->data.source == source) input->data.source = NULL;
   input->data.action = WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
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

   input->data.action = dnd_action;
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
_unset_serial(void *user_data, void *event)
{
   Ecore_Wl2_Offer *offer = user_data;

   if (offer)
     offer->serial = 0;

   free(event);
}

void
_ecore_wl2_dnd_enter(Ecore_Wl2_Input *input, struct wl_data_offer *offer, struct wl_surface *surface, int x, int y, uint32_t serial)
{
   Ecore_Wl2_Window *window;
   Ecore_Wl2_Event_Dnd_Enter *ev;

   window = _ecore_wl2_display_window_surface_find(input->display, surface);
   if (!window) return;

   if (offer)
     {
        input->drag = wl_data_offer_get_user_data(offer);

        if (!input->drag)
          {
             ERR("Userdata of offer not found");
             goto emit;
          }

        input->drag->serial = serial;
        input->drag->window_id = window->id;

        if (input->display->wl.data_device_manager_version >=
            WL_DATA_OFFER_SET_ACTIONS_SINCE_VERSION)
          {
             ecore_wl2_offer_actions_set(input->drag,
                ECORE_WL2_DRAG_ACTION_MOVE | ECORE_WL2_DRAG_ACTION_COPY,
                ECORE_WL2_DRAG_ACTION_MOVE);
          }
     }
   else
     {
        input->drag = NULL;
     }

emit:
   ev = calloc(1, sizeof(Ecore_Wl2_Event_Dnd_Enter));
   if (!ev) return;

   if (input->focus.keyboard)
     ev->source = input->focus.keyboard->id;

   ev->win = input->drag->window_id;
   ev->x = x;
   ev->y = y;
   ev->offer = input->drag;

   ecore_event_add(ECORE_WL2_EVENT_DND_ENTER, ev, _unset_serial, input->drag);
}

static void
_delay_offer_destroy(void *user_data, void *event)
{
   Ecore_Wl2_Offer *offer = user_data;

   if (offer)
     _ecore_wl2_offer_unref(offer);

   free(event);
}

void
_ecore_wl2_dnd_leave(Ecore_Wl2_Input *input)
{
   Ecore_Wl2_Event_Dnd_Leave *ev;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Dnd_Leave));
   if (!ev) return;

   if (input->focus.keyboard)
     ev->source = input->focus.keyboard->id;

   ev->win = input->drag->window_id;
   ev->offer = input->drag;
   ev->offer->ref++;

   input->drag->window_id = 0;
   ecore_event_add(ECORE_WL2_EVENT_DND_LEAVE, ev, _delay_offer_destroy, ev->offer);
   input->drag = NULL;
}

void
_ecore_wl2_dnd_motion(Ecore_Wl2_Input *input, int x, int y, uint32_t serial)
{
   Ecore_Wl2_Event_Dnd_Motion *ev;

   input->pointer.sx = x;
   input->pointer.sy = y;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Dnd_Motion));
   if (!ev) return;

   input->drag->serial = serial;

   if (input->focus.keyboard)
     ev->source = input->focus.keyboard->id;

   ev->win = input->drag->window_id;
   ev->x = x;
   ev->y = y;
   ev->offer = input->drag;

   ecore_event_add(ECORE_WL2_EVENT_DND_MOTION, ev, _unset_serial, input->drag);
}

void
_ecore_wl2_dnd_drop(Ecore_Wl2_Input *input)
{
   Ecore_Wl2_Event_Dnd_Drop *ev;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Dnd_Drop));
   if (!ev) return;

   if (input->focus.keyboard)
     ev->source = input->focus.keyboard->id;

   ev->win = input->drag->window_id;
   ev->x = input->pointer.sx;
   ev->y = input->pointer.sy;
   ev->offer = input->drag;

   ecore_event_add(ECORE_WL2_EVENT_DND_DROP, ev, NULL, NULL);
}

void
_ecore_wl2_dnd_selection(Ecore_Wl2_Input *input, struct wl_data_offer *offer)
{
   if (input->selection) _ecore_wl2_offer_unref(input->selection);
   input->selection = NULL;

   if (offer)
     input->selection = wl_data_offer_get_user_data(offer);
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
   if (source->offer)
     {
        wl_data_offer_destroy(source->offer);
        source->offer = NULL;
     }
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

   if (input->focus.keyboard)
     ev->source = input->focus.keyboard->id;


   ev->win = _win_id_get(input);

   ecore_event_add(ECORE_WL2_EVENT_DND_END, ev, NULL, NULL);
}

EAPI Ecore_Wl2_Offer*
ecore_wl2_dnd_selection_get(Ecore_Wl2_Input *input)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(input, NULL);

   return input->selection;
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
ecore_wl2_dnd_selection_clear(Ecore_Wl2_Input *input)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(input, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(input->data.device, EINA_FALSE);

   wl_data_device_set_selection(input->data.device,
                                NULL, input->display->serial);

   return EINA_TRUE;
}

static Ecore_Wl2_Drag_Action
_wl_to_action_convert(uint32_t action)
{
#define PAIR(wl, ac) if (action == wl) return ac;
   PAIR(WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY, ECORE_WL2_DRAG_ACTION_COPY)
   PAIR(WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK, ECORE_WL2_DRAG_ACTION_ASK)
   PAIR(WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE, ECORE_WL2_DRAG_ACTION_MOVE)
   PAIR(WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE, ECORE_WL2_DRAG_ACTION_NONE)
#undef PAIR
   return ECORE_WL2_DRAG_ACTION_NONE;
}

static uint32_t
_action_to_wl_convert(Ecore_Wl2_Drag_Action action)
{
#define PAIR(wl, ac) if (action == ac) return wl;
   PAIR(WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY, ECORE_WL2_DRAG_ACTION_COPY)
   PAIR(WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK, ECORE_WL2_DRAG_ACTION_ASK)
   PAIR(WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE, ECORE_WL2_DRAG_ACTION_MOVE)
   PAIR(WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE, ECORE_WL2_DRAG_ACTION_NONE)
#undef PAIR
   return WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
}
static void
data_offer_offer(void *data, struct wl_data_offer *wl_data_offer EINA_UNUSED, const char *type)
{
   Ecore_Wl2_Offer *offer = data;
   char *str;

   if (type)
     eina_array_push(offer->mimetypes, strdup(type)); /*LEEEAK */
   else
     {
        while((str = eina_array_pop(offer->mimetypes)))
          {
             free(str);
          }
     }
}

static void
data_offer_source_actions(void *data, struct wl_data_offer *wl_data_offer EINA_UNUSED, uint32_t source_actions)
{
   Ecore_Wl2_Offer *offer;
   unsigned int i;
   uint32_t types[] = {WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE,
                       WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY,
                       WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK,
                       WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE};

   offer = data;

   offer->actions = 0;

   for (i = 0; i < sizeof(types); ++i)
   {
      if (source_actions & types[i])
        offer->actions |= _wl_to_action_convert(types[i]);
   }
}

static void
data_offer_action(void *data, struct wl_data_offer *wl_data_offer EINA_UNUSED, uint32_t dnd_action)
{
   Ecore_Wl2_Offer *offer;

   offer = data;
   offer->action = _wl_to_action_convert(dnd_action);
}

static const struct wl_data_offer_listener _offer_listener =
{
   data_offer_offer,
   data_offer_source_actions,
   data_offer_action
};

void
_ecore_wl2_dnd_add(Ecore_Wl2_Input *input, struct wl_data_offer *offer)
{
   Ecore_Wl2_Offer *result;

   result = calloc(1, sizeof(Ecore_Wl2_Offer));
   result->offer = offer;
   result->input = input;
   result->mimetypes = eina_array_new(10);
   result->ref = 1;

   wl_data_offer_add_listener(offer, &_offer_listener, result);
}

EAPI Ecore_Wl2_Drag_Action
ecore_wl2_offer_actions_get(Ecore_Wl2_Offer *offer)
{
   return offer->actions;
}

EAPI void
ecore_wl2_offer_actions_set(Ecore_Wl2_Offer *offer, Ecore_Wl2_Drag_Action actions, Ecore_Wl2_Drag_Action action)
{
   uint32_t val = 0;
   int i = 0;

   EINA_SAFETY_ON_NULL_RETURN(offer);

   for (i = 0; i < ECORE_WL2_DRAG_ACTION_LAST; ++i)
     {
        if (actions & i)
          val |= _action_to_wl_convert(i);
     }

   offer->action = _action_to_wl_convert(action);

   wl_data_offer_set_actions(offer->offer, val, offer->action);
}

EAPI Ecore_Wl2_Drag_Action
ecore_wl2_offer_action_get(Ecore_Wl2_Offer *offer)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(offer, ECORE_WL2_DRAG_ACTION_NONE);
   return offer->action;
}

EAPI Eina_Array*
ecore_wl2_offer_mimes_get(Ecore_Wl2_Offer *offer)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(offer, NULL);
   return offer->mimetypes;
}

static unsigned char
_emit_mime(const void *container EINA_UNUSED, void *elem, void *data)
{
   Ecore_Wl2_Offer *offer = data;

   wl_data_offer_accept(offer->offer, offer->serial, elem);

   return 1;
}

EAPI void
ecore_wl2_offer_mimes_set(Ecore_Wl2_Offer *offer, Eina_Array *mimes)
{
   EINA_SAFETY_ON_NULL_RETURN(offer);

   wl_data_offer_accept(offer->offer, offer->serial, NULL);
   if (mimes)
     eina_array_foreach(mimes, _emit_mime, offer);
}

typedef struct {
   int len;
   void *data;
   Ecore_Wl2_Offer *offer;
} Read_Buffer;

static void
_free_buf(void *user_data, void *event)
{
   Read_Buffer *buf = user_data;

   _ecore_wl2_offer_unref(buf->offer);

   free(buf->data);
   free(user_data);
   free(event);
}

static Eina_Bool
_offer_receive_fd_cb(void *data, Ecore_Fd_Handler *fdh)
{
   Read_Buffer *buf = data;
   int fd = -1;
   char buffer[255];
   int len;

   fd = ecore_main_fd_handler_fd_get(fdh);
   if (fd >= 0)
     len = read(fd, buffer, sizeof(buffer));
   else
     return ECORE_CALLBACK_RENEW;

   if (len > 0)
     {
        int old_len = buf->len;

        buf->len += len;
        buf->data = realloc(buf->data, buf->len);

        memcpy(((char*)buf->data) + old_len, buffer, len);
        return ECORE_CALLBACK_RENEW;
     }
   else
     {
        Ecore_Wl2_Event_Offer_Data_Ready *ev;

        ev = calloc(1, sizeof(Ecore_Wl2_Event_Offer_Data_Ready));
        ev->offer = buf->offer;

        ev->data = buf->data;
        ev->len = buf->len;

        ecore_event_add(ECORE_WL2_EVENT_OFFER_DATA_READY, ev, _free_buf, buf);

        buf->offer->read = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
}

EAPI Eina_Bool
ecore_wl2_offer_receive(Ecore_Wl2_Offer *offer, char *mime)
{
   Read_Buffer *buffer;
   int pipe[2];

   EINA_SAFETY_ON_NULL_RETURN_VAL(offer, EINA_FALSE);

   //if a read is going on exit
   if (offer->read) return EINA_FALSE;

   buffer = calloc(1, sizeof(Read_Buffer));
   buffer->offer = offer;

   // no data yet, we would have to fetch it and then tell when the data is ready
   if (pipe2(pipe, O_CLOEXEC) == -1)
     return EINA_FALSE;

   offer->ref ++; // we are keeping this ref until the read is done AND emitted

   wl_data_offer_receive(offer->offer, mime, pipe[1]);
   close(pipe[1]);

   offer->read =
     ecore_main_fd_handler_file_add(pipe[0], ECORE_FD_READ | ECORE_FD_ERROR,
                                    _offer_receive_fd_cb, buffer, NULL, NULL);
   return EINA_FALSE;
}

EAPI void
ecore_wl2_offer_finish(Ecore_Wl2_Offer *offer)
{
   EINA_SAFETY_ON_NULL_RETURN(offer);

   wl_data_offer_finish(offer->offer);
}

void
_ecore_wl2_offer_unref(Ecore_Wl2_Offer *offer)
{
   char *str;

   EINA_SAFETY_ON_NULL_RETURN(offer);

   offer->ref--;

   if (offer->ref > 0) return;

   wl_data_offer_destroy(offer->offer);

   if (offer->mimetypes)
     {
        while((str = eina_array_pop(offer->mimetypes)))
          free(str);
        eina_array_free(offer->mimetypes);
        offer->mimetypes = NULL;
     }

   if (offer->input->drag == offer) offer->input->drag = NULL;
   if (offer->input->selection == offer) offer->input->selection = NULL;

   free(offer);
}

static unsigned char
_compare(const void *container EINA_UNUSED, void *elem, void *data)
{
   if (!strcmp(elem, data))
     return EINA_FALSE;
   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_wl2_offer_supprts_mime(Ecore_Wl2_Offer *offer, const char *mime)
{
  EINA_SAFETY_ON_NULL_RETURN_VAL(offer, EINA_FALSE);
  EINA_SAFETY_ON_NULL_RETURN_VAL(mime, EINA_FALSE);

  return !eina_array_foreach(offer->mimetypes, _compare, (void*) mime);
}