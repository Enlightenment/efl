#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Evas.h>
#include <Emotion.h>
#include <stdio.h>
#include <string.h>

#define WIDTH  (320)
#define HEIGHT (240)

static Eina_List *filenames = NULL;
static Eina_List *curfile = NULL;

static void
_playback_started_cb(void *data, Evas_Object *o, void *event_info)
{
    printf("Emotion object started playback.\n");
}

static Evas_Object *
_create_emotion_object(Evas *e)
{
   Evas_Object *em = emotion_object_add(e);

   emotion_object_init(em, "gstreamer");

   evas_object_smart_callback_add(
       em, "playback_started", _playback_started_cb, NULL);

   return em;
}

static void
_on_key_down(void *data, Evas *e, Evas_Object *o, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   Evas_Object *em = data;

   if (!strcmp(ev->keyname, "Return"))
     {
	emotion_object_play_set(em, EINA_TRUE);
     }
   else if (!strcmp(ev->keyname, "space"))
     {
	emotion_object_play_set(em, EINA_FALSE);
     }
   else if (!strcmp(ev->keyname, "Escape"))
     {
	ecore_main_loop_quit();
     }
   else if (!strcmp(ev->keyname, "n"))
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
   else if (!strcmp(ev->keyname, "p"))
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
   else
     {
	fprintf(stderr, "unhandled key: %s\n", ev->keyname);
     }
}

static void
_frame_decode_cb(void *data, Evas_Object *o, void *event_info)
{
   // fprintf(stderr, "smartcb: frame_decode\n");
}

static void
_length_change_cb(void *data, Evas_Object *o, void *event_info)
{
   fprintf(stderr, "smartcb: length_change: %0.3f\n", emotion_object_play_length_get(o));
}

static void
_position_update_cb(void *data, Evas_Object *o, void *event_info)
{
   fprintf(stderr, "smartcb: position_update: %0.3f\n", emotion_object_position_get(o));
}

static void
_progress_change_cb(void *data, Evas_Object *o, void *event_info)
{
   fprintf(stderr, "smartcb: progress_change: %0.3f, %s\n",
	   emotion_object_progress_status_get(o),
	   emotion_object_progress_info_get(o));
}

static void
_frame_resize_cb(void *data, Evas_Object *o, void *event_info)
{
   int w, h;
   emotion_object_size_get(o, &w, &h);
   fprintf(stderr, "smartcb: frame_resize: %dx%d\n", w, h);
}

static void /* adjust canvas' contents on resizes */
_canvas_resize_cb(Ecore_Evas *ee)
{
   int w, h;
   Evas_Object *bg, *em;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);

   bg = ecore_evas_data_get(ee, "bg");
   em = ecore_evas_data_get(ee, "emotion");

   evas_object_resize(bg, w, h);
   evas_object_resize(em, w, h);
}

int
main(int argc, const char *argv[])
{
   int err;
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

   ecore_evas_callback_resize_set(ee, _canvas_resize_cb);

   ecore_evas_show(ee);

   /* the canvas pointer, de facto */
   e = ecore_evas_get(ee);

   /* adding a background to this example */
   bg = evas_object_rectangle_add(e);
   evas_object_name_set(bg, "our dear rectangle");
   evas_object_color_set(bg, 255, 0, 0, 255); /* white bg */
   evas_object_move(bg, 0, 0); /* at canvas' origin */
   evas_object_resize(bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(bg);

   ecore_evas_data_set(ee, "bg", bg);

   /* Creating the emotion object */
   em = _create_emotion_object(e);
   emotion_object_file_set(em, eina_list_data_get(curfile));
   evas_object_move(em, 0, 0);
   evas_object_resize(em, WIDTH, HEIGHT);
   emotion_object_border_set(em, -30, -30, -30, -30);
   evas_object_show(em);

   ecore_evas_data_set(ee, "emotion", em);

   evas_object_smart_callback_add(em, "frame_decode", _frame_decode_cb, NULL);
   evas_object_smart_callback_add(em, "length_change", _length_change_cb, NULL);
   evas_object_smart_callback_add(em, "position_update", _position_update_cb, NULL);
   evas_object_smart_callback_add(em, "progress_change", _progress_change_cb, NULL);
   evas_object_smart_callback_add(em, "frame_resize", _frame_resize_cb, NULL);

   evas_object_event_callback_add(bg, EVAS_CALLBACK_KEY_DOWN, _on_key_down, em);
   evas_object_focus_set(bg, EINA_TRUE);

   emotion_object_play_set(em, EINA_TRUE);

   ecore_main_loop_begin();

   ecore_evas_free(ee);
   ecore_evas_shutdown();
   return 0;

error:
   fprintf(stderr, "you got to have at least one evas engine built and linked"
                   " up to ecore-evas for this example to run properly.\n");

   EINA_LIST_FREE(filenames, curfile)
      eina_stringshare_del(eina_list_data_get(curfile));

   ecore_evas_shutdown();
   eina_shutdown();
   return -1;
}
