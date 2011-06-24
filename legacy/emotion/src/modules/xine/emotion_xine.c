#include "Emotion.h"
#include "emotion_private.h"
#include "emotion_xine.h"

int _emotion_xine_log_domain = -1;

/* module api */
static unsigned char  em_init                    (Evas_Object *obj, void **emotion_video, Emotion_Module_Options *opt);
static int            em_shutdown                (void *ef);
static unsigned char  em_file_open               (const char *file, Evas_Object *obj, void *ef);
static void           em_file_close              (void *ef);
static void           em_play                    (void *ef, double pos);
static void           em_stop                    (void *ef);
static void           em_size_get                (void *ef, int *w, int *h);
static void           em_pos_set                 (void *ef, double pos);
static double         em_len_get                 (void *ef);
static int            em_fps_num_get             (void *ef);
static int            em_fps_den_get             (void *ef);
static double         em_fps_get                 (void *ef);
static double         em_pos_get                 (void *ef);
static void           em_vis_set                 (void *ef, Emotion_Vis vis);
static Emotion_Vis    em_vis_get                 (void *ef);
static Eina_Bool      em_vis_supported           (void *ef, Emotion_Vis vis);
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
static Eina_Bool   _em_fd_active     (void *data, Ecore_Fd_Handler *fdh);
static void  _em_event         (void *data, const xine_event_t *event);
static void  _em_module_event  (void *data, int type);
static Eina_Bool   _em_fd_ev_active  (void *data, Ecore_Fd_Handler *fdh);
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
		       DBG("OPEN VIDEO PLUGIN...");
		       if (!ev->opt_no_video)
			 ev->video = xine_open_video_driver(ev->decoder, "emotion",
							    XINE_VISUAL_TYPE_NONE, ev);
		       DBG("RESULT: xine_open_video_driver() = %p", ev->video);
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
		       DBG("shutdown stop");
		       xine_stop(ev->stream);
		       //   pthread_mutex_lock(&(ev->get_pos_len_mutex));
		       if (!ev->get_pos_thread_deleted)
			 {
			    DBG("closing get_pos thread, %p", ev);
			    pthread_mutex_lock(&(ev->get_pos_len_mutex));
			    pthread_cond_broadcast(&(ev->get_pos_len_cond));
			    pthread_mutex_unlock(&(ev->get_pos_len_mutex));
			    while (ev->get_poslen);
			 }
		       DBG("dispose %p", ev);
		       xine_dispose(ev->stream);
		       DBG("dispose evq %p", ev);
		       xine_event_dispose_queue(ev->queue);
		       DBG("close video drv %p", ev);
		       if (ev->video) xine_close_video_driver(ev->decoder, ev->video);
		       DBG("wait for vo to go");
		       while (ev->have_vo);
		       DBG("vo gone");
		       DBG("close audio drv %p", ev);
		       if (ev->audio) xine_close_audio_driver(ev->decoder, ev->audio);
		       DBG("xine exit %p", ev);
		       xine_exit(ev->decoder);
		       DBG("DONE %p", ev);
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
		       DBG("OPEN STREAM %s", file);
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
			    xine_set_param(ev->stream, XINE_PARAM_AUDIO_VOLUME, ev->volume * 100);
			 }
		       _em_module_event(ev, 2); /* event - open done */
		    }
		  break;
		case 11: /* file close */
		    {
		       DBG("done %p", ev);
		       em_frame_done(ev); 
		       DBG("stop %p", ev);
		       xine_stop(ev->stream);
		       DBG("close %p", ev);
		       xine_close(ev->stream);
		       DBG("close done %p", ev);
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
		       if (ev->no_time)
			 xine_play(ev->stream, pos * 65535, 0);
		       else
			 xine_play(ev->stream, 0, pos * 1000);
		       if (!ev->play)
			 xine_set_param(ev->stream, XINE_PARAM_SPEED, XINE_SPEED_PAUSE);
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
		       xine_set_param(ev->stream, XINE_PARAM_AUDIO_VOLUME, ev->volume * 100);
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
   ev->volume = 0.8;
   ev->delete_me = 0;
   ev->get_pos_thread_deleted = 0;
   ev->opening = 1;
   ev->play_ok = 0;
   
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
   DBG("del fds %p", ev);
   ecore_main_fd_handler_del(ev->fd_handler);
   ev->fd_handler = NULL;
   ecore_main_fd_handler_del(ev->fd_ev_handler);
   ev->fd_ev_handler = NULL;
   if (ev->anim)
     {
        ecore_animator_del(ev->anim);
        ev->anim = NULL;
     }
   
   ev->closing = 1;
   _em_slave_event(ev, 3, NULL);
   DBG("done %p", ev);
   return 1;
}

static unsigned char
em_file_open(const char *file, Evas_Object *obj __UNUSED__, void *ef)
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
   ppos = malloc(sizeof(double));
   *ppos = pos;
   _em_slave_event(ev, 6, ppos);
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
em_fps_den_get(void *ef __UNUSED__)
{
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
   _em_get_pos_len(ev);
   return ev->pos;
}

