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

#include <sys/prctl.h>
#include <signal.h>

#include <vlc/vlc.h>
#include <Emotion_Generic_Plugin.h>

enum _Thread_Events {
     EM_THREAD_POSITION_CHANGED,
     EM_THREAD_LAST
};

struct _App {
     Emotion_Generic_Video_Shared *vs;
     Emotion_Generic_Video_Frame vf;
     libvlc_instance_t *libvlc;
     libvlc_media_t *m;
     libvlc_media_player_t *mp;
     libvlc_event_manager_t *event_mgr;
     libvlc_event_manager_t *mevent_mgr;
     char *filename;
     char *shmname;
     void *tmpbuffer;
     int w, h;
     int fd_read; // read commands from theads here
     int fd_write; // write commands from threads here
     int em_read; // read commands from emotion here
     int em_write; // write commands to emotion here
     int size_sent;
     int opening;
     int closing;
     int playing;
};

static pthread_mutex_t _mutex_fd = PTHREAD_MUTEX_INITIALIZER;

int
_em_read_safe(int fd, void *buf, ssize_t size)
{
   ssize_t todo;
   char *p;

   todo = size;
   p = buf;

   while (todo > 0)
     {
        ssize_t r;

        r = read(fd, p, todo);
        if (r > 0)
          {
             todo -= r;
             p += r;
          }
        else if (r == 0)
          return 0;
        else
          {
             if (errno == EINTR || errno == EAGAIN)
               continue;
             else
               {
		  fprintf(stderr, "could not read from fd %d: %s",
			  fd, strerror(errno));
                  return 0;
               }
          }
     }

   return 1;
}

int
_em_write_safe(int fd, const void *buf, ssize_t size)
{
   ssize_t todo;
   const char *p;

   todo = size;
   p = buf;

   while (todo > 0)
     {
        ssize_t r;

        r = write(fd, p, todo);
        if (r > 0)
          {
             todo -= r;
             p += r;
          }
        else if (r == 0)
          return 0;
        else
          {
             if (errno == EINTR || errno == EAGAIN)
               continue;
             else
               {
		  fprintf(stderr, "could not write to fd %d: %s",
			  fd, strerror(errno));
		  return 0;
               }
          }
     }

   return 1;
}

static int
_em_str_read(int fd, char **str)
{
   int size;
   int r;
   char buf[PATH_MAX];

   r = _em_read_safe(fd, &size, sizeof(size));
   if (!r)
     {
        *str = NULL;
        return 0;
     }

   if (!size)
     {
        *str = NULL;
        return 1;
     }

   r = _em_read_safe(fd, buf, size);
   if (!r)
     {
        *str = NULL;
        return 0;
     }

   *str = strdup(buf);
   return 1;
}

static int
_em_cmd_read(struct _App *app)
{
   int cmd;
   _em_read_safe(app->em_read, &cmd, sizeof(cmd));

   return cmd;
}

static void
_send_cmd_start(struct _App *app, int cmd)
{
   pthread_mutex_lock(&_mutex_fd);
   _em_write_safe(app->em_write, &cmd, sizeof(cmd));
}

static void
_send_cmd_finish(struct _App *app)
{
   pthread_mutex_unlock(&_mutex_fd);
}

static void
_send_cmd(struct _App *app, int cmd)
{
   _send_cmd_start(app, cmd);
   _send_cmd_finish(app);
}

static void
_send_cmd_str(struct _App *app, const char *str)
{
   int len;
   if (str)
     len = strlen(str) + 1;
   else
     len = 0;
   _em_write_safe(app->em_write, &len, sizeof(len));
   _em_write_safe(app->em_write, str, len);
}

#define SEND_CMD_PARAM(app, i) \
   _em_write_safe((app)->em_write, &(i), sizeof((i)));

static void
_send_resize(struct _App *app, int width, int height)
{
   _send_cmd_start(app, EM_RESULT_FRAME_SIZE);
   SEND_CMD_PARAM(app, width);
   SEND_CMD_PARAM(app, height);
   _send_cmd_finish(app);
}

static void
_send_length_changed(struct _App *app, const struct libvlc_event_t *ev)
{
   float length = ev->u.media_player_length_changed.new_length;
   length /= 1000;

   fprintf(stderr, "length changed: %0.3f\n", length);
   _send_cmd_start(app, EM_RESULT_LENGTH_CHANGED);
   SEND_CMD_PARAM(app, length);
   _send_cmd_finish(app);
}

