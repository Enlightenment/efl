#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <Eina.h>
#include <Evas.h>

#include "Emotion.h"
#include "emotion_private.h"
#include "emotion_generic.h"

static Eina_Prefix *pfx = NULL;

static int _emotion_generic_log_domain = -1;
#define DBG(...) EINA_LOG_DOM_DBG(_emotion_generic_log_domain, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_emotion_generic_log_domain, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_emotion_generic_log_domain, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_emotion_generic_log_domain, __VA_ARGS__)
#define CRITICAL(...) EINA_LOG_DOM_CRIT(_emotion_generic_log_domain, __VA_ARGS__)


struct _default_players {
   const char *name;
   const char *cmdline;
};

static struct _default_players players[] = {
#ifdef EMOTION_BUILD_VLC
       { "vlc", "em_generic_vlc" },
#endif
       { NULL, NULL }
};

static const char *
_get_player(const char *name)
{
   const char *selected_name = NULL;
   const char *libdir = eina_prefix_lib_get(pfx);
   static char buf[PATH_MAX];
   int i;

   if (name)
     {
        for (i = 0; players[i].name; i++)
          {
             if (!strcmp(players[i].name, name))
               {
                  selected_name = players[i].cmdline;
                  break;
               }
          }
     }

   if ((!selected_name) && (name))
     selected_name = name;

   if (selected_name)
     {
        const char *cmd;

        if (selected_name[0] == '/') cmd = selected_name;
        else
          {
             snprintf(buf, sizeof(buf), "%s/emotion/utils/%s",
                      libdir, selected_name);
             cmd = buf;
          }

        DBG("Try generic player '%s'", cmd);
        if (access(cmd, R_OK | X_OK) == 0)
          {
             INF("Using generic player '%s'", cmd);
             return cmd;
          }
     }

   for (i = 0; players[i].name; i++)
     {
        snprintf(buf, sizeof(buf), "%s/emotion/utils/%s",
                 libdir, players[i].cmdline);
        DBG("Try generic player '%s'", buf);
        if (access(buf, R_OK | X_OK) == 0)
          {
             INF("Using fallback player '%s'", buf);
             return buf;
          }
     }

   ERR("no generic player found, given name='%s'", name ? name : "");
   return NULL;
}

static void
_player_send_cmd(Emotion_Generic_Video *ev, int cmd)
{
   if (cmd >= EM_CMD_LAST)
     {
	ERR("invalid command to player.");
	return;
     }
   ecore_exe_send(ev->player.exe, &cmd, sizeof(cmd));
}

static void
_player_send_int(Emotion_Generic_Video *ev, int number)
{
   ecore_exe_send(ev->player.exe, &number, sizeof(number));
}

static void
_player_send_float(Emotion_Generic_Video *ev, float number)
{
   ecore_exe_send(ev->player.exe, &number, sizeof(number));
}

static void
_player_send_str(Emotion_Generic_Video *ev, const char *str, Eina_Bool stringshared)
{
   int len;

   if (stringshared)
     len = eina_stringshare_strlen(str) + 1;
   else
     len = strlen(str) + 1;
   ecore_exe_send(ev->player.exe, &len, sizeof(len));
   ecore_exe_send(ev->player.exe, str, len);
}

static Eina_Bool
_create_shm_data(Emotion_Generic_Video *ev, const char *shmname)
{
   int shmfd;
   int npages;
   size_t size;
   Emotion_Generic_Video_Shared *vs;

   shmfd = shm_open(shmname, O_CREAT | O_RDWR | O_TRUNC, 0777);
   size = 3 * (ev->w * ev->h * DEFAULTPITCH) + sizeof(*vs);

   npages = (int)(size / getpagesize()) + 1;
   size = npages * getpagesize();

   if (ftruncate(shmfd, size))
     {
	ERR("error when allocating shared memory (size = %zd): "
	    "%s", size, strerror(errno));
	return EINA_FALSE;
     }
   vs = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, shmfd, 0);
   if (vs == MAP_FAILED)
     {
	ERR("error when mapping shared memory.\n");
	return EINA_FALSE;
     }

   vs->size = size;
   vs->width = ev->w;
   vs->height = ev->h;
   vs->pitch = DEFAULTPITCH;
   vs->frame.emotion = 0;
   vs->frame.player = 1;
   vs->frame.last = 2;
   vs->frame.next = 2;
   sem_init(&vs->lock, 1, 1);
   ev->frame.frames[0] = (unsigned char *)vs + sizeof(*vs);
   ev->frame.frames[1] = (unsigned char *)vs + sizeof(*vs) + vs->height * vs->width * vs->pitch;
   ev->frame.frames[2] = (unsigned char *)vs + sizeof(*vs) + 2 * vs->height * vs->width * vs->pitch;

   if (ev->shared)
     munmap(ev->shared, ev->shared->size);
   ev->shared = vs;

   return EINA_TRUE;
}

