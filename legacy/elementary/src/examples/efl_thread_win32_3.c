//Compile with:
//gcc -o efl_thread_3 efl_thread_win32_3.c -g `pkg-config --cflags --libs elementary`
#include <Elementary.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static Evas_Object *win = NULL;
static Evas_Object *rect = NULL;

struct info
{
   double x, y;
};

static void my_thread_mainloop_code(void *data);

static HANDLE thread;

// BEGIN - code running in my custom win32 thread instance
//
static DWORD WINAPI
my_thread_run(LPVOID arg)
{
   double t = 0.0;

   // inside the thread function lets loop forever incrementing a time point
   for (;;)
     {
        struct info *inf = malloc(sizeof(struct info));

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
     }
   return NULL;
}
//
// END - code running in my custom win32 thread instance
static void
my_thread_new(void)
{
   thread = CreateThread(NULL, 0, my_thread_run, NULL, 0, NULL);
   if (!thread)
     {
        char *str = evil_last_error_get();
        if (str)
          {
             fprintf("thread creation failed: %s\n", str);
             free(str);
          }
     }
}

static void
my_thread_mainloop_code(void *data)
{
   struct info *inf = data;
   evas_object_move(rect, inf->x - 50, inf->y - 50);
   free(inf);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *o;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("efl-thread-3", "EFL Thread 3");
   elm_win_autodel_set(win, EINA_TRUE);

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
