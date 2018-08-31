#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>

#include <Eina.h>
#include <Ecore.h>

#include "ecore_suite.h"

#define DEBUG 0

/////////////////////////////////////////////////////////////////////////////
static Eina_Thread_Queue *thq1, *thq2, *thqmaster;

/////////////////////////////////////////////////////////////////////////////
typedef struct
{
   Eina_Thread_Queue_Msg  head;
   int                    value;
   char                   pad[10];
} Msg;

static void
th1_do(void *data EINA_UNUSED, Ecore_Thread *th)
{
   int val = 100;

   for (;;)
     {
        Msg *msg;
        void *ref;

        msg = eina_thread_queue_send(thq1, sizeof(Msg), &ref);
        if (!msg) fail();
        msg->value = val;
        memset(msg->pad, 0x32, 10);
        eina_thread_queue_send_done(thq1, ref);
        if (val == 1000 || (ecore_thread_check(th))) break;
        val++;
     }
}

static void
th2_do(void *data EINA_UNUSED, Ecore_Thread *th)
{
   int val;

   for (;;)
     {
        Msg *msg;
        void *ref;

        msg = eina_thread_queue_wait(thq1, &ref);
        if (!msg) fail();
        val = msg->value;
        eina_thread_queue_wait_done(thq1, ref);

        msg = eina_thread_queue_send(thq2, sizeof(Msg), &ref);
        if (!msg) fail();
        msg->value = val;
        memset(msg->pad, 0x32, 10);
        eina_thread_queue_send_done(thq2, ref);
        if (val == 1000 || (ecore_thread_check(th))) break;
     }
}

EFL_START_TEST(ecore_test_ecore_thread_eina_thread_queue_t1)
{
   int val = 99;
   Ecore_Thread *eth1, *eth2;

   thq1 = eina_thread_queue_new();
   if (!thq1) fail();
   thq2 = eina_thread_queue_new();
   if (!thq2) fail();
   eth1 = ecore_thread_feedback_run(th1_do, NULL, NULL, NULL, NULL, EINA_TRUE);
   eth2 = ecore_thread_feedback_run(th2_do, NULL, NULL, NULL, NULL, EINA_TRUE);

   for (;;)
     {
        Msg *msg;
        void *ref;
        msg = eina_thread_queue_wait(thq2, &ref);
        if (!msg) fail();
        if (DEBUG) printf("V: %i   [%i]\n", msg->value, eina_thread_queue_pending_get(thq2));
        if (msg->value != (val + 1))
          {
             ck_abort_msg("ERRR %i not next after %i\n", msg->value, val);
          }
        val = msg->value;
        eina_thread_queue_wait_done(thq2, ref);
        if (val == 1000) break;
     }
   ecore_thread_wait(eth1, 0.1);
   ecore_thread_wait(eth2, 0.1);
   eina_thread_queue_free(thq1);
   eina_thread_queue_free(thq2);
}
EFL_END_TEST

/////////////////////////////////////////////////////////////////////////////
typedef struct
{
   Eina_Thread_Queue_Msg  head;
   int ops[1];
} Msg2;

static volatile int msgs = 0;

static void
thspeed2_do(void *data EINA_UNUSED, Ecore_Thread *th)
{
   Msg2 *msg;
   void *ref;

   for (;;)
     {
        msg = eina_thread_queue_wait(thq1, &ref);
        if (msg)
          {
             msgs++;
             eina_thread_queue_wait_done(thq1, ref);
          }
        if (msgs == 1000 || (ecore_thread_check(th)))
          {
             if (DEBUG) printf("msgs done\n");
             break;
          }
     }
}

EFL_START_TEST(ecore_test_ecore_thread_eina_thread_queue_t2)
{
   Msg2 *msg;
   void *ref;
   int i, mcount;
   Ecore_Thread *th;

   thq1 = eina_thread_queue_new();
   if (!thq1) fail();
   th = ecore_thread_feedback_run(thspeed2_do, NULL, NULL, NULL, NULL, EINA_TRUE);

   for (i = 0; i < 1000; i++)
     {
        msg = eina_thread_queue_send(thq1, sizeof(Msg2), &ref);
        if (!msg) fail();
        if (msg) eina_thread_queue_send_done(thq1, ref);
     }
   ecore_thread_wait(th, 100.0);
   mcount = msgs;
   if (mcount < 1000)
     {
        ck_abort_msg("ERR: not enough messages received -> %i\n", mcount);
     }
   if (DEBUG) printf("%i messages sent\n", i);
   ecore_thread_wait(th, 0.1);
   eina_thread_queue_free(thq1);
}
EFL_END_TEST