static void
_player_new_frame(Emotion_Generic_Video *ev)
{
   if (!ev->drop++)
     _emotion_frame_new(ev->obj);
}

static void
_player_file_set_done(Emotion_Generic_Video *ev)
{
   if (!_create_shm_data(ev, ev->shmname))
     {
	ERR("could not create shared memory.");
	return;
     }
   _player_send_cmd(ev, EM_CMD_FILE_SET_DONE);
}

static void
_file_open(Emotion_Generic_Video *ev)
{
   INF("Opening file: %s", ev->filename);
   ev->w = DEFAULTWIDTH;
   ev->h = DEFAULTHEIGHT;
   ev->ratio = (double)DEFAULTWIDTH / DEFAULTHEIGHT;
   ev->speed = 1.0;
   ev->len = 0;
   ev->drop = 0;

   if (!ev->ready)
     return;
   _player_send_cmd(ev, EM_CMD_FILE_SET);
   _player_send_str(ev, ev->filename, EINA_TRUE);
}

static void
_player_ready(Emotion_Generic_Video *ev)
{
   INF("received: player ready.");

   ev->initializing = EINA_FALSE;
   ev->ready = EINA_TRUE;

   if (!ev->filename)
     return;

   _file_open(ev);
}

#define RCV_CMD_PARAM(src, param) \
   memcpy(&(param), (src), sizeof((param))); \
   (src) = (char *)(src) + sizeof((param));

#define RCV_CMD_STR(src, buf, len) \
   RCV_CMD_PARAM((src), (len)); \
   memcpy((buf), (src), (len)); \
   (src) = (char *)(src) + len;

static int
_player_int_read(Emotion_Generic_Video *ev __UNUSED__, void **data)
{
   int number;
   memcpy(&number, *data, sizeof(number));
   *data = (char *)(*data) + sizeof(number);

   return number;
}

static void
_player_frame_resize(Emotion_Generic_Video *ev, void *line)
{
   int w, h;
   RCV_CMD_PARAM(line, w);
   RCV_CMD_PARAM(line, h);

   INF("received frame resize: %dx%d", w, h);
   ev->w = w;
   ev->h = h;
   ev->ratio = (float)w / h;

   if (ev->opening)
     return;

   _emotion_frame_resize(ev->obj, ev->w, ev->h, ev->ratio);
}

static void
_player_length_changed(Emotion_Generic_Video *ev, void *line)
{
   float length;
   RCV_CMD_PARAM(line, length);

   INF("received length changed: %0.3f", length);

   ev->len = length;
   _emotion_video_pos_update(ev->obj, ev->pos, ev->len);
}

static void
_player_position_changed(Emotion_Generic_Video *ev, void *line)
{
   float position;
   RCV_CMD_PARAM(line, position);

   INF("received position changed: %0.3f", position);

   ev->pos = position;
   _emotion_video_pos_update(ev->obj, ev->pos, ev->len);

   if (ev->len == 0)
     return;

   float progress = ev->pos / ev->len;
   char buf[16];
   snprintf(buf, sizeof(buf), "%0.1f%%", progress * 100);

   _emotion_progress_set(ev->obj, buf, progress);
}

static void
_player_seekable_changed(Emotion_Generic_Video *ev, void *line)
{
   int seekable;
   RCV_CMD_PARAM(line, seekable);

   INF("received seekable changed: %d", seekable);

   seekable = !!seekable;

   ev->seekable = seekable;
}

