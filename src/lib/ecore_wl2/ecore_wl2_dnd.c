#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_wl2_private.h"

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
_ecore_wl2_dnd_enter(Ecore_Wl2_Input *input, struct wl_data_offer *offer, struct wl_surface *surface, int x, int y, unsigned int timestamp)
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

   ev->x = x;
   ev->y = y;
   ev->offer = offer;
   ev->serial = timestamp;
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

   ecore_event_add(ECORE_WL2_EVENT_DND_LEAVE, ev, NULL, NULL);
}

void
_ecore_wl2_dnd_motion(Ecore_Wl2_Input *input, int x, int y, unsigned int timestamp)
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

   ev->x = x;
   ev->y = y;
   ev->serial = timestamp;

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
