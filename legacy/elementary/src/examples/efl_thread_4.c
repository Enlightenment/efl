#include <Elementary.h>
#include <pthread.h>

static Evas_Object *win = NULL;
static Evas_Object *rect = NULL;

struct info
{
   double x, y;
};

static void my_thread_mainloop_code(void *data);

static pthread_t thread_id;
static pthread_mutex_t th_lock;
static int th_exit = 0;

// BEGIN - code running in my custom pthread instance
//
static void *
my_thread_run(void *arg)
{
   double t = 0.0;

   // inside the pthread function lets loop forever incrimenting a time point
   for (;;)
     {
        struct info *inf = malloc(sizeof(struct info));
        int do_exit;
        
        if (inf)
          {
             inf->x = 200 + (200 * sin(t));
             inf->y = 200 + (200 * cos(t));
             // now call a function in the mainloop and pass it our allocated
             // data that it will free when it gets it
             ecore_main_loop_thread_safe_call_async
                (my_thread_mainloop_code, inf);
          }
        // and sleep and loop
        usleep(1000);
        t += 0.02;
        // in case someone has asked us to cancel - then cacnel this loop
        // co-operatively (cancelling is co-operative)
        pthread_mutex_lock(&th_lock);
        do_exit = th_exit;
        pthread_mutex_unlock(&th_lock);
        if (do_exit) break;
     }
   return NULL;
}
//
// END - code running in my custom pthread instance

static void
my_thread_new(void)
{
   pthread_attr_t attr;

   pthread_mutex_init(&th_lock, NULL);
   if (pthread_attr_init(&attr) != 0)
      perror("pthread_attr_init");
   if (pthread_create(&thread_id, &attr, my_thread_run, NULL) != 0)
      perror("pthread_create");
}

static void
my_thread_mainloop_code(void *data)
{
   struct info *inf = data;
   evas_object_move(rect, inf->x - 50, inf->y - 50);
   free(inf);
}

// just test cancelling the thread
static void
down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   pthread_mutex_lock(&th_lock);
   th_exit = 1;
   pthread_mutex_unlock(&th_lock);
}

int
elm_main(int argc, char **argv)
{
   Evas_Object *o, *bg;
   
   win = elm_win_add(NULL, "efl-thread-4", ELM_WIN_BASIC);
   elm_win_title_set(win, "EFL Thread 4");
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
   // new in the examples - we have a mouse down on the blue box cancel
   // the thread
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, down, NULL);
   rect = o;
   
   // create custom thread to do some "work on the side"
   my_thread_new();
   
   elm_run();
   return 0;
}

ELM_MAIN()
   
