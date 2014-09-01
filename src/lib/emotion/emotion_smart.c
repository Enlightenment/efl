#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Evas.h>
#include <Ecore.h>

#ifdef HAVE_EIO
# include <math.h>
# include <Eio.h>
#endif

#include "Emotion.h"
#include "emotion_private.h"

#ifdef _WIN32
# define FMT_UCHAR "%c"
#else
# define FMT_UCHAR "%hhu"
#endif

#define E_SMART_OBJ_GET(smart, o, type) \
     { \
        if (!o) return; \
        if (!eo_isa(o, MY_CLASS)) { \
             ERR("Tried calling on a non-emotion object."); \
             return; \
        } \
        smart = eo_data_scope_get(o, MY_CLASS); \
        if (!smart) return; \
     }

#define E_SMART_OBJ_GET_RETURN(smart, o, type, ret) \
   { \
      if (!o) return ret; \
      if (!eo_isa(o, MY_CLASS)) { \
           ERR("Tried calling on a non-emotion object."); \
           return ret; \
      } \
      smart = eo_data_scope_get(o, MY_CLASS); \
      if (!smart) return ret; \
   }

#define E_OBJ_NAME "emotion_object"

#ifdef MY_CLASS
# undef MY_CLASS
#endif

#define MY_CLASS EMOTION_OBJECT_CLASS

typedef struct _Emotion_Object_Data Emotion_Object_Data;

struct _Emotion_Object_Data
{
   EINA_REFCOUNT;
   Emotion_Engine_Instance *engine_instance;

   const char    *engine;
   const char    *file;
   Evas_Object   *smartobj;
   Evas_Object   *obj;
   Evas_Object   *bg;

   Ecore_Job     *job;

   const char *title;

#ifdef HAVE_EIO
   Eio_File *load_xattr;
   Eio_File *save_xattr;
#endif

   struct {
      const char *info;
      double  stat;
   } progress;
   struct {
      const char *file;
      int   num;
   } ref;
   struct {
      int button_num;
      int button;
   } spu;
   struct {
      int l; /* left */
      int r; /* right */
      int t; /* top */
      int b; /* bottom */
      Evas_Object *clipper;
   } crop;

   struct {
      int         w, h;
   } video;
   struct {
      double      w, h;
   } fill;

   double         ratio;
   double         pos;
   double         remember_jump;
   double         seek_pos;
   double         len;

   Emotion_Module_Options module_options;

   Emotion_Suspend state;
   Emotion_Aspect aspect;

   Ecore_Animator *anim;

   Eina_Bool open : 1;
   Eina_Bool play : 1;
   Eina_Bool remember_play : 1;
   Eina_Bool seek : 1;
   Eina_Bool seeking : 1;
};

static void _mouse_move(void *data, Evas *ev, Evas_Object *obj, void *event_info);
static void _mouse_down(void *data, Evas *ev, Evas_Object *obj, void *event_info);
static void _pos_set_job(void *data);
static void _pixels_get(void *data, Evas_Object *obj);

/**********************************/
/* Globals for the E Video Object */
/**********************************/
static const char SIG_FRAME_DECODE[] = "frame_decode";
static const char SIG_POSITION_UPDATE[] = "position_update";
static const char SIG_LENGTH_CHANGE[] = "length_change";
static const char SIG_FRAME_RESIZE[] = "frame_resize";
static const char SIG_DECODE_STOP[] = "decode_stop";
static const char SIG_PLAYBACK_STARTED[] = "playback_started";
static const char SIG_PLAYBACK_FINISHED[] = "playback_finished";
static const char SIG_AUDIO_LEVEL_CHANGE[] = "audio_level_change";
static const char SIG_CHANNELS_CHANGE[] = "channels_change";
static const char SIG_TITLE_CHANGE[] = "title_change";
static const char SIG_PROGRESS_CHANGE[] = "progress_change";
static const char SIG_REF_CHANGE[] = "ref_change";
static const char SIG_BUTTON_NUM_CHANGE[] = "button_num_change";
static const char SIG_BUTTON_CHANGE[] = "button_change";
static const char SIG_OPEN_DONE[] = "open_done";
static const char SIG_POSITION_SAVE_SUCCEED[] = "position_save,succeed";
static const char SIG_POSITION_SAVE_FAILED[] = "position_save,failed";
static const char SIG_POSITION_LOAD_SUCCEED[] = "position_load,succeed";
static const char SIG_POSITION_LOAD_FAILED[] = "position_load,failed";

static void
_engine_init(Eo *obj, Emotion_Object_Data *sd)
{
   if (sd->engine_instance) return;
   sd->engine_instance = emotion_engine_instance_new(sd->engine, obj,
                                                     &(sd->module_options));
}

static void
_emotion_image_data_zero(Evas_Object *img)
{
   void *data = NULL;

   data = evas_object_image_data_get(img, 1);
   if (data)
     {
        int w, h, sz = 0;
        Evas_Colorspace cs;

        evas_object_image_size_get(img, &w, &h);
        cs = evas_object_image_colorspace_get(img);
        if (cs == EVAS_COLORSPACE_ARGB8888)
           sz = w * h * 4;
        if ((cs == EVAS_COLORSPACE_YCBCR422P601_PL) ||
            (cs == EVAS_COLORSPACE_YCBCR422P709_PL))
           sz = h * 2 * sizeof(unsigned char *);
        if (sz != 0) memset(data, 0, sz);
     }
   evas_object_image_data_set(img, data);
}

static void
_smart_data_free(Emotion_Object_Data *sd)
{
#ifdef HAVE_EIO
   /* Only cancel the load_xattr or we will loose ref to time_seek stringshare */
   if (sd->load_xattr) eio_file_cancel(sd->load_xattr);
   sd->load_xattr = NULL;
   if (sd->save_xattr) eio_file_cancel(sd->save_xattr);
   sd->save_xattr = NULL;
#endif

   if (sd->engine_instance)
     {
        emotion_engine_instance_file_close(sd->engine_instance);
        emotion_engine_instance_del(sd->engine_instance);
        sd->engine_instance = NULL;
     }
   if (sd->obj) evas_object_del(sd->obj);
   sd->obj = NULL;
   if (sd->crop.clipper) evas_object_del(sd->crop.clipper);
   sd->crop.clipper = NULL;
   if (sd->bg) evas_object_del(sd->bg);
   sd->bg = NULL;
   if (sd->file) eina_stringshare_del(sd->file);
   sd->file = NULL;
   if (sd->job) ecore_job_del(sd->job);
   sd->job = NULL;
   if (sd->anim) ecore_animator_del(sd->anim);
   sd->anim = NULL;
   eina_stringshare_del(sd->progress.info);
   sd->progress.info = NULL;
   eina_stringshare_del(sd->ref.file);
   sd->ref.file = NULL;
   if (sd->engine) eina_stringshare_del(sd->engine);
   sd->engine = NULL;

   /* TODO: remove legacy: emotion used to have no shutdown, call automatically */
   emotion_shutdown();
}

