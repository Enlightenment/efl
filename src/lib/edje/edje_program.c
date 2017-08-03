#include "edje_private.h"

static void _edje_emit_cb(Edje *ed, const char *sig, const char *src, Edje_Message_Signal_Data *data, Eina_Bool prop);
static void _edje_param_copy(Edje *ed, Edje_Real_Part *src_part, const char *src_param, Edje_Real_Part *dst_part, const char *dst_param);
static void _edje_param_set(Edje *ed, Edje_Real_Part *part, const char *param, const char *value);

Eina_List *_edje_animators = NULL;
static double _edje_transition_duration_scale = 0;

static Eina_Bool
_edje_animator_cb(void *data)
{
   const Efl_Event event = { NULL, NULL, NULL };
   _edje_timer_cb(data, &event);
   return EINA_TRUE;
}

static Eina_Bool
_edje_emit_aliased(Edje *ed, const char *part, const char *sig, const char *src)
{
   char *alias, *aliased;
   int alien, nslen, length;

   /* lookup for alias */
   if ((!ed->collection) || (!ed->collection->alias)) return EINA_FALSE;
   alias = eina_hash_find(ed->collection->alias, part);
   if (!alias) return EINA_FALSE;

   alien = strlen(alias);
   nslen = strlen(sig);
   length = alien + nslen + 2;

   aliased = alloca(length);
   memcpy(aliased, alias, alien);
   aliased[alien] = EDJE_PART_PATH_SEPARATOR;
   memcpy(aliased + alien + 1, sig, nslen + 1);

   _edje_emit(ed, aliased, src);
   return EINA_TRUE;
}

static Eina_Bool
_edje_emit_child(Edje *ed, Edje_Real_Part *rp, const char *part, const char *sig, const char *src)
{
   Edje *ed2;
   char *idx;

   /* search for the index if present and remove it from the part */
   idx = strchr(part, EDJE_PART_PATH_SEPARATOR_INDEXL);
   if (idx)
     {
        char *end;

        end = strchr(idx + 1, EDJE_PART_PATH_SEPARATOR_INDEXR);
        if (end && end != idx + 1)
          {
             char *tmp;

             tmp = alloca(end - idx);
             memcpy(tmp, idx + 1, end - idx - 1);
             tmp[end - idx - 1] = '\0';
             *idx = '\0';
             idx = tmp;
          }
        else
          {
             idx = NULL;
          }
     }

   /* search for the right part now */
   if (!rp)
     rp = _edje_real_part_get(ed, part);
   if (!rp) return ed->collection->broadcast_signal;

   switch (rp->part->type)
     {
      case EDJE_PART_TYPE_GROUP:
        if (((rp->type != EDJE_RP_TYPE_SWALLOW) ||
             (!rp->typedata.swallow)) ||
            (!rp->typedata.swallow->swallowed_object))
          break;
        ed2 = _edje_fetch(rp->typedata.swallow->swallowed_object);
        if (!ed2) break;

        _edje_emit(ed2, sig, src);
        return EINA_FALSE;

      case EDJE_PART_TYPE_EXTERNAL:
        if (((rp->type != EDJE_RP_TYPE_SWALLOW) ||
             (!rp->typedata.swallow)) ||
            (!rp->typedata.swallow->swallowed_object))
          break;

        if (!idx)
          {
             _edje_external_signal_emit(rp->typedata.swallow->swallowed_object, sig, src);
          }
        else
          {
             Evas_Object *child;

             child = _edje_children_get(rp, idx);
             ed2 = _edje_fetch(child);
             if (!ed2) break;
             _edje_emit(ed2, sig, src);
          }
        return EINA_FALSE;

      case EDJE_PART_TYPE_BOX:
      case EDJE_PART_TYPE_TABLE:
        if (idx)
          {
             Evas_Object *child;

             child = _edje_children_get(rp, idx);
             ed2 = _edje_fetch(child);
             if (!ed2) break;
             _edje_emit(ed2, sig, src);
             return EINA_FALSE;
          }
        break;

      default:
        //              ERR("SPANK SPANK SPANK !!!\nYou should never be here !");
        break;
     }
   return ed->collection->broadcast_signal;
}

static void
_edje_emit_send(Edje *ed, Eina_Bool broadcast, const char *sig, const char *src, void *data, Ecore_Cb free_func)
{
   Edje_Message_Signal emsg;

   emsg.sig = sig;
   emsg.src = src;
   if (data)
     {
        emsg.data = calloc(1, sizeof(*(emsg.data)));
        emsg.data->ref = 1;
        emsg.data->data = data;
        emsg.data->free_func = free_func;
     }
   else
     {
        emsg.data = NULL;
     }
   /* new sends code */
   if (broadcast)
     edje_object_message_send(ed->obj, EDJE_MESSAGE_SIGNAL, 0, &emsg);
   else
     _edje_util_message_send(ed, EDJE_QUEUE_SCRIPT, EDJE_MESSAGE_SIGNAL, 0, &emsg);
   /* old send code - use api now
      _edje_util_message_send(ed, EDJE_QUEUE_SCRIPT, EDJE_MESSAGE_SIGNAL, 0, &emsg);
      EINA_LIST_FOREACH(ed->subobjs, l, obj)
      {
      Edje *ed2;

      ed2 = _edje_fetch(obj);
      if (!ed2) continue;
      if (ed2->delete_me) continue;
      _edje_util_message_send(ed2, EDJE_QUEUE_SCRIPT, EDJE_MESSAGE_SIGNAL, 0, &emsg);
      }
    */
   if (emsg.data && (--(emsg.data->ref) == 0))
     {
        if (emsg.data->free_func)
          {
             emsg.data->free_func(emsg.data->data);
          }
        free(emsg.data);
     }
}

/*============================================================================*
*                                   API                                      *
*============================================================================*/

EOLIAN Eina_Stringshare*
_edje_object_seat_name_get(Eo *obj EINA_UNUSED, Edje *ed, Efl_Input_Device *device)
{
   return _edje_seat_name_get(ed, device);
}

EOLIAN Efl_Input_Device *
_edje_object_seat_get(Eo *obj EINA_UNUSED, Edje *ed, Eina_Stringshare *name)
{
   return _edje_seat_get(ed, name);
}

EAPI void
edje_frametime_set(double t)
{
   ecore_animator_frametime_set(t);
}

EAPI double
edje_frametime_get(void)
{
   return ecore_animator_frametime_get();
}

EAPI double
edje_transition_duration_factor_get(void)
{
   return _edje_transition_duration_scale;
}

EAPI void
edje_transition_duration_factor_set(double scale)
{
   _edje_transition_duration_scale = FROM_DOUBLE(scale);
}

EOLIAN void
_edje_object_transition_duration_factor_set(Eo *obj EINA_UNUSED,
                                            Edje *pd, double scale)
{
   pd->duration_scale = scale;
}

EOLIAN double
_edje_object_transition_duration_factor_get(Eo *obj EINA_UNUSED, Edje *pd)
{
   return pd->duration_scale;
}

static inline Eina_Bool
_edje_object_signal_callback_add(Edje *ed, const char *emission, const char *source, Efl_Signal_Cb func, void *data)
{
   Edje_Signal_Callback_Group *gp;
   const char *sig;
   const char *src;
   Eina_Bool ok;

   if (!ed->callbacks)
     ed->callbacks = _edje_signal_callback_alloc();
   if (!ed->callbacks) return EINA_FALSE;

   sig = eina_stringshare_add(emission);
   src = eina_stringshare_add(source);

   gp = (Edje_Signal_Callback_Group *) ed->callbacks;
   ok = _edje_signal_callback_push(gp, sig, src, func, data, EINA_TRUE);

   eina_stringshare_del(sig);
   eina_stringshare_del(src);

   return ok;
}

void
edje_object_propagate_callback_add(Evas_Object *obj, Efl_Signal_Cb func, void *data)
{
   Edje *ed;

   ed = _edje_fetch(obj);
   if (!ed || ed->delete_me) return;
   _edje_object_signal_callback_add(ed, "*", "*", func, data);
}

EOLIAN Eina_Bool
_edje_object_efl_canvas_layout_signal_signal_callback_add(Eo *obj EINA_UNUSED, Edje *ed, const char *emission, const char *source, Efl_Signal_Cb func, void *data)
{
   return _edje_object_signal_callback_add(ed, emission, source, func, data);
}

EOLIAN Eina_Bool
_edje_object_efl_canvas_layout_signal_signal_callback_del(Eo *obj EINA_UNUSED, Edje *ed, const char *emission, const char *source, Efl_Signal_Cb func, void *data)
{
   Edje_Signal_Callback_Group *gp;
   Eina_Bool ok;

   if (ed->delete_me) return EINA_FALSE;
   if ((!emission) || (!source) || (!func)) return EINA_FALSE;

   gp = (Edje_Signal_Callback_Group *) ed->callbacks;
   if (!gp) return EINA_FALSE;

   emission = eina_stringshare_add(emission);
   source = eina_stringshare_add(source);

   ok = _edje_signal_callback_disable(gp, emission, source, func, data);

   eina_stringshare_del(emission);
   eina_stringshare_del(source);

   return ok;
}

