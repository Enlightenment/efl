#include "Edje.h"
#include "edje_private.h"

static double       _edje_frametime = 1.0 / 60.0;

int          _edje_anim_count = 0;
Ecore_Timer *_edje_timer = NULL;
Evas_List   *_edje_animators = NULL;

/* API Routines */
void
edje_frametime_set(double t)
{
   if (t == _edje_frametime) return;
   _edje_frametime = t;
   if (_edje_timer)
     {
	ecore_timer_del(_edje_timer);
	_edje_timer = ecore_timer_add(_edje_frametime, _edje_timer_cb, NULL);
     }
}

double
edje_frametime_get(void)
{
   return _edje_frametime;
}

void
edje_signal_callback_add(Evas_Object *obj, const char *emission, const char *source, void (*func) (void *data, Evas_Object *o, const char *emission, const char *source), void *data)
{
   Edje *ed;
   Edje_Signal_Callback *escb;
   
   if ((!emission) || (!source) || (!func)) return;
   ed = _edje_fetch(obj);
   if (!ed) return;
   escb = calloc(1, sizeof(Edje_Signal_Callback));
   escb->signal = strdup(emission);
   escb->source = strdup(source);
   escb->func = func;
   escb->data = data;
   ed->callbacks = evas_list_append(ed->callbacks, escb);
}

void *
edje_signal_callback_del(Evas_Object *obj, const char *emission, const char *source, void (*func) (void *data, Evas_Object *o, const char *emission, const char *source))
{
   Edje *ed;
   Evas_List *l;
   
   if ((!emission) || (!source) || (!func)) return NULL;
   ed = _edje_fetch(obj);
   if (!ed) return;
   for (l = ed->callbacks; l; l = l->next)
     {
	Edje_Signal_Callback *escb;
	
	escb = l->data;
	if ((escb->func == func) && 
	    (!strcmp(escb->signal, emission)) &&
	    (!strcmp(escb->source, source)))
	  {
	     void *data;
	     
	     data = escb->data;
	     free(escb->signal);
	     free(escb->source);
	     free(escb);
	     return data;
	  }
     }
   return NULL;
}

void
edje_signal_emit(Evas_Object *obj, const char *emission, const char *source)
{
   Edje *ed;

   if ((!emission) || (!source)) return;
   ed = _edje_fetch(obj);
   if (!ed) return;
   _edje_emit(ed, (char *)emission, (char *)source);
}

/* Private Routines */

int
_edje_program_run_iterate(Edje_Running_Program *runp, double tim)
{
   double t, total;
   Evas_List *l;

   _edje_freeze(runp->edje);
   t = tim - runp->start_time;
   total = runp->program->tween.time;
   t /= total;
   if (t > 1.0) t = 1.0;
   for (l = runp->program->targets; l; l = l->next)
     {
	Edje_Real_Part *rp;
	Edje_Program_Target *pt;
	
	pt = l->data;
	rp = evas_list_nth(runp->edje->parts, pt->id);
	if (rp) _edje_part_pos_set(runp->edje, rp, 
				   runp->program->tween.mode, t);
     }
   if (t >= 1.0)
     {
	for (l = runp->program->targets; l; l = l->next)
	  {
	     Edje_Real_Part *rp;
	     Edje_Program_Target *pt;
	     
	     pt = l->data;
	     rp = evas_list_nth(runp->edje->parts, pt->id);
	     if (rp)
	       {
		  _edje_part_description_apply(runp->edje, rp, 
					       runp->program->state, 
					       runp->program->value,
					       NULL,
					       0.0);
		  _edje_part_pos_set(runp->edje, rp, 
				     runp->program->tween.mode, 0.0);
		  rp->program = NULL;
	       }
	  }
	_edje_recalc(runp->edje);
	_edje_anim_count--;
	runp->edje->actions = evas_list_remove(runp->edje->actions, runp);
	if (!runp->edje->actions)
	  _edje_animators = evas_list_remove(_edje_animators, runp->edje);
	_edje_emit(runp->edje, "anim,stop", runp->program->name);
	if (runp->program->after >= 0)
	  {
	     Edje_Program *pr;
	     
	     pr = evas_list_nth(runp->edje->collection->programs, 
				runp->program->after);
	     if (pr) _edje_program_run(runp->edje, pr);
	  }
	_edje_thaw(runp->edje);
	free(runp);
	return  0;
     }
   _edje_recalc(runp->edje);
   _edje_thaw(runp->edje);
   return 1;
}

void
_edje_program_end(Edje *ed, Edje_Running_Program *runp)
{
   Evas_List *l;

   _edje_freeze(runp->edje);
   for (l = runp->program->targets; l; l = l->next)
     {
	Edje_Real_Part *rp;
	Edje_Program_Target *pt;
	
	pt = l->data;
	rp = evas_list_nth(runp->edje->parts, pt->id);
	if (rp)
	  {
	     _edje_part_description_apply(runp->edje, rp, 
					  runp->program->state, 
					  runp->program->value,
					  NULL,
					  0.0);
	     _edje_part_pos_set(runp->edje, rp, 
				runp->program->tween.mode, 0.0);
	     rp->program = NULL;
	  }
     }
   _edje_recalc(runp->edje);
   _edje_anim_count--;
   runp->edje->actions = evas_list_remove(runp->edje->actions, runp);
   if (!runp->edje->actions)
     _edje_animators = evas_list_remove(_edje_animators, runp->edje);
   _edje_emit(runp->edje, "anim,stop", runp->program->name);
   _edje_thaw(runp->edje);
   free(runp);
}
   