static void
_clipper_position_size_update(Evas_Object *obj, int x, int y, int w, int h, int vid_w, int vid_h)
{
   Emotion_Object_Data *sd;
   double scale_w, scale_h;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);

   if (vid_w == 0 || vid_h == 0)
     {
       evas_object_image_fill_set(sd->obj, 0, 0, 0, 0);
       evas_object_move(sd->obj, x, y);
       evas_object_resize(sd->obj, 0, 0);
       evas_object_move(sd->crop.clipper, x, y);
       evas_object_resize(sd->crop.clipper, 0, 0);
     }
   else
     {
       evas_object_move(sd->crop.clipper, x, y);
       scale_w = (double)w / (double)(vid_w - sd->crop.l - sd->crop.r);
       scale_h = (double)h / (double)(vid_h - sd->crop.t - sd->crop.b);

       if (sd->fill.w < 0 && sd->fill.h < 0)
         evas_object_image_fill_set(sd->obj, 0, 0, vid_w * scale_w, vid_h * scale_h);
       else
         evas_object_image_fill_set(sd->obj, 0, 0, sd->fill.w * w, sd->fill.h * h);
       evas_object_resize(sd->obj, vid_w * scale_w, vid_h * scale_h);
       evas_object_move(sd->obj, x - sd->crop.l * scale_w, y - sd->crop.t * scale_h);
       evas_object_resize(sd->crop.clipper, w, h);
     }
}

/*******************************/
/* Externally accessible calls */
/*******************************/



EAPI Evas_Object *
emotion_object_add(Evas *evas)
{
   Evas_Object *e;
   e = eo_add(MY_CLASS, evas);
   eo_unref(e);
   return e;
}

EOLIAN static void
_emotion_object_eo_base_constructor(Eo *obj, Emotion_Object_Data *pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj, evas_obj_type_set(E_OBJ_NAME));
}

EAPI Evas_Object *
emotion_object_image_get(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return NULL;
   return sd->obj;
}

EOLIAN static void
_emotion_object_option_set(Eo *obj EINA_UNUSED, Emotion_Object_Data *pd, const char *opt, const char *val)
{
   Emotion_Object_Data *sd = pd;

   if ((!opt) || (!val)) return;

   if (strcmp(opt, "video") == 0)
     {
        if (strcmp(val, "off") == 0)
          sd->module_options.no_video = EINA_TRUE;
        else if (strcmp(val, "on") == 0)
          sd->module_options.no_video = EINA_FALSE;
        else
          sd->module_options.no_video = !!atoi(val);

        ERR("Deprecated. Use emotion_object_video_mute_set()");
     }
   else if (strcmp(opt, "audio") == 0)
     {
        if (strcmp(val, "off") == 0)
          sd->module_options.no_audio = EINA_TRUE;
        else if (strcmp(val, "on") == 0)
          sd->module_options.no_audio = EINA_FALSE;
        else
          sd->module_options.no_audio = !!atoi(val);

        ERR("Deprecated. Use emotion_object_audio_mute_set()");
     }
   else
     ERR("Unsupported %s=%s", opt, val);
}

EOLIAN static Eina_Bool
_emotion_object_engine_set(Eo *obj, Emotion_Object_Data *pd, const char *engine)
{
   Emotion_Object_Data *sd = pd;
   const char *file;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);

   if (!engine) engine = "gstreamer1";
   if (!strcmp(engine, sd->engine)) return EINA_TRUE;

   eina_stringshare_replace(&(sd->engine), engine);

   file = sd->file;
   sd->file = NULL;

   eina_stringshare_del(sd->title);
   sd->title = NULL;
   eina_stringshare_del(sd->progress.info);
   sd->progress.info = NULL;
   sd->progress.stat = 0.0;
   eina_stringshare_del(sd->ref.file);
   sd->ref.file = NULL;
   sd->ref.num = 0;
   sd->spu.button_num = 0;
   sd->spu.button = -1;
   sd->ratio = 1.0;
   sd->pos = 0;
   sd->remember_jump = 0;
   sd->seek_pos = 0;
   sd->len = 0;
   sd->remember_play = 0;

   if (sd->anim) ecore_animator_del(sd->anim);
   sd->anim = NULL;

   if (sd->engine_instance) emotion_engine_instance_del(sd->engine_instance);
   sd->engine_instance = NULL;
   _engine_init(obj, sd);
   if (!sd->engine_instance)
     {
        sd->file = file;
        return EINA_FALSE;
     }

   if (file)
     {
        emotion_object_file_set(obj, file);
        eina_stringshare_del(file);
     }

   return EINA_TRUE;
}

EAPI Eina_Bool
emotion_object_file_set(Evas_Object *obj, const char *file)
{
   return eo_do(obj, efl_file_set(file, NULL));
}

EOLIAN static Eina_Bool
_emotion_object_efl_file_file_set(Eo *obj EINA_UNUSED, Emotion_Object_Data *sd, const char *file, const char *key EINA_UNUSED)
{
   DBG("file=%s", file);

   if (!eina_stringshare_replace(&sd->file, file)) return EINA_TRUE;

   if (!sd->engine_instance) _engine_init(obj, sd);
   if (!sd->engine_instance)
     {
        WRN("No engine choosen. Please set an engine.");
        return EINA_FALSE;
     }

   sd->video.w = 0;
   sd->video.h = 0;
   if ((file) && (file[0] != 0))
     {
        eina_stringshare_replace(&sd->file, file);
        emotion_engine_instance_file_close(sd->engine_instance);
        evas_object_image_data_set(sd->obj, NULL);
        evas_object_image_size_set(sd->obj, 1, 1);
        _emotion_image_data_zero(sd->obj);
        sd->open = 0;
        if (!emotion_engine_instance_file_open(sd->engine_instance, sd->file))
          {
             WRN("Couldn't open file=%s", sd->file);
             return EINA_FALSE;
          }
        DBG("successfully opened file=%s", sd->file);
        sd->pos = 0.0;
        if (sd->play) emotion_engine_instance_play(sd->engine_instance, 0.0);
     }
   else
     {
        emotion_engine_instance_file_close(sd->engine_instance);
        evas_object_image_data_set(sd->obj, NULL);
        evas_object_image_size_set(sd->obj, 1, 1);
        _emotion_image_data_zero(sd->obj);
        eina_stringshare_replace(&sd->file, NULL);
     }

   if (sd->anim) ecore_animator_del(sd->anim);
   sd->anim = NULL;

#ifdef HAVE_EIO
   /* Only cancel the load_xattr or we will loose ref to time_seek stringshare */
   if (sd->load_xattr) eio_file_cancel(sd->load_xattr);
   sd->load_xattr = NULL;
   if (sd->save_xattr) eio_file_cancel(sd->save_xattr);
   sd->save_xattr = NULL;
#endif

   return EINA_TRUE;
}

EAPI const char *
emotion_object_file_get(const Evas_Object *obj)
{
   const char *file = NULL;
   eo_do(obj, efl_file_get(&file, NULL));
   return file;
}