/////////////////////////////////////////////////////////////////////////////
typedef struct
{
   Eina_Thread_Queue_Msg  head;
   int                    value;
} Msg3;

static void
th31_do(void *data EINA_UNUSED, Ecore_Thread *th)
{
   int val = 100;

   for (;;)
     {
        Msg3 *msg;
        void *ref;

        msg = eina_thread_queue_send(thq1, sizeof(Msg3), &ref);
        if (!msg) fail();
        msg->value = val;
        eina_thread_queue_send_done(thq1, ref);
        val++;
        if (val == 1100 || (ecore_thread_check(th))) break;
     }
}

static void
th32_do(void *data EINA_UNUSED, Ecore_Thread *th)
{
   int val = 100;

   for (;;)
     {
        Msg3 *msg;
        void *ref;

        msg = eina_thread_queue_send(thq2, sizeof(Msg3), &ref);
        if (!msg) fail();
        msg->value = val;
        eina_thread_queue_send_done(thq2, ref);
        val++;
        if (val == 1100 || (ecore_thread_check(th))) break;
     }
}

EFL_START_TEST(ecore_test_ecore_thread_eina_thread_queue_t3)
{
   int val1 = 99, val2 = 99, cnt = 0;
   Eina_Thread_Queue *parent;
   Ecore_Thread *eth1, *eth2;

   thq1 = eina_thread_queue_new();
   if (!thq1) fail();
   thq2 = eina_thread_queue_new();
   if (!thq2) fail();
   thqmaster = eina_thread_queue_new();
   if (!thqmaster) fail();
   eina_thread_queue_parent_set(thq1, thqmaster);
   eina_thread_queue_parent_set(thq2, thqmaster);

   parent = eina_thread_queue_parent_get(thq1);
   fail_if(parent != thqmaster);
   parent = eina_thread_queue_parent_get(thq2);
   fail_if(parent != thqmaster);

   eth1 = ecore_thread_feedback_run(th31_do, NULL, NULL, NULL, NULL, EINA_TRUE);
   eth2 = ecore_thread_feedback_run(th32_do, NULL, NULL, NULL, NULL, EINA_TRUE);
   for (;;)
     {
        Eina_Thread_Queue_Msg_Sub *sub;
        Eina_Thread_Queue *thq;
        Msg3 *msg;
        void *ref;

        sub = eina_thread_queue_wait(thqmaster, &ref);
        thq = sub->queue;
        eina_thread_queue_wait_done(thqmaster, ref);

        msg = eina_thread_queue_wait(thq, &ref);
        if (DEBUG) printf("V %09i: %p - %i  [%i]\n", cnt, thq, msg->value, eina_thread_queue_pending_get(thqmaster));
        if (thq == thq1)
          {
             if ((val1 + 1) != msg->value)
               {
                  ck_abort_msg("ERR: thq1 val wrong %i -> %i\n", val1, msg->value);
               }
             val1 = msg->value;
          }
        else if (thq == thq2)
          {
             if ((val2 + 1) != msg->value)
               {
                  ck_abort_msg("ERR: thq2 val wrong %i -> %i\n", val2, msg->value);
               }
             val2 = msg->value;
          }
        else
          {
             ck_abort_msg("ERRR: unknown thq\n");
          }
        eina_thread_queue_wait_done(thq, ref);
        cnt++;
        if (cnt == 2000) break;
     }
   if (DEBUG) printf("enough msgs\n");
   ecore_thread_wait(eth1, 0.1);
   ecore_thread_wait(eth2, 0.1);
   eina_thread_queue_free(thq1);
   eina_thread_queue_free(thq2);
   eina_thread_queue_free(thqmaster);
}
EFL_END_TEST

/////////////////////////////////////////////////////////////////////////////
typedef struct
{
   Eina_Thread_Queue_Msg  head;
   int                    value;
} Msg4;

static void
th41_do(void *data EINA_UNUSED, Ecore_Thread *th)
{
   int val = 100;

   for (;;)
     {
        Msg4 *msg;
        void *ref;

        msg = eina_thread_queue_send(thq1, sizeof(Msg4), &ref);
        msg->value = val;
        eina_thread_queue_send_done(thq1, ref);
        val++;
        if (val == 1100 || (ecore_thread_check(th))) break;
     }
}