EOLIAN void
_edje_object_efl_canvas_layout_signal_signal_emit(Eo *obj EINA_UNUSED, Edje *ed, const char *emission, const char *source)
{
   if (ed->delete_me) return;
   if ((!emission) || (!source)) return;
   _edje_emit(ed, emission, source);
}

/* FIXDOC: Verify/Expand */
EOLIAN void
_edje_object_play_set(Eo *obj EINA_UNUSED, Edje *ed, Eina_Bool play)
{
   double t;
   Eina_List *l;
   Edje_Running_Program *runp;
   unsigned short i;

   if (!ed) return;
   if (ed->delete_me) return;
   if (play)
     {
        if (!ed->paused) return;
        ed->paused = EINA_FALSE;
        t = ecore_time_get() - ed->paused_at;
        EINA_LIST_FOREACH(ed->actions, l, runp)
          runp->start_time += t;
     }
   else
     {
        if (ed->paused) return;
        ed->paused = EINA_TRUE;
        ed->paused_at = ecore_time_get();
     }

   for (i = 0; i < ed->table_parts_size; i++)
     {
        Edje_Real_Part *rp;
        rp = ed->table_parts[i];
        if ((rp->part->type == EDJE_PART_TYPE_GROUP) &&
            ((rp->type == EDJE_RP_TYPE_SWALLOW) &&
             (rp->typedata.swallow)) &&
            (rp->typedata.swallow->swallowed_object))
          edje_object_play_set(rp->typedata.swallow->swallowed_object, play);
     }
}

EOLIAN Eina_Bool
_edje_object_play_get(Eo *obj EINA_UNUSED, Edje *ed)
{
   if (!ed) return EINA_FALSE;
   if (ed->delete_me) return EINA_FALSE;
   if (ed->paused) return EINA_FALSE;

   return EINA_TRUE;
}

/* FIXDOC: Verify/Expand */
EOLIAN void
_edje_object_animation_set(Eo *obj, Edje *ed, Eina_Bool on)
{
   Eina_List *l;
   unsigned short i;

   if (!ed) return;
   if (ed->delete_me) return;
   _edje_block(ed);
   ed->no_anim = !on;
   _edje_util_freeze(ed);
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
             _edje_program_run_iterate(runp, runp->start_time + TO_DOUBLE(runp->program->tween.time));
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
        if ((rp->part->type == EDJE_PART_TYPE_GROUP) &&
            ((rp->type == EDJE_RP_TYPE_SWALLOW) &&
             (rp->typedata.swallow)) &&
            (rp->typedata.swallow->swallowed_object))
          edje_object_animation_set(rp->typedata.swallow->swallowed_object, on);
     }

   _edje_util_thaw(ed);
   _edje_unblock(ed);
}

EOLIAN Eina_Bool
_edje_object_animation_get(Eo *obj EINA_UNUSED, Edje *ed)
{
   if (!ed) return EINA_FALSE;
   if (ed->delete_me) return EINA_FALSE;
   if (ed->no_anim) return EINA_FALSE;

   return EINA_TRUE;
}

/* Private Routines */
void
_edje_program_run_cleanup(Edje *ed, Edje_Running_Program *runp)
{
   ed->actions = eina_list_remove(ed->actions, runp);
   if (!ed->actions)
     {
        efl_event_callback_del(ed->obj, EFL_EVENT_ANIMATOR_TICK, _edje_timer_cb, ed);
        ecore_animator_del(ed->animator);
        ed->animator = NULL;
     }
}

Eina_Bool
_edje_program_run_iterate(Edje_Running_Program *runp, double tim)
{
   FLOAT_T t, total, t_scale = runp->edje->duration_scale;
   Eina_List *l;
   Edje *ed;
   Edje_Program_Target *pt;
   Edje_Real_Part *rp;

   ed = runp->edje;
   if (ed->delete_me) return EINA_FALSE;
   _edje_block(ed);
   _edje_ref(ed);
   _edje_util_freeze(ed);

   if (runp->program->tween.use_duration_factor)
     t_scale = _edje_transition_duration_scale;
   t = FROM_DOUBLE(tim - runp->start_time);
   total = runp->program->tween.time * t_scale;
   t = DIV(t, total);
   if (t > FROM_INT(1)) t = FROM_INT(1);
   EINA_LIST_FOREACH(runp->program->targets, l, pt)
     {
        if (pt->id >= 0)
          {
             rp = ed->table_parts[pt->id % ed->table_parts_size];
             if (rp)
               _edje_part_pos_set(ed, rp,
                                  runp->program->tween.mode, t,
                                  runp->program->tween.v1,
                                  runp->program->tween.v2,
                                  runp->program->tween.v3,
                                  runp->program->tween.v4);
          }
     }
   if (t >= FROM_INT(1))
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
                                          runp->program->tween.mode, ZERO,
                                          runp->program->tween.v1,
                                          runp->program->tween.v2,
                                          runp->program->tween.v3,
                                          runp->program->tween.v4);
                       rp->program = NULL;
                    }
               }
          }
        _edje_recalc(ed);
        runp->delete_me = EINA_TRUE;
        if (!ed->walking_actions)
          _edje_program_run_cleanup(ed, runp);
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
                  pr = ed->collection->patterns.table_programs[pa->id % ed->collection->patterns.table_programs_size];
                  if (pr) _edje_program_run(ed, pr, 0, "", "");
                  if (_edje_block_break(ed))
                    {
                       if (!ed->walking_actions) free(runp);
                       goto break_prog;
                    }
               }
          }
        _edje_util_thaw(ed);
        _edje_unref(ed);
        if (!ed->walking_actions) free(runp);
        _edje_unblock(ed);
        return EINA_FALSE;
     }
break_prog:
   _edje_recalc(ed);
   _edje_util_thaw(ed);
   _edje_unref(ed);
   _edje_unblock(ed);
   return EINA_TRUE;
}

void
_edje_program_end(Edje *ed, Edje_Running_Program *runp)
{
   Eina_List *l;
   Edje_Program_Target *pt;
   //   const char *pname = NULL;
   int free_runp = 0;

   if (ed->delete_me) return;
   _edje_ref(ed);
   _edje_util_freeze(ed);
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
                                     runp->program->tween.mode, ZERO,
                                     runp->program->tween.v1,
                                     runp->program->tween.v2,
                                     runp->program->tween.v3,
                                     runp->program->tween.v4);

                  rp->program = NULL;
               }
          }
     }
   _edje_recalc(ed);
   runp->delete_me = EINA_TRUE;
   //   pname = runp->program->name;
   if (!ed->walking_actions)
     {
        _edje_program_run_cleanup(ed, runp);
        free_runp = 1;
     }
   //   _edje_emit(ed, "program,stop", pname);
   _edje_util_thaw(ed);
   _edje_unref(ed);
   if (free_runp) free(runp);
}

#ifdef HAVE_EPHYSICS
static Eina_Bool
_edje_physics_action_set(Edje *ed, Edje_Program *pr, void (*func)(EPhysics_Body *body, double x, double y, double z))
{
   Edje_Program_Target *pt;
   Edje_Real_Part *rp;
   Eina_List *l;

   if (_edje_block_break(ed)) return EINA_FALSE;

   EINA_LIST_FOREACH(pr->targets, l, pt)
     {
        if (pt->id >= 0)
          {
             rp = ed->table_parts[pt->id % ed->table_parts_size];
             if ((rp) && (rp->body))
               func(rp->body, pr->physics.x, pr->physics.y, pr->physics.z);
          }
     }

   return EINA_TRUE;
}

#endif

static void
_edje_seat_name_emit(Edje *ed, const char *name, const char *sig, const char *src)
{
   char buf[128];

   /* keep sending signals without seat information for legacy compatibility */
   _edje_emit_full(ed, sig, src, NULL, NULL);

   if (!name) return;

   snprintf(buf, sizeof(buf), "seat,%s,%s", name, sig);
   _edje_emit_full(ed, buf, src, NULL, NULL);
}

void
_edje_part_focus_set(Edje *ed, const char *seat_name, Edje_Real_Part *rp)
{
   Edje_Real_Part *focused_part;
   Eina_Stringshare *sname;
   Efl_Input_Device *seat;
   Evas *e;

   if (seat_name)
     sname = eina_stringshare_add(seat_name);
   else /* Use default seat name */
     {
        e = evas_object_evas_get(ed->obj);
        seat = evas_canvas_default_device_get(e, EFL_INPUT_DEVICE_TYPE_SEAT);
        sname = eina_stringshare_ref(_edje_seat_name_get(ed, seat));
     }

   focused_part = _edje_focused_part_get(ed, sname);

   if (focused_part != rp)
     {
        if (rp && (rp->part->allowed_seats) &&
            (!_edje_part_allowed_seat_find(rp, sname)))
          goto not_allowed;

        if (focused_part)
          _edje_seat_name_emit(ed, sname, "focus,part,out",
                               focused_part->part->name);
        _edje_focused_part_set(ed, sname, rp);
        if (rp)
          _edje_seat_name_emit(ed, sname, "focus,part,in", rp->part->name);
     }

not_allowed:
   eina_stringshare_del(sname);
}