static void
_player_volume(Emotion_Generic_Video *ev, void *line)
{
   float vol, oldvol;
   RCV_CMD_PARAM(line, vol);

   INF("received volume: %0.3f", vol);

   oldvol = ev->volume;
   ev->volume = vol;
   if (vol != oldvol && !ev->opening)
     _emotion_audio_level_change(ev->obj);
}

static void
_player_audio_mute(Emotion_Generic_Video *ev, void *line)
{
   int mute;
   RCV_CMD_PARAM(line, mute);

   INF("received audio mute: %d", mute);

   ev->audio_mute = !!mute;
}

static void
_audio_channels_free(Emotion_Generic_Video *ev)
{
   int i;
   for (i = 0; i < ev->audio_channels_count; i++)
     eina_stringshare_del(ev->audio_channels[i].name);
   free(ev->audio_channels);
   ev->audio_channels_count = 0;
}

static void
_player_audio_tracks_info(Emotion_Generic_Video *ev, void *line)
{
   int track_current, tracks_count;
   int i;

   if (ev->audio_channels_count)
     _audio_channels_free(ev);

   RCV_CMD_PARAM(line, track_current);
   RCV_CMD_PARAM(line, tracks_count);
   INF("video with %d audio tracks (current = %d):", tracks_count, track_current);
   ev->audio_channels = calloc(
      tracks_count, sizeof(Emotion_Generic_Audio_Channel));
   ev->audio_channels_count = tracks_count;
   ev->audio_channel_current = track_current;
   for (i = 0; i < tracks_count; i++)
     {
	int tid, len;
	char buf[PATH_MAX];
	RCV_CMD_PARAM(line, tid);
	RCV_CMD_STR(line, buf, len);
	ev->audio_channels[i].id = tid;
	ev->audio_channels[i].name = eina_stringshare_add_length(buf, len);
	INF("\t%d: %s", tid, buf);
     }
}

static void
_player_file_closed(Emotion_Generic_Video *ev)
{
   INF("Closed previous file.");
   sem_destroy(&ev->shared->lock);

   ev->closing = EINA_FALSE;

   if (ev->opening)
     _file_open(ev);
}

static void
_player_open_done(Emotion_Generic_Video *ev)
{
   ev->opening = EINA_FALSE;
   shm_unlink(ev->shmname);
   _emotion_open_done(ev->obj);

   if (ev->play)
     {
	_player_send_cmd(ev, EM_CMD_PLAY);
	_player_send_float(ev, ev->pos);
     }

   INF("Open done");
}

static void
_player_read_cmd(Emotion_Generic_Video *ev, void *line, int size __UNUSED__)
{
   int type;
   RCV_CMD_PARAM(line, type);

   switch (type) {
      case EM_RESULT_INIT:
	 _player_ready(ev);
	 break;
      case EM_RESULT_FRAME_NEW:
	 _player_new_frame(ev);
	 break;
      case EM_RESULT_FILE_SET:
	 _player_file_set_done(ev);
	 break;
      case EM_RESULT_FILE_SET_DONE:
	 _player_open_done(ev);
	 break;
      case EM_RESULT_FILE_CLOSE:
	 _player_file_closed(ev);
	 break;
      case EM_RESULT_PLAYBACK_STOPPED:
	 _emotion_playback_finished(ev->obj);
	 break;
      case EM_RESULT_FRAME_SIZE:
	 _player_frame_resize(ev, line);
	 break;
      case EM_RESULT_LENGTH_CHANGED:
	 _player_length_changed(ev, line);
	 break;
      case EM_RESULT_POSITION_CHANGED:
	 _player_position_changed(ev, line);
	 break;
      case EM_RESULT_SEEKABLE_CHANGED:
	 _player_seekable_changed(ev, line);
	 break;
      case EM_RESULT_AUDIO_TRACK_INFO:
	 _player_audio_tracks_info(ev, line);
	 break;
      default:
	 WRN("received wrong command: %d", type);
   };
}

#undef RCV_CMD_PARAM

