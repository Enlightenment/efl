#include "Emotion.h"
#include "emotion_private.h"

#define E_SMART_OBJ_GET(smart, o, type) \
     { \
	char *_e_smart_str; \
	\
	if (!o) return; \
	smart = evas_object_smart_data_get(o); \
	if (!smart) return; \
	_e_smart_str = (char *)evas_object_type_get(o); \
	if (!_e_smart_str) return; \
	if (strcmp(_e_smart_str, type)) return; \
     }

#define E_SMART_OBJ_GET_RETURN(smart, o, type, ret) \
   { \
      char *_e_smart_str; \
      \
      if (!o) return ret; \
      smart = evas_object_smart_data_get(o); \
      if (!smart) return ret; \
      _e_smart_str = (char *)evas_object_type_get(o); \
      if (!_e_smart_str) return ret; \
      if (strcmp(_e_smart_str, type)) return ret; \
   }

#define E_OBJ_NAME "emotion_object"

typedef struct _Smart_Data Smart_Data;

struct _Smart_Data
{
   Emotion_Video_Module  *module;
   void                  *video;

   char                  *module_name;

   char          *file;
   Evas_Object   *obj;
   double         ratio;
   double         pos;
   double         seek_pos;
   double         len;

   Ecore_Job     *job;

   unsigned char  play : 1;
   unsigned char  seek : 1;

   char *title;
   struct {
      char   *info;
      double  stat;
   } progress;
   struct {
      char *file;
      int   num;
   } ref;
   struct {
      int button_num;
      int button;
   } spu;

   Emotion_Module_Options module_options;
};

static void _mouse_move(void *data, Evas *ev, Evas_Object *obj, void *event_info);
static void _mouse_down(void *data, Evas *ev, Evas_Object *obj, void *event_info);
static void _pos_set_job(void *data);
static void _pixels_get(void *data, Evas_Object *obj);

static void _smart_init(void);
static void _smart_add(Evas_Object * obj);
static void _smart_del(Evas_Object * obj);
static void _smart_move(Evas_Object * obj, Evas_Coord x, Evas_Coord y);
static void _smart_resize(Evas_Object * obj, Evas_Coord w, Evas_Coord h);
static void _smart_show(Evas_Object * obj);
static void _smart_hide(Evas_Object * obj);
static void _smart_color_set(Evas_Object * obj, int r, int g, int b, int a);
static void _smart_clip_set(Evas_Object * obj, Evas_Object * clip);
static void _smart_clip_unset(Evas_Object * obj);

/**********************************/
/* Globals for the E Video Object */
/**********************************/
static Evas_Smart  *smart = NULL;
static Ecore_Path_Group *path_group = NULL;

static unsigned char
_emotion_module_open(const char *name, Evas_Object *obj, Emotion_Video_Module **mod, void **video)
{
   Ecore_Plugin *plugin;
   char *tmp = NULL;
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!path_group)
     path_group = ecore_path_group_new();
   tmp = getenv("EMOTION_MODULES_DIR");
   if (tmp)
     ecore_path_group_add(path_group, tmp);
   ecore_path_group_add(path_group, PACKAGE_LIB_DIR"/emotion/");
   plugin = ecore_plugin_load(path_group, name, NULL);
   if (plugin)
     {
	unsigned char (*func_module_open)(Evas_Object *, Emotion_Video_Module **, void **, Emotion_Module_Options *);

	func_module_open = ecore_plugin_symbol_get(plugin, "module_open");
	if (func_module_open)
	  {
	     if (func_module_open(obj, mod, video, &(sd->module_options)))
	       {
		  if (*mod)
		    {
		       (*mod)->plugin = plugin;
		       (*mod)->path_group = path_group;
		       return 1;
		    }
	       }
	  }
	ecore_plugin_unload(plugin);
     }
   else
     printf ("Unable to load module %s\n", name);

   if (path_group)
     {
        ecore_path_group_del(path_group);
        path_group = NULL;
     }

   return 0;
}

