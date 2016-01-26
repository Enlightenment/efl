#include "edje_private.h"

static Eina_Bool
_edje_hold_signal_cb(void *data, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   Evas_Event_Hold *ev;
   Edje *ed;
   Edje_Real_Part *rp;

   ev = event_info;
   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if (!rp) return EO_CALLBACK_CONTINUE;
   if (ev->hold)
     _edje_emit(ed, "hold,on", rp->part->name);
   else
     _edje_emit(ed, "hold,off", rp->part->name);

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_edje_focus_in_signal_cb(void *data, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if ((!rp) || (!ed))
     return EO_CALLBACK_CONTINUE;

   _edje_emit(ed, "focus,part,in", rp->part->name);

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_edje_focus_out_signal_cb(void *data, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if ((!rp) || (!ed))
     return EO_CALLBACK_CONTINUE;

   _edje_emit(ed, "focus,part,out", rp->part->name);

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_edje_mouse_in_signal_cb(void *data, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_In *ev;
   Edje *ed;
   Edje_Real_Part *rp;

   ev = event_info;
   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if (rp)
     {
        if (!(ev->event_flags) || !(rp->part->ignore_flags & ev->event_flags))
          _edje_emit(ed, "mouse,in", rp->part->name);

        ev->event_flags |= rp->part->mask_flags;
     }

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_edje_mouse_out_signal_cb(void *data, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Out *ev;
   Edje *ed;
   Edje_Real_Part *rp;

   ev = event_info;
   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if (rp)
     {
        if (!(ev->event_flags) || !(rp->part->ignore_flags & ev->event_flags))
          _edje_emit(ed, "mouse,out", rp->part->name);

        ev->event_flags |= rp->part->mask_flags;
     }

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_edje_mouse_down_signal_cb(void *data, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   Edje *ed;
   Edje_Real_Part *rp;
   char buf[256];
   int ignored;

   ev = event_info;
   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if (!rp) return EO_CALLBACK_CONTINUE;

   ignored = rp->part->ignore_flags & ev->event_flags;

   _edje_ref(ed);
   _edje_util_freeze(ed);

   if ((!ev->event_flags) || (!ignored))
     {
        if (ev->flags & EVAS_BUTTON_TRIPLE_CLICK)
          snprintf(buf, sizeof(buf), "mouse,down,%i,triple", ev->button);
        else if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
          snprintf(buf, sizeof(buf), "mouse,down,%i,double", ev->button);
        else
          snprintf(buf, sizeof(buf), "mouse,down,%i", ev->button);
        _edje_emit(ed, buf, rp->part->name);
     }

   if (rp->part->dragable.event_id >= 0)
     {
        rp = ed->table_parts[rp->part->dragable.event_id % ed->table_parts_size];
        if (!ignored)
          {
             snprintf(buf, sizeof(buf), "mouse,down,%i", ev->button);
             _edje_emit(ed, buf, rp->part->name);
          }
     }

   if (rp->drag)
     {
        if (rp->drag->down.count == 0)
          {
             if (rp->part->dragable.x)
               rp->drag->down.x = ev->canvas.x;
             if (rp->part->dragable.y)
               rp->drag->down.y = ev->canvas.y;
             rp->drag->threshold_x = EINA_FALSE;
             rp->drag->threshold_y = EINA_FALSE;
             rp->drag->threshold_started_x = EINA_TRUE;
             rp->drag->threshold_started_y = EINA_TRUE;
          }
        rp->drag->down.count++;
     }

   if (rp->clicked_button == 0)
     {
        rp->clicked_button = ev->button;
        if (!(ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD))
          rp->still_in = EINA_TRUE;
     }
//   _edje_recalc_do(ed);
   _edje_util_thaw(ed);
   _edje_unref(ed);

   ev->event_flags |= rp->part->mask_flags;

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_edje_mouse_up_signal_cb(void *data, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Up *ev;
   Edje *ed;
   Edje_Real_Part *rp;
   char buf[256];
   int ignored;

   ev = event_info;
   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if (!rp) return EO_CALLBACK_CONTINUE;

   ignored = rp->part->ignore_flags & ev->event_flags;

   _edje_ref(ed);
   _edje_util_freeze(ed);

   if ((!ev->event_flags) || (!ignored))
     {
        snprintf(buf, sizeof(buf), "mouse,up,%i", ev->button);
        _edje_emit(ed, buf, rp->part->name);
     }

   if (rp->part->dragable.event_id >= 0)
     {
        rp = ed->table_parts[rp->part->dragable.event_id % ed->table_parts_size];
        if (!ignored)
          {
             snprintf(buf, sizeof(buf), "mouse,up,%i", ev->button);
             _edje_emit(ed, buf, rp->part->name);
          }
     }

   if (rp->drag)
     {
        if (rp->drag->down.count > 0)
          {
             rp->drag->down.count--;
             if (rp->drag->down.count == 0)
               {
                  rp->drag->threshold_started_x = EINA_FALSE;
                  rp->drag->threshold_started_y = EINA_FALSE;
                  rp->drag->need_reset = 1;
                  ed->recalc_call = EINA_TRUE;
                  ed->dirty = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
                  rp->invalidate = EINA_TRUE;
#endif
                  if (!ignored && rp->drag->started)
                    _edje_emit(ed, "drag,stop", rp->part->name);
                  rp->drag->started = EINA_FALSE;
                  _edje_recalc_do(ed);
               }
          }
     }

   if ((rp->still_in) && (rp->clicked_button == ev->button) && (!ignored))
     {
        snprintf(buf, sizeof(buf), "mouse,clicked,%i", ev->button);
        _edje_emit(ed, buf, rp->part->name);
     }
   rp->clicked_button = 0;
   rp->still_in = EINA_FALSE;

//   _edje_recalc_do(ed);
   _edje_util_thaw(ed);
   _edje_unref(ed);

   ev->event_flags |= rp->part->mask_flags;

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_edje_mouse_move_signal_cb(void *data, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Move *ev;
   Edje *ed;
   Edje_Real_Part *rp;
   int ignored;

   ev = event_info;
   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if (!rp) return EO_CALLBACK_CONTINUE;
   if (rp->part->dragable.event_id >= 0)
     {
        rp = ed->table_parts[rp->part->dragable.event_id % ed->table_parts_size];
     }

   ignored = rp->part->ignore_flags & ev->event_flags;

   _edje_ref(ed);
   if ((!ev->event_flags) || (!ignored))
     _edje_emit(ed, "mouse,move", rp->part->name);

   if (rp->still_in)
     {
        if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
          rp->still_in = EINA_FALSE;
        else
          {
             Evas_Coord x, y, w, h;

             evas_object_geometry_get(obj, &x, &y, &w, &h);
             if ((ev->cur.canvas.x < x) || (ev->cur.canvas.y < y) ||
                 (ev->cur.canvas.x >= (x + w)) || (ev->cur.canvas.y >= (y + h)))
               {
                  if ((ev->buttons) && ((!ev->event_flags) || (!ignored)))
                    _edje_emit(ed, "mouse,pressed,out", rp->part->name);

                  rp->still_in = EINA_FALSE;
               }
          }
     }
   else
     {
        if (!(ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD))
          {
             Evas_Coord x, y, w, h;

             evas_object_geometry_get(obj, &x, &y, &w, &h);
             if ((ev->cur.canvas.x >= x) && (ev->cur.canvas.y >= y) &&
                 (ev->cur.canvas.x < (x + w)) && (ev->cur.canvas.y < (y + h)))
               {
                  if ((ev->buttons) && ((!ev->event_flags) || (!ignored)))
                    _edje_emit(ed, "mouse,pressed,in", rp->part->name);

                  rp->still_in = EINA_TRUE;
               }
          }
     }
   _edje_util_freeze(ed);
   if (rp->drag)
     {
        if (rp->drag->down.count > 0)
          {
             if (rp->part->dragable.x)
               rp->drag->tmp.x = ev->cur.canvas.x - rp->drag->down.x;
             if (rp->part->dragable.y)
               rp->drag->tmp.y = ev->cur.canvas.y - rp->drag->down.y;
             ed->recalc_call = EINA_TRUE;
             ed->dirty = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
             rp->invalidate = EINA_TRUE;
#endif
          }
        _edje_recalc_do(ed);

        if (rp->drag->down.count > 0)
          {
             FLOAT_T dx, dy;

             _edje_part_dragable_calc(ed, rp, &dx, &dy);
             if ((dx != rp->drag->val.x) || (dy != rp->drag->val.y))
               {
                  rp->drag->val.x = dx;
                  rp->drag->val.y = dy;
                  if (!ignored)
                    {
                       if (!rp->drag->started)
                         _edje_emit(ed, "drag,start", rp->part->name);
                       _edje_emit(ed, "drag", rp->part->name);
                       rp->drag->started = EINA_TRUE;
                    }
                  ed->recalc_call = EINA_TRUE;
                  ed->dirty = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
                  rp->invalidate = EINA_TRUE;
#endif
                  _edje_recalc_do(ed);
               }
          }
     }
   _edje_unref(ed);
   _edje_util_thaw(ed);

   ev->event_flags |= rp->part->mask_flags;

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_edje_mouse_wheel_signal_cb(void *data, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Wheel *ev;
   Edje *ed;
   Edje_Real_Part *rp;
   char buf[256];

   ev = event_info;
   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if (rp)
     {
        if (!(ev->event_flags) || !(rp->part->ignore_flags & ev->event_flags))
          {
             snprintf(buf, sizeof(buf), "mouse,wheel,%i,%i", ev->direction, (ev->z < 0) ? (-1) : (1));
             _edje_emit(ed, buf, rp->part->name);
          }

        ev->event_flags |= rp->part->mask_flags;
     }

   return EO_CALLBACK_CONTINUE;
}

Eina_Bool
_edje_timer_cb(void *data,
               Eo *obj EINA_UNUSED,
               const Eo_Event_Description *desc EINA_UNUSED,
               void *event_info EINA_UNUSED) // FIXME: figure out how to use event_info
{
   double t;
   Eina_List *l;
   Eina_List *newl = NULL;
   Edje *ed = data;

   t = ecore_loop_time_get();
   _edje_ref(ed);

   _edje_block(ed);
   _edje_util_freeze(ed);
   if ((!ed->paused) && (!ed->delete_me))
     {
        const void *tmp;

        ed->walking_actions = EINA_TRUE;
        EINA_LIST_FOREACH(ed->actions, l, tmp)
          newl = eina_list_append(newl, tmp);
        while (newl)
          {
             Edje_Running_Program *runp;

             runp = eina_list_data_get(newl);
             newl = eina_list_remove(newl, eina_list_data_get(newl));
             if (!runp->delete_me)
               _edje_program_run_iterate(runp, t);
             if (_edje_block_break(ed))
               {
                  eina_list_free(newl);
                  newl = NULL;
                  goto break_prog;
               }
          }
        EINA_LIST_FOREACH(ed->actions, l, tmp)
          newl = eina_list_append(newl, tmp);
        while (newl)
          {
             Edje_Running_Program *runp;

             runp = eina_list_data_get(newl);
             newl = eina_list_remove(newl, eina_list_data_get(newl));
             if (runp->delete_me)
               {
                  _edje_program_run_cleanup(ed, runp);
                  free(runp);
               }
          }
        ed->walking_actions = EINA_FALSE;
     }
break_prog:
   _edje_unblock(ed);
   _edje_util_thaw(ed);
   _edje_unref(ed);

   return EO_CALLBACK_CONTINUE;
}

Eina_Bool
_edje_pending_timer_cb(void *data)
{
   Edje_Pending_Program *pp;

   pp = data;
   pp->edje->pending_actions = eina_list_remove(pp->edje->pending_actions, pp);
   _edje_program_run(pp->edje, pp->program, 1, "", "");
   free(pp);
   return ECORE_CALLBACK_CANCEL;
}

EO_CALLBACKS_ARRAY_DEFINE(edje_callbacks,
                          { EVAS_OBJECT_EVENT_HOLD, _edje_hold_signal_cb },
                          { EVAS_OBJECT_EVENT_MOUSE_IN, _edje_mouse_in_signal_cb },
                          { EVAS_OBJECT_EVENT_MOUSE_OUT, _edje_mouse_out_signal_cb },
                          { EVAS_OBJECT_EVENT_MOUSE_DOWN, _edje_mouse_down_signal_cb },
                          { EVAS_OBJECT_EVENT_MOUSE_UP, _edje_mouse_up_signal_cb },
                          { EVAS_OBJECT_EVENT_MOUSE_MOVE, _edje_mouse_move_signal_cb },
                          { EVAS_OBJECT_EVENT_MOUSE_WHEEL, _edje_mouse_wheel_signal_cb });

EO_CALLBACKS_ARRAY_DEFINE(edje_focus_callbacks,
                          { EVAS_OBJECT_EVENT_FOCUS_IN, _edje_focus_in_signal_cb },
                          { EVAS_OBJECT_EVENT_FOCUS_OUT, _edje_focus_out_signal_cb });

void
_edje_callbacks_add(Evas_Object *obj, Edje *ed, Edje_Real_Part *rp)
{
   eo_do(obj, eo_event_callback_array_add(edje_callbacks(), ed));
   evas_object_data_set(obj, "real_part", rp);
}

void
_edje_callbacks_del(Evas_Object *obj, Edje *ed)
{
   eo_do(obj, eo_event_callback_array_del(edje_callbacks(), ed));
   evas_object_data_del(obj, "real_part");
}

void
_edje_callbacks_focus_add(Evas_Object *obj, Edje *ed, Edje_Real_Part *rp)
{
   eo_do(obj, eo_event_callback_array_add(edje_focus_callbacks(), ed));
   evas_object_data_set(obj, "real_part", rp);
}

void
_edje_callbacks_focus_del(Evas_Object *obj, Edje *ed)
{
   eo_do(obj, eo_event_callback_array_del(edje_focus_callbacks(), ed));
   evas_object_data_del(obj, "real_part");
}

