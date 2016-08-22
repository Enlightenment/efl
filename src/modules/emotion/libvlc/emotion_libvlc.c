#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>

#include <Eina.h>
#include <Evas.h>
#include <Ecore.h>

#include <vlc/vlc.h>
#include <vlc/libvlc_media.h>
#include <vlc/libvlc_media_player.h>

#include "emotion_modules.h"

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_emotion_libvlc_log_domain, __VA_ARGS__)

#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_emotion_libvlc_log_domain, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_emotion_libvlc_log_domain, __VA_ARGS__)

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_emotion_libvlc_log_domain, __VA_ARGS__)

#ifdef CRI
#undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_emotion_libvlc_log_domain, __VA_ARGS__)

static int _emotion_libvlc_log_domain = -1;
static Eina_Bool debug_fps = EINA_FALSE;
static libvlc_instance_t *libvlc = NULL;

typedef struct _Emotion_LibVLC Emotion_LibVLC;

static void em_file_close(void *);

struct _Emotion_LibVLC
{
   /* Evas object */
   Evas_Object                   *obj;
   Evas_Object                   *evas_obj;
   Emotion_Module_Options         opt;

   /* libvlc */
   libvlc_media_t                *m;
   libvlc_media_player_t         *mp;
   unsigned int                   nb_tracks;
   libvlc_media_track_t         **tracks;
   int                            nb_chapters;
   libvlc_chapter_description_t **chapters;
   char                          *subtitle_file;
   char                          *metas[META_TRACK_COUNT];

   /* options */
   int                            video_mute;
   int                            video_mute_force;
   int                            audio_mute;
   int                            spu_mute;
   int                            audio_vol;
   Emotion_Vis                    vis;

   /* There can be remaining mainloop callbacks that owns the Emotion_LibVLC
    * object when em_del is called. Use a ref_count to delete the ev object
    * when em_del is called and when all callbacks are processed. */
   unsigned int                   ref_count;

   /* locks */
   Eina_Lock                      lock;
   Eina_Condition                 wait;
   Eina_List                     *event_list;

   /* stats */
   double                         pos;
   double                         len;
   double                         buffer_cache;
   Eina_Bool                      seeking;
   Eina_Bool                      started;
   Eina_Bool                      invalidate_tracks;
};

struct close_data
{
   libvlc_media_player_t *mp;
   Evas_Object           *evas_obj;
};

static const libvlc_event_type_t mp_events[] = {
   //libvlc_MediaPlayerMediaChanged,
   //libvlc_MediaPlayerNothingSpecial,
   //libvlc_MediaPlayerOpening,
   libvlc_MediaPlayerBuffering,
   libvlc_MediaPlayerPlaying,
   //libvlc_MediaPlayerPaused,
   libvlc_MediaPlayerStopped,
   libvlc_MediaPlayerForward,
   //libvlc_MediaPlayerBackward,
   libvlc_MediaPlayerEndReached,
   libvlc_MediaPlayerEncounteredError,
   libvlc_MediaPlayerTimeChanged,
   //libvlc_MediaPlayerPositionChanged,
   //libvlc_MediaPlayerSeekableChanged,
   //libvlc_MediaPlayerPausableChanged,
   //libvlc_MediaPlayerTitleChanged,
   //libvlc_MediaPlayerSnapshotTaken,
   libvlc_MediaPlayerLengthChanged,
   //libvlc_MediaPlayerVout,
   //libvlc_MediaPlayerScrambledChanged,
   libvlc_MediaPlayerESAdded,
   libvlc_MediaPlayerESDeleted,
   //libvlc_MediaPlayerESSelected,
   //libvlc_MediaPlayerCorked,
   //libvlc_MediaPlayerUncorked,
   //libvlc_MediaPlayerMuted,
   //libvlc_MediaPlayerUnmuted,
   //libvlc_MediaPlayerAudioVolume,
   //libvlc_MediaPlayerAudioDevice,
   -1,
};

static void
em_del_safe(Emotion_LibVLC *ev)
{
   eina_lock_free(&ev->lock);
   eina_condition_free(&ev->wait);
   free(ev->subtitle_file);
   free(ev);
}

/* Take the ev->lock from a mainloop callback.
 * Returns false if the ev object is destroyed. */
