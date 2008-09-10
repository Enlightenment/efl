#include "Emotion.h"
#include "emotion_private.h"
#include "emotion_xine.h"

/* module api */
static unsigned char  em_init                    (Evas_Object *obj, void **emotion_video, Emotion_Module_Options *opt);
static int            em_shutdown                (void *ef);
static unsigned char  em_file_open               (const char *file, Evas_Object *obj, void *ef);
static void           em_file_close              (void *ef);
static void           em_play                    (void *ef, double pos);
static void           em_stop                    (void *ef);
static void           em_size_get                (void *ef, int *w, int *h);
static void           em_pos_set                 (void *ef, double pos);
static void           em_vis_set                 (void *ef, Emotion_Vis vis);
static double         em_len_get                 (void *ef);
static int            em_fps_num_get             (void *ef);
static int            em_fps_den_get             (void *ef);
static double         em_fps_get                 (void *ef);
static double         em_pos_get                 (void *ef);
static Emotion_Vis    em_vis_get                 (void *ef);
static double         em_ratio_get               (void *ef);
static int            em_seekable                (void *ef);
static void           em_frame_done              (void *ef);
static Emotion_Format em_format_get              (void *ef);
static void           em_video_data_size_get     (void *ef, int *w, int *h);
static int            em_yuv_rows_get            (void *ef, int w, int h, unsigned char **yrows, unsigned char **urows, unsigned char **vrows);
static int            em_bgra_data_get           (void *ef, unsigned char **bgra_data);
static void           em_event_feed              (void *ef, int event);
static void           em_event_mouse_button_feed (void *ef, int button, int x, int y);
static void           em_event_mouse_move_feed   (void *ef, int x, int y);
static int            em_video_channel_count     (void *ef);
static void           em_video_channel_set       (void *ef, int channel);
static int            em_video_channel_get       (void *ef);
static const char    *em_video_channel_name_get  (void *ef, int channel);
static void           em_video_channel_mute_set  (void *ef, int mute);
static int            em_video_channel_mute_get  (void *ef);
static int            em_audio_channel_count     (void *ef);
static void           em_audio_channel_set       (void *ef, int channel);
static int            em_audio_channel_get       (void *ef);
static const char    *em_audio_channel_name_get  (void *ef, int channel);
static void           em_audio_channel_mute_set  (void *ef, int mute);
static int            em_audio_channel_mute_get  (void *ef);
static void           em_audio_channel_volume_set(void *ef, double vol);
static double         em_audio_channel_volume_get(void *ef);
static int            em_spu_channel_count       (void *ef);
static void           em_spu_channel_set         (void *ef, int channel);
static int            em_spu_channel_get         (void *ef);
static const char    *em_spu_channel_name_get    (void *ef, int channel);
static void           em_spu_channel_mute_set    (void *ef, int mute);
static int            em_spu_channel_mute_get    (void *ef);
static int            em_chapter_count           (void *ef);
static void           em_chapter_set             (void *ef, int chapter);
static int            em_chapter_get             (void *ef);
static const char    *em_chapter_name_get        (void *ef, int chapter);
static void           em_speed_set               (void *ef, double speed);
static double         em_speed_get               (void *ef);
static int            em_eject                   (void *ef);
static const char    *em_meta_get                (void *ef, int meta);

/* internal util calls */
static void *_em_slave         (void *par);
static void  _em_slave_event   (void *data, int type, void *arg);
static int   _em_fd_active     (void *data, Ecore_Fd_Handler *fdh);
static void  _em_event         (void *data, const xine_event_t *event);
static void  _em_module_event  (void *data, int type);
static int   _em_fd_ev_active  (void *data, Ecore_Fd_Handler *fdh);
//static int   _em_timer         (void *data);
static void *_em_get_pos_len_th(void *par);
static void  _em_get_pos_len   (Emotion_Xine_Video *ev);

extern plugin_info_t emotion_xine_plugin_info[];

/* this is a slave controller thread for the xine module - libxine loves
 * to deadlock, internally stall and otherwise have unpredictable behavior
 * if we use the main process thread for many things - so a lot will be
 * farmed off to this slave. its job is to handle opening, closing, file
 * opening, recoder init etc. and all sorts of things can that often block.
 * anything this thread needs to return, it will return via the event pipe.
 */
