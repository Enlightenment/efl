#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <poll.h>

#include <signal.h>

#include <vlc/vlc.h>

#include <Emotion_Generic_Plugin.h>
#include <Eina.h>
#include <Ecore.h>

static int _em_vlc_log_dom = -1;
#define ERR(...) EINA_LOG_DOM_ERR(_em_vlc_log_dom, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(_em_vlc_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_em_vlc_log_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_em_vlc_log_dom, __VA_ARGS__)
#define CRIT(...) EINA_LOG_DOM_CRIT(_em_vlc_log_dom, __VA_ARGS__)

enum _Thread_Events {
   EM_THREAD_POSITION_CHANGED,
   EM_THREAD_PLAYBACK_STARTED,
   EM_THREAD_PLAYBACK_STOPPED,
   EM_THREAD_LAST
};

typedef struct _App App;
struct _App {
   Emotion_Generic_Video_Shared *vs;
   Emotion_Generic_Video_Frame vf;
   libvlc_instance_t *libvlc;
   libvlc_media_t *m;
   libvlc_media_player_t *mp;
   libvlc_event_manager_t *event_mgr;
   libvlc_event_manager_t *mevent_mgr;
   char *filename;
   char *subtitle_path;
   char *shmname;
   void *tmpbuffer;
   int w, h;
   // Use Ecore infra for that instead
   Ecore_Pipe *fd_read; // read commands from theads here
   Ecore_Pipe *fd_write; // write commands from threads here
   /* int em_read; // read commands from emotion here */
   /* int em_write; // write commands to emotion here */
   int size_sent;
   int opening;
   int closing;
   int playing;

   int last_order;

   Eina_Bool inited;
};


/* Commands sent to the emotion pipe */
#define SEND_CMD_PARAM(app, i)                                  \
  if ((app)->fd_write)                                          \
    if (!ecore_pipe_write((app)->fd_write, &(i), sizeof((i))))  \
      ecore_main_loop_quit();

static void
_send_cmd(App *app, int cmd)
{
   if (app->fd_write)
     if (!ecore_pipe_write(app->fd_write, &cmd, sizeof(cmd)))
       ecore_main_loop_quit();
}

static void
_send_cmd_str(App *app, const char *str)
{
   int len;

   len = str ? strlen(str) + 1 : 0;
   if (app->fd_write)
     if (!ecore_pipe_write(app->fd_write, &len, sizeof(len)))
       ecore_main_loop_quit();
   if (app->fd_write)
     if (!ecore_pipe_write(app->fd_write, str, len))
       ecore_main_loop_quit();
}

static void
_send_file_closed(App *app)
{
   app->closing = 0;
   emotion_generic_shm_free(app->vs);
   _send_cmd(app, EM_RESULT_FILE_CLOSE);
}

static void
_send_time_changed(struct _App *app, const struct libvlc_event_t *ev)
{
   float new_time = ev->u.media_player_time_changed.new_time;

   new_time /= 1000;
   if (app->vs->frame_drop > 1)
     return;
   _send_cmd(app, EM_RESULT_POSITION_CHANGED);
   SEND_CMD_PARAM(app, new_time);
}

static void
_send_resize(struct _App *app, int width, int height)
{
   _send_cmd(app, EM_RESULT_FRAME_SIZE);
   SEND_CMD_PARAM(app, width);
   SEND_CMD_PARAM(app, height);
}

static void
_send_track_info(struct _App *app, int cmd, int current, int count, libvlc_track_description_t *desc)
{
   _send_cmd(app, cmd);
   SEND_CMD_PARAM(app, current);
   SEND_CMD_PARAM(app, count);
   while (desc)
     {
        int tid = desc->i_id;
        const char *name = desc->psz_name;
        SEND_CMD_PARAM(app, tid);
        _send_cmd_str(app, name);
        desc = desc->p_next;
     }
}

static void
_send_all_track_info(struct _App *app)
{
   int track_count, current;
   libvlc_track_description_t *desc;

   current = libvlc_audio_get_track(app->mp);
   track_count = libvlc_audio_get_track_count(app->mp);
   desc = libvlc_audio_get_track_description(app->mp);

   _send_track_info(app, EM_RESULT_AUDIO_TRACK_INFO,
                    current, track_count, desc);

   current = libvlc_video_get_track(app->mp);
   track_count = libvlc_video_get_track_count(app->mp);
   desc = libvlc_video_get_track_description(app->mp);

   _send_track_info(app, EM_RESULT_VIDEO_TRACK_INFO,
                    current, track_count, desc);

   current = libvlc_video_get_spu(app->mp);
   track_count = libvlc_video_get_spu_count(app->mp);
   desc = libvlc_video_get_spu_description(app->mp);

   _send_track_info(app, EM_RESULT_SPU_TRACK_INFO,
                    current, track_count, desc);
}