static void
_emotion_module_close(Emotion_Video_Module *mod, void *video)
{
   Ecore_Plugin *plugin;
   void (*module_close) (Emotion_Video_Module *module, void *);

   plugin = mod->plugin;
   module_close = ecore_plugin_symbol_get(mod->plugin, "module_close");
   if ((module_close) && (video)) module_close(mod, video);
   /* FIXME: we can't go dlclosing here as a thread still may be running from
    * the module - this in theory will leak- but it shouldnt be too bad and
    * mean that once a module is dlopened() it cant be closed - its refcount
    * will just keep going up
    */
   /*
   ecore_plugin_unload(plugin);
   */
   if (path_group)
     {
        ecore_path_group_del(path_group);
        path_group = NULL;
     }
}

/*******************************/
/* Externally accessible calls */
/*******************************/



EAPI Evas_Object *
emotion_object_add(Evas *evas)
{
   _smart_init();
   return evas_object_smart_add(evas, smart);
}

EAPI void
emotion_object_module_option_set(Evas_Object *obj, const char *opt, const char *val)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if ((!opt) || (!val)) return;
   if (!strcmp(opt, "video"))
     {
	if (!strcmp(val, "off")) sd->module_options.no_video = 1;
     }
   else if (!strcmp(opt, "audio"))
     {
	if (!strcmp(val, "off")) sd->module_options.no_audio = 1;
     }
}

EAPI Evas_Bool
emotion_object_init(Evas_Object *obj, const char *module_filename)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);

   if (sd->file) free(sd->file);
   sd->file = NULL;
   if (sd->title) free(sd->title);
   sd->title = NULL;
   if (sd->progress.info) free(sd->progress.info);
   sd->progress.info = NULL;
   sd->progress.stat = 0.0;
   if (sd->ref.file) free(sd->ref.file);
   sd->ref.file = NULL;
   sd->ref.num = 0;
   sd->spu.button_num = 0;
   sd->spu.button = -1;
   sd->ratio = 1.0;
   sd->pos = 0;
   sd->seek_pos = 0;
   sd->len = 0;

   ecore_job_init();

   if ((!sd->module) || (!sd->video))
     {
	if (!_emotion_module_open(module_filename, obj,
				  &sd->module, &sd->video))
	  return 0;
     }

   return 1;
}

EAPI void
emotion_object_file_set(Evas_Object *obj, const char *file)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);

   if ((file) && (sd->file) && (!strcmp(file, sd->file))) return;
   if ((file) && (file[0] != 0))
     {
        int w, h;

        if (sd->file) free(sd->file);
	sd->file = strdup(file);
	if (sd->module)
	  {
	    sd->module->file_close(sd->video);
	    evas_object_image_size_set(sd->obj, 0, 0);
	  }
	if (!sd->module->file_open(sd->file, obj, sd->video))
	  return;
	sd->module->size_get(sd->video, &w, &h);
	evas_object_image_size_set(sd->obj, w, h);
	sd->ratio = sd->module->ratio_get(sd->video);
	sd->pos = 0.0;
	if (sd->play) sd->module->play(sd->video, 0.0);
     }
   else
     {
        if (sd->video && sd->module)
	  {
	     sd->module->file_close(sd->video);
	     evas_object_image_size_set(sd->obj, 0, 0);
	  }
        if (sd->file) free(sd->file);
        sd->file = NULL;
     }
}

EAPI const char *
emotion_object_file_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, NULL);
   return sd->file;
}

EAPI void
emotion_object_play_set(Evas_Object *obj, Evas_Bool play)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (((play) && (sd->play)) || ((!play) && (!sd->play))) return;
   if (!sd->module) return;
   if (!sd->video) return;
   sd->play = play;
   if (sd->play) sd->module->play(sd->video, sd->pos);
   else sd->module->stop(sd->video);
}

EAPI Evas_Bool
emotion_object_play_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->video) return 0;

   return sd->play;
}

EAPI void
emotion_object_position_set(Evas_Object *obj, double sec)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->module) return;
   if (!sd->video) return;
   sd->seek_pos = sec;
   sd->seek = 1;
   sd->pos = sd->seek_pos;
   if (sd->job) ecore_job_del(sd->job);
   sd->job = ecore_job_add(_pos_set_job, obj);
}