static void
em_vis_set(void *ef, Emotion_Vis vis)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   if (ev->vis == vis) return;
   ev->vis = vis;
}

static Emotion_Vis
em_vis_get(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;

   return ev->vis;
}

static Eina_Bool
em_vis_supported(void *ef __UNUSED__, Emotion_Vis vis __UNUSED__)
{
   return EINA_FALSE;
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
em_yuv_rows_get(void *ef, int w __UNUSED__, int h, unsigned char **yrows, unsigned char **urows, unsigned char **vrows)
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
em_event_mouse_button_feed(void *ef, int button __UNUSED__, int x, int y)
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
em_video_channel_name_get(void *ef __UNUSED__, int channel __UNUSED__)
{
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
   
   if (vol < 0.0) vol = 0.0;
   else if (vol > 1.0) vol = 1.0;

   ev = (Emotion_Xine_Video *)ef;
   ev->volume = vol;
   _em_slave_event(ev, 10, NULL);
}

static double
em_audio_channel_volume_get(void *ef)
{
   Emotion_Xine_Video *ev;
   
   ev = (Emotion_Xine_Video *)ef;
   if (ev->opening || (!ev->play_ok)) return ev->volume;
   ev->volume = xine_get_param(ev->stream, XINE_PARAM_AUDIO_VOLUME) / 100.0;
   return ev->volume;
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
em_chapter_set(void *ef __UNUSED__, int chapter __UNUSED__)
{
}

static int
em_chapter_get(void *ef __UNUSED__)
{
   return 0;
}

static const char *
em_chapter_name_get(void *ef __UNUSED__, int chapter __UNUSED__)
{
   return NULL;
}

static void
em_speed_set(void *ef __UNUSED__, double speed __UNUSED__)
{
}

static double
em_speed_get(void *ef __UNUSED__)
{
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

static Eina_Bool
_em_fd_active(void *data, Ecore_Fd_Handler *fdh)
{
   void *buf;
   int fd, len;
   Emotion_Xine_Video_Frame *fr;
   
   fd = ecore_main_fd_handler_fd_get(fdh);
   while ((len = read(fd, &buf, sizeof(buf))) > 0)
     {
	if (len == sizeof(buf))
	  {
	     Emotion_Xine_Video *ev;

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
   return EINA_TRUE;
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

static Eina_Bool
_em_audio_only_poller(void *data)
{
   Emotion_Xine_Video *ev;
   
   ev = data;
   _em_get_pos_len(ev);
   return EINA_TRUE;
}

static Eina_Bool
_em_fd_ev_active(void *data __UNUSED__, Ecore_Fd_Handler *fdh)
{
   int fd, len;
   void *buf[2];
   
   fd = ecore_main_fd_handler_fd_get(fdh);
   while ((len = read(fd, buf, sizeof(buf))) > 0)
     {
	if (len == sizeof(buf))
	  {
	     Emotion_Xine_Video *ev;
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
                       if (ev->anim)
                         {
                            ecore_animator_del(ev->anim);
                            ev->anim = NULL;
                         }
                       _emotion_open_done(ev->obj);
		       _emotion_frame_resize(ev->obj, ev->w, ev->h, ev->ratio);
		       break;
		     case 3: /* shutdown done */
                       if (ev->anim)
                         {
                            ecore_animator_del(ev->anim);
                            ev->anim = NULL;
                         }
		       ev->play_ok = 1;
		       break;
		     case 4: /* play done */
		       ev->play_ok = 1;
                       if (ev->anim)
                         {
                            ecore_animator_del(ev->anim);
                            ev->anim = NULL;
                         }
                       if ((!(xine_get_stream_info(ev->stream, XINE_STREAM_INFO_HAS_VIDEO) &&
                              xine_get_stream_info(ev->stream, XINE_STREAM_INFO_VIDEO_HANDLED))) &&
                           (xine_get_stream_info(ev->stream, XINE_STREAM_INFO_HAS_AUDIO) &&
                               xine_get_stream_info(ev->stream, XINE_STREAM_INFO_AUDIO_HANDLED)))
                          ev->anim = ecore_animator_add(_em_audio_only_poller, ev);
                       _emotion_playback_started(ev->obj);
		       break;
		     case 5: /* stop done */
                       if (ev->anim)
                         {
                            ecore_animator_del(ev->anim);
                            ev->anim = NULL;
                         }
		       ev->play_ok = 1;
		       break;
		     case 6: /* seek done */
		       ev->play_ok = 1;
                       _emotion_seek_done(ev->obj);
                       _em_get_pos_len(ev);
                       _emotion_video_pos_update(ev->obj, ev->pos, ev->len);
		       break;
		     case 7: /* eject done */
                       if (ev->anim)
                         {
                            ecore_animator_del(ev->anim);
                            ev->anim = NULL;
                         }
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
                       if (ev->anim)
                         {
                            ecore_animator_del(ev->anim);
                            ev->anim = NULL;
                         }
		       ev->play_ok = 1;
		       break;
                     case 15: /* get pos done */
                       if (ev->last_pos != ev->pos)
                         {
                            ev->last_pos = ev->pos;
                            _emotion_video_pos_update(ev->obj, ev->pos, ev->len);
                         }
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
		       break;
		     case XINE_EVENT_UI_MESSAGE:
			 {
			    WRN("UI Message [FIXME: break this out to emotion api]");
			    // e->type = error type(XINE_MSG_NO_ERROR, XINE_MSG_GENERAL_WARNING, XINE_MSG_UNKNOWN_HOST etc.)
			    // e->messages is a list of messages DOUBLE null terminated
			 }
		       break;
		     case XINE_EVENT_AUDIO_LEVEL:
			 {
			    _emotion_audio_level_change(ev->obj);
			    WRN("Audio Level [FIXME: break this out to emotion api]");
			    // e->left (0->100) 
			    // e->right
			    // e->mute
			 }
		       break;
		     case XINE_EVENT_PROGRESS:
			 {
			    xine_progress_data_t *e;
			    
			    e = (xine_progress_data_t *)eev->xine_event;
			    DBG("PROGRESS: %i", e->percent);
			    _emotion_progress_set(ev->obj, (char *)e->description, (double)e->percent / 100.0);
			 }
		       break;
		     case XINE_EVENT_MRL_REFERENCE_EXT:
			 {
			    xine_mrl_reference_data_ext_t *e;
			    
			    e = (xine_mrl_reference_data_ext_t *)eev->xine_event;
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
			    WRN("Dropped Frames (skipped %i) (discarded %i) [FIXME: break this out to the emotion api]", e->skipped_frames, e->discarded_frames);
			    // e->skipped_frames = % frames skipped * 10
			    // e->discarded_frames = % frames skipped * 10
			 }
		       break;
		     default:
		       // DBG("unknown event type %i", eev->type);
		       break;
		    }
	       }
	     if (eev->xine_event) free(eev->xine_event);
	     free(eev);
	  }
     }
   return EINA_TRUE;
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
             _em_module_event(ev, 15); /* event - getpos done */
	     //DBG("get pos %3.3f", ev->pos);
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
   if (!ev->play_ok) return;
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
     _emotion_object_extension_can_play_generic_get, /* can_play_get */
     NULL /* handle */
};

static Eina_Bool
module_open(Evas_Object *obj, const Emotion_Video_Module **module, void **video, Emotion_Module_Options *opt)
{
   if (!module)
      return EINA_FALSE;

   if (_emotion_xine_log_domain < 0)
     {
        eina_threads_init();
        eina_log_threads_enable();
        _emotion_xine_log_domain = eina_log_domain_register
          ("emotion-xine", EINA_COLOR_LIGHTCYAN);
        if (_emotion_xine_log_domain < 0)
          {
             EINA_LOG_CRIT("Could not register log domain 'emotion-xine'");
             return EINA_FALSE;
          }
     }

   if (!em_module.init(obj, video, opt))
      return EINA_FALSE;

   *module = &em_module;
   return EINA_TRUE;
}

static void
module_close(Emotion_Video_Module *module __UNUSED__, void *video)
{
   em_module.shutdown(video);
}

Eina_Bool
xine_module_init(void)
{
   return _emotion_module_register("xine", module_open, module_close);
}

void
xine_module_shutdown(void)
{
   _emotion_module_unregister("xine");
}

#ifndef EMOTION_STATIC_BUILD_XINE

EINA_MODULE_INIT(xine_module_init);
EINA_MODULE_SHUTDOWN(xine_module_shutdown);

#endif

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
   DBG("has_chapters = %i", has_chapters);
   DBG("max_spu = %i", max_spu);
   DBG("max_audio = %i", max_audio);
   DBG("video_channels = %i", video_channels);
   DBG("video_streams = %i", video_streams);
   DBG("video_seekable = %i", video_seekable);
   DBG("title = %s", title);
   DBG("comment = %s", comment);
   DBG("artist = %s", artist);
   DBG("genre = %s", genre);
   DBG("album = %s", album);
   DBG("year = %s", year);
   DBG("cdindex_discid = %s", cdindex_discid);
   DBG("video_channel = %i", video_channel);
   DBG("audio_channel = %i", audio_channel);
   DBG("spu_channels = %i", spu_channel);
   DBG("video_ratio = %i", video_ratio);
   DBG("audio_mode = %i", audio_mode);
     {
	int i;
	
	for (i = 0; i <= max_audio; i++)
	  {
	     char lang[XINE_LANG_MAX + 1];
             char buf[128] = "NONE";

	     lang[0] = 0;
	     if (xine_get_audio_lang(ev->stream, i, lang))
	       eina_strlcpy(buf, lang, sizeof(buf));
	     DBG("  AUDIO %i = %s", i, buf);
	  }
	for (i = 0; i <= max_spu; i++)
	  {
	     char lang[XINE_LANG_MAX + 1];
             char buf[128] = "NONE";

	     lang[0] = 0;
	     if (xine_get_spu_lang(ev->stream, i, lang))
               eina_strlcpy(buf, lang, sizeof(buf));
	     DBG("  SPU %i = %s", i, buf);
	  }
     }
}
#endif
