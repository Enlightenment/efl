#include "Evas_private.h"
#include "Evas.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void
_evas_callback_call(Evas e, Evas_Object o, Evas_Callback_Type callback,
		    int b, int x, int y)
{
   Evas_List l;
   static in_cb = 0;
   
   if (in_cb) return;
   in_cb = 1;
   if (o->callbacks)
     {
	for (l = o->callbacks; l; l = l->next)
	  {
	     Evas_Callback cb;
	     
	     cb = l->data;
	     if (cb->type == callback)
		cb->callback(cb->data, e, o, b, x, y);
	  }
     }   
   in_cb = 0;
}

/* callbacks */
void
evas_callback_add(Evas e, Evas_Object o, Evas_Callback_Type callback, void (*func) (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y), void *data)

{
   Evas_Callback cb;
   
   if (!e) return;
   if (!o) return;
   cb = malloc(sizeof(struct _Evas_Callback));
   cb->type = callback;
   cb->data = data;
   cb->callback = func;
   o->callbacks = evas_list_append(o->callbacks, cb);
}

void
evas_callback_del(Evas e, Evas_Object o, Evas_Callback_Type callback)
{
   Evas_List l;
   int have_cb;
   
   if (!e) return;
   if (!o) return;
   have_cb = 1;
   while (have_cb)
     {
	for (l = o->callbacks; l; l = l->next)
	  {
	     Evas_Callback cb;
	     
	     cb = l->data;
	     if (cb->type == callback)
	       {
		  o->callbacks = evas_list_remove(o->callbacks, cb);
		  free(cb);
		  have_cb = 1;
		  break;
	       }
	  }
     }   
}