EAPI double
emotion_object_position_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0.0);
   if (!sd->module) return 0.0;
   if (!sd->video) return 0.0;
   return sd->pos;
}

EAPI Evas_Bool
emotion_object_seekable_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->seekable(sd->video);
}

EAPI Evas_Bool
emotion_object_video_handled_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->video_handled(sd->video);
}

EAPI Evas_Bool
emotion_object_audio_handled_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->audio_handled(sd->video);
}

EAPI double
emotion_object_play_length_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0.0);
   if (!sd->module) return 0.0;
   if (!sd->video) return 0.0;
   sd->len = sd->module->len_get(sd->video);
   return sd->len;
}

EAPI void
emotion_object_size_get(Evas_Object *obj, int *iw, int *ih)
{
   Smart_Data *sd;

   if (iw) *iw = 0;
   if (ih) *ih = 0;
   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   evas_object_image_size_get(sd->obj, iw, ih);
}

EAPI void
emotion_object_smooth_scale_set(Evas_Object *obj, Evas_Bool smooth)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   evas_object_image_smooth_scale_set(sd->obj, smooth);
}

EAPI Evas_Bool
emotion_object_smooth_scale_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   return evas_object_image_smooth_scale_get(sd->obj);
}

EAPI double
emotion_object_ratio_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 1.0);
   if (!sd->module) return 0.0;
   if (!sd->video) return 0.0;
   return sd->ratio;
}

EAPI void
emotion_object_event_simple_send(Evas_Object *obj, Emotion_Event ev)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->module) return;
   if (!sd->video) return;
   sd->module->event_feed(sd->video, ev);
}

EAPI void
emotion_object_audio_volume_set(Evas_Object *obj, double vol)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->module) return;
   if (!sd->video) return;
   sd->module->audio_channel_volume_set(sd->video, vol);
}

EAPI double
emotion_object_audio_volume_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 1.0);
   if (!sd->module) return 0.0;
   if (!sd->video) return 0.0;
   return sd->module->audio_channel_volume_get(sd->video);
}

EAPI void
emotion_object_audio_mute_set(Evas_Object *obj, Evas_Bool mute)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->module) return;
   if (!sd->video) return;
   sd->module->audio_channel_mute_set(sd->video, mute);
}

EAPI Evas_Bool
emotion_object_audio_mute_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->audio_channel_mute_get(sd->video);
}

EAPI int
emotion_object_audio_channel_count(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->audio_channel_count(sd->video);
}

EAPI const char *
emotion_object_audio_channel_name_get(Evas_Object *obj, int channel)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, NULL);
   if (!sd->module) return NULL;
   if (!sd->video) return NULL;
   return sd->module->audio_channel_name_get(sd->video, channel);
}

EAPI void
emotion_object_audio_channel_set(Evas_Object *obj, int channel)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->module) return;
   if (!sd->video) return;
   sd->module->audio_channel_set(sd->video, channel);
}

EAPI int
emotion_object_audio_channel_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->audio_channel_get(sd->video);
}

EAPI void
emotion_object_video_mute_set(Evas_Object *obj, Evas_Bool mute)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->module) return;
   if (!sd->video) return;
   sd->module->video_channel_mute_set(sd->video, mute);
}

EAPI Evas_Bool
emotion_object_video_mute_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->video_channel_mute_get(sd->video);
}

EAPI int
emotion_object_video_channel_count(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->video_channel_count(sd->video);
}

EAPI const char *
emotion_object_video_channel_name_get(Evas_Object *obj, int channel)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, NULL);
   if (!sd->module) return NULL;
   if (!sd->video) return NULL;
   return sd->module->video_channel_name_get(sd->video, channel);
}

EAPI void
emotion_object_video_channel_set(Evas_Object *obj, int channel)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->module) return;
   if (!sd->video) return;
   sd->module->video_channel_set(sd->video, channel);
}

EAPI int
emotion_object_video_channel_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->video_channel_get(sd->video);
}

EAPI void
emotion_object_spu_mute_set(Evas_Object *obj, Evas_Bool mute)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->module) return;
   if (!sd->video) return;
   sd->module->spu_channel_mute_set(sd->video, mute);
}

