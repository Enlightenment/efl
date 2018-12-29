//Compile with:
// gcc -o emotion_basic_example emotion_basic_example.c `pkg-config --libs --cflags emotion evas ecore ecore-evas eo`

#ifndef EFL_EO_API_SUPPORT
# define EFL_EO_API_SUPPORT
#endif
#ifndef EFL_BETA_API_SUPPORT
# define EFL_BETA_API_SUPPORT
#endif
#include <Eo.h>

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Evas.h>
#include <Emotion.h>
#include <stdio.h>

#define WIDTH  (320)
#define HEIGHT (240)

static void
_playback_started_cb(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
    printf("Emotion object started playback.\n");
}

int
main(int argc, const char *argv[])
{
   Ecore_Evas *ee;
   Evas *e;
   Evas_Object *bg, *em;
   const char *filename = NULL;

   if (argc < 2)
     {
	printf("One argument is necessary. Usage:\n");
	printf("\t%s <filename>\n", argv[0]);
     }

   filename = argv[1];

   if (!ecore_evas_init())
     return EXIT_FAILURE;

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   ee = ecore_evas_new(NULL, 10, 10, WIDTH, HEIGHT, NULL);
   if (!ee)
     goto error;

   ecore_evas_show(ee);

   /* the canvas pointer, de facto */
   e = ecore_evas_get(ee);

   /* adding a background to this example */
   bg = evas_object_rectangle_add(e);
   evas_object_name_set(bg, "our dear rectangle");
   evas_object_color_set(bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(bg, 0, 0); /* at canvas' origin */
   evas_object_resize(bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(bg);

   /* Creating the emotion object */
   em = emotion_object_add(e);
   emotion_object_init(em, NULL);

   efl_event_callback_add
     (em, EFL_CANVAS_VIDEO_EVENT_PLAYBACK_START, _playback_started_cb, NULL);

   emotion_object_file_set(em, filename);

   evas_object_move(em, 0, 0);
   evas_object_resize(em, WIDTH, HEIGHT);
   evas_object_show(em);

   emotion_object_play_set(em, EINA_TRUE);

   ecore_main_loop_begin();

   ecore_evas_free(ee);
   ecore_evas_shutdown();
   return 0;

error:
   fprintf(stderr, "error: Requires at least one Evas engine built and linked"
                   " to ecore-evas for this example to run properly.\n");

   ecore_evas_shutdown();
   return -1;
}
