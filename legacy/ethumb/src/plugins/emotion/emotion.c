#include "Ethumb.h"
#include "Ethumb_Plugin.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define EDJE_EDIT_IS_UNSTABLE_AND_I_KNOW_ABOUT_IT 1

#include <stdio.h>
#include <stdlib.h>
#include <Eina.h>
#include <Eet.h>
#include <Ecore_File.h>
#include <Evas.h>
#include <Ecore.h>
#include <Edje.h>
#include <Edje_Edit.h>
#include <Emotion.h>

static int _log_dom = -1;
#define DBG(...) EINA_LOG_DOM_DBG(_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_log_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_log_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_log_dom, __VA_ARGS__)

struct _emotion_plugin
{
   unsigned int fps;
   double ptotal, len, pi;
   double total_time, tmp_time;
   unsigned int pcount;
   unsigned int frnum;
   Eina_Bool first;
   Eet_File *ef;
   Evas_Object *video;
   Ethumb *e;
   int w, h;
};

static void
_resize_movie(struct _emotion_plugin *_plugin)
{
   Ethumb *e = _plugin->e;
   double ratio;
   int w, h;
   int fx, fy, fw, fh;

   ratio = emotion_object_ratio_get(_plugin->video);
   ethumb_calculate_aspect_from_ratio(e, ratio, &w, &h);
   ethumb_calculate_fill_from_ratio(e, ratio, &fx, &fy, &fw, &fh);
   DBG("size: w=%d, h=%d fill: x=%d, y=%d, w=%d, h=%d", w, h, fx, fy, fw, fh);

   _plugin->w = w;
   _plugin->h = h;

   ethumb_plugin_image_resize(e, _plugin->w, _plugin->h);

   evas_object_resize(_plugin->video, fw, fh);
   evas_object_move(_plugin->video, fx, fy);
   emotion_object_audio_mute_set(_plugin->video, 1);
}

static void
_frame_resized_cb(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   _resize_movie(data);
}

static void
_video_stopped_cb(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   struct _emotion_plugin *_plugin = data;

   _plugin->pi = 0;
   _plugin->ptotal = 0;
   _plugin->first = EINA_FALSE;
   _plugin->total_time = _plugin->tmp_time;
}

static void
_video_pos_set(struct _emotion_plugin *_plugin)
{
   double pos;
   double interval;

   pos = ethumb_video_start_get(_plugin->e);
   interval = ethumb_video_interval_get(_plugin->e);
   _plugin->len = emotion_object_play_length_get(_plugin->video);

   if (_plugin->len > 0)
     _plugin->first = EINA_TRUE;

   if (pos <=0 || pos >= 1)
     _plugin->pi = 0.1 * _plugin->len + _plugin->pcount *
	_plugin->len * interval;
   else
     _plugin->pi = pos * _plugin->len + _plugin->pcount *
	_plugin->len * interval;

   emotion_object_position_set(_plugin->video, _plugin->pi);
}

static int
_setup_thumbnail(struct _emotion_plugin *_plugin)
{
   char buf[4096];
   Evas *evas;
   Evas_Object *edje;
   unsigned int i;
   const char *thumb_path;

   ethumb_thumb_path_get(_plugin->e, &thumb_path, NULL);
   evas = ethumb_evas_get(_plugin->e);

   if (!edje_file_group_exists(thumb_path, "movie/thumb"))
     {
	fprintf(stderr, "ERROR: no group 'movie/thumb' found.\n");
	goto exit_error;
     }

   edje = edje_edit_object_add(evas);
   edje_object_file_set(edje, thumb_path, "movie/thumb");
   if (!edje_object_part_exists(edje, "image"))
     {
	fprintf(stderr, "ERROR: no 'image' part found.\n");
	evas_object_del(edje);
	goto exit_error;
     }
   if (!edje_edit_program_exist(edje, "animate"))
     {
	fprintf(stderr, "ERROR: no 'animate' program found.\n");
	evas_object_del(edje);
	goto exit_error;
     }

   for (i = 0; i < _plugin->frnum; i++)
     {
	snprintf(buf, sizeof(buf), "images/%d", i);
	edje_edit_image_data_add(edje, buf, i);
	if (i == 0)
	  edje_edit_state_image_set(edje, "image", "default", 0.00, buf);
	else
	  edje_edit_state_tween_add(edje, "image", "default", 0.00, buf);
     }

   edje_edit_program_transition_time_set(edje, "animate",
					 _plugin->total_time);
   edje_edit_program_transition_time_set(edje, "animate_loop",
					 _plugin->total_time);
   edje_edit_group_min_w_set(edje, _plugin->w);
   edje_edit_group_max_w_set(edje, _plugin->w);
   edje_edit_group_min_h_set(edje, _plugin->h);
   edje_edit_group_max_h_set(edje, _plugin->h);
   edje_edit_save(edje);

   evas_object_del(edje);

   return 1;

exit_error:
   return 0;
}

static void
_finish_thumb_generation(struct _emotion_plugin *_plugin, int success)
{
   int r = 0;
   evas_object_smart_callback_del(_plugin->video, "frame_resize",
				  _frame_resized_cb);
   emotion_object_play_set(_plugin->video, 0);
   evas_object_del(_plugin->video);
   if (_plugin->ef)
     eet_close(_plugin->ef);

   if (success)
     r = _setup_thumbnail(_plugin);

   free(_plugin);
   ethumb_finished_callback_call(_plugin->e, r);
}