static void
_send_time_changed(struct _App *app, const struct libvlc_event_t *ev)
{
   float new_time = ev->u.media_player_time_changed.new_time;
   new_time /= 1000;
   if (app->vs->frame_drop > 1)
     return;
   _send_cmd_start(app, EM_RESULT_POSITION_CHANGED);
   SEND_CMD_PARAM(app, new_time);
   _send_cmd_finish(app);
}

static void
_send_seekable_changed(struct _App *app, const struct libvlc_event_t *ev)
{
   int seekable = ev->u.media_player_seekable_changed.new_seekable;
   _send_cmd_start(app, EM_RESULT_SEEKABLE_CHANGED);
   SEND_CMD_PARAM(app, seekable);
   _send_cmd_finish(app);
}

static void *
_lock(void *data, void **pixels)
{
   struct _App *app = data;

   if (app->playing)
     *pixels = app->vf.frames[app->vs->frame.player];
   else
     *pixels = NULL;

   return NULL; // picture identifier, not needed here
}

static void
_unlock(void *data, void *id, void *const *pixels)
{
}

static void
_display(void *data, void *id)
{
   struct _App *app = data;
   if (!app->playing)
     return;

   sem_wait(&app->vs->lock);
   app->vs->frame.last = app->vs->frame.player;
   app->vs->frame.player = app->vs->frame.next;
   app->vs->frame.next = app->vs->frame.last;
   if (!app->vs->frame_drop++)
     _send_cmd(app, EM_RESULT_FRAME_NEW);
   sem_post(&app->vs->lock);
}

static void *
_tmp_lock(void *data, void **pixels)
{
   struct _App *app = data;
   *pixels = app->tmpbuffer;
   return NULL;
}

static void
_tmp_unlock(void *data, void *id, void *const *pixels)
{
}

static void
_tmp_display(void *data, void *id)
{
}

static void
_play(struct _App *app)
{
   float pos;

   if (!app->mp)
     return;

   _em_read_safe(app->em_read, &pos, sizeof(pos));

   if (app->playing)
     {
	libvlc_media_player_set_pause(app->mp, 0);
     }
   else
     {
	libvlc_time_t new_time = pos * 1000;
	libvlc_media_player_play(app->mp);
	libvlc_media_player_set_time(app->mp, new_time);
	app->playing = 1;
     }
}

static void
_stop(struct _App *app)
{
   if (app->mp)
     libvlc_media_player_set_pause(app->mp, 1);
}

static void
_send_file_closed(struct _App *app)
{
   app->closing = 0;
   emotion_generic_shm_free(app->vs);
   _send_cmd(app, EM_RESULT_FILE_CLOSE);
}

static void
_send_file_set(struct _App *app)
{
   if (app->opening)
      _send_cmd(app, EM_RESULT_FILE_SET);

   if (app->closing)
     _send_file_closed(app);
}

static void
_event_cb(const struct libvlc_event_t *ev, void *data)
{
   struct _App *app = data;
   int thread_event;

   switch (ev->type) {
      case libvlc_MediaPlayerTimeChanged:
	 _send_time_changed(app, ev);
	 break;
      case libvlc_MediaPlayerPositionChanged:
	 thread_event = EM_THREAD_POSITION_CHANGED;
	 write(app->fd_write, &thread_event, sizeof(thread_event));
	 break;
      case libvlc_MediaPlayerLengthChanged:
	 _send_length_changed(app, ev);
	 break;
      case libvlc_MediaPlayerSeekableChanged:
	 _send_seekable_changed(app, ev);
	 break;
      case libvlc_MediaPlayerPlaying:
	 _send_resize(app, app->w, app->h);
	 break;
      case libvlc_MediaPlayerStopped:
	 _send_file_set(app);
	 break;
      case libvlc_MediaPlayerEndReached:
	 _send_cmd(app, EM_RESULT_PLAYBACK_STOPPED);
	 break;
   }
}