EOLIAN void
_emotion_object_efl_file_file_get(Eo *obj EINA_UNUSED, Emotion_Object_Data *sd, const char **file, const char **key)
{
   if (file) *file = sd->file;
   if (key) *key = NULL;
}

static void
_emotion_aspect_borders_apply(Evas_Object *obj, Emotion_Object_Data *sd, int w, int h, int iw, int ih)
{
   int x, y;

   evas_object_geometry_get(obj, &x, &y, NULL, NULL);

   /* applying calculated borders */
   if ((sd->crop.l == 0) && (sd->crop.r == 0) &&
       (sd->crop.t == 0) && (sd->crop.b == 0))
     {
        Evas_Object *old_clipper;
        if (sd->crop.clipper)
          {
             old_clipper = evas_object_clip_get(sd->crop.clipper);
             evas_object_clip_unset(sd->obj);
             evas_object_clip_set(sd->obj, old_clipper);
             evas_object_del(sd->crop.clipper);
             sd->crop.clipper = NULL;
          }
     }
   else
     {
        if (!sd->crop.clipper)
          {
             Evas_Object *old_clipper;
             sd->crop.clipper = evas_object_rectangle_add
               (evas_object_evas_get(obj));
             evas_object_color_set(sd->crop.clipper, 255, 255, 255, 255);
             evas_object_smart_member_add(sd->crop.clipper, obj);
             old_clipper = evas_object_clip_get(sd->obj);
             evas_object_clip_set(sd->obj, sd->crop.clipper);
             evas_object_clip_set(sd->crop.clipper, old_clipper);
             if (evas_object_visible_get(sd->obj))
               evas_object_show(sd->crop.clipper);
          }
     }
   _clipper_position_size_update(obj, x, y, w, h, iw, ih);
}

static void
_emotion_object_aspect_border_apply(Evas_Object *obj, Emotion_Object_Data *sd, int w, int h)
{
   int iw, ih;
   double ir;
   double r;

   int aspect_opt = 0;

   iw = sd->video.w;
   ih = sd->video.h;

   ir = (double)iw / ih;
   r = (double)w / h;

   /* First check if we should fit the width or height of the video inside the
    * width/height of the object.  This check takes into account the original
    * aspect ratio and the object aspect ratio, if we are keeping both sizes or
    * cropping the exceding area.
    */
   if (sd->aspect == EMOTION_ASPECT_KEEP_NONE)
     {
        sd->crop.l = 0;
        sd->crop.r = 0;
        sd->crop.t = 0;
        sd->crop.b = 0;
        aspect_opt = 0; // just ignore keep_aspect
     }
   else if (sd->aspect == EMOTION_ASPECT_KEEP_WIDTH)
     {
        aspect_opt = 1;
     }
   else if (sd->aspect == EMOTION_ASPECT_KEEP_HEIGHT)
     {
        aspect_opt = 2;
     }
   else if (sd->aspect == EMOTION_ASPECT_KEEP_BOTH)
     {
        if (ir > r) aspect_opt = 1;
        else aspect_opt = 2;
     }
   else if (sd->aspect == EMOTION_ASPECT_CROP)
     {
        if (ir > r) aspect_opt = 2;
        else aspect_opt = 1;
     }
   else if (sd->aspect == EMOTION_ASPECT_CUSTOM)
     {
        // nothing to do, just respect the border settings
        aspect_opt = 0;
     }

   /* updating borders based on keep_aspect settings */
   if (aspect_opt == 1) // keep width
     {
        int th, dh;
        double scale;

        sd->crop.l = 0;
        sd->crop.r = 0;
        scale = (double)iw / w;
        th = h * scale;
        dh = ih - th;
        sd->crop.t = sd->crop.b = dh / 2;
     }
   else if (aspect_opt == 2) // keep height
     {
        int tw, dw;
        double scale;

        sd->crop.t = 0;
        sd->crop.b = 0;
        scale = (double)ih / h;
        tw = w * scale;
        dw = iw - tw;
        sd->crop.l = sd->crop.r = dw / 2;
     }

   _emotion_aspect_borders_apply(obj, sd, w, h, iw, ih);
}

EAPI void
emotion_object_border_set(Evas_Object *obj, int l, int r, int t, int b)
{
   Emotion_Object_Data *sd;
   int w, h;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);

   sd->aspect = EMOTION_ASPECT_CUSTOM;
   sd->crop.l = -l;
   sd->crop.r = -r;
   sd->crop.t = -t;
   sd->crop.b = -b;
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   _emotion_object_aspect_border_apply(obj, sd, w, h);
}

EAPI void
emotion_object_border_get(const Evas_Object *obj, int *l, int *r, int *t, int *b)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   *l = -sd->crop.l;
   *r = -sd->crop.r;
   *t = -sd->crop.t;
   *b = -sd->crop.b;
}

EAPI void
emotion_object_bg_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);

   evas_object_color_set(sd->bg, r, g, b, a);

   if (!evas_object_visible_get(obj)) return;

   if (a > 0) evas_object_show(sd->bg);
   else evas_object_hide(sd->bg);
}

EAPI void
emotion_object_bg_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   evas_object_color_get(sd->bg, r, g, b, a);
}

EAPI void
emotion_object_keep_aspect_set(Evas_Object *obj, Emotion_Aspect a)
{
   Emotion_Object_Data *sd;
   int w, h;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (a == sd->aspect) return;

   sd->aspect = a;
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   _emotion_object_aspect_border_apply(obj, sd, w, h);
}

EAPI Emotion_Aspect
emotion_object_keep_aspect_get(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, EMOTION_ASPECT_KEEP_NONE);
   return sd->aspect;
}

EAPI void
emotion_object_play_set(Evas_Object *obj, Eina_Bool play)
{
   eo_do(obj, efl_player_play_set(play));
}

EOLIAN static void
_emotion_object_efl_player_play_set(Eo *obj, Emotion_Object_Data *sd, Eina_Bool play)
{
   DBG("play=" FMT_UCHAR ", was=" FMT_UCHAR, play, sd->play);
   if (!sd->engine_instance) return;
   if (!sd->open)
     {
        sd->remember_play = play;
        return;
     }
   if (play == sd->play) return;
   sd->play = play;
   sd->remember_play = play;
   if (sd->state != EMOTION_WAKEUP) emotion_object_suspend_set(obj, EMOTION_WAKEUP);
   if (sd->play) emotion_engine_instance_play(sd->engine_instance, sd->pos);
   else emotion_engine_instance_stop(sd->engine_instance);
}

EAPI Eina_Bool
emotion_object_play_get(const Evas_Object *obj)
{
   return eo_do(obj, efl_player_play_get());
}

EOLIAN static Eina_Bool
_emotion_object_efl_player_play_get(Eo *obj EINA_UNUSED, Emotion_Object_Data *sd)
{
   if (!sd->engine_instance) return EINA_FALSE;
   return sd->play;
}

EAPI void
emotion_object_position_set(Evas_Object *obj, double sec)
{
   eo_do(obj, efl_player_position_set(sec));
}