static void *
_em_slave(void *par)
{
   Emotion_Xine_Video *ev;
   void *buf[2];
   int len;
   
   ev = (Emotion_Xine_Video *)par;
   while ((len = read(ev->fd_slave_read, buf, sizeof(buf))) > 0)
     {
	if (len == sizeof(buf))
	  {
	     Emotion_Xine_Event *eev;

	     ev = buf[0];
	     eev = buf[1];
	     switch (eev->mtype)
	       {
		case 0: /* noop */
		  break;
		case 1: /* init */
		    {
		       ev->decoder = xine_new();
		       xine_init(ev->decoder);
		       xine_register_plugins(ev->decoder, emotion_xine_plugin_info);
		       if (1)
			 {
			    xine_cfg_entry_t cf;
			    if (xine_config_lookup_entry(ev->decoder, "input.dvd_use_readahead", &cf))
			      {
				 cf.num_value = 1; // 0 or 1
				 xine_config_update_entry(ev->decoder, &cf);
			      }
			 }
		       printf("OPEN VIDEO PLUGIN...\n");
		       if (!ev->opt_no_video)
			 ev->video = xine_open_video_driver(ev->decoder, "emotion",
							    XINE_VISUAL_TYPE_NONE, ev);
		       printf("RESULT: xine_open_video_driver() = %p\n", ev->video);
		       // Let xine autodetect the best audio output driver
		       if (!ev->opt_no_audio)
			 ev->audio = xine_open_audio_driver(ev->decoder, NULL, ev);
		       //   ev->audio = xine_open_audio_driver(ev->decoder, "oss", ev);
		       // dont use alsa - alsa has oss emulation.   
		       //   ev->audio = xine_open_audio_driver(ev->decoder, "alsa", ev);
		       //   ev->audio = xine_open_audio_driver(ev->decoder, "arts", ev);
		       //   ev->audio = xine_open_audio_driver(ev->decoder, "esd", ev);
		       ev->stream = xine_stream_new(ev->decoder, ev->audio, ev->video);
		       ev->queue = xine_event_new_queue(ev->stream);
		       xine_event_create_listener_thread(ev->queue, _em_event, ev);
		       ev->opening = 0;
		       ev->play_ok = 1;
		       _em_module_event(ev, 1); /* event - open done */
		    }
		  break;
		case 3: /* shutdown */
		    {
		       _em_module_event(ev, 3);
		       printf("EX shutdown stop\n");
		       xine_stop(ev->stream);
		       //   pthread_mutex_lock(&(ev->get_pos_len_mutex));
		       if (!ev->get_pos_thread_deleted)
			 {
			    printf("closing get_pos thread, %p\n", ev);
			    pthread_mutex_lock(&(ev->get_pos_len_mutex));
			    pthread_cond_broadcast(&(ev->get_pos_len_cond));
			    pthread_mutex_unlock(&(ev->get_pos_len_mutex));
			    while (ev->get_poslen);
			 }
		       printf("EX dispose %p\n", ev);
		       xine_dispose(ev->stream);
		       printf("EX dispose evq %p\n", ev);
		       xine_event_dispose_queue(ev->queue);
		       printf("EX close video drv %p\n", ev);
		       if (ev->video) xine_close_video_driver(ev->decoder, ev->video);
		       printf("EX wait for vo to go\n");
		       while (ev->have_vo);
		       printf("EX vo gone\n");
		       printf("EX close audio drv %p\n", ev);
		       if (ev->audio) xine_close_audio_driver(ev->decoder, ev->audio);
		       printf("EX xine exit %p\n", ev);
		       xine_exit(ev->decoder);
		       printf("EX DONE %p\n", ev);
		       close(ev->fd_write);
		       close(ev->fd_read);
		       close(ev->fd_ev_write);
		       close(ev->fd_ev_read);
		       close(ev->fd_slave_write);
		       close(ev->fd_slave_read);
   		       ev->closing = 0;
		       if (eev->xine_event) free(eev->xine_event);
		       free(eev);
		       free(ev);
		       return NULL;
		    }
		  break;
		case 2: /* file open */
		    {
		       int pos_stream = 0;
		       int pos_time = 0;
		       int length_time = 0;
		       uint32_t v;
		       char *file;
		       
		       file = eev->xine_event;
		       printf("OPN STREAM %s\n", file);
		       if (xine_open(ev->stream, file))
			 {
			    if (xine_get_pos_length(ev->stream, &pos_stream, &pos_time, &length_time))
			      {
				 if (length_time == 0)
				   {
				      ev->pos = (double)pos_stream / 65535;
				      ev->len = 1.0;
				      ev->no_time = 1;
				   }
				 else
				   {
				      ev->pos = 0.0;
				      ev->len = (double)length_time / 1000.0;
				   }
			      }
			    else
			      {
				 ev->pos = 0.0;
				 ev->len = 1.0;
			      }
			    v = xine_get_stream_info(ev->stream, XINE_STREAM_INFO_FRAME_DURATION);
			    if (v > 0) ev->fps = 90000.0 / (double)v;
			    v = xine_get_stream_info(ev->stream, XINE_STREAM_INFO_VIDEO_WIDTH);
			    ev->w = v;
			    v = xine_get_stream_info(ev->stream, XINE_STREAM_INFO_VIDEO_HEIGHT);
			    ev->h = v;
			    v = xine_get_stream_info(ev->stream, XINE_STREAM_INFO_VIDEO_RATIO);
			    ev->ratio = (double)v / 10000.0;
			    ev->just_loaded = 1;
			    ev->get_poslen = 0;
			 }
		       _em_module_event(ev, 2); /* event - open done */
		    }
		  break;
		case 11: /* file close */
		    {
		       printf("EX done %p\n", ev);
		       em_frame_done(ev); 
		       printf("EX stop %p\n", ev);
		       xine_stop(ev->stream);
		       printf("EX close %p\n", ev);
		       xine_close(ev->stream);
		       printf("EX close done %p\n", ev);
		       _em_module_event(ev, 11);
		    }
		  break;
		case 4: /* play */
		    {
		       double pos;
		       int pos_stream, pos_time, length_time;
		       
		       pos = *((double *)eev->xine_event);
		       if ((xine_get_param(ev->stream, XINE_PARAM_SPEED) == XINE_SPEED_PAUSE) &&
			   (pos == ev->pos) &&
			   (!ev->just_loaded))
			 {
			    xine_set_param(ev->stream, XINE_PARAM_SPEED, XINE_SPEED_NORMAL);
			 }
		       else
			 {
			    if (ev->no_time)
			      xine_play(ev->stream, pos * 65535, 0);
			    else
			      xine_play(ev->stream, 0, pos * 1000);
			 }
		       ev->just_loaded = 0;
		       
		       if (xine_get_pos_length(ev->stream,
					       &pos_stream,
					       &pos_time,
					       &length_time))
			 {
			    if (length_time == 0)
			      {
				 ev->pos = (double)pos_stream / 65535;
				 ev->len = 1.0;
				 ev->no_time = 1;
			      }
			    else
			      {
				 ev->pos = (double)pos_time / 1000.0;
				 ev->len = (double)length_time / 1000.0;
			      }
			 }
		       _em_module_event(ev, 4);
		    }
		  break;
		case 5: /* stop */
		    {
		       xine_set_param(ev->stream, XINE_PARAM_SPEED, XINE_SPEED_PAUSE);
		       _em_module_event(ev, 5);
		    }
		  break;
		case 6: /* seek */
		    {
		       double pos;
		       
		       pos = *((double *)eev->xine_event);
		       if (ev->seeked_pos != ev->seek_to_pos)
			 {
			    if (ev->no_time)
			      xine_play(ev->stream, pos * 65535, 0);
			    else
			      xine_play(ev->stream, 0, pos * 1000);
			    if (!ev->play)
			      xine_set_param(ev->stream, XINE_PARAM_SPEED, XINE_SPEED_PAUSE);
			    ev->seeked_pos = ev->seek_to_pos;
			 }
		       _em_module_event(ev, 6);
		    }
		  break;
		case 7: /* eject */
		    {
		       xine_eject(ev->stream);
		       _em_module_event(ev, 7);
		    }
		  break;
		case 8: /* spu mute */
		    {
		       xine_set_param(ev->stream, XINE_PARAM_IGNORE_SPU, ev->spu_mute);
		       _em_module_event(ev, 8);
		    }
		  break;
		case 9: /* channel */
		    {
		       xine_set_param(ev->stream, XINE_PARAM_SPU_CHANNEL, ev->spu_channel);
		       _em_module_event(ev, 9);
		    }
		  break;
		case 10: /* vol */
		    {
		       double vol;
		       
		       vol = *((double *)eev->xine_event);
		       if (vol < 0.0) vol = 0.0;
		       if (vol > 1.0) vol = 1.0;
		       xine_set_param(ev->stream, XINE_PARAM_AUDIO_VOLUME, vol * 100);
		       _em_module_event(ev, 10);
		    }
		  break;
		case 12: /* audio mute */
		    {
		       xine_set_param(ev->stream, XINE_PARAM_AUDIO_MUTE, ev->audio_mute);
		    }
		  break;
		case 13: /* audio mute */
		    {
		       xine_set_param(ev->stream, XINE_PARAM_AUDIO_CHANNEL_LOGICAL, ev->audio_channel);
		    }
		  break;
		case 14: /* audio mute */
		    {
		       xine_set_param(ev->stream, XINE_PARAM_VIDEO_CHANNEL, ev->video_channel);
		    }
		  break;
		default:
		  break;
	       }
	     if (eev->xine_event) free(eev->xine_event);
 	     free(eev);
	  }
     }
   return NULL;
}
static void
_em_slave_event(void *data, int type, void *arg)
{
   void *buf[2];
   Emotion_Xine_Event *new_ev;
   Emotion_Xine_Video *ev;
   
   ev = data;
   new_ev = calloc(1, sizeof(Emotion_Xine_Event));
   if (!new_ev) return;
   new_ev->mtype = type;
   new_ev->type = -1;
   new_ev->xine_event = arg;
   buf[0] = data;
   buf[1] = new_ev;
   write(ev->fd_slave_write, buf, sizeof(buf));
}

