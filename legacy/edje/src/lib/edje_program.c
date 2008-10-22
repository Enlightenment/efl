/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "edje_private.h"

static void _edje_emit_cb(Edje *ed, const char *sig, const char *src);

int             _edje_anim_count = 0;
Ecore_Animator *_edje_timer = NULL;
Eina_List      *_edje_animators = NULL;

/************************** API Routines **************************/

/* FIXDOC: Expand */
/** Set the frametime
 * @param t The frametime
 *
 * Sets the global frametime in seconds, by default this is 1/30.
 */
EAPI void
edje_frametime_set(double t)
{
   ecore_animator_frametime_set(t);
}

/* FIXDOC: Expand */
/** Get the frametime
 * @return The frametime
 *
 * Returns the frametime in seconds, by default this is 1/30.
 */
EAPI double
edje_frametime_get(void)
{
   return ecore_animator_frametime_get();
}

/* FIXDOC: Expand */
/** Add a callback for a signal emitted by @a obj.
 * @param obj A valid Evas_Object handle
 * @param emission The signal name
 * @param source The signal source
 * @param func The callback function to be executed when the signal is emitted
 * @param data A pointer to data to pass in to the callback function
 *
 * Connects a callback function to a signal emitted by @a obj.
 * In EDC, a program can emit a signal as follows:
 *
 * @code
 * program {
 *   name: "emit_example";
 *   action: SIGNAL_EMIT "a_signal" "a_source";
 * }
 * @endcode
 *
 * Assuming a function with the following declaration is definded:
 *
 * @code
 * void cb_signal(void *data, Evas_Object *o, const char *emission, const char *source);
 * @endcode
 *
 * a callback is attached using:
 *
 * @code
 * edje_object_callback_add(obj, "a_signal", "a_source", cb_signal, data);
 * @endcode
 *
 * Here, @a data is an arbitrary pointer to be used as desired.
 * Note that @a emission and @a source correspond respectively to first and
 * second parameters to the SIGNAL_EMIT action.
 *
 * Internal edje signals can also be attached to, and globs can be in either
 * the emission or source name. e.g.
 *
 * @code
 * edje_object_callback_add(obj, "mouse,down,*", "button.*", NULL);
 * @endcode
 *
 * Here, any mouse down events on an edje part whose name begins with
 * "button." will trigger the callback. The actual signal and source name
 * will be passed in to the @a emission and @a source parameters of the
 * callback function. (e.g. "mouse,down,2" and "button.close").
 */
EAPI void
edje_object_signal_callback_add(Evas_Object *obj, const char *emission, const char *source, void (*func) (void *data, Evas_Object *o, const char *emission, const char *source), void *data)
{
   Edje *ed;
   Edje_Signal_Callback *escb;

   if ((!emission) || (!source) || (!func)) return;
   ed = _edje_fetch(obj);
   if (!ed) return;
   if (ed->delete_me) return;
   escb = calloc(1, sizeof(Edje_Signal_Callback));
   if ((emission) && (emission[0]))
     escb->signal = eina_stringshare_add(emission);
   if ((source) && (source[0]))
     escb->source = eina_stringshare_add(source);
   escb->func = func;
   escb->data = data;
   ed->callbacks = eina_list_append(ed->callbacks, escb);
   if (ed->walking_callbacks)
     {
	escb->just_added = 1;
	ed->just_added_callbacks = 1;
     }
   else
     _edje_callbacks_patterns_clean(ed);
}

/** Remove a callback from an object
 * @param obj A valid Evas_Object handle
 * @param emission the emission string
 * @param source the source string
 * @param func the callback function
 * @return the data pointer
 *
 * Removes a callback from an object. The parameters @a emission, @a source
 * and @a func must match exactly those passed to a previous call to
 * edje_object_signal_callback_add(). The data pointer that was passed to
 * this call will be returned.
 */
