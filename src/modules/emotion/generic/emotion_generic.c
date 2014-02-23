#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <Eina.h>
#include <Evas.h>
#include <Ecore.h>

#include "emotion_modules.h"
#include "emotion_generic.h"

static Eina_Prefix *pfx = NULL;
static Eina_List *_generic_players = NULL;
static int _emotion_init_count = 0;

static int _emotion_generic_log_domain = -1;
#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_emotion_generic_log_domain, __VA_ARGS__)

#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_emotion_generic_log_domain, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_emotion_generic_log_domain, __VA_ARGS__)

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_emotion_generic_log_domain, __VA_ARGS__)

#ifdef CRI
#undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_emotion_generic_log_domain, __VA_ARGS__)


static Eina_Bool _fork_and_exec(Emotion_Generic_Video *ev);
static void em_partial_shutdown(Emotion_Generic_Video *ev);


static void
_player_send_cmd(Emotion_Generic_Video *ev, int cmd)
{
   if (cmd >= EM_CMD_LAST)
     {
        ERR("invalid command to player.");
        return;
     }
   if (!ev->fd_write)
     {
        ERR("you should wait for emotion to be ready to take action.");
        return;
     }
   ecore_pipe_write(ev->fd_write, &cmd, sizeof(cmd));
}

static void
_player_send_int(Emotion_Generic_Video *ev, int number)
{
   if (!ev->fd_write)
     {
        ERR("you should wait for emotion to be ready to take action.");
        return;
     }
   ecore_pipe_write(ev->fd_write, &number, sizeof(number));
}

static void
_player_send_float(Emotion_Generic_Video *ev, float number)
{
   if (!ev->fd_write)
     {
        ERR("you should wait for emotion to be ready to take action.");
        return;
     }
   ecore_pipe_write(ev->fd_write, &number, sizeof(number));
}

static void
_player_send_str(Emotion_Generic_Video *ev, const char *str, Eina_Bool stringshared)
{
   int len;

   if (stringshared)
     len = str ? eina_stringshare_strlen(str) + 1 : 0;
   else
     len = str ? strlen(str) + 1 : 0;

   if (str)
     ecore_pipe_write(ev->fd_write, str, len);
}

static Eina_Bool
_create_shm_data(Emotion_Generic_Video *ev, const char *shmname)
{
   int shmfd;
   int npages;
   size_t size;
   Emotion_Generic_Video_Shared *vs;

   shmfd = shm_open(shmname, O_CREAT | O_RDWR | O_TRUNC, 0700);
   if (shmfd == -1)
     {
        ERR("player: could not create shm %s: %s", shmname,  strerror(errno));
        return 0;
     }
   size = 3 * (ev->w * ev->h * DEFAULTPITCH) + sizeof(*vs);

   npages = (int)(size / getpagesize()) + 1;
   size = npages * getpagesize();

   if (ftruncate(shmfd, size))
     {
        ERR("error when allocating shared memory (size = %zd): "
            "%s", size, strerror(errno));
        close(shmfd);
        shm_unlink(shmname);
        return EINA_FALSE;
     }
   vs = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, shmfd, 0);
   if (vs == MAP_FAILED)
     {
        ERR("error when mapping shared memory: %s", strerror(errno));
        close(shmfd);
        shm_unlink(shmname);
        return EINA_FALSE;
     }
   close(shmfd);

   vs->size = size;
   vs->width = ev->w;
   vs->height = ev->h;
   vs->pitch = DEFAULTPITCH;
   vs->frame.emotion = 0;
   vs->frame.player = 1;
   vs->frame.last = 2;
   vs->frame.next = 2;
   vs->frame_drop = 0;
   if (!eina_semaphore_new(&vs->lock, 1))
     {
        ERR("can not create semaphore");
        munmap(vs, size);
        shm_unlink(shmname);
        return EINA_FALSE;
     }
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
   if (!ev->file_ready)
     return;
   _emotion_frame_new(ev->obj);
}

static void
_file_open(Emotion_Generic_Video *ev)
{
   INF("Opening file: %s", ev->filename);
   ev->drop = 0;

   if (!ev->ready || !ev->filename)
     return;
   _player_send_cmd(ev, EM_CMD_FILE_SET);
   _player_send_str(ev, ev->filename, EINA_TRUE);
}

