#include "edje.h"

double       start_time = 0.0;
Ecore_Evas  *ecore_evas = NULL;
Evas        *evas       = NULL;

static int  main_start(int argc, char **argv);
static void main_stop(void);
static void main_resize(Ecore_Evas *ee);
static int  main_signal_exit(void *data, int ev_type, void *ev);
static void main_delete_request(Ecore_Evas *ee);

static int
main_start(int argc, char **argv)
{
   start_time = ecore_time_get();
   if (!ecore_init()) return -1;
   ecore_app_args_set(argc, (const char **)argv);
   ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, main_signal_exit, NULL);
   if (!ecore_evas_init()) return -1;
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
     ecore_evas = ecore_evas_software_x11_new(NULL, 0,  0, 0, 240, 320);
   if (!ecore_evas) return -1;
   ecore_evas_callback_delete_request_set(ecore_evas, main_delete_request);
   ecore_evas_callback_resize_set(ecore_evas, main_resize);
   ecore_evas_title_set(ecore_evas, "Evoak");
   ecore_evas_name_class_set(ecore_evas, "evoak", "main");
   ecore_evas_show(ecore_evas);
   evas = ecore_evas_get(ecore_evas);
   evas_image_cache_set(evas, 8192 * 1024);
   evas_font_cache_set(evas, 512 * 1024);
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
   // FIXME: do something for resize
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

int
main(int argc, char **argv)
{   
   if (main_start(argc, argv) < 1) return -1;
   
   ecore_main_loop_begin();
   
   main_stop();
   
   return 0;
}

