#include "ecore_xcb_private.h"

/**
 * @defgroup Ecore_X_Sync_Group X Sync Extension Functions
 *
 * Functions related to the X Sync extension.
 */

#ifdef ECORE_XCB_SYNC
static int _sync_available = 0;
static xcb_sync_initialize_cookie_t _ecore_xcb_sync_init_cookie;
#endif /* ECORE_XCB_SYNC */

/* To avoid round trips, the initialization is separated in 2
   functions: _ecore_xcb_sync_init and
   _ecore_xcb_sync_init_finalize. The first one gets the cookies and
   the second one gets the replies and set the atoms. */

void
_ecore_x_sync_init(const xcb_query_extension_reply_t *reply)
{
#ifdef ECORE_XCB_SYNC
   if (reply && (reply->present))
      _ecore_xcb_sync_init_cookie = xcb_sync_initialize_unchecked(_ecore_xcb_conn,
                                                                  XCB_SYNC_MAJOR_VERSION,
                                                                  XCB_SYNC_MINOR_VERSION);

#endif /* ECORE_XCB_SYNC */
} /* _ecore_x_sync_init */

void
_ecore_x_sync_init_finalize(void)
{
#ifdef ECORE_XCB_SYNC
   xcb_sync_initialize_reply_t *reply;

   reply = xcb_sync_initialize_reply(_ecore_xcb_conn,
                                     _ecore_xcb_sync_init_cookie, NULL);

   if (reply)
     {
        if (reply->major_version >= 3)
           _sync_available = 1;

        free(reply);
     }

#endif /* ECORE_XCB_SYNC */
} /* _ecore_x_sync_init_finalize */

/**
 * Return whether the X server supports the Sync Extension.
 * @return 1 if the X Sync Extension is available, 0 otherwise.
 *
 * Return 1 if the X server supports the Sync Extension version 3.0,
 * 0 otherwise.
 * @ingroup Ecore_X_Sync_Group
 */
EAPI Eina_Bool
ecore_x_sync_query(void)
{
#ifdef ECORE_XCB_SYNC
   return _sync_available;
#else /* ifdef ECORE_XCB_SYNC */
   return 0;
#endif /* ECORE_XCB_SYNC */
} /* ecore_x_sync_query */

/**
 * Create a new alarm.
 * @param counter A counter.
 * @return        A newly created alarm.
 *
 * Create a new alarm.
 * @ingroup Ecore_X_Sync_Group
 */
EAPI Ecore_X_Sync_Alarm
ecore_x_sync_alarm_new(Ecore_X_Sync_Counter counter)
{
#ifdef ECORE_XCB_SYNC
   uint32_t value_list[6];
   xcb_sync_int64_t init;
   Ecore_X_Sync_Alarm alarm;
   uint32_t value_mask;

   init.lo = 0;
   init.hi = 0;
   xcb_sync_set_counter(_ecore_xcb_conn, counter, init);

   value_mask =
      XCB_SYNC_CA_COUNTER | XCB_SYNC_CA_VALUE_TYPE |
      XCB_SYNC_CA_VALUE | XCB_SYNC_CA_TEST_TYPE |
      XCB_SYNC_CA_DELTA | XCB_SYNC_CA_EVENTS;
   value_list[0] = counter;
   value_list[1] = XCB_SYNC_VALUETYPE_ABSOLUTE;
   value_list[2] = 1;
   value_list[3] = XCB_SYNC_TESTTYPE_POSITIVE_COMPARISON;
   value_list[4] = 1;
   value_list[5] = 1;
   alarm = xcb_generate_id(_ecore_xcb_conn);
   xcb_sync_create_alarm(_ecore_xcb_conn,
                         alarm,
                         value_mask,
                         (const uint32_t *)value_list);

   ecore_x_sync();
   return alarm;
#else /* ifdef ECORE_XCB_SYNC */
   return 0;
#endif /* ECORE_XCB_SYNC */
} /* ecore_x_sync_alarm_new */

/**
 * Delete an alarm.
 * @param alarm The alarm to delete.
 * @return      1 on success, 0 otherwise.
 *
 * Delete the @p alarm. Returns 1 on success, 0 otherwise.
 * @ingroup Ecore_X_Sync_Group
 */