static Eina_Bool
_player_data_cb(void *data, int type __UNUSED__, void *event)
{
   Ecore_Exe_Event_Data *ev = event;
   Emotion_Generic_Video *evideo = data;
   int psize;
   char *pdata;
   int i;

   if (ev->exe != evideo->player.exe)
     {
        ERR("slave != ev->exe");
	return ECORE_CALLBACK_DONE;
     }

   if (ev->size < 4)
     {
	ERR("invalid command: missing bytes.");
	return ECORE_CALLBACK_DONE;
     }

   for (i = 0; ev->lines[i].line; i++)
     _player_read_cmd(evideo, ev->lines[i].line, ev->lines[i].size);

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_player_add_cb(void *data, int type __UNUSED__, void *event)
{
   Ecore_Exe_Event_Add *event_add = event;
   Ecore_Exe *player = event_add->exe;
   Emotion_Generic_Video *ev = data;

   if (ev->player.exe != player)
     {
	ERR("ev->player != player.");
	return ECORE_CALLBACK_DONE;
     }

   _player_send_cmd(ev, EM_CMD_INIT);
   _player_send_str(ev, ev->shmname, EINA_TRUE);

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_player_del_cb(void *data, int type __UNUSED__, void *event __UNUSED__)
{
   Ecore_Exe_Event_Del *event_del = event;
   Ecore_Exe *player = event_del->exe;
   Emotion_Generic_Video *ev = data;
   ERR("player died.");

   ev->player.exe = NULL;
   ev->ready = EINA_FALSE;
   _emotion_decode_stop(ev->obj);

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_fork_and_exec(Evas_Object *obj __UNUSED__, Emotion_Generic_Video *ev)
{
   char shmname[256];
   struct timeval tv;

   gettimeofday(&tv, NULL);
   snprintf(shmname, sizeof(shmname), "/em-generic-shm_%d_%d",
	    (int)tv.tv_sec, (int)tv.tv_usec);

   ev->shmname = eina_stringshare_add(shmname);

   ev->player_add = ecore_event_handler_add(
      ECORE_EXE_EVENT_ADD, _player_add_cb, ev);
   ev->player_del = ecore_event_handler_add(
      ECORE_EXE_EVENT_DEL, _player_del_cb, ev);
   ev->player_data = ecore_event_handler_add(
      ECORE_EXE_EVENT_DATA, _player_data_cb, ev);

   ev->player.exe = ecore_exe_pipe_run(
      ev->cmdline,
      ECORE_EXE_PIPE_READ | ECORE_EXE_PIPE_WRITE |
      ECORE_EXE_PIPE_READ_LINE_BUFFERED | ECORE_EXE_NOT_LEADER,
      ev);

   if (!ev->player.exe)
     {
        ERR("could not start player.");
        return EINA_FALSE;
     }

   ev->initializing = EINA_TRUE;

   return EINA_TRUE;
}

static unsigned char
em_init(Evas_Object *obj, void **emotion_video, Emotion_Module_Options *opt)
{
   Emotion_Generic_Video *ev;
   const char *player;

   if (!emotion_video) return 0;
   player = _get_player(opt ? opt->player : NULL);
   if (!player) return 0;

   ev = (Emotion_Generic_Video *)calloc(1, sizeof(*ev));
   if (!ev) return 0;

   ev->speed = 1.0;
   ev->volume = 0.5;
   ev->audio_mute = EINA_FALSE;

   ev->obj = obj;
   ev->cmdline = eina_stringshare_add(player);
   *emotion_video = ev;

   return _fork_and_exec(obj, ev);
}

static int
em_shutdown(void *data)
{
   Emotion_Generic_Video *ev = data;

   if (!ev) return 0;

   if (ev->player.exe)
     {
	ecore_exe_terminate(ev->player.exe);
	ecore_exe_free(ev->player.exe);
	ev->player.exe = NULL;
     }

   if (ev->shared)
     munmap(ev->shared, ev->shared->size);

   _audio_channels_free(ev);

   eina_stringshare_del(ev->cmdline);
   eina_stringshare_del(ev->shmname);

   ecore_event_handler_del(ev->player_add);
   ecore_event_handler_del(ev->player_data);
   ecore_event_handler_del(ev->player_del);

   return 1;
}

static unsigned char
em_file_open(const char *file, Evas_Object *obj __UNUSED__, void *data)
{
   Emotion_Generic_Video *ev = data;
   INF("file set: %s", file);
   if (!ev) return 0;

   ev->pos = 0;
   ev->opening = EINA_TRUE;

   eina_stringshare_replace(&ev->filename, file);

   if (!ev->closing)
     _file_open(ev);

   return 1;
}

static void
em_file_close(void *data)
{
   Emotion_Generic_Video *ev = data;

   if (!ev) return;
   INF("file close: %s", ev->filename);

   if (!ev->filename)
     return;

   _player_send_cmd(ev, EM_CMD_FILE_CLOSE);
   ev->closing = EINA_TRUE;
}

static Emotion_Format
em_format_get(void *ef __UNUSED__)
{
   return EMOTION_FORMAT_BGRA;
}

static void
em_video_data_size_get(void *data, int *w, int *h)
{
   Emotion_Generic_Video *ev = data;

   if (!ev) return;
   if (w) *w = ev->w;
   if (h) *h = ev->h;
}

static void
em_play(void *data, double pos)
{
   Emotion_Generic_Video *ev = data;

   if (!ev)
     return;

   ev->play = EINA_TRUE;
   INF("play: %0.3f", pos);

   if (ev->initializing || ev->opening)
     return;

   if (ev->ready)
     {
	_player_send_cmd(ev, EM_CMD_PLAY);
	_player_send_float(ev, ev->pos);
	return;
     }

   ev->player.exe = ecore_exe_pipe_run(
      ev->cmdline,
      ECORE_EXE_PIPE_READ | ECORE_EXE_PIPE_WRITE |
      ECORE_EXE_PIPE_READ_LINE_BUFFERED | ECORE_EXE_NOT_LEADER,
      ev);

   if (!ev->player.exe)
     ERR("could not start player.");
}

static void
em_stop(void *data)
{
   Emotion_Generic_Video *ev = data;

   if (!ev)
     return;

   ev->play = EINA_FALSE;

   if (!ev->ready)
     return;

   _player_send_cmd(ev, EM_CMD_STOP);
   _emotion_decode_stop(ev->obj);
}

static void
em_size_get(void *data, int *w, int *h)
{
   Emotion_Generic_Video *ev = data;
   if(w) *w = ev->w;
   if(h) *h = ev->h;
}

static void
em_pos_set(void *data, double pos)
{
   Emotion_Generic_Video *ev = data;
   float position = pos;
   _player_send_cmd(ev, EM_CMD_POSITION_SET);
   _player_send_float(ev, position);
   _emotion_seek_done(ev->obj);
}

static double
em_len_get(void *data)
{
   Emotion_Generic_Video *ev = data;
   return ev->len;
}

static int
em_fps_num_get(void *data)
{
   Emotion_Generic_Video *ev = data;
   return (int)(ev->fps * 1000.0);
}

static int
em_fps_den_get(void *ef __UNUSED__)
{
   return 1000;
}

static double
em_fps_get(void *data)
{
   Emotion_Generic_Video *ev = data;
   return ev->fps;
}

static double
em_pos_get(void *data)
{
   Emotion_Generic_Video *ev = data;
   return ev->pos;
}

static void
em_vis_set(void *ef __UNUSED__, Emotion_Vis vis __UNUSED__)
{
}

static Emotion_Vis
em_vis_get(void *data)
{
   Emotion_Generic_Video *ev = data;
   return ev->vis;
}

static Eina_Bool
em_vis_supported(void *ef __UNUSED__, Emotion_Vis vis __UNUSED__)
{
   return EINA_FALSE;
}

static double
em_ratio_get(void *data)
{
   Emotion_Generic_Video *ev = data;
   return ev->ratio;
}

static int em_video_handled(void *ef __UNUSED__)
{
   fprintf(stderr, "video handled!\n");
   return 1;
}

static int em_audio_handled(void *ef __UNUSED__)
{
   fprintf(stderr, "audio handled!\n");
   return 1;
}

static int em_seekable(void *data)
{
   Emotion_Generic_Video *ev = data;
   return ev->seekable;
}

static void em_frame_done(void *ef __UNUSED__)
{
}

static int
em_yuv_rows_get(void *data __UNUSED__, int w __UNUSED__, int h __UNUSED__, unsigned char **yrows __UNUSED__, unsigned char **urows __UNUSED__, unsigned char **vrows __UNUSED__)
{
   Emotion_Generic_Video *ev;
   volatile Emotion_Generic_Video_Shared *vs;
   return 0;
}

static int
em_bgra_data_get(void *data, unsigned char **bgra_data)
{
   Emotion_Generic_Video *ev = data;

   if (!ev || ev->opening || ev->closing)
     return 0;

   // lock frame here
   sem_wait(&ev->shared->lock);

   // send current frame to emotion
   if (ev->shared->frame.emotion != ev->shared->frame.last)
     {
	ev->shared->frame.next = ev->shared->frame.emotion;
	ev->shared->frame.emotion = ev->shared->frame.last;
     }
   *bgra_data = ev->frame.frames[ev->shared->frame.emotion];

   // unlock frame here
   sem_post(&ev->shared->lock);
   ev->drop = 0;

   return 1;
}

static void
em_event_feed(void *ef __UNUSED__, int event __UNUSED__)
{
}

static void
em_event_mouse_button_feed(void *ef __UNUSED__, int button __UNUSED__, int x __UNUSED__, int y __UNUSED__)
{
}

static void
em_event_mouse_move_feed(void *ef __UNUSED__, int x __UNUSED__, int y __UNUSED__)
{
}

static int
em_video_channel_count(void *ef __UNUSED__)
{
   int ret  = 0;
   return ret;
}

static void
em_video_channel_set(void *ef __UNUSED__, int channel __UNUSED__)
{
}

static int
em_video_channel_get(void *ef __UNUSED__)
{
   return 1;
}

static const char *
em_video_channel_name_get(void *ef __UNUSED__, int channel __UNUSED__)
{
   return NULL;
}

static void
em_video_channel_mute_set(void *ef __UNUSED__, int mute __UNUSED__)
{
}

static int
em_video_channel_mute_get(void *data)
{
   Emotion_Generic_Video *ev = data;
   return ev->video_mute;
}

static int
em_audio_channel_count(void *data)
{
   Emotion_Generic_Video *ev = data;
   return ev->audio_channels_count;
}

static void
em_audio_channel_set(void *data, int channel)
{
   Emotion_Generic_Video *ev = data;
   int i;

   for (i = 0; i < ev->audio_channels_count; i++)
     {
	if (ev->audio_channels[i].id == channel)
	  {
	     _player_send_cmd(ev, EM_CMD_AUDIO_TRACK_SET);
	     _player_send_int(ev, channel);
	     break;
	  }
     }
}

static int
em_audio_channel_get(void *data)
{
   Emotion_Generic_Video *ev = data;
   return ev->audio_channel_current;
}

static const char *
em_audio_channel_name_get(void *data, int channel)
{
   Emotion_Generic_Video *ev = data;
   int i;

   for (i = 0; i < ev->audio_channels_count; i++)
     {
	if (ev->audio_channels[i].id == channel)
	  return ev->audio_channels[i].name;
     }

   return NULL;
}

static void
em_audio_channel_mute_set(void *data, int mute)
{
   Emotion_Generic_Video *ev = data;
   _player_send_cmd(ev, EM_CMD_AUDIO_MUTE_SET);
   _player_send_int(ev, mute);
   ev->audio_mute = !!mute;
}

static int
em_audio_channel_mute_get(void *data)
{
   Emotion_Generic_Video *ev = data;
   return ev->audio_mute;
}

static void
em_audio_channel_volume_set(void *data, double vol)
{
   Emotion_Generic_Video *ev = data;
   float fvol;

   if (vol > 1.0) vol = 1.0;
   if (vol < 0.0) vol = 0.0;

   fvol = vol;
   _player_send_cmd(ev, EM_CMD_VOLUME_SET);
   _player_send_float(ev, fvol);

   ev->volume = vol;
}

static double
em_audio_channel_volume_get(void *data)
{
   Emotion_Generic_Video *ev = data;
   return ev->volume;
}

static int
em_spu_channel_count(void *ef __UNUSED__)
{
   return 0;
}

static void
em_spu_channel_set(void *ef __UNUSED__, int channel __UNUSED__)
{
}

static int
em_spu_channel_get(void *ef __UNUSED__)
{
   int num = 0;
   return num;
}

static const char *
em_spu_channel_name_get(void *ef __UNUSED__, int channel __UNUSED__)
{
   return NULL;
}

static void
em_spu_channel_mute_set(void *ef __UNUSED__, int mute __UNUSED__)
{
   return;
}

static int
em_spu_channel_mute_get(void *ef __UNUSED__)
{
   return 0;
}

static int
em_chapter_count(void *ef __UNUSED__)
{
   int num = 0;
   return num;
}

static void
em_chapter_set(void *ef __UNUSED__, int chapter __UNUSED__)
{
}

static int
em_chapter_get(void *ef __UNUSED__)
{
   int num = 0;
   return num;
}

static const char *
em_chapter_name_get(void *ef __UNUSED__, int chapter __UNUSED__)
{
   return NULL;
}

static void
em_speed_set(void *data, double speed)
{
   Emotion_Generic_Video *ev = data;
   float rate = speed;

   _player_send_cmd(ev, EM_CMD_SPEED_SET);
   _player_send_float(ev, rate);

   ev->speed = rate;
}

static double
em_speed_get(void *data)
{
   Emotion_Generic_Video *ev = data;
   return (double)ev->speed;
}

static int
em_eject(void *ef __UNUSED__)
{
   return 1;
}

static const char *
em_meta_get(void *ef __UNUSED__, int meta __UNUSED__)
{
   char * meta_data = NULL;
   return meta_data;
}

static Emotion_Video_Module em_module =
{
   em_init, /* init */
   em_shutdown, /* shutdown */
   em_file_open, /* file_open */
   em_file_close, /* file_close */
   em_play, /* play */
   em_stop, /* stop */
   em_size_get, /* size_get */
   em_pos_set, /* pos_set */
   em_len_get, /* len_get */
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
   em_video_data_size_get, /* video_data_size_get */
   em_yuv_rows_get, /* yuv_rows_get */
   em_bgra_data_get, /* bgra_data_get */
   em_event_feed, /* event_feed */
   em_event_mouse_button_feed, /* event_mouse_button_feed */
   em_event_mouse_move_feed, /* event_mouse_move_feed */
   em_video_channel_count, /* video_channel_count */
   em_video_channel_set, /* video_channel_set */
   em_video_channel_get, /* video_channel_get */
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
   NULL /* handle */
};

static Eina_Bool
module_open(Evas_Object *obj, const Emotion_Video_Module **module, void **video, Emotion_Module_Options *opt)
{
   if (!module)	{
	return EINA_FALSE;
   }

   if (_emotion_generic_log_domain < 0)
     {
        eina_threads_init();
        eina_log_threads_enable();
        _emotion_generic_log_domain = eina_log_domain_register
          ("emotion-generic", EINA_COLOR_LIGHTCYAN);
        if (_emotion_generic_log_domain < 0)
          {
             EINA_LOG_CRIT("Could not register log domain 'emotion-generic'");
             return EINA_FALSE;
          }
     }


   if (!em_module.init(obj, video, opt))	{
	return EINA_FALSE;
   }

   *module = &em_module;

   return EINA_TRUE;
}

static void module_close(Emotion_Video_Module *module __UNUSED__, void *video)
{
	em_module.shutdown(video);
}


static Eina_Bool
generic_module_init(void)
{
   if (!pfx)
     {
        pfx = eina_prefix_new(NULL, emotion_object_add,
                              "EMOTION", "emotion", NULL,
                              PACKAGE_BIN_DIR,
                              PACKAGE_LIB_DIR,
                              PACKAGE_DATA_DIR,
                              "");
        if (!pfx) return EINA_FALSE;
     }
   return _emotion_module_register("generic", module_open, module_close);
}

static void
generic_module_shutdown(void)
{
   if (pfx)
     {
        eina_prefix_free(pfx);
        pfx = NULL;
     }
   _emotion_module_unregister("generic");
}

#ifndef EMOTION_STATIC_BUILD_GENERIC

EINA_MODULE_INIT(generic_module_init);
EINA_MODULE_SHUTDOWN(generic_module_shutdown);

#endif