static void
_send_all_meta_info(struct _App *app)
{
   const char *meta;

   _send_cmd(app, EM_RESULT_META_INFO);

   /*
    * Will send in this order: title, artist, album, year,
    * genre, comments, disc id and track count.
    */
   meta = libvlc_media_get_meta(app->m, libvlc_meta_Title);
   _send_cmd_str(app, meta);
   meta = libvlc_media_get_meta(app->m, libvlc_meta_Artist);
   _send_cmd_str(app, meta);
   meta = libvlc_media_get_meta(app->m, libvlc_meta_Album);
   _send_cmd_str(app, meta);
   meta = libvlc_media_get_meta(app->m, libvlc_meta_Date);
   _send_cmd_str(app, meta);
   meta = libvlc_media_get_meta(app->m, libvlc_meta_Genre);
   _send_cmd_str(app, meta);
   meta = NULL; // sending empty comments
   _send_cmd_str(app, meta);
   meta = NULL; // sending empty disc id
   _send_cmd_str(app, meta);
   meta = libvlc_media_get_meta(app->m, libvlc_meta_TrackNumber);
   _send_cmd_str(app, meta);
}

static void
_send_length_changed(struct _App *app, const struct libvlc_event_t *ev)
{
   float length = ev->u.media_player_length_changed.new_length;
   length /= 1000;

   _send_cmd(app, EM_RESULT_LENGTH_CHANGED);
   SEND_CMD_PARAM(app, length);
}

static void
_send_seekable_changed(struct _App *app, const struct libvlc_event_t *ev)
{
   int seekable = ev->u.media_player_seekable_changed.new_seekable;

   _send_cmd(app, EM_RESULT_SEEKABLE_CHANGED);
   SEND_CMD_PARAM(app, seekable);
}

static void
_send_file_set(struct _App *app)
{
   if (app->opening)
      _send_cmd(app, EM_RESULT_FILE_SET);

   if (app->closing)
     _send_file_closed(app);
}


/* libvlc */
static Eina_Bool
_loaded_idler(void *data)
{
   App *app = data;

   if (app->mp)
     libvlc_media_player_stop(app->mp);

   return EINA_FALSE;
}

static void
_position_changed(App *app)
{
   int r;
   unsigned int w, h;

   if (!app->opening)
     return;

   /* sending size info only once */
   r = libvlc_video_get_size(app->mp, 0, &w, &h);
   if (r < 0)
     {
        w = 1;
        h = 1;
     }

   if (w > 0 || h > 0)
     {
        _send_resize(app, w, h);
        app->size_sent = 1;
     }

   /* sending audio track info */
   _send_all_track_info(app);

   /* sending meta info */
   _send_all_meta_info(app);

   ecore_idler_add(_loaded_idler, app);
}

static void
_event_cb(const struct libvlc_event_t *ev, void *data)
{
   App *app = data;

   ecore_thread_main_loop_begin();
   switch (ev->type)
   {
      case libvlc_MediaPlayerTimeChanged:
         _send_time_changed(app, ev);
         break;
      case libvlc_MediaPlayerPositionChanged:
         _position_changed(app);
         break;
      case libvlc_MediaPlayerLengthChanged:
         _send_length_changed(app, ev);
         break;
      case libvlc_MediaPlayerSeekableChanged:
         _send_seekable_changed(app, ev);
         break;
      case libvlc_MediaPlayerPlaying:
         _send_resize(app, app->w, app->h);
         _send_cmd(app, EM_RESULT_PLAYBACK_STARTED);
         break;
      case libvlc_MediaPlayerStopped:
         _send_file_set(app);
         break;
      case libvlc_MediaPlayerEndReached:
         app->playing = 0;
         _send_cmd(app, EM_RESULT_PLAYBACK_STOPPED);
         break;
   }
   ecore_thread_main_loop_end();
}

static void *
_tmp_lock(void *data, void **pixels)
{
   App *app = data;
   *pixels = app->tmpbuffer;
   return NULL;
}

