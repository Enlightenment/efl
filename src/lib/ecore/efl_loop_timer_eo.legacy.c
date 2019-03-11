
EAPI void
ecore_timer_interval_set(Efl_Loop_Timer *obj, double in)
{
   efl_loop_timer_interval_set(obj, in);
}

EAPI double
ecore_timer_interval_get(const Efl_Loop_Timer *obj)
{
   return efl_loop_timer_interval_get(obj);
}

EAPI double
ecore_timer_pending_get(const Efl_Loop_Timer *obj)
{
   return efl_loop_timer_pending_get(obj);
}

EAPI void
ecore_timer_reset(Efl_Loop_Timer *obj)
{
   efl_loop_timer_reset(obj);
}

EAPI void
ecore_timer_loop_reset(Efl_Loop_Timer *obj)
{
   efl_loop_timer_loop_reset(obj);
}

EAPI void
ecore_timer_delay(Efl_Loop_Timer *obj, double add)
{
   efl_loop_timer_delay(obj, add);
}
