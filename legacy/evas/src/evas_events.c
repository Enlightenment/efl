#include "Evas.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int
_evas_point_in_object(Evas e, Evas_Object o, int x, int y)
{
   int ox, oy, ow, oh;
   
   _evas_object_get_current_translated_coords(e, o, &ox, &oy, &ow, &oh);
   if ((x >= ox) && (x < (ox + ow)) && (y >= oy) && (y < (oy + oh)))
      return 1;
   return 0;
}

Evas_Object
_evas_highest_object_at_point(Evas e, int x, int y)
{
   Evas_List l, ll;
   Evas_Layer layer;
   Evas_Object o;
   
   o = NULL;
   for (l = e->layers; l ; l = l->next)
     {
	layer = l->data;
	
	for (ll = layer->objects; ll; ll = ll->next)
	  {
	     if (_evas_point_in_object(e, ll->data, x, y)) o = ll->data;
	  }
     }
   return o;
}

Evas_List
_evas_objects_at_point(Evas e, int x, int y)
{
   Evas_List l, ll, objs;
   Evas_Layer layer;
   
   objs = NULL;
   for (l = e->layers; l ; l = l->next)
     {
	layer = l->data;
	
	for (ll = layer->objects; ll; ll = ll->next)
	  {
	     if (_evas_point_in_object(e, ll->data, x, y))
		objs = evas_list_prepend(objs, ll->data);
	  }
     }
   return objs;
}	 
   
/* events */
void
evas_event_button_down(Evas e, int x, int y, int b)
{
   if ((b > 1)  || (b < 32)) return;
   e->mouse.buttons |= (1 << (b - 1));
}

void
evas_event_button_up(Evas e, int x, int y, int b)
{
   if ((b > 1)  || (b < 32)) return;
   e->mouse.buttons &= ~(1 << (b - 1));
}

void
evas_event_move(Evas e, int x, int y)
{
   e->mouse.x = x;
   e->mouse.y = y;
}

void
evas_event_enter(Evas e)
{
   e->mouse.in = 1;
}

void
evas_event_leave(Evas e)
{
   e->mouse.in = 0;
}