static void
_tmp_unlock(void *data EINA_UNUSED, void *id EINA_UNUSED, void *const *pixels EINA_UNUSED)
{
}

static void
_tmp_display(void *data EINA_UNUSED, void *id EINA_UNUSED)
{
}

static unsigned
_tmp_format(void **opaque, char *chroma,
            unsigned *width, unsigned *height,
            unsigned *pitches, unsigned *lines)
{
   App *app = *opaque;

   app->tmpbuffer = realloc(app->tmpbuffer,
                            *width * *height * 4 * sizeof (char));
   strcpy(chroma, "RV32");
   pitches[0] = pitches[1] = pitches[2] = *width * 4;
   lines[0] = lines[1] = lines[2] = *height;

   _send_resize(app, *width, *height);
   
   return 1;
}

/*
static void
_tmp_play(void *data EINA_UNUSED,
          const void *samples EINA_UNUSED, unsigned count EINA_UNUSED,
          int64_t pts EINA_UNUSED)
{
}
*/

static void *
_lock(void *data, void **pixels)
{
   App *app = data;

   if (app->playing)
     *pixels = app->vf.frames[app->vs->frame.player];
   else
     *pixels = NULL;

   return NULL; // picture identifier, not needed here
}

static void
_unlock(void *data EINA_UNUSED, void *id EINA_UNUSED, void *const *pixels EINA_UNUSED)
{
}

static void
_display(void *data, void *id EINA_UNUSED)
{
   App *app = data;

   if (!app->playing)
     return;

   eina_semaphore_lock(&app->vs->lock);
   app->vs->frame.last = app->vs->frame.player;
   app->vs->frame.player = app->vs->frame.next;
   app->vs->frame.next = app->vs->frame.last;
   if (!app->vs->frame_drop++)
     _send_cmd(app, EM_RESULT_FRAME_NEW);
   eina_semaphore_release(&app->vs->lock, 1);
}


/* Commands received from the emotion pipe */
static void
_file_set(App *app)
{
   DBG("Path: %s", app->filename);
   app->m = libvlc_media_new_path(app->libvlc, app->filename);
   if (!app->m)
     {
        ERR("could not open path: \"%s\"", app->filename);
        return;
     }

   app->mp = libvlc_media_player_new_from_media(app->m);
   if (!app->mp)
     {
        ERR("could not create new player from media.");
        return;
     }

   app->opening = 1;
   libvlc_video_set_format(app->mp, "RV32", DEFAULTWIDTH, DEFAULTHEIGHT, DEFAULTWIDTH * 4);
   libvlc_video_set_callbacks(app->mp, _tmp_lock, _tmp_unlock, _tmp_display, app);
   libvlc_video_set_format_callbacks(app->mp, _tmp_format, NULL);
   /* On my system the mute below is not working and I can't find a way
      to make it work, so the following set should help, but then it has
      other side effect...
   */
   /* libvlc_audio_set_callbacks(app->mp, _tmp_play, NULL, NULL, NULL, NULL, app); */

   app->event_mgr = libvlc_media_player_event_manager(app->mp);
   libvlc_event_attach(app->event_mgr, libvlc_MediaPlayerPositionChanged,
                       _event_cb, app);
   libvlc_event_attach(app->event_mgr, libvlc_MediaPlayerStopped,
                       _event_cb, app);

   app->mevent_mgr = libvlc_media_event_manager(app->m);

   app->tmpbuffer = malloc(sizeof(char) * DEFAULTWIDTH * DEFAULTHEIGHT * 4);
   libvlc_audio_set_mute(app->mp, 1);
   libvlc_media_player_play(app->mp);
}

