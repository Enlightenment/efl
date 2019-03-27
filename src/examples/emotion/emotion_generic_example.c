//Compile with:
// gcc -o emotion_generic_example emotion_generic_example.c `pkg-config --libs --cflags emotion evas ecore ecore-evas eina eo`
#ifndef EFL_BETA_API_SUPPORT
# define EFL_BETA_API_SUPPORT
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Evas.h>
#include <Emotion.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define WIDTH  (320)
#define HEIGHT (240)

static Eina_List *filenames = NULL;
static Eina_List *curfile = NULL;

static void
_playback_started_cb(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
    printf("Emotion object started playback.\n");
}

static void
_playback_stopped_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   printf("Emotion playback stopped.\n");
   emotion_object_play_set(ev->object, EINA_FALSE);
   emotion_object_position_set(ev->object, 0);
}

static Evas_Object *
_create_emotion_object(Evas *e)
{
   Evas_Object *em = emotion_object_add(e);

   emotion_object_init(em, "generic");

   efl_event_callback_add
     (em, EFL_CANVAS_VIDEO_EVENT_PLAYBACK_START, _playback_started_cb, NULL);
   efl_event_callback_add
     (em, EFL_CANVAS_VIDEO_EVENT_PLAYBACK_STOP, _playback_stopped_cb, NULL);

   return em;
}

static void
_on_key_down(void *data, Evas *e EINA_UNUSED, Evas_Object *o EINA_UNUSED, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   Evas_Object *em = data;

   if (!strcmp(ev->key, "Return"))
     {
	emotion_object_play_set(em, EINA_TRUE);
     }
   else if (!strcmp(ev->key, "space"))
     {
	emotion_object_play_set(em, EINA_FALSE);
     }
   else if (!strcmp(ev->key, "Escape"))
     {
	ecore_main_loop_quit();
     }
   else if (!strcmp(ev->key, "t"))
     {
	int w, h;
	emotion_object_size_get(em, &w, &h);
	fprintf(stderr, "example -> size: %dx%d\n", w, h);
     }
   else if (!strcmp(ev->key, "s"))
     {
        float len, pos;
        len = emotion_object_play_length_get(em);
        pos = 0.98 * len;
	fprintf(stderr, "skipping to position %0.3f\n", pos);
	emotion_object_position_set(em, pos);
     }
   else if (!strcmp(ev->key, "1"))
     {
	fprintf(stderr, "setting speed to 1.0\n");
	emotion_object_play_speed_set(em, 1.0);
     }
   else if (!strcmp(ev->key, "2"))
     {
	fprintf(stderr, "setting speed to 2.0\n");
	emotion_object_play_speed_set(em, 2.0);
     }
   else if (!strcmp(ev->key, "n"))
     {
	const char *file;
	if (!curfile)
	  curfile = filenames;
	else
	  curfile = eina_list_next(curfile);
	file = eina_list_data_get(curfile);
	fprintf(stderr, "playing next file: %s\n", file);
	emotion_object_file_set(em, file);
     }
   else if (!strcmp(ev->key, "p"))
     {
	const char *file;
	if (!curfile)
	  curfile = eina_list_last(filenames);
	else
	  curfile = eina_list_prev(curfile);
	file = eina_list_data_get(curfile);
	fprintf(stderr, "playing next file: %s\n", file);
	emotion_object_file_set(em, file);
     }
   else if (!strcmp(ev->key, "d"))
     {
	evas_object_del(em);
     }
   else if (!strcmp(ev->key, "l"))
     {
	// force frame dropping
	sleep(5);
     }
   else
     {
	fprintf(stderr, "unhandled key: %s\n", ev->key);
     }
}

static void
_frame_decode_cb(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   // fprintf(stderr, "smartcb: frame_decode\n");
}

static void
_length_change_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   fprintf(stderr, "smartcb: length_change: %0.3f\n", emotion_object_play_length_get(ev->object));
}

static void
_position_update_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   fprintf(stderr, "smartcb: position_update: %0.3f\n", emotion_object_position_get(ev->object));
}

static void
_progress_change_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   fprintf(stderr, "smartcb: progress_change: %0.3f, %s\n",
	   emotion_object_progress_status_get(ev->object),
	   emotion_object_progress_info_get(ev->object));
}

EFL_CALLBACKS_ARRAY_DEFINE(emotion_object_example_callbacks,
       { EFL_CANVAS_VIDEO_EVENT_FRAME_DECODE, _frame_decode_cb },
       { EFL_CANVAS_VIDEO_EVENT_LENGTH_CHANGE, _length_change_cb },
       { EFL_CANVAS_VIDEO_EVENT_POSITION_CHANGE, _position_update_cb },
       { EFL_CANVAS_VIDEO_EVENT_PROGRESS_CHANGE, _progress_change_cb });

int
main(int argc, const char *argv[])
{
   Ecore_Evas *ee;
   Evas *e;
   Evas_Object *bg, *em;
   int i;

   if (argc < 2)
     {
	printf("One argument is necessary. Usage:\n");
	printf("\t%s <filename>\n", argv[0]);
     }

   eina_init();
   for (i = 1; i < argc; i++)
     filenames = eina_list_append(filenames, eina_stringshare_add(argv[i]));

   curfile = filenames;

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
   em = _create_emotion_object(e);
   emotion_object_file_set(em, eina_list_data_get(curfile));
   evas_object_move(em, 0, 0);
   evas_object_resize(em, WIDTH, HEIGHT);
   evas_object_show(em);

   efl_event_callback_array_add(em, emotion_object_example_callbacks(), NULL);

   evas_object_event_callback_add(bg, EVAS_CALLBACK_KEY_DOWN, _on_key_down, em);
   evas_object_focus_set(bg, EINA_TRUE);

   emotion_object_play_set(em, EINA_TRUE);

   ecore_main_loop_begin();

   ecore_evas_free(ee);
   ecore_evas_shutdown();
   return 0;

error:
   fprintf(stderr, "error: Requires at least one Evas engine built and linked"
                   " to ecore-evas for this example to run properly.\n");

   EINA_LIST_FREE(filenames, curfile)
      eina_stringshare_del(eina_list_data_get(curfile));

   ecore_evas_shutdown();
   eina_shutdown();
   return -1;
}
