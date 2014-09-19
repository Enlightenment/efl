#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <fcntl.h>
#include <sys/epoll.h>
#include "ecore_wl_private.h"

/* local structures */
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

/* local function prototypes */
static void _ecore_wl_dnd_selection_data_receive(Ecore_Wl_Dnd_Source *source, const char *type);
static Eina_Bool _ecore_wl_dnd_selection_data_read(void *data, Ecore_Fd_Handler *fd_handler EINA_UNUSED);
static void _ecore_wl_dnd_selection_data_ready_cb_free(void *data EINA_UNUSED, void *event);
static Eina_Bool _ecore_wl_dnd_selection_cb_idle(void *data);

static void _ecore_wl_dnd_source_cb_target(void *data, struct wl_data_source *source EINA_UNUSED, const char *mime_type EINA_UNUSED);
static void _ecore_wl_dnd_source_cb_send(void *data, struct wl_data_source *source EINA_UNUSED, const char *mime_type, int32_t fd);
static void _ecore_wl_dnd_source_cb_send_free(void *data EINA_UNUSED, void *event);
static void _ecore_wl_dnd_source_cb_cancelled(void *data EINA_UNUSED, struct wl_data_source *source);

static void _ecore_wl_dnd_offer_cb_offer(void *data, struct wl_data_offer *data_offer EINA_UNUSED, const char *type);

/* local wayland interfaces */
static const struct wl_data_source_listener 
_ecore_wl_dnd_source_listener = 
{
   _ecore_wl_dnd_source_cb_target,
   _ecore_wl_dnd_source_cb_send,
   _ecore_wl_dnd_source_cb_cancelled
};

static const struct wl_data_offer_listener 
_ecore_wl_dnd_offer_listener = 
{
   _ecore_wl_dnd_offer_cb_offer
};

/**
 * @deprecated use ecore_wl_dnd_selection_set
 * @since 1.7 
*/
EINA_DEPRECATED EAPI Eina_Bool 
ecore_wl_dnd_set_selection(Ecore_Wl_Dnd *dnd, const char **types_offered)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return ecore_wl_dnd_selection_set(dnd->input, types_offered);
}

/**
 * @deprecated use ecore_wl_dnd_selection_get
 * @since 1.7 
*/
EINA_DEPRECATED EAPI Eina_Bool 
ecore_wl_dnd_get_selection(Ecore_Wl_Dnd *dnd, const char *type)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return ecore_wl_dnd_selection_get(dnd->input, type);
}

/**
 * @deprecated Do Not Use
 * @since 1.7
 */
EINA_DEPRECATED EAPI Ecore_Wl_Dnd *
ecore_wl_dnd_get(void)
{
   return NULL;
}

/**
 * @deprecated use ecore_wl_dnd_drag_start
 * @since 1.7
 */
EINA_DEPRECATED EAPI Eina_Bool 
ecore_wl_dnd_start_drag(Ecore_Wl_Dnd *dnd EINA_UNUSED)
{
   return EINA_FALSE;
}

/**
 * @deprecated use ecore_wl_dnd_selection_owner_has
 * @since 1.7
 */
EINA_DEPRECATED EAPI Eina_Bool 
ecore_wl_dnd_selection_has_owner(Ecore_Wl_Dnd *dnd)
{
   return ecore_wl_dnd_selection_owner_has(dnd->input);
}

/**
 * @ingroup Ecore_Wl_Dnd_Group
 * @since 1.8
 */
EAPI Eina_Bool 
ecore_wl_dnd_selection_set(Ecore_Wl_Input *input, const char **types_offered)
{
   struct wl_data_device_manager *man;
   const char **type;
   char **t;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!input) return EINA_FALSE;

   man = input->display->wl.data_device_manager;

   /* free any old types offered */
   if (input->data_types.data)
     {
        wl_array_for_each(t, &input->data_types)
          free(*t);
        wl_array_release(&input->data_types);
        wl_array_init(&input->data_types);
     }

   input->data_source = NULL;

   if (!types_offered[0]) return EINA_FALSE;

   /* try to create a new data source */
   if (!(input->data_source = wl_data_device_manager_create_data_source(man)))
     return EINA_FALSE;

   /* add these types to the data source */
   for (type = types_offered; *type; type++)
     {
        t = wl_array_add(&input->data_types, sizeof(*t));
        if (t) *t = strdup(*type);
        wl_data_source_offer(input->data_source, *t);
     }

   /* add a listener for data source events */
   wl_data_source_add_listener(input->data_source, 
                               &_ecore_wl_dnd_source_listener, input);

   /* set the selection */
   wl_data_device_set_selection(input->data_device, input->data_source, 
                                input->display->serial);

   return EINA_TRUE;
}