void
_edje_program_run(Edje *ed, Edje_Program *pr, Eina_Bool force, const char *ssig, const char *ssrc)
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
        ERR("Programs recursing up to recursion limit of %i in '%s' with sig='%s', src='%s' from '%s', '%s'. Disabled.",
            64, pr->name, ssig, ssrc, ed->path, ed->group);
        if (pr->action == EDJE_ACTION_TYPE_STATE_SET && (EQ(pr->tween.time, ZERO) || (ed->no_anim)))
          ERR("Possible solution: try adding transition time to prevent Schrödinger's part state");
        recursion_limit = 1;
        return;
     }
   recursions++;
   _edje_block(ed);
   _edje_ref(ed);
   _edje_util_freeze(ed);
   switch (pr->action)
     {
      case EDJE_ACTION_TYPE_STATE_SET:
        if ((pr->tween.time > ZERO) && (!ed->no_anim))
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
                            if ((rp->object) && (pr->tween.mode & EDJE_TWEEN_MODE_OPT_FROM_CURRENT))
                              {
                                 Edje_Calc_Params *tmp;

                                 tmp = calloc(1, sizeof(Edje_Calc_Params));
                                 if (!tmp) goto low_mem_current;
                                 _edje_part_recalc(ed, rp, FLAG_XY, tmp);

                                 if (rp->current)
                                   {
#ifdef EDJE_CALC_CACHE
                                      _edje_calc_params_clear(rp->current);
#endif
                                      free(rp->current);
                                   }
                                 rp->current = tmp;
                              }
                            else
                              {
low_mem_current:
                                 if (rp->current)
                                   {
#ifdef EDJE_CALC_CACHE
                                      _edje_calc_params_clear(rp->current);
#endif
                                      free(rp->current);
                                   }
                                 rp->current = NULL;
                              }

                            if (rp->program)
                              _edje_program_end(ed, rp->program);
                            _edje_part_description_apply(ed, rp,
                                                         rp->param1.description->state.name,
                                                         rp->param1.description->state.value,
                                                         pr->state,
                                                         pr->value);
                            _edje_part_pos_set(ed, rp, pr->tween.mode, ZERO,
                                               pr->tween.v1,
                                               pr->tween.v2,
                                               pr->tween.v3,
                                               pr->tween.v4);
                            rp->program = runp;
                         }
                    }
               }
             // _edje_emit(ed, "program,start", pr->name);
             if (_edje_block_break(ed))
               {
                  ed->actions = eina_list_append(ed->actions, runp);
                  goto break_prog;
               }

             if (!ed->actions)
               {
                  if (ed->canvas_animator)
                    efl_event_callback_add(ed->obj, EFL_EVENT_ANIMATOR_TICK, _edje_timer_cb, ed);
                  else
                    ed->animator = ecore_animator_add(_edje_animator_cb, ed);
               }
             ed->actions = eina_list_append(ed->actions, runp);

             runp->start_time = ecore_loop_time_get();
             runp->edje = ed;
             runp->program = pr;
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
                            _edje_part_pos_set(ed, rp, pr->tween.mode, ZERO,
                                               pr->tween.v1,
                                               pr->tween.v2,
                                               pr->tween.v3,
                                               pr->tween.v4);
                         }
                    }
               }
             // _edje_emit(ed, "program,start", pr->name);
             if (_edje_block_break(ed)) goto break_prog;
             // _edje_emit(ed, "program,stop", pr->name);
             if (_edje_block_break(ed)) goto break_prog;

             EINA_LIST_FOREACH(pr->after, l, pa)
               {
                  if (pa->id >= 0)
                    {
                       pr2 = ed->collection->patterns.table_programs[pa->id % ed->collection->patterns.table_programs_size];
                       if (pr2) _edje_program_run(ed, pr2, 0, "", "");
                       if (_edje_block_break(ed)) goto break_prog;
                    }
               }
             _edje_recalc(ed);
          }
        break;

      case EDJE_ACTION_TYPE_ACTION_STOP:
        // _edje_emit(ed, "program,start", pr->name);
        EINA_LIST_FOREACH(pr->targets, l, pt)
          {
             Eina_List *ll;
             Edje_Running_Program *runp;
             Edje_Pending_Program *pp;

             for (ll = ed->actions; ll; )
               {
                  runp = ll->data;
                  ll = ll->next;
                  if (pt->id == runp->program->id)
                    {
                       _edje_program_end(ed, runp);
                       //		       goto done;
                    }
               }
             for (ll = ed->pending_actions; ll; )
               {
                  pp = ll->data;
                  ll = ll->next;
                  if (pt->id == pp->program->id)
                    {
                       ed->pending_actions = eina_list_remove(ed->pending_actions, pp);
                       ecore_timer_del(pp->timer);
                       pp->timer = NULL;
                       free(pp);
                       //		       goto done;
                    }
               }
             //	     done:
             //	        continue;
          }
        // _edje_emit(ed, "program,stop", pr->name);
        if (_edje_block_break(ed)) goto break_prog;
        break;

      case EDJE_ACTION_TYPE_SIGNAL_EMIT:
        // _edje_emit(ed, "program,start", pr->name);
        if (_edje_block_break(ed)) goto break_prog;
        if (pr->targets)
          {
             EINA_LIST_FOREACH(pr->targets, l, pt)
               {
                  if (pt->id < 0) continue;
                  rp = ed->table_parts[pt->id % ed->table_parts_size];
                  if (!rp) continue;
                  if (!_edje_emit_aliased(ed, rp->part->name, pr->state, pr->state2))
                    {
                       Eina_Bool broadcast;

                       broadcast = _edje_emit_child(ed, rp, rp->part->name, pr->state, pr->state2);
                       _edje_emit_send(ed, broadcast, pr->state, pr->state2, NULL, NULL);
                    }
               }
          }
        else
          _edje_emit(ed, pr->state, pr->state2);
        if (_edje_block_break(ed)) goto break_prog;
        // _edje_emit(ed, "program,stop", pr->name);
        if (_edje_block_break(ed)) goto break_prog;
        break;

      case EDJE_ACTION_TYPE_DRAG_VAL_SET:
        // _edje_emit(ed, "program,start", pr->name);
        if (_edje_block_break(ed)) goto break_prog;
        EINA_LIST_FOREACH(pr->targets, l, pt)
          {
             if (pt->id >= 0)
               {
                  rp = ed->table_parts[pt->id % ed->table_parts_size];
                  if ((rp) && (rp->drag) && (rp->drag->down.count == 0))
                    {
                       rp->drag->val.x = pr->value;
                       rp->drag->val.y = pr->value2;
                       if (rp->drag->val.x < 0.0) rp->drag->val.x = 0.0;
                       else if (rp->drag->val.x > 1.0)
                         rp->drag->val.x = 1.0;
                       if (rp->drag->val.y < 0.0) rp->drag->val.y = 0.0;
                       else if (rp->drag->val.y > 1.0)
                         rp->drag->val.y = 1.0;
                       _edje_dragable_pos_set(ed, rp, rp->drag->val.x, rp->drag->val.y);
                       _edje_emit(ed, "drag,set", rp->part->name);
                       if (_edje_block_break(ed)) goto break_prog;
                    }
               }
          }
        // _edje_emit(ed, "program,stop", pr->name);
        if (_edje_block_break(ed)) goto break_prog;
        break;

      case EDJE_ACTION_TYPE_DRAG_VAL_STEP:
        // _edje_emit(ed, "program,start", pr->name);
        if (_edje_block_break(ed)) goto break_prog;
        EINA_LIST_FOREACH(pr->targets, l, pt)
          {
             if (pt->id >= 0)
               {
                  rp = ed->table_parts[pt->id % ed->table_parts_size];
                  if ((rp) && (rp->drag) && (rp->drag->down.count == 0))
                    {
                       rp->drag->val.x += pr->value * rp->drag->step.x * rp->part->dragable.x;
                       rp->drag->val.y += pr->value2 * rp->drag->step.y * rp->part->dragable.y;
                       if (rp->drag->val.x < 0.0) rp->drag->val.x = 0.0;
                       else if (rp->drag->val.x > 1.0)
                         rp->drag->val.x = 1.0;
                       if (rp->drag->val.y < 0.0) rp->drag->val.y = 0.0;
                       else if (rp->drag->val.y > 1.0)
                         rp->drag->val.y = 1.0;
                       _edje_dragable_pos_set(ed, rp, rp->drag->val.x, rp->drag->val.y);
                       _edje_emit(ed, "drag,step", rp->part->name);
                       if (_edje_block_break(ed)) goto break_prog;
                    }
               }
          }
        // _edje_emit(ed, "program,stop", pr->name);
        if (_edje_block_break(ed)) goto break_prog;
        break;

      case EDJE_ACTION_TYPE_DRAG_VAL_PAGE:
        // _edje_emit(ed, "program,start", pr->name);
        if (_edje_block_break(ed)) goto break_prog;
        EINA_LIST_FOREACH(pr->targets, l, pt)
          {
             if (pt->id >= 0)
               {
                  rp = ed->table_parts[pt->id % ed->table_parts_size];
                  if ((rp) && (rp->drag) && (rp->drag->down.count == 0))
                    {
                       rp->drag->val.x += pr->value * rp->drag->page.x * rp->part->dragable.x;
                       rp->drag->val.y += pr->value2 * rp->drag->page.y * rp->part->dragable.y;
                       if (rp->drag->val.x < 0.0) rp->drag->val.x = 0.0;
                       else if (rp->drag->val.x > 1.0)
                         rp->drag->val.x = 1.0;
                       if (rp->drag->val.y < 0.0) rp->drag->val.y = 0.0;
                       else if (rp->drag->val.y > 1.0)
                         rp->drag->val.y = 1.0;
                       _edje_dragable_pos_set(ed, rp, rp->drag->val.x, rp->drag->val.y);
                       _edje_emit(ed, "drag,page", rp->part->name);
                       if (_edje_block_break(ed)) goto break_prog;
                    }
               }
          }
        // _edje_emit(ed, "program,stop", pr->name);
        if (_edje_block_break(ed)) goto break_prog;
        break;

      case EDJE_ACTION_TYPE_SCRIPT:
      {
         // _edje_emit(ed, "program,start", pr->name);
         if (_edje_block_break(ed)) goto break_prog;
         _edje_embryo_test_run(ed, pr, ssig, ssrc);
         // _edje_emit(ed, "program,stop", pr->name);
         if (_edje_block_break(ed)) goto break_prog;
         _edje_recalc_do(ed);
      }
      break;

      case EDJE_ACTION_TYPE_FOCUS_SET:
      {
        if (!pr->targets)
          _edje_part_focus_set(ed, pr->seat, NULL);
        else
          {
             EINA_LIST_FOREACH(pr->targets, l, pt)
               {
                  if (pt->id >= 0)
                    {
                       rp = ed->table_parts[pt->id % ed->table_parts_size];
                       if (rp)
                         _edje_part_focus_set(ed, pr->seat, rp);
                    }
               }
          }
      }
      break;

      case EDJE_ACTION_TYPE_FOCUS_OBJECT:
      {
         Efl_Input_Device *seat = NULL;

         if (pr->seat)
           {
              Eina_Stringshare *seat_name;

              seat_name = eina_stringshare_add(pr->seat);
              seat = _edje_seat_get(ed, seat_name);
              eina_stringshare_del(seat_name);
           }
         if (!seat)
           {
              Evas *e;

              e = evas_object_evas_get(ed->obj);
              seat = evas_canvas_default_device_get(e, EFL_INPUT_DEVICE_TYPE_SEAT);
           }
        if (!pr->targets)
          {
             Evas_Object *focused;

             focused = evas_seat_focus_get(evas_object_evas_get(ed->obj), seat);
             if (focused)
               {
                  unsigned int i;

                  /* Check if the current swallowed object is one of my child. */
                  for (i = 0; i < ed->table_parts_size; ++i)
                    {
                       rp = ed->table_parts[i];
                       if ((rp) &&
                           ((rp->type == EDJE_RP_TYPE_SWALLOW) &&
                            (rp->typedata.swallow)) &&
                           (rp->typedata.swallow->swallowed_object == focused))
                         {
                            efl_canvas_object_seat_focus_del(focused, seat);
                            break;
                         }
                    }
               }
          }
        else
          {
             EINA_LIST_FOREACH(pr->targets, l, pt)
               {
                  if (pt->id >= 0)
                    {
                       rp = ed->table_parts[pt->id % ed->table_parts_size];
                       if (rp &&
                           ((rp->type == EDJE_RP_TYPE_SWALLOW) &&
                            (rp->typedata.swallow)) &&
                           (rp->typedata.swallow->swallowed_object))
                         efl_canvas_object_seat_focus_add(
                            rp->typedata.swallow->swallowed_object, seat);
                    }
               }
          }
      }
      break;

      case EDJE_ACTION_TYPE_SOUND_SAMPLE:
        if (_edje_block_break(ed))
          goto break_prog;
        _edje_multisense_internal_sound_sample_play(ed, pr->sample_name, pr->speed, pr->channel);
        break;

      case EDJE_ACTION_TYPE_SOUND_TONE:
        if (_edje_block_break(ed))
          goto break_prog;
        _edje_multisense_internal_sound_tone_play(ed, pr->tone_name, pr->duration, pr->channel);
        break;

      case EDJE_ACTION_TYPE_VIBRATION_SAMPLE:
        if (_edje_block_break(ed))
          goto break_prog;
        _edje_multisense_internal_vibration_sample_play(ed, pr->vibration_name, pr->vibration_repeat);
        break;

      case EDJE_ACTION_TYPE_PARAM_COPY:
      {
         Edje_Real_Part *src_part, *dst_part;

         // _edje_emit(ed, "program,start", pr->name);
         if (_edje_block_break(ed)) goto break_prog;

         src_part = ed->table_parts[pr->param.src % ed->table_parts_size];
         dst_part = ed->table_parts[pr->param.dst % ed->table_parts_size];
         _edje_param_copy(ed, src_part, pr->state, dst_part, pr->state2);

         if (_edje_block_break(ed)) goto break_prog;
         // _edje_emit(ed, "program,stop", pr->name);
         if (_edje_block_break(ed)) goto break_prog;
      }
      break;

      case EDJE_ACTION_TYPE_PARAM_SET:
      {
         Edje_Real_Part *part;

         // _edje_emit(ed, "program,start", pr->name);
         if (_edje_block_break(ed)) goto break_prog;

         part = ed->table_parts[pr->param.dst % ed->table_parts_size];
         _edje_param_set(ed, part, pr->state, pr->state2);

         if (_edje_block_break(ed)) goto break_prog;
         // _edje_emit(ed, "program,stop", pr->name);
         if (_edje_block_break(ed)) goto break_prog;
      }
      break;