static unsigned char
em_init(Evas_Object *obj, void **emotion_video, Emotion_Module_Options *opt)
{
   Emotion_Xine_Video *ev;
   int fds[2];
   
   if (!emotion_video) return 0;
   
   ev = calloc(1, sizeof(Emotion_Xine_Video));
   if (!ev) return 0;
   ev->obj = obj;
   
   if (pipe(fds) == 0)
     {
	ev->fd_read = fds[0];
	ev->fd_write = fds[1];
	fcntl(ev->fd_read, F_SETFL, O_NONBLOCK);
	ev->fd_handler = ecore_main_fd_handler_add(ev->fd_read, ECORE_FD_READ,
						   _em_fd_active, ev, 
						   NULL, NULL);
	ecore_main_fd_handler_active_set(ev->fd_handler, ECORE_FD_READ);
     }
   if (pipe(fds) == 0)
     {
	ev->fd_ev_read = fds[0];
	ev->fd_ev_write = fds[1];
	fcntl(ev->fd_ev_read, F_SETFL, O_NONBLOCK);
	ev->fd_ev_handler = ecore_main_fd_handler_add(ev->fd_ev_read,
						      ECORE_FD_READ, _em_fd_ev_active, ev, NULL, NULL);
	ecore_main_fd_handler_active_set(ev->fd_ev_handler, ECORE_FD_READ);
     }
   if (pipe(fds) == 0)
     {
	ev->fd_slave_read = fds[0];
	ev->fd_slave_write = fds[1];
	fcntl(ev->fd_slave_write, F_SETFL, O_NONBLOCK);
     }
   ev->delete_me = 0;
   ev->get_pos_thread_deleted = 0;
   ev->opening = 1;
   ev->play_ok = 0;
   ev->seek_to_pos = -1;
   ev->seeked_pos = -1;
   
   if (opt)
     {
	ev->opt_no_audio = opt->no_audio;
	ev->opt_no_video = opt->no_video;
     }
   
   pthread_cond_init(&(ev->get_pos_len_cond), NULL);
   pthread_mutex_init(&(ev->get_pos_len_mutex), NULL);
   pthread_create(&ev->get_pos_len_th, NULL, _em_get_pos_len_th, ev);

   pthread_create(&ev->slave_th, NULL, _em_slave, ev);
   pthread_detach(ev->slave_th);
   _em_slave_event(ev, 1, NULL);
   
   *emotion_video = ev;
   return 1;
}

static int
em_shutdown(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   ev->closing = 1;
   ev->delete_me = 1;
   printf("EXM del fds %p\n", ev);
   ecore_main_fd_handler_del(ev->fd_handler);
   ecore_main_fd_handler_del(ev->fd_ev_handler);
   
   ev->closing = 1;
   _em_slave_event(ev, 3, NULL);
   printf("EXM done %p\n", ev);
   return 1;
}

static unsigned char
em_file_open(const char *file, Evas_Object *obj, void *ef)
{
   Emotion_Xine_Video *ev;

   ev = (Emotion_Xine_Video *)ef;
   if (!ev) return 0;
   _em_slave_event(ev, 2, strdup(file));
   return 1;
}

static void
em_file_close(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   if (!ev) return;
   _em_slave_event(ev, 11, NULL);
}

static void
em_play(void *ef, double pos)
{
   Emotion_Xine_Video *ev;
   double *ppos;
   
   ev = (Emotion_Xine_Video *)ef;
   ev->play = 1;
   ev->play_ok = 0;
   ppos = malloc(sizeof(double));
   *ppos = pos;
   _em_slave_event(ev, 4, ppos);
}