EAPI void *
edje_object_signal_callback_del(Evas_Object *obj, const char *emission, const char *source, void (*func) (void *data, Evas_Object *o, const char *emission, const char *source))
{
   Edje *ed;
   Eina_List *l;
   Edje_Signal_Callback *escb;

   if ((!emission) || (!source) || (!func)) return NULL;
   ed = _edje_fetch(obj);
   if (!ed) return NULL;
   if (ed->delete_me) return NULL;
   EINA_LIST_FOREACH(ed->callbacks, l, escb)
     {
	if ((escb->func == func) &&
	    ((!escb->signal && !emission[0]) ||
             (escb->signal && !strcmp(escb->signal, emission))) &&
	    ((!escb->source && !source[0]) ||
             (escb->source && !strcmp(escb->source, source))))
	  {
	     void *data;

	     data = escb->data;
	     if (ed->walking_callbacks)
	       {
		  escb->delete_me = 1;
		  ed->delete_callbacks = 1;
	       }
	     else
	       {
		  _edje_callbacks_patterns_clean(ed);

		  ed->callbacks = eina_list_remove_list(ed->callbacks, l);
		  if (escb->signal) eina_stringshare_del(escb->signal);
		  if (escb->source) eina_stringshare_del(escb->source);
		  free(escb);
	       }
	     return data;
	  }
     }
   return NULL;
}

/* FIXDOC: Verify/Expand */
/** Send a signal to the Edje object
 * @param obj A vaild Evas_Object handle
 * @param emission The signal
 * @param source The signal source
 *
 * This sends a signal to the edje object.
 *
 * An edje program can respond to a signal by specifying matching 'signal'
 * and 'source' fields.
 *
 * E.g.
 *
 * @code
 * edje_object_signal_emit(obj, "a_signal", "");
 * @endcode
 *
 * will trigger a program whose edc is:
 *
 * @code
 * program {
 *  name: "a_program";
 *  signal: "a_signal";
 *  source: "";
 *  action: ...
 * }
 * @endcode
 *
 * FIXME should this signal be sent to children also?
 */
EAPI void
edje_object_signal_emit(Evas_Object *obj, const char *emission, const char *source)
{
   Edje *ed;

   if ((!emission) || (!source)) return;
   ed = _edje_fetch(obj);
   if (!ed) return;
   if (ed->delete_me) return;
   _edje_emit(ed, (char *)emission, (char *)source);
}

/* FIXDOC: Verify/Expand */
/** Set the Edje to play or pause
 * @param obj A valid Evas_Object handle
 * @param play Play instruction (1 to play, 0 to pause)
 *
 * This sets the Edje to play or pause depending on the parameter.
 * This has no effect if the Edje is already in that state.
 */
EAPI void
edje_object_play_set(Evas_Object *obj, int play)
{
   Edje *ed;
   double t;
   Eina_List *l;
   Edje_Running_Program *runp;
   int i;

   ed = _edje_fetch(obj);
   if (!ed) return;
   if (ed->delete_me) return;
   if (play)
     {
	if (!ed->paused) return;
	ed->paused = 0;
	t = ecore_time_get() - ed->paused_at;
	EINA_LIST_FOREACH(ed->actions, l, runp)
	  runp->start_time += t;
     }
   else
     {
	if (ed->paused) return;
	ed->paused = 1;
	ed->paused_at = ecore_time_get();
     }

   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *rp;
	rp = ed->table_parts[i];
	if (rp->part->type == EDJE_PART_TYPE_GROUP && rp->swallowed_object)
	  edje_object_play_set(rp->swallowed_object, play);
     }
}

/* FIXDOC: Verify/Expand */
/** Get the Edje play/pause state
 * @param obj A valid Evas_Object handle
 * @return 0 if Edje not connected, Edje delete_me, or Edje paused\n
 * 1 if Edje set to play
 */
EAPI int
edje_object_play_get(const Evas_Object *obj)
{
   Edje *ed;

   ed = _edje_fetch(obj);
   if (!ed) return 0;
   if (ed->delete_me) return 0;
   if (ed->paused) return 0;
   return 1;
}