#ifdef HAVE_EPHYSICS
      case EDJE_ACTION_TYPE_PHYSICS_IMPULSE:
        if (EPH_LOAD())
          {
             if (!_edje_physics_action_set(ed, pr, EPH_CALL(ephysics_body_central_impulse_apply)))
               goto break_prog;
          }
        break;

      case EDJE_ACTION_TYPE_PHYSICS_TORQUE_IMPULSE:
        if (EPH_LOAD())
          {
             if (!_edje_physics_action_set(ed, pr, EPH_CALL(ephysics_body_torque_impulse_apply)))
               goto break_prog;
          }
        break;

      case EDJE_ACTION_TYPE_PHYSICS_FORCE:
        if (EPH_LOAD())
          {
             if (!_edje_physics_action_set(ed, pr, EPH_CALL(ephysics_body_central_force_apply)))
               goto break_prog;
          }
        break;

      case EDJE_ACTION_TYPE_PHYSICS_TORQUE:
        if (EPH_LOAD())
          {
             if (!_edje_physics_action_set(ed, pr, EPH_CALL(ephysics_body_torque_apply)))
               goto break_prog;
          }
        break;

      case EDJE_ACTION_TYPE_PHYSICS_FORCES_CLEAR:
        if (EPH_LOAD())
          {
             if (_edje_block_break(ed))
               goto break_prog;
             EINA_LIST_FOREACH(pr->targets, l, pt)
               {
                  if (pt->id >= 0)
                    {
                       rp = ed->table_parts[pt->id % ed->table_parts_size];
                       if ((rp) && (rp->body))
                         EPH_CALL(ephysics_body_forces_clear)(rp->body);
                    }
               }
          }
        break;

      case EDJE_ACTION_TYPE_PHYSICS_VEL_SET:
        if (EPH_LOAD())
          {
             if (!_edje_physics_action_set(ed, pr, EPH_CALL(ephysics_body_linear_velocity_set)))
               goto break_prog;
          }
        break;

      case EDJE_ACTION_TYPE_PHYSICS_ANG_VEL_SET:
        if (EPH_LOAD())
          {
             if (!_edje_physics_action_set(ed, pr, EPH_CALL(ephysics_body_angular_velocity_set)))
               goto break_prog;
          }
        break;

      case EDJE_ACTION_TYPE_PHYSICS_STOP:
        if (EPH_LOAD())
          {
             if (_edje_block_break(ed))
               goto break_prog;
             EINA_LIST_FOREACH(pr->targets, l, pt)
               {
                  if (pt->id >= 0)
                    {
                       rp = ed->table_parts[pt->id % ed->table_parts_size];
                       if ((rp) && (rp->body))
                         EPH_CALL(ephysics_body_stop)(rp->body);
                    }
               }
          }
        break;

      case EDJE_ACTION_TYPE_PHYSICS_ROT_SET:
        if (EPH_LOAD())
          {
             if (_edje_block_break(ed))
               goto break_prog;
             EINA_LIST_FOREACH(pr->targets, l, pt)
               {
                  if (pt->id >= 0)
                    {
                       rp = ed->table_parts[pt->id % ed->table_parts_size];
                       if ((rp) && (rp->body))
                         {
                            EPhysics_Quaternion quat;
                            EPH_CALL(ephysics_quaternion_set)(&quat, pr->physics.x,
                                                              pr->physics.y, pr->physics.z,
                                                              pr->physics.w);
                            EPH_CALL(ephysics_quaternion_normalize)(&quat);
                            EPH_CALL(ephysics_body_rotation_set)(rp->body, &quat);
                         }
                    }
               }
          }
        break;