static void
em_stop(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   ev->play = 0;
   ev->play_ok = 0;
   _em_slave_event(ev, 5, NULL);
}

static void 
em_size_get(void *ef, int *w, int *h)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   if (w) *w = ev->w;
   if (h) *h = ev->h;
}

static void
em_pos_set(void *ef, double pos)
{
   Emotion_Xine_Video *ev;
   double *ppos;
   
   ev = (Emotion_Xine_Video *)ef;
   if (pos == ev->seek_to_pos) return;
   ppos = malloc(sizeof(double));
   *ppos = pos;
   ev->seek_to_pos = pos;
   _em_slave_event(ev, 6, ppos);
}

static void
em_vis_set(void       *ef,
	   Emotion_Vis vis)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   if (ev->vis == vis) return;
   ev->vis = vis;
}

static double
em_len_get(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   return ev->len;
}

static int
em_fps_num_get(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   return (int)(ev->fps * 10000.0);
}

static int
em_fps_den_get(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   return 10000;
}

static double
em_fps_get(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   return ev->fps;
}

static double
em_pos_get(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   return ev->pos;
}

static Emotion_Vis
em_vis_get(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;

   return ev->vis;
}

static double
em_ratio_get(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   return ev->ratio;
}

static int
em_video_handled(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   if (ev->opening || (!ev->play_ok)) return 0;
   return (xine_get_stream_info(ev->stream, XINE_STREAM_INFO_HAS_VIDEO) &&
      xine_get_stream_info(ev->stream, XINE_STREAM_INFO_VIDEO_HANDLED));
}

static int
em_audio_handled(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   if (ev->opening || (!ev->play_ok)) return 0;
   return (xine_get_stream_info(ev->stream, XINE_STREAM_INFO_HAS_AUDIO) &&
      xine_get_stream_info(ev->stream, XINE_STREAM_INFO_AUDIO_HANDLED));
}

static int
em_seekable(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   if (ev->opening || (!ev->play_ok)) return 0;
   return xine_get_stream_info(ev->stream, XINE_STREAM_INFO_SEEKABLE);
}

static void
em_frame_done(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   if (ev->cur_frame)
     {
	ev->fq--;
	if (ev->cur_frame->done_func)
	  ev->cur_frame->done_func(ev->cur_frame->done_data);
	ev->cur_frame = NULL;
     }
}

static Emotion_Format
em_format_get(void *ef)
{
   Emotion_Xine_Video *ev;
   volatile Emotion_Xine_Video_Frame *fr;
   
   ev = (Emotion_Xine_Video *)ef;
   fr = ev->cur_frame;
   if (fr) return fr->format;
   return EMOTION_FORMAT_YV12;
}

static void
em_video_data_size_get(void *ef, int *w, int *h)
{
   Emotion_Xine_Video *ev;
   volatile Emotion_Xine_Video_Frame *fr;
   
   ev = (Emotion_Xine_Video *)ef;
   fr = ev->cur_frame;
   if (!fr)
     {
	if (w) *w = 0;
	if (h) *h = 0;
	return;
     }
   if (w) *w = fr->w;
   if (h) *h = fr->h;
}

static int
em_yuv_rows_get(void *ef, int w, int h, unsigned char **yrows, unsigned char **urows, unsigned char **vrows)
{
   Emotion_Xine_Video *ev;
   volatile Emotion_Xine_Video_Frame *fr;
   
   ev = (Emotion_Xine_Video *)ef;
   fr = ev->cur_frame;
   if (!fr) return 0;
   if (fr->y)
     {
	int i;
	
	for (i = 0; i < h; i++) yrows[i] = fr->y + (i * fr->y_stride);
	for (i = 0; i < (h / 2); i++) urows[i] = fr->u + (i * fr->u_stride);
	for (i = 0; i < (h / 2); i++) vrows[i] = fr->v + (i * fr->v_stride);
	return 1;
     }
   return 0;
}

static int
em_bgra_data_get(void *ef, unsigned char **bgra_data)
{
   Emotion_Xine_Video *ev;
   volatile Emotion_Xine_Video_Frame *fr;
   
   ev = (Emotion_Xine_Video *)ef;
   fr = ev->cur_frame;
   if (!fr) return 0;
   if (fr->bgra_data)
     {
	*bgra_data = fr->bgra_data;
	return 1;
     }
   return 0;
}

