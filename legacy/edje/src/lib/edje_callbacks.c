#include "Edje.h"
#include "edje_private.h"

void
_edje_mouse_in_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Event_Mouse_In *ev;
   Edje *ed;
   Edje_Real_Part *rp;
   
   ev = event_info;
   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if (!rp) return;
   _edje_emit(ed, "mouse,in", rp->part->name);
}

void
_edje_mouse_out_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Event_Mouse_Out *ev;
   Edje *ed;
   Edje_Real_Part *rp;
   
   ev = event_info;
   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if (!rp) return;
   _edje_emit(ed, "mouse,out", rp->part->name);
}

void
_edje_mouse_down_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   Edje *ed;
   Edje_Real_Part *rp;
   char buf[256];
   
   ev = event_info;
   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if (!rp) return;
#ifndef EDJE_FB_ONLY
   if (ecore_event_current_type_get() == ECORE_X_EVENT_MOUSE_BUTTON_DOWN)
     {
	Ecore_X_Event_Mouse_Button_Down *evx;
	
	evx = ecore_event_current_event_get();
	if (evx)
	  {
	     if (evx->triple_click)
	       snprintf(buf, sizeof(buf), "mouse,down,%i,triple", ev->button);
	     else if (evx->double_click)
	       snprintf(buf, sizeof(buf), "mouse,down,%i,double", ev->button);
	     else
	       snprintf(buf, sizeof(buf), "mouse,down,%i", ev->button);
	  }
	else
	  snprintf(buf, sizeof(buf), "mouse,down,%i", ev->button);
     }
   else
#endif     
     snprintf(buf, sizeof(buf), "mouse,down,%i", ev->button);
   if (rp->clicked_button == 0)
     {
	rp->clicked_button = ev->button;
	rp->still_in = 1;
     }
   _edje_emit(ed, buf, rp->part->name);
}

void
_edje_mouse_up_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Event_Mouse_Up *ev;
   Edje *ed;
   Edje_Real_Part *rp;
   char buf[256];
   
   ev = event_info;
   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if (!rp) return;
   snprintf(buf, sizeof(buf), "mouse,up,%i", ev->button);
   _edje_emit(ed, buf, rp->part->name);
   if ((rp->still_in) && (rp->clicked_button == ev->button))
     {
	rp->clicked_button = 0;
	rp->still_in = 0;
	snprintf(buf, sizeof(buf), "mouse,clicked,%i", ev->button);
	_edje_emit(ed, buf, rp->part->name);
     }
}

void
_edje_mouse_move_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Event_Mouse_Move *ev;
   Edje *ed;
   Edje_Real_Part *rp;
   
   ev = event_info;
   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if (!rp) return;
   if (rp->still_in)
     {
	double x, y, w, h;
	
	evas_object_geometry_get(obj, &x, &y, &w, &h);
	if ((ev->cur.canvas.x < x) || (ev->cur.canvas.y < y) || 
	    (ev->cur.canvas.x >= (x + w)) || (ev->cur.canvas.y >= (y + h)))
	  rp->still_in = 0;
     }
   else
     {
	double x, y, w, h;
	
	evas_object_geometry_get(obj, &x, &y, &w, &h);
	if ((ev->cur.canvas.x >= x) && (ev->cur.canvas.y >= y) && 
	    (ev->cur.canvas.x < (x + w)) && (ev->cur.canvas.y < (y + h)))
	  rp->still_in = 1;
     }
     
   _edje_emit(ed, "mouse,move", rp->part->name);
}

void
_edje_mouse_wheel_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Event_Mouse_Wheel *ev;
   Edje *ed;
   Edje_Real_Part *rp;
   char buf[256];
   
   ev = event_info;
   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if (!rp) return;
   snprintf(buf, sizeof(buf), "mouse,wheel,%i,%i", ev->direction, (ev->z < 0) ? (-1) : (1));
   _edje_emit(ed, buf, rp->part->name);
}

int
_edje_timer_cb(void *data)
{
   double t;
   Evas_List *l;
   Evas_List *animl = NULL;
   Edje *ed;
   
   t = ecore_time_get();
   for (l = _edje_animators; l; l = l->next)
     {
	ed = l->data;
	_edje_ref(ed);
	animl = evas_list_append(animl, l->data);
     }
   while (animl)
     {
	Evas_List *newl = NULL;
	
	ed = animl->data;
	_edje_freeze(ed);
	animl = evas_list_remove(animl, animl->data);
	if (!ed->paused)
	  {
	     ed->walking_actions = 1;
	     for (l = ed->actions; l; l = l->next)
	       newl = evas_list_append(newl, l->data);
	     while (newl)
	       {
		  Edje_Running_Program *runp;
		  
		  runp = newl->data;
		  newl = evas_list_remove(newl, newl->data);
		  if (!runp->delete_me)
		    _edje_program_run_iterate(runp, t);
	       }
	     for (l = ed->actions; l; l = l->next)
	       newl = evas_list_append(newl, l->data);
	     while (newl)
	       {
		  Edje_Running_Program *runp;
		  
		  runp = newl->data;
		  newl = evas_list_remove(newl, newl->data);
		  if (runp->delete_me)
		    {
		       _edje_anim_count--;		       
		       runp->edje->actions = 
			 evas_list_remove(runp->edje->actions, runp);
		       if (!runp->edje->actions)
			 _edje_animators = 
			 evas_list_remove(_edje_animators, runp->edje);
		       free(runp);
		    }
	       }
	     ed->walking_actions = 0;
	  }
	_edje_thaw(ed);
	_edje_unref(ed);
     }
   if (_edje_anim_count > 0) return 1;
   _edje_timer = NULL;
   return 0;
}

int
_edje_pending_timer_cb(void *data)
{
   Edje_Pending_Program *pp;
   
   pp = data;
   pp->edje->pending_actions = evas_list_remove(pp->edje->pending_actions, pp);
   _edje_program_run(pp->edje, pp->program, 1);
   free(pp);
   return 0;
}
