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
	     Evas_Object ob;
	     
	     ob = ll->data;
	     if (ob->current.visible)
	       {
		  if (_evas_point_in_object(e, ob, x, y)) 
		     o = ob;
	       }
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
	     Evas_Object ob;
	     
	     ob = ll->data;
	     if (ob->current.visible)
	       {
		  if (_evas_point_in_object(e, ll->data, x, y))
		     objs = evas_list_prepend(objs, ll->data);
	       }
	  }
     }
   return objs;
}	 
   
/* events */
void
evas_event_button_down(Evas e, int x, int y, int b)
{
   Evas_Object o;
   
   if ((b < 1) || (b > 32)) return;
   if (!e->mouse.buttons) 
      {
	 o = _evas_highest_object_at_point(e, e->mouse.x, e->mouse.y);
	 e->mouse.button_object = o;
      }
   e->mouse.buttons |= (1 << (b - 1));
   e->mouse.x = x;
   e->mouse.y = y;
   if (e->mouse.button_object)
      _evas_callback_call(e, e->mouse.button_object, CALLBACK_MOUSE_DOWN, 
			  b, x, y);
}

void
evas_event_button_up(Evas e, int x, int y, int b)
{
   Evas_Object o;
   
   if ((b < 1) || (b > 32)) return;
   e->mouse.buttons &= ~(1 << (b - 1));
   e->mouse.x = x;
   e->mouse.y = y;
   if (e->mouse.button_object)
      _evas_callback_call(e, e->mouse.button_object, CALLBACK_MOUSE_UP, 
			  b, x, y);
   if (!e->mouse.buttons) e->mouse.button_object = NULL;
}

void
evas_event_move(Evas e, int x, int y)
{
   Evas_Object o;

   if (!e->mouse.button_object)
     {
	o = _evas_highest_object_at_point(e, e->mouse.x, e->mouse.y);
	if (o != e->mouse.object)
	  {
	     if (e->mouse.object)
	       {
		  _evas_callback_call(e, e->mouse.object, CALLBACK_MOUSE_MOVE, 
				      e->mouse.buttons, e->mouse.x, e->mouse.y);
		  _evas_callback_call(e, e->mouse.object, CALLBACK_MOUSE_OUT, 
				      e->mouse.buttons, e->mouse.x, e->mouse.y);
	       }
	     e->mouse.x = x;
	     e->mouse.y = y;
	     e->mouse.object = o;
	     if (e->mouse.object)
	       {
		  _evas_callback_call(e, e->mouse.object, CALLBACK_MOUSE_IN, 
				      e->mouse.buttons, e->mouse.x, e->mouse.y);
		  _evas_callback_call(e, e->mouse.object, CALLBACK_MOUSE_MOVE, 
				      e->mouse.buttons, e->mouse.x, e->mouse.y);
	       }
	     return;
	  }   
	e->mouse.x = x;
	e->mouse.y = y;
	if (e->mouse.object)
	   _evas_callback_call(e, e->mouse.object, CALLBACK_MOUSE_MOVE, 
			       e->mouse.buttons, e->mouse.x, e->mouse.y);
	return;
     }
   e->mouse.x = x;
   e->mouse.y = y;
   if (e->mouse.button_object)
      _evas_callback_call(e, e->mouse.button_object, CALLBACK_MOUSE_MOVE, 
			  e->mouse.buttons, e->mouse.x, e->mouse.y);
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

Evas_Object
evas_get_object_under_mouse(Evas e)
{
   return _evas_highest_object_at_point(e, e->mouse.x, e->mouse.y);
}

Evas_Object
evas_get_object_at_pos(Evas e, double x, double y)
{
   return _evas_highest_object_at_point(e, x, y);
}