static void
_player_file_set_done(Emotion_Generic_Video *ev)
{
   if (ev->file_changed)
     {
        _file_open(ev);
        ev->file_changed = EINA_FALSE;
        return;
     }

   if (!_create_shm_data(ev, ev->shmname))
     {
        ERR("could not create shared memory.");
        return;
     }
   _player_send_cmd(ev, EM_CMD_FILE_SET_DONE);
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

static Eina_Bool
_player_cmd_param_read(Emotion_Generic_Video *ev, void *param, size_t size)
{
   ssize_t done, todo, i;

   /* When a parameter must be read, we cannot make sure it will be entirely
    * available. Thus we store the bytes that could be read in a temp buffer,
    * and when more data is read we try to complete the buffer and finally use
    * the read value.
    */
   if (!ev->cmd.tmp)
     {
        ev->cmd.tmp = malloc(size);
        ev->cmd.i = 0;
        ev->cmd.total = size;
     }

   todo = ev->cmd.total - ev->cmd.i;
   i = ev->cmd.i;

   done = (ev->offset + todo > ev->length) ? ev->length - ev->offset : todo;
   memcpy(&ev->cmd.tmp[i], &ev->buffer[ev->offset], done);
   ev->offset += done;

   if (done == todo)
     {
        memcpy(param, ev->cmd.tmp, size);
        free(ev->cmd.tmp);
        ev->cmd.tmp = NULL;
        return EINA_TRUE;
     }

   if (done > 0)
     ev->cmd.i += done;

   return EINA_FALSE;
}

static void
_player_frame_resize(Emotion_Generic_Video *ev)
{
   int w, h;

   w = ev->cmd.param.size.width;
   h = ev->cmd.param.size.height;

   INF("received frame resize: %dx%d", w, h);
   ev->w = w;
   ev->h = h;
   ev->ratio = (float)w / h;

   _emotion_frame_resize(ev->obj, ev->w, ev->h, ev->ratio);
}

static void
_player_length_changed(Emotion_Generic_Video *ev)
{
   float length = ev->cmd.param.f_num;

   INF("received length changed: %0.3f", length);

   ev->len = length;
   _emotion_video_pos_update(ev->obj, ev->pos, ev->len);
}

static void
_player_position_changed(Emotion_Generic_Video *ev)
{
   float position = ev->cmd.param.f_num;

   // INF("received position changed: %0.3f", position);

   ev->pos = position;
   _emotion_video_pos_update(ev->obj, ev->pos, ev->len);
/* hmmm. no _emotion_progress_set() is for "buffering" progress.
   if (ev->len == 0)
     return;

   float progress = ev->pos / ev->len;
   char buf[16];
   snprintf(buf, sizeof(buf), "%0.1f%%", progress * 100);

   _emotion_progress_set(ev->obj, buf, progress);
 */
}

static void
_player_seekable_changed(Emotion_Generic_Video *ev)
{
   int seekable = ev->cmd.param.i_num;

   INF("received seekable changed: %d", seekable);

   seekable = !!seekable;

   ev->seekable = seekable;
}

static void
_audio_channels_free(Emotion_Generic_Video *ev)
{
   int i;
   for (i = 0; i < ev->audio_channels_count; i++)
     eina_stringshare_del(ev->audio_channels[i].name);
   free(ev->audio_channels);
   ev->audio_channels = NULL;
   ev->audio_channels_count = 0;
}

static void
_video_channels_free(Emotion_Generic_Video *ev)
{
   int i;
   for (i = 0; i < ev->video_channels_count; i++)
     eina_stringshare_del(ev->video_channels[i].name);
   free(ev->video_channels);
   ev->video_channels = NULL;
   ev->video_channels_count = 0;
}

static void
_spu_channels_free(Emotion_Generic_Video *ev)
{
   int i;
   for (i = 0; i < ev->spu_channels_count; i++)
     eina_stringshare_del(ev->spu_channels[i].name);
   free(ev->spu_channels);
   ev->spu_channels = NULL;
   ev->spu_channels_count = 0;
}

static void
_player_tracks_info(Emotion_Generic_Video *ev, Emotion_Generic_Channel **channels, int *count, int *current)
{
   Emotion_Generic_Channel *pchannels;
   int i;

   *count = ev->cmd.param.track.total;
   *current = ev->cmd.param.track.current;
   pchannels = ev->cmd.param.track.channels;

   INF("number of tracks: %d (current = %d):", *count, *current);
   for (i = 0; i < *count; i++)
     {
        INF("\tchannel %d: %s", pchannels[i].id, pchannels[i].name);
     }

   *channels = pchannels;
}

static void
_player_audio_tracks_info(Emotion_Generic_Video *ev)
{
   INF("Receiving audio channels:");
   if (ev->audio_channels_count)
     _audio_channels_free(ev);

   _player_tracks_info(ev, &ev->audio_channels, &ev->audio_channels_count,
                       &ev->audio_channel_current);
}

static void
_player_video_tracks_info(Emotion_Generic_Video *ev)
{
   INF("Receiving video channels:");
   if (ev->video_channels_count)
     _video_channels_free(ev);

   _player_tracks_info(ev, &ev->video_channels, &ev->video_channels_count,
                       &ev->video_channel_current);
}

static void
_player_spu_tracks_info(Emotion_Generic_Video *ev)
{
   INF("Receiving spu channels:");
   if (ev->spu_channels_count)
     _spu_channels_free(ev);

   _player_tracks_info(ev, &ev->spu_channels, &ev->spu_channels_count,
                       &ev->spu_channel_current);
}

static void
_player_meta_info_free(Emotion_Generic_Video *ev)
{
   eina_stringshare_replace(&ev->meta.title, NULL);
   eina_stringshare_replace(&ev->meta.artist, NULL);
   eina_stringshare_replace(&ev->meta.album, NULL);
   eina_stringshare_replace(&ev->meta.year, NULL);
   eina_stringshare_replace(&ev->meta.genre, NULL);
   eina_stringshare_replace(&ev->meta.comment, NULL);
   eina_stringshare_replace(&ev->meta.disc_id, NULL);
   eina_stringshare_replace(&ev->meta.count, NULL);
}

static void
_player_meta_info_read(Emotion_Generic_Video *ev)
{
   INF("Receiving meta info:");
   _player_meta_info_free(ev);
   ev->meta.title = ev->cmd.param.meta.title;
   ev->meta.artist = ev->cmd.param.meta.artist;
   ev->meta.album = ev->cmd.param.meta.album;
   ev->meta.year = ev->cmd.param.meta.year;
   ev->meta.genre = ev->cmd.param.meta.genre;
   ev->meta.comment = ev->cmd.param.meta.comment;
   ev->meta.disc_id = ev->cmd.param.meta.disc_id;
   ev->meta.count = ev->cmd.param.meta.count;
   INF("title: '%s'", ev->meta.title);
   INF("artist: '%s'", ev->meta.artist);
   INF("album: '%s'", ev->meta.album);
   INF("year: '%s'", ev->meta.year);
   INF("genre: '%s'", ev->meta.genre);
   INF("comment: '%s'", ev->meta.comment);
   INF("disc_id: '%s'", ev->meta.disc_id);
   INF("count: '%s'", ev->meta.count);
}

static void
_player_file_closed(Emotion_Generic_Video *ev)
{
   INF("Closed previous file.");
   eina_semaphore_free(&ev->shared->lock);
   ev->closing = EINA_FALSE;

   if (ev->opening)
     _file_open(ev);
}

static void
_player_open_done(Emotion_Generic_Video *ev)
{
   int success;

   success = ev->cmd.param.i_num;
   shm_unlink(ev->shmname);

   if (ev->file_changed)
     {
        _file_open(ev);
        ev->file_changed = EINA_FALSE;
        return;
     }

   ev->opening = EINA_FALSE;
   if (!success)
     {
        ERR("Could not open file.");
        return;
     }

   ev->file_ready = EINA_TRUE;

   _emotion_open_done(ev->obj);

   _player_send_cmd(ev, EM_CMD_VOLUME_SET);
   _player_send_float(ev, ev->volume);

   _player_send_cmd(ev, EM_CMD_SPEED_SET);
   _player_send_float(ev, ev->speed);

   int mute = ev->audio_mute;
   _player_send_cmd(ev, EM_CMD_AUDIO_MUTE_SET);
   _player_send_int(ev, mute);

   mute = ev->video_mute;
   _player_send_cmd(ev, EM_CMD_VIDEO_MUTE_SET);
   _player_send_int(ev, mute);

   mute = ev->spu_mute;
   _player_send_cmd(ev, EM_CMD_SPU_MUTE_SET);
   _player_send_int(ev, mute);

   if (ev->play)
     {
        _player_send_cmd(ev, EM_CMD_PLAY);
        _player_send_float(ev, ev->pos);
     }

   INF("Open done");
}

static void
_player_cmd_process(Emotion_Generic_Video *ev)
{
   switch (ev->cmd.type) {
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
      case EM_RESULT_PLAYBACK_STARTED:
         _emotion_playback_started(ev->obj);
         break;
      case EM_RESULT_PLAYBACK_STOPPED:
         ev->play = 0;
         _emotion_playback_finished(ev->obj);
         _emotion_decode_stop(ev->obj);
         break;
      case EM_RESULT_FRAME_SIZE:
         _player_frame_resize(ev);
         break;
      case EM_RESULT_LENGTH_CHANGED:
         _player_length_changed(ev);
         break;
      case EM_RESULT_POSITION_CHANGED:
         _player_position_changed(ev);
         break;
      case EM_RESULT_SEEKABLE_CHANGED:
         _player_seekable_changed(ev);
         break;
      case EM_RESULT_AUDIO_TRACK_INFO:
         _player_audio_tracks_info(ev);
         break;
      case EM_RESULT_VIDEO_TRACK_INFO:
         _player_video_tracks_info(ev);
         break;
      case EM_RESULT_SPU_TRACK_INFO:
         _player_spu_tracks_info(ev);
         break;
      case EM_RESULT_META_INFO:
         _player_meta_info_read(ev);
         break;
      default:
         WRN("received wrong command: %d", ev->cmd.type);
   }

   ev->cmd.type = -1;
}

static void
_player_cmd_single_int_process(Emotion_Generic_Video *ev)
{
   if (!_player_cmd_param_read(ev, &ev->cmd.param.i_num, sizeof(ev->cmd.param.i_num)))
     return;

   _player_cmd_process(ev);
}

static void
_player_cmd_single_float_process(Emotion_Generic_Video *ev)
{
   if (!_player_cmd_param_read(ev, &ev->cmd.param.f_num, sizeof(ev->cmd.param.f_num)))
     return;

   _player_cmd_process(ev);
}

static void
_player_cmd_double_int_process(Emotion_Generic_Video *ev)
{
   int param;

   if (ev->cmd.num_params == 0)
     {
        ev->cmd.num_params = 2;
        ev->cmd.cur_param = 0;
        ev->cmd.param.size.width = 0;
        ev->cmd.param.size.height = 0;
     }

   if (!_player_cmd_param_read(ev, &param, sizeof(param)))
     return;

   if (ev->cmd.cur_param == 0)
     ev->cmd.param.size.width = param;
   else
     ev->cmd.param.size.height = param;

   ev->cmd.cur_param++;
   if (ev->cmd.cur_param == ev->cmd.num_params)
     _player_cmd_process(ev);
}

static void
_player_cmd_track_info(Emotion_Generic_Video *ev)
{
   int param;
   int i;

   if (ev->cmd.num_params == 0)
     {
        ev->cmd.cur_param = 0;
        ev->cmd.num_params = 2;
        ev->cmd.param.track.channels = NULL;
        ev->cmd.s_len = -1;
     }

   while (ev->cmd.cur_param < 2)
     {
        if (!_player_cmd_param_read(ev, &param, sizeof(param)))
          return;

        if (ev->cmd.cur_param == 0)
          ev->cmd.param.track.current = param;
        else
          {
             ev->cmd.param.track.total = param;
             ev->cmd.num_params += param * 2;
             ev->cmd.param.track.channels =
             calloc(param, sizeof(*ev->cmd.param.track.channels));
          }
        ev->cmd.cur_param++;
     }

   if (ev->cmd.cur_param == ev->cmd.num_params)
     {
        _player_cmd_process(ev);
        return;
     }

   i = (ev->cmd.cur_param - 2) / 2;
   if ((ev->cmd.cur_param % 2) == 0) // reading track id
     {
        if (!_player_cmd_param_read(ev, &param, sizeof(param)))
          return;
        ev->cmd.param.track.channels[i].id = param;
        ev->cmd.cur_param++;
     }
   else // reading track name
     {
        char buf[PATH_MAX];

        if (ev->cmd.s_len == -1)
          {
             if (!_player_cmd_param_read(ev, &param, sizeof(param)))
               return;
             ev->cmd.s_len = param;
          }

        if (!_player_cmd_param_read(ev, buf, ev->cmd.s_len))
          return;
        ev->cmd.param.track.channels[i].name = 
        eina_stringshare_add_length(buf, ev->cmd.s_len);
        ev->cmd.cur_param++;
        ev->cmd.s_len = -1;
     }

   if (ev->cmd.cur_param == ev->cmd.num_params)
     _player_cmd_process(ev);
}

static void
_player_cmd_meta_info(Emotion_Generic_Video *ev)
{
   int param;
   const char *info;
   char buf[PATH_MAX];

   if (ev->cmd.num_params == 0)
     {
        ev->cmd.cur_param = 0;
        ev->cmd.num_params = 8;
        ev->cmd.param.meta.title = NULL;
        ev->cmd.param.meta.artist = NULL;
        ev->cmd.param.meta.album = NULL;
        ev->cmd.param.meta.year = NULL;
        ev->cmd.param.meta.genre = NULL;
        ev->cmd.param.meta.comment = NULL;
        ev->cmd.param.meta.disc_id = NULL;
        ev->cmd.param.meta.count = NULL;
        ev->cmd.s_len = -1;
     }

   if (ev->cmd.s_len == -1)
     {
        if (!_player_cmd_param_read(ev, &param, sizeof(param)))
          return;
        ev->cmd.s_len = param;
     }

   if (!_player_cmd_param_read(ev, buf, ev->cmd.s_len))
     return;

   info = eina_stringshare_add_length(buf, ev->cmd.s_len);
   ev->cmd.s_len = -1;

   if (ev->cmd.cur_param == 0)
     ev->cmd.param.meta.title = info;
   else if (ev->cmd.cur_param == 1)
     ev->cmd.param.meta.artist = info;
   else if (ev->cmd.cur_param == 2)
     ev->cmd.param.meta.album = info;
   else if (ev->cmd.cur_param == 3)
     ev->cmd.param.meta.year = info;
   else if (ev->cmd.cur_param == 4)
     ev->cmd.param.meta.genre = info;
   else if (ev->cmd.cur_param == 5)
     ev->cmd.param.meta.comment = info;
   else if (ev->cmd.cur_param == 6)
     ev->cmd.param.meta.disc_id = info;
   else if (ev->cmd.cur_param == 7)
     ev->cmd.param.meta.count = info;

   ev->cmd.cur_param++;

   if (ev->cmd.cur_param == 8)
     _player_cmd_process(ev);
}

static void
_player_cmd_read(Emotion_Generic_Video *ev)
{
   if (ev->cmd.type < 0)
     {
        if (!_player_cmd_param_read(ev, &ev->cmd.type, sizeof(ev->cmd.type)))
          return;
        ev->cmd.num_params = 0;
     }

   switch (ev->cmd.type) {
      case EM_RESULT_INIT:
      case EM_RESULT_FILE_SET:
      case EM_RESULT_PLAYBACK_STARTED:
      case EM_RESULT_PLAYBACK_STOPPED:
      case EM_RESULT_FILE_CLOSE:
      case EM_RESULT_FRAME_NEW:
         _player_cmd_process(ev);
         break;
      case EM_RESULT_FILE_SET_DONE:
      case EM_RESULT_SEEKABLE_CHANGED:
         _player_cmd_single_int_process(ev);
         break;
      case EM_RESULT_LENGTH_CHANGED:
      case EM_RESULT_POSITION_CHANGED:
         _player_cmd_single_float_process(ev);
         break;
      case EM_RESULT_FRAME_SIZE:
         _player_cmd_double_int_process(ev);
         break;
      case EM_RESULT_AUDIO_TRACK_INFO:
      case EM_RESULT_VIDEO_TRACK_INFO:
      case EM_RESULT_SPU_TRACK_INFO:
         _player_cmd_track_info(ev);
         break;
      case EM_RESULT_META_INFO:
         _player_cmd_meta_info(ev);
         break;

      default:
         WRN("received wrong command: %d", ev->cmd.type);
         ev->cmd.type = -1;
   }
}

static void
_player_cmd_handler_cb(void *data, void *buffer, unsigned int nbyte)
{
   Emotion_Generic_Video *ev = data;

   ev->buffer = buffer;
   ev->length = nbyte;
   ev->offset = 0;

   _player_cmd_read(ev);

   ev->buffer = NULL;
   ev->length = 0;
}

static Eina_Bool
_player_data_cb(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Exe_Event_Data *ev = event;
   Emotion_Generic_Video *evideo = data;
   int i;

   if (ev->exe != evideo->player.exe)
     {
       INF("slave != ev->exe");
       return ECORE_CALLBACK_PASS_ON;
     }

   for (i = 0; ev->lines[i].line; i++)
     INF("received input from player: \"%s\"", ev->lines[i].line);

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_player_add_cb(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Exe_Event_Add *event_add = event;
   Ecore_Exe *player = event_add->exe;
   Emotion_Generic_Video *ev = data;

   if (ev->player.exe != player)
     {
        INF("ev->player != player.");
        return ECORE_CALLBACK_PASS_ON;
     }

   _player_send_cmd(ev, EM_CMD_INIT);
   _player_send_str(ev, ev->shmname, EINA_TRUE);

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_player_del_cb(void *data, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   Ecore_Exe_Event_Del *event_del = event;
   Ecore_Exe *player = event_del->exe;
   Emotion_Generic_Video *ev = data;

   if (ev->player.exe != player)
     {
        INF("ev->player != player.");
        return ECORE_CALLBACK_PASS_ON;
     }

   ERR("player died.");

   ev->player.exe = NULL;
   ev->ready = EINA_FALSE;
   ev->file_ready = EINA_FALSE;
   ecore_pipe_del(ev->fd_read);
   ecore_pipe_del(ev->fd_write);
   ev->fd_read = NULL;
   ev->fd_write = NULL;
   _emotion_decode_stop(ev->obj);

   return ECORE_CALLBACK_DONE;
}

static void
_player_dummy(void *data EINA_UNUSED,
              void *buffer EINA_UNUSED,
              unsigned int nbyte EINA_UNUSED)
{
}

static Eina_Bool
_player_exec(Emotion_Generic_Video *ev)
{
   Ecore_Pipe *in;
   Ecore_Pipe *out;
   char buf[PATH_MAX];

   out = ecore_pipe_full_add(_player_dummy, NULL, -1, -1, EINA_TRUE, EINA_FALSE);
   if (!out)
     {
        ERR("could not create pipe for communication emotion -> player: %s", strerror(errno));
        return EINA_FALSE;
     }

   in = ecore_pipe_full_add(_player_cmd_handler_cb, ev, -1, -1, EINA_FALSE, EINA_TRUE);
   if (!in)
     {
        ERR("could not create pipe for communication player -> emotion: %s", strerror(errno));
        ecore_pipe_del(in);
        ecore_pipe_del(out);
        return EINA_FALSE;
     }

   snprintf(buf, sizeof(buf), "%s %d %d", ev->engine->path,
            ecore_pipe_read_fd(out),
            ecore_pipe_write_fd(in));

   ev->player.exe = ecore_exe_pipe_run(
      buf, ECORE_EXE_NOT_LEADER | ECORE_EXE_TERM_WITH_PARENT, ev);

   INF("created pipe emotion -> player: %d -> %d",
       ecore_pipe_write_fd(out), ecore_pipe_read_fd(out));
   INF("created pipe player -> emotion: %d -> %d",
       ecore_pipe_write_fd(in), ecore_pipe_read_fd(in));

   ecore_pipe_write_close(in);
   ecore_pipe_read_close(out);

   if (!ev->player.exe)
     {
        ecore_pipe_del(in);
        ecore_pipe_del(out);
        return EINA_FALSE;
     }

   ev->fd_read = in;
   ev->fd_write = out;

   return EINA_TRUE;
}

static Eina_Bool
_fork_and_exec(Emotion_Generic_Video *ev)
{
   char shmname[256];

   snprintf(shmname, sizeof(shmname), "/em-generic-shm_%d_%p_%f",
            getpid(), ev->obj, ecore_time_get());

   ev->shmname = eina_stringshare_add(shmname);

   ev->player_add = ecore_event_handler_add(ECORE_EXE_EVENT_ADD,
                                            _player_add_cb, ev);
   ev->player_del = ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
                                            _player_del_cb, ev);
   ev->player_data = ecore_event_handler_add(ECORE_EXE_EVENT_DATA,
                                             _player_data_cb, ev);

   if (!_player_exec(ev))
     {
        ERR("could not start player.");
        return EINA_FALSE;
     }

   ev->initializing = EINA_TRUE;

   return EINA_TRUE;
}

typedef struct _Delay_Munmap Delay_Munmap;
struct _Delay_Munmap
{
   void *map;
   size_t size;
};

static void
_delayed_munmap(void *data, Evas *e, void *event_info EINA_UNUSED)
{
   Delay_Munmap *dm = data;

   evas_event_callback_del_full(e, EVAS_CALLBACK_RENDER_POST, _delayed_munmap, data);
   fprintf(stderr, "munmapping !\n");
   munmap(dm->map, dm->size);
   free(dm);
}

static void
_delayed_next_frame(void *data, Evas *e, void *event_info EINA_UNUSED)
{
   evas_event_callback_add(e, EVAS_CALLBACK_RENDER_POST, _delayed_munmap, data);
   evas_event_callback_del_full(e, EVAS_CALLBACK_RENDER_PRE, _delayed_next_frame, data);
}

static void
em_partial_shutdown(Emotion_Generic_Video *ev)
{
   if (ev->player.exe)
     {
        ecore_exe_terminate(ev->player.exe);
        ecore_exe_free(ev->player.exe);
        ev->player.exe = NULL;
     }

   ev->file_ready = EINA_FALSE;

   if (ev->shared)
     {
        Evas_Object *o;
        Delay_Munmap *dm;

        dm = malloc(sizeof (Delay_Munmap));
        if (dm)
          {
             dm->map = ev->shared;
             dm->size = ev->shared->size;
             evas_event_callback_add(evas_object_evas_get(ev->obj),
                                     EVAS_CALLBACK_RENDER_PRE,
                                     _delayed_next_frame, dm);
          }

        o = emotion_object_image_get(ev->obj);
        evas_object_image_data_set(o, NULL);
        evas_object_image_size_set(o, 1, 1);     
     }
   ev->shared = NULL;

   _emotion_image_reset(ev->obj);

   if (ev->fd_read)
     ecore_pipe_del(ev->fd_read);
   ev->fd_read = NULL;
   if (ev->fd_write)
     ecore_pipe_del(ev->fd_write);
   ev->fd_write = NULL;

   if (ev->player_add) ecore_event_handler_del(ev->player_add);
   ev->player_add = NULL;
   if (ev->player_data) ecore_event_handler_del(ev->player_data);
   ev->player_data = NULL;
   if (ev->player_del) ecore_event_handler_del(ev->player_del);
   ev->player_del = NULL;
}


/* Emotion interface */
static void *
em_add(const Emotion_Engine *api, Evas_Object *obj, const Emotion_Module_Options *opt EINA_UNUSED)
{
   Emotion_Generic_Video *ev;

   ev = calloc(1, sizeof(*ev));
   EINA_SAFETY_ON_NULL_RETURN_VAL(ev, NULL);

   ev->fd_read = NULL;
   ev->fd_write = NULL;
   ev->speed = 1.0;
   ev->volume = 1.0;
   ev->audio_mute = EINA_FALSE;
   ev->cmd.type = -1;

   ev->obj = obj;
   ev->engine = (Emotion_Engine_Generic *)api;

   if (!_fork_and_exec(ev))
     {
        free(ev);
        return NULL;
     }

   return ev;
}

static void
em_del(void *data)
{
   Emotion_Generic_Video *ev = data;

   eina_stringshare_del(ev->shmname);

   em_partial_shutdown(ev);
}

static unsigned char
em_file_open(void *data, const char *file)
{
   Emotion_Generic_Video *ev = data;
   INF("file set: %s", file);
   if (!ev) return 0;

   eina_stringshare_replace(&ev->filename, file);

   ev->pos = 0;
   ev->w = 0;
   ev->h = 0;
   ev->ratio = 1;
   ev->len = 0;

   if (ev->ready && ev->opening)
     {
        INF("file changed while opening.");
        ev->file_changed = EINA_TRUE;
        return 1;
     }

   ev->opening = EINA_TRUE;

   if (!ev->closing)
     _file_open(ev);

   return 1;
}

static void
em_file_close(void *data)
{
   Emotion_Generic_Video *ev = data;

   if (!ev || !ev->filename) return;

   INF("file close: %s", ev->filename);

   eina_stringshare_replace(&ev->filename, NULL);
   eina_stringshare_replace(&ev->subtitle_path, NULL);

   ev->file_ready = EINA_FALSE;
   _audio_channels_free(ev);
   _video_channels_free(ev);
   _spu_channels_free(ev);
   _player_meta_info_free(ev);

   if (ev->opening)
     return;

   _player_send_cmd(ev, EM_CMD_FILE_CLOSE);
   ev->closing = EINA_TRUE;
}

static Emotion_Format
em_format_get(void *ef EINA_UNUSED)
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
        if (ev->subtitle_path)
          {
             _player_send_cmd(ev, EM_CMD_SUBTITLE_SET);
             _player_send_str(ev, ev->subtitle_path, EINA_TRUE);
          }

        _player_send_cmd(ev, EM_CMD_PLAY);
        _player_send_float(ev, ev->pos);

        return;
     }

   if (!_player_exec(ev))
     ERR("could not start player.");
}