/* FIXDOC: Verify/Expand */
/** Set Animation state
 * @param obj A valid Evas_Object handle
 * @param on Animation State
 *
 * Stop or start an Edje animation.
 */
EAPI void
edje_object_animation_set(Evas_Object *obj, int on)
{
   Edje *ed;
   Eina_List *l;
   int i;

   ed = _edje_fetch(obj);
   if (!ed) return;
   if (ed->delete_me) return;
   _edje_block(ed);
   ed->no_anim = !on;
   _edje_freeze(ed);
   if (!on)
     {
	Eina_List *newl = NULL;
	const void *data;

	EINA_LIST_FOREACH(ed->actions, l, data)
	  newl = eina_list_append(newl, data);
	while (newl)
	  {
	     Edje_Running_Program *runp;

	     runp = eina_list_data_get(newl);
	     newl = eina_list_remove(newl, eina_list_data_get(newl));
	     _edje_program_run_iterate(runp, runp->start_time + runp->program->tween.time);
	     if (_edje_block_break(ed))
	       {
		  eina_list_free(newl);
		  goto break_prog;
	       }
	  }
     }
   else
     {
	_edje_emit(ed, "load", NULL);
	if (evas_object_visible_get(obj))
	  {
	     evas_object_hide(obj);
	     evas_object_show(obj);
	  }
     }
   break_prog:

   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *rp;
	rp = ed->table_parts[i];
	if (rp->part->type == EDJE_PART_TYPE_GROUP && rp->swallowed_object)
	  edje_object_animation_set(rp->swallowed_object, on);
     }

   _edje_thaw(ed);
   _edje_unblock(ed);
}

/* FIXDOC: Verify/Expand */
/** Get the animation state
 * @param obj A valid Evas_Object handle
 * @return 0 on Error or if not animated\n
 * 1 if animated
 */
EAPI int
edje_object_animation_get(const Evas_Object *obj)
{
   Edje *ed;

   ed = _edje_fetch(obj);
   if (!ed) return 0;
   if (ed->delete_me) return 0;
   if (ed->no_anim) return 0;
   return 1;
}

/* Private Routines */

int
_edje_program_run_iterate(Edje_Running_Program *runp, double tim)
{
   double t, total;
   Eina_List *l;
   Edje *ed;
   Edje_Program_Target *pt;
   Edje_Real_Part *rp;

   ed = runp->edje;
   if (ed->delete_me) return 0;
   _edje_block(ed);
   _edje_ref(ed);
   _edje_freeze(ed);
   t = tim - runp->start_time;
   total = runp->program->tween.time;
   t /= total;
   if (t > 1.0) t = 1.0;
   EINA_LIST_FOREACH(runp->program->targets, l, pt)
     {
	if (pt->id >= 0)
	  {
	     rp = ed->table_parts[pt->id % ed->table_parts_size];
	     if (rp) _edje_part_pos_set(ed, rp,
					runp->program->tween.mode, t);
	  }
     }
   if (t >= 1.0)
     {
        Edje_Program_After *pa;

        EINA_LIST_FOREACH(runp->program->targets, l, pt)
	  {
	     if (pt->id >= 0)
	       {
		  rp = ed->table_parts[pt->id % ed->table_parts_size];
		  if (rp)
		    {
		       _edje_part_description_apply(ed, rp,
						    runp->program->state,
						    runp->program->value,
						    NULL,
						    0.0);
		       _edje_part_pos_set(ed, rp,
					  runp->program->tween.mode, 0.0);
		       rp->program = NULL;
		    }
	       }
	  }
	_edje_recalc(ed);
	runp->delete_me = 1;
	if (!ed->walking_actions)
	  {
	     _edje_anim_count--;
	     ed->actions = eina_list_remove(ed->actions, runp);
	     if (!ed->actions)
	       _edje_animators = eina_list_remove(_edje_animators, ed);
	  }
//	_edje_emit(ed, "program,stop", runp->program->name);
	if (_edje_block_break(ed))
	  {
	     if (!ed->walking_actions) free(runp);
	     goto break_prog;
	  }
	EINA_LIST_FOREACH(runp->program->after, l, pa)
	  {
	     Edje_Program *pr;

	     if (pa->id >= 0)
	       {
		  pr = ed->table_programs[pa->id % ed->table_programs_size];
		  if (pr) _edje_program_run(ed, pr, 0, "", "");
		  if (_edje_block_break(ed))
		    {
		       if (!ed->walking_actions) free(runp);
		       goto break_prog;
		    }
	       }
	  }
	_edje_thaw(ed);
	_edje_unref(ed);
	if (!ed->walking_actions) free(runp);
	_edje_unblock(ed);
	return  0;
     }
   break_prog:
   _edje_recalc(ed);
   _edje_thaw(ed);
   _edje_unref(ed);
   _edje_unblock(ed);
   return 1;
}

