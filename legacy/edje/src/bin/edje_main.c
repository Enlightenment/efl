#include "edje.h"

static int  main_start(int argc, char **argv);
static void main_stop(void);
static void main_resize(Ecore_Evas *ee);
static int  main_signal_exit(void *data, int ev_type, void *ev);
static void main_delete_request(Ecore_Evas *ee);

void bg_setup(void);
void bg_resize(double w, double h);

void test_setup(char *file, char *name);
void test_reize(double w, double h);
    
double       start_time = 0.0;
Ecore_Evas  *ecore_evas = NULL;
Evas        *evas       = NULL;

static int
main_start(int argc, char **argv)
{
   start_time = ecore_time_get();
   if (!ecore_init()) return -1;
   ecore_app_args_set(argc, (const char **)argv);
   ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, main_signal_exit, NULL);
   if (!ecore_evas_init()) return -1;
#if 0   
   if ((argc > 1) && (!strcmp(argv[1], "-fb")))
     ecore_evas = ecore_evas_fb_new(NULL, 0,  240, 320);
   else if ((argc > 1) && (!strcmp(argv[1], "-x")))
     ecore_evas = ecore_evas_software_x11_new(NULL, 0,  0, 0, 240, 320);
   else if ((argc > 1) && (!strcmp(argv[1], "-h")))
     {
	printf("%s -x         Run in X (default)\n"
	       "%s -fb        Run in the Framebuffer\n"
	       "%s -h         Display this help\n",
	       argv[0], argv[0], argv[0]);
	ecore_evas_shutdown();
	ecore_shutdown();
	return 0;
     }
   else
#endif     
     ecore_evas = ecore_evas_software_x11_new(NULL, 0,  0, 0, 240, 320);
   if (!ecore_evas) return -1;
   ecore_evas_callback_delete_request_set(ecore_evas, main_delete_request);
   ecore_evas_callback_resize_set(ecore_evas, main_resize);
   ecore_evas_title_set(ecore_evas, "Edje Test Program");
   ecore_evas_name_class_set(ecore_evas, "edje", "main");
   ecore_evas_show(ecore_evas);
   evas = ecore_evas_get(ecore_evas);
   evas_image_cache_set(evas, 1024 * 1024);
   evas_font_cache_set(evas, 256 * 1024);
//   evas_font_path_append(evas, FN);
   return 1;
}

static void
main_stop(void)
{
   ecore_evas_shutdown();
   ecore_shutdown();
}

static void
main_resize(Ecore_Evas *ee)
{
   double w, h;
   
   evas_output_viewport_get(evas, NULL, NULL, &w, &h);
   bg_resize(w, h);
   test_reize(w, h);
}

static int
main_signal_exit(void *data, int ev_type, void *ev)
{
   ecore_main_loop_quit();
   return 1;
}

static void
main_delete_request(Ecore_Evas *ee)
{
   ecore_main_loop_quit();
}

static Evas_Object *o_bg = NULL;

void
bg_setup(void)
{
   Evas_Object *o;
   
   o = evas_object_rectangle_add(evas);
   evas_object_move(o, 0, 0);
   evas_object_resize(o, 240, 320);
   evas_object_layer_set(o, -999);
   evas_object_color_set(o, 240, 240, 240, 255);
   evas_object_show(o);   
   o_bg = o;
}

void
bg_resize(double w, double h)
{
   evas_object_resize(o_bg, w, h);
}

static Evas_Object *o_edje = NULL;

static void
cb (void *data, Evas_Object *o, const char *sig, const char *src)
{
   printf("CALLBACK for %p %p \"%s\" \"%s\"\n", data, o, sig, src);
}

void
test_setup(char *file, char *name)
{
   Evas_Object *o;
   
   o = edje_add(evas);
   edje_file_set(o, file, name);
   edje_signal_callback_add(o, "do_it", "the_source", cb, NULL);
   edje_signal_callback_add(o, "mouse,*", "logo", cb, NULL);
   evas_object_move(o, 10, 10);
   evas_object_resize(o, 220, 300);
   evas_object_show(o);
   o_edje = o;
}

void
test_reize(double w, double h)
{
   evas_object_move(o_edje, 10, 10);   
   evas_object_resize(o_edje, w - 20, h - 20);
}

int
main(int argc, char **argv)
{
   char *file;
   char *coll;
   
   if (main_start(argc, argv) < 1) return -1;

   edje_init();
   edje_frametime_set(1.0 / 60.0); /* 60 fps */
   
   bg_setup();

   if (argc < 2)
     {
	printf("Usage: %s file_to_show.eet [collection_to_show]\n", argv[0]);
	printf("       The default collection name to show is \"test\"\n");
	exit(-1);
     }
   file = "./data/e_logo.eet";
   coll = "test";
   file = argv[1];
   if (argc >= 3) coll = argv[2];
   /* FIXME: list collections */
   test_setup(file, coll);
   
   ecore_main_loop_begin();
   
   main_stop();
   
   return 0;
}