EAPI Evas_Bool
emotion_object_spu_mute_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->spu_channel_mute_get(sd->video);
}

EAPI int
emotion_object_spu_channel_count(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->spu_channel_count(sd->video);
}

EAPI const char *
emotion_object_spu_channel_name_get(Evas_Object *obj, int channel)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, NULL);
   if (!sd->module) return NULL;
   if (!sd->video) return NULL;
   return sd->module->spu_channel_name_get(sd->video, channel);
}

EAPI void
emotion_object_spu_channel_set(Evas_Object *obj, int channel)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->module) return;
   if (!sd->video) return;
   sd->module->spu_channel_set(sd->video, channel);
}

EAPI int
emotion_object_spu_channel_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->spu_channel_get(sd->video);
}

EAPI int
emotion_object_chapter_count(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->chapter_count(sd->video);
}

EAPI void
emotion_object_chapter_set(Evas_Object *obj, int chapter)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->module) return;
   if (!sd->video) return;
   sd->module->chapter_set(sd->video, chapter);
}

EAPI int
emotion_object_chapter_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->chapter_get(sd->video);
}

EAPI const char *
emotion_object_chapter_name_get(Evas_Object *obj, int chapter)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, NULL);
   if (!sd->module) return NULL;
   if (!sd->video) return NULL;
   return sd->module->chapter_name_get(sd->video, chapter);
}

EAPI void
emotion_object_play_speed_set(Evas_Object *obj, double speed)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->module) return;
   if (!sd->video) return;
   sd->module->speed_set(sd->video, speed);
}

EAPI double
emotion_object_play_speed_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0.0);
   if (!sd->module) return 0.0;
   if (!sd->video) return 0.0;
   return sd->module->speed_get(sd->video);
}

EAPI void
emotion_object_eject(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->module) return;
   if (!sd->video) return;
   sd->module->eject(sd->video);
}

EAPI const char *
emotion_object_title_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, NULL);
   return sd->title;
}

EAPI const char *
emotion_object_progress_info_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, NULL);
   return sd->progress.info;
}

EAPI double
emotion_object_progress_status_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0.0);
   return sd->progress.stat;
}

EAPI const char *
emotion_object_ref_file_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, NULL);
   return sd->ref.file;
}

EAPI int
emotion_object_ref_num_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   return sd->ref.num;
}

EAPI int
emotion_object_spu_button_count_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   return sd->spu.button_num;
}

EAPI int
emotion_object_spu_button_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   return sd->spu.button;
}

EAPI const char *
emotion_object_meta_info_get(Evas_Object *obj, Emotion_Meta_Info meta)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, NULL);
   if (!sd->module) return NULL;
   if (!sd->video) return NULL;
   switch (meta)
     {
      case EMOTION_META_INFO_TRACK_TITLE:
	return sd->module->meta_get(sd->video, META_TRACK_TITLE);
	break;
      case EMOTION_META_INFO_TRACK_ARTIST:
	return sd->module->meta_get(sd->video, META_TRACK_ARTIST);
	break;
      case EMOTION_META_INFO_TRACK_ALBUM:
	return sd->module->meta_get(sd->video, META_TRACK_ALBUM);
	break;
      case EMOTION_META_INFO_TRACK_YEAR:
	return sd->module->meta_get(sd->video, META_TRACK_YEAR);
	break;
      case EMOTION_META_INFO_TRACK_GENRE:
	return sd->module->meta_get(sd->video, META_TRACK_GENRE);
	break;
      case EMOTION_META_INFO_TRACK_COMMENT:
	return sd->module->meta_get(sd->video, META_TRACK_COMMENT);
	break;
      case EMOTION_META_INFO_TRACK_DISC_ID:
	return sd->module->meta_get(sd->video, META_TRACK_DISCID);
	break;
      default:
	break;
     }
   return NULL;
}

EAPI void
emotion_object_vis_set(Evas_Object *obj, Emotion_Vis visualization)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->module) return;
   if (!sd->video) return;
   if (!sd->module->vis_set) return;
   sd->module->vis_set(sd->video, visualization);
}

