#include <Elementary.h>

static Ecore_Thread *th = NULL;

static Evas_Object *win = NULL;
static Evas_Object *rect = NULL;

struct info
{
   double x, y;
};

// BEGIN - code running in my custom pthread instance
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
// END - code running in my custom pthread instance

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
   ecore_thread_cancel(th);
}

int
elm_main(int argc, char **argv)
{
   Evas_Object *o, *bg;
   
   win = elm_win_add(NULL, "efl-thread-5", ELM_WIN_BASIC);
   elm_win_title_set(win, "EFL Thread 5");
   evas_object_resize(win, 400, 400);
   evas_object_show(win);
   
   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);
   
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
   th = ecore_thread_feedback_run(th_do, th_feedback, th_end, th_cancel, 
                                  NULL, EINA_TRUE);
   elm_run();
   return 0;
}

ELM_MAIN()