static Eina_Bool
_frame_grab_single(void *data)
{
   struct _emotion_plugin *_plugin = data;
   Ethumb *e = _plugin->e;
   double p;

   if (_plugin->len <= 0)
     {
	_video_pos_set(_plugin);
	return EINA_TRUE;
     }

   p = emotion_object_position_get(_plugin->video);
//   if (p < _plugin->pi)
   if (p <= 0.0)
     return EINA_TRUE;

   DBG("saving static thumbnail at position=%f (intended=%f)", p, _plugin->pi);

   ethumb_image_save(e);

   evas_object_smart_callback_del(_plugin->video, "frame_resize",
				  _frame_resized_cb);
   emotion_object_play_set(_plugin->video, 0);
   evas_object_del(_plugin->video);
   free(_plugin);
   
   ethumb_finished_callback_call(e, 1);

   return EINA_FALSE;
}

static Eina_Bool
_frame_grab(void *data)
{
   struct _emotion_plugin *_plugin = data;
   Ethumb *e = _plugin->e;
   char buf[4096];
   const void *pixels;
   double p;

   if (_plugin->len <= 0)
     {
	_video_pos_set(_plugin);
	return EINA_TRUE;
     }

   p = emotion_object_position_get(_plugin->video);
   if (p < _plugin->pi)
     return EINA_TRUE;

   if (_plugin->first)
     {
	_plugin->pi = p;
	_plugin->first = EINA_FALSE;
     }

   if (p > _plugin->pi + _plugin->ptotal)
     {
	_plugin->total_time += _plugin->tmp_time;
	if (_plugin->pcount >= ethumb_video_ntimes_get(e))
	  {
	     _finish_thumb_generation(_plugin, EINA_TRUE);
	     return EINA_FALSE;
	  }
	else
	  {
	     _plugin->pcount++;
	     _video_pos_set(_plugin);
	     return EINA_TRUE;
	  }
     }

   _plugin->tmp_time = p - _plugin->pi;

   if (_plugin->ef)
     {
	Ecore_Evas *ee = ethumb_ecore_evas_get(e);
	int quality, compress;

	quality = ethumb_thumb_quality_get(e);
	compress = ethumb_thumb_compress_get(e);

	pixels = ecore_evas_buffer_pixels_get(ee);
	snprintf(buf, sizeof(buf), "images/%d", _plugin->frnum);
	eet_data_image_write(_plugin->ef, buf, pixels, _plugin->w, _plugin->h,
			     0, compress, quality, quality);
	_plugin->frnum++;
     }

   return EINA_TRUE;
}

static void
_generate_animated_thumb(struct _emotion_plugin *_plugin)
{
   const char *thumb_path;
   char *thumb_dir;
   char buf[4096];
   Ethumb *e = _plugin->e;

   snprintf(buf, sizeof(buf), "%s/data/emotion_template.edj", PLUGINSDIR);
   ethumb_thumb_path_get(e, &thumb_path, NULL);
   thumb_dir = ecore_file_dir_get(thumb_path);
   ecore_file_mkpath(thumb_dir);
   free(thumb_dir);
   ecore_file_cp(buf, thumb_path);
   _plugin->ef = eet_open(thumb_path, EET_FILE_MODE_READ_WRITE);
   if (!_plugin->ef)
     {
	fprintf(stderr, "ERROR: could not open '%s'\n", thumb_path);
	_finish_thumb_generation(_plugin, 0);
     }

   ecore_timer_add(1.0 / ethumb_video_fps_get(e), _frame_grab, _plugin);
}

static void
_generate_thumb(Ethumb *e)
{
   Evas_Object *o;
   int r;
   const char *file;
   Ethumb_Thumb_Format f;
   struct _emotion_plugin *_plugin = calloc(sizeof(struct _emotion_plugin), 1);

   o = emotion_object_add(ethumb_evas_get(e));
   r = emotion_object_init(o, "xine");
   if (!r)
     {
	fprintf(stderr, "ERROR: could not start emotion using gstreamer"
		" plugin.\n");
	evas_object_del(o);
	ethumb_finished_callback_call(e, 0);
	free(_plugin);
	return;
     }

   _plugin->video = o;

   ethumb_file_get(e, &file, NULL);
   f = ethumb_thumb_format_get(e);

   emotion_object_file_set(o, file);
   emotion_object_audio_mute_set(o, EINA_TRUE);

   _plugin->video = o;
   _plugin->e = e;

   _plugin->ptotal = ethumb_video_time_get(e) / ethumb_video_ntimes_get(e);
   _plugin->pcount = 1;

   _resize_movie(_plugin);
   evas_object_smart_callback_add(o, "frame_resize",
				  _frame_resized_cb, _plugin);
   evas_object_smart_callback_add(o, "decode_stop",
				  _video_stopped_cb, _plugin);

   if (f == ETHUMB_THUMB_EET)
     {
	_generate_animated_thumb(_plugin);
     }
   else
     {
	ecore_timer_add(0.1, _frame_grab_single, _plugin);
     }

   _video_pos_set(_plugin);
   emotion_object_play_set(o, 1);
   evas_object_show(o);
}

EAPI Ethumb_Plugin *
ethumb_plugin_get(void)
{
   static const char *extensions[] = { "avi", "mp4", "ogv", "mov", "mpg", "wmv",
				       NULL };
   static Ethumb_Plugin plugin =
     {
	extensions,
	_generate_thumb,
     };

   _log_dom = eina_log_domain_register("ethumb_emotion", EINA_COLOR_GREEN);

   return &plugin;
}

static Eina_Bool
_module_init(void)
{
   return EINA_TRUE;
}

static void
_module_shutdown(void)
{
}

EINA_MODULE_INIT(_module_init);
EINA_MODULE_SHUTDOWN(_module_shutdown);