static void
_file_set(struct _App *app)
{
   if (app->opening)
     {
	libvlc_media_release(app->m);
	libvlc_media_player_release(app->mp);
	free(app->filename);
     }

   _em_str_read(app->em_read, &app->filename);

   app->m = libvlc_media_new_path(app->libvlc, app->filename);
   if (!app->m)
     {
	fprintf(stderr, "could not open path: \"%s\"\n", app->filename);
	return;
     }
   app->mp = libvlc_media_player_new_from_media(app->m);

   if (!app->mp)
     {
	fprintf(stderr, "could not create new player from media.\n");
	return;
     }

   app->opening = 1;
   libvlc_video_set_format(app->mp, "RV32", DEFAULTWIDTH, DEFAULTHEIGHT, DEFAULTWIDTH * 4);
   libvlc_video_set_callbacks(app->mp, _tmp_lock, _tmp_unlock, _tmp_display, app);
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
_position_set(struct _App *app)
{
   if (!app->mp)
     return;

   float position;
   _em_read_safe(app->em_read, &position, sizeof(position));

   libvlc_time_t new_time = position * 1000;
   libvlc_media_player_set_time(app->mp, new_time);
}

static void
_speed_set(struct _App *app)
{
   float rate;

   if (!app->mp)
     return;

   _em_read_safe(app->em_read, &rate, sizeof(rate));

   libvlc_media_player_set_rate(app->mp, rate);
}

static void
_mute_set(struct _App *app)
{
   int mute;

   if (!app->mp)
     return;

   _em_read_safe(app->em_read, &mute, sizeof(mute));

   libvlc_audio_set_mute(app->mp, mute);
}

static void
_volume_set(struct _App *app)
{
   float volume;
   int vol;

   if (!app->mp)
     return;

   _em_read_safe(app->em_read, &volume, sizeof(volume));
   vol = volume * 100;

   libvlc_audio_set_volume(app->mp, vol);
}

static void
_audio_track_set(struct _App *app)
{
   int track;

   _em_read_safe(app->em_read, &track, sizeof(track));

   libvlc_audio_set_track(app->mp, track);
}

static void
_video_track_set(struct _App *app)
{
   int track;

   _em_read_safe(app->em_read, &track, sizeof(track));

   libvlc_video_set_track(app->mp, track);
}

static void
_spu_track_set(struct _App *app)
{
   int track;

   _em_read_safe(app->em_read, &track, sizeof(track));

   libvlc_video_set_spu(app->mp, track);
}

static void
_file_set_done(struct _App *app)
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
	_send_cmd_start(app, EM_RESULT_FILE_SET_DONE);
	SEND_CMD_PARAM(app, r);
	_send_cmd_finish(app);
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

   libvlc_audio_set_mute(app->mp, 0);

   _send_cmd_start(app, EM_RESULT_FILE_SET_DONE);
   SEND_CMD_PARAM(app, r);
   _send_cmd_finish(app);
}

static void
_file_close(struct _App *app)
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
_process_emotion_commands(struct _App *app)
{
   int cmd = _em_cmd_read(app);
   switch (cmd) {
      case EM_CMD_FILE_SET:
	 _file_set(app);
	 break;
      case EM_CMD_FILE_SET_DONE:
	 _file_set_done(app);
	 break;
      case EM_CMD_FILE_CLOSE:
	 _file_close(app);
	 break;
      case EM_CMD_PLAY:
	 _play(app);
	 break;
      case EM_CMD_STOP:
	 _stop(app);
	 break;
      case EM_CMD_POSITION_SET:
	 _position_set(app);
	 break;
      case EM_CMD_SPEED_SET:
	 _speed_set(app);
	 break;
      case EM_CMD_AUDIO_MUTE_SET:
	 _mute_set(app);
	 break;
      case EM_CMD_VOLUME_SET:
	 _volume_set(app);
	 break;
      case EM_CMD_AUDIO_TRACK_SET:
	 _audio_track_set(app);
	 break;
      case EM_CMD_VIDEO_TRACK_SET:
	 _video_track_set(app);
	 break;
   };
}

static void
_send_track_info(struct _App *app, int cmd, int current, int count, libvlc_track_description_t *desc)
{
   _send_cmd_start(app, cmd);
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
   _send_cmd_finish(app);
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

   _send_cmd_start(app, EM_RESULT_META_INFO);

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
   _send_cmd_finish(app);
}

static void
_position_changed(struct _App *app)
{
   if (!app->opening)
     return;

   /* sending size info only once */
   int r, w, h;
   r = libvlc_video_get_size(app->mp, 0, &w, &h);
   if (r < 0)
     return;
   _send_resize(app, w, h);

   /* sending audio track info */
   _send_all_track_info(app);

   /* sending meta info */
   _send_all_meta_info(app);

   libvlc_media_player_stop(app->mp);
}