/**
 * @ingroup Ecore_Wl_Dnd_Group
 * @since 1.8
 */
EAPI Eina_Bool 
ecore_wl_dnd_selection_get(Ecore_Wl_Input *input, const char *type)
{
   char **t;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* check for valid input and selection source */
   if ((!input) || (!input->selection_source)) return EINA_FALSE;

   for (t = input->selection_source->types.data; *t; t++)
     {
        if (!strcmp(type, *t)) break;
     }

   if (!*t) return EINA_FALSE;

   _ecore_wl_dnd_selection_data_receive(input->selection_source, type);

   return EINA_TRUE;
}

/**
 * @ingroup Ecore_Wl_Dnd_Group
 * @since 1.8
 */
EAPI Eina_Bool 
ecore_wl_dnd_selection_owner_has(Ecore_Wl_Input *input)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!input) return EINA_FALSE;
   return (input->selection_source != NULL);
}

/**
 * @ingroup Ecore_Wl_Dnd_Group
 * @since 1.8
 */
EAPI Eina_Bool 
ecore_wl_dnd_selection_clear(Ecore_Wl_Input *input)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* check for valid input */
   if (!input) return EINA_FALSE;

   /* set the selection to NULL */
   wl_data_device_set_selection(input->data_device, NULL, 
                                input->display->serial);

   return EINA_TRUE;
}

/**
 * @ingroup Ecore_Wl_Dnd_Group
 * @since 1.8
 */
EAPI void 
ecore_wl_dnd_drag_start(Ecore_Wl_Input *input, Ecore_Wl_Window *win, Ecore_Wl_Window *dragwin, int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED)
{
   struct wl_surface *drag_surface;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* check for valid input. if not, get the default one */
   if (!input) input = _ecore_wl_disp->input;

   /* check for valid data source */
   if (!input->data_source) return;

   /* get the surface from this drag window */
   drag_surface = ecore_wl_window_surface_get(dragwin);

   /* release any existing grabs */
   ecore_wl_input_ungrab(input);

   /* add a listener for data source events */
   wl_data_source_add_listener(input->data_source, 
                               &_ecore_wl_dnd_source_listener, input);

   /* start the drag */
   wl_data_device_start_drag(input->data_device, input->data_source, 
                             ecore_wl_window_surface_get(win), 
                             drag_surface, input->display->serial);

   /* set pointer image */
   ecore_wl_input_cursor_from_name_set(input, "move");

   /* NB: Below code disabled for now
    * 
    * This Was for adjusting the "drag icon" to be centered on the mouse 
    * based on the hotspot, but it crashes for some reason :(
    */

   /* struct wl_buffer *drag_buffer; */
   /* struct wl_cursor_image *cursor; */
   /* int cx = 0, cy = 0; */
   /* drag_buffer = wl_surface_get_user_data(drag_surface); */
   /* cursor = input->cursor->images[input->cursor_current_index]; */
   /* cx = cursor->hotspot_x - x; */
   /* cy = cursor->hotspot_y - y; */
   /* wl_surface_attach(drag_surface, drag_buffer, cx, cy); */
   /* wl_surface_damage(drag_surface, 0, 0, w, h); */
   /* wl_surface_commit(drag_surface); */
}

/**
 * @ingroup Ecore_Wl_Dnd_Group
 * @since 1.8
 */
