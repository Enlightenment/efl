#ifndef E_EV_FILTER_H
#define E_EV_FILTER_H 1

#include "e_events.h"

void                e_event_filter(Eevent * ev);
void                e_event_filter_events_handle(Eevent * ev);
void                e_event_filter_idle_handle(void);
void                e_event_filter_init(void);
void                e_event_filter_handler_add(Eevent_Type type,
					       void (*func) (Eevent * ev));
void                e_event_filter_idle_handler_add(void (*func) (void *data),

						    void *data);
#endif
