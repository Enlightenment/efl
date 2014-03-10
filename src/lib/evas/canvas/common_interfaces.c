#include "evas_common_private.h"
#include "evas_private.h"

#include "Eo.h"

Eina_Hash* signals_hash_table = NULL;

EAPI const Eo_Event_Description _CLICKED_EVENT =
   EO_EVENT_DESCRIPTION("clicked", "");

EAPI const Eo_Event_Description _CLICKED_DOUBLE_EVENT =
   EO_EVENT_DESCRIPTION("clicked,double", "");

EAPI const Eo_Event_Description _CLICKED_TRIPLE_EVENT =
   EO_EVENT_DESCRIPTION("clicked,triple", "");

EAPI const Eo_Event_Description _PRESSED_EVENT =
   EO_EVENT_DESCRIPTION("pressed", "");

EAPI const Eo_Event_Description _UNPRESSED_EVENT =
   EO_EVENT_DESCRIPTION("unpressed", "");

EAPI const Eo_Event_Description _LONGPRESSED_EVENT =
   EO_EVENT_DESCRIPTION("longpressed", "");

EAPI const Eo_Event_Description _REPEATED_EVENT =
   EO_EVENT_DESCRIPTION("repeated", "");

EAPI const Eo_Event_Description _SCROLL_EVENT =
   EO_EVENT_DESCRIPTION("scroll", "");

EAPI const Eo_Event_Description _SCROLL_ANIM_START_EVENT =
   EO_EVENT_DESCRIPTION("scroll,anim,start", "");

EAPI const Eo_Event_Description _SCROLL_ANIM_STOP_EVENT =
   EO_EVENT_DESCRIPTION("scroll,anim,stop", "");

EAPI const Eo_Event_Description _SCROLL_DRAG_START_EVENT =
   EO_EVENT_DESCRIPTION("scroll,drag,start", "");

EAPI const Eo_Event_Description _SCROLL_DRAG_STOP_EVENT =
   EO_EVENT_DESCRIPTION("scroll,drag,stop", "");

EAPI const Eo_Event_Description _ZOOM_START_EVENT =
   EO_EVENT_DESCRIPTION("zoom,start", "");

EAPI const Eo_Event_Description _ZOOM_STOP_EVENT =
   EO_EVENT_DESCRIPTION("zoom,stop", "");

EAPI const Eo_Event_Description _ZOOM_CHANGE_EVENT =
   EO_EVENT_DESCRIPTION("zoom,changed", "");

EAPI const Eo_Event_Description _SELECTED_EVENT =
   EO_EVENT_DESCRIPTION("selected", "");

EAPI const Eo_Event_Description _UNSELECTED_EVENT =
   EO_EVENT_DESCRIPTION("unselected", "");

EAPI const Eo_Event_Description _SELECTION_PASTE_EVENT =
   EO_EVENT_DESCRIPTION("selection,paste", "");

EAPI const Eo_Event_Description _SELECTION_COPY_EVENT =
   EO_EVENT_DESCRIPTION("selection,copy", "");

EAPI const Eo_Event_Description _SELECTION_CUT_EVENT =
   EO_EVENT_DESCRIPTION("selection,cut", "");

EAPI const Eo_Event_Description _SELECTION_START_EVENT =
   EO_EVENT_DESCRIPTION("selection,start", "");

EAPI const Eo_Event_Description _SELECTION_CHANGED_EVENT =
   EO_EVENT_DESCRIPTION("selection,changed", "");

EAPI const Eo_Event_Description _SELECTION_CLEARED_EVENT =
   EO_EVENT_DESCRIPTION("selection,cleared", "");

EAPI const Eo_Event_Description _DRAG_EVENT =
   EO_EVENT_DESCRIPTION("drag", "");

EAPI const Eo_Event_Description _DRAG_START_EVENT =
   EO_EVENT_DESCRIPTION("drag,start", "");

EAPI const Eo_Event_Description _DRAG_STOP_EVENT =
   EO_EVENT_DESCRIPTION("drag,stop", "");

EAPI const Eo_Event_Description _DRAG_END_EVENT =
   EO_EVENT_DESCRIPTION("drag,end", "");

EAPI const Eo_Event_Description _DRAG_START_UP_EVENT =
   EO_EVENT_DESCRIPTION("drag,start,up", "");

EAPI const Eo_Event_Description _DRAG_START_DOWN_EVENT =
   EO_EVENT_DESCRIPTION("drag,start,down", "");

EAPI const Eo_Event_Description _DRAG_START_RIGHT_EVENT =
   EO_EVENT_DESCRIPTION("drag,start,right", "");

EAPI const Eo_Event_Description _DRAG_START_LEFT_EVENT =
   EO_EVENT_DESCRIPTION("drag,start,left", "");

