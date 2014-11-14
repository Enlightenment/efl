//Compile with:
//gcc -o efl_thread_5 efl_thread_5.c -g `pkg-config --cflags --libs elementary`
#include <Elementary.h>

static Ecore_Thread *thr = NULL;

static Evas_Object *win = NULL;
static Evas_Object *rect = NULL;

struct info
{
   double x, y;
};

// BEGIN - code running in my custom thread instance
//
static void
th_do(void *data, Ecore_Thread *th)
{
   double t = 0.0;

   // inside our "do" function for the ecore thread, lets do the real work
   for (;;)
     {
        struct info *inf = malloc(sizeof(struct info));

        if (inf)
          {
             inf->x = 200 + (200 * sin(t));
             inf->y = 200 + (200 * cos(t));
             // now we have recorded the timepoint we pass it as feedback
             // back to the mainloop. it will free it when done
             ecore_thread_feedback(th, inf);
          }
        // and sleep and loop
        usleep(1000);
        t += 0.02;
        // in case someone has asked us to cancel - then cancel this loop
        // co-operatively (cancelling is co-operative)
        if (ecore_thread_check(th)) break;
     }
}
//
// END - code running in my custom thread instance

static void // when mainloop gets feedback from worker
th_feedback(void *data, Ecore_Thread *th, void *msg)
{
   struct info *inf = msg;
   evas_object_move(rect, inf->x - 50, inf->y - 50);
   free(inf);
}

// BONUS (optional): called after th_do returns and has NOT been cancelled
static void th_end(void *data, Ecore_Thread *th) { printf("thread ended\n"); }
// BONUS (optional): called in mainloop AFTER thread has finished cancelling
static void th_cancel(void *data, Ecore_Thread *th) { printf("thread cancelled\n"); }

// just test cancelling the thread worker
static void
down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   if (thr) ecore_thread_cancel(thr);
   thr = NULL;
}

// on window delete - cancel thread then delete window and exit mainloop
static void
del(void *data, Evas_Object *obj, void *event_info)
{
   if (thr) ecore_thread_cancel(thr);
   thr = NULL;
   evas_object_del(obj);
   elm_exit();
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *o;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("efl-thread-5", "EFL Thread 5");
   evas_object_smart_callback_add(win, "delete,request", del, NULL);

   o = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(o, 50, 80, 180, 255);
   evas_object_resize(o, 100, 100);
   evas_object_show(o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, down, NULL);
   rect = o;

   // explicitly create ecore thread to do some "work on the side" and pass
   // in NULL as data ptr to callbacks and true at the end means to actually
   // make a new thread and not use the thread pool (there is a thread pool
   // with as many thread workers as there are cpu's so this means you do not
   // overload the cpu's with more work than you actually have processing
   // units *IF* your threads do actually spend their time doing actual
   // heavy computation)
   thr = ecore_thread_feedback_run(th_do, th_feedback, th_end, th_cancel,
                                   NULL, EINA_TRUE);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);

   elm_run();
   if (thr) ecore_thread_cancel(thr);

   return 0;
}
ELM_MAIN()
