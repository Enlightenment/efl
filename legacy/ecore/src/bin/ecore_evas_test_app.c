#include "ecore_evas_test.h"
   
#ifdef BUILD_ECORE_EVAS

double       start_time = 0.0;
Ecore_Evas  *ee         = NULL;
Evas        *evas       = NULL;

static void app_resize(Ecore_Evas *ee);
static int app_signal_exit(void *data, int ev_type, void *ev);
static void app_delete_request(Ecore_Evas *ee);

int
app_start(int argc, const char **argv)
{
   /* init the app */
   start_time = ecore_time_get();
   ecore_init();
   ecore_app_args_set(argc, argv);
   ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, app_signal_exit, NULL);
   
   /* create an evas */
   if (!ecore_evas_init()) return -1;
   if ((argc > 1) && (!strcmp(argv[1], "-fb")))
     ee = ecore_evas_fb_new(NULL, 0,  240, 320);
   else if ((argc > 1) && (!strcmp(argv[1], "-x")))
     ee = ecore_evas_software_x11_new(NULL, 0,  0, 0, 240, 320);
   else if ((argc > 1) && (!strcmp(argv[1], "-gl")))
     ee = ecore_evas_gl_x11_new(NULL, 0,  0, 0, 240, 320);
   else if ((argc > 1) && (!strcmp(argv[1], "-h")))
     {
	printf("%s -x         Test ecore_evas in X (default)\n"
	       "%s -gl        Test ecore_evas in X GL\n"
	       "%s -fb        Test ecore_evas in the Framebuffer\n"
	       "%s -h         Display this help\n",
	       argv[0], argv[0], argv[0], argv[0]);
	ecore_evas_shutdown();
	ecore_shutdown();
	return 0;
     }
   else
#ifdef BUILD_ECORE_X
     ee = ecore_evas_software_x11_new(NULL, 0,  0, 0, 240, 320);	 
#else
#ifdef BUILD_ECORE_FB
     ee = ecore_evas_fb_new(NULL, 270,  240, 320);
#endif   
#endif   
   if (!ee) return -1;
   ecore_evas_callback_delete_request_set(ee, app_delete_request);
   ecore_evas_callback_resize_set(ee, app_resize);
   ecore_evas_title_set(ee, "Ecore Evas Test");
   ecore_evas_name_class_set(ee, "ecore_test", "test_evas");   
   ecore_evas_show(ee);
   evas = ecore_evas_get(ee);
   evas_image_cache_set(evas, 8192 * 1024);
   evas_font_cache_set(evas, 512 * 1024);
   evas_font_path_append(evas, FN);
   return 1;
}

void
app_finish(void)
{
   ecore_evas_shutdown();
   ecore_shutdown();
}

static void
app_resize(Ecore_Evas *ee)
{
   double w, h;
   
   evas_output_viewport_get(evas, NULL, NULL, &w, &h);
   bg_resize(w, h);
}

static int
app_signal_exit(void *data, int ev_type, void *ev)
{
   ecore_main_loop_quit();
   return 1;
}

static void
app_delete_request(Ecore_Evas *ee)
{
   ecore_main_loop_quit();
}

#endif
