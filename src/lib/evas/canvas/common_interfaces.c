#include "evas_common_private.h"
#include "evas_private.h"

#include "Eo.h"

Eina_Hash* signals_hash_table = NULL;

static const Eo_Event_Description *_evas_clickable_interface_event_desc[]; 
static const Eo_Event_Description *_evas_draggable_interface_event_desc[];
static const Eo_Event_Description *_evas_scrollable_interface_event_desc[];
static const Eo_Event_Description *_evas_selectable_interface_event_desc[];
static const Eo_Event_Description *_evas_zoomable_interface_event_desc[];

#define ADD_SIGNAL(name, event) eina_hash_add(signals_hash_table, name, event)
#define ADD_INTERFACE_SIGNALS(events_desc) \
   do \
   { \
      int i = 0; \
      while (events_desc[i]) { \
           _Evas_Event_Description *desc = calloc(1, sizeof(*desc)); \
           desc->eo_desc = (Eo_Event_Description *)events_desc[i];                           \
           desc->is_desc_allocated = EINA_FALSE;                     \
           ADD_SIGNAL(events_desc[i]->name, desc);                   \
           i++; \
      } \
   } \
   while (0);

static void
_signal_interface_del(void *data)
{
   _Evas_Event_Description *desc = data;
   if (desc->is_desc_allocated) free(desc->eo_desc);
   free(desc);
}

static void
_evas_signal_interface_class_constructor(Eo_Class *klass EINA_UNUSED)
{
   signals_hash_table = eina_hash_string_superfast_new(_signal_interface_del);
   ADD_INTERFACE_SIGNALS(_evas_draggable_interface_event_desc);
   ADD_INTERFACE_SIGNALS(_evas_scrollable_interface_event_desc);
   ADD_INTERFACE_SIGNALS(_evas_zoomable_interface_event_desc);
   ADD_INTERFACE_SIGNALS(_evas_selectable_interface_event_desc);
   ADD_INTERFACE_SIGNALS(_evas_draggable_interface_event_desc);
}
#undef ADD_INTERFACE_SIGNALS
#undef ADD_SIGNAL

static void
_evas_signal_interface_class_destructor(Eo_Class *klass EINA_UNUSED)
{
   eina_hash_free(signals_hash_table);
}

#include "canvas/evas_signal_interface.eo.c"
#include "canvas/evas_draggable_interface.eo.c"
#include "canvas/evas_clickable_interface.eo.c"
#include "canvas/evas_scrollable_interface.eo.c"
#include "canvas/evas_selectable_interface.eo.c"
#include "canvas/evas_zoomable_interface.eo.c"