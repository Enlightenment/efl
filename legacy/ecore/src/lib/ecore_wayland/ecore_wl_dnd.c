#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <fcntl.h>
#include <sys/epoll.h>
#include "ecore_wl_private.h"

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
static void _ecore_wl_dnd_offer(void *data, struct wl_data_offer *wl_data_offer __UNUSED__, const char *type);
static void _ecore_wl_dnd_cb_enter_free(void *data __UNUSED__, void *event);

static void _ecore_wl_dnd_data_source_target(void *data, struct wl_data_source *source, const char *mime_type);
static void _ecore_wl_dnd_data_source_send(void *data, struct wl_data_source *source, const char *mime_type, int32_t fd);
static void _ecore_wl_dnd_data_source_cancelled(void *data, struct wl_data_source *source);
static void _ecore_wl_dnd_source_receive_data(Ecore_Wl_Dnd_Source *source, const char *type);

/* wayland listeners */
static const struct wl_data_offer_listener _ecore_wl_data_offer_listener = 
{
   _ecore_wl_dnd_offer,
};

static const struct wl_data_source_listener _ecore_wl_data_source_listener = 
{
   _ecore_wl_dnd_data_source_target,
   _ecore_wl_dnd_data_source_send,
   _ecore_wl_dnd_data_source_cancelled
};

extern Ecore_Wl_Dnd *glb_dnd;

EAPI Ecore_Wl_Dnd *
ecore_wl_dnd_get()
{
   return glb_dnd;
}

EAPI Eina_Bool
ecore_wl_dnd_start_drag(Ecore_Wl_Dnd *dnd __UNUSED__)
{
   //TODO:
   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_wl_dnd_set_selection(Ecore_Wl_Dnd *dnd, const char **types_offered)
{
   char **p;
   const char **type;

   dnd->data_source = _ecore_wl_create_data_source(dnd->ewd);

   /* free old types */
   if (dnd->types_offered.data)
     {
        wl_array_for_each(p, &dnd->types_offered)
          free(*p);
        wl_array_release(&dnd->types_offered);
        wl_array_init(&dnd->types_offered);
     }

   for (type = types_offered; *type; type++) 
     {
        p = wl_array_add(&dnd->types_offered, sizeof(*p));
        *p = strdup(*type);
        wl_data_source_offer(dnd->data_source, *p);
     }

   wl_data_source_add_listener(dnd->data_source, &_ecore_wl_data_source_listener, dnd);

   _ecore_wl_input_set_selection(dnd->input, dnd->data_source);

   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_wl_dnd_get_selection(Ecore_Wl_Dnd *dnd, const char *type)
{
   char **p;
   Ecore_Wl_Input *input;

   input = dnd->input;

   if (!input->selection_source) return EINA_FALSE;

   wl_array_for_each(p, &input->selection_source->types)
     if (strcmp(type, *p) == 0) break;

   if (!*p) return EINA_FALSE;

   _ecore_wl_dnd_source_receive_data(input->selection_source, type);

   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_wl_dnd_selection_has_owner(Ecore_Wl_Dnd *dnd)
{
   Ecore_Wl_Input *input;

   input = dnd->input;
   return (input->selection_source != NULL);
}

/* local functions */
static void
_ecore_wl_dnd_data_source_target(void *data __UNUSED__, struct wl_data_source *source __UNUSED__, const char *mime_type __UNUSED__)
{
   //TODO:
}

static void 
_ecore_wl_dnd_cb_data_source_send_free(void *data __UNUSED__, void *event)
{
   Ecore_Wl_Event_Data_Source_Send *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = event)) return;

   free(ev->type);
   free(ev);
}

static void
_ecore_wl_dnd_data_source_send(void *data, struct wl_data_source *source __UNUSED__, const char *mime_type, int32_t fd)
{
   Ecore_Wl_Event_Data_Source_Send *event;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!data) return;

   if (!(event = calloc(1, sizeof(Ecore_Wl_Event_Data_Source_Send)))) return;

   event->type = strdup(mime_type);
   event->fd = fd;

   ecore_event_add(ECORE_WL_EVENT_DATA_SOURCE_SEND, event, _ecore_wl_dnd_cb_data_source_send_free, NULL);
}

static void
_ecore_wl_dnd_data_source_cancelled(void *data __UNUSED__, struct wl_data_source *source)
{
   wl_data_source_destroy(source);
}

void 
_ecore_wl_dnd_add(Ecore_Wl_Input *input, struct wl_data_device *data_device __UNUSED__, struct wl_data_offer *offer)
{
   Ecore_Wl_Dnd_Source *source;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(source = malloc(sizeof(Ecore_Wl_Dnd_Source)))) return;
   wl_array_init(&source->types);
   source->refcount = 1;
   source->input = input;
   source->offer = offer;
   wl_data_offer_add_listener(source->offer, 
                              &_ecore_wl_data_offer_listener, source);
}