EAPI Emotion_Vis
emotion_object_vis_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, EMOTION_VIS_NONE);
   if (!sd->module) return EMOTION_VIS_NONE;
   if (!sd->video) return EMOTION_VIS_NONE;
   if (!sd->module->vis_get) return EMOTION_VIS_NONE;
   return sd->module->vis_get(sd->video);
}

EAPI Evas_Bool
emotion_object_vis_supported(Evas_Object *obj, Emotion_Vis visualization)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   if (!sd->module->vis_supported) return 0;
   return sd->module->vis_supported(sd->video, visualization);
}



/*****************************/
/* Utility calls for modules */
/*****************************/

EAPI void *
_emotion_video_get(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, NULL);
   return sd->video;
}

EAPI void
_emotion_frame_new(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
//   printf("pix get set 1 %p\n", sd->obj);
   evas_object_image_pixels_dirty_set(sd->obj, 1);
   evas_object_smart_callback_call(obj, "frame_decode", NULL);
}

EAPI void
_emotion_video_pos_update(Evas_Object *obj, double pos, double len)
{
   Smart_Data *sd;
   int npos = 0, nlen = 0;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (pos != sd->pos) npos = 1;
   if (len != sd->len) nlen = 1;
   sd->pos = pos;
   sd->len = len;
   if (npos) evas_object_smart_callback_call(obj, "position_update", NULL);
   if (nlen) evas_object_smart_callback_call(obj, "length_change", NULL);
}

EAPI void
_emotion_frame_resize(Evas_Object *obj, int w, int h, double ratio)
{
   Smart_Data *sd;
   int iw, ih;
   int changed = 0;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   evas_object_image_size_get(sd->obj, &iw, &ih);
   if ((w != iw) || (h != ih))
     {
	if (h > 0) sd->ratio  = (double)w / (double)h;
	else sd->ratio = 1.0;
	evas_object_image_size_set(sd->obj, w, h);
	changed = 1;
     }
   if (ratio != sd->ratio)
     {
	sd->ratio = ratio;
	changed = 1;
     }
   if (changed) evas_object_smart_callback_call(obj, "frame_resize", NULL);
}

EAPI void
_emotion_decode_stop(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (sd->play)
     {
	sd->play = 0;
	evas_object_smart_callback_call(obj, "decode_stop", NULL);
     }
}

EAPI void
_emotion_playback_finished(Evas_Object *obj)
{
   evas_object_smart_callback_call(obj, "playback_finished", NULL);
}

EAPI void
_emotion_audio_level_change(Evas_Object *obj)
{
   evas_object_smart_callback_call(obj, "audio_level_change", NULL);
}

EAPI void
_emotion_channels_change(Evas_Object *obj)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   evas_object_smart_callback_call(obj, "channels_change", NULL);
}

EAPI void
_emotion_title_set(Evas_Object *obj, char *title)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (sd->title) free(sd->title);
   sd->title = strdup(title);
   evas_object_smart_callback_call(obj, "title_change", NULL);
}

EAPI void
_emotion_progress_set(Evas_Object *obj, char *info, double stat)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (sd->progress.info) free(sd->progress.info);
   sd->progress.info = strdup(info);
   sd->progress.stat = stat;
   evas_object_smart_callback_call(obj, "progress_change", NULL);
}

EAPI void
_emotion_file_ref_set(Evas_Object *obj, char *file, int num)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (sd->ref.file) free(sd->ref.file);
   sd->ref.file = strdup(file);
   sd->ref.num = num;
   evas_object_smart_callback_call(obj, "ref_change", NULL);
}

EAPI void
_emotion_spu_button_num_set(Evas_Object *obj, int num)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   sd->spu.button_num = num;
   evas_object_smart_callback_call(obj, "button_num_change", NULL);
}

EAPI void
_emotion_spu_button_set(Evas_Object *obj, int button)
{
   Smart_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   sd->spu.button = button;
   evas_object_smart_callback_call(obj, "button_change", NULL);
}


/****************************/
/* Internal object routines */
/****************************/

