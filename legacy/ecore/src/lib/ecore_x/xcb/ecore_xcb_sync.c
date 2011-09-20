#include "ecore_xcb_private.h"
# ifdef ECORE_XCB_SYNC
#  include <xcb/sync.h>
# endif

/* local variables */
static Eina_Bool _sync_avail = EINA_FALSE;

/* external variables */
int _ecore_xcb_event_sync = -1;

void 
_ecore_xcb_sync_init(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_SYNC
   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_sync_id);
#endif
}

void 
_ecore_xcb_sync_finalize(void) 
{
#ifdef ECORE_XCB_SYNC
   const xcb_query_extension_reply_t *ext_reply;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_SYNC
   ext_reply = xcb_get_extension_data(_ecore_xcb_conn, &xcb_sync_id);
   if ((ext_reply) && (ext_reply->present)) 
     {
        xcb_sync_initialize_cookie_t cookie;
        xcb_sync_initialize_reply_t *reply;

        cookie = 
          xcb_sync_initialize_unchecked(_ecore_xcb_conn, 
                                        XCB_SYNC_MAJOR_VERSION, 
                                        XCB_SYNC_MINOR_VERSION);
        reply = xcb_sync_initialize_reply(_ecore_xcb_conn, cookie, NULL);
        if (reply) 
          {
             if (reply->major_version >= 3) _sync_avail = EINA_TRUE;
             free(reply);
          }

        if (_sync_avail)
          _ecore_xcb_event_sync = ext_reply->first_event;
     }
#endif
}

void 
_ecore_xcb_sync_magic_send(int val, Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ecore_x_client_message32_send(win, 27777, XCB_EVENT_MASK_NO_EVENT, 
                                 0x7162534, (0x10000000 + val), win, 0, 0);
//   ecore_x_flush();
}

/* public functions */
EAPI Ecore_X_Sync_Alarm 
ecore_x_sync_alarm_new(Ecore_X_Sync_Counter counter) 
{
#ifdef ECORE_XCB_SYNC
   uint32_t list[6], mask;
   xcb_sync_int64_t init;
   Ecore_X_Sync_Alarm alarm;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if ((!_sync_avail) || (!counter)) return 0;

#ifdef ECORE_XCB_SYNC
   init.lo = 0;
   init.hi = 0;
   xcb_sync_set_counter(_ecore_xcb_conn, counter, init);

   mask = (XCB_SYNC_CA_COUNTER | XCB_SYNC_CA_VALUE_TYPE | 
           XCB_SYNC_CA_VALUE | XCB_SYNC_CA_TEST_TYPE | 
           XCB_SYNC_CA_DELTA | XCB_SYNC_CA_EVENTS);
   list[0] = counter;
   list[1] = XCB_SYNC_VALUETYPE_ABSOLUTE;
   list[2] = 1;
   list[3] = XCB_SYNC_TESTTYPE_POSITIVE_COMPARISON;
   list[4] = 1;
   list[5] = 1;
   alarm = xcb_generate_id(_ecore_xcb_conn);

   xcb_sync_create_alarm(_ecore_xcb_conn, alarm, mask, list);
   ecore_x_sync(); // needed

   return alarm;
#endif
   return 0;
}

EAPI Eina_Bool 
ecore_x_sync_alarm_free(Ecore_X_Sync_Alarm alarm) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if ((!_sync_avail) || (!alarm)) return EINA_FALSE;

#ifdef ECORE_XCB_SYNC
   xcb_sync_destroy_alarm(_ecore_xcb_conn, alarm);
//   ecore_x_flush();
   return EINA_TRUE;
#endif

   return EINA_FALSE;
}

EAPI Eina_Bool 
ecore_x_sync_counter_query(Ecore_X_Sync_Counter counter, unsigned int *val) 
{
#ifdef ECORE_XCB_SYNC
   xcb_sync_query_counter_cookie_t cookie;
   xcb_sync_query_counter_reply_t *reply;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if ((!_sync_avail) || (!counter)) return EINA_FALSE;

#ifdef ECORE_XCB_SYNC
   cookie = xcb_sync_query_counter_unchecked(_ecore_xcb_conn, counter);
   reply = xcb_sync_query_counter_reply(_ecore_xcb_conn, cookie, NULL);
   if (reply) 
     {
        if (val) *val = (unsigned int)reply->counter_value.lo;
        free(reply);
        return EINA_TRUE;
     }
#endif
   return EINA_FALSE;
}

