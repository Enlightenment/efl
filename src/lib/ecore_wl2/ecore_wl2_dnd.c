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

static void
_source_cb_target(void *data, struct wl_data_source *source EINA_UNUSED, const char *mime_type EINA_UNUSED)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;
}

static void
_source_cb_send(void *data, struct wl_data_source *source EINA_UNUSED, const char *mime_type, int32_t fd)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;
}

static void
_source_cb_cancelled(void *data, struct wl_data_source *source)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;
}

static const struct wl_data_source_listener _source_listener =
{
   _source_cb_target,
   _source_cb_send,
   _source_cb_cancelled
};

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
        ERR("Could not create data source: %m");
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

   /* TODO: selection_data_receive */

   return EINA_TRUE;
}

EAPI void
ecore_wl2_dnd_drag_end(Ecore_Wl2_Input *input)
{
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

   /* TODO: Raise dnd end event */
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
        ERR("Could not create data source: %m");
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

   /* TODO: selection_data_receive */

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