void
_edje_program_end(Edje *ed, Edje_Running_Program *runp)
{
   Eina_List *l;
   Edje_Program_Target *pt;
   const char *pname = NULL;
   int free_runp = 0;

   if (ed->delete_me) return;
   _edje_ref(ed);
   _edje_freeze(ed);
   EINA_LIST_FOREACH(runp->program->targets, l, pt)
     {
	Edje_Real_Part *rp;

	if (pt->id >= 0)
	  {
	     rp = ed->table_parts[pt->id % ed->table_parts_size];
	     if (rp)
	       {
		  _edje_part_description_apply(ed, rp,
					       runp->program->state,
					       runp->program->value,
					       NULL,
					       0.0);
		  _edje_part_pos_set(ed, rp,
				     runp->program->tween.mode, 0.0);
		  rp->program = NULL;
	       }
	  }
     }
   _edje_recalc(ed);
   runp->delete_me = 1;
   pname = runp->program->name;
   if (!ed->walking_actions)
     {
	_edje_anim_count--;
	ed->actions = eina_list_remove(ed->actions, runp);
	free_runp = 1;
	if (!ed->actions)
	  {
	     _edje_animators = eina_list_remove(_edje_animators, ed);
	  }
     }
//   _edje_emit(ed, "program,stop", pname);
   _edje_thaw(ed);
   _edje_unref(ed);
   if (free_runp) free(runp);
}

