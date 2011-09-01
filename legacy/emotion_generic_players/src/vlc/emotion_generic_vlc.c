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
     int w, h;
     int fd_read;
     int fd_write;
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
_em_str_read(char **str)
{
   int size;
   int r;
   char buf[PATH_MAX];

   r = _em_read_safe(STDIN_FILENO, &size, sizeof(size));
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

   r = _em_read_safe(STDIN_FILENO, buf, size);
   if (!r)
     {
        *str = NULL;
        return 0;
     }

   *str = strdup(buf);
   return 1;
}

static int
_em_cmd_read(void)
{
   int cmd;
   _em_read_safe(STDIN_FILENO, &cmd, sizeof(cmd));

   return cmd;
}

static void
_send_cmd_start(int cmd)
{
   pthread_mutex_lock(&_mutex_fd);
   _em_write_safe(STDOUT_FILENO, &cmd, sizeof(cmd));
}

static void
_send_cmd_finish(void)
{
   static const char c = '\n';
   _em_write_safe(STDOUT_FILENO, &c, sizeof(c));
   pthread_mutex_unlock(&_mutex_fd);
}

static void
_send_cmd(int cmd)
{
   _send_cmd_start(cmd);
   _send_cmd_finish();
}

static void
_send_cmd_str(const char *str)
{
   int len;
   len = strlen(str) + 1;
   _em_write_safe(STDOUT_FILENO, &len, sizeof(len));
   _em_write_safe(STDOUT_FILENO, str, len);
}

#define SEND_CMD_PARAM(i) \
   _em_write_safe(STDOUT_FILENO, &(i), sizeof((i)));

static void
_send_resize(int width, int height)
{
   _send_cmd_start(EM_RESULT_FRAME_SIZE);
   SEND_CMD_PARAM(width);
   SEND_CMD_PARAM(height);
   _send_cmd_finish();
}

static void
_send_length_changed(const struct libvlc_event_t *ev)
{
   float length = ev->u.media_player_length_changed.new_length;
   length /= 1000;

   fprintf(stderr, "length changed: %0.3f\n", length);
   _send_cmd_start(EM_RESULT_LENGTH_CHANGED);
   SEND_CMD_PARAM(length);
   _send_cmd_finish();
}

static void
_send_time_changed(const struct libvlc_event_t *ev)
{
   float new_time = ev->u.media_player_time_changed.new_time;
   new_time /= 1000;
   _send_cmd_start(EM_RESULT_POSITION_CHANGED);
   SEND_CMD_PARAM(new_time);
   _send_cmd_finish();
}

static void
_send_seekable_changed(const struct libvlc_event_t *ev)
{
   int seekable = ev->u.media_player_seekable_changed.new_seekable;
   _send_cmd_start(EM_RESULT_SEEKABLE_CHANGED);
   SEND_CMD_PARAM(seekable);
   _send_cmd_finish();
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
   struct _App *app = data;

   if (!app->playing)
     return;

   sem_wait(&app->vs->lock);
   app->vs->frame.last = app->vs->frame.player;
   app->vs->frame.player = app->vs->frame.next;
   app->vs->frame.next = app->vs->frame.last;

   sem_post(&app->vs->lock);
}

static void
_display(void *data, void *id)
{
   struct _App *app = data;
   if (!app->playing)
     return;

   _send_cmd(EM_RESULT_FRAME_NEW);
}

static void *
_tmp_lock(void *data, void **pixels)
{
   *pixels = NULL;
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

   _em_read_safe(STDIN_FILENO, &pos, sizeof(pos));

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
   _send_cmd(EM_RESULT_FILE_CLOSE);
}

static void
_send_file_set(struct _App *app)
{
   if (app->opening)
      _send_cmd(EM_RESULT_FILE_SET);

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
	 _send_time_changed(ev);
	 break;
      case libvlc_MediaPlayerPositionChanged:
	 thread_event = EM_THREAD_POSITION_CHANGED;
	 write(app->fd_write, &thread_event, sizeof(thread_event));
	 break;
      case libvlc_MediaPlayerLengthChanged:
	 _send_length_changed(ev);
	 break;
      case libvlc_MediaPlayerSeekableChanged:
	 _send_seekable_changed(ev);
	 break;
      case libvlc_MediaPlayerPlaying:
	 _send_resize(app->w, app->h);
	 break;
      case libvlc_MediaPlayerStopped:
	 _send_file_set(app);
	 break;
      case libvlc_MediaPlayerEndReached:
	 _send_cmd(EM_RESULT_PLAYBACK_STOPPED);
	 break;
   }
}

static void
_file_set(struct _App *app)
{
   _em_str_read(&app->filename);

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
   libvlc_video_set_callbacks(app->mp, _tmp_lock, _tmp_unlock, _tmp_display, NULL);
   app->event_mgr = libvlc_media_player_event_manager(app->mp);
   libvlc_event_attach(app->event_mgr, libvlc_MediaPlayerPositionChanged,
		       _event_cb, app);
   libvlc_event_attach(app->event_mgr, libvlc_MediaPlayerStopped,
		       _event_cb, app);

   app->mevent_mgr = libvlc_media_event_manager(app->m);

   libvlc_audio_set_mute(app->mp, 1);
   libvlc_media_player_play(app->mp);
}

static void
_position_set(struct _App *app)
{
   if (!app->mp)
     return;

   float position;
   _em_read_safe(STDIN_FILENO, &position, sizeof(position));

   libvlc_time_t new_time = position * 1000;
   libvlc_media_player_set_time(app->mp, new_time);
}

