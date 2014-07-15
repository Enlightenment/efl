#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include <Eina.h>
#include <unistd.h>
#include <stdio.h>

#include "ecore_suite.h"

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
th1_do(void *data EINA_UNUSED, Ecore_Thread *th EINA_UNUSED)
{
   int val = 100;

   for (;;)
     {
        Msg *msg;
        void *ref;

        usleep((rand() % 10) * 1000);
        msg = eina_thread_queue_send(thq1, sizeof(Msg), &ref);
        msg->value = val;
        eina_thread_queue_send_done(thq1, ref);
        if (val == 1000) break;
        val++;
     }
}

static void
th2_do(void *data EINA_UNUSED, Ecore_Thread *th EINA_UNUSED)
{
   int val;

   for (;;)
     {
        Msg *msg;
        void *ref;

        msg = eina_thread_queue_wait(thq1, &ref);
        val = msg->value;
        usleep((rand() % 20) * 1000);
        eina_thread_queue_wait_done(thq1, ref);
        msg = eina_thread_queue_send(thq2, sizeof(Msg), &ref);
        msg->value = val;
        eina_thread_queue_send_done(thq2, ref);
        if (val == 1000) break;
     }
}

START_TEST(ecore_test_ecore_thread_eina_thread_queue_t1)
{
   int val = 99;

   eina_init();
   ecore_init();

   thq1 = eina_thread_queue_new();
   thq2 = eina_thread_queue_new();
   ecore_thread_feedback_run(th1_do, NULL, NULL, NULL, NULL, EINA_TRUE);
   ecore_thread_feedback_run(th2_do, NULL, NULL, NULL, NULL, EINA_TRUE);

   for (;;)
     {
        Msg *msg;
        void *ref;
        msg = eina_thread_queue_wait(thq2, &ref);
        printf("V: %i   [%i]\n", msg->value, eina_thread_queue_pending_get(thq2));
        if (msg->value != (val + 1))
          {
             printf("ERRR %i not next after %i\n", msg->value, val);
             fail();
          }
        val = msg->value;
        usleep((rand() % 30) * 1000);
        eina_thread_queue_wait_done(thq2, ref);
        if (val == 1000) break;
     }
}
END_TEST

/////////////////////////////////////////////////////////////////////////////
typedef struct
{
   Eina_Thread_Queue_Msg  head;
   int ops[1];
} Msg2;

static int msgs = 0;

static void
thspeed1_do(void *data EINA_UNUSED, Ecore_Thread *th EINA_UNUSED)
{
   Msg2 *msg;
   void *ref;

   for (;;)
     {
        msg = eina_thread_queue_wait(thq1, &ref);
        if (msg)
          {
             eina_thread_queue_wait_done(thq1, ref);
             msgs++;
          }
        if (msgs == 10000000)
          {
             printf("msgs done\n");
             break;
          }
     }
}

START_TEST(ecore_test_ecore_thread_eina_thread_queue_t2)
{
   Msg2 *msg;
   void *ref;
   int i;

   eina_init();
   ecore_init();

   thq1 = eina_thread_queue_new();
   ecore_thread_feedback_run(thspeed1_do, NULL, NULL, NULL, NULL, EINA_TRUE);

   for (i = 0; i < 10000000; i++)
     {
        msg = eina_thread_queue_send(thq1, sizeof(Msg2), &ref);
        if (msg) eina_thread_queue_send_done(thq1, ref);
     }
   if (msgs < 1000000)
     {
        printf("ERR: not enough messages recieved\n");
        fail();
     }
   printf("%i messages sent\n", i);
}
END_TEST

/////////////////////////////////////////////////////////////////////////////
typedef struct
{
   Eina_Thread_Queue_Msg  head;
   int                    value;
} Msg3;