#endif
      default:
        // _edje_emit(ed, "program,start", pr->name);
        // _edje_emit(ed, "program,stop", pr->name);
        break;
     }
   if (!((pr->action == EDJE_ACTION_TYPE_STATE_SET)
         /* hmm this fucks somethgin up. must look into it later */
         /* && (pr->tween.time > ZERO) && (!ed->no_anim))) */
         ))
     {
        EINA_LIST_FOREACH(pr->after, l, pa)
          {
             if (pa->id >= 0)
               {
                  pr2 = ed->collection->patterns.table_programs[pa->id % ed->collection->patterns.table_programs_size];
                  if (pr2) _edje_program_run(ed, pr2, 0, "", "");
                  if (_edje_block_break(ed)) goto break_prog;
               }
          }
     }
break_prog:
   _edje_util_thaw(ed);
   _edje_unref(ed);
   recursions--;
   if (recursions == 0) recursion_limit = 0;
   _edje_unblock(ed);
}

void
_edje_emit(Edje *ed, const char *sig, const char *src)
{
   _edje_emit_full(ed, sig, src, NULL, NULL);
}

void
_edje_seat_emit(Edje *ed, Efl_Input_Device *dev, const char *sig, const char *src)
{
   Efl_Input_Device *seat;
   char buf[128];

   /* keep sending signals without seat information for legacy compatibility */
   _edje_emit_full(ed, sig, src, NULL, NULL);

   /* send extra signal with ",$SEAT" suffix if the input device originating
    * the signal belongs to a seat */
   if (!dev) return;

   seat = efl_input_device_seat_get(dev);
   if (!seat) return;

   snprintf(buf, sizeof(buf), "seat,%s,%s", _edje_seat_name_get(ed, seat), sig);
   _edje_emit_full(ed, buf, src, NULL, NULL);
}

/* data should either be NULL or a malloc allocated data */
void
_edje_emit_full(Edje *ed, const char *sig, const char *src, void *data, void (*free_func)(void *))
{
   const char *sep;
   Eina_Bool broadcast;

   if (!ed->collection) return;
   if (ed->delete_me) return;

   sep = strchr(sig, EDJE_PART_PATH_SEPARATOR);
   /* If we are not sending the signal to a part of the child, the
    * signal if for ourself
    */
   if (sep)
     {
        const char *newsig;
        char *part;
        unsigned int length;

        /* the signal contains a colon, split the signal into "parts:signal" */
        length = sep - sig + 1;
        part = alloca(length);
        memcpy(part, sig, length - 1);
        part[length - 1] = '\0';

        newsig = sep + 1;

        if (_edje_emit_aliased(ed, part, newsig, src)) return;

        broadcast = _edje_emit_child(ed, NULL, part, newsig, src);
     }
   else
     broadcast = ed->collection->broadcast_signal;

   _edje_emit_send(ed, broadcast, sig, src, data, free_func);
}

void
_edje_focused_part_set(Edje *ed, Eina_Stringshare *seat_name, Edje_Real_Part *rp)
{
   Edje_Seat *seat;
   Eina_List *l;

   EINA_LIST_FOREACH(ed->seats, l, seat)
     {
        if (seat_name == seat->name)
          {
             seat->focused_part = rp;
             return;
          }
     }

   /* A part to be set for a seat not yet announced by Evas */
   seat = calloc(1, sizeof(Edje_Seat));
   EINA_SAFETY_ON_NULL_RETURN(seat);

   seat->name = eina_stringshare_ref(seat_name);
   seat->focused_part = rp;
   ed->seats = eina_list_append(ed->seats, seat);

   return;
}

Edje_Real_Part *
_edje_focused_part_get(Edje *ed, Eina_Stringshare *seat_name)
{
   Edje_Seat *seat;
   Eina_List *l;

   EINA_LIST_FOREACH(ed->seats, l, seat)
     {
        if (seat_name == seat->name)
          return seat->focused_part;
     }

   return NULL;
}

Eina_Stringshare*
_edje_seat_name_get(Edje *ed, Efl_Input_Device *device)
{
   Edje_Seat *seat;
   Eina_List *l;

   EINA_LIST_FOREACH(ed->seats, l, seat)
     {
        if (seat->device == device)
          return seat->name;
     }

   return NULL;
}

Efl_Input_Device *
_edje_seat_get(Edje *ed, Eina_Stringshare *name)
{
   Edje_Seat *seat;
   Eina_List *l;

   EINA_LIST_FOREACH(ed->seats, l, seat)
     {
        if (seat->name == name)
          return seat->device;
     }

   return NULL;
}

struct _Edje_Program_Data
{
   Eina_List  *matches;
   Edje       *ed;
   const char *source;
};

static Eina_Bool
_edje_glob_callback(Edje_Program *pr, void *dt)
{
   struct _Edje_Program_Data *data = dt;
   Edje_Real_Part *rp = NULL;
   Eina_Bool exec = EINA_TRUE;

   if (pr->filter.state)
     {
        rp = _edje_real_part_get(data->ed, pr->filter.part ? pr->filter.part : data->source);
        if (rp)
          exec = !strcmp(rp->chosen_description->state.name, pr->filter.state);
     }

   pr->exec = exec;

   data->matches = eina_list_append(data->matches, pr);

   return EINA_FALSE;
}

/* FIXME: what if we delete the evas object??? */
void
_edje_emit_handle(Edje *ed, const char *sig, const char *src,
                  Edje_Message_Signal_Data *sdata, Eina_Bool prop)
{
   if (ed->delete_me) return;
   if (!sig) sig = "";
   if (!src) src = "";
   DBG("EDJE EMIT: (%p) signal: \"%s\" source: \"%s\"", ed, sig, src);
   _edje_block(ed);
   _edje_ref(ed);
   _edje_util_freeze(ed);

   if (ed->collection && ed->L)
     _edje_lua2_script_func_signal(ed, sig, src);

   if (ed->collection)
     {
#ifdef EDJE_PROGRAM_CACHE
        Edje_Part_Collection *ec;
        char *tmps;
        int l1, l2;
#endif
        int done;

#ifdef EDJE_PROGRAM_CACHE
        ec = ed->collection;
        l1 = strlen(sig);
        l2 = strlen(src);
        tmps = alloca(l1 + l2 + 3); /* \0, \337, \0 */
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

           if (eina_hash_find(ec->prog_cache.no_matches, tmps))
             {
                done = 1;
             }
           else if ((matches = eina_hash_find(ec->prog_cache.matches, tmps)))
             {
                EINA_LIST_FOREACH(matches, l, pr)
                  {
                     Eina_Bool exec = EINA_TRUE;

                     if (pr->filter.state)
                       {
                          Edje_Real_Part *rp;

                          rp = _edje_real_part_get(ed, pr->filter.part ? pr->filter.part : src);
                          if (rp)
                            {
                               if (rp->program)
                                 exec = EINA_FALSE;
                               else
                                 exec = (rp->chosen_description->state.name != pr->filter.state) ?
                                   !strcmp(rp->chosen_description->state.name, pr->filter.state) : EINA_TRUE;
                            }
                       }

                     pr->exec = exec;
                  }

                EINA_LIST_FOREACH(matches, l, pr)
                  if (pr->exec)
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
             struct _Edje_Program_Data data;

             data.ed = ed;
             data.source = src;
             data.matches = NULL;

             if (ed->collection->patterns.table_programs_size > 0)
               {
                  const Eina_Inarray *match;
#ifdef EDJE_PROGRAM_CACHE
                  const Eina_List *l;
#endif
                  Edje_Program *pr;

                  if (ed->collection->patterns.programs.u.programs.globing)
                    if (edje_match_programs_exec(ed->collection->patterns.programs.signals_patterns,
                                                 ed->collection->patterns.programs.sources_patterns,
                                                 sig,
                                                 src,
                                                 ed->collection->patterns.programs.u.programs.globing,
                                                 _edje_glob_callback,
                                                 &data,
                                                 prop) == 0)
                      goto break_prog;

                  match = edje_match_signal_source_hash_get(sig, src,
                                                            ed->collection->patterns.programs.exact_match);
                  if (match)
                    {
                       Edje_Program **tpr;

                       EINA_INARRAY_FOREACH(match, tpr)
                       _edje_glob_callback(*tpr, &data);
                    }

#ifdef EDJE_PROGRAM_CACHE
                  EINA_LIST_FOREACH(data.matches, l, pr)
#else
                  EINA_LIST_FREE(data.matches, pr)
#endif
                    {
                       if (pr->exec)
                         _edje_program_run(ed, pr, 0, sig, src);

                       if (_edje_block_break(ed))
                         {
                            eina_list_free(data.matches);
                            data.matches = NULL;
                            goto break_prog;
                         }
                    }
               }

#ifdef EDJE_PROGRAM_CACHE
             if (tmps)
               {
                  if (data.matches == NULL)
                    {
                       if (!ec->prog_cache.no_matches)
                         ec->prog_cache.no_matches = eina_hash_string_superfast_new(NULL);
                       eina_hash_add(ec->prog_cache.no_matches, tmps, ed);
                    }
                  else
                    {
                       if (!ec->prog_cache.matches)
                         ec->prog_cache.matches = eina_hash_string_superfast_new(NULL);
                       eina_hash_add(ec->prog_cache.matches, tmps, data.matches);
                    }
               }
#endif
          }
        _edje_emit_cb(ed, sig, src, sdata, prop);
        if (_edje_block_break(ed))
          {
             goto break_prog;
          }
     }
break_prog:
   _edje_util_thaw(ed);
   _edje_unref(ed);
   _edje_unblock(ed);
}

