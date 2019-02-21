#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#include "perf.h"
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
static Evas *evas;
static Evas_Object *win, *bg;
static double total_time = 0.0;
static int total_frames = 0;
///////////////////////////////////////////////////////////////////////////////

typedef struct
{
   void        (*init) (Evas *e);
   void        (*tick) (Evas *e, double pos, Evas_Coord win_w, Evas_Coord win_h);
   const char   *desc;
   double        weight;
} Test;

static Eina_List *cleanup_list = NULL;

void
cleanup_add(Evas_Object *o)
{
   cleanup_list = eina_list_append(cleanup_list, o);
}

#define T2
#include "perf_list.c"
#undef T2

#define T1
static Test tests[] = {
#define TFUN(x) test_ ## x ## _init, test_ ## x ## _tick
#include "perf_list.c"
   { NULL, NULL, NULL, 0.0 }
};
#undef T1

static unsigned int test_pos = 0;
static double time_start = 0.0;
static double anim_tick_delta_total = 0.0;
static int anim_tick_total = 0;
static Eina_Array *tests_to_do = NULL;
static double tests_fps = 0.0;
static double tests_weights = 0.0;
static double run_time = 5.0;
static double spin_up_delay = 2.0;

static void all_tests(Evas *e);

static Eina_Bool
next_test_delay(void *data EINA_UNUSED)
{
   all_tests(data);
   return EINA_FALSE;
}

#define ANIMATOR 1

#ifdef ANIMATOR
static Ecore_Animator *animator = NULL;

static Eina_Bool
anim_tick(void *data)
#else
static void
anim_tick(void *data, const Efl_Event *event EINA_UNUSED)
#endif
{
   Evas_Coord win_w, win_h;
   double f = ecore_time_get() - time_start;
   static double pf = 0.0;
   int p;

   if (total_frames == 1) time_start = ecore_time_get();
   if (anim_tick_total == 1)
     {
        anim_tick_delta_total = 0.0;
        pf  = f;
     }
   else
     {
        anim_tick_delta_total += (f - pf);
     }
   anim_tick_total++;
   pf = f;
   f = f / run_time; // time per test - 5sec.
   p = (int)(uintptr_t)eina_array_data_get(tests_to_do, test_pos) - 1;
   evas_output_viewport_get(data, NULL, NULL, &win_w, &win_h);
   tests[p].tick(data, f, win_w, win_h);
   if (f >= 1.0)
     {
        Evas_Object *o;
        double time_spent = ecore_time_get() - time_start;
        double load = total_time / time_spent;

        // only got 1 frame rendered? eek. just assume we got one
        if (total_frames < 2) total_frames = 2;
        if (anim_tick_total < 2) anim_tick_total = 2;
        if ((load <= 0.0) || (anim_tick_delta_total <= 0.0) ||
           (run_time <= 0))
          {
             printf("?? | %s\n", tests[p].desc);
          }
        else
          {
             printf("%1.2f (fr=%i load=%1.5f tick=%i@%1.2fHz) | %1.2f %s\n",
                    (double)(total_frames - 2) / (load * run_time),
                    total_frames - 2,
                    load,
                    anim_tick_total - 2,
                    (double)(anim_tick_total - 2) / anim_tick_delta_total,
                    tests[p].weight,
                    tests[p].desc);
             tests_fps += ((double)(total_frames - 2) / (load * run_time)) *
               tests[p].weight;
             tests_weights += tests[p].weight;
          }
        total_frames = 0.0;
        total_time = 0.0;
        EINA_LIST_FREE(cleanup_list, o) evas_object_del(o);
        test_pos++;
#ifdef ANIMATOR
        ecore_animator_del(animator);
        animator = NULL;
#else
        efl_event_callback_del(win, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, anim_tick, data);
#endif
        ecore_timer_add(0.5, next_test_delay, data);
     }
#ifdef ANIMATOR
   return EINA_TRUE;
#endif
}

static Eina_Bool
exit_delay(void *data EINA_UNUSED)
{
   elm_exit();
   return EINA_FALSE;
}