EAPI void 
ecore_x_sync_counter_inc(Ecore_X_Sync_Counter counter, int by) 
{
#ifdef ECORE_XCB_SYNC
   xcb_sync_int64_t v;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if ((!_sync_avail) || (!counter)) return;

#ifdef ECORE_XCB_SYNC
   v.hi = (by < 0) ? ~0 : 0;
   v.lo = by;

   xcb_sync_change_counter(_ecore_xcb_conn, counter, v);
//   ecore_x_flush();
#endif
}

EAPI void 
ecore_x_sync_counter_val_wait(Ecore_X_Sync_Counter counter, int val) 
{
#ifdef ECORE_XCB_SYNC
   xcb_sync_query_counter_cookie_t cookie;
   xcb_sync_query_counter_reply_t *reply;
   xcb_sync_int64_t v1, v2;
   xcb_sync_waitcondition_t cond;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if ((!_sync_avail) || (!counter)) return;

#ifdef ECORE_XCB_SYNC
   cookie = xcb_sync_query_counter_unchecked(_ecore_xcb_conn, counter);
   reply = xcb_sync_query_counter_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return;
   v1 = reply->counter_value;
   free(reply);

   v1.hi = (val < 0) ? ~0 : 0;
   v1.lo = val;
   v2.hi = ((val + 1) < 0) ? ~0 : 0;
   v2.lo = (val + 1);

   cond.trigger.counter = counter;
   cond.trigger.wait_type = XCB_SYNC_VALUETYPE_ABSOLUTE;
   cond.trigger.wait_value = v1;
   cond.trigger.test_type = XCB_SYNC_TESTTYPE_POSITIVE_COMPARISON;
   cond.event_threshold = v2;

   xcb_sync_await(_ecore_xcb_conn, 1, &cond);
//   ecore_x_flush();
#endif
}

EAPI Ecore_X_Sync_Counter 
ecore_x_sync_counter_new(int val) 
{
#ifdef ECORE_XCB_SYNC
   xcb_sync_counter_t counter;
   xcb_sync_int64_t v;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_sync_avail) return 0;

#ifdef ECORE_XCB_SYNC
   v.hi = (val < 0) ? ~0 : 0;
   v.lo = val;

   counter = xcb_generate_id(_ecore_xcb_conn);
   xcb_sync_create_counter(_ecore_xcb_conn, counter, v);
//   ecore_x_flush();

   return counter;
#endif

   return 0;
}

EAPI void 
ecore_x_sync_counter_free(Ecore_X_Sync_Counter counter) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if ((!_sync_avail) || (!counter)) return;

#ifdef ECORE_XCB_SYNC
   xcb_sync_destroy_counter(_ecore_xcb_conn, counter);
//   ecore_x_flush();
#endif
}

EAPI void 
ecore_x_sync_counter_set(Ecore_X_Sync_Counter counter, int val) 
{
#ifdef ECORE_XCB_SYNC
   xcb_sync_int64_t v;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if ((!_sync_avail) || (!counter)) return;

#ifdef ECORE_XCB_SYNC
   v.hi = (val < 0) ? ~0 : 0;
   v.lo = val;

   xcb_sync_set_counter(_ecore_xcb_conn, counter, v);
//   ecore_x_flush();
#endif
}

EAPI void 
ecore_x_sync_counter_2_set(Ecore_X_Sync_Counter counter, int val_hi, unsigned int val_lo) 
{
#ifdef ECORE_XCB_SYNC
   xcb_sync_int64_t v;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if ((!_sync_avail) || (!counter)) return;

#ifdef ECORE_XCB_SYNC
   v.hi = val_hi;
   v.lo = val_lo;

   xcb_sync_set_counter(_ecore_xcb_conn, counter, v);
//   ecore_x_flush();
#endif
}

EAPI Eina_Bool 
ecore_x_sync_counter_2_query(Ecore_X_Sync_Counter counter, int *val_hi, unsigned int *val_lo) 
{
#ifdef ECORE_XCB_SYNC
   xcb_sync_query_counter_cookie_t cookie;
   xcb_sync_query_counter_reply_t *reply;
   xcb_sync_int64_t value;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if ((!_sync_avail) || (!counter)) return EINA_FALSE;

#ifdef ECORE_XCB_SYNC
   cookie = 
     xcb_sync_query_counter_unchecked(_ecore_xcb_conn, 
                                      (xcb_sync_counter_t)counter);
   reply = xcb_sync_query_counter_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return EINA_FALSE;
   value = reply->counter_value;
   free(reply);
   if (val_hi) *val_hi = (int)value.hi;
   if (val_lo) *val_lo = (unsigned int)value.lo;
   return EINA_TRUE;
#endif

   return EINA_FALSE;
}
