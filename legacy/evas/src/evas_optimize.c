#include "Evas_private.h"
#include "Evas.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "evas_gl_routines.h"
#include "evas_imlib_routines.h"
#include "evas_image_routines.h"
#include "evas_x11_routines.h"
#include "evas_render_routines.h"

void
_evas_opt_invalidate(Evas e)
{
   Evas_List l;
   
   for (l = e->layers; l; l = l->next)
     {
	Evas_Layer layer;
	Evas_List ll;
	
	layer = l->data;
	layer->opt.visible.valid = 0;
	if (layer->opt.visible.objects)
	  {
	     for (ll = layer->opt.visible.objects; ll; ll = ll->next)
	       {
		  Evas_Object o;
		  
		  o = ll->data;
		  o->opt.layer = NULL;
		  o->opt.is = 0;
	       }
	     evas_list_free(layer->opt.visible.objects);
	     layer->opt.visible.objects = NULL;
	  }
     }
}

void
_evas_opt_validate_all(Evas e)
{
   Evas_List l;
   
   for (l = e->layers; l; l = l->next)
     {
	Evas_Layer layer;
	Evas_List ll;
	
	layer = l->data;
	if (!layer->opt.visible.valid)
	  {
	     layer->opt.visible.valid = 1;
	     for (ll = layer->objects; ll; ll = ll->next)
	       {
		  Evas_Object o;
		  
		  o = ll->data;
		  o->opt.layer = layer;
		  _evas_opt_validate_object(e, o);
	       }
	  }
     }
}

Evas_List;
_evas_opt_cleanup_validate_all(Evas e)
{
   Evas_List l;
   Evas_List removes = NULL;
   
   for (l = e->layers; l; l = l->next)
     {
	Evas_Layer layer;
	Evas_List ll;
	
	layer = l->data;
	if (layer->opt.visible.valid)
	  {
	     for (ll = layer->opt.visible.objects; ll; ll = ll->next)
	       {
		  Evas_Object o;
		  
		  o = ll->data;
		  o->opt.layer = layer;
		  if ((o->previous.visible) && (!o->current.visible))
		    {
		       removes = evas_list_append(removes, o);
		    }
	       }
	  }
     }
   return removes;
}

void
_evas_opt_cleanup_validate_all_removes(Evas e, Evas_List removes)
{
   Evas_List l;
   
   if (removes)
     {
	for (l = removes; l; l = l->next)
	  {
	     Evas_Object o;
	     
	     o = l->data;
	     o->opt.is = 0;
	     o->opt.layer->opt.visible.objects = evas_list_remove(o->opt.layer->opt.visible.objects, o);
	  }
	evas_list_free(removes);
	removes = NULL;
     }
}

void
_evas_opt_set_object_layer(Evas e, Evas_Object o, Evas_Layer layer)
{
   o->opt.layer = layer;
}

void
_evas_opt_validate_object(Evas e, Evas_Object o)
{
   double cx, cy, cw, ch, px, py, pw, ph;
   
   if (o->opt.is) return;
/*   if ((!o->current.visible) && (!o->previous.visible)) return;*/
   if (!o->opt.layer) return;
   _evas_get_current_clipped_geometry(e, o, &cx, &cy, &cw, &ch);
   _evas_get_previous_clipped_geometry(e, o, &px, &py, &pw, &ph);
   if ((!(INTERSECTS(cx, cy, cw, ch,
		   e->current.viewport.x, e->current.viewport.y,
		   e->current.viewport.w, e->current.viewport.h))) &&
       (!(INTERSECTS(px, py, pw, ph,
		     e->previous.viewport.x, e->previous.viewport.y,
		     e->previous.viewport.w, e->previous.viewport.h))))
     return;
   o->opt.is = 1;
   o->opt.layer->opt.visible.objects = evas_list_append(o->opt.layer->opt.visible.objects, o);
}

void
_evas_opt_invalidate_object(Evas e, Evas_Object o)
{
   if (!o->opt.is) return;
   o->opt.is = 0;
   if (o->opt.layer)
     {
	o->opt.layer->opt.visible.objects = evas_list_remove(o->opt.layer->opt.visible.objects, o);
     }
}
