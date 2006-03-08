#include "ecore_evas_test.h"
   
#ifdef BUILD_ECORE_EVAS

double       start_time = 0.0;
Ecore_Evas  *ee         = NULL;
Evas        *evas       = NULL;

Ecore_Idle_Enterer *getpix_handler = NULL;

static void app_resize(Ecore_Evas *ee);
static int app_signal_exit(void *data, int ev_type, void *ev);
static void app_delete_request(Ecore_Evas *ee);

static int
getpix_cb(void *data)
{
   const int *pix;
   int p;
   int w, h;
   FILE *f;
   int x, y;
   static int count = 0;
   char buf[256];
   unsigned char *line;
   
   pix = ecore_evas_buffer_pixels_get(ee);
   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   snprintf(buf, sizeof(buf), "out%04i.ppm", count);
   count++;
   f = fopen(buf, "wb");
   if (f)
     {
	line = malloc(w * 4);
	fprintf(f, "P6\n%i %i\n255\n", w, h);
	for (y = 0; y < h; y++)
	  {
	     for (x = 0; x < w; x++)
	       {
		  p = pix[(y * w) + x];
		  line[(x * 3) + 0] = ((p >> 16) & 0xff);
		  line[(x * 3) + 1] = ((p >> 8 ) & 0xff);
		  line[(x * 3) + 2] = ((p      ) & 0xff);
/*		  line[(x * 3) + 3] = ((p >> 24) & 0xff);*/
	       }
	     fwrite(line, w * 3, 1, f);
	  }
	free(line);
	fclose(f);
     }
   return 1;
}

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
     {
	ee = ecore_evas_fb_new(NULL, 0,  240, 320);
	evas = ecore_evas_get(ee);
     }
   else if ((argc > 1) && (!strcmp(argv[1], "-x")))
     {
	ee = ecore_evas_software_x11_new(NULL, 0,  0, 0, 240, 320);
	evas = ecore_evas_get(ee);
     }
   else if ((argc > 1) && (!strcmp(argv[1], "-xr")))
     {
	ee = ecore_evas_xrender_x11_new(NULL, 0,  0, 0, 240, 320);
	evas = ecore_evas_get(ee);
     }
#if HAVE_ECORE_EVAS_GL
   else if ((argc > 1) && (!strcmp(argv[1], "-gl")))
     {
	ee = ecore_evas_gl_x11_new(NULL, 0,  0, 0, 240, 320);
	evas = ecore_evas_get(ee);
     }
#endif
#if HAVE_ECORE_EVAS_DIRECTFB
   else if ((argc > 1) && (!strcmp(argv[1], "-dfb")))
     {
	ee = ecore_evas_directfb_new(NULL, 0,  0, 0, 240, 320);
	evas = ecore_evas_get(ee);
     }
#endif
   else if ((argc > 1) && (!strcmp(argv[1], "-buf")))
     {
	ee = ecore_evas_buffer_new(240, 320);
	evas = ecore_evas_get(ee);
	getpix_handler = ecore_idle_enterer_add(getpix_cb, NULL);
     }
   else if ((argc > 1) && (!strcmp(argv[1], "-buf2")))
     {
	Evas_Object *o;
	
	ee = ecore_evas_software_x11_new(NULL, 0,  0, 0, 240, 320);
	
	o = evas_object_rectangle_add(ecore_evas_get(ee));
	evas_object_move(o, 0, 0);
	evas_object_resize(o, 240, 320);
	evas_object_color_set(o, 150, 200, 250, 255);
	evas_object_show(o);
	
	o = ecore_evas_object_image_new(ee);
	evas = ecore_evas_get(evas_object_data_get(o, "Ecore_Evas"));
	evas_object_move(o, 50, 50);
	evas_object_resize(o, 120, 160);
	evas_object_image_fill_set(o, 0, 0, 120, 160);
	evas_object_image_size_set(o, 240, 320);
	ecore_evas_resize(evas_object_data_get(o, "Ecore_Evas"), 240, 320);
	evas_object_color_set(o, 255, 255, 255, 200);
	evas_object_show(o);
     }
   else if ((argc > 1) && (!strcmp(argv[1], "-h")))
     {
	printf("%s -x         Test ecore_evas in X (default)\n"
	       "%s -xr        Test ecore_evas in XRender\n"
	       "%s -gl        Test ecore_evas in X GL\n"
	       "%s -dfb       Test ecore_evas in DirectFB\n"
	       "%s -fb        Test ecore_evas in the Framebuffer\n"
	       "%s -buf       Test ecore_evas in the Buffer\n"
	       "%s -buf2      Test ecore_evas in the Image Buffer\n"
	       "%s -h         Display this help\n",
	       argv[0], argv[0], argv[0], argv[0], argv[0], argv[0], argv[0]);
	ecore_evas_shutdown();
	ecore_shutdown();
	return 0;
     }
   else {
#ifdef BUILD_ECORE_X
	ee = ecore_evas_software_x11_new(NULL, 0,  0, 0, 240, 320);	 
#else
#ifdef BUILD_ECORE_FB
	ee = ecore_evas_fb_new(NULL, 270,  240, 320);
#endif
#endif   
	evas = ecore_evas_get(ee);
     }
   if (!ee) return -1;
   ecore_evas_callback_delete_request_set(ee, app_delete_request);
   ecore_evas_callback_resize_set(ee, app_resize);
   ecore_evas_title_set(ee, "Ecore Evas Test");
   ecore_evas_name_class_set(ee, "ecore_test", "test_evas");   
   ecore_evas_show(ee);
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
   Evas_Coord w, h;
   
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