static void
all_tests(Evas *e)
{
   Evas_Coord win_w, win_h;
   int p;

   evas_output_viewport_get(e, NULL, NULL, &win_w, &win_h);
   if (test_pos >= eina_array_count_get(tests_to_do))
     {
        printf("--------------------------------------------------------------------------------\n");
        printf("Average weighted FPS: %1.2f\n", tests_fps / tests_weights);
        printf("--------------------------------------------------------------------------------\n");
        ecore_timer_add(1.0, exit_delay, NULL);
        return;
     }
   p = (int)(uintptr_t)eina_array_data_get(tests_to_do, test_pos) - 1;
   tests[p].init(e);
   tests[p].tick(e, 0.0, win_h, win_h);
   time_start = ecore_time_get();
   anim_tick_delta_total = 0.0;
   anim_tick_total = 0;
#ifdef ANIMATOR
   animator = ecore_animator_add(anim_tick, e);
#else
   efl_event_callback_add(win, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK, anim_tick, e);
#endif
}

static Eina_Bool
all_tests_delay(void *data)
{
   all_tests(data);
   return EINA_FALSE;
}

static double rtime = 0.0;

static void
render_pre(void *data EINA_UNUSED, Evas *e EINA_UNUSED, void *info EINA_UNUSED)
{
   rtime = ecore_time_get();
}

static void
render_post(void *data EINA_UNUSED, Evas *e EINA_UNUSED, void *info EINA_UNUSED)
{
   double spent = ecore_time_get() - rtime;
   if (total_frames == 2) total_time = 0.0;
   total_time += spent;
   total_frames++;
}

static Eina_Bool
_spincpu_up_idler(void *data EINA_UNUSED)
{
   return EINA_TRUE;
}

///////////////////////////////////////////////////////////////////////////////
EAPI int
elm_main(int argc, char **argv)
{
   int i, j;

   for (i = 1; i < argc; i++)
     {
        if ((!strcmp(argv[i], "--help")) ||
            (!strcmp(argv[i], "-help")) ||
            (!strcmp(argv[i], "-h")))
          {
             printf("Usage:\n"
                    "  -h              : This help\n"
                    "  -l              : List all tests\n"
                    "  -t N            : Run test number N\n"
                    "  -r N            : Run each test for N seconds\n"
                    "  -d N            : Initial spin-up delay\n"
                    "\n");
             elm_exit();
             return 1;
          }
        else if (!strcmp(argv[i], "-l"))
          {
             for (j = 0; tests[j].init; j++)
               {
                  printf("  %3i | %s\n", j, tests[j].desc);
               }
             elm_exit();
             return 1;
          }
        else if ((!strcmp(argv[i], "-t")) && (i < (argc - 1)))
          {
             i++;
             if (!tests_to_do) tests_to_do = eina_array_new(32);
             eina_array_push(tests_to_do, (void *)(uintptr_t)atoi(argv[i]));
          }
        else if ((!strcmp(argv[i], "-r")) && (i < (argc - 1)))
          {
             i++;
             run_time = atof(argv[i]);
          }
        else if ((!strcmp(argv[i], "-d")) && (i < (argc - 1)))
          {
             i++;
             spin_up_delay = atof(argv[i]);
          }
     }
   if (!tests_to_do)
     {
        tests_to_do = eina_array_new(32);
        for (j = 0; tests[j].init; j++)
          {
             eina_array_push(tests_to_do, (void *)(uintptr_t)(j + 1));
          }
     }
   elm_app_compile_bin_dir_set(PACKAGE_BIN_DIR);
   elm_app_compile_lib_dir_set(PACKAGE_LIB_DIR);
   elm_app_compile_data_dir_set(PACKAGE_DATA_DIR);
   elm_app_info_set(elm_main, "elementary", "images/logo.png");

   win = elm_win_add(NULL, "main", ELM_WIN_BASIC);
   if (!win)
     {
        elm_exit();
        return 1;
     }
   evas = evas_object_evas_get(win);
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_title_set(win, "Elementary Performace Test");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   bg = evas_object_rectangle_add(evas);
   evas_object_color_set(bg, 128, 128, 128, 255);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   evas_event_callback_add(evas, EVAS_CALLBACK_RENDER_PRE, render_pre, NULL);
   evas_event_callback_add(evas, EVAS_CALLBACK_RENDER_FLUSH_POST, render_post, NULL);

   ecore_idler_add(_spincpu_up_idler, NULL);
   printf("--------------------------------------------------------------------------------\n");
   printf("Performance Test Engine: %s\n",
          ecore_evas_engine_name_get(ecore_evas_ecore_evas_get(evas)));
   printf("--------------------------------------------------------------------------------\n");
   ecore_timer_add(spin_up_delay, all_tests_delay, evas);

   evas_object_resize(win, 800, 800);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
///////////////////////////////////////////////////////////////////////////////