static void
_process_thread_events(struct _App *app)
{
   int event;
   size_t size;

   size = read(app->fd_read, &event, sizeof(event));
   if (size != sizeof(event))
     {
	fprintf(stderr, "player: problem when reading thread event. size = %zd\n", size);
	return;
     }

   switch (event) {
      case EM_THREAD_POSITION_CHANGED:
	 _position_changed(app);
	 break;
   }
}

int
main(int argc, const char *argv[])
{
   struct _App app;
   Emotion_Generic_Video_Shared *vs;
   struct pollfd fds[3];
   int tpipe[2]; // pipe for comunicating events from threads
   char shmname[256];
   char cwidth[64], cheight[64], cpitch[64], chroma[64];
   char buf[64];
   const char *vlc_argv[] =
     {
        "--quiet",
	"--vout",
	"vmem",
	"--vmem-width",
	cwidth,
	"--vmem-height",
	cheight,
	"--vmem-pitch",
	cpitch,
	"--vmem-chroma",
	chroma
     };

   if (argc < 3)
     {
	fprintf(stderr, "player: missing paramters.\n");
	fprintf(stderr, "syntax:\n\t%s <fd read> <fd write>\n", argv[0]);
	return -1;
     }

   app.em_read = atoi(argv[1]);
   app.em_write = atoi(argv[2]);

   fprintf(stderr, "reading commands from fd: %d, writing on fd: %d\n", app.em_read, app.em_write);

   int vlc_argc = sizeof(vlc_argv) / sizeof(*vlc_argv);
   snprintf(cwidth, sizeof(cwidth), "%d", DEFAULTWIDTH);
   snprintf(cheight, sizeof(cheight), "%d", DEFAULTHEIGHT);
   snprintf(cpitch, sizeof(cpitch), "%d", DEFAULTWIDTH * 4);
   snprintf(chroma, sizeof(chroma), "RV32");

   /*
    * Naughty xattr in emotion uses ecore_thread to run its thing, this
    * may leave emotion's reference count high and it won't kill us...
    * letting us play the video in the background.  not good.
    *
    * prctl(PR_SET_PDEATHSIG) is a linux only thing. Need to find ways
    * to do it on other platforms. Until then leave it breaking on
    * such platforms so people port it instead of ignoring.
    */
   prctl(PR_SET_PDEATHSIG, SIGHUP);

   app.libvlc = libvlc_new(vlc_argc, vlc_argv);
   app.mp = NULL;
   app.filename = NULL;
   app.w = 0;
   app.h = 0;
   app.size_sent = 0;
   app.opening = 0;
   app.playing = 0;
   app.closing = 0;

   if (_em_cmd_read(&app) != EM_CMD_INIT)
     {
	fprintf(stderr, "player: wrong init command!\n");
	return -1;
     }

   int size;
   _em_read_safe(app.em_read, &size, sizeof(size));
   _em_read_safe(app.em_read, buf, size);
   app.shmname = strdup(buf);

   _send_cmd(&app, EM_RESULT_INIT);

   pipe(tpipe);
   app.fd_read = tpipe[0];
   app.fd_write = tpipe[1];
   fds[0].fd = app.em_read;
   fds[0].events = POLLIN;
   fds[1].fd = app.fd_read;
   fds[1].events = POLLIN;
   fds[2].fd = STDERR_FILENO;
   fds[2].events = 0;

   while (1)
     {
	int r;

	r = poll(fds, 3, -1);
	if (r == 0)
	  continue;
	else if (r < 0)
	  {
	     fprintf(stderr,
                     "emotion_generic_vlc: an error ocurred on poll(): %s\n",
                     strerror(errno));
	     break;
	  }

        if (fds[0].revents & (POLLERR | POLLHUP | POLLNVAL))
          {
             fputs("emotion_generic_vlc: error communicating with stdin\n",
                   stderr);
             break;
          }
        if (fds[1].revents & (POLLERR | POLLHUP | POLLNVAL))
          {
             fputs("emotion_generic_vlc: error communicating with thread\n",
                   stderr);
             break;
          }

	if (fds[0].revents & POLLIN)
	  _process_emotion_commands(&app);
	if (fds[1].revents & POLLIN)
	  _process_thread_events(&app);
        if (fds[2].revents & (POLLERR | POLLHUP | POLLNVAL))
          break;
     }

   libvlc_release(app.libvlc);


   return 0;
}
#undef SEND_CMD_PARAM
