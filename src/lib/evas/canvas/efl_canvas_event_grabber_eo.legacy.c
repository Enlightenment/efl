
EVAS_API void
evas_object_event_grabber_freeze_when_visible_set(Efl_Canvas_Event_Grabber *obj, Eina_Bool set)
{
   efl_canvas_event_grabber_freeze_when_visible_set(obj, set);
}

EVAS_API Eina_Bool
evas_object_event_grabber_freeze_when_visible_get(const Efl_Canvas_Event_Grabber *obj)
{
   return efl_canvas_event_grabber_freeze_when_visible_get(obj);
}