static void
em_event_feed(void *ef, int event)
{
   Emotion_Xine_Video *ev;
   xine_event_t xine_event;

   ev = (Emotion_Xine_Video *)ef;
   if ((ev->opening) || (!ev->play_ok)) return;
   xine_event.data_length = 0;
   xine_event.data        = NULL;
   xine_event.stream      = ev->stream;
   gettimeofday(&xine_event.tv, NULL);
   switch (event)
     {
      case EMOTION_EVENT_MENU1:
	xine_event.type = XINE_EVENT_INPUT_MENU1;
	break;
      case EMOTION_EVENT_MENU2:
	xine_event.type = XINE_EVENT_INPUT_MENU2;
	break;
      case EMOTION_EVENT_MENU3:
	xine_event.type = XINE_EVENT_INPUT_MENU3;
	break;
      case EMOTION_EVENT_MENU4:
	xine_event.type = XINE_EVENT_INPUT_MENU4;
	break;
      case EMOTION_EVENT_MENU5:
	xine_event.type = XINE_EVENT_INPUT_MENU5;
	break;
      case EMOTION_EVENT_MENU6:
	xine_event.type = XINE_EVENT_INPUT_MENU6;
	break;
      case EMOTION_EVENT_MENU7:
	xine_event.type = XINE_EVENT_INPUT_MENU7;
	break;
      case EMOTION_EVENT_UP:
	xine_event.type = XINE_EVENT_INPUT_UP;
	break;
      case EMOTION_EVENT_DOWN:
	xine_event.type = XINE_EVENT_INPUT_DOWN;
	break;
      case EMOTION_EVENT_LEFT:
	xine_event.type = XINE_EVENT_INPUT_LEFT;
	break;
      case EMOTION_EVENT_RIGHT:
	xine_event.type = XINE_EVENT_INPUT_RIGHT;
	break;
      case EMOTION_EVENT_SELECT:
	xine_event.type = XINE_EVENT_INPUT_SELECT;
	break;
      case EMOTION_EVENT_NEXT:
	xine_event.type = XINE_EVENT_INPUT_NEXT;
	break;
      case EMOTION_EVENT_PREV:
	xine_event.type = XINE_EVENT_INPUT_PREVIOUS;
	break;
      case EMOTION_EVENT_ANGLE_NEXT:
	xine_event.type = XINE_EVENT_INPUT_ANGLE_NEXT;
	break;
      case EMOTION_EVENT_ANGLE_PREV:
	xine_event.type = XINE_EVENT_INPUT_ANGLE_PREVIOUS;
	break;
      case EMOTION_EVENT_FORCE:
	xine_event.type = XINE_EVENT_INPUT_BUTTON_FORCE;
	break;
      case EMOTION_EVENT_0:
	xine_event.type = XINE_EVENT_INPUT_NUMBER_0;
	break;
      case EMOTION_EVENT_1:
	xine_event.type = XINE_EVENT_INPUT_NUMBER_1;
	break;
      case EMOTION_EVENT_2:
	xine_event.type = XINE_EVENT_INPUT_NUMBER_2;
	break;
      case EMOTION_EVENT_3:
	xine_event.type = XINE_EVENT_INPUT_NUMBER_3;
	break;
      case EMOTION_EVENT_4:
	xine_event.type = XINE_EVENT_INPUT_NUMBER_4;
	break;
      case EMOTION_EVENT_5:
	xine_event.type = XINE_EVENT_INPUT_NUMBER_5;
	break;
      case EMOTION_EVENT_6:
	xine_event.type = XINE_EVENT_INPUT_NUMBER_6;
	break;
      case EMOTION_EVENT_7:
	xine_event.type = XINE_EVENT_INPUT_NUMBER_7;
	break;
      case EMOTION_EVENT_8:
	xine_event.type = XINE_EVENT_INPUT_NUMBER_8;
	break;
      case EMOTION_EVENT_9:
	xine_event.type = XINE_EVENT_INPUT_NUMBER_9;
	break;
      case EMOTION_EVENT_10:
	xine_event.type = XINE_EVENT_INPUT_NUMBER_10_ADD;
	break;
      default:
	return;
	break;
     }
   xine_event_send(ev->stream, &xine_event);
}

static void
em_event_mouse_button_feed(void *ef, int button, int x, int y)
{
   Emotion_Xine_Video *ev;
   xine_event_t xine_event;
   xine_input_data_t xine_input;

   ev = (Emotion_Xine_Video *)ef;
   if ((ev->opening) || (!ev->play_ok)) return;
   xine_event.stream      = ev->stream;
   gettimeofday(&xine_event.tv, NULL);
   xine_event.type = XINE_EVENT_INPUT_MOUSE_BUTTON;
   xine_input.button = 1;
   xine_input.x = x;
   xine_input.y = y;
   xine_event.data = &xine_input;
   xine_event.data_length = sizeof(xine_input);
   xine_event_send(ev->stream, &xine_event);
}

static void
em_event_mouse_move_feed(void *ef, int x, int y)
{
   Emotion_Xine_Video *ev;
   xine_event_t xine_event;
   xine_input_data_t xine_input;

   ev = (Emotion_Xine_Video *)ef;
   if ((ev->opening) || (!ev->play_ok)) return;
   xine_event.stream      = ev->stream;
   gettimeofday(&xine_event.tv, NULL);
   xine_event.type = XINE_EVENT_INPUT_MOUSE_MOVE;
   xine_input.button = 0;
   xine_input.x = x;
   xine_input.y = y;
   xine_event.data = &xine_input;
   xine_event.data_length = sizeof(xine_input);
   xine_event_send(ev->stream, &xine_event);
}

static int
em_video_channel_count(void *ef)
{
   Emotion_Xine_Video *ev;
   int v;
   
   ev = (Emotion_Xine_Video *)ef;
   if (ev->opening || (!ev->play_ok)) return 0;
   v = xine_get_stream_info(ev->stream, XINE_STREAM_INFO_VIDEO_CHANNELS);
   if ((v < 1) &&
       xine_get_stream_info(ev->stream, XINE_STREAM_INFO_HAS_VIDEO)) return 1;
   return v;
}

static void
em_video_channel_set(void *ef, int channel)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   if (channel < 0) channel = 0;
   ev->video_channel = channel;
   _em_slave_event(ev, 14, NULL);
}

static int
em_video_channel_get(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   if (ev->opening || (!ev->play_ok)) return 0;
   return xine_get_param(ev->stream, XINE_PARAM_VIDEO_CHANNEL);
}

static const char *
em_video_channel_name_get(void *ef, int channel)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   return NULL;
}

static void
em_video_channel_mute_set(void *ef, int mute)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   ev->video_mute = mute;
}

static int
em_video_channel_mute_get(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   return ev->video_mute;
}

static int
em_audio_channel_count(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   if (ev->opening || (!ev->play_ok)) return 0;
   return xine_get_stream_info(ev->stream, XINE_STREAM_INFO_MAX_AUDIO_CHANNEL);
}

static void
em_audio_channel_set(void *ef, int channel)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   if (channel < -1) channel = -1;
   ev->audio_channel = channel;
   _em_slave_event(ev, 13, NULL);
}

static int
em_audio_channel_get(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   if (ev->opening || (!ev->play_ok)) return 0;
   return xine_get_param(ev->stream, XINE_PARAM_AUDIO_CHANNEL_LOGICAL);
}

static const char *
em_audio_channel_name_get(void *ef, int channel)
{
   Emotion_Xine_Video *ev;
   static char lang[XINE_LANG_MAX + 1];
   
   ev = (Emotion_Xine_Video *)ef;
   if (ev->opening) return NULL;
   lang[0] = 0;
   if (xine_get_audio_lang(ev->stream, channel, lang)) return lang;
   return NULL;
}