static void
th21_do(void *data EINA_UNUSED, Ecore_Thread *th EINA_UNUSED)
{
   int val = 100;

   for (;;)
     {
        Msg3 *msg;
        void *ref;

        msg = eina_thread_queue_send(thq1, sizeof(Msg3), &ref);
        msg->value = val;
        eina_thread_queue_send_done(thq1, ref);
        val++;
        if (val == 1000100) break;
     }
}

static void
th22_do(void *data EINA_UNUSED, Ecore_Thread *th EINA_UNUSED)
{
   int val = 100;

   for (;;)
     {
        Msg3 *msg;
        void *ref;

        msg = eina_thread_queue_send(thq2, sizeof(Msg3), &ref);
        msg->value = val;
        eina_thread_queue_send_done(thq2, ref);
        val++;
        if (val == 1000100) break;
     }
}

START_TEST(ecore_test_ecore_thread_eina_thread_queue_t3)
{
   int val1 = 99, val2 = 99, cnt = 0;
   eina_init();
   ecore_init();

   thq1 = eina_thread_queue_new();
   thq2 = eina_thread_queue_new();
   thqmaster = eina_thread_queue_new();
   eina_thread_queue_parent_set(thq1, thqmaster);
   eina_thread_queue_parent_set(thq2, thqmaster);
   ecore_thread_feedback_run(th21_do, NULL, NULL, NULL, NULL, EINA_TRUE);
   ecore_thread_feedback_run(th22_do, NULL, NULL, NULL, NULL, EINA_TRUE);
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
        printf("V %09i: %p - %i  [%i]\n", cnt, thq, msg->value, eina_thread_queue_pending_get(thqmaster));
        if (thq == thq1)
          {
             if ((val1 + 1) != msg->value)
               {
                  printf("ERR: thq1 val wrong %i -> %i\n", val1, msg->value);
                  fail();
               }
             val1 = msg->value;
          }
        else if (thq == thq2)
          {
             if ((val2 + 1) != msg->value)
               {
                  printf("ERR: thq2 val wrong %i -> %i\n", val2, msg->value);
                  fail();
               }
             val2 = msg->value;
          }
        else
          {
             printf("ERRR: unknown thq\n");
             fail();
          }
        eina_thread_queue_wait_done(thq, ref);
        cnt++;
        if (cnt == 2000000) break;
     }
   printf("enough msgs\n");
}
END_TEST

/////////////////////////////////////////////////////////////////////////////
typedef struct
{
   Eina_Thread_Queue_Msg  head;
   int                    value;
} Msg4;

static void
th31_do(void *data EINA_UNUSED, Ecore_Thread *th EINA_UNUSED)
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
        if (val == 1000100) break;
     }
}

static void
th32_do(void *data EINA_UNUSED, Ecore_Thread *th EINA_UNUSED)
{
   int val = 10000000;

   for (;;)
     {
        Msg4 *msg;
        void *ref;

        msg = eina_thread_queue_send(thq1, sizeof(Msg4), &ref);
        msg->value = val;
        eina_thread_queue_send_done(thq1, ref);
        val++;
        if (val == 11000000) break;
     }
}


START_TEST(ecore_test_ecore_thread_eina_thread_queue_t4)
{
   int cnt = 0;
   int val1 = 99, val2 = 9999999;

   eina_init();
   ecore_init();
   thq1 = eina_thread_queue_new();
   ecore_thread_feedback_run(th31_do, NULL, NULL, NULL, NULL, EINA_TRUE);
   ecore_thread_feedback_run(th32_do, NULL, NULL, NULL, NULL, EINA_TRUE);
   for (;;)
     {
        Msg4 *msg;
        void *ref;
        msg = eina_thread_queue_wait(thq1, &ref);
        printf("V %08i: %i  [%i]\n", cnt, msg->value, eina_thread_queue_pending_get(thq1));
        if (msg->value >= 10000000)
          {
             if ((val2 + 1) != msg->value)
               {
                  printf("ERR: val wrong %i -> %i\n", val2, msg->value);
                  fail();
               }
             val2 = msg->value;
          }
        else
          {
             if ((val1 + 1) != msg->value)
               {
                  printf("ERR: val wrong %i -> %i\n", val1, msg->value);
                  fail();
               }
             val1 = msg->value;
          }
        eina_thread_queue_wait_done(thq1, ref);
        cnt++;
        if (cnt == 2000000) break;
     }
   printf("msgs ok\n");
}
END_TEST