EOLIAN static void
_emotion_object_efl_player_position_set(Eo *obj, Emotion_Object_Data *sd, double sec)
{
   DBG("sec=%f", sec);
   if (!sd->engine_instance) return;
   if (!sd->open)
     {
        sd->remember_jump = sec;
        return;
     }
   sd->remember_jump = 0;
   sd->seek_pos = sec;
   sd->seek = 1;
   sd->pos = sd->seek_pos;
   if (sd->job) ecore_job_del(sd->job);
   sd->job = ecore_job_add(_pos_set_job, obj);
}

EAPI double
emotion_object_position_get(const Evas_Object *obj)
{
   return eo_do(obj, efl_player_position_get());
}

EOLIAN static double
_emotion_object_efl_player_position_get(Eo *obj EINA_UNUSED, Emotion_Object_Data *sd)
{
   if (!sd->engine_instance) return 0.0;
   sd->pos = emotion_engine_instance_pos_get(sd->engine_instance);
   return sd->pos;
}

EAPI double
emotion_object_buffer_size_get(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 1.0);
   if (!sd->engine_instance) return 0.0;
   return emotion_engine_instance_buffer_size_get(sd->engine_instance);
}

EAPI Eina_Bool
emotion_object_seekable_get(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->engine_instance) return EINA_FALSE;
   return emotion_engine_instance_seekable(sd->engine_instance);
}

EAPI Eina_Bool
emotion_object_video_handled_get(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->engine_instance) return EINA_FALSE;
   return emotion_engine_instance_video_handled(sd->engine_instance);
}

EAPI Eina_Bool
emotion_object_audio_handled_get(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->engine_instance) return EINA_FALSE;
   return emotion_engine_instance_audio_handled(sd->engine_instance);
}

EAPI double
emotion_object_play_length_get(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0.0);
   if (!sd->engine_instance) return 0.0;
   sd->len = emotion_engine_instance_len_get(sd->engine_instance);
   return sd->len;
}

EAPI void
emotion_object_size_get(const Evas_Object *obj, int *iw, int *ih)
{
   if (iw) *iw = 0;
   if (ih) *ih = 0;

   eo_do(obj, efl_image_load_size_get(iw, ih));
}

EOLIAN static void
_emotion_object_efl_image_load_size_get(Eo *obj EINA_UNUSED, Emotion_Object_Data *sd, int *iw, int *ih)
{
   if (iw) *iw = sd->video.w;
   if (ih) *ih = sd->video.h;
}

EAPI void
emotion_object_smooth_scale_set(Evas_Object *obj, Eina_Bool smooth)
{
   eo_do(obj, efl_image_smooth_scale_set(smooth));
}

EOLIAN static void
_emotion_object_efl_image_smooth_scale_set(Eo *obj EINA_UNUSED, Emotion_Object_Data *sd, Eina_Bool smooth)
{
   evas_object_image_smooth_scale_set(sd->obj, smooth);
}

EAPI Eina_Bool
emotion_object_smooth_scale_get(const Evas_Object *obj)
{
   return eo_do(obj, efl_image_smooth_scale_get());
}

EOLIAN static Eina_Bool
_emotion_object_efl_image_smooth_scale_get(Eo *obj EINA_UNUSED, Emotion_Object_Data *sd)
{
   return evas_object_image_smooth_scale_get(sd->obj);
}

EAPI double
emotion_object_ratio_get(const Evas_Object *obj)
{
   return eo_do(obj, efl_image_ratio_get());
}

EOLIAN static double
_emotion_object_efl_image_ratio_get(Eo *obj EINA_UNUSED, Emotion_Object_Data *sd)
{
   if (!sd->engine_instance) return 0.0;
   return sd->ratio;
}

/*
 * Send a control event to the DVD.
 */
EAPI void
emotion_object_event_simple_send(Evas_Object *obj, Emotion_Event ev)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->engine_instance) return;
   emotion_engine_instance_event_feed(sd->engine_instance, ev);
}

EAPI void
emotion_object_audio_volume_set(Evas_Object *obj, double vol)
{
   eo_do(obj, efl_player_audio_volume_set(vol));
}

EOLIAN static void
_emotion_object_efl_player_audio_volume_set(Eo *obj EINA_UNUSED, Emotion_Object_Data *sd, double vol)
{
   DBG("vol=%f", vol);
   if (!sd->engine_instance) return;
   emotion_engine_instance_audio_channel_volume_set(sd->engine_instance, vol);
}

EAPI double
emotion_object_audio_volume_get(const Evas_Object *obj)
{
   return eo_do(obj, efl_player_audio_volume_get());
}

EOLIAN static double
_emotion_object_efl_player_audio_volume_get(Eo *obj EINA_UNUSED, Emotion_Object_Data *sd)
{
   if (!sd->engine_instance) return 0.0;
   return emotion_engine_instance_audio_channel_volume_get(sd->engine_instance);
}

EAPI void
emotion_object_audio_mute_set(Evas_Object *obj, Eina_Bool mute)
{
   eo_do(obj, efl_player_audio_mute_set(mute));
}

EOLIAN static void
_emotion_object_efl_player_audio_mute_set(Eo *obj EINA_UNUSED, Emotion_Object_Data *sd, Eina_Bool mute)
{
   DBG("mute=" FMT_UCHAR, mute);
   if (!sd->engine_instance) return;
   emotion_engine_instance_audio_channel_mute_set(sd->engine_instance, mute);
}

EAPI Eina_Bool
emotion_object_audio_mute_get(const Evas_Object *obj)
{
   return eo_do(obj, efl_player_audio_mute_get());
}

EOLIAN static Eina_Bool
_emotion_object_efl_player_audio_mute_get(Eo *obj EINA_UNUSED, Emotion_Object_Data *sd)
{
   if (!sd->engine_instance) return EINA_FALSE;
   return emotion_engine_instance_audio_channel_mute_get(sd->engine_instance);
}

EAPI int
emotion_object_audio_channel_count(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->engine_instance) return 0;
   return emotion_engine_instance_audio_channel_count(sd->engine_instance);
}

EAPI const char *
emotion_object_audio_channel_name_get(const Evas_Object *obj, int channel)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, NULL);
   if (!sd->engine_instance) return NULL;
   return emotion_engine_instance_audio_channel_name_get(sd->engine_instance, channel);
}

EAPI void
emotion_object_audio_channel_set(Evas_Object *obj, int channel)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   DBG("channel=%d", channel);
   if (!sd->engine_instance) return;
   emotion_engine_instance_audio_channel_set(sd->engine_instance, channel);
}

EAPI int
emotion_object_audio_channel_get(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->engine_instance) return 0;
   return emotion_engine_instance_audio_channel_get(sd->engine_instance);
}

EAPI void
emotion_object_video_mute_set(Evas_Object *obj, Eina_Bool mute)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   DBG("mute=" FMT_UCHAR, mute);
   if (!sd->engine_instance) return;
   emotion_engine_instance_video_channel_mute_set(sd->engine_instance, mute);
}