void 
_ecore_wl_dnd_enter(void *data, struct wl_data_device *data_device __UNUSED__, unsigned int timestamp __UNUSED__, struct wl_surface *surface, wl_fixed_t x, wl_fixed_t y, struct wl_data_offer *offer)
{
   Ecore_Wl_Event_Dnd_Enter *event;
   Ecore_Wl_Input *input;
   Ecore_Wl_Window *win;
   char **p;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((!(input = data)) || (!offer)) return;

   if (!(input->drag_source = wl_data_offer_get_user_data(offer)))
     return;

   win = wl_surface_get_user_data(surface);
//   input->pointer_focus = win;

   p = wl_array_add(&input->drag_source->types, sizeof(*p));
   *p = NULL;

   if (!(event = calloc(1, sizeof(Ecore_Wl_Event_Dnd_Enter)))) return;

   event->win = win->id;
   if (input->drag_source->input)
     {
        if (input->drag_source->input->keyboard_focus)
          event->source = input->drag_source->input->keyboard_focus->id;
     }

   event->position.x = wl_fixed_to_int(x);
   event->position.y = wl_fixed_to_int(y);
   event->num_types = input->drag_source->types.size;
   event->types = input->drag_source->types.data;

   ecore_event_add(ECORE_WL_EVENT_DND_ENTER, event, 
                   _ecore_wl_dnd_cb_enter_free, NULL);
}

void 
_ecore_wl_dnd_leave(void *data, struct wl_data_device *data_device __UNUSED__)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;
   /* FIXME: NB: This MAY need to raise a wl_event_dnd_leave for the 
    * source window */
   _ecore_wl_dnd_del(input->drag_source);
   input->drag_source = NULL;
}

void 
_ecore_wl_dnd_motion(void *data, struct wl_data_device *data_device __UNUSED__, unsigned int timestamp __UNUSED__, wl_fixed_t x, wl_fixed_t y)
{
   Ecore_Wl_Event_Dnd_Position *event;
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   input->sx = wl_fixed_to_int(x);
   input->sy = wl_fixed_to_int(y);

   if (!(event = calloc(1, sizeof(Ecore_Wl_Event_Dnd_Position)))) return;

   if (input->drag_source)
     {
        if (input->drag_source->input)
          {
             if (input->drag_source->input->pointer_focus)
               event->win = input->drag_source->input->pointer_focus->id;
             if (input->drag_source->input->keyboard_focus)
               event->source = input->drag_source->input->keyboard_focus->id;
          }
     }

   event->position.x = input->sx;
   event->position.y = input->sy;

   ecore_event_add(ECORE_WL_EVENT_DND_POSITION, event, NULL, NULL);
}

void 
_ecore_wl_dnd_drop(void *data, struct wl_data_device *data_device __UNUSED__)
{
   Ecore_Wl_Event_Dnd_Drop *event;
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   if (!(event = calloc(1, sizeof(Ecore_Wl_Event_Dnd_Drop)))) return;

   if (input->drag_source)
     {
        if (input->drag_source->input)
          {
             if (input->drag_source->input->pointer_focus)
               event->win = input->drag_source->input->pointer_focus->id;
             if (input->drag_source->input->keyboard_focus)
               event->source = input->drag_source->input->keyboard_focus->id;
          }
     }

   event->position.x = input->sx;
   event->position.y = input->sy;

   ecore_event_add(ECORE_WL_EVENT_DND_DROP, event, NULL, NULL);
}

void 
_ecore_wl_dnd_selection(void *data, struct wl_data_device *data_device __UNUSED__, struct wl_data_offer *offer)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;
   if (input->selection_source) _ecore_wl_dnd_del(input->selection_source);
   input->selection_source = NULL;
   if (offer)
     {
        char **p;

        input->selection_source = wl_data_offer_get_user_data(offer);
        p = wl_array_add(&input->selection_source->types, sizeof(*p));
        *p = NULL;
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
        char **p;

        wl_data_offer_destroy(source->offer);
        for (p = source->types.data; *p; p++)
          free(*p);
        wl_array_release(&source->types);
        free(source);
     }
}

/* local functions */
static void 
_ecore_wl_dnd_offer(void *data, struct wl_data_offer *wl_data_offer __UNUSED__, const char *type)
{
   Ecore_Wl_Dnd_Source *source;
   char **p;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(source = data)) return;
   p = wl_array_add(&source->types, sizeof(*p));
   *p = strdup(type);
}

static void 
_ecore_wl_dnd_cb_enter_free(void *data __UNUSED__, void *event)
{
   Ecore_Wl_Event_Dnd_Enter *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = event)) return;
   free(ev);
}

static void 
_ecore_wl_dnd_cb_selection_data_ready_free(void *data __UNUSED__, void *event)
{
   Ecore_Wl_Event_Selection_Data_Ready *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = event)) return;

   free(ev->data);
   free(ev);
}

static Eina_Bool 
_ecore_wl_dnd_read_data(void *data, Ecore_Fd_Handler *fd_handler __UNUSED__)
{
   int len;
   char buffer[4096];
   Ecore_Wl_Dnd_Source *source;
   Ecore_Wl_Event_Selection_Data_Ready *event;
   Eina_Bool ret;

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
        if (!event->data) return ECORE_CALLBACK_CANCEL;
        strncpy(event->data, buffer, len);
        event->data[len] = '\0';
        event->len = len;
        event->done = EINA_FALSE;
        ret = ECORE_CALLBACK_RENEW;
     }

   ecore_event_add(ECORE_WL_EVENT_SELECTION_DATA_READY, event, 
                   _ecore_wl_dnd_cb_selection_data_ready_free, NULL);
   return ret;
}


static Eina_Bool
_ecore_wl_dnd_idler_cb(void *data)
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
_ecore_wl_dnd_source_receive_data(Ecore_Wl_Dnd_Source *source, const char *type)
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
   task->cb = _ecore_wl_dnd_read_data;
   ep->events = EPOLLIN;
   ep->data.ptr = task;

   if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, p[0], ep) < 0) goto err;

   read_ctx->epoll_fd = epoll_fd;
   read_ctx->ep = ep;

   if (!ecore_idler_add(_ecore_wl_dnd_idler_cb, read_ctx)) goto err;

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
