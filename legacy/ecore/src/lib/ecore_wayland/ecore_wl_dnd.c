#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"
#include "ecore_wl_private.h"
#include "Ecore_Wayland.h"

/* local function prototypes */
static void _ecore_wl_dnd_offer(void *data, struct wl_data_offer *wl_data_offer __UNUSED__, const char *type);
static void _ecore_wl_dnd_cb_enter_free(void *data __UNUSED__, void *event);

/* wayland listeners */
static const struct wl_data_offer_listener _ecore_wl_data_offer_listener = 
{
   _ecore_wl_dnd_offer,
};

void 
_ecore_wl_dnd_add(Ecore_Wl_Input *input, struct wl_data_device *data_device, unsigned int id)
{
   Ecore_Wl_Dnd_Source *source;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(source = malloc(sizeof(Ecore_Wl_Dnd_Source)))) return;
   wl_array_init(&source->types);
   source->refcount = 1;
   source->input = input;
   /* FIXME: Change this when wayland has typesafe wrapper for it */
   source->offer = (struct wl_data_offer *)
     wl_proxy_create_for_id((struct wl_proxy *)data_device, 
                            id, &wl_data_offer_interface);
   wl_data_offer_add_listener(source->offer, 
                              &_ecore_wl_data_offer_listener, source);
}

void 
_ecore_wl_dnd_enter(void *data, struct wl_data_device *data_device __UNUSED__, unsigned int timestamp __UNUSED__, struct wl_surface *surface, int x, int y, struct wl_data_offer *offer)
{
   Ecore_Wl_Event_Dnd_Enter *event;
   Ecore_Wl_Input *input;
   Ecore_Wl_Window *win;
   char **p;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   input->drag_source = wl_data_offer_get_user_data(offer);

   win = wl_surface_get_user_data(surface);
//   input->pointer_focus = win;

   p = wl_array_add(&input->drag_source->types, sizeof(*p));
   *p = NULL;

   if (!(event = calloc(1, sizeof(Ecore_Wl_Event_Dnd_Enter)))) return;

   event->win = win->id;
   event->source = input->drag_source->input->keyboard_focus->id;
   event->position.x = x;
   event->position.y = y;
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
   _ecore_wl_dnd_del(input->drag_source);
   input->drag_source = NULL;
}

void 
_ecore_wl_dnd_motion(void *data, struct wl_data_device *data_device __UNUSED__, unsigned int timestamp __UNUSED__, int x, int y)
{
   Ecore_Wl_Event_Dnd_Position *event;
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   input->sx = x;
   input->sy = y;

   if (!(event = calloc(1, sizeof(Ecore_Wl_Event_Dnd_Position)))) return;

   event->win = input->drag_source->input->pointer_focus->id;
   event->source = input->drag_source->input->keyboard_focus->id;
   event->position.x = x;
   event->position.y = y;

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

   event->win = input->drag_source->input->pointer_focus->id;
   event->source = input->drag_source->input->keyboard_focus->id;
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