EAPI Eina_Bool
emotion_object_video_mute_get(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->engine_instance) return EINA_FALSE;
   return emotion_engine_instance_video_channel_mute_get(sd->engine_instance);
}

EAPI void
emotion_object_video_subtitle_file_set(Evas_Object *obj, const char *filepath)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   DBG("subtitle=%s", filepath);
   if (!sd->engine_instance) _engine_init(obj, sd);
   if (!sd->engine_instance) return;
   emotion_engine_instance_video_subtitle_file_set(sd->engine_instance, filepath);
}

EAPI const char *
emotion_object_video_subtitle_file_get(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->engine_instance) return NULL;
   return emotion_engine_instance_video_subtitle_file_get(sd->engine_instance);
}

EAPI int
emotion_object_video_channel_count(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->engine_instance) return 0;
   return emotion_engine_instance_video_channel_count(sd->engine_instance);
}

EAPI const char *
emotion_object_video_channel_name_get(const Evas_Object *obj, int channel)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, NULL);
   if (!sd->engine_instance) return NULL;
   return emotion_engine_instance_video_channel_name_get(sd->engine_instance, channel);
}

EAPI void
emotion_object_video_channel_set(Evas_Object *obj, int channel)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   DBG("channel=%d", channel);
   if (!sd->engine_instance) return;
   emotion_engine_instance_video_channel_set(sd->engine_instance, channel);
}

EAPI int
emotion_object_video_channel_get(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->engine_instance) return 0;
   return emotion_engine_instance_video_channel_get(sd->engine_instance);
}

EAPI void
emotion_object_spu_mute_set(Evas_Object *obj, Eina_Bool mute)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   DBG("mute=" FMT_UCHAR, mute);
   if (!sd->engine_instance) return;
   emotion_engine_instance_spu_channel_mute_set(sd->engine_instance, mute);
}

EAPI Eina_Bool
emotion_object_spu_mute_get(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->engine_instance) return EINA_FALSE;
   return emotion_engine_instance_spu_channel_mute_get(sd->engine_instance);
}

EAPI int
emotion_object_spu_channel_count(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->engine_instance) return 0;
   return emotion_engine_instance_spu_channel_count(sd->engine_instance);
}

EAPI const char *
emotion_object_spu_channel_name_get(const Evas_Object *obj, int channel)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, NULL);
   if (!sd->engine_instance) return NULL;
   return emotion_engine_instance_spu_channel_name_get(sd->engine_instance, channel);
}

EAPI void
emotion_object_spu_channel_set(Evas_Object *obj, int channel)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   DBG("channel=%d", channel);
   if (!sd->engine_instance) return;
   emotion_engine_instance_spu_channel_set(sd->engine_instance, channel);
}

EAPI int
emotion_object_spu_channel_get(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->engine_instance) return 0;
   return emotion_engine_instance_spu_channel_get(sd->engine_instance);
}

EAPI int
emotion_object_chapter_count(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->engine_instance) return 0;
   return emotion_engine_instance_chapter_count(sd->engine_instance);
}

EAPI void
emotion_object_chapter_set(Evas_Object *obj, int chapter)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   DBG("chapter=%d", chapter);
   if (!sd->engine_instance) return;
   emotion_engine_instance_chapter_set(sd->engine_instance, chapter);
}

EAPI int
emotion_object_chapter_get(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->engine_instance) return 0;
   return emotion_engine_instance_chapter_get(sd->engine_instance);
}

EAPI const char *
emotion_object_chapter_name_get(const Evas_Object *obj, int chapter)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, NULL);
   if (!sd->engine_instance) return NULL;
   return emotion_engine_instance_chapter_name_get(sd->engine_instance, chapter);
}

EAPI void
emotion_object_play_speed_set(Evas_Object *obj, double speed)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   DBG("speed=%f", speed);
   if (!sd->engine_instance) return;
   emotion_engine_instance_speed_set(sd->engine_instance, speed);
}

EAPI double
emotion_object_play_speed_get(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0.0);
   if (!sd->engine_instance) return 0.0;
   return emotion_engine_instance_speed_get(sd->engine_instance);
}

EAPI void
emotion_object_eject(Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->engine_instance) return;
   emotion_engine_instance_eject(sd->engine_instance);
}

EAPI const char *
emotion_object_title_get(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, NULL);
   return sd->title;
}

EAPI const char *
emotion_object_progress_info_get(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, NULL);
   return sd->progress.info;
}

EAPI double
emotion_object_progress_status_get(const Evas_Object *obj)
{
   return eo_do(obj, efl_player_progress_get());
}

EOLIAN static double
_emotion_object_efl_player_progress_get(Eo *obj EINA_UNUSED, Emotion_Object_Data *sd)
{
   return sd->progress.stat;
}

EAPI const char *
emotion_object_ref_file_get(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, NULL);
   return sd->ref.file;
}

EAPI int
emotion_object_ref_num_get(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   return sd->ref.num;
}

EAPI int
emotion_object_spu_button_count_get(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   return sd->spu.button_num;
}

EAPI int
emotion_object_spu_button_get(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   return sd->spu.button;
}

EAPI const char *
emotion_object_meta_info_get(const Evas_Object *obj, Emotion_Meta_Info meta)
{
   Emotion_Object_Data *sd;
   int id;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, NULL);
   if (!sd->engine_instance) return NULL;
   switch (meta)
     {
      case EMOTION_META_INFO_TRACK_TITLE:
         id = META_TRACK_TITLE;
         break;
      case EMOTION_META_INFO_TRACK_ARTIST:
         id = META_TRACK_ARTIST;
         break;
      case EMOTION_META_INFO_TRACK_ALBUM:
         id = META_TRACK_ALBUM;
         break;
      case EMOTION_META_INFO_TRACK_YEAR:
         id = META_TRACK_YEAR;
         break;
      case EMOTION_META_INFO_TRACK_GENRE:
         id = META_TRACK_GENRE;
         break;
      case EMOTION_META_INFO_TRACK_COMMENT:
         id = META_TRACK_COMMENT;
         break;
      case EMOTION_META_INFO_TRACK_DISC_ID:
         id = META_TRACK_DISCID;
        break;
      default:
         ERR("Unknown meta info id: %d", meta);
         return NULL;
     }

   return emotion_engine_instance_meta_get(sd->engine_instance, id);
}

EAPI void
emotion_object_vis_set(Evas_Object *obj, Emotion_Vis visualization)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   DBG("visualization=%d", visualization);
   if (!sd->engine_instance) return;
   emotion_engine_instance_vis_set(sd->engine_instance, visualization);
}

EAPI Emotion_Vis
emotion_object_vis_get(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, EMOTION_VIS_NONE);
   if (!sd->engine_instance) return EMOTION_VIS_NONE;
   return emotion_engine_instance_vis_get(sd->engine_instance);
}

EAPI Eina_Bool
emotion_object_vis_supported(const Evas_Object *obj, Emotion_Vis visualization)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, EINA_FALSE);
   if (!sd->engine_instance) return EINA_FALSE;
   return emotion_engine_instance_vis_supported(sd->engine_instance, visualization);
}

