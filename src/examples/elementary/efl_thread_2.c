//Compile with:
//gcc -o efl_thread_2 efl_thread_2.c -g `pkg-config --cflags --libs elementary`
#include <Elementary.h>
#include <pthread.h>

static Evas_Object *win = NULL;
static Evas_Object *rect = NULL;

struct info
{
   double x, y;
};

static void *my_thread_mainloop_code(void *data);

static pthread_t thread_id;

// BEGIN - code running in my custom pthread instance
//
static void *
my_thread_run(void *arg)
{
   double t = 0.0;

   for (;;)
     {
        struct info *inf = malloc(sizeof(struct info));

        if (inf)
          {
             inf->x = 200 + (200 * sin(t));
             inf->y = 200 + (200 * cos(t));
             ecore_main_loop_thread_safe_call_sync
                (my_thread_mainloop_code, inf);
          }
        // and sleep and loop
        usleep(1000);
        t += 0.02;
     }
   return NULL;
}
//
// END - code running in my custom pthread instance
static void
my_thread_new(void)
{
   pthread_attr_t attr;

   if (pthread_attr_init(&attr) != 0)
     perror("pthread_attr_init");
   if (pthread_create(&thread_id, &attr, my_thread_run, NULL) != 0)
     perror("pthread_create");
}

static void *
my_thread_mainloop_code(void *data)
{
   struct info *inf = data;
   evas_object_move(rect, inf->x - 50, inf->y - 50);
   free(inf);
   return NULL;
}

// on window delete - cancel thread then delete window and exit mainloop
static void
del(void *data, Evas_Object *obj, void *event_info)
{
   exit(0);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *o;

   win = elm_win_util_standard_add("efl-thread-2", "EFL Thread 2");
   evas_object_smart_callback_add(win, "delete,request", del, NULL);

   o = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(o, 50, 80, 180, 255);
   evas_object_resize(o, 100, 100);
   evas_object_show(o);
   rect = o;

   // create custom thread to do some "work on the side"
   my_thread_new();

   evas_object_resize(win, 400, 400);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