/* Extra data for callbacks */
static void *callback_extra_data = NULL;

EAPI void *
edje_object_signal_callback_extra_data_get(void)
{
   return callback_extra_data;
}

/* FIXME: what if we delete the evas object??? */
static void
_edje_emit_cb(Edje *ed, const char *sig, const char *src, Edje_Message_Signal_Data *data, Eina_Bool prop)
{
   const Edje_Signals_Sources_Patterns *ssp;
   Edje_Signal_Callback_Matches *m;
   const Eina_Inarray *match;
   int r = 1;

   if (ed->delete_me) return;
   if (!ed->callbacks || !ed->callbacks->matches) return;

   _edje_ref(ed);
   _edje_util_freeze(ed);
   _edje_block(ed);

   ed->walking_callbacks++;

   ssp = _edje_signal_callback_patterns_ref(ed->callbacks);
   if (ssp)
     {
        m = (Edje_Signal_Callback_Matches *)ed->callbacks->matches;
        EINA_REFCOUNT_REF(m);

        callback_extra_data = (data) ? data->data : NULL;

        if (eina_inarray_count(&ssp->u.callbacks.globing))
          r = edje_match_callback_exec(ssp,
                                       m->matches,
                                       sig,
                                       src,
                                       ed,
                                       prop);

        if (!r) goto break_prog;

        match = edje_match_signal_source_hash_get(sig, src,
                                                  ssp->exact_match);
        if (match)
          {
             const Edje_Signal_Callback_Match *cb;
             unsigned int *i;

             EINA_INARRAY_FOREACH(match, i)
               {
                  if (ed->callbacks->flags[*i].delete_me) continue;
                  if ((prop) && (ed->callbacks->flags[*i].propagate)) continue;

                  cb = &m->matches[*i];

                  cb->func((void *)ed->callbacks->custom_data[*i], ed->obj, sig, src);
                  if (_edje_block_break(ed)) break;
               }
          }

break_prog:
        _edje_signal_callback_matches_unref(m);

        _edje_signal_callback_patterns_unref(ssp);
     }

   ed->walking_callbacks--;

   if (ed->walking_callbacks == 0)
     _edje_signal_callback_reset(ed->callbacks->flags,
                                 ed->callbacks->matches->matches_count);

   _edje_unblock(ed);
   _edje_util_thaw(ed);
   _edje_unref(ed);
}

static const Edje_External_Param_Info *
_edje_external_param_info_get(const Evas_Object *obj, const char *name)
{
   const Edje_External_Type *type;
   const Edje_External_Param_Info *info;

   type = evas_object_data_get(obj, "Edje_External_Type");
   if (!type) return NULL;
   for (info = type->parameters_info; info->name; info++)
     if (!strcmp(info->name, name)) return info;

   return NULL;
}

static Edje_External_Param *
_edje_param_external_get(Edje_Real_Part *rp, const char *name, Edje_External_Param *param)
{
   Evas_Object *swallowed_object;
   const Edje_External_Param_Info *info;

   if ((rp->type != EDJE_RP_TYPE_SWALLOW) ||
       (!rp->typedata.swallow)) return NULL;
   swallowed_object = rp->typedata.swallow->swallowed_object;

   info = _edje_external_param_info_get(swallowed_object, name);
   if (!info) return NULL;

   memset(param, 0, sizeof(*param));
   param->name = info->name;
   param->type = info->type;
   if (!_edje_external_param_get(NULL, rp, param)) return NULL;
   return param;
}

/* simulate external properties for native objects */
static Edje_External_Param *
_edje_param_native_get(Edje *ed, Edje_Real_Part *rp, const char *name, Edje_External_Param *param, void **free_ptr)
{
   *free_ptr = NULL;
   if ((rp->part->type == EDJE_PART_TYPE_TEXT) ||
       (rp->part->type == EDJE_PART_TYPE_TEXTBLOCK))
     {
        if (!strcmp(name, "text"))
          {
             param->name = name;
             param->type = EDJE_EXTERNAL_PARAM_TYPE_STRING;

             _edje_recalc_do(ed);
             if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
               param->s = _edje_entry_text_get(rp);
             else if ((rp->part->type == EDJE_PART_TYPE_TEXT) &&
                      ((rp->type == EDJE_RP_TYPE_TEXT) &&
                       (rp->typedata.text)))
               param->s = rp->typedata.text->text;
             else
               param->s = evas_object_textblock_text_markup_get(rp->object);
             return param;
          }
        if (rp->part->type == EDJE_PART_TYPE_TEXTBLOCK)
          {
             if (!strcmp(name, "text_unescaped"))
               {
                  param->name = name;
                  param->type = EDJE_EXTERNAL_PARAM_TYPE_STRING;

                  _edje_recalc_do(ed);
                  if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
                    {
                       const char *tmp = _edje_entry_text_get(rp);
                       char *unescaped = _edje_text_unescape(tmp);
                       *free_ptr = unescaped;
                       param->s = unescaped;
                    }
                  else if ((rp->part->type == EDJE_PART_TYPE_TEXT) &&
                           ((rp->type == EDJE_RP_TYPE_TEXT) &&
                            (rp->typedata.text)))
                    param->s = rp->typedata.text->text;
                  else
                    {
                       const char *tmp;
                       char *unescaped;

                       tmp = evas_object_textblock_text_markup_get(rp->object);
                       unescaped = _edje_text_unescape(tmp);
                       *free_ptr = unescaped;
                       param->s = unescaped;
                    }

                  return param;
               }

             if (((rp->type == EDJE_RP_TYPE_TEXT) &&
                  (rp->typedata.text)) &&
                 ((rp->typedata.text->entry_data) &&
                  (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE) &&
                  (!strcmp(name, "select_allow"))))
               {
                  param->name = name;
                  param->type = EDJE_EXTERNAL_PARAM_TYPE_BOOL;
                  param->i = _edje_entry_select_allow_get(rp);
                  return param;
               }
          }
     }

   if ((rp->drag) && (rp->drag->down.count == 0))
     {
        if (!strncmp(name, "drag_", sizeof("drag_") - 1))
          {
             const char *sub_name = name + sizeof("drag_") - 1;
             if (!strcmp(sub_name, "value_x"))
               {
                  double d;

                  _edje_recalc_do(ed);
                  d = TO_DOUBLE(rp->drag->val.x);
                  if (rp->part->dragable.x < 0) d = 1.0 - d;
                  param->name = name;
                  param->type = EDJE_EXTERNAL_PARAM_TYPE_DOUBLE;
                  param->d = d;
                  return param;
               }
             if (!strcmp(sub_name, "value_y"))
               {
                  double d;

                  _edje_recalc_do(ed);
                  d = TO_DOUBLE(rp->drag->val.y);
                  if (rp->part->dragable.y < 0) d = 1.0 - d;
                  param->name = name;
                  param->type = EDJE_EXTERNAL_PARAM_TYPE_DOUBLE;
                  param->d = d;
                  return param;
               }

             if (!strcmp(sub_name, "size_w"))
               {
                  _edje_recalc_do(ed);
                  param->name = name;
                  param->type = EDJE_EXTERNAL_PARAM_TYPE_DOUBLE;
                  param->d = TO_DOUBLE(rp->drag->size.x);
                  return param;
               }
             if (!strcmp(sub_name, "size_h"))
               {
                  _edje_recalc_do(ed);
                  param->name = name;
                  param->type = EDJE_EXTERNAL_PARAM_TYPE_DOUBLE;
                  param->d = TO_DOUBLE(rp->drag->size.y);
                  return param;
               }

             if (!strcmp(sub_name, "step_x"))
               {
                  _edje_recalc_do(ed);
                  param->name = name;
                  param->type = EDJE_EXTERNAL_PARAM_TYPE_DOUBLE;
                  param->d = TO_DOUBLE(rp->drag->step.x);
                  return param;
               }
             if (!strcmp(sub_name, "step_y"))
               {
                  _edje_recalc_do(ed);
                  param->name = name;
                  param->type = EDJE_EXTERNAL_PARAM_TYPE_DOUBLE;
                  param->d = TO_DOUBLE(rp->drag->step.y);
                  return param;
               }

             if (!strcmp(sub_name, "page_x"))
               {
                  _edje_recalc_do(ed);
                  param->name = name;
                  param->type = EDJE_EXTERNAL_PARAM_TYPE_DOUBLE;
                  param->d = TO_DOUBLE(rp->drag->page.x);
                  return param;
               }
             if (!strcmp(sub_name, "page_y"))
               {
                  _edje_recalc_do(ed);
                  param->name = name;
                  param->type = EDJE_EXTERNAL_PARAM_TYPE_DOUBLE;
                  param->d = TO_DOUBLE(rp->drag->page.y);
                  return param;
               }

             return NULL;
          }
     }

   return NULL;
}