static const Eo_Event_Description *_clickable_events_desc[] = {
     EVAS_SMART_CLICKED_EVENT,
     EVAS_SMART_CLICKED_DOUBLE_EVENT,
     EVAS_SMART_CLICKED_TRIPLE_EVENT,
     EVAS_SMART_PRESSED_EVENT,
     EVAS_SMART_UNPRESSED_EVENT,
     EVAS_SMART_LONGPRESSED_EVENT,
     EVAS_SMART_REPEATED_EVENT,
     NULL
};

static const Eo_Event_Description *_scrollable_events_desc[] = {
     EVAS_SMART_SCROLL_EVENT,
     EVAS_SMART_SCROLL_ANIM_START_EVENT,
     EVAS_SMART_SCROLL_ANIM_STOP_EVENT,
     EVAS_SMART_SCROLL_DRAG_START_EVENT,
     EVAS_SMART_SCROLL_DRAG_STOP_EVENT,
     NULL
};

static const Eo_Event_Description *_zoomable_events_desc[] = {
     EVAS_SMART_ZOOM_START_EVENT,
     EVAS_SMART_ZOOM_STOP_EVENT,
     EVAS_SMART_ZOOM_CHANGE_EVENT,
     NULL
};

static const Eo_Event_Description *_selectable_events_desc[] = {
     EVAS_SMART_SELECTED_EVENT,
     EVAS_SMART_UNSELECTED_EVENT,
     EVAS_SMART_SELECTION_PASTE_EVENT,
     EVAS_SMART_SELECTION_COPY_EVENT,
     EVAS_SMART_SELECTION_CUT_EVENT,
     EVAS_SMART_SELECTION_START_EVENT,
     EVAS_SMART_SELECTION_CHANGED_EVENT,
     EVAS_SMART_SELECTION_CLEARED_EVENT,
     NULL
};

static const Eo_Event_Description *_draggable_events_desc[] = {
     EVAS_SMART_DRAG_EVENT,
     EVAS_SMART_DRAG_START_EVENT,
     EVAS_SMART_DRAG_STOP_EVENT,
     EVAS_SMART_DRAG_END_EVENT,
     EVAS_SMART_DRAG_START_UP_EVENT,
     EVAS_SMART_DRAG_START_DOWN_EVENT,
     EVAS_SMART_DRAG_START_RIGHT_EVENT,
     EVAS_SMART_DRAG_START_LEFT_EVENT,
     NULL
};

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
   ADD_INTERFACE_SIGNALS(_clickable_events_desc);
   ADD_INTERFACE_SIGNALS(_scrollable_events_desc);
   ADD_INTERFACE_SIGNALS(_zoomable_events_desc);
   ADD_INTERFACE_SIGNALS(_selectable_events_desc);
   ADD_INTERFACE_SIGNALS(_draggable_events_desc);
}
#undef ADD_INTERFACE_SIGNALS
#undef ADD_SIGNAL

static void
_evas_signal_interface_class_destructor(Eo_Class *klass EINA_UNUSED)
{
   eina_hash_free(signals_hash_table);
}

static const Eo_Class_Description clickable_interface_desc = {
     EO_VERSION,
     "Evas_Clickable_Interface",
     EO_CLASS_TYPE_INTERFACE,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     _clickable_events_desc,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(evas_smart_clickable_interface_get, &clickable_interface_desc, EVAS_SIGNAL_INTERFACE_CLASS, NULL);

static const Eo_Class_Description scrollable_interface_desc = {
     EO_VERSION,
     "Evas_Scrollable_Interface",
     EO_CLASS_TYPE_INTERFACE,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     _scrollable_events_desc,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(evas_smart_scrollable_interface_get, &scrollable_interface_desc, EVAS_SIGNAL_INTERFACE_CLASS, NULL);

static const Eo_Class_Description zoomable_interface_desc = {
     EO_VERSION,
     "Evas_Zoomable_Interface",
     EO_CLASS_TYPE_INTERFACE,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     _zoomable_events_desc,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(evas_smart_zoomable_interface_get, &zoomable_interface_desc, EVAS_SIGNAL_INTERFACE_CLASS, NULL);

static const Eo_Class_Description selectable_interface_desc = {
     EO_VERSION,
     "Evas_Selectable_Interface",
     EO_CLASS_TYPE_INTERFACE,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     _selectable_events_desc,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(evas_smart_selectable_interface_get, &selectable_interface_desc, EVAS_SIGNAL_INTERFACE_CLASS, NULL);

static const Eo_Class_Description draggable_interface_desc = {
     EO_VERSION,
     "Evas_Draggable_Interface",
     EO_CLASS_TYPE_INTERFACE,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     _draggable_events_desc,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(evas_smart_draggable_interface_get, &draggable_interface_desc, EVAS_SIGNAL_INTERFACE_CLASS, NULL);

#include "canvas/evas_signal_interface.eo.c"