static void
em_audio_channel_mute_set(void *ef, int mute)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   ev->audio_mute = mute;
   _em_slave_event(ev, 12, NULL);
}

static int
em_audio_channel_mute_get(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   return ev->audio_mute;
}

static void
em_audio_channel_volume_set(void *ef, double vol)
{
   Emotion_Xine_Video *ev;
   double *ppos;
   
   ev = (Emotion_Xine_Video *)ef;
   ppos = malloc(sizeof(double));
   *ppos = vol;
   _em_slave_event(ev, 10, ppos);
}

static double
em_audio_channel_volume_get(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   if (ev->opening || (!ev->play_ok)) return 0;
   return (double)xine_get_param(ev->stream, XINE_PARAM_AUDIO_VOLUME) / 100.0;
}

static int
em_spu_channel_count(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   if (ev->opening || (!ev->play_ok)) return 0;
   return xine_get_stream_info(ev->stream, XINE_STREAM_INFO_MAX_SPU_CHANNEL);
}

static void
em_spu_channel_set(void *ef, int channel)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   if (channel < 0) channel = 0;
   ev->spu_channel = channel;
   _em_slave_event(ev, 9, NULL);
}

static int
em_spu_channel_get(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   if (ev->opening || (!ev->play_ok)) return 0;
   return xine_get_param(ev->stream, XINE_PARAM_SPU_CHANNEL);
}

static const char *
em_spu_channel_name_get(void *ef, int channel)
{
   Emotion_Xine_Video *ev;
   static char lang[XINE_LANG_MAX + 1];
   
   ev = (Emotion_Xine_Video *)ef;
   if (ev->opening) return NULL;
   lang[0] = 0;
   if (xine_get_spu_lang(ev->stream, channel, lang)) return lang;
   return NULL;
}

static void
em_spu_channel_mute_set(void *ef, int mute)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   ev->spu_mute = mute;
   _em_slave_event(ev, 8, NULL);
}

static int
em_spu_channel_mute_get(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   return ev->spu_mute;
}

static int
em_chapter_count(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   if (ev->opening || (!ev->play_ok)) return 0;
   if (xine_get_stream_info(ev->stream, XINE_STREAM_INFO_HAS_CHAPTERS))
     return 99;
   return 0;
}

static void
em_chapter_set(void *ef, int chapter)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
}

static int
em_chapter_get(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   return 0;
}

static const char *
em_chapter_name_get(void *ef, int chapter)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   return NULL;
}

static void
em_speed_set(void *ef, double speed)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
}

static double
em_speed_get(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   return 1.0;
}

static int
em_eject(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   _em_slave_event(ev, 7, NULL);
   return 1;
}

static const char *
em_meta_get(void *ef, int meta)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   if ((ev->opening) || (!ev->play_ok)) return NULL;
   switch (meta)
     {
      case META_TRACK_TITLE:
	return xine_get_meta_info(ev->stream, XINE_META_INFO_TITLE);
	break;
      case META_TRACK_ARTIST:
	return xine_get_meta_info(ev->stream, XINE_META_INFO_ARTIST);
	break;
      case META_TRACK_GENRE:
	return xine_get_meta_info(ev->stream, XINE_META_INFO_GENRE);
	break;
      case META_TRACK_COMMENT:
	return xine_get_meta_info(ev->stream, XINE_META_INFO_COMMENT);
	break;
      case META_TRACK_ALBUM:
	return xine_get_meta_info(ev->stream, XINE_META_INFO_ALBUM);
	break;
      case META_TRACK_YEAR:
	return xine_get_meta_info(ev->stream, XINE_META_INFO_YEAR);
	break;
      case META_TRACK_DISCID:
	return xine_get_meta_info(ev->stream, XINE_META_INFO_CDINDEX_DISCID);
	break;
      default:
	break;
     }
   return NULL;
}

static int
_em_fd_active(void *data, Ecore_Fd_Handler *fdh)
{
   void *buf;
   int fd, len;
   Emotion_Xine_Video_Frame *fr;
   Emotion_Xine_Video *ev;
   
   ev = data;
   fd = ecore_main_fd_handler_fd_get(fdh);
   while ((len = read(fd, &buf, sizeof(buf))) > 0)
     {
	if (len == sizeof(buf))
	  {
	     fr = buf;
	     ev = _emotion_video_get(fr->obj);
	     if (ev)
	       {
		  em_frame_done(ev);
		  ev->cur_frame = fr;
		  _em_get_pos_len(ev);
		  if ((xine_get_stream_info(ev->stream, XINE_STREAM_INFO_HAS_VIDEO)) &&
		      (xine_get_stream_info(ev->stream, XINE_STREAM_INFO_VIDEO_HANDLED)))
		    {
 		       if (ev->video_mute) em_frame_done(ev);
		       else _emotion_frame_new(fr->obj);
		    }
		  _emotion_frame_resize(fr->obj, fr->w, fr->h, fr->ratio);
		  _emotion_video_pos_update(fr->obj, ev->pos, ev->len);
	       }
	  }
     }
   return 1;
}

static void
_em_event(void *data, const xine_event_t *event)
{
   void *buf[2];
   Emotion_Xine_Event *new_ev;
   Emotion_Xine_Video *ev;
   
   ev = data;
   new_ev = calloc(1, sizeof(Emotion_Xine_Event));
   if (!new_ev) return;
   new_ev->mtype = 0;
   new_ev->type = event->type;
   if (event->data)
     {
	new_ev->xine_event = malloc(event->data_length);
	if (!new_ev->xine_event)
	  {
	     free(new_ev);
	     return;
	  }
	memcpy(new_ev->xine_event, event->data, event->data_length);
     }
   buf[0] = data;
   buf[1] = new_ev;
   write(ev->fd_ev_write, buf, sizeof(buf));
}