EAPI void 
ecore_wl_dnd_drag_end(Ecore_Wl_Input *input)
{
   Ecore_Wl_Event_Dnd_End *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* check for valid input. if not, get the default one */
   if (!input) input = _ecore_wl_disp->input;

   if (input->data_types.data)
     {
        char **t;

        wl_array_for_each(t, &input->data_types)
          free(*t);
        wl_array_release(&input->data_types);
        wl_array_init(&input->data_types);
     }

   /* if (input->drag_source) _ecore_wl_dnd_del(input->drag_source); */
   /* input->drag_source = NULL; */

   /* destroy any existing data source */
   if (input->data_source) wl_data_source_destroy(input->data_source);
   input->data_source = NULL;

   if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Dnd_End)))) return;

   if (input->pointer_focus)
     ev->win = input->pointer_focus->id;

   if (input->keyboard_focus)
     ev->source = input->keyboard_focus->id;

   ecore_event_add(ECORE_WL_EVENT_DND_END, ev, NULL, NULL);
}

/**
 * @ingroup Ecore_Wl_Dnd_Group
 * @since 1.8
 */
EAPI Eina_Bool 
ecore_wl_dnd_drag_get(Ecore_Wl_Input *input, const char *type)
{
   char **t;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* check for valid input and drag source */
   if ((!input) || (!input->drag_source)) return EINA_FALSE;

   wl_array_for_each(t, &input->drag_source->types)
     if (!strcmp(type, *t)) break;

   if (!*t) return EINA_FALSE;

   _ecore_wl_dnd_selection_data_receive(input->drag_source, type);

   return EINA_TRUE;
}

/**
 * @ingroup Ecore_Wl_Dnd_Group
 * @since 1.8
 */
EAPI void 
ecore_wl_dnd_drag_types_set(Ecore_Wl_Input *input, const char **types_offered)
{
   struct wl_data_device_manager *man;
   const char **type;
   char **t;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* check for valid input. if not, get the default one */
   if (!input) input = _ecore_wl_disp->input;

   man = input->display->wl.data_device_manager;

   /* free any old types offered */
   if (input->data_types.data)
     {
        wl_array_for_each(t, &input->data_types)
          free(*t);
        wl_array_release(&input->data_types);
        wl_array_init(&input->data_types);
     }

   /* destroy any existing data source */
   if (input->data_source) wl_data_source_destroy(input->data_source);
   input->data_source = NULL;

   /* try to create a new data source */
   if (!(input->data_source = wl_data_device_manager_create_data_source(man)))
     {
        printf("Failed to create new data source for drag\n");
        return;
     }

   /* add these types to the data source */
   for (type = types_offered; *type; type++)
     {
        if (!*type) continue;
        t = wl_array_add(&input->data_types, sizeof(*t));
        if (t) 
          {
             *t = strdup(*type);
             wl_data_source_offer(input->data_source, *t);
          }
     }
}

/**
 * @ingroup Ecore_Wl_Dnd_Group
 * @since 1.8
 */
EAPI struct wl_array *
ecore_wl_dnd_drag_types_get(Ecore_Wl_Input *input)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* check for valid input. if not, get the default one */
   if (!input) input = _ecore_wl_disp->input;

   return &input->data_types;
}

/* private functions */
void 
_ecore_wl_dnd_add(Ecore_Wl_Input *input, struct wl_data_device *data_device EINA_UNUSED, struct wl_data_offer *offer)
{
   Ecore_Wl_Dnd_Source *source;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(source = malloc(sizeof(Ecore_Wl_Dnd_Source))))
     return;

   wl_array_init(&source->types);
   source->refcount = 1;
   source->input = input;
   source->data_offer = offer;

   wl_data_offer_add_listener(source->data_offer, 
                              &_ecore_wl_dnd_offer_listener, source);
}

