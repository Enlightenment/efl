/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

/*
 * XSync code
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"

EAPI Ecore_X_Sync_Alarm
ecore_x_sync_alarm_new(Ecore_X_Sync_Counter counter)
{
   Ecore_X_Sync_Alarm alarm;
   XSyncAlarmAttributes values;
   XSyncValue init;

   XSyncIntToValue(&init, 0);
   XSyncSetCounter(_ecore_x_disp, counter, init);
   
   values.trigger.counter = counter;
   values.trigger.value_type = XSyncAbsolute;
   XSyncIntToValue(&values.trigger.wait_value, 1);
   values.trigger.test_type = XSyncPositiveComparison;

   XSyncIntToValue(&values.delta, 1);

   values.events = True;

   alarm = XSyncCreateAlarm(_ecore_x_disp,
			    XSyncCACounter |
			    XSyncCAValueType |
			    XSyncCAValue |
			    XSyncCATestType |
			    XSyncCADelta |
			    XSyncCAEvents,
			    &values);

   ecore_x_sync();
   return alarm;
}

EAPI int
ecore_x_sync_alarm_free(Ecore_X_Sync_Alarm alarm)
{
   return XSyncDestroyAlarm(_ecore_x_disp, alarm);
}