static void
_em_module_event(void *data, int type)
{
   void *buf[2];
   Emotion_Xine_Event *new_ev;
   Emotion_Xine_Video *ev;
   
   ev = data;
   new_ev = calloc(1, sizeof(Emotion_Xine_Event));
   if (!new_ev) return;
   new_ev->mtype = type;
   new_ev->type = -1;
   buf[0] = data;
   buf[1] = new_ev;
   write(ev->fd_ev_write, buf, sizeof(buf));
}

static int
_em_fd_ev_active(void *data, Ecore_Fd_Handler *fdh)
{
   Emotion_Xine_Video *ev;
   int fd, len;
   void *buf[2];
   
   ev = data;
   fd = ecore_main_fd_handler_fd_get(fdh);
   while ((len = read(fd, buf, sizeof(buf))) > 0)
     {
	if (len == sizeof(buf))
	  {
	     Emotion_Xine_Event *eev;
	     
	     ev = buf[0];
	     eev = buf[1];
	     if (eev->mtype != 0)
	       {
		  switch (eev->mtype)
		    {
		     case 1: /* init done */
		       ev->play_ok = 1;
		       break;
		     case 2: /* open done */
		       ev->play_ok = 1;
		       break;
		     case 3: /* shutdown done */
		       ev->play_ok = 1;
		       break;
		     case 4: /* play done */
		       ev->play_ok = 1;
		       break;
		     case 5: /* stop done */
		       ev->play_ok = 1;
		       break;
		     case 6: /* seek done */
		       ev->play_ok = 1;
		       break;
		     case 7: /* eject done */
		       ev->play_ok = 1;
		       break;
		     case 8: /* spu mute done */
		       ev->play_ok = 1;
		       break;
		     case 9: /* channel done */
		       ev->play_ok = 1;
		       break;
		     case 10: /* volume done */
		       ev->play_ok = 1;
		       break;
		     case 11: /* close done */
		       ev->play_ok = 1;
		       break;
		     default:
		       break;
		    }
	       }
	     else
	       {
		  switch (eev->type)
		    {
		     case XINE_EVENT_UI_PLAYBACK_FINISHED:
			 {
			    ev->play = 0;
			    _emotion_decode_stop(ev->obj);
			    _emotion_playback_finished(ev->obj);
			 }
		       break;
		     case XINE_EVENT_UI_CHANNELS_CHANGED:
			 {
			    _emotion_channels_change(ev->obj);
			 }
		       break;
		     case XINE_EVENT_UI_SET_TITLE:
			 {
			    xine_ui_data_t *e;
			    
			    e = (xine_ui_data_t *)eev->xine_event;
			    _emotion_title_set(ev->obj, e->str);
			 }
		       break;
		     case XINE_EVENT_FRAME_FORMAT_CHANGE:
			 {
			    xine_format_change_data_t *e;
			    
			    e = (xine_format_change_data_t *)eev->xine_event;
			 }
		       break;
		     case XINE_EVENT_UI_MESSAGE:
			 {
			    xine_ui_message_data_t *e;
			    
			    e = (xine_ui_message_data_t *)eev->xine_event;
			    printf("EV: UI Message [FIXME: break this out to emotion api]\n");
			    // e->type = error type(XINE_MSG_NO_ERROR, XINE_MSG_GENERAL_WARNING, XINE_MSG_UNKNOWN_HOST etc.)
			    // e->messages is a list of messages DOUBLE null terminated
			 }
		       break;
		     case XINE_EVENT_AUDIO_LEVEL:
			 {
			    xine_audio_level_data_t *e;
			    
			    e = (xine_audio_level_data_t *)eev->xine_event;
			    _emotion_audio_level_change(ev->obj);
			    printf("EV: Audio Level [FIXME: break this out to emotion api]\n");
			    // e->left (0->100) 
			    // e->right
			    // e->mute
			 }
		       break;
		     case XINE_EVENT_PROGRESS:
			 {
			    xine_progress_data_t *e;
			    
			    e = (xine_progress_data_t *)eev->xine_event;
			    printf("PROGRESS: %i\n", e->percent);
			    _emotion_progress_set(ev->obj, (char *)e->description, (double)e->percent / 100.0);
			 }
		       break;
		     case XINE_EVENT_MRL_REFERENCE:
			 {
			    xine_mrl_reference_data_t *e;
			    
			    e = (xine_mrl_reference_data_t *)eev->xine_event;
			    _emotion_file_ref_set(ev->obj, e->mrl, e->alternative);
			 }
		       break;
		     case XINE_EVENT_UI_NUM_BUTTONS:
			 {
			    xine_ui_data_t *e;
			    
			    e = (xine_ui_data_t *)eev->xine_event;
			    _emotion_spu_button_num_set(ev->obj, e->num_buttons);
			 }
		       break;
		     case XINE_EVENT_SPU_BUTTON:
			 {
			    xine_spu_button_t *e;
			    
			    e = (xine_spu_button_t *)eev->xine_event;
			    if (e->direction == 1)
			      _emotion_spu_button_set(ev->obj, e->button);
			    else
			      _emotion_spu_button_set(ev->obj, -1);
			 }
		       break;
		     case XINE_EVENT_DROPPED_FRAMES:
			 {
			    xine_dropped_frames_t *e;
			    
			    e = (xine_dropped_frames_t *)eev->xine_event;
			    printf("EV: Dropped Frames (skipped %i) (discarded %i) [FIXME: break this out to the emotion api]\n", e->skipped_frames, e->discarded_frames);
			    // e->skipped_frames = % frames skipped * 10
			    // e->discarded_frames = % frames skipped * 10
			 }
		       break;
		     default:
		       // printf("EV: unknown event type %i\n", eev->type);
		       break;
		    }
	       }
	     if (eev->xine_event) free(eev->xine_event);
	     free(eev);
	  }
     }
   return 1;
}