static void
th42_do(void *data EINA_UNUSED, Ecore_Thread *th)
{
   int val = 10000;

   for (;;)
     {
        Msg4 *msg;
        void *ref;

        msg = eina_thread_queue_send(thq1, sizeof(Msg4), &ref);
        msg->value = val;
        eina_thread_queue_send_done(thq1, ref);
        val++;
        if (val == 11000 || (ecore_thread_check(th))) break;
     }
}


EFL_START_TEST(ecore_test_ecore_thread_eina_thread_queue_t4)
{
   int cnt = 0;
   int val1 = 99, val2 = 9999;
   Ecore_Thread *eth1, *eth2;

   thq1 = eina_thread_queue_new();
   if (!thq1) fail();
   eth1 = ecore_thread_feedback_run(th41_do, NULL, NULL, NULL, NULL, EINA_TRUE);
   eth2 = ecore_thread_feedback_run(th42_do, NULL, NULL, NULL, NULL, EINA_TRUE);
   for (;;)
     {
        Msg4 *msg;
        void *ref;
        msg = eina_thread_queue_wait(thq1, &ref);
        if (DEBUG) printf("V %08i: %i  [%i]\n", cnt, msg->value, eina_thread_queue_pending_get(thq1));
        if (msg->value >= 10000)
          {
             if ((val2 + 1) != msg->value)
               {
                  ck_abort_msg("ERR: val wrong %i -> %i\n", val2, msg->value);
               }
             val2 = msg->value;
          }
        else
          {
             if ((val1 + 1) != msg->value)
               {
                  ck_abort_msg("ERR: val wrong %i -> %i\n", val1, msg->value);
               }
             val1 = msg->value;
          }
        eina_thread_queue_wait_done(thq1, ref);
        cnt++;
        if (cnt == 2000) break;
     }
   if (DEBUG) printf("msgs ok\n");
   ecore_thread_wait(eth1, 0.1);
   ecore_thread_wait(eth2, 0.1);
   eina_thread_queue_free(thq1);
}
EFL_END_TEST

/////////////////////////////////////////////////////////////////////////////
typedef struct
{
   Eina_Thread_Queue_Msg  head;
   int                    value;
   char                   pad[10];
} Msg5;

static Eina_Semaphore th4_sem;

static void
th51_do(void *data EINA_UNUSED, Ecore_Thread *th)
{
   int val = 100;

   for (;;)
     {
        Msg5 *msg;
        void *ref;

        msg = eina_thread_queue_send(thq1, sizeof(Msg5), &ref);
        msg->value = val;
        eina_thread_queue_send_done(thq1, ref);
        if (val == 1100 || (ecore_thread_check(th))) break;
        val++;
     }

   eina_semaphore_release(&th4_sem, 1);
}

static void
th52_do(void *data EINA_UNUSED, Ecore_Thread *th)
{
   int val;

   for (;;)
     {
        Msg5 *msg;
        void *ref;

        msg = eina_thread_queue_wait(thq1, &ref);
        if (!msg) fail();
        val = msg->value;
        eina_thread_queue_wait_done(thq1, ref);

        msg = eina_thread_queue_send(thq2, sizeof(Msg5), &ref);
        if (!msg) fail();
        msg->value = val;
        eina_thread_queue_send_done(thq2, ref);
        if (val == 1100 || (ecore_thread_check(th))) break;
     }

   eina_semaphore_release(&th4_sem, 1);
}


EFL_START_TEST(ecore_test_ecore_thread_eina_thread_queue_t5)
{
   int val = 99;
   Ecore_Thread *eth1, *eth2;

   eina_semaphore_new(&th4_sem, 0);

   thq1 = eina_thread_queue_new();
   if (!thq1) fail();
   thq2 = eina_thread_queue_new();
   if (!thq2) fail();
   eth1 = ecore_thread_feedback_run(th51_do, NULL, NULL, NULL, NULL, EINA_TRUE);
   eth2 = ecore_thread_feedback_run(th52_do, NULL, NULL, NULL, NULL, EINA_TRUE);

   for (;;)
     {
        Msg5 *msg;
        void *ref;
        msg = eina_thread_queue_poll(thq2, &ref);
        if (msg)
          {
             if (DEBUG) printf("V: %i   [%i]\n", msg->value, eina_thread_queue_pending_get(thq2));
             if (msg->value != (val + 1))
               {
                  ck_abort_msg("ERRR %i not next after %i\n", msg->value, val);
               }
             val = msg->value;
             eina_thread_queue_wait_done(thq2, ref);
             if (val == 1100) break;
          }
        else
          {
             if (DEBUG) printf("V: none!\n");
          }
     }

   eina_semaphore_lock(&th4_sem);
   eina_semaphore_lock(&th4_sem);

   // All done!
   ecore_thread_wait(eth1, 0.1);
   ecore_thread_wait(eth2, 0.1);
   eina_semaphore_free(&th4_sem);
   eina_thread_queue_free(thq1);
   eina_thread_queue_free(thq2);
}
EFL_END_TEST