static void
_mouse_move(void *data, Evas *ev, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Move *e;
   Smart_Data *sd;
   int x, y, iw, ih;
   Evas_Coord ox, oy, ow, oh;

   e = event_info;
   sd = data;
   if (!sd->module) return;
   if (!sd->video) return;
   evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);
   evas_object_image_size_get(obj, &iw, &ih);
   if ((iw < 1) || (ih < 1)) return;
   x = (((int)e->cur.canvas.x - ox) * iw) / ow;
   y = (((int)e->cur.canvas.y - oy) * ih) / oh;
   sd->module->event_mouse_move_feed(sd->video, x, y);
}

static void
_mouse_down(void *data, Evas *ev, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *e;
   Smart_Data *sd;
   int x, y, iw, ih;
   Evas_Coord ox, oy, ow, oh;

   e = event_info;
   sd = data;
   if (!sd->module) return;
   if (!sd->video) return;
   evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);
   evas_object_image_size_get(obj, &iw, &ih);
   if ((iw < 1) || (ih < 1)) return;
   x = (((int)e->canvas.x - ox) * iw) / ow;
   y = (((int)e->canvas.y - oy) * ih) / oh;
   sd->module->event_mouse_button_feed(sd->video, 1, x, y);
}

static void
_pos_set_job(void *data)
{
   Evas_Object *obj;
   Smart_Data *sd;

   obj = data;
   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   sd->job = NULL;
   if (sd->seek)
     {
	sd->module->pos_set(sd->video, sd->seek_pos);
	sd->seek = 0;
     }
}

/* called by evas when it needs pixels for the image object */
static void
_pixels_get(void *data, Evas_Object *obj)
{
   int iw, ih, w, h;
   Smart_Data *sd;
   Emotion_Format format;
   unsigned char *bgra_data;

   sd = data;
   evas_object_image_size_get(obj, &iw, &ih);
   sd->module->video_data_size_get(sd->video, &w, &h);
   w = (w >> 1) << 1;
   h = (h >> 1) << 1;
   if ((w != iw) || (h != ih))
     {
	evas_object_image_colorspace_set(obj, EVAS_COLORSPACE_YCBCR422P601_PL);
        evas_object_image_alpha_set(obj, 0);
	evas_object_image_size_set(obj, w, h);
	iw = w;
	ih = h;
     }
   if ((iw < 1) || (ih < 1))
     {
//	printf("pix get set 0 (1)\n");
	evas_object_image_pixels_dirty_set(obj, 0);
     }
   else
     {
	format = sd->module->format_get(sd->video);
	if ((format == EMOTION_FORMAT_YV12) || (format == EMOTION_FORMAT_I420))
	  {
	     unsigned char **rows;

	     evas_object_image_colorspace_set(obj, EVAS_COLORSPACE_YCBCR422P601_PL);
	     rows = evas_object_image_data_get(obj, 1);
	     if (rows)
	       {
		  if (sd->module->yuv_rows_get(sd->video, iw, ih,
					       rows,
					       &rows[ih],
					       &rows[ih + (ih / 2)]))
		    evas_object_image_data_update_add(obj, 0, 0, iw, ih);
	       }
	     evas_object_image_data_set(obj, rows);
//	     printf("pix get set 0 (2)\n");
	     evas_object_image_pixels_dirty_set(obj, 0);
	  }
	else if (format == EMOTION_FORMAT_BGRA)
	  {
	     if (sd->module->bgra_data_get(sd->video, &bgra_data))
	       {
		  evas_object_image_data_set(obj, bgra_data);
//		  printf("pix get set 0 (3)\n");
		  evas_object_image_pixels_dirty_set(obj, 0);
	       }
	  }
     }
// no need for this because on any new frame decode info from the decoder
// module, the previous "current frame" is released (done) for us anyway
//   sd->module->frame_done(sd->video);

/*
   evas_object_image_size_get(obj, &iw, &ih);
   sd->module->video_data_size_get(sd->video, &w, &h);
   if ((w != iw) || (h != ih))
     {
	evas_object_image_size_set(obj, w, h);
	iw = w;
	ih = h;
     }
   format = sd->module->format_get(sd->video);
   if ((format == EMOTION_FORMAT_YV12) || (format == EMOTION_FORMAT_I420))
     {
	unsigned char **rows;
	Evas_Pixel_Import_Source ps;

	ps.format = EVAS_PIXEL_FORMAT_YUV420P_601;
	ps.w = iw;
	ps.h = ih;

	ps.rows = malloc(ps.h * 2 * sizeof(void *));
	if (!ps.rows)
	  {
	     sd->module->frame_done(sd->video);
	     return;
	  }

	rows = (unsigned char **)ps.rows;

	if (sd->module->yuv_rows_get(sd->video, iw, ih,
				     rows,
				     &rows[ps.h],
				     &rows[ps.h + (ps.h / 2)]))
	  evas_object_image_pixels_import(obj, &ps);
	evas_object_image_pixels_dirty_set(obj, 0);
	free(ps.rows);
     }
   else if (format == EMOTION_FORMAT_BGRA)
     {
	if (sd->module->bgra_data_get(sd->video, &bgra_data));
	  {
	     evas_object_image_data_set(obj, bgra_data);
	     evas_object_image_pixels_dirty_set(obj, 0);
	  }
     }
   sd->module->frame_done(sd->video);
 */
}