void 
_ecore_wl_dnd_enter(void *data, struct wl_data_device *data_device EINA_UNUSED, unsigned int timestamp, struct wl_surface *surface, int x, int y, struct wl_data_offer *offer)
{
   Ecore_Wl_Event_Dnd_Enter *ev;
   Ecore_Wl_Window *win;
   Ecore_Wl_Input *input;
   char **types;
   int num = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   win = ecore_wl_window_surface_find(surface);

   input->pointer_enter_serial = timestamp;
   input->pointer_focus = win;

   if (offer)
     {
        input->drag_source = wl_data_offer_get_user_data(offer);

        num = (input->drag_source->types.size / sizeof(char *));
        types = input->drag_source->types.data;
     }
   else
     {
        input->drag_source = NULL;
        types = NULL;
     }

   if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Dnd_Enter)))) return;

   if (win) 
     ev->win = win->id;

   if (input->keyboard_focus)
     ev->source = input->keyboard_focus->id;

   ev->offer = offer;
   ev->serial = timestamp;
   ev->position.x = wl_fixed_to_int(x);
   ev->position.y = wl_fixed_to_int(y);
   ev->num_types = num;
   ev->types = types;

   ecore_event_add(ECORE_WL_EVENT_DND_ENTER, ev, NULL, NULL);
}

void 
_ecore_wl_dnd_leave(void *data, struct wl_data_device *data_device EINA_UNUSED)
{
   Ecore_Wl_Event_Dnd_Leave *ev;
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Dnd_Leave)))) return;

   if (input->pointer_focus)
     ev->win = input->pointer_focus->id;

   if (input->keyboard_focus)
     ev->source = input->keyboard_focus->id;

   ecore_event_add(ECORE_WL_EVENT_DND_LEAVE, ev, NULL, NULL);
}

void 
_ecore_wl_dnd_motion(void *data, struct wl_data_device *data_device EINA_UNUSED, unsigned int timestamp EINA_UNUSED, int x, int y)
{
   Ecore_Wl_Event_Dnd_Position *ev;
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   input->sx = wl_fixed_to_int(x);
   input->sy = wl_fixed_to_int(y);

   if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Dnd_Position)))) return;

   if (input->pointer_focus)
     ev->win = input->pointer_focus->id;

   if (input->keyboard_focus)
     ev->source = input->keyboard_focus->id;

   ev->position.x = input->sx;
   ev->position.y = input->sy;

   ecore_event_add(ECORE_WL_EVENT_DND_POSITION, ev, NULL, NULL);
}

void 
_ecore_wl_dnd_drop(void *data, struct wl_data_device *data_device EINA_UNUSED)
{
   Ecore_Wl_Event_Dnd_Drop *ev;
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Dnd_Drop)))) return;

   if (input->drag_source)
     {
        if (input->pointer_focus)
          ev->win = input->pointer_focus->id;
        if (input->keyboard_focus)
          ev->source = input->keyboard_focus->id;
     }

   ev->position.x = input->sx;
   ev->position.y = input->sy;

   ecore_event_add(ECORE_WL_EVENT_DND_DROP, ev, NULL, NULL);
}

void 
_ecore_wl_dnd_selection(void *data, struct wl_data_device *data_device EINA_UNUSED, struct wl_data_offer *offer)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   if (input->selection_source) _ecore_wl_dnd_del(input->selection_source);
   input->selection_source = NULL;

   if (offer)
     {
        char **t;

        input->selection_source = wl_data_offer_get_user_data(offer);
        t = wl_array_add(&input->selection_source->types, sizeof(*t));
        *t = NULL;
     }
}

void 
_ecore_wl_dnd_del(Ecore_Wl_Dnd_Source *source)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!source) return;
   source->refcount--;
   if (source->refcount == 0)
     {
        wl_data_offer_destroy(source->data_offer);
        wl_array_release(&source->types);
        free(source);
     }
}

/* local functions */
static void 
_ecore_wl_dnd_selection_data_receive(Ecore_Wl_Dnd_Source *source, const char *type)
{
   int epoll_fd;
   struct epoll_event *ep = NULL;
   struct _dnd_task *task = NULL;
   struct _dnd_read_ctx *read_ctx = NULL;
   int p[2];

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (pipe2(p, O_CLOEXEC) == -1)
     return;

   wl_data_offer_receive(source->data_offer, type, p[1]);
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
   task->cb = _ecore_wl_dnd_selection_data_read;
   ep->events = EPOLLIN;
   ep->data.ptr = task;

   if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, p[0], ep) < 0) goto err;

   read_ctx->epoll_fd = epoll_fd;
   read_ctx->ep = ep;

   if (!ecore_idler_add(_ecore_wl_dnd_selection_cb_idle, read_ctx)) goto err;

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