static Eina_Bool
_edje_param_native_set(Edje *ed, Edje_Real_Part *rp, const char *name, const Edje_External_Param *param)
{
   if ((rp->part->type == EDJE_PART_TYPE_TEXT) ||
       (rp->part->type == EDJE_PART_TYPE_TEXTBLOCK))
     {
        if (!strcmp(name, "text"))
          {
             if (param->type != EDJE_EXTERNAL_PARAM_TYPE_STRING)
               return EINA_FALSE;

             _edje_object_part_text_raw_set(ed, ed->obj, rp, rp->part->name, param->s);
             return EINA_TRUE;
          }
        if (rp->part->type == EDJE_PART_TYPE_TEXTBLOCK)
          {
             if (!strcmp(name, "text_unescaped"))
               {
                  if (param->type != EDJE_EXTERNAL_PARAM_TYPE_STRING)
                    return EINA_FALSE;

                  if (rp->part->type == EDJE_PART_TYPE_TEXT)
                    _edje_object_part_text_raw_set(ed, ed->obj, rp, rp->part->name, param->s);
                  else
                    {
                       char *escaped = _edje_text_escape(param->s);
                       _edje_object_part_text_raw_set(ed, ed->obj, rp, rp->part->name, escaped);
                       free(escaped);
                    }

                  return EINA_TRUE;
               }

             if (((rp->type == EDJE_RP_TYPE_TEXT) &&
                  (rp->typedata.text)) &&
                 ((rp->typedata.text->entry_data) &&
                  (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE) &&
                  (!strcmp(name, "select_allow"))))
               {
                  if (param->type != EDJE_EXTERNAL_PARAM_TYPE_BOOL)
                    return EINA_FALSE;
                  _edje_entry_select_allow_set(rp, param->i);
                  return EINA_TRUE;
               }
          }
     }

   if ((rp->drag) && (rp->drag->down.count == 0))
     {
        if (!strncmp(name, "drag_", sizeof("drag_") - 1))
          {
             const char *sub_name = name + sizeof("drag_") - 1;
             if (!strcmp(sub_name, "value_x"))
               {
                  double d;
                  if (param->type != EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
                    return EINA_FALSE;
                  d = param->d;
                  if (rp->part->dragable.confine_id != -1)
                    d = CLAMP(d, 0.0, 1.0);
                  if (rp->part->dragable.x < 0) d = 1.0 - d;
                  if (EQ(rp->drag->val.x, FROM_DOUBLE(d))) return EINA_TRUE;
                  rp->drag->val.x = FROM_DOUBLE(d);
#ifdef EDJE_CALC_CACHE
                  rp->invalidate = EINA_TRUE;
#endif
                  _edje_dragable_pos_set
                    (ed, rp, rp->drag->val.x, rp->drag->val.y);
                  _edje_emit(ed, "drag,set", rp->part->name);
                  return EINA_TRUE;
               }
             if (!strcmp(sub_name, "value_y"))
               {
                  double d;
                  if (param->type != EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
                    return EINA_FALSE;
                  d = param->d;
                  if (rp->part->dragable.confine_id != -1)
                    d = CLAMP(d, 0.0, 1.0);
                  if (rp->part->dragable.y < 0) d = 1.0 - d;
                  if (EQ(rp->drag->val.y, FROM_DOUBLE(d))) return EINA_TRUE;
                  rp->drag->val.y = FROM_DOUBLE(d);
#ifdef EDJE_CALC_CACHE
                  rp->invalidate = EINA_TRUE;
#endif
                  _edje_dragable_pos_set
                    (ed, rp, rp->drag->val.x, rp->drag->val.y);
                  _edje_emit(ed, "drag,set", rp->part->name);
                  return EINA_TRUE;
               }

             if (!strcmp(sub_name, "size_w"))
               {
                  if (param->type != EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
                    return EINA_FALSE;
                  rp->drag->size.x = FROM_DOUBLE(CLAMP(param->d, 0.0, 1.0));
                  ed->recalc_call = EINA_TRUE;
                  ed->dirty = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
                  rp->invalidate = EINA_TRUE;
#endif
                  _edje_recalc(ed);
                  return EINA_TRUE;
               }
             if (!strcmp(sub_name, "size_h"))
               {
                  if (param->type != EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
                    return EINA_FALSE;
                  rp->drag->size.y = FROM_DOUBLE(CLAMP(param->d, 0.0, 1.0));
                  ed->recalc_call = EINA_TRUE;
                  ed->dirty = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
                  rp->invalidate = EINA_TRUE;
#endif
                  _edje_recalc(ed);
                  return EINA_TRUE;
               }

             if (!strcmp(sub_name, "step_x"))
               {
                  if (param->type != EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
                    return EINA_FALSE;
                  rp->drag->step.x = FROM_DOUBLE(CLAMP(param->d, 0.0, 1.0));
#ifdef EDJE_CALC_CACHE
                  rp->invalidate = EINA_TRUE;
#endif
                  return EINA_TRUE;
               }
             if (!strcmp(sub_name, "step_y"))
               {
                  if (param->type != EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
                    return EINA_FALSE;
                  rp->drag->step.y = FROM_DOUBLE(CLAMP(param->d, 0.0, 1.0));
#ifdef EDJE_CALC_CACHE
                  rp->invalidate = EINA_TRUE;
#endif
                  return EINA_TRUE;
               }

             if (!strcmp(sub_name, "page_x"))
               {
                  if (param->type != EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
                    return EINA_FALSE;
                  rp->drag->page.x = FROM_DOUBLE(CLAMP(param->d, 0.0, 1.0));
#ifdef EDJE_CALC_CACHE
                  rp->invalidate = EINA_TRUE;
#endif
                  return EINA_TRUE;
               }
             if (!strcmp(sub_name, "page_y"))
               {
                  if (param->type != EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
                    return EINA_FALSE;
                  rp->drag->page.y = FROM_DOUBLE(CLAMP(param->d, 0.0, 1.0));
#ifdef EDJE_CALC_CACHE
                  rp->invalidate = EINA_TRUE;
#endif
                  return EINA_TRUE;
               }

             return EINA_FALSE;
          }
     }

   return EINA_FALSE;
}

static const Edje_External_Param_Info *
_edje_native_param_info_get(const Edje_Real_Part *rp, const char *name)
{
   if ((rp->part->type == EDJE_PART_TYPE_TEXT) ||
       (rp->part->type == EDJE_PART_TYPE_TEXTBLOCK))
     {
        if (!strcmp(name, "text"))
          {
             static const Edje_External_Param_Info pi =
               EDJE_EXTERNAL_PARAM_INFO_STRING("text");
             return &pi;
          }
        if (rp->part->type == EDJE_PART_TYPE_TEXTBLOCK)
          {
             if (!strcmp(name, "text_unescaped"))
               {
                  static const Edje_External_Param_Info pi =
                    EDJE_EXTERNAL_PARAM_INFO_STRING("text_unescaped");
                  return &pi;
               }
             if (!strcmp(name, "select_allow"))
               {
                  static const Edje_External_Param_Info pi =
                    EDJE_EXTERNAL_PARAM_INFO_BOOL("text_unescaped");
                  return &pi;
               }
          }
     }

   if ((rp->drag) && (rp->drag->down.count == 0))
     {
        if (!strncmp(name, "drag_", sizeof("drag_") - 1))
          {
             name += sizeof("drag_") - 1;
             if (!strcmp(name, "value_x"))
               {
                  static const Edje_External_Param_Info pi =
                    EDJE_EXTERNAL_PARAM_INFO_DOUBLE("drag_value_x");
                  return &pi;
               }
             if (!strcmp(name, "value_y"))
               {
                  static const Edje_External_Param_Info pi =
                    EDJE_EXTERNAL_PARAM_INFO_DOUBLE("drag_value_y");
                  return &pi;
               }
             if (!strcmp(name, "size_w"))
               {
                  static const Edje_External_Param_Info pi =
                    EDJE_EXTERNAL_PARAM_INFO_DOUBLE("drag_size_w");
                  return &pi;
               }
             if (!strcmp(name, "size_h"))
               {
                  static const Edje_External_Param_Info pi =
                    EDJE_EXTERNAL_PARAM_INFO_DOUBLE("drag_size_h");
                  return &pi;
               }
             if (!strcmp(name, "step_x"))
               {
                  static const Edje_External_Param_Info pi =
                    EDJE_EXTERNAL_PARAM_INFO_DOUBLE("drag_step_x");
                  return &pi;
               }
             if (!strcmp(name, "step_y"))
               {
                  static const Edje_External_Param_Info pi =
                    EDJE_EXTERNAL_PARAM_INFO_DOUBLE("drag_step_y");
                  return &pi;
               }
             if (!strcmp(name, "page_x"))
               {
                  static const Edje_External_Param_Info pi =
                    EDJE_EXTERNAL_PARAM_INFO_DOUBLE("drag_page_x");
                  return &pi;
               }
             if (!strcmp(name, "page_y"))
               {
                  static const Edje_External_Param_Info pi =
                    EDJE_EXTERNAL_PARAM_INFO_DOUBLE("drag_page_y");
                  return &pi;
               }

             return NULL;
          }
     }

   return NULL;
}

static Edje_External_Param *
_edje_param_convert(Edje_External_Param *param, const Edje_External_Param_Info *dst_info)
{
   if (param->type == dst_info->type) return param;

   switch (dst_info->type)
     {
      case EDJE_EXTERNAL_PARAM_TYPE_BOOL:
      case EDJE_EXTERNAL_PARAM_TYPE_INT:
      {
         int i;
         switch (param->type)
           {
            case EDJE_EXTERNAL_PARAM_TYPE_DOUBLE:
              i = (int)param->d;
              break;

            case EDJE_EXTERNAL_PARAM_TYPE_STRING:
            case EDJE_EXTERNAL_PARAM_TYPE_CHOICE:
              i = (param->s) ? atoi(param->s) : 0;
              break;

            case EDJE_EXTERNAL_PARAM_TYPE_BOOL:
            case EDJE_EXTERNAL_PARAM_TYPE_INT:
              i = param->i;
              break;

            default:
              return NULL;
           }
         if (dst_info->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
           i = !!i;
         param->type = dst_info->type;
         param->i = i;
         return param;
      }

      case EDJE_EXTERNAL_PARAM_TYPE_DOUBLE:
      {
         double d;
         switch (param->type)
           {
            case EDJE_EXTERNAL_PARAM_TYPE_INT:
              d = (double)param->i;
              break;

            case EDJE_EXTERNAL_PARAM_TYPE_STRING:
            case EDJE_EXTERNAL_PARAM_TYPE_CHOICE:
              d = (param->s) ? atof(param->s) : 0.0;
              break;

            case EDJE_EXTERNAL_PARAM_TYPE_BOOL:
              d = (double)param->i;
              break;

            default:
              return NULL;
           }
         param->type = dst_info->type;
         param->d = d;
         return param;
      }

      case EDJE_EXTERNAL_PARAM_TYPE_STRING:
      {
         static char s[64];
         switch (param->type)
           {
            case EDJE_EXTERNAL_PARAM_TYPE_BOOL:
            case EDJE_EXTERNAL_PARAM_TYPE_INT:
              if (!snprintf(s, sizeof(s), "%i", param->i)) return NULL;
              break;

            case EDJE_EXTERNAL_PARAM_TYPE_DOUBLE:
              if (!snprintf(s, sizeof(s), "%f", param->d)) return NULL;
              break;

            case EDJE_EXTERNAL_PARAM_TYPE_CHOICE:
              param->type = dst_info->type;
              return param;

            default:
              return NULL;
           }
         param->type = dst_info->type;
         param->s = s;
         return param;
      }

      case EDJE_EXTERNAL_PARAM_TYPE_CHOICE:
      {
         static char s[64];
         const char *val;
         switch (param->type)
           {
            case EDJE_EXTERNAL_PARAM_TYPE_BOOL:
            case EDJE_EXTERNAL_PARAM_TYPE_INT:
              if (!snprintf(s, sizeof(s), "%i", param->i)) return NULL;
              val = s;
              break;

            case EDJE_EXTERNAL_PARAM_TYPE_DOUBLE:
              if (!snprintf(s, sizeof(s), "%f", param->d)) return NULL;
              val = s;
              break;

            case EDJE_EXTERNAL_PARAM_TYPE_STRING:
              val = param->s;
              break;

            default:
              return NULL;
           }

         param->type = dst_info->type;
         if (param->s != val) param->s = val;
         return param;
      }

      default: return NULL;
     }
}

static Eina_Bool
_edje_param_validate(const Edje_External_Param *param, const Edje_External_Param_Info *info)
{
   switch (info->type)
     {
      case EDJE_EXTERNAL_PARAM_TYPE_BOOL:
        return (param->i == 0) || (param->i == 1);

      case EDJE_EXTERNAL_PARAM_TYPE_INT:
        if ((info->info.i.min != EDJE_EXTERNAL_INT_UNSET) &&
            (info->info.i.min > param->i))
          return EINA_FALSE;

        if ((info->info.i.max != EDJE_EXTERNAL_INT_UNSET) &&
            (info->info.i.max < param->i))
          return EINA_FALSE;

        return EINA_TRUE;

      case EDJE_EXTERNAL_PARAM_TYPE_DOUBLE:
        if (!EINA_DBL_EQ(info->info.d.min, EDJE_EXTERNAL_DOUBLE_UNSET) &&
            (info->info.d.min > param->d))
          return EINA_FALSE;

        if (!EINA_DBL_EQ(info->info.d.max, EDJE_EXTERNAL_DOUBLE_UNSET) &&
            (info->info.d.max < param->d))
          return EINA_FALSE;

        return EINA_TRUE;

      case EDJE_EXTERNAL_PARAM_TYPE_STRING:
        if (!param->s) return EINA_FALSE;
        if (info->info.s.accept_fmt)
          INF("string 'accept_fmt' validation not implemented.");
        if (info->info.s.deny_fmt)
          INF("string 'deny_fmt' validation not implemented.");
        return EINA_TRUE;

      case EDJE_EXTERNAL_PARAM_TYPE_CHOICE:
      {
         const char **itr = info->info.c.choices;
         if (!itr) return EINA_FALSE;
         for (; *itr; itr++)
           if (!strcmp(*itr, param->s))
             return EINA_TRUE;
         return EINA_FALSE;
      }

      default: return EINA_FALSE;
     }
}

static void
_edje_param_copy(Edje *ed, Edje_Real_Part *src_part, const char *src_param, Edje_Real_Part *dst_part, const char *dst_param)
{
   Edje_External_Param val;
   const Edje_External_Param_Info *dst_info;
   void *free_ptr = NULL;

   if ((!src_part) || (!src_param) || (!dst_part) || (!dst_param))
     return;

   if ((dst_part->part->type == EDJE_PART_TYPE_EXTERNAL) &&
       (dst_part->type == EDJE_RP_TYPE_SWALLOW) &&
       (dst_part->typedata.swallow))
     dst_info = _edje_external_param_info_get
         (dst_part->typedata.swallow->swallowed_object, dst_param);
   else
     dst_info = _edje_native_param_info_get(dst_part, dst_param);

   if (!dst_info)
     {
        ERR("cannot copy, invalid destination parameter '%s' of part '%s'",
            dst_param, dst_part->part->name);
        return;
     }

   if (src_part->part->type == EDJE_PART_TYPE_EXTERNAL)
     {
        if (!_edje_param_external_get
              (src_part, src_param, &val))
          {
             ERR("cannot get parameter '%s' of part '%s'",
                 src_param, src_part->part->name);
             return;
          }
     }
   else
     {
        if (!_edje_param_native_get(ed, src_part, src_param, &val, &free_ptr))
          {
             ERR("cannot get parameter '%s' of part '%s'",
                 src_param, src_part->part->name);
             return;
          }
     }

   if (!_edje_param_convert(&val, dst_info))
     {
        ERR("cannot convert parameter type %s to requested type %s",
            edje_external_param_type_str(val.type),
            edje_external_param_type_str(dst_info->type));
        goto end;
     }

   if (!_edje_param_validate(&val, dst_info))
     {
        ERR("incorrect parameter value failed validation for type %s",
            edje_external_param_type_str(dst_info->type));
        goto end;
     }

   if (dst_part->part->type == EDJE_PART_TYPE_EXTERNAL)
     {
        val.name = dst_param;
        if (!_edje_external_param_set(NULL, dst_part, &val))
          {
             ERR("failed to set parameter '%s' (%s) of part '%s'",
                 dst_param, edje_external_param_type_str(dst_info->type),
                 dst_part->part->name);
             goto end;
          }
     }
   else
     {
        if (!_edje_param_native_set(ed, dst_part, dst_param, &val))
          {
             ERR("failed to set parameter '%s' (%s) of part '%s'",
                 dst_param, edje_external_param_type_str(dst_info->type),
                 dst_part->part->name);
             goto end;
          }
     }

end:
   free(free_ptr);
}

static void
_edje_param_set(Edje *ed, Edje_Real_Part *part, const char *param, const char *value)
{
   Edje_External_Param val;
   const Edje_External_Param_Info *info;

   if ((!part) || (!param) || (!value))
     return;

   if ((part->part->type == EDJE_PART_TYPE_EXTERNAL) &&
       (part->type == EDJE_RP_TYPE_SWALLOW) &&
       (part->typedata.swallow))
     info = _edje_external_param_info_get(part->typedata.swallow->swallowed_object, param);
   else
     info = _edje_native_param_info_get(part, param);

   if (!info)
     {
        ERR("cannot copy, invalid destination parameter '%s' of part '%s'",
            param, part->part->name);
        return;
     }

   val.name = "(temp)";
   val.type = EDJE_EXTERNAL_PARAM_TYPE_STRING;
   val.s = value;

   if (!_edje_param_convert(&val, info))
     {
        ERR("cannot convert parameter type STRING to requested type %s",
            edje_external_param_type_str(info->type));
        return;
     }

   if (!_edje_param_validate(&val, info))
     {
        ERR("incorrect parameter value failed validation for type %s",
            edje_external_param_type_str(info->type));
        return;
     }

   if (part->part->type == EDJE_PART_TYPE_EXTERNAL)
     {
        val.name = param;
        if (!_edje_external_param_set(NULL, part, &val))
          {
             ERR("failed to set parameter '%s' (%s) of part '%s'",
                 param, edje_external_param_type_str(info->type),
                 part->part->name);
             return;
          }
     }
   else
     {
        if (!_edje_param_native_set(ed, part, param, &val))
          {
             ERR("failed to set parameter '%s' (%s) of part '%s'",
                 param, edje_external_param_type_str(info->type),
                 part->part->name);
             return;
          }
     }
}