/////////////////////////////////////////////////////////////////////////////
typedef struct
{
   Eina_Thread_Queue_Msg  head;
   int                    value;
} Msg6;

static Eina_Spinlock msgnum_lock;
static volatile int msgnum = 0;
static Eina_Semaphore th6_sem;
const int EXIT_MESSAGE = -42;

static void
th61_do(void *data EINA_UNUSED, Ecore_Thread *th)
{
   int val = 100;

   for (val = 100; val < 200; val++)
     {
        Msg6 *msg;
        void *ref;

        msg = eina_thread_queue_send(thq1, sizeof(Msg6), &ref);
        fail_if(!msg);
        msg->value = val;
        eina_thread_queue_send_done(thq1, ref);
        if (ecore_thread_check(th)) break;
     }

   eina_semaphore_release(&th6_sem, 1);
   if (DEBUG) printf("%s: message sending done!\n", __FUNCTION__);
}

static void
th62_do(void *data EINA_UNUSED, Ecore_Thread *th)
{
   int cnt = 0;

   for (;;)
     {
        Msg6 *msg;
        void *ref;
        int val;

        msg = eina_thread_queue_wait(thq1, &ref);
        fail_if(!msg);
        if (DEBUG) printf("%s: v %08i: %i  [%i]\n", __FUNCTION__, cnt, msg->value, eina_thread_queue_pending_get(thq1));
        val = msg->value;
        eina_thread_queue_wait_done(thq1, ref);
        if (val == EXIT_MESSAGE || (ecore_thread_check(th))) break;
        cnt++;
        eina_spinlock_take(&msgnum_lock);
        msgnum++;
        eina_spinlock_release(&msgnum_lock);
     }

   eina_spinlock_take(&msgnum_lock);
   ck_assert_int_eq(msgnum, 100);
   eina_spinlock_release(&msgnum_lock);
   eina_semaphore_release(&th6_sem, 1);
   if (DEBUG) printf("%s: message reading done!\n", __FUNCTION__);
}

static void
th63_do(void *data EINA_UNUSED, Ecore_Thread *th)
{
   int cnt = 0;

   for (;;)
     {
        Msg6 *msg;
        void *ref;
        int val;

        msg = eina_thread_queue_wait(thq1, &ref);
        fail_if(!msg);
        if (DEBUG) printf("%s: v %08i: %i  [%i]\n", __FUNCTION__, cnt, msg->value, eina_thread_queue_pending_get(thq1));
        val = msg->value;
        eina_thread_queue_wait_done(thq1, ref);
        if (val == EXIT_MESSAGE || (ecore_thread_check(th))) break;
        cnt++;
        eina_spinlock_take(&msgnum_lock);
        msgnum++;
        eina_spinlock_release(&msgnum_lock);
     }

   eina_spinlock_take(&msgnum_lock);
   ck_assert_int_eq(msgnum, 100);
   eina_spinlock_release(&msgnum_lock);
   eina_semaphore_release(&th6_sem, 1);
   if (DEBUG) printf("%s: message reading done!\n", __FUNCTION__);
}