EAPI Eina_Bool
ecore_x_sync_alarm_free(Ecore_X_Sync_Alarm alarm)
{
#ifdef ECORE_XCB_SYNC
   xcb_sync_destroy_alarm(_ecore_xcb_conn, alarm);
   return 1;
#else /* ifdef ECORE_XCB_SYNC */
   return 0;
#endif /* ECORE_XCB_SYNC */
} /* ecore_x_sync_alarm_free */

/* FIXME: round trip */

EAPI Eina_Bool
ecore_x_sync_counter_query(Ecore_X_Sync_Counter counter, unsigned int *val)
{
#ifdef ECORE_XCB_SYNC
   xcb_sync_query_counter_cookie_t cookie;
   xcb_sync_query_counter_reply_t *reply;

   cookie = xcb_sync_query_counter_unchecked(_ecore_xcb_conn, counter);
   reply = xcb_sync_query_counter_reply(_ecore_xcb_conn, cookie, NULL);
   if (reply)
     {
        *val = (unsigned int)reply->counter_value.lo;
        free(reply);
        return 1;
     }

#endif /* ECORE_XCB_SYNC */

   return 0;
} /* ecore_x_sync_counter_query */

EAPI Ecore_X_Sync_Counter
ecore_x_sync_counter_new(int val)
{
#ifdef ECORE_XCB_SYNC
   xcb_sync_counter_t counter;
   xcb_sync_int64_t v;

   counter = xcb_generate_id(_ecore_xcb_conn);
   v.hi = (val < 0) ? ~0 : 0;
   v.lo = val;
   xcb_sync_create_counter(_ecore_xcb_conn, counter, v);
   return counter;
#else  /* ! ECORE_XCB_SYNC */
   return 0;
#endif /* ! ECORE_XCB_SYNC */
} /* ecore_x_sync_counter_new */

EAPI void
ecore_x_sync_counter_free(Ecore_X_Sync_Counter counter)
{
#ifdef ECORE_XCB_SYNC
   xcb_sync_destroy_counter(_ecore_xcb_conn, counter);
#endif /* ECORE_XCB_SYNC */
} /* ecore_x_sync_counter_free */

EAPI void
ecore_x_sync_counter_inc(Ecore_X_Sync_Counter counter, int by)
{
#ifdef ECORE_XCB_SYNC
   xcb_sync_int64_t v;

   v.hi = (by < 0) ? ~0 : 0;
   v.lo = by;
   xcb_sync_change_counter(_ecore_xcb_conn, counter, v);
#endif /* ECORE_XCB_SYNC */
} /* ecore_x_sync_counter_inc */

EAPI void
ecore_x_sync_counter_val_wait(Ecore_X_Sync_Counter counter, int val)
{
#ifdef ECORE_XCB_SYNC
   xcb_sync_query_counter_cookie_t cookie;
   xcb_sync_query_counter_reply_t *reply;
   xcb_sync_int64_t v1;
   xcb_sync_int64_t v2;
   xcb_sync_waitcondition_t cond;

   /* what's the purpose of that call ?? as the value is erased... */
   cookie = xcb_sync_query_counter_unchecked(_ecore_xcb_conn, counter);
   reply = xcb_sync_query_counter_reply(_ecore_xcb_conn, cookie, NULL);
   v1 = reply->counter_value;
   free(reply);

   v1.hi = (val < 0) ? ~0 : 0;
   v1.lo = val;
   v2.hi = ((val + 1) < 0) ? ~0 : 0;
   v2.lo = val + 1;
   cond.trigger.counter = counter;
   cond.trigger.wait_type = XCB_SYNC_VALUETYPE_ABSOLUTE;
   cond.trigger.wait_value = v1;
   cond.trigger.test_type = XCB_SYNC_TESTTYPE_POSITIVE_COMPARISON;
   cond.event_threshold = v2;

   xcb_sync_await(_ecore_xcb_conn, 1, (const xcb_sync_waitcondition_t *)&cond);
#endif /* ECORE_XCB_SYNC */
} /* ecore_x_sync_counter_val_wait */