void
_edje_program_run(Edje *ed, Edje_Program *pr)
{
   Evas_List *l;

   _edje_freeze(ed);
   _edje_emit(ed, "anim,start", pr->name);
   if (pr->action == EDJE_ACTION_TYPE_STATE_SET)
     {
	if (pr->tween.time > 0.0)
	  {
	     Edje_Running_Program *runp;
	     
	     runp = calloc(1, sizeof(Edje_Running_Program));
	     for (l = pr->targets; l; l = l->next)
	       {
		  Edje_Real_Part *rp;
		  Edje_Program_Target *pt;
		  
		  pt = l->data;
		  rp = evas_list_nth(ed->parts, pt->id);
		  if (rp)
		    {
		       if (rp->program)
			 _edje_program_end(ed, rp->program);
		       _edje_part_description_apply(ed, rp, 
						    rp->param1.description->state.name,
						    rp->param1.description->state.value, 
						    pr->state, 
						    pr->value);
		       _edje_part_pos_set(ed, rp, pr->tween.mode, 0.0);
		       rp->program = runp;
		    }
	       }
	     
	     if (!ed->actions)
	       _edje_animators = evas_list_append(_edje_animators, ed);
	     ed->actions = evas_list_append(ed->actions, runp);
	     runp->start_time = ecore_time_get();
	     runp->edje = ed;
	     runp->program = pr;
	     if (!_edje_timer)
	       _edje_timer = ecore_timer_add(_edje_frametime, _edje_timer_cb, NULL);
	     _edje_anim_count++;
	  }
	else
	  {
	     for (l = pr->targets; l; l = l->next)
	       {
		  Edje_Real_Part *rp;
		  Edje_Program_Target *pt;
		  
		  pt = l->data;
		  rp = evas_list_nth(ed->parts, pt->id);
		  if (rp)
		    {
		       if (rp->program)
			 _edje_program_end(ed, rp->program);
		       _edje_part_description_apply(ed, rp, 
						    pr->state, 
						    pr->value,
						    NULL,
						    0.0);
		       _edje_part_pos_set(ed, rp, pr->tween.mode, 0.0);
		    }
	       }
	     _edje_recalc(ed);
	  }
     }
   else if (pr->action == EDJE_ACTION_TYPE_ACTION_STOP)
     {
	for (l = pr->targets; l; l = l->next)
	  {
	     Edje_Program_Target *pt;
	     Evas_List *ll;
	     
	     pt = l->data;
	     for (ll = ed->actions; ll; ll = ll->next)
	       {
		  Edje_Running_Program *runp;
		  
		  runp = ll->data;
		  if (pt->id == runp->program->id)
		    {
		       _edje_program_end(ed, runp);
		       break;
		    }
	       }
	  }
     }
   else if (pr->action == EDJE_ACTION_TYPE_SIGNAL_EMIT)
     {
	_edje_emit(ed, pr->state, pr->state2);
     }
   _edje_thaw(ed);
}

void
_edje_emit(Edje *ed, char *sig, char *src)
{
   Evas_List *l;
   static Evas_List *emissions = NULL;
   Edje_Emission *ee;

   _edje_freeze(ed);
   printf("EMIT \"%s\" \"%s\"\n", sig, src);
   ee = calloc(1, sizeof(Edje_Emission));
   if (!ee) return;
   ee->signal = strdup(sig);
   ee->source = strdup(src);
   if (emissions)
     {
	emissions = evas_list_append(emissions, ee);
	_edje_thaw(ed);
	return;
     }
   else
     emissions = evas_list_append(emissions, ee);
   while (emissions)
     {
	ee = emissions->data;
	emissions = evas_list_remove(emissions, ee);
	if (ed->collection)
	  {
	     for (l = ed->collection->programs; l; l = l->next)
	       {
		  Edje_Program *pr;
		  
		  pr = l->data;
		  if ((_edje_glob_match(ee->signal, pr->signal)) &&
		      (_edje_glob_match(ee->source, pr->source)))
		    _edje_program_run(ed, pr);
	       }
	     for (l = ed->callbacks; l; l = l->next)
	       {
		  Edje_Signal_Callback *escb;
		  
		  escb = l->data;
		  if ((_edje_glob_match(ee->signal, escb->signal)) &&
		      (_edje_glob_match(ee->source, escb->source)))
	       escb->func(escb->data, ed->obj, ee->signal, ee->source);
	       }
	  }
	free(ee->signal);
	free(ee->source);
	free(ee);
     }
   _edje_thaw(ed);
}