/////////////////////////////////////////////////////////////////////////////
typedef struct
{
   Eina_Thread_Queue_Msg  head;
   int                    value;
   char                   pad[10];
} Msg5;

static void
th41_do(void *data EINA_UNUSED, Ecore_Thread *th EINA_UNUSED)
{
   int val = 100;

   for (;;)
     {
        Msg5 *msg;
        void *ref;

        usleep((rand() % 10) * 1000);
        msg = eina_thread_queue_send(thq1, sizeof(Msg5), &ref);
        msg->value = val;
        eina_thread_queue_send_done(thq1, ref);
        if (val == 1100) break;
        val++;
     }
}

static void
th42_do(void *data EINA_UNUSED, Ecore_Thread *th EINA_UNUSED)
{
   int val;

   for (;;)
     {
        Msg5 *msg;
        void *ref;

        msg = eina_thread_queue_wait(thq1, &ref);
        val = msg->value;
        usleep((rand() % 20) * 1000);
        eina_thread_queue_wait_done(thq1, ref);
        msg = eina_thread_queue_send(thq2, sizeof(Msg5), &ref);
        msg->value = val;
        eina_thread_queue_send_done(thq2, ref);
        if (val == 1100) break;
     }
}


START_TEST(ecore_test_ecore_thread_eina_thread_queue_t5)
{
   int val = 99;

   eina_init();
   ecore_init();

   thq1 = eina_thread_queue_new();
   thq2 = eina_thread_queue_new();
   ecore_thread_feedback_run(th41_do, NULL, NULL, NULL, NULL, EINA_TRUE);
   ecore_thread_feedback_run(th42_do, NULL, NULL, NULL, NULL, EINA_TRUE);

   for (;;)
     {
        Msg5 *msg;
        void *ref;
        msg = eina_thread_queue_poll(thq2, &ref);
        if (msg)
          {
             printf("V: %i   [%i]\n", msg->value, eina_thread_queue_pending_get(thq2));
             if (msg->value != (val + 1))
               {
                  printf("ERRR %i not next after %i\n", msg->value, val);
                  fail();
               }
             val = msg->value;
             usleep((rand() % 10) * 1000);
             eina_thread_queue_wait_done(thq2, ref);
             if (val == 1100) break;
          }
        else
          {
             printf("V: none!\n");
             usleep((rand() % 10) * 1000);
          }
     }
}
END_TEST

/////////////////////////////////////////////////////////////////////////////
typedef struct
{
   Eina_Thread_Queue_Msg  head;
   int                    value;
} Msg6;

static Eina_Spinlock msgnum_lock;
static int msgnum = 0;

static void
th51_do(void *data EINA_UNUSED, Ecore_Thread *th EINA_UNUSED)
{
   int val = 100;

   for (;;)
     {
        Msg6 *msg;
        void *ref;

        msg = eina_thread_queue_send(thq1, sizeof(Msg6), &ref);
        msg->value = val;
        eina_thread_queue_send_done(thq1, ref);
        val++;
        if (val == 10100) break;
        usleep(1);
     }
}

static void
th52_do(void *data EINA_UNUSED, Ecore_Thread *th EINA_UNUSED)
{
   int cnt = 0;

   for (;;)
     {
        Msg6 *msg;
        void *ref;
        msg = eina_thread_queue_wait(thq1, &ref);
        printf("v %08i: %i  [%i]\n", cnt, msg->value, eina_thread_queue_pending_get(thq1));
        eina_thread_queue_wait_done(thq1, ref);
        cnt++;
        eina_spinlock_take(&msgnum_lock);
        msgnum++;
        eina_spinlock_release(&msgnum_lock);
     }
}