static Eina_Bool
emotion_mainloop_lock(Emotion_LibVLC *ev)
{
   eina_lock_take(&ev->lock);
   _emotion_pending_ecore_end();
   if (--ev->ref_count == 0)
     {
        eina_lock_release(&ev->lock);
        WRN("callbacks ended, deleting Emotion_LibVLC");
        em_del_safe(ev);
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

/* Send a callback to the mainloop */
static void
emotion_mainloop_call_locked(Emotion_LibVLC *ev, Ecore_Cb callback)
{
   ++ev->ref_count;
   _emotion_pending_ecore_begin();
   ecore_main_loop_thread_safe_call_async(callback, ev);
}

/* Process one libvlc event from the mainloop. */
static void
emotion_mainloop_event(Emotion_LibVLC *ev, const libvlc_event_t *event)
{
   switch (event->type)
     {
      case libvlc_MediaPlayerBuffering:
        ev->buffer_cache = event->u.media_player_buffering.new_cache / 100.0;
        break;

      case libvlc_MediaPlayerPlaying:
        if (!ev->started)
          {
             _emotion_open_done(ev->obj);
             _emotion_playback_started(ev->obj);
             ev->started = EINA_TRUE;
          }
        break;

      case libvlc_MediaPlayerStopped:
      case libvlc_MediaPlayerEndReached:
      case libvlc_MediaPlayerEncounteredError:
        _emotion_decode_stop(ev->obj);
        _emotion_playback_finished(ev->obj);
        break;

      case libvlc_MediaPlayerTimeChanged:
      {
         if (ev->seeking)
           {
              _emotion_seek_done(ev->obj);
              ev->seeking = EINA_FALSE;
           }

         ev->pos = event->u.media_player_time_changed.new_time / 1000.0;
         if (ev->pos > 0 && ev->len > 0)
           _emotion_video_pos_update(ev->obj, ev->pos, ev->len);
         break;
      }

      case libvlc_MediaPlayerLengthChanged:
        ev->len = event->u.media_player_length_changed.new_length / 1000.0;
        if (ev->pos > 0 && ev->len > 0)
          _emotion_video_pos_update(ev->obj, ev->pos, ev->len);
        break;

      case libvlc_MediaPlayerESAdded:
      case libvlc_MediaPlayerESDeleted:
        ev->invalidate_tracks = EINA_TRUE;
        _emotion_channels_change(ev->obj);
        break;
     }
}

/* Mainloop callback, sent by libvlc_on_mp_event. It processes a list of libvlc
 * event. */
static void
emotion_mainloop_event_list(void *data)
{
   Emotion_LibVLC *ev = data;
   Eina_List *event_list;
   libvlc_event_t *event;

   if (!emotion_mainloop_lock(ev)) return;
   event_list = ev->event_list;
   ev->event_list = NULL;
   eina_lock_release(&ev->lock);

   if (!event_list) return;

   EINA_LIST_FREE(event_list, event)
     {
        if (ev->mp)
          emotion_mainloop_event(ev, event);
        free(event);
     }
}

/* Libvlc callback, see libvlc_event_manager_t. */
static void
libvlc_on_mp_event(const libvlc_event_t *event, void *opaque)
{
   Emotion_LibVLC *ev = opaque;

   if (eina_main_loop_is())
     {
        /* Process the event directly */
        emotion_mainloop_event(ev, event);
     }
   else
     {
        /* Add the event to a list of events that will be processed by the
         * mainloop */

        void *data = malloc(sizeof(libvlc_event_t));
        if (!data) return;
        memcpy(data, event, sizeof(libvlc_event_t));

        eina_lock_take(&ev->lock);
        if (!ev->event_list)
          emotion_mainloop_call_locked(ev, emotion_mainloop_event_list);
        ev->event_list = eina_list_append(ev->event_list, data);
        eina_lock_release(&ev->lock);
     }
}

static void
evas_resize_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
               void *event EINA_UNUSED)
{
   Emotion_LibVLC *ev = data;
   int w, h;

   evas_object_image_size_get(ev->evas_obj, &w, &h);
   _emotion_frame_resize(ev->obj, w, h, w / (double) h);
   efl_event_callback_legacy_call(ev->obj, EMOTION_OBJECT_EVENT_FRAME_DECODE, NULL);
}

/* Fetch all libvlc tracks. */
static int
libvlc_fetch_tracks(Emotion_LibVLC *ev)
{
   if (ev->invalidate_tracks)
     {
        if (ev->nb_tracks)
          libvlc_media_tracks_release(ev->tracks, ev->nb_tracks);
        ev->nb_tracks = libvlc_media_tracks_get(ev->m, &ev->tracks);
        ev->invalidate_tracks = EINA_FALSE;
     }
   return ev->nb_tracks;
}

/* Get a libvlc tracks from a track_id. */
static libvlc_media_track_t *
libvlc_get_track(Emotion_LibVLC *ev, libvlc_track_type_t type, int id)
{
   unsigned int i;

   if (!ev->m || id < 0 || type == libvlc_track_unknown) return NULL;

   if (!libvlc_fetch_tracks(ev)) return NULL;

   for (i = 0; i < ev->nb_tracks; ++i)
     {
        libvlc_media_track_t *track = ev->tracks[i];

        if (track->i_id == id && track->i_type == type)
          return track;
     }

   return NULL;
}

/* Get the current libvlc video track. */
static libvlc_media_track_t *
libvlc_get_current_video_track(Emotion_LibVLC *ev)
{
   int id = libvlc_video_get_track(ev->mp);
   return id >= 0 ? libvlc_get_track(ev, libvlc_track_video, id) : NULL;
}

/* Get a libvlc video track at a pos.
 * XXX: Libvlc use a track_id to get and select a track. The first track_id doesn't
 * necessarily starts with 0. Emotion use a position (that starts with 0) to
 * get and select a track. */
static libvlc_media_track_t *
libvlc_get_track_at_pos(Emotion_LibVLC *ev,
                        int pos, libvlc_track_type_t type)
{
   unsigned int i;

   if (!ev->m || pos < 0 || type == libvlc_track_unknown) return NULL;

   if (!libvlc_fetch_tracks(ev)) return NULL;

   for (i = 0; i < ev->nb_tracks; ++i)
     {
        libvlc_media_track_t *track = ev->tracks[i];

        if (type == track->i_type && pos-- == 0)
          return track;
     }

   return NULL;
}

/* Get the position of the libvlc track.
 * See libvlc_get_track_at_pos. */
static int
libvlc_get_track_pos(Emotion_LibVLC *ev, int id, libvlc_track_type_t type)
{
   unsigned int i;
   int pos = 0;

   if (!ev->m || id < 0 || type == libvlc_track_unknown) return -1;

   if (!libvlc_fetch_tracks(ev)) return -1;

   for (i = 0; i < ev->nb_tracks; ++i)
     {
        libvlc_media_track_t *track = ev->tracks[i];

        if (type == track->i_type)
          {
             if (id == track->i_id)
               return pos;
             else
               pos++;
          }
     }

   return -1;
}

static void *
em_add(const Emotion_Engine *api EINA_UNUSED,
       Evas_Object *obj,
       const Emotion_Module_Options *opt)
{
   Emotion_LibVLC *ev;
   ev = calloc(1, sizeof(Emotion_LibVLC));
   EINA_SAFETY_ON_NULL_RETURN_VAL(ev, NULL);

   ev->obj = obj;
   ev->opt = *opt;
   eina_lock_new(&ev->lock);
   eina_condition_new(&ev->wait, &ev->lock);
   ev->ref_count = 1;
   ev->audio_vol = -1;

   return ev;
}

static void
em_del(void *video)
{
   Emotion_LibVLC *ev = video;

   if (!ev) return;

   em_file_close(video);

   eina_lock_take(&ev->lock);
   if (--ev->ref_count > 0)
     {
        eina_lock_release(&ev->lock);
        WRN("em_del delayed, some callbacks are still running");
     }
   else
     {
        eina_lock_release(&ev->lock);
        em_del_safe(ev);
     }
}

static Eina_Bool
em_file_open(void *video,
             const char *file)
{
   int ret, i;
   Emotion_LibVLC *ev = video;
   libvlc_event_manager_t *event_m;

   if (!file) return EINA_FALSE;

   ev->evas_obj = emotion_object_image_get(ev->obj);
   if (!ev->evas_obj)
     {
        WRN("emotion_object_image_get failed: no video");
        ev->opt.no_video = EINA_TRUE;
     }

   evas_object_image_pixels_get_callback_set(ev->evas_obj, NULL, NULL);

   ev->invalidate_tracks = true;

   /* Create libvlc_media */
   ev->m = libvlc_media_new_path(libvlc, file);
   if (strstr(file, "://") == NULL)
     ev->m = libvlc_media_new_path(libvlc, file);
   else
     ev->m = libvlc_media_new_location(libvlc, file);

   EINA_SAFETY_ON_NULL_GOTO(ev->m, error);

   if (ev->opt.no_audio || ev->audio_mute)
     libvlc_media_add_option(ev->m, ":no-audio");

   if (ev->opt.no_video || ev->video_mute)
     libvlc_media_add_option(ev->m, ":no-video");

   if (ev->spu_mute)
     libvlc_media_add_option(ev->m, ":no-spu");

   /* Create libvlc_media_player */
   ev->mp = libvlc_media_player_new_from_media(ev->m);
   EINA_SAFETY_ON_NULL_GOTO(ev->mp, error);

   event_m = libvlc_media_player_event_manager(ev->mp);
   for (i = 0; mp_events[i] != -1; ++i)
     libvlc_event_attach(event_m, mp_events[i], libvlc_on_mp_event, ev);

   libvlc_media_player_set_video_title_display(ev->mp,
                                               libvlc_position_disable, 0);

   evas_object_ref(ev->evas_obj);
   if (libvlc_media_player_set_evas_object(ev->mp, ev->evas_obj) == -1)
     {
        CRI("libvlc_media_player_set_evas_object failed");
        libvlc_media_add_option(ev->m, ":no-video");
        ev->video_mute = ev->video_mute_force = 1;
     }

   evas_object_event_callback_add(ev->evas_obj, EVAS_CALLBACK_IMAGE_RESIZE,
                                  evas_resize_cb, ev);

   if (ev->audio_vol != -1)
     libvlc_audio_set_volume(ev->mp, ev->audio_vol);

   ret = libvlc_media_player_play(ev->mp);
   EINA_SAFETY_ON_FALSE_GOTO(ret == 0, error);

   return EINA_TRUE;
error:
   em_file_close(video);
   return EINA_FALSE;
}

static void
emotion_close_cb(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   struct close_data *close_data = data;

   libvlc_media_player_release(close_data->mp);
}

static void
emotion_close_mainloop_cb(void *data,
                                     Ecore_Thread *thread EINA_UNUSED)
{
   struct close_data *close_data = data;

   evas_object_unref(close_data->evas_obj);
   free(close_data);
   _emotion_pending_ecore_end();
}

static void
em_file_close(void *video)
{
   Emotion_LibVLC *ev = video;
   unsigned int i;

   if (ev->mp)
     {
        struct close_data *close_data;
        libvlc_event_manager_t *event_m;

        evas_object_event_callback_del(ev->evas_obj, EVAS_CALLBACK_IMAGE_RESIZE,
                                       evas_resize_cb);

        event_m = libvlc_media_player_event_manager(ev->mp);
        for (i = 0; mp_events[i] != -1; ++i)
          libvlc_event_detach(event_m, mp_events[i], libvlc_on_mp_event, ev);

        libvlc_media_player_set_evas_object(ev->mp, NULL);

        close_data = malloc(sizeof(struct close_data));
        if (close_data)
          {
             close_data->evas_obj = ev->evas_obj;
             close_data->mp = ev->mp;
             _emotion_pending_ecore_begin();
             ecore_thread_run(emotion_close_cb,
                              emotion_close_mainloop_cb,
                              NULL, close_data);
          }

        ev->evas_obj = NULL;
        ev->mp = NULL;

        if (ev->seeking)
          {
             ev->seeking = EINA_FALSE;
             _emotion_seek_done(ev->obj);
          }
     }
   if (ev->m)
     {
        libvlc_media_release(ev->m);
        ev->m = NULL;
     }
   if (ev->nb_tracks > 0)
     {
        libvlc_media_tracks_release(ev->tracks, ev->nb_tracks);
        ev->nb_tracks = 0;
        ev->tracks = NULL;
     }
   if (ev->nb_chapters > 0)
     {
        libvlc_chapter_descriptions_release(ev->chapters, ev->nb_chapters);
        ev->nb_chapters = 0;
        ev->chapters = NULL;
     }
   for (i = 0; i < META_TRACK_COUNT; ++i)
     {
        free(ev->metas[i]);
        ev->metas[i] = NULL;
     }
   if (ev->subtitle_file)
     {
        free(ev->subtitle_file);
        ev->subtitle_file = NULL;
     }
   ev->vis = EMOTION_VIS_NONE;
   ev->started = ev->seeking = ev->invalidate_tracks = EINA_FALSE;
   ev->pos = ev->len = ev->buffer_cache = 0.0;
}

static void
em_play(void *video,
        double pos EINA_UNUSED)
{
   Emotion_LibVLC *ev = video;

   libvlc_media_player_set_pause(ev->mp, false);
}

static void
em_stop(void *video)
{
   Emotion_LibVLC *ev = video;

   libvlc_media_player_set_pause(ev->mp, true);
}

static void
em_size_get(void *video,
            int *width,
            int *height)
{
   Emotion_LibVLC *ev = video;
   libvlc_media_track_t *track;

   if (!width || !height) return;
   *width = 0;
   *height = 0;

   if (!ev->started) return;

   track = libvlc_get_current_video_track(ev);
   if (track)
     {
        *width = track->video->i_width;
        *height = track->video->i_height;
     }
}

static double
em_pos_get(void *video)
{
   Emotion_LibVLC *ev = video;

   if (!ev->started) return 0.0;

   return ev->pos;
}

static void
em_pos_set(void *video,
           double pos)
{
   Emotion_LibVLC *ev = video;

   if (!ev->started)
     {
        _emotion_seek_done(ev->obj);
        return;
     }

   libvlc_media_player_set_time(ev->mp, pos * 1000);

   ev->seeking = EINA_TRUE;
}

static double
em_len_get(void *video)
{
   Emotion_LibVLC *ev = video;

   if (!ev->started) return 0.0;

   return ev->len;
}

static double
em_buffer_size_get(void *video)
{
   Emotion_LibVLC *ev = video;

   if (!ev->started) return 0.0;

   return ev->buffer_cache;
}

static void
em_fps_num_den_get(Emotion_LibVLC *ev, int *num, int *den)
{
   libvlc_media_track_t *track;

   if (!ev->started) return;

   track = libvlc_get_current_video_track(ev);
   if (track)
     {
        if (num)
          *num = track->video->i_frame_rate_num;
        if (den)
          *den = track->video->i_frame_rate_den;
     }
}

static int
em_fps_num_get(void *video)
{
   int num = 0;

   em_fps_num_den_get(video, &num, NULL);
   return num;
}

static int
em_fps_den_get(void *video)
{
   int den = 0;

   em_fps_num_den_get(video, NULL, &den);
   return den;
}

static double
em_fps_get(void *video)
{
   int num = 0, den = 0;

   em_fps_num_den_get(video, &num, &den);
   if (den > 0) return num / (double)den;
   return 0.0;
}

static void
em_vis_set(void *video,
           Emotion_Vis vis)
{
   Emotion_LibVLC *ev = video;

   ev->vis = vis;
}

static Emotion_Vis
em_vis_get(void *video)
{
   Emotion_LibVLC *ev = video;

   return ev->vis;
}

static Eina_Bool
em_vis_supported(void *ev EINA_UNUSED, Emotion_Vis vis)
{
   /* FIXME */
   if (vis == EMOTION_VIS_NONE)
     return EINA_TRUE;
   else
     return EINA_FALSE;
}

static double
em_ratio_get(void *video)
{
   Emotion_LibVLC *ev = video;
   libvlc_media_track_t *track;

   if (!ev->started) return 0.0;

   track = libvlc_get_current_video_track(ev);
   if (track)
     {
        double ratio = track->video->i_sar_num
          / (double)track->video->i_sar_den;
        return ratio;
     }
   else
     return 0.0;
}

static int
em_video_handled(void *video)
{
   Emotion_LibVLC *ev = video;

   if (!ev->started) return 0.0;

   return libvlc_video_get_track_count(ev->mp) > 0;
}

static int
em_audio_handled(void *video)
{
   Emotion_LibVLC *ev = video;

   if (!ev->started) return 0.0;

   return libvlc_audio_get_track_count(ev->mp) > 0;
}

static int
em_seekable(void *video)
{
   Emotion_LibVLC *ev = video;

   if (!ev->started) return 0;

   return libvlc_media_player_is_seekable(ev->mp);
}

static void
em_frame_done(void *video EINA_UNUSED)
{
}

static Emotion_Format
em_format_get(void *video EINA_UNUSED)
{
   return EMOTION_FORMAT_NONE;
}

static void
em_videfl_data_size_get(void *video EINA_UNUSED, int *w EINA_UNUSED, int *h EINA_UNUSED)
{
}

static int
em_yuv_rows_get(void *video EINA_UNUSED,
                int w EINA_UNUSED,
                int h EINA_UNUSED,
                unsigned char **yrows EINA_UNUSED,
                unsigned char **urows EINA_UNUSED,
                unsigned char **vrows EINA_UNUSED)
{
   return 0;
}

static int
em_bgra_data_get(void *video EINA_UNUSED, unsigned char **bgra_data EINA_UNUSED)
{
   return 0;
}

static void
em_event_feed(void *video, int event)
{
   Emotion_LibVLC *ev = video;
   unsigned int navigate;

   if (!ev->started) return;

   switch (event)
     {
      case EMOTION_EVENT_UP:
        navigate = libvlc_navigate_up;
        break;

      case EMOTION_EVENT_DOWN:
        navigate = libvlc_navigate_down;
        break;

      case EMOTION_EVENT_LEFT:
        navigate = libvlc_navigate_left;
        break;

      case EMOTION_EVENT_RIGHT:
        navigate = libvlc_navigate_right;
        break;

      case EMOTION_EVENT_SELECT:
        navigate = libvlc_navigate_activate;
        break;

      /* FIXME */
      default:
      case EMOTION_EVENT_MENU1:
      case EMOTION_EVENT_MENU2:
      case EMOTION_EVENT_MENU3:
      case EMOTION_EVENT_MENU4:
      case EMOTION_EVENT_MENU5:
      case EMOTION_EVENT_MENU6:
      case EMOTION_EVENT_MENU7:
      case EMOTION_EVENT_NEXT:
      case EMOTION_EVENT_PREV:
      case EMOTION_EVENT_ANGLE_NEXT:
      case EMOTION_EVENT_ANGLE_PREV:
      case EMOTION_EVENT_FORCE:
      case EMOTION_EVENT_0:
      case EMOTION_EVENT_1:
      case EMOTION_EVENT_2:
      case EMOTION_EVENT_3:
      case EMOTION_EVENT_4:
      case EMOTION_EVENT_5:
      case EMOTION_EVENT_6:
      case EMOTION_EVENT_7:
      case EMOTION_EVENT_8:
      case EMOTION_EVENT_9:
      case EMOTION_EVENT_10:
        return;
     }
   libvlc_media_player_navigate(ev->mp, navigate);
}

static void
em_event_mouse_button_feed(void *video EINA_UNUSED, int button EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED)
{
   /* Handled directly by VLC evas vout module */
}

static void
em_event_mouse_move_feed(void *video EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED)
{
   /* Handled directly by VLC evas vout module */
}

static int
em_video_channel_count(void *video)
{
   Emotion_LibVLC *ev = video;

   if (!ev->started) return -1;

   return libvlc_video_get_track_count(ev->mp);
}

static void
em_video_channel_set(void *video,
                     int channel)
{
   Emotion_LibVLC *ev = video;

   if (!ev->started) return;

   if (channel < 0)
     libvlc_video_set_track(ev->mp, -1);
   else
     {
        libvlc_media_track_t *track;
        track = libvlc_get_track_at_pos(ev, channel, libvlc_track_video);
        if (track)
          libvlc_video_set_track(ev->mp, track->i_id);
     }
}

static int
em_video_channel_get(void *video)
{
   Emotion_LibVLC *ev = video;
   int id;

   if (!ev->started) return -1;

   id = libvlc_video_get_track(ev->mp);

   return id >= 0 ? libvlc_get_track_pos(ev, id, libvlc_track_video) : -1;
}

static void
em_video_subtitle_file_set(void *video,
                           const char *filepath)
{
   Emotion_LibVLC *ev = video;

   if (!ev->started) return;

   free(ev->subtitle_file);
   ev->subtitle_file = filepath ? strdup(filepath) : NULL;
   libvlc_video_set_subtitle_file(ev->mp, ev->subtitle_file);
}

static const char *
em_video_subtitle_file_get(void *video)
{
   Emotion_LibVLC *ev = video;

   return ev->subtitle_file;
}

static const char *
em_video_channel_name_get(void *video,
                          int channel)
{
   Emotion_LibVLC *ev = video;
   libvlc_media_track_t *track;

   if (!ev->started) return NULL;

   track = libvlc_get_track_at_pos(ev, channel, libvlc_track_video);
   if (track)
     return track->psz_description;
   else
     return NULL;
}

static void
em_video_channel_mute_set(void *video,
                          int mute)
{
   Emotion_LibVLC *ev = video;

   if (ev->video_mute_force)
     return;
   ev->video_mute = mute;

   if (ev->started)
     em_video_channel_set(video, mute ? -1 : 0);
}

static int
em_video_channel_mute_get(void *video)
{
   Emotion_LibVLC *ev = video;

   return ev->video_mute;
}

static int
em_channel_count(int vlc_count)
{
   /* vlc count the -1 track that deactivate the channel for audio and spu */
   return vlc_count > 0 ? vlc_count - 1 : vlc_count;
}

static int
em_audio_channel_count(void *video)
{
   Emotion_LibVLC *ev = video;

   if (!ev->started) return -1;

   return em_channel_count(libvlc_audio_get_track_count(ev->mp));
}

static void
em_audio_channel_set(void *video,
                     int channel)
{
   Emotion_LibVLC *ev = video;

   if (!ev->started) return;

   if (channel < 0)
     libvlc_audio_set_track(ev->mp, -1);
   else
     {
        libvlc_media_track_t *track;
        track = libvlc_get_track_at_pos(ev, channel, libvlc_track_audio);
        if (track)
          libvlc_audio_set_track(ev->mp, track->i_id);
     }
}

static int
em_audio_channel_get(void *video)
{
   Emotion_LibVLC *ev = video;
   int id;

   if (!ev->started) return -1;

   id = libvlc_audio_get_track(ev->mp);

   return id >= 0 ? libvlc_get_track_pos(ev, id, libvlc_track_audio) : -1;
}

static const char *
em_audio_channel_name_get(void *video,
                          int channel)
{
   Emotion_LibVLC *ev = video;
   libvlc_media_track_t *track;

   if (!ev->started) return NULL;

   track = libvlc_get_track_at_pos(ev, channel, libvlc_track_audio);
   if (track)
     return track->psz_description;
   else
     return NULL;
}

static void
em_audio_channel_mute_set(void *video,
                          int mute)
{
   Emotion_LibVLC *ev = video;

   ev->audio_mute = mute;

   if (ev->started)
     em_audio_channel_set(video, mute ? -1 : 0);
}

static int
em_audio_channel_mute_get(void *video)
{
   Emotion_LibVLC *ev = video;

   return ev->audio_mute;
}

static void
em_audio_channel_volume_set(void *video,
                            double vol)
{
   Emotion_LibVLC *ev = video;

   if (vol < 0.0)
     vol = 0.0;
   else if (vol > 1.0)
     vol = 1.0;
   ev->audio_vol = vol * 100;

   if (!ev->started) return;

   libvlc_audio_set_volume(ev->mp, ev->audio_vol);
}

static double
em_audio_channel_volume_get(void *video)
{
   Emotion_LibVLC *ev = video;

   if (!ev->started)
     return ev->audio_vol / 100.0;

   return libvlc_audio_get_volume(ev->mp) / 100.0;
}

static int
em_spu_channel_count(void *video)
{
   Emotion_LibVLC *ev = video;

   if (!ev->started) return -1;

   return em_channel_count(libvlc_video_get_spu_count(ev->mp));
}

static void
em_spu_channel_set(void *video,
                   int channel)
{
   Emotion_LibVLC *ev = video;

   if (!ev->started) return;

   if (channel < 0)
     libvlc_video_set_spu(ev->mp, -1);
   else
     {
        libvlc_media_track_t *track;
        track = libvlc_get_track_at_pos(ev, channel, libvlc_track_text);
        if (track)
          libvlc_video_set_spu(ev->mp, track->i_id);
     }
}

static int
em_spu_channel_get(void *video)
{
   Emotion_LibVLC *ev = video;
   int id;

   if (!ev->started) return -1;

   id = libvlc_video_get_spu(ev->mp);

   return id >= 0 ? libvlc_get_track_pos(ev, id, libvlc_track_text) : -1;
}

static const char *
em_spu_channel_name_get(void *video,
                        int channel)
{
   Emotion_LibVLC *ev = video;
   libvlc_media_track_t *track;

   if (!ev->started) return NULL;

   track = libvlc_get_track_at_pos(ev, channel, libvlc_track_text);
   if (track)
     return track->psz_description;
   else
     return NULL;
}

static void
em_spu_channel_mute_set(void *video, int mute)
{
   Emotion_LibVLC *ev = video;

   ev->spu_mute = mute;

   if (ev->started)
     em_spu_channel_set(video, mute ? -1 : 0);
}

static int
em_spu_channel_mute_get(void *video)
{
   Emotion_LibVLC *ev = video;

   return ev->spu_mute;
}

static int
em_chapter_count(void *video)
{
   Emotion_LibVLC *ev = video;

   if (!ev->started) return 0;

   return libvlc_media_player_get_chapter_count(ev->mp);
}

static void
em_chapter_set(void *video, int chapter)
{
   Emotion_LibVLC *ev = video;

   if (!ev->started) return;

   libvlc_media_player_set_chapter(ev->mp, chapter);
}

static int
em_chapter_get(void *video)
{
   Emotion_LibVLC *ev = video;

   if (!ev->started) return 0;

   return libvlc_media_player_get_chapter(ev->mp);
}

static const char *
em_chapter_name_get(void *video, int chapter)
{
   Emotion_LibVLC *ev = video;

   if (!ev->started) return NULL;

   if (ev->nb_chapters == 0)
     {
        ev->nb_chapters =
          libvlc_media_player_get_full_chapter_descriptions(ev->mp,
                                                            -1,
                                                            &ev->chapters);
        if (ev->nb_chapters == 0)
          ev->nb_chapters = -1;
     }
   return chapter < ev->nb_chapters ? ev->chapters[chapter]->psz_name : NULL;
}

static void
em_speed_set(void *video, double speed)
{
   Emotion_LibVLC *ev = video;

   if (!ev->started) return;

   libvlc_media_player_set_rate(ev->mp, speed);
}

static double
em_speed_get(void *video)
{
   Emotion_LibVLC *ev = video;

   if (!ev->started) return 1.0;

   return libvlc_media_player_get_rate(ev->mp);
}

static int
em_eject(void *video EINA_UNUSED)
{
   return 1;
}

static const char *
em_meta_get(void *video, int meta)
{
   Emotion_LibVLC *ev = video;

   if (!ev->started) return NULL;

   if (meta <= 0 || meta >= META_TRACK_COUNT)
     return NULL;

   if (ev->metas[meta])
     return ev->metas[meta];
   else
     {
        libvlc_meta_t vlc_meta;
        switch (meta)
          {
           case META_TRACK_TITLE:
             vlc_meta = libvlc_meta_Title;
             break;

           case META_TRACK_ARTIST:
             vlc_meta = libvlc_meta_Artist;
             break;

           case  META_TRACK_ALBUM:
             vlc_meta = libvlc_meta_Album;
             break;

           case META_TRACK_YEAR:
             vlc_meta = libvlc_meta_Date;
             break;

           case META_TRACK_GENRE:
             vlc_meta = libvlc_meta_Genre;
             break;

           case META_TRACK_COMMENT:
             vlc_meta = libvlc_meta_Description;
             break;

           case META_TRACK_DISCID:
             vlc_meta = libvlc_meta_TrackID;
             break;

           default:
             return NULL;
             break;
          }
        ev->metas[meta] = libvlc_media_get_meta(ev->m, vlc_meta);

        return ev->metas[meta];
     }
}

static const Emotion_Engine em_engine =
{
   EMOTION_ENGINE_API_VERSION,
   EMOTION_ENGINE_PRIORITY_DEFAULT,
   "libvlc",
   em_add, /* add */
   em_del, /* del */
   em_file_open, /* file_open */
   em_file_close, /* file_close */
   em_play, /* play */
   em_stop, /* stop */
   em_size_get, /* size_get */
   em_pos_set, /* pos_set */
   em_len_get, /* len_get */
   em_buffer_size_get, /* buffer_size_get */
   em_fps_num_get, /* fps_num_get */
   em_fps_den_get, /* fps_den_get */
   em_fps_get, /* fps_get */
   em_pos_get, /* pos_get */
   em_vis_set, /* vis_set */
   em_vis_get, /* vis_get */
   em_vis_supported, /* vis_supported */
   em_ratio_get, /* ratio_get */
   em_video_handled, /* video_handled */
   em_audio_handled, /* audio_handled */
   em_seekable, /* seekable */
   em_frame_done, /* frame_done */
   em_format_get, /* format_get */
   em_videfl_data_size_get, /* videfl_data_size_get */
   em_yuv_rows_get, /* yuv_rows_get */
   em_bgra_data_get, /* bgra_data_get */
   em_event_feed, /* event_feed */
   em_event_mouse_button_feed, /* event_mouse_button_feed */
   em_event_mouse_move_feed, /* event_mouse_move_feed */
   em_video_channel_count, /* video_channel_count */
   em_video_channel_set, /* video_channel_set */
   em_video_channel_get, /* video_channel_get */
   em_video_subtitle_file_set, /* video_subtitle_file_set */
   em_video_subtitle_file_get, /* video_subtitle_file_get */
   em_video_channel_name_get, /* video_channel_name_get */
   em_video_channel_mute_set, /* video_channel_mute_set */
   em_video_channel_mute_get, /* video_channel_mute_get */
   em_audio_channel_count, /* audio_channel_count */
   em_audio_channel_set, /* audio_channel_set */
   em_audio_channel_get, /* audio_channel_get */
   em_audio_channel_name_get, /* audio_channel_name_get */
   em_audio_channel_mute_set, /* audio_channel_mute_set */
   em_audio_channel_mute_get, /* audio_channel_mute_get */
   em_audio_channel_volume_set, /* audio_channel_volume_set */
   em_audio_channel_volume_get, /* audio_channel_volume_get */
   em_spu_channel_count, /* spu_channel_count */
   em_spu_channel_set, /* spu_channel_set */
   em_spu_channel_get, /* spu_channel_get */
   em_spu_channel_name_get, /* spu_channel_name_get */
   em_spu_channel_mute_set, /* spu_channel_mute_set */
   em_spu_channel_mute_get, /* spu_channel_mute_get */
   em_chapter_count, /* chapter_count */
   em_chapter_set, /* chapter_set */
   em_chapter_get, /* chapter_get */
   em_chapter_name_get, /* chapter_name_get */
   em_speed_set, /* speed_set */
   em_speed_get, /* speed_get */
   em_eject, /* eject */
   em_meta_get, /* meta_get */
   NULL, /* priority_set */
   NULL /* priority_get */
};

static void
libvlc_log(void *data EINA_UNUSED, int level,
           const libvlc_log_t *ctx EINA_UNUSED,
           const char *fmt, va_list args)
{
   Eina_Log_Level eina_log_level;
   const char *name, *header;
   uintptr_t id;

   libvlc_log_get_object(ctx, &name, &header, &id);
   switch (level)
     {
      case LIBVLC_DEBUG:
        eina_log_level = EINA_LOG_LEVEL_DBG;
        break;

      case LIBVLC_NOTICE:
        eina_log_level = EINA_LOG_LEVEL_INFO;
        break;

      case LIBVLC_WARNING:
        eina_log_level = EINA_LOG_LEVEL_WARN;
        break;

      case LIBVLC_ERROR:
      default:
        eina_log_level = EINA_LOG_LEVEL_ERR;
        break;
     }
   eina_log_vprint(_emotion_libvlc_log_domain, eina_log_level,
                   "", name, id, fmt, args);
}

static libvlc_instance_t *
libvlc_new_env_args(void)
{
   unsigned int argc = 0, i = 0;
   const char **argv = NULL;
   char *args_env, *args_dup = NULL, *str = NULL, *token, *saveptr;
   libvlc_instance_t *instance = NULL;

   args_env = getenv("EMOTION_LIBVLC_ARGS");
   if (!args_env)
     goto fallback;

   /* dup since strtok modify the str */
   args_dup = strdup(args_env);
   if (!args_dup)
     goto fallback;

   /* call strtok to count the numbers of arguments */
   str = strdup(args_dup);
   if (!str)
     goto fallback;

   token = strtok_r(str, " ", &saveptr);
   while (token)
     {
        argc++;
        token = strtok_r(NULL, " ", &saveptr);
     }
   if (!argc)
     goto fallback;

   /* alloc argv */
   argv = calloc(1, argc * sizeof(char *));
   if (!argv)
     goto fallback;

   /* call strtok to fill argv */
   free(str);
   str = strdup(args_dup);
   if (!str)
     goto fallback;

   token = strtok_r(str, " ", &saveptr);
   while (token && i < argc)
     {
        argv[i++] = token;
        token = strtok_r(NULL, " ", &saveptr);
     }
   argc = i;

   for (i = 0; i < argc; ++i)
     INF("libvlc_argv[%d]: %s", i, argv[i]);

   instance = libvlc_new(argc, argv);

fallback:
   free(args_dup);
   free(str);
   free(argv);
   return instance ? instance : libvlc_new(0, NULL);
}

Eina_Bool
libvlc_module_init(void)
{
   if (libvlc)
     {
        return EINA_TRUE;
     }

   if (getenv("EMOTION_FPS_DEBUG")) debug_fps = EINA_TRUE;

   eina_threads_init();
   eina_log_threads_enable();
   _emotion_libvlc_log_domain = eina_log_domain_register
       ("emotion-libvlc", EINA_COLOR_ORANGE);
   if (_emotion_libvlc_log_domain < 0)
     {
        EINA_LOG_CRIT("Could not register log domain 'emotion-libvlc'");
        return EINA_FALSE;
     }

   libvlc = libvlc_new_env_args();
   if (!libvlc)
     {
        CRI("could not create libvlc instance");
        goto error_register;
     }
   EINA_LOG_INFO("using libvlc: %s", libvlc_get_version());
   libvlc_log_set(libvlc, libvlc_log, NULL);

   if (!_emotion_module_register(&em_engine))
     {
        ERR("Could not register module %p", &em_engine);
        goto error_register;
     }

   return EINA_TRUE;

error_register:
   if (libvlc)
     {
        libvlc_release(libvlc);
        libvlc = NULL;
     }
   eina_log_domain_unregister(_emotion_libvlc_log_domain);
   _emotion_libvlc_log_domain = -1;
   return EINA_FALSE;
}

void
libvlc_module_shutdown(void)
{
   if (!libvlc)
     {
        EINA_LOG_ERR("too many libvlc_module_shutdown()");
        return;
     }

   _emotion_module_unregister(&em_engine);

   libvlc_release(libvlc);
   libvlc = NULL;

   eina_log_domain_unregister(_emotion_libvlc_log_domain);
   _emotion_libvlc_log_domain = -1;
}

#ifndef EMOTION_STATIC_BUILD_LIBVLC

EINA_MODULE_INIT(libvlc_module_init);
EINA_MODULE_SHUTDOWN(libvlc_module_shutdown);

#endif