EAPI void
emotion_object_priority_set(Evas_Object *obj, Eina_Bool priority)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->engine_instance) return;
   emotion_engine_instance_priority_set(sd->engine_instance, priority);
}

EAPI Eina_Bool
emotion_object_priority_get(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, EINA_FALSE);
   if (!sd->engine_instance) return EINA_FALSE;
   return emotion_engine_instance_priority_get(sd->engine_instance);
}

#ifdef HAVE_EIO
static void
_eio_load_xattr_cleanup(Emotion_Object_Data *sd, Eio_File *handler)
{
   if (handler == sd->load_xattr) sd->load_xattr = NULL;

   EINA_REFCOUNT_UNREF(sd)
     {
        if (sd->smartobj) evas_object_smart_data_set(sd->smartobj, NULL);
        sd->smartobj = NULL;
        _smart_data_free(sd);
     }
}

static void
_eio_load_xattr_done(void *data, Eio_File *handler, double xattr_double)
{
   Emotion_Object_Data *sd = data;

   emotion_object_position_set(evas_object_smart_parent_get(sd->obj), xattr_double);
   evas_object_smart_callback_call(evas_object_smart_parent_get(sd->obj), SIG_POSITION_LOAD_SUCCEED, NULL);
   _eio_load_xattr_cleanup(sd, handler);
}

static void
_eio_load_xattr_error(void *data, Eio_File *handler, int err EINA_UNUSED)
{
   Emotion_Object_Data *sd = data;

   evas_object_smart_callback_call(evas_object_smart_parent_get(sd->obj), SIG_POSITION_LOAD_FAILED, NULL);
   _eio_load_xattr_cleanup(sd, handler);
}
#endif

EAPI void
emotion_object_last_position_load(Evas_Object *obj)
{
   Emotion_Object_Data *sd;
   const char *tmp;
#ifndef HAVE_EIO
   double xattr;
#endif

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->file) return;

   if (!strncmp(sd->file, "file://", 7)) tmp = sd->file + 7;
   else if (!strstr(sd->file, "://")) tmp = sd->file;
   else return;

#ifdef HAVE_EIO
   if (sd->load_xattr) return;

   EINA_REFCOUNT_REF(sd);

   sd->load_xattr = eio_file_xattr_double_get(tmp,
                                              "user.e.time_seek",
                                              _eio_load_xattr_done,
                                              _eio_load_xattr_error,
                                              sd);
#else
   if (eina_xattr_double_get(tmp, "user.e.time_seek", &xattr))
     {
        emotion_object_position_set(obj, xattr);
        evas_object_smart_callback_call(obj, SIG_POSITION_LOAD_SUCCEED, NULL);
     }
   else
     evas_object_smart_callback_call(obj, SIG_POSITION_LOAD_FAILED, NULL);
#endif
}

#ifdef HAVE_EIO
static void
_eio_save_xattr_cleanup(Emotion_Object_Data *sd, Eio_File *handler)
{
   if (handler == sd->save_xattr) sd->save_xattr = NULL;

   EINA_REFCOUNT_UNREF(sd)
     {
        if (sd->smartobj) evas_object_smart_data_set(sd->smartobj, NULL);
        sd->smartobj = NULL;
        _smart_data_free(sd);
     }
}

static void
_eio_save_xattr_done(void *data, Eio_File *handler)
{
   Emotion_Object_Data *sd = data;

   evas_object_smart_callback_call(sd->obj, SIG_POSITION_SAVE_SUCCEED, NULL);
   _eio_save_xattr_cleanup(sd, handler);
}

static void
_eio_save_xattr_error(void *data, Eio_File *handler, int err EINA_UNUSED)
{
   Emotion_Object_Data *sd = data;

   evas_object_smart_callback_call(sd->obj, SIG_POSITION_SAVE_FAILED, NULL);
   _eio_save_xattr_cleanup(sd, handler);
}
#endif

EAPI void
emotion_object_last_position_save(Evas_Object *obj)
{
   Emotion_Object_Data *sd;
   const char *tmp;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->file) return;

   if (!strncmp(sd->file, "file://", 7)) tmp = sd->file + 7;
   else if (!strstr(sd->file, "://")) tmp = sd->file;
   else return;
#ifdef HAVE_EIO
   if (sd->save_xattr) return;

   EINA_REFCOUNT_REF(sd);
   sd->save_xattr = eio_file_xattr_double_set(tmp,
                                              "user.e.time_seek",
                                              emotion_object_position_get(obj),
                                              0,
                                              _eio_save_xattr_done,
                                              _eio_save_xattr_error,
                                              sd);
#else
   if (eina_xattr_double_set(tmp, "user.e.time_seek", emotion_object_position_get(obj), 0))
     evas_object_smart_callback_call(obj, SIG_POSITION_SAVE_SUCCEED, NULL);
   else
     evas_object_smart_callback_call(obj, SIG_POSITION_SAVE_FAILED, NULL);
#endif
}

EAPI void
emotion_object_suspend_set(Evas_Object *obj, Emotion_Suspend state)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   switch (state)
     {
      case EMOTION_WAKEUP:
         /* Restore the rendering pipeline, offset and everything back to play again (this will be called automatically by play_set) */
      case EMOTION_SLEEP:
         /* This destroy some part of the rendering pipeline */
      case EMOTION_DEEP_SLEEP:
         /* This destroy all the rendering pipeline and just keep the last rendered image (fullscreen) */
      case EMOTION_HIBERNATE:
         /* This destroy all the rendering pipeline and keep 1/4 of the last rendered image */
      default:
         break;
     }
   sd->state = state;
}

EAPI Emotion_Suspend
emotion_object_suspend_get(Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, EMOTION_WAKEUP);
   return sd->state;
}

/*****************************/
/* Utility calls for modules */
/*****************************/

EAPI void *
_emotion_video_get(const Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, NULL);
   return emotion_engine_instance_data_get(sd->engine_instance);
}

static Eina_Bool
_emotion_frame_anim(void *data)
{
   Evas_Object *obj = data;
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, EINA_FALSE);

   sd->anim = NULL;
   evas_object_image_pixels_dirty_set(sd->obj, 1);
   _emotion_video_pos_update(obj,
                             emotion_engine_instance_pos_get(sd->engine_instance),
                             emotion_engine_instance_len_get(sd->engine_instance));
   evas_object_smart_callback_call(obj, SIG_FRAME_DECODE, NULL);
   return EINA_FALSE;
}

EAPI void
_emotion_frame_new(Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->anim) sd->anim = ecore_animator_add(_emotion_frame_anim, obj);
}

EAPI void
_emotion_video_pos_update(Evas_Object *obj, double pos, double len)
{
   Emotion_Object_Data *sd;
   int npos = 0, nlen = 0;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (pos != sd->pos) npos = 1;
   if (len != sd->len) nlen = 1;
   sd->pos = pos;
   sd->len = len;
   if (npos) evas_object_smart_callback_call(obj, SIG_POSITION_UPDATE, NULL);
   if (nlen) evas_object_smart_callback_call(obj, SIG_LENGTH_CHANGE, NULL);
}