static void
_file_set_done(App *app)
{
   int r;

   app->opening = 0;

   r = emotion_generic_shm_get(app->shmname, &app->vs, &app->vf);
   if (!r)
     {
        free(app->filename);
        libvlc_media_release(app->m);
        libvlc_media_player_release(app->mp);
        app->filename = NULL;
        app->m = NULL;
        app->mp = NULL;

        _send_cmd(app, EM_RESULT_FILE_SET_DONE);
        SEND_CMD_PARAM(app, r);
     }
   app->w = app->vs->width;
   app->h = app->vs->height;
   libvlc_video_set_format(app->mp, "RV32", app->w, app->h, app->w * 4);
   libvlc_video_set_callbacks(app->mp, _lock, _unlock, _display, app);

   libvlc_event_attach(app->event_mgr, libvlc_MediaPlayerPlaying,
                      _event_cb, app);
   libvlc_event_attach(app->event_mgr, libvlc_MediaPlayerTimeChanged,
                      _event_cb, app);
   libvlc_event_attach(app->event_mgr, libvlc_MediaPlayerLengthChanged,
                      _event_cb, app);
   libvlc_event_attach(app->event_mgr, libvlc_MediaPlayerSeekableChanged,
                      _event_cb, app);
   libvlc_event_attach(app->event_mgr, libvlc_MediaPlayerEndReached,
                       _event_cb, app);

   libvlc_audio_set_mute(app->mp, 0);

   _send_cmd(app, EM_RESULT_FILE_SET_DONE);
   SEND_CMD_PARAM(app, r);
}

static void
_file_close(App *app)
{
   app->playing = 0;
   if (app->opening)
     goto release_resources;

   if (libvlc_media_player_get_state(app->mp) != libvlc_Playing)
     {
        _send_file_closed(app);
        return;
     }

   app->closing = 1;

release_resources:
   libvlc_media_player_stop(app->mp);
   if (app->filename)
     free(app->filename);
   if (app->mp)
     {
        libvlc_media_release(app->m);
        libvlc_media_player_release(app->mp);
        free(app->tmpbuffer);
     }
}

static void
_stop(App *app)
{
   if (app->mp)
     libvlc_media_player_set_pause(app->mp, 1);
}

static void
_play(App *app, float pos)
{
   if (!app->mp)
     return;

   if (app->playing)
     {
        libvlc_media_player_set_pause(app->mp, 0);
     }
   else
     {
        libvlc_time_t new_time = pos * 1000;
        libvlc_media_player_set_time(app->mp, new_time);
        libvlc_media_player_play(app->mp);

        if (app->subtitle_path)
          libvlc_video_set_subtitle_file(app->mp, app->subtitle_path);

        app->playing = 1;
     }
}

static void
_position_set(struct _App *app, float position)
{
   libvlc_time_t new_time;

   if (!app->mp)
     return;

   new_time = position * 1000;
   libvlc_media_player_set_time(app->mp, new_time);
}

static void
_speed_set(App *app, float rate)
{
   if (!app->mp)
     return;

   libvlc_media_player_set_rate(app->mp, rate);
}

static void
_mute_set(App *app, int mute)
{
   if (!app->mp)
     return;

   libvlc_audio_set_mute(app->mp, mute);
}

static void
_volume_set(App *app, float volume)
{
   int vol;

   if (!app->mp)
     return;

   vol = volume * 100;

   libvlc_audio_set_volume(app->mp, vol);
}

static void
_spu_track_set(App *app, int track)
{
   libvlc_video_set_spu(app->mp, track);
}

static void
_audio_track_set(App *app, int track)
{
   libvlc_audio_set_track(app->mp, track);
}

static void
_video_track_set(App *app, int track)
{
   libvlc_video_set_track(app->mp, track);
}

static void
_remote_command(void *data, void *buffer, unsigned int nbyte)
{
   App *app = data;

   if (nbyte == 0)
     {
        ecore_main_loop_quit();
        return ;
     }

   if (app->last_order == EM_CMD_LAST)
     {
        if (nbyte != sizeof (int))
          {
             ERR("didn't receive a valid command from emotion (%i) !", nbyte);
             ecore_main_loop_quit();
             return ;
          }

        app->last_order = *((int*) buffer);

        if (!app->inited &&
            app->last_order != EM_CMD_INIT)
          {
             ERR("wrong init command!");
             ecore_main_loop_quit();
             return ;
          }

        switch (app->last_order)
          {
           case EM_CMD_FILE_SET:
              if (app->opening)
                {
                   libvlc_media_release(app->m);
                   libvlc_media_player_release(app->mp);
                   free(app->filename);
                   app->opening = 0;
                }
              break;
           case EM_CMD_FILE_SET_DONE:
              _file_set_done(app);
              app->last_order = EM_CMD_LAST;
              break;
           case EM_CMD_FILE_CLOSE:
              _file_close(app);
              app->last_order = EM_CMD_LAST;
              break;
           case EM_CMD_STOP:
              _stop(app);
              app->last_order = EM_CMD_LAST;
              break;
          }
     }
   else
     {
        switch (app->last_order)
          {
           case EM_CMD_INIT:
              app->shmname = strdup(buffer);
              app->inited = EINA_TRUE;
              _send_cmd(app, EM_RESULT_INIT);
              break;
           case EM_CMD_FILE_SET:
              app->filename = strdup(buffer);
              _file_set(app);
              break;
           case EM_CMD_SUBTITLE_SET:
              app->subtitle_path = strdup(buffer);
              break;
           case EM_CMD_PLAY:
              _play(app, *(float*) buffer);
              break;
           case EM_CMD_POSITION_SET:
              _position_set(app, *(float*) buffer);
              break;
           case EM_CMD_SPEED_SET:
              _speed_set(app, *(float*) buffer);
              break;
           case EM_CMD_AUDIO_MUTE_SET:
              _mute_set(app, *(int*) buffer);
              break;
           case EM_CMD_VOLUME_SET:
              _volume_set(app, *(float*) buffer);
              break;
           case EM_CMD_SPU_TRACK_SET:
              _spu_track_set(app, *(int*) buffer);
              break;
           case EM_CMD_AUDIO_TRACK_SET:
              _audio_track_set(app, *(int*) buffer);
              break;
           case EM_CMD_VIDEO_TRACK_SET:
              _video_track_set(app, *(int*) buffer);
              break;
          }
        app->last_order = EM_CMD_LAST;
     }
}