static void *
_em_get_pos_len_th(void *par)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)par;
   
   for (;;)
     {
	pthread_mutex_lock(&(ev->get_pos_len_mutex));
	pthread_cond_wait(&(ev->get_pos_len_cond), &(ev->get_pos_len_mutex));
	pthread_mutex_unlock(&(ev->get_pos_len_mutex));
	if (ev->get_poslen)
	  {
	     int pos_stream = 0;
	     int pos_time = 0;
	     int length_time = 0;
	     
	     if (xine_get_pos_length(ev->stream, &pos_stream, &pos_time, &length_time))
	       {
		  if (length_time == 0)
		    {
		       ev->pos = (double)pos_stream / 65535;
		       ev->len = 1.0;
		       ev->no_time = 1;
		    }
		  else
		    {
		       ev->pos = (double)pos_time / 1000.0;
		       ev->len = (double)length_time / 1000.0;
		       ev->no_time = 0;
		    }
	       }
	     ev->get_poslen = 0;
	     //printf("get pos %3.3f\n", ev->pos);
	  }
	if (ev->delete_me)
	  {
	     ev->get_pos_thread_deleted = 1;
	     return NULL;     
	  }
     }
   return NULL;
}

static void
_em_get_pos_len(Emotion_Xine_Video *ev)
{
   if (ev->get_poslen) return;
   ev->get_poslen = 1;
   pthread_mutex_lock(&(ev->get_pos_len_mutex));
   pthread_cond_broadcast(&(ev->get_pos_len_cond));
   pthread_mutex_unlock(&(ev->get_pos_len_mutex));
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
     em_vis_set, /* vis_set */
     em_len_get, /* len_get */
     em_fps_num_get, /* fps_num_get */
     em_fps_den_get, /* fps_den_get */
     em_fps_get, /* fps_get */
     em_pos_get, /* pos_get */
     em_vis_get, /* vis_get */
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

unsigned char
module_open(Evas_Object *obj, const Emotion_Video_Module **module, void **video, Emotion_Module_Options *opt)
{
   if (!module)
      return 0;
   
   if (!em_module.init(obj, video, opt))
      return 0;

   *module = &em_module;
   return 1;
}

void
module_close(Emotion_Video_Module *module, void *video)
{
   em_module.shutdown(video);
}

#if 0
void
em_debug(Emotion_Xine_Video *ev)
{
   int has_chapters = 0;
   int max_spu = 0;
   int max_audio = 0;
   int video_channels = 0;
   int video_streams = 0;
   int video_seekable = 0;
   char *title;
   char *comment;
   char *artist;
   char *genre;
   char *album;
   char *year;
   char *cdindex_discid;
   int video_channel = 0;
   int audio_channel = 0;
   int spu_channel = 0;
   int video_ratio = 0;
   int audio_mode = 0;

//   return;
   has_chapters = xine_get_stream_info(ev->stream, XINE_STREAM_INFO_HAS_CHAPTERS);
   max_spu = xine_get_stream_info(ev->stream, XINE_STREAM_INFO_MAX_SPU_CHANNEL);
   max_audio = xine_get_stream_info(ev->stream, XINE_STREAM_INFO_MAX_AUDIO_CHANNEL);
   video_channels = xine_get_stream_info(ev->stream, XINE_STREAM_INFO_VIDEO_CHANNELS);
   video_streams = xine_get_stream_info(ev->stream, XINE_STREAM_INFO_VIDEO_STREAMS);
   video_seekable = xine_get_stream_info(ev->stream, XINE_STREAM_INFO_SEEKABLE);
   title = xine_get_meta_info(ev->stream, XINE_META_INFO_TITLE);
   comment = xine_get_meta_info(ev->stream, XINE_META_INFO_COMMENT);
   artist = xine_get_meta_info(ev->stream, XINE_META_INFO_ARTIST);
   genre = xine_get_meta_info(ev->stream, XINE_META_INFO_GENRE);
   album = xine_get_meta_info(ev->stream, XINE_META_INFO_ALBUM);
   year = xine_get_meta_info(ev->stream, XINE_META_INFO_YEAR);
   cdindex_discid = xine_get_meta_info(ev->stream, XINE_META_INFO_CDINDEX_DISCID);
   video_channel = xine_get_param(ev->stream, XINE_PARAM_VIDEO_CHANNEL);
   audio_channel = xine_get_param(ev->stream, XINE_PARAM_AUDIO_CHANNEL_LOGICAL);
   spu_channel = xine_get_param(ev->stream, XINE_PARAM_SPU_CHANNEL);
   video_ratio = xine_get_stream_info(ev->stream, XINE_STREAM_INFO_VIDEO_RATIO);
   audio_mode = xine_get_stream_info(ev->stream, XINE_STREAM_INFO_AUDIO_MODE);
   printf("has_chapters = %i\n", has_chapters);
   printf("max_spu = %i\n", max_spu);
   printf("max_audio = %i\n", max_audio);
   printf("video_channels = %i\n", video_channels);
   printf("video_streams = %i\n", video_streams);
   printf("video_seekable = %i\n", video_seekable);
   printf("title = %s\n", title);
   printf("comment = %s\n", comment);
   printf("artist = %s\n", artist);
   printf("genre = %s\n", genre);
   printf("album = %s\n", album);
   printf("year = %s\n", year);
   printf("cdindex_discid = %s\n", cdindex_discid);
   printf("video_channel = %i\n", video_channel);
   printf("audio_channel = %i\n", audio_channel);
   printf("spu_channels = %i\n", spu_channel);
   printf("video_ratio = %i\n", video_ratio);
   printf("audio_mode = %i\n", audio_mode);
     {
	int i;
	
	for (i = 0; i <= max_audio; i++)
	  {
	     char lang[XINE_LANG_MAX + 1];
	     
	     lang[0] = 0;
	     printf("  AUDIO %i = ", i);
	     if (xine_get_audio_lang(ev->stream, i, lang))
	       printf("%s\n", lang);
	     else
	       printf("NONE\n");
	  }
	for (i = 0; i <= max_spu; i++)
	  {
	     char lang[XINE_LANG_MAX + 1];
	     
	     lang[0] = 0;
	     printf("  SPU %i = ", i);
	     if (xine_get_spu_lang(ev->stream, i, lang))
	       printf("%s\n", lang);
	     else
	       printf("NONE\n");
	  }
     }
}
#endif