EAPI void
_emotion_frame_resize(Evas_Object *obj, int w, int h, double ratio)
{
   Emotion_Object_Data *sd;
   double tmp;
   int changed = 0;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if ((w != sd->video.w) || (h != sd->video.h))
     {
        sd->video.w = w;
        sd->video.h = h;
        _emotion_image_data_zero(sd->obj);
        changed = 1;
     }
   if (h > 0) tmp  = (double)w / (double)h;
   else tmp = 1.0;
   if (ratio != tmp) tmp = ratio;
   if (tmp != sd->ratio)
     {
        sd->ratio = tmp;
        changed = 1;
     }
   if (changed)
     {
        evas_object_size_hint_request_set(obj, w, h);
        evas_object_smart_callback_call(obj, SIG_FRAME_RESIZE, NULL);
        evas_object_geometry_get(obj, NULL, NULL, &w, &h);
        _emotion_object_aspect_border_apply(obj, sd, w, h);
     }
}

EAPI void
_emotion_image_reset(Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   _emotion_image_data_zero(sd->obj);
}

EAPI void
_emotion_decode_stop(Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (sd->play)
     {
        sd->play = 0;
        evas_object_smart_callback_call(obj, SIG_DECODE_STOP, NULL);
     }
}

EAPI void
_emotion_open_done(Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   sd->open = 1;

   if (sd->remember_jump)
     emotion_object_position_set(obj, sd->remember_jump);
   if (sd->remember_play != sd->play)
     emotion_object_play_set(obj, sd->remember_play);
   evas_object_smart_callback_call(obj, SIG_OPEN_DONE, NULL);
}

EAPI void
_emotion_playback_started(Evas_Object *obj)
{
   evas_object_smart_callback_call(obj, SIG_PLAYBACK_STARTED, NULL);
}

EAPI void
_emotion_playback_finished(Evas_Object *obj)
{
   evas_object_smart_callback_call(obj, SIG_PLAYBACK_FINISHED, NULL);
}

EAPI void
_emotion_audio_level_change(Evas_Object *obj)
{
   evas_object_smart_callback_call(obj, SIG_AUDIO_LEVEL_CHANGE, NULL);
}

EAPI void
_emotion_channels_change(Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   evas_object_smart_callback_call(obj, SIG_CHANNELS_CHANGE, NULL);
}

EAPI void
_emotion_title_set(Evas_Object *obj, char *title)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   eina_stringshare_replace(&sd->title, title);
   evas_object_smart_callback_call(obj, SIG_TITLE_CHANGE, NULL);
}

EAPI void
_emotion_progress_set(Evas_Object *obj, char *info, double st)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   eina_stringshare_replace(&sd->progress.info, info);
   sd->progress.stat = st;
   evas_object_smart_callback_call(obj, SIG_PROGRESS_CHANGE, NULL);
}

EAPI void
_emotion_file_ref_set(Evas_Object *obj, const char *file, int num)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   eina_stringshare_replace(&sd->ref.file, file);
   sd->ref.num = num;
   evas_object_smart_callback_call(obj, SIG_REF_CHANGE, NULL);
}

EAPI void
_emotion_spu_button_num_set(Evas_Object *obj, int num)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   sd->spu.button_num = num;
   evas_object_smart_callback_call(obj, SIG_BUTTON_NUM_CHANGE, NULL);
}

EAPI void
_emotion_spu_button_set(Evas_Object *obj, int button)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   sd->spu.button = button;
   evas_object_smart_callback_call(obj, SIG_BUTTON_CHANGE, NULL);
}

EAPI void
_emotion_seek_done(Evas_Object *obj)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (sd->seeking)
     {
        sd->seeking = 0;
        if (sd->seek) emotion_object_position_set(obj, sd->seek_pos);
     }
}

EAPI void
_emotion_frame_refill(Evas_Object *obj, double w, double h)
{
   Emotion_Object_Data *sd;

   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if ((sd->fill.w != w) || (sd->fill.h != h))
     {
        Evas_Coord ow, oh;

        evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
        if ((w <= 0) || (h <= 0))
          {
             double scale_w, scale_h;

             sd->fill.w = -1;
             sd->fill.h = -1;

             scale_w = (double)ow / (double)(sd->video.w - sd->crop.l - sd->crop.r);
             scale_h = (double)oh / (double)(sd->video.h - sd->crop.t - sd->crop.b);
             evas_object_image_fill_set(sd->obj, 0, 0, scale_w * sd->video.w, scale_h * sd->video.h);
          }
        else
          {
             sd->fill.w = w;
             sd->fill.h = h;
             evas_object_image_fill_set(sd->obj, 0, 0, w * ow, h * oh);
          }
     }
}

/****************************/
/* Internal object routines */
/****************************/

static void
_mouse_move(void *data, Evas *ev EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Move *e;
   Emotion_Object_Data *sd;
   int x, y, iw, ih;
   Evas_Coord ox, oy, ow, oh;

   e = event_info;
   sd = data;
   if (!sd->engine_instance) return;
   evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);
   evas_object_image_size_get(obj, &iw, &ih);
   if ((iw < 1) || (ih < 1)) return;
   x = (((int)e->cur.canvas.x - ox) * iw) / ow;
   y = (((int)e->cur.canvas.y - oy) * ih) / oh;
   emotion_engine_instance_event_mouse_move_feed(sd->engine_instance, x, y);
}

static void
_mouse_down(void *data, Evas *ev EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *e;
   Emotion_Object_Data *sd;
   int x, y, iw, ih;
   Evas_Coord ox, oy, ow, oh;

   e = event_info;
   sd = data;
   if (!sd->engine_instance) return;
   evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);
   evas_object_image_size_get(obj, &iw, &ih);
   if ((iw < 1) || (ih < 1)) return;
   x = (((int)e->canvas.x - ox) * iw) / ow;
   y = (((int)e->canvas.y - oy) * ih) / oh;
   emotion_engine_instance_event_mouse_button_feed(sd->engine_instance, 1, x, y);
}

static void
_pos_set_job(void *data)
{
   Evas_Object *obj;
   Emotion_Object_Data *sd;

   obj = data;
   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   sd->job = NULL;
   if (!sd->engine_instance) return;
   if (sd->seeking) return;
   if (sd->seek)
     {
        sd->seeking = 1;
        emotion_engine_instance_pos_set(sd->engine_instance, sd->seek_pos);
        sd->seek = 0;
     }
}

