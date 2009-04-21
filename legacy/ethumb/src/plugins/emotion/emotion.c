#include "Ethumb.h"
#include "Ethumb_Plugin.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <Evas.h>
#include <Emotion.h>

static Evas_Object *_emotion = NULL;

static void
_shutdown(Ethumb_Plugin *plugin)
{
}

static void
_frame_resized_cb(void *data, Evas_Object *o, void *event_info)
{
   Ethumb_File *ef = data;
   Ethumb *e = ef->ethumb;
   Evas_Coord ww, hh;
   Evas_Coord mw, mh;

   emotion_object_size_get(o, &mw, &mh);
   ethumb_calculate_aspect(e, mw, mh, &ww, &hh);
   ethumb_plugin_image_resize(ef, ww, hh);

   evas_object_resize(o, ww, hh);
   evas_object_move(o, 0, 0);

   ethumb_image_save(ef);

   evas_object_smart_callback_del(o, "frame_resize", _frame_resized_cb);
   emotion_object_play_set(o, 0);
   evas_object_del(o);
   ethumb_finished_callback_call(ef);
}

static int
_generate_thumb(Ethumb_File *ef)
{
   Ethumb *e = ef->ethumb;
   Evas_Object *o;
   int r;

   o = emotion_object_add(e->sub_e);
   r = emotion_object_init(o, "xine");
   if (!r)
     {
	fprintf(stderr, "ERROR: could not start emotion using gstreamer"
		" plugin.\n");
	evas_object_del(o);
	return r;
     }

   emotion_object_file_set(o, ef->src_path);
   emotion_object_position_set(o, e->video.time);
   emotion_object_play_set(o, 1);

   evas_object_smart_callback_add(o, "frame_resize", _frame_resized_cb, ef);

   evas_object_show(o);

   _emotion = o;

   return 1;
}

Ethumb_Plugin *
ethumb_plugin_init(void)
{
   static const char *extensions[] = { "avi", NULL };
   static Ethumb_Plugin plugin =
     {
	extensions,
	_generate_thumb,
	_shutdown
     };

   return &plugin;
}