static void
em_stop(void *data)
{
   Emotion_Generic_Video *ev = data;

   if (!ev)
     return;

   ev->play = EINA_FALSE;

   if (!ev->file_ready)
     return;

   _player_send_cmd(ev, EM_CMD_STOP);
   _emotion_decode_stop(ev->obj);
}

static void
em_size_get(void *data, int *w, int *h)
{
   Emotion_Generic_Video *ev = data;
   if (w) *w = ev->w;
   if (h) *h = ev->h;
}

static void
em_pos_set(void *data, double pos)
{
   Emotion_Generic_Video *ev = data;
   float position = pos;

   if (!ev->file_ready)
     return;

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

static double
em_buffer_size_get(void *data EINA_UNUSED)
{
   return 1.0;
}

static int
em_fps_num_get(void *data)
{
   Emotion_Generic_Video *ev = data;
   return (int)(ev->fps * 1000.0);
}

static int
em_fps_den_get(void *ef EINA_UNUSED)
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
em_vis_set(void *ef EINA_UNUSED, Emotion_Vis vis EINA_UNUSED)
{
}

static Emotion_Vis
em_vis_get(void *data)
{
   Emotion_Generic_Video *ev = data;
   return ev->vis;
}

static Eina_Bool
em_vis_supported(void *ef EINA_UNUSED, Emotion_Vis vis EINA_UNUSED)
{
   return EINA_FALSE;
}

static double
em_ratio_get(void *data)
{
   Emotion_Generic_Video *ev = data;
   return ev->ratio;
}

static int
em_video_handled(void *ef EINA_UNUSED)
{
   DBG("video handled!");
   return 1;
}

static int
em_audio_handled(void *ef EINA_UNUSED)
{
   DBG("audio handled!");
   return 1;
}

static int
em_seekable(void *data)
{
   Emotion_Generic_Video *ev = data;
   return ev->seekable;
}

static void
em_frame_done(void *ef EINA_UNUSED)
{
}

static int
em_yuv_rows_get(void *data EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED, unsigned char **yrows EINA_UNUSED, unsigned char **urows EINA_UNUSED, unsigned char **vrows EINA_UNUSED)
{
   return 0;
}

static int
em_bgra_data_get(void *data, unsigned char **bgra_data)
{
   Emotion_Generic_Video *ev = data;

   if (!ev || !ev->file_ready)
     return 0;

   // lock frame here
   if (!eina_semaphore_lock(&ev->shared->lock))
     return 0;

   // send current frame to emotion
   if (ev->shared->frame.emotion != ev->shared->frame.last)
     {
        ev->shared->frame.next = ev->shared->frame.emotion;
        ev->shared->frame.emotion = ev->shared->frame.last;
     }
   *bgra_data = ev->frame.frames[ev->shared->frame.emotion];

   if (ev->shared->frame_drop > 1)
     WRN("dropped frames: %d", ev->shared->frame_drop - 1);
   ev->shared->frame_drop = 0;

   // unlock frame here
   eina_semaphore_release(&ev->shared->lock, 1);
   ev->drop = 0;

   return 1;
}

static void
em_event_feed(void *ef EINA_UNUSED, int event EINA_UNUSED)
{
}

static void
em_event_mouse_button_feed(void *ef EINA_UNUSED, int button EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED)
{
}

static void
em_event_mouse_move_feed(void *ef EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED)
{
}

static int
em_video_channel_count(void *data)
{
   Emotion_Generic_Video *ev = data;
   return ev->video_channels_count;
}

static void
em_video_channel_set(void *data, int channel)
{
   Emotion_Generic_Video *ev = data;

   if (channel < 0 || channel >= ev->video_channels_count)
     {
        WRN("video channel out of range.");
        return;
     }

   _player_send_cmd(ev, EM_CMD_VIDEO_TRACK_SET);
   _player_send_int(ev, ev->video_channels[channel].id);
   ev->video_channel_current = channel;
}

static int
em_video_channel_get(void *data)
{
   Emotion_Generic_Video *ev = data;
   return ev->video_channel_current;
}

static void
em_video_subtitle_file_set(void *data, const char *filepath)
{
   Emotion_Generic_Video *ev = data;
   eina_stringshare_replace(&ev->subtitle_path, filepath);
}

static const char *
em_video_subtitle_file_get(void *data)
{
   Emotion_Generic_Video *ev = data;
   return ev->subtitle_path;
}

static const char *
em_video_channel_name_get(void *data, int channel)
{
   Emotion_Generic_Video *ev = data;

   if (channel < 0 || channel >= ev->video_channels_count)
     {
        WRN("video channel out of range.");
        return NULL;
     }

   return ev->video_channels[channel].name;
}

static void
em_video_channel_mute_set(void *data, int mute)
{
   Emotion_Generic_Video *ev = data;

   ev->video_mute = !!mute;

   if (!ev || !ev->file_ready)
     return;

   _player_send_cmd(ev, EM_CMD_VIDEO_MUTE_SET);
   _player_send_int(ev, mute);
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

   if (channel < 0 || channel >= ev->audio_channels_count)
     {
        WRN("audio channel out of range.");
        return;
     }

   _player_send_cmd(ev, EM_CMD_AUDIO_TRACK_SET);
   _player_send_int(ev, ev->audio_channels[channel].id);
   ev->audio_channel_current = channel;
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

   if (channel < 0 || channel >= ev->audio_channels_count)
     {
        WRN("audio channel out of range.");
        return NULL;
     }

   return ev->audio_channels[channel].name;
}

static void
em_audio_channel_mute_set(void *data, int mute)
{
   Emotion_Generic_Video *ev = data;

   ev->audio_mute = !!mute;

   if (!ev || !ev->file_ready)
     return;

   _player_send_cmd(ev, EM_CMD_AUDIO_MUTE_SET);
   _player_send_int(ev, mute);
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

   if (vol > 1.0) vol = 1.0;
   if (vol < 0.0) vol = 0.0;

   ev->volume = vol;

   if (!ev || !ev->file_ready)
     return;

   _player_send_cmd(ev, EM_CMD_VOLUME_SET);
   _player_send_float(ev, ev->volume);
}

static double
em_audio_channel_volume_get(void *data)
{
   Emotion_Generic_Video *ev = data;
   return ev->volume;
}

static int
em_spu_channel_count(void *data)
{
   Emotion_Generic_Video *ev = data;
   return ev->spu_channels_count;
}

static void
em_spu_channel_set(void *data, int channel)
{
   Emotion_Generic_Video *ev = data;

   if (channel < 0 || channel >= ev->spu_channels_count)
     {
        WRN("spu channel out of range.");
        return;
     }

   _player_send_cmd(ev, EM_CMD_SPU_TRACK_SET);
   _player_send_int(ev, ev->spu_channels[channel].id);
   ev->spu_channel_current = channel;
}

static int
em_spu_channel_get(void *data)
{
   Emotion_Generic_Video *ev = data;
   return ev->spu_channel_current;
}

static const char *
em_spu_channel_name_get(void *data, int channel)
{
   Emotion_Generic_Video *ev = data;

   if (channel < 0 || channel >= ev->spu_channels_count)
     {
        WRN("spu channel out of range.");
        return NULL;
     }

   return ev->spu_channels[channel].name;
}

static void
em_spu_channel_mute_set(void *data, int mute)
{
   Emotion_Generic_Video *ev = data;

   ev->spu_mute = !!mute;

   if (!ev || !ev->file_ready)
     return;

   _player_send_cmd(ev, EM_CMD_SPU_MUTE_SET);
   _player_send_int(ev, mute);
}

static int
em_spu_channel_mute_get(void *data)
{
   Emotion_Generic_Video *ev = data;
   return ev->spu_mute;
}

static int
em_chapter_count(void *ef EINA_UNUSED)
{
   int num = 0;
   return num;
}

static void
em_chapter_set(void *ef EINA_UNUSED, int chapter EINA_UNUSED)
{
}

static int
em_chapter_get(void *ef EINA_UNUSED)
{
   int num = 0;
   return num;
}

static const char *
em_chapter_name_get(void *ef EINA_UNUSED, int chapter EINA_UNUSED)
{
   return NULL;
}

static void
em_speed_set(void *data, double speed)
{
   Emotion_Generic_Video *ev = data;
   float rate = speed;
   ev->speed = rate;

   if (!ev || !ev->file_ready)
     return;

   _player_send_cmd(ev, EM_CMD_SPEED_SET);
   _player_send_float(ev, rate);
}

static double
em_speed_get(void *data)
{
   Emotion_Generic_Video *ev = data;
   return (double)ev->speed;
}

static int
em_eject(void *ef EINA_UNUSED)
{
   return 1;
}

static const char *
em_meta_get(void *data, int meta)
{
   Emotion_Generic_Video *ev = data;

   switch (meta)
   {
      case EMOTION_META_INFO_TRACK_TITLE:
         return ev->meta.title;
      case EMOTION_META_INFO_TRACK_ARTIST:
         return ev->meta.artist;
      case EMOTION_META_INFO_TRACK_ALBUM:
         return ev->meta.album;
      case EMOTION_META_INFO_TRACK_YEAR:
         return ev->meta.year;
      case EMOTION_META_INFO_TRACK_GENRE:
         return ev->meta.genre;
      case EMOTION_META_INFO_TRACK_COMMENT:
         return ev->meta.comment;
      case EMOTION_META_INFO_TRACK_DISC_ID:
         return ev->meta.disc_id;
      case EMOTION_META_INFO_TRACK_COUNT:
         return ev->meta.count;
   }

   return NULL;
}


/* Players/modules */
static const Emotion_Engine em_template_engine =
{
   EMOTION_ENGINE_API_VERSION,
   EMOTION_ENGINE_PRIORITY_DEFAULT,
   "generic",
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
   em_video_data_size_get, /* video_data_size_get */
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
_player_entry_add(const Eina_File_Direct_Info *info)
{
   Emotion_Engine_Generic *eg;
   const char *name;
   char *endptr;
   int priority;

   name = info->path + info->name_start;

   priority = strtol(name, &endptr, 10);
   if (endptr == name)
     priority = EMOTION_ENGINE_PRIORITY_DEFAULT;
   else
     {
        if ((*endptr == '-') || (*endptr == '_'))
          endptr++;
        name = endptr;
     }

   if (*name == '\0')
     {
        ERR("Invalid generic player: %s", info->path);
        return;
     }

   eg = malloc(sizeof(Emotion_Engine_Generic));
   EINA_SAFETY_ON_NULL_RETURN(eg);

   /* inherit template */
   memcpy(&(eg->engine), &em_template_engine, sizeof(em_template_engine));

   eg->path = strdup(info->path);
   EINA_SAFETY_ON_NULL_GOTO(eg->path, error_path);

   eg->engine.name = strdup(name);
   EINA_SAFETY_ON_NULL_GOTO(eg->engine.name, error_name);

   eg->engine.priority = priority;

   DBG("Add player name=%s, priority=%d, path=%s",
       eg->engine.name, eg->engine.priority, eg->path);
   _generic_players = eina_list_append(_generic_players, eg);

   return;

 error_name:
   free(eg->path);
 error_path:
   free(eg);
}

static void
_player_entry_free(Emotion_Engine_Generic *eg)
{
   free(eg->path);
   free((void *)eg->engine.name);
   free(eg);
}

static void
_players_all_from(const char *path)
{
   const Eina_File_Direct_Info *info;
   int count = 0;
   Eina_Iterator *itr = eina_file_direct_ls(path);
   if (!itr) goto end;
   EINA_ITERATOR_FOREACH(itr, info)
     {
        if (access(info->path, R_OK | X_OK) == 0)
          {
             _player_entry_add(info);
             count++;
          }
     }
   eina_iterator_free(itr);

 end:
   if (count == 0)
     DBG("No generic players at %s", path);
}

static void
_players_load(void)
{
   char buf[PATH_MAX];

   eina_str_join(buf, sizeof(buf), '/',
                 eina_prefix_lib_get(pfx),
                 "emotion/generic_players/" MODULE_ARCH);
   _players_all_from(buf);

   if (!_generic_players)
     {
        WRN("no generic players available");
     }
   else
     {
        const Eina_List *n;
        const Emotion_Engine_Generic *eg;
        INF("Found %d generic players", eina_list_count(_generic_players));
        EINA_LIST_FOREACH(_generic_players, n, eg)
          _emotion_module_register(&(eg->engine));
     }
}

Eina_Bool
generic_module_init(void)
{
   if (_emotion_init_count > 0)
     {
        _emotion_init_count++;
        return EINA_TRUE;
     }

   _emotion_generic_log_domain = eina_log_domain_register("emotion_generic",
                                                          EINA_COLOR_LIGHTCYAN);
   if (_emotion_generic_log_domain < 0)
     {
        EINA_LOG_CRIT("Could not register log domain 'emotion_generic'");
        return EINA_FALSE;
     }

   pfx = eina_prefix_new(NULL, emotion_init,
                         "EMOTION", "emotion", "checkme",
                         PACKAGE_BIN_DIR, PACKAGE_LIB_DIR,
                         PACKAGE_DATA_DIR, PACKAGE_DATA_DIR);
   if (!pfx)
     {
        CRI("Could not get prefix for emotion");
        eina_log_domain_unregister(_emotion_generic_log_domain);
        _emotion_generic_log_domain = -1;
        return EINA_FALSE;
     }

   _players_load();

   _emotion_init_count = 1;
   return EINA_TRUE;
}

void
generic_module_shutdown(void)
{
   Emotion_Engine_Generic *eg;

   if (_emotion_init_count > 1)
     {
        _emotion_init_count--;
        return;
     }
   else if (_emotion_init_count == 0)
     {
        EINA_LOG_ERR("too many generic_module_shutdown()");
        return;
     }
   _emotion_init_count = 0;

   EINA_LIST_FREE(_generic_players, eg)
     {
        _emotion_module_unregister(&(eg->engine));
        _player_entry_free(eg);
     }

   eina_log_domain_unregister(_emotion_generic_log_domain);
   _emotion_generic_log_domain = -1;

   eina_prefix_free(pfx);
   pfx = NULL;
}

#ifndef EMOTION_STATIC_BUILD_GENERIC

EINA_MODULE_INIT(generic_module_init);
EINA_MODULE_SHUTDOWN(generic_module_shutdown);

#endif