/* called by evas when it needs pixels for the image object */
static void
_pixels_get(void *data, Evas_Object *obj)
{
   int iw, ih, w, h;
   Emotion_Object_Data *sd;
   Emotion_Format format;
   unsigned char *bgra_data;

   sd = data;
   if (!sd->engine_instance) return;
   emotion_engine_instance_video_data_size_get(sd->engine_instance, &w, &h);
   w = (w >> 1) << 1;
   h = (h >> 1) << 1;

   evas_object_image_colorspace_set(obj, EVAS_COLORSPACE_YCBCR422P601_PL);
   evas_object_image_alpha_set(obj, 0);
   evas_object_image_size_set(obj, w, h);
   iw = w;
   ih = h;

   if ((iw <= 1) || (ih <= 1))
     {
        _emotion_image_data_zero(sd->obj);
        evas_object_image_pixels_dirty_set(obj, 0);
     }
   else
     {
        format = emotion_engine_instance_format_get(sd->engine_instance);
        if ((format == EMOTION_FORMAT_YV12) || (format == EMOTION_FORMAT_I420))
          {
             unsigned char **rows;

             evas_object_image_colorspace_set(obj, EVAS_COLORSPACE_YCBCR422P601_PL);
             rows = evas_object_image_data_get(obj, 1);
             if (rows)
               {
                  if (emotion_engine_instance_yuv_rows_get(sd->engine_instance, iw, ih,
                                                           rows,
                                                           &rows[ih],
                                                           &rows[ih + (ih / 2)]))
                  evas_object_image_data_update_add(obj, 0, 0, iw, ih);
               }
             evas_object_image_data_set(obj, rows);
             evas_object_image_pixels_dirty_set(obj, 0);
          }
        else if (format == EMOTION_FORMAT_BGRA)
          {
             evas_object_image_colorspace_set(obj, EVAS_COLORSPACE_ARGB8888);
             if (emotion_engine_instance_bgra_data_get(sd->engine_instance, &bgra_data))
               {
                  evas_object_image_data_set(obj, bgra_data);
                  evas_object_image_pixels_dirty_set(obj, 0);
               }
          }
     }
}

/*******************************************/
/* Internal smart object required routines */
/*******************************************/

EOLIAN static void
_emotion_object_evas_object_smart_add(Evas_Object *obj, Emotion_Object_Data *sd)
{
   unsigned int *pixel;

   /* TODO: remove legacy: emotion used to have no init, call automatically */
   emotion_init();

   EINA_REFCOUNT_INIT(sd);
   sd->state = EMOTION_WAKEUP;
   sd->smartobj = obj;
   sd->obj = evas_object_image_add(evas_object_evas_get(obj));
   sd->bg = evas_object_rectangle_add(evas_object_evas_get(obj));
   sd->engine = eina_stringshare_add("gstreamer1");
   evas_object_color_set(sd->bg, 0, 0, 0, 0);
   evas_object_event_callback_add(sd->obj, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move, sd);
   evas_object_event_callback_add(sd->obj, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down, sd);
   evas_object_image_pixels_get_callback_set(sd->obj, _pixels_get, sd);
   evas_object_smart_member_add(sd->obj, obj);
   evas_object_smart_member_add(sd->bg, obj);
   evas_object_lower(sd->bg);
   sd->ratio = 1.0;
   sd->spu.button = -1;
   sd->fill.w = -1;
   sd->fill.h = -1;
   evas_object_image_alpha_set(sd->obj, 0);
   pixel = evas_object_image_data_get(sd->obj, 1);
   if (pixel)
     {
        *pixel = 0xff000000;
        evas_object_image_data_set(obj, pixel);
     }
   evas_object_smart_data_set(obj, sd);
}

EOLIAN static void
_emotion_object_evas_object_smart_del(Evas_Object *obj EINA_UNUSED, Emotion_Object_Data *sd)
{
   if (!sd) return;
   if (sd->engine_instance)
     {
        emotion_engine_instance_file_close(sd->engine_instance);
        emotion_engine_instance_del(sd->engine_instance);
     }
   sd->engine_instance = NULL;
   if (sd->obj) evas_object_del(sd->obj);
   sd->obj = NULL;
   if (sd->crop.clipper) evas_object_del(sd->crop.clipper);
   sd->crop.clipper = NULL;
   if (sd->bg) evas_object_del(sd->bg);
   sd->bg = NULL;
   if (sd->file) eina_stringshare_del(sd->file);
   sd->file = NULL;
   if (sd->job) ecore_job_del(sd->job);
   sd->job = NULL;
   if (sd->anim) ecore_animator_del(sd->anim);
   sd->anim = NULL;
   eina_stringshare_del(sd->progress.info);
   sd->progress.info = NULL;
   eina_stringshare_del(sd->ref.file);
   sd->ref.file = NULL;
   if (sd->smartobj) evas_object_smart_data_set(sd->smartobj, NULL);
   sd->smartobj = NULL;
   EINA_REFCOUNT_UNREF(sd) _smart_data_free(sd);
}

EOLIAN static void
_emotion_object_evas_object_smart_move(Evas_Object *obj, Emotion_Object_Data *sd, Evas_Coord x, Evas_Coord y)
{
   int w, h;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   _clipper_position_size_update(obj, x, y, w, h, sd->video.w, sd->video.h);
   evas_object_move(sd->bg, x, y);
}

EOLIAN static void
_emotion_object_evas_object_smart_resize(Evas_Object *obj, Emotion_Object_Data *sd, Evas_Coord w, Evas_Coord h)
{
   _emotion_object_aspect_border_apply(obj, sd, w, h);
   evas_object_resize(sd->bg, w, h);
}

EOLIAN static void
_emotion_object_evas_object_smart_show(Evas_Object *obj EINA_UNUSED, Emotion_Object_Data *sd)
{
   int a;

   evas_object_show(sd->obj);
   if (sd->crop.clipper) evas_object_show(sd->crop.clipper);

   evas_object_color_get(sd->bg, NULL, NULL, NULL, &a);
   if (a > 0) evas_object_show(sd->bg);
}

EOLIAN static void
_emotion_object_evas_object_smart_hide(Evas_Object *obj EINA_UNUSED, Emotion_Object_Data *sd)
{
   evas_object_hide(sd->obj);
   if (sd->crop.clipper) evas_object_hide(sd->crop.clipper);
   evas_object_hide(sd->bg);
}

EOLIAN static void
_emotion_object_evas_object_smart_color_set(Evas_Object *obj EINA_UNUSED, Emotion_Object_Data *sd, int r, int g, int b, int a)
{
   evas_object_color_set(sd->obj, r, g, b, a);
   evas_object_color_set(sd->crop.clipper, r, g, b, a);
}

EOLIAN static void
_emotion_object_evas_object_smart_clip_set(Evas_Object *obj EINA_UNUSED, Emotion_Object_Data *sd, Evas_Object * clip)
{
   if (sd->crop.clipper) evas_object_clip_set(sd->crop.clipper, clip);
   else evas_object_clip_set(sd->obj, clip);
   evas_object_clip_set(sd->bg, clip);
}

EOLIAN static void
_emotion_object_evas_object_smart_clip_unset(Evas_Object *obj EINA_UNUSED, Emotion_Object_Data *sd)
{
   if (sd->crop.clipper) evas_object_clip_unset(sd->crop.clipper);
   else evas_object_clip_unset(sd->obj);
   evas_object_clip_unset(sd->bg);
}

#include "emotion_object.eo.c"
