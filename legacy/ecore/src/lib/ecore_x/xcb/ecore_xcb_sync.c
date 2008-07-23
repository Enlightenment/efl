/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

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
}

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
}


/**
 * Return whether the X server supports the Sync Extension.
 * @return 1 if the X Sync Extension is available, 0 otherwise.
 *
 * Return 1 if the X server supports the Sync Extension version 3.0,
 * 0 otherwise.
 * @ingroup Ecore_X_Sync_Group
 */
EAPI int
ecore_x_sync_query(void)
{
#ifdef ECORE_XCB_SYNC
   return _sync_available;
#else
   return 0;
#endif /* ECORE_XCB_SYNC */
}


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
   uint32_t           value_list[6];
   xcb_sync_int64_t   init;
   Ecore_X_Sync_Alarm alarm;
   uint32_t           value_mask;

   init.lo = 0;
   init.hi = 0;
   xcb_sync_set_counter(_ecore_xcb_conn, counter, init);

   value_mask =
     XCB_SYNC_CA_COUNTER | XCB_SYNC_CA_VALUE_TYPE |
     XCB_SYNC_CA_VALUE   | XCB_SYNC_CA_TEST_TYPE  |
     XCB_SYNC_CA_DELTA   | XCB_SYNC_CA_EVENTS;
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
#else
   return 0;
#endif /* ECORE_XCB_SYNC */
}


/**
 * Delete an alarm.
 * @param alarm The alarm to delete.
 * @return      1 on success, 0 otherwise.
 *
 * Delete the @p alarm. Returns 1 on success, 0 otherwise.
 * @ingroup Ecore_X_Sync_Group
 */
EAPI int
ecore_x_sync_alarm_free(Ecore_X_Sync_Alarm alarm)
{
#ifdef ECORE_XCB_SYNC
   xcb_sync_destroy_alarm(_ecore_xcb_conn, alarm);
   return 1;
#else
   return 0;
#endif /* ECORE_XCB_SYNC */
}