static void
th53_do(void *data EINA_UNUSED, Ecore_Thread *th EINA_UNUSED)
{
   int cnt = 0;

   for (;;)
     {
        Msg6 *msg;
        void *ref;
        msg = eina_thread_queue_wait(thq1, &ref);
        printf("v %08i: %i  [%i]\n", cnt, msg->value, eina_thread_queue_pending_get(thq1));
        eina_thread_queue_wait_done(thq1, ref);
        cnt++;
        eina_spinlock_take(&msgnum_lock);
        msgnum++;
        eina_spinlock_release(&msgnum_lock);
     }
}

START_TEST(ecore_test_ecore_thread_eina_thread_queue_t6)
{
   eina_init();
   ecore_init();

   eina_spinlock_new(&msgnum_lock);
   thq1 = eina_thread_queue_new();
   ecore_thread_feedback_run(th51_do, NULL, NULL, NULL, NULL, EINA_TRUE);
   ecore_thread_feedback_run(th52_do, NULL, NULL, NULL, NULL, EINA_TRUE);
   ecore_thread_feedback_run(th53_do, NULL, NULL, NULL, NULL, EINA_TRUE);

   for (;;)
     {
        eina_spinlock_take(&msgnum_lock);
        printf("msgnum %i\n", msgnum);
        if (msgnum == 10000)
          {
             eina_spinlock_release(&msgnum_lock);
             break;
          }
        eina_spinlock_release(&msgnum_lock);
     }
   printf("msg multi to 1 ok\n");
}
END_TEST

/////////////////////////////////////////////////////////////////////////////
typedef struct
{
   Eina_Thread_Queue_Msg  head;
   int ops[1];
} Msg7;

int p[2];

static void
thspeed21_do(void *data EINA_UNUSED, Ecore_Thread *th EINA_UNUSED)
{
   Msg7 *msg;
   void *ref;
   int i;

   for (i = 0; i < 1000000; i++)
     {
        msg = eina_thread_queue_send(thq1, sizeof(Msg7), &ref);
        if (msg) eina_thread_queue_send_done(thq1, ref);
     }
}

START_TEST(ecore_test_ecore_thread_eina_thread_queue_t7)
{
   Msg7 *msg;
   void *ref;
   int msgcnt = 0;

   eina_init();
   ecore_init();

   thq1 = eina_thread_queue_new();
   if (pipe(p) != 0)
     {
        printf("ERR: pipe create fail\n");
        fail();
     }
   eina_thread_queue_fd_set(thq1, p[1]);
   ecore_thread_feedback_run(thspeed21_do, NULL, NULL, NULL, NULL, EINA_TRUE);
   for (;;)
     {
        char buf;

        read(p[0], &buf, 1);
        msg = eina_thread_queue_wait(thq1, &ref);
        if (msg)
          {
             eina_thread_queue_wait_done(thq1, ref);
             msgcnt++;
             printf("msgcnt: %i\n", msgcnt);
          }
        if (msgcnt == 1000000) break;
     }
   printf("msg fd ok\n");
}
END_TEST

void ecore_test_ecore_thread_eina_thread_queue(TCase *tc EINA_UNUSED)
{
   tcase_add_test(tc, ecore_test_ecore_thread_eina_thread_queue_t1);
   tcase_add_test(tc, ecore_test_ecore_thread_eina_thread_queue_t2);
   tcase_add_test(tc, ecore_test_ecore_thread_eina_thread_queue_t3);
   tcase_add_test(tc, ecore_test_ecore_thread_eina_thread_queue_t4);
   tcase_add_test(tc, ecore_test_ecore_thread_eina_thread_queue_t5);
   tcase_add_test(tc, ecore_test_ecore_thread_eina_thread_queue_t6);
   tcase_add_test(tc, ecore_test_ecore_thread_eina_thread_queue_t7);
}