/*******************************************/
/* Internal smart object required routines */
/*******************************************/
static void
_smart_init(void)
{
   if (smart) return;
     {
	static const Evas_Smart_Class sc =
	  {
	     E_OBJ_NAME,
	       EVAS_SMART_CLASS_VERSION,
	       _smart_add,
	       _smart_del,
	       _smart_move,
	       _smart_resize,
	       _smart_show,
	       _smart_hide,
	       _smart_color_set,
	       _smart_clip_set,
	       _smart_clip_unset,
	       NULL,
	       NULL
	  };
        smart = evas_smart_class_new(&sc);
     }
}

static void
_smart_add(Evas_Object * obj)
{
   Smart_Data *sd;
   unsigned int *pixel;

   sd = calloc(1, sizeof(Smart_Data));
   if (!sd) return;
   sd->obj = evas_object_image_add(evas_object_evas_get(obj));
   evas_object_event_callback_add(sd->obj, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move, sd);
   evas_object_event_callback_add(sd->obj, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down, sd);
   evas_object_image_pixels_get_callback_set(sd->obj, _pixels_get, sd);
   evas_object_smart_member_add(sd->obj, obj);
   sd->ratio = 1.0;
   sd->spu.button = -1;
   evas_object_image_alpha_set(sd->obj, 0);
   pixel = evas_object_image_data_get(sd->obj, 1);
   if (pixel)
     {
	*pixel = 0xff000000;
	evas_object_image_data_set(obj, pixel);
     }
   evas_object_smart_data_set(obj, sd);
}

static void
_smart_del(Evas_Object * obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (sd->video) sd->module->file_close(sd->video);
   _emotion_module_close(sd->module, sd->video);
   evas_object_del(sd->obj);
   if (sd->file) free(sd->file);
   if (sd->job) ecore_job_del(sd->job);
   if (sd->progress.info) free(sd->progress.info);
   if (sd->ref.file) free(sd->ref.file);
   free(sd);

   ecore_job_shutdown();
}

static void
_smart_move(Evas_Object * obj, Evas_Coord x, Evas_Coord y)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_move(sd->obj, x, y);
}

static void
_smart_resize(Evas_Object * obj, Evas_Coord w, Evas_Coord h)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_image_fill_set(sd->obj, 0, 0, w, h);
   evas_object_resize(sd->obj, w, h);
}

static void
_smart_show(Evas_Object * obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_show(sd->obj);

}

static void
_smart_hide(Evas_Object * obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_hide(sd->obj);
}

static void
_smart_color_set(Evas_Object * obj, int r, int g, int b, int a)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_color_set(sd->obj, r, g, b, a);
}

static void
_smart_clip_set(Evas_Object * obj, Evas_Object * clip)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_clip_set(sd->obj, clip);
}

static void
_smart_clip_unset(Evas_Object * obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_clip_unset(sd->obj);
}