EFL_START_TEST(ecore_test_ecore_thread_eina_thread_queue_t6)
{
   int do_break = 0;
   Msg6 *msg;
   void *ref;
   Ecore_Thread *eth1, *eth2, *eth3;

   if (DEBUG) setbuf(stdout, NULL);

   eina_semaphore_new(&th6_sem, 0);
   eina_spinlock_new(&msgnum_lock);
   thq1 = eina_thread_queue_new();
   fail_if(!thq1);
   eth1 = ecore_thread_feedback_run(th61_do, NULL, NULL, NULL, NULL, EINA_TRUE);
   eth2 = ecore_thread_feedback_run(th62_do, NULL, NULL, NULL, NULL, EINA_TRUE);
   eth3 = ecore_thread_feedback_run(th63_do, NULL, NULL, NULL, NULL, EINA_TRUE);

   // Spin until we reach 10000 messages sent
   while (!do_break)
     {
        eina_spinlock_take(&msgnum_lock);
        if (DEBUG) printf("msgnum %i\n", msgnum);
        if (msgnum == 100) do_break = 1;
        else ck_assert_int_lt(msgnum, 100);
        eina_spinlock_release(&msgnum_lock);
     }

   // Send exit message twice
   msg = eina_thread_queue_send(thq1, sizeof(Msg6), &ref);
   msg->value = EXIT_MESSAGE;
   eina_thread_queue_send_done(thq1, ref);

   msg = eina_thread_queue_send(thq1, sizeof(Msg6), &ref);
   msg->value = EXIT_MESSAGE;
   eina_thread_queue_send_done(thq1, ref);

   // Wait for 3 threads
   fail_if(!eina_semaphore_lock(&th6_sem));
   fail_if(!eina_semaphore_lock(&th6_sem));
   fail_if(!eina_semaphore_lock(&th6_sem));

   // All done!
   ecore_thread_wait(eth1, 0.1);
   ecore_thread_wait(eth2, 0.1);
   ecore_thread_wait(eth3, 0.1);
   eina_semaphore_free(&th6_sem);
   eina_thread_queue_free(thq1);
   eina_spinlock_free(&msgnum_lock);

}
EFL_END_TEST

/////////////////////////////////////////////////////////////////////////////
typedef struct
{
   Eina_Thread_Queue_Msg  head;
   int ops[1];
} Msg7;

int p[2];

static void
thspeed7_do(void *data EINA_UNUSED, Ecore_Thread *th)
{
   Msg7 *msg;
   void *ref;
   int i;

   for (i = 0; i < 10000; i++)
     {
        msg = eina_thread_queue_send(thq1, sizeof(Msg7), &ref);
        if (msg) eina_thread_queue_send_done(thq1, ref);
        if (ecore_thread_check(th)) break;
     }
}

EFL_START_TEST(ecore_test_ecore_thread_eina_thread_queue_t7)
{
   Msg7 *msg;
   void *ref;
   int msgcnt = 0, ret;
   Ecore_Thread *eth1;

   thq1 = eina_thread_queue_new();
   if (!thq1) fail();
   if (pipe(p) != 0)
     {
        ck_abort_msg("ERR: pipe create fail\n");
     }
   eina_thread_queue_fd_set(thq1, p[1]);

   ret = eina_thread_queue_fd_get(thq1);
   fail_if(ret != p[1]);

   eth1 = ecore_thread_feedback_run(thspeed7_do, NULL, NULL, NULL, NULL, EINA_TRUE);
   for (;;)
     {
        char buf;

        read(p[0], &buf, 1);
        msg = eina_thread_queue_wait(thq1, &ref);
        if (msg)
          {
             eina_thread_queue_wait_done(thq1, ref);
             msgcnt++;
             if (DEBUG) printf("msgcnt: %i\n", msgcnt);
          }
        if (msgcnt == 10000) break;
     }
   if (DEBUG) printf("msg fd ok\n");
   ecore_thread_wait(eth1, 0.1);
   eina_thread_queue_free(thq1);
   close(p[0]);
   close(p[1]);
}
EFL_END_TEST

void ecore_test_ecore_thread_eina_thread_queue(TCase *tc EINA_UNUSED)
{
   tcase_add_test(tc, ecore_test_ecore_thread_eina_thread_queue_t1);
   tcase_add_test(tc, ecore_test_ecore_thread_eina_thread_queue_t2);
   tcase_add_test(tc, ecore_test_ecore_thread_eina_thread_queue_t3);
}

void ecore_test_ecore_thread_eina_thread_queue2(TCase *tc EINA_UNUSED)
{
   tcase_add_test(tc, ecore_test_ecore_thread_eina_thread_queue_t4);
   tcase_add_test(tc, ecore_test_ecore_thread_eina_thread_queue_t5);
   tcase_add_test(tc, ecore_test_ecore_thread_eina_thread_queue_t6);
   tcase_add_test(tc, ecore_test_ecore_thread_eina_thread_queue_t7);
}
