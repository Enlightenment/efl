#include "edje_thumb.h"

static void save_pixels(int *pixels, int w, int h, char *out);
static void args_parse(void);
static void help_show(void);
static int signal_exit(void *data, int ev_type, void *ev);
static int frame_grab(void *data);

Ecore_Evas *ee = NULL;
Evas *evas = NULL;
Evas_Object *edje = NULL;
char *file = NULL;
char *group = NULL;
char *outfile = NULL;
double fps = 10.0;
int frames = 0;
int w = 640;
int h = 480;
int outw = 160;
int outh = 120;
int frnum = 0;

int
main(int argc, char **argv)
{
   if (!ecore_init()) return -1;
   ecore_app_args_set(argc, (const char **)argv);
   ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, signal_exit, NULL);
   if (!ecore_evas_init()) return -1;
   if (!edje_init()) return -1;

   args_parse();
   
   ee = ecore_evas_buffer_new(w, h);
   if (!ee)
     {
	printf("Cannot create buffer canvas! ERROR!\n");
	exit(-1);
     }
   evas = ecore_evas_get(ee);
   
   edje = edje_object_add(evas);
   if (!edje_object_file_set(edje, file, group))
     {
	printf("Cannot load file %s, group %s\n", file, group);
	exit(-1);
     }
   evas_object_move(edje, 0, 0);
   evas_object_resize(edje, w, h);
   evas_object_show(edje);
   edje_message_signal_process();
   
   if (frames > 0)
     {
	frame_grab(NULL);
	ecore_timer_add(1.0 / fps, frame_grab, NULL);
	ecore_main_loop_begin();
     }
   else
     {
	int *pixels;

	pixels = ecore_evas_buffer_pixels_get(ee);
	save_pixels(pixels, w, h, outfile);
     }
   
   evas_object_del(edje);
   ecore_evas_free(ee);   
   edje_shutdown();
   ecore_evas_shutdown();
   ecore_shutdown();
   return 0;
}

static void
save_pixels(int *pixels, int w, int h, char *out)
{
   Imlib_Image im;
   
   im = imlib_create_image_using_data(w, h, pixels);
   imlib_context_set_image(im);
   imlib_image_set_irrelevant_alpha(0);
   imlib_image_set_has_alpha(1);
   if ((w != outw) || (h != outh))
     {
	Imlib_Image im2;
	
	im2 = imlib_create_cropped_scaled_image(0, 0, w, h, outw, outh);
	imlib_free_image();
	imlib_context_set_image(im2);
     }
   imlib_save_image(out);
   imlib_free_image();
}

static void
args_parse(void)
{
   int argc;
   char **argv;
   int i;
   
   ecore_app_args_get(&argc, &argv);
   for (i = 1; i < argc; i++)
     {
	if ((!strcmp(argv[i], "-h")) ||
	    (!strcmp(argv[i], "-help")) ||
	    (!strcmp(argv[i], "--help")))
	  {
	     help_show();
	     exit(-1);
	  }
	else if (((!strcmp(argv[i], "-g")) ||
		  (!strcmp(argv[i], "-geometry")) ||
		  (!strcmp(argv[i], "--geometry"))) && (i < (argc - 1)))
	  {
	     int n;
	     char buf[16], buf2[16];
	     
	     n = sscanf(argv[i +1], "%10[^x]x%10s", buf, buf2);
	     if (n == 2)
	       {
		  w = atoi(buf);
		  h = atoi(buf2);
	       }
	     i++;
	  }
	else if (((!strcmp(argv[i], "-og")) ||
		  (!strcmp(argv[i], "-out-geometry")) ||
		  (!strcmp(argv[i], "--out-geometry"))) && (i < (argc - 1)))
	  {
	     int n;
	     char buf[16], buf2[16];
	     
	     n = sscanf(argv[i +1], "%10[^x]x%10s", buf, buf2);
	     if (n == 2)
	       {
		  outw = atoi(buf);
		  outh = atoi(buf2);
	       }
	     i++;
	  }
	else if ((!strcmp(argv[i], "-fps")) && (i < (argc - 1)))
	  {
	     fps = atof(argv[i + 1]);
	     if (fps < 0.1) fps = 0.1;
	     i++;
	  }
	else if ((!strcmp(argv[i], "-n")) && (i < (argc - 1)))
	  {
	     frames = atoi(argv[i + 1]);
	     if (frames < 1) frames = 1;
	     i++;
	  }
	else if (!file)
	  {
	     file = argv[i];
	  }
	else if (!group)
	  {
	     group = argv[i];
	  }
	else if (!outfile)
	  {
	     outfile = argv[i];
	  }
     }
   if ((!file) || (!group) || (!outfile))
     {
	help_show();
	exit(-1);
     }
}

static void
help_show(void)
{
   printf("Usage: edje_thumb INPUT_EDJE GROUP_TO_LOAD OUT_FILE [OPTIONS]\n"
	  "\n"
	  "Where required parameters are:\n"
	  "  INPUT_EDJE    the edje file to look at\n"
	  "  GROUP_TO_LOAD the group name in the edge\n"
	  "  OUT_FILE      the output file or format (for multiple frames include a %%i format to accept the number of the frame - same as printf)\n"
	  "\n"
	  "Where optional OPTIONS are:\n"
	  "  -h          this help\n"
	  "  -g WxH      rendering geometry\n"
	  "  -og WxH     output file geometry\n"
	  "  -fps N      frames per second if capturing an animation\n"
	  "  -n NUM      number of frames to capture if capturing an animation\n"
	  );
}

static int
signal_exit(void *data, int ev_type, void *ev)
{
   ecore_main_loop_quit();
   return 1;
}

static int
frame_grab(void *data)
{
   char buf[4096];
   int *pixels;
   
   pixels = ecore_evas_buffer_pixels_get(ee);
   snprintf(buf, sizeof(buf), outfile, frnum);
   save_pixels(pixels, w, h, buf);
   frnum++;
   if (frnum == frames)
     {
	ecore_main_loop_quit();
	return 0;
     }
   return 1;
}