void
_edje_program_run(Edje *ed, Edje_Program *pr, int force, const char *ssig, const char *ssrc)
{
   Eina_List *l;
   Edje_Real_Part *rp;
   Edje_Program_Target *pt;
   Edje_Program *pr2;
   Edje_Program_After *pa;
   /* limit self-feeding loops in programs to 64 levels */
   static int recursions = 0;
   static int recursion_limit = 0;

   if (ed->delete_me) return;
   if ((pr->in.from > 0.0) && (pr->in.range >= 0.0) && (!force))
     {
	Edje_Pending_Program *pp;
	double r = 0.0;

	pp = calloc(1, sizeof(Edje_Pending_Program));
	if (!pp) return;
	if (pr->in.range > 0.0) r = ((double)rand() / RAND_MAX);
	pp->timer = ecore_timer_add(pr->in.from + (pr->in.range * r),
				    _edje_pending_timer_cb, pp);
	if (!pp->timer)
	  {
	     free(pp);
	     return;
	  }
	pp->edje = ed;
	pp->program = pr;
	ed->pending_actions = eina_list_append(ed->pending_actions, pp);
	return;
     }
   if ((recursions >= 64) || (recursion_limit))
     {
	printf("EDJE ERROR: programs recursing up to recursion limit of %i. Disabled.\n",
	       64);
	recursion_limit = 1;
	return;
     }
   recursions++;
   _edje_block(ed);
   _edje_ref(ed);
   _edje_freeze(ed);
   if (pr->action == EDJE_ACTION_TYPE_STATE_SET)
     {
	if ((pr->tween.time > 0.0) && (!ed->no_anim))
	  {
	     Edje_Running_Program *runp;

	     runp = calloc(1, sizeof(Edje_Running_Program));
	     EINA_LIST_FOREACH(pr->targets, l, pt)
	       {
		  if (pt->id >= 0)
		    {
		       rp = ed->table_parts[pt->id % ed->table_parts_size];
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
	       }
//	     _edje_emit(ed, "program,start", pr->name);
	     if (_edje_block_break(ed))
	       {
		  ed->actions = eina_list_append(ed->actions, runp);
		  goto break_prog;
	       }
	     if (!ed->actions)
	       _edje_animators = eina_list_append(_edje_animators, ed);
	     ed->actions = eina_list_append(ed->actions, runp);
	     runp->start_time = ecore_time_get();
	     runp->edje = ed;
	     runp->program = pr;
	     if (!_edje_timer)
	       _edje_timer = ecore_animator_add(_edje_timer_cb, NULL);
	     _edje_anim_count++;
	  }
	else
	  {
	     EINA_LIST_FOREACH(pr->targets, l, pt)
	       {
		  if (pt->id >= 0)
		    {
		       rp = ed->table_parts[pt->id % ed->table_parts_size];
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
	       }
//	     _edje_emit(ed, "program,start", pr->name);
	     if (_edje_block_break(ed)) goto break_prog;
//	     _edje_emit(ed, "program,stop", pr->name);
	     if (_edje_block_break(ed)) goto break_prog;

	     EINA_LIST_FOREACH(pr->after, l, pa)
	       {
		  if (pa->id >= 0)
		    {
		       pr2 = ed->table_programs[pa->id % ed->table_programs_size];
		       if (pr2) _edje_program_run(ed, pr2, 0, "", "");
		       if (_edje_block_break(ed)) goto break_prog;
		    }
	       }
	     _edje_recalc(ed);
	  }
     }
   else if (pr->action == EDJE_ACTION_TYPE_ACTION_STOP)
     {
//	_edje_emit(ed, "program,start", pr->name);
        EINA_LIST_FOREACH(pr->targets, l, pt)
	  {
	     Eina_List *ll;
	     Edje_Running_Program *runp;
	     Edje_Pending_Program *pp;

	     EINA_LIST_FOREACH(ed->actions, ll, runp)
	       {
		  if (pt->id == runp->program->id)
		    {
		       _edje_program_end(ed, runp);
		       goto done;
		    }
	       }
	     EINA_LIST_FOREACH(ed->pending_actions, ll, pp)
	       {
		  if (pt->id == pp->program->id)
		    {
		       ed->pending_actions = eina_list_remove(ed->pending_actions, pp);
		       ecore_timer_del(pp->timer);
		       free(pp);
		       goto done;
		    }
	       }
	     done:
	        continue;
	  }
//	_edje_emit(ed, "program,stop", pr->name);
	if (_edje_block_break(ed)) goto break_prog;
     }
   else if (pr->action == EDJE_ACTION_TYPE_SIGNAL_EMIT)
     {
//	_edje_emit(ed, "program,start", pr->name);
	if (_edje_block_break(ed)) goto break_prog;
	_edje_emit(ed, pr->state, pr->state2);
	if (_edje_block_break(ed)) goto break_prog;
//	_edje_emit(ed, "program,stop", pr->name);
	if (_edje_block_break(ed)) goto break_prog;
     }
   else if (pr->action == EDJE_ACTION_TYPE_DRAG_VAL_SET)
     {
//	_edje_emit(ed, "program,start", pr->name);
	if (_edje_block_break(ed)) goto break_prog;
	EINA_LIST_FOREACH(pr->targets, l, pt)
	  {
	     if (pt->id >= 0)
	       {
		  rp = ed->table_parts[pt->id % ed->table_parts_size];
		  if ((rp) && (rp->drag.down.count == 0))
		    {
		       rp->drag.val.x = pr->value;
		       rp->drag.val.y = pr->value2;
		       if      (rp->drag.val.x < 0.0) rp->drag.val.x = 0.0;
		       else if (rp->drag.val.x > 1.0) rp->drag.val.x = 1.0;
		       if      (rp->drag.val.y < 0.0) rp->drag.val.y = 0.0;
		       else if (rp->drag.val.y > 1.0) rp->drag.val.y = 1.0;
		       _edje_dragable_pos_set(ed, rp, rp->drag.val.x, rp->drag.val.y);
		       _edje_emit(ed, "drag,set", rp->part->name);
		       if (_edje_block_break(ed)) goto break_prog;
		    }
	       }
	  }
//	_edje_emit(ed, "program,stop", pr->name);
	if (_edje_block_break(ed)) goto break_prog;
     }
   else if (pr->action == EDJE_ACTION_TYPE_DRAG_VAL_STEP)
     {
//	_edje_emit(ed, "program,start", pr->name);
	if (_edje_block_break(ed)) goto break_prog;
	EINA_LIST_FOREACH(pr->targets, l, pt)
	  {
	     if (pt->id >= 0)
	       {
		  rp = ed->table_parts[pt->id % ed->table_parts_size];
		  if ((rp) && (rp->drag.down.count == 0))
		    {
		       rp->drag.val.x += pr->value * rp->drag.step.x * rp->part->dragable.x;
		       rp->drag.val.y += pr->value2 * rp->drag.step.y * rp->part->dragable.y;
		       if      (rp->drag.val.x < 0.0) rp->drag.val.x = 0.0;
		       else if (rp->drag.val.x > 1.0) rp->drag.val.x = 1.0;
		       if      (rp->drag.val.y < 0.0) rp->drag.val.y = 0.0;
		       else if (rp->drag.val.y > 1.0) rp->drag.val.y = 1.0;
		       _edje_dragable_pos_set(ed, rp, rp->drag.val.x, rp->drag.val.y);
		       _edje_emit(ed, "drag,step", rp->part->name);
		       if (_edje_block_break(ed)) goto break_prog;
		    }
	       }
	  }
//	_edje_emit(ed, "program,stop", pr->name);
	if (_edje_block_break(ed)) goto break_prog;
     }
   else if (pr->action == EDJE_ACTION_TYPE_DRAG_VAL_PAGE)
     {
//	_edje_emit(ed, "program,start", pr->name);
	if (_edje_block_break(ed)) goto break_prog;
	EINA_LIST_FOREACH(pr->targets, l, pt)
	  {
	     if (pt->id >= 0)
	       {
		  rp = ed->table_parts[pt->id % ed->table_parts_size];
		  if ((rp) && (rp->drag.down.count == 0))
		    {
		       rp->drag.val.x += pr->value * rp->drag.page.x * rp->part->dragable.x;
		       rp->drag.val.y += pr->value2 * rp->drag.page.y * rp->part->dragable.y;
		       if      (rp->drag.val.x < 0.0) rp->drag.val.x = 0.0;
		       else if (rp->drag.val.x > 1.0) rp->drag.val.x = 1.0;
		       if      (rp->drag.val.y < 0.0) rp->drag.val.y = 0.0;
		       else if (rp->drag.val.y > 1.0) rp->drag.val.y = 1.0;
		       _edje_dragable_pos_set(ed, rp, rp->drag.val.x, rp->drag.val.y);
		       _edje_emit(ed, "drag,page", rp->part->name);
		       if (_edje_block_break(ed)) goto break_prog;
		    }
	       }
	  }
//	_edje_emit(ed, "program,stop", pr->name);
	if (_edje_block_break(ed)) goto break_prog;
     }
   else if (pr->action == EDJE_ACTION_TYPE_SCRIPT)
     {
	char fname[128];

//	_edje_emit(ed, "program,start", pr->name);
	if (_edje_block_break(ed)) goto break_prog;
	snprintf(fname, sizeof(fname), "_p%i", pr->id);
	_edje_embryo_test_run(ed, fname, ssig, ssrc);
//	_edje_emit(ed, "program,stop", pr->name);
	if (_edje_block_break(ed)) goto break_prog;
	_edje_recalc(ed);
     }
   else if (pr->action == EDJE_ACTION_TYPE_FOCUS_SET)
     {
	if (!pr->targets)
	  {
	     ed->focused_part = NULL;
	  }
	else
	  {
	    EINA_LIST_FOREACH(pr->targets, l, pt)
	       {
		  if (pt->id >= 0)
		    {
		       rp = ed->table_parts[pt->id % ed->table_parts_size];
		       if (rp)
			 {
			    if (ed->focused_part != rp)
			      {
				 if (ed->focused_part)
				   _edje_emit(ed, "focus,part,out", 
					      ed->focused_part->part->name);
				 ed->focused_part = rp;
				 _edje_emit(ed, "focus,part,in",
					    ed->focused_part->part->name);
			      }
 			 }
		    }
	       }
	  }
     }
   else
     {
//	_edje_emit(ed, "program,start", pr->name);
//	_edje_emit(ed, "program,stop", pr->name);
     }
   if (!((pr->action == EDJE_ACTION_TYPE_STATE_SET)
	 /* hmm this fucks somethgin up. must look into it later */
	 /* && (pr->tween.time > 0.0) && (!ed->no_anim))) */
	 ))
     {
        EINA_LIST_FOREACH(pr->after, l, pa)
	  {
	     if (pa->id >= 0)
	       {
		  pr2 = ed->table_programs[pa->id % ed->table_programs_size];
		  if (pr2) _edje_program_run(ed, pr2, 0, "", "");
		  if (_edje_block_break(ed)) goto break_prog;
	       }
	  }
     }
   break_prog:
   _edje_thaw(ed);
   _edje_unref(ed);
   recursions--;
   if (recursions == 0) recursion_limit = 0;
   _edje_unblock(ed);
}

void
_edje_emit(Edje *ed, const char *sig, const char *src)
{
   Edje_Message_Signal emsg;

   if (ed->delete_me) return;
   emsg.sig = sig;
   emsg.src = src;
   _edje_message_send(ed, EDJE_QUEUE_SCRIPT, EDJE_MESSAGE_SIGNAL, 0, &emsg);
}

struct _Edje_Program_Data
{
#ifdef EDJE_PROGRAM_CACHE
  Eina_List     *matches;
  int            matched;
#endif
  Edje          *ed;
  const char    *signal;
  const char    *source;
};

static int _edje_glob_callback(Edje_Program *pr, void *dt)
{
   struct _Edje_Program_Data    *data = dt;

#ifdef EDJE_PROGRAM_CACHE
   data->matched++;
#endif

   _edje_program_run(data->ed, pr, 0, data->signal, data->source);
   if (_edje_block_break(data->ed))
     {
#ifdef EDJE_PROGRAM_CACHE
        eina_list_free(data->matches);
        data->matches = NULL;
#endif
        return 1;
     }

#ifdef EDJE_PROGRAM_CACHE
   data->matches = eina_list_append(data->matches, pr);
#endif

   return 0;
}


void
_edje_callbacks_patterns_clean(Edje *ed)
{
   _edje_signals_sources_patterns_clean(&ed->patterns.callbacks);
}

static void
_edje_callbacks_patterns_init(Edje *ed)
{
   Edje_Signals_Sources_Patterns *ssp = &ed->patterns.callbacks;

   if (ssp->signals_patterns)
     return;

   ssp->signals_patterns = edje_match_callback_signal_init(ed->callbacks);
   ssp->sources_patterns = edje_match_callback_source_init(ed->callbacks);
}

/* FIXME: what if we delete the evas object??? */
void
_edje_emit_handle(Edje *ed, const char *sig, const char *src)
{
   if (ed->delete_me) return;
   if (!sig) sig = "";
   if (!src) src = "";
//   printf("EDJE EMIT: signal: \"%s\" source: \"%s\"\n", sig, src);
   _edje_block(ed);
   _edje_ref(ed);
   _edje_freeze(ed);
   if (ed->collection)
     {
	Edje_Part_Collection *ec;
#ifdef EDJE_PROGRAM_CACHE
	char *tmps;
	int l1, l2;
#endif
	int done;

	ec = ed->collection;
#ifdef EDJE_PROGRAM_CACHE
	l1 = strlen(sig);
	l2 = strlen(src);
	tmps = alloca(l1 + l2 + 2);
	strcpy(tmps, sig);
	tmps[l1] = '\377';
	strcpy(&(tmps[l1 + 1]), src);
#endif
	done = 0;

#ifdef EDJE_PROGRAM_CACHE
	  {
	     Eina_List *matches;
	     Eina_List *l;
	     Edje_Program *pr;

	     if (evas_hash_find(ec->prog_cache.no_matches, tmps))
	       {
		  done = 1;
	       }
	     else if ((matches = evas_hash_find(ec->prog_cache.matches, tmps)))
	       {
		 EINA_LIST_FOREACH(matches, l, pr)
		    {
		       _edje_program_run(ed, pr, 0, sig, src);
		       if (_edje_block_break(ed))
			 {
			    goto break_prog;
			 }
		    }
		  done = 1;
	       }
	  }
#endif
	if (!done)
	  {
             struct _Edje_Program_Data  data;

             data.ed = ed;
             data.source = src;
             data.signal = sig;
#ifdef EDJE_PROGRAM_CACHE
	     data.matched = 0;
	     data.matches = NULL;
#endif
             if (ed->collection->programs)
               {
                  if (edje_match_programs_exec(ed->patterns.programs.signals_patterns,
                                               ed->patterns.programs.sources_patterns,
                                               sig,
                                               src,
                                               ed->collection->programs,
                                               _edje_glob_callback,
                                               &data) == 0)
                    goto break_prog;
               }

#ifdef EDJE_PROGRAM_CACHE
	     if (tmps)
	       {
		  if (data.matched == 0)
		    ec->prog_cache.no_matches =
		    evas_hash_add(ec->prog_cache.no_matches, tmps, ed);
		  else
		    ec->prog_cache.matches =
		    evas_hash_add(ec->prog_cache.matches, tmps, data.matches);
	       }
#endif
	  }
	_edje_emit_cb(ed, sig, src);
	if (_edje_block_break(ed))
	  {
	     goto break_prog;
	  }
     }
   break_prog:
   _edje_thaw(ed);
   _edje_unref(ed);
   _edje_unblock(ed);
}

/* FIXME: what if we delete the evas object??? */
static void
_edje_emit_cb(Edje *ed, const char *sig, const char *src)
{
   Eina_List            *l;

   if (ed->delete_me) return;
   _edje_ref(ed);
   _edje_freeze(ed);
   _edje_block(ed);

   if (ed->just_added_callbacks)
     _edje_callbacks_patterns_clean(ed);

   ed->walking_callbacks = 1;

   if (ed->callbacks)
     {
        int     r;

	_edje_callbacks_patterns_init(ed);
        r = edje_match_callback_exec(ed->patterns.callbacks.signals_patterns,
                                     ed->patterns.callbacks.sources_patterns,
                                     sig,
                                     src,
                                     ed->callbacks,
                                     ed);

        if (!r)
          goto break_prog;
     }

   ed->walking_callbacks = 0;
   if ((ed->delete_callbacks) || (ed->just_added_callbacks))
     {
        Edje_Signal_Callback *escb;

	ed->delete_callbacks = 0;
	ed->just_added_callbacks = 0;
	EINA_LIST_FOREACH(ed->callbacks, l, escb)
	  {
	     Eina_List *next_l;

	     next_l = eina_list_next(l);
	     if (escb->just_added)
	       escb->just_added = 0;
	     if (escb->delete_me)
	       {
		  ed->callbacks = eina_list_remove_list(ed->callbacks, l);
		  if (escb->signal) eina_stringshare_del(escb->signal);
		  if (escb->source) eina_stringshare_del(escb->source);
		  free(escb);
	       }
	     l = next_l;
	  }

        _edje_callbacks_patterns_clean(ed);
     }
   break_prog:
   _edje_unblock(ed);
   _edje_thaw(ed);
   _edje_unref(ed);
}