static void
_speed_set(struct _App *app)
{
   float rate;

   if (!app->mp)
     return;

   _em_read_safe(STDIN_FILENO, &rate, sizeof(rate));

   libvlc_media_player_set_rate(app->mp, rate);
}

static void
_mute_set(struct _App *app)
{
   int mute;

   if (!app->mp)
     return;

   _em_read_safe(STDIN_FILENO, &mute, sizeof(mute));

   libvlc_audio_set_mute(app->mp, mute);
}

static void
_volume_set(struct _App *app)
{
   float volume;
   int vol;

   if (!app->mp)
     return;

   _em_read_safe(STDIN_FILENO, &volume, sizeof(volume));
   vol = volume * 100;

   libvlc_audio_set_volume(app->mp, vol);
}

static void
_audio_track_set(struct _App *app)
{
   int track;

   _em_read_safe(STDIN_FILENO, &track, sizeof(track));

   libvlc_audio_set_track(app->mp, track);
}

static void
_file_set_done(struct _App *app)
{
   emotion_generic_shm_get(app->shmname, &app->vs, &app->vf);
   app->w = app->vs->width;
   app->h = app->vs->height;
   libvlc_video_set_format(app->mp, "RV32", app->w, app->h, app->w * 4);
   libvlc_video_set_callbacks(app->mp, _lock, _unlock, _display, app);
   app->opening = 0;


   libvlc_event_attach(app->event_mgr, libvlc_MediaPlayerPlaying,
		       _event_cb, app);
   libvlc_event_attach(app->event_mgr, libvlc_MediaPlayerTimeChanged,
		       _event_cb, app);
   libvlc_event_attach(app->event_mgr, libvlc_MediaPlayerLengthChanged,
		       _event_cb, app);
   libvlc_event_attach(app->event_mgr, libvlc_MediaPlayerSeekableChanged,
		       _event_cb, app);

   libvlc_audio_set_mute(app->mp, 0);
   _send_cmd(EM_RESULT_FILE_SET_DONE);
}

static void
_file_close(struct _App *app)
{
   app->playing = 0;
   if (libvlc_media_player_get_state(app->mp) != libvlc_Playing)
     {
	_send_file_closed(app);
	return;
     }

   app->closing = 1;
   libvlc_media_player_stop(app->mp);
   if (app->filename)
     free(app->filename);
   if (app->mp)
     {
        libvlc_media_release(app->m);
        libvlc_media_player_release(app->mp);
     }
}

static void
_process_emotion_commands(struct _App *app)
{
   int cmd = _em_cmd_read();
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
   };
}

static void
_send_track_info(libvlc_media_player_t *mp)
{
   int track_count, current;
   libvlc_track_description_t *desc;

   current = libvlc_audio_get_track(mp);
   track_count = libvlc_audio_get_track_count(mp);
   desc = libvlc_audio_get_track_description(mp);

   _send_cmd_start(EM_RESULT_AUDIO_TRACK_INFO);
   SEND_CMD_PARAM(current);
   SEND_CMD_PARAM(track_count);
   while (desc)
     {
	int tid = desc->i_id;
	const char *name = desc->psz_name;
	SEND_CMD_PARAM(tid);
	_send_cmd_str(name);
	desc = desc->p_next;
     }
   _send_cmd_finish();
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
   _send_resize(w, h);

   /* sending audio track info */
   // _send_track_info(app->mp);

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
   struct pollfd fds[2]; // watching on 2 file descriptors
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

   int vlc_argc = sizeof(vlc_argv) / sizeof(*vlc_argv);
   snprintf(cwidth, sizeof(cwidth), "%d", DEFAULTWIDTH);
   snprintf(cheight, sizeof(cheight), "%d", DEFAULTHEIGHT);
   snprintf(cpitch, sizeof(cpitch), "%d", DEFAULTWIDTH * 4);
   snprintf(chroma, sizeof(chroma), "RV32");

   app.libvlc = libvlc_new(vlc_argc, vlc_argv);
   app.mp = NULL;
   app.filename = NULL;
   app.w = 0;
   app.h = 0;
   app.size_sent = 0;
   app.opening = 0;
   app.playing = 0;
   app.closing = 0;

   if (_em_cmd_read() != EM_CMD_INIT)
     {
	fprintf(stderr, "player: wrong init command!\n");
	return -1;
     }

   int size;
   _em_read_safe(STDIN_FILENO, &size, sizeof(size));
   _em_read_safe(STDIN_FILENO, buf, size);
   app.shmname = strdup(buf);

   _send_cmd(EM_RESULT_INIT);

   pipe(tpipe);
   app.fd_read = tpipe[0];
   app.fd_write = tpipe[1];
   fds[0].fd = STDIN_FILENO;
   fds[0].events = POLLIN;
   fds[1].fd = app.fd_read;
   fds[1].events = POLLIN;

   while (1)
     {
	int r;

	r = poll(fds, 2, 30);
	if (r == 0)
	  continue;
	else if (r < 0)
	  {
	     fprintf(stderr, "an error ocurred on poll().\n");
	     break;
	  }

	if (fds[0].revents & POLLIN)
	  _process_emotion_commands(&app);
	if (fds[1].revents & POLLIN)
	  _process_thread_events(&app);
     }

   libvlc_release(app.libvlc);


   return 0;
}
#undef SEND_CMD_PARAM