static Eina_Bool 
_ecore_wl_dnd_selection_data_read(void *data, Ecore_Fd_Handler *fd_handler EINA_UNUSED)
{
   int len;
   char buffer[PATH_MAX];
   Ecore_Wl_Dnd_Source *source;
   Ecore_Wl_Event_Selection_Data_Ready *event;
   Eina_Bool ret;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   source = data;

   len = read(source->fd, buffer, sizeof buffer);

   if (!(event = calloc(1, sizeof(Ecore_Wl_Event_Selection_Data_Ready)))) 
     return ECORE_CALLBACK_CANCEL;

   if (len <= 0)
     {
        close(source->fd);
        _ecore_wl_dnd_del(source);
        event->done = EINA_TRUE;
        event->data = NULL;
        event->len = 0;
        ret = ECORE_CALLBACK_CANCEL;
     }
   else
     {
        event->data = malloc(len + 1);
        if (!event->data)
          {
             free(event);
             return ECORE_CALLBACK_CANCEL;
          }
        strncpy(event->data, buffer, len);
        event->data[len] = '\0';
        event->len = len;
        event->done = EINA_FALSE;
        ret = ECORE_CALLBACK_RENEW;
     }

   ecore_event_add(ECORE_WL_EVENT_SELECTION_DATA_READY, event, 
                   _ecore_wl_dnd_selection_data_ready_cb_free, NULL);

   return ret;
}

static void 
_ecore_wl_dnd_selection_data_ready_cb_free(void *data EINA_UNUSED, void *event)
{
   Ecore_Wl_Event_Selection_Data_Ready *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = event)) return;

   free(ev->data);
   free(ev);
}

static Eina_Bool
_ecore_wl_dnd_selection_cb_idle(void *data)
{
   struct _dnd_read_ctx *ctx;
   struct _dnd_task *task;
   int count, i;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

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
_ecore_wl_dnd_source_cb_target(void *data, struct wl_data_source *source EINA_UNUSED, const char *mime_type EINA_UNUSED)
{
   Ecore_Wl_Input *input;

   if (!(input = data)) return;

   printf("Dnd Source Target\n");
}

static void 
_ecore_wl_dnd_source_cb_send(void *data, struct wl_data_source *source EINA_UNUSED, const char *mime_type, int32_t fd)
{
   Ecore_Wl_Event_Data_Source_Send *event;
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   if (!(event = calloc(1, sizeof(Ecore_Wl_Event_Data_Source_Send)))) return;

   event->type = strdup(mime_type);
   event->fd = fd;

   ecore_event_add(ECORE_WL_EVENT_DATA_SOURCE_SEND, event, 
                   _ecore_wl_dnd_source_cb_send_free, NULL);
}

static void 
_ecore_wl_dnd_source_cb_send_free(void *data EINA_UNUSED, void *event)
{
   Ecore_Wl_Event_Data_Source_Send *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = event)) return;

   free(ev->type);
   free(ev);
}

static void 
_ecore_wl_dnd_source_cb_cancelled(void *data, struct wl_data_source *source)
{
   Ecore_Wl_Input *input;
   Ecore_Wl_Event_Data_Source_Cancelled *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   wl_data_source_destroy(source);
   if (input->data_source == source) input->data_source = NULL;

   if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Data_Source_Cancelled)))) return;

   if (input->pointer_focus)
     ev->win = input->pointer_focus->id;

   if (input->keyboard_focus)
     ev->source = input->keyboard_focus->id;

   ecore_event_add(ECORE_WL_EVENT_DATA_SOURCE_CANCELLED, ev, NULL, NULL);
}

static void 
_ecore_wl_dnd_offer_cb_offer(void *data, struct wl_data_offer *data_offer EINA_UNUSED, const char *type)
{
   Ecore_Wl_Dnd_Source *source;
   char **t;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(source = data)) return;
   if (!type) return;

   t = wl_array_add(&source->types, sizeof(*t));
   if (t) *t = strdup(type);
}