static void
_dummy(void *data EINA_UNUSED, void *buffer EINA_UNUSED, unsigned int nbyte EINA_UNUSED)
{
   /* This function is useless for the pipe we use to send message back
      to emotion, but still needed */
}

/* Main */
Eina_Bool
exit_func(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *ev EINA_UNUSED)
{
   ecore_main_loop_quit();
   return EINA_TRUE;
}

int
main(int argc, const char *argv[])
{
   App app;
   Ecore_Event_Handler *hld;
   int vlc_argc;

   const char *vlc_argv[] =
     {
        "--quiet",
        "--intf", "dummy",              /* no interface                      */
        "--vout", "dummy",              /* we don't want video (output)      */
        "--no-video-title-show",        /* nor the filename displayed        */
        "--no-sub-autodetect-file",     /* we don't want automatic subtitles */
        "--no-stats",                   /* no stats                          */
        "--no-inhibit",                 /* we don't want interfaces          */
        "--no-disable-screensaver",     /* we don't want interfaces          */
     };
   vlc_argc = sizeof(vlc_argv) / sizeof(*vlc_argv);

   if (!eina_init())
     {
        EINA_LOG_CRIT("Can't initialize generic vlc player, eina failed.");
        return -1;
     }

   _em_vlc_log_dom = eina_log_domain_register("emotion_generic_vlc",
                                              EINA_COLOR_CYAN);
   if (_em_vlc_log_dom < 0)
     {
        EINA_LOG_CRIT("Unable to register emotion_generic_vlc log domain.");
        goto error;
     }

   if (!eina_log_domain_level_check(_em_vlc_log_dom, EINA_LOG_LEVEL_WARN))
     eina_log_domain_level_set("emotion_generic_vlc", EINA_LOG_LEVEL_WARN);

   if (argc < 3)
     {
        ERR("missing parameters.");
        ERR("syntax:\n\t%s <fd read> <fd write>", argv[0]);
        goto error;
     }

   ecore_init();

   app.fd_read = ecore_pipe_full_add(_remote_command, &app,
                                     atoi(argv[1]), -1, EINA_FALSE, EINA_FALSE);
   app.fd_write = ecore_pipe_full_add(_dummy, NULL,
                                      -1, atoi(argv[2]), EINA_FALSE, EINA_FALSE);

   hld = ecore_event_handler_add(ECORE_EVENT_SIGNAL_HUP, exit_func, NULL);

   app.libvlc = libvlc_new(vlc_argc, vlc_argv);
   app.mp = NULL;
   app.filename = NULL;
   app.subtitle_path = NULL;
   app.w = 0;
   app.h = 0;
   app.size_sent = 0;
   app.opening = 0;
   app.playing = 0;
   app.closing = 0;
   app.last_order = EM_CMD_LAST;
   app.inited = EINA_FALSE;

   ecore_main_loop_begin();

   libvlc_release(app.libvlc);
   ecore_event_handler_del(hld);

   ecore_shutdown();
   eina_shutdown();
   return 0;

 error:
   eina_shutdown();
   return -1;
}
#undef SEND_CMD_PARAM
