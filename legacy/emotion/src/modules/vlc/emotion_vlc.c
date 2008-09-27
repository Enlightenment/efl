/* 
 * SICMA AERO SEAT
 * code@ife-sit.info
 */

#ifndef EMOTION_VLC_C
#define EMOTION_VLC_C

#include "Emotion.h"
#include "emotion_private.h"
#include "emotion_vlc.h"

void * _em_slave_thread(void * t)
{
	int event;
	pthread_detach(pthread_self());
	Emotion_Vlc_Video *ev=  (Emotion_Vlc_Video*)t;
	while(read(ev->fd_slave_read, &event,  sizeof(int)) > 0)	{
		switch(event)	{
			case VLC_RESIZE:
				_em_resize(ev, -1, -1);
			break;
			case VLC_CHANGE_VOL:
				_emotion_audio_level_change(ev->obj);
			break;
			default:
			break;
		}
	}
	fprintf(stderr,"BYE BYE");
	return NULL;
}

static unsigned char em_init(Evas_Object *obj, void **emotion_video, Emotion_Module_Options *opt)
{
	Emotion_Vlc_Video *ev;
	Emotion_Vlc_Video_Frame *ef;
	int fds[2], x, y;
	int event = VLC_NEW_FRAME;
	pthread_t t_id;
	x = WIDTH;
	y = HEIGHT;


	if (!emotion_video) return 0;

	ev = (Emotion_Vlc_Video*)malloc(sizeof(Emotion_Vlc_Video));
	ASSERT_EV(ev) return 0;
	memset(ev, 0, sizeof(Emotion_Vlc_Video));

	ef = (Emotion_Vlc_Video_Frame*)malloc(sizeof(Emotion_Vlc_Video_Frame));
	if (!ef) return 0;
	memset(ef, 0, sizeof(Emotion_Vlc_Video_Frame));

	/* init internal frames */
	if(ef->frame_A)	{
		free(ef->frame_A);
	}
	if(ef->frame_B)	{
		free(ef->frame_B);
	}
	ef->frame_A = (char*)malloc(x*y*sizeof(char)*4);
	ef->frame_B = (char*)malloc(x*y*sizeof(char)*4);
	memset(ef->frame_A, 0x00, x*y*sizeof(char)*4);
	memset(ef->frame_B, 0x00, x*y*sizeof(char)*4);
	ef->frame = ef->frame_A;

	/* update size in object */
	ev->w = x;
	ev->h = y;
	ev->ratio = (double)x/(double)y;

	ev->cur_frame = ef;

	if(pipe(fds) == 0)
	{
		ev->fd_read = fds[0];
		ev->fd_write = fds[1];
		fcntl(ev->fd_read, F_SETFL, O_NONBLOCK);
		ev->fd_handler = ecore_main_fd_handler_add(ev->fd_read, ECORE_FD_READ, _em_fd_active, ev, NULL, NULL);
		ecore_main_fd_handler_active_set(ev->fd_handler, ECORE_FD_READ);
	}

	if (pipe(fds) == 0)
	{
		ev->fd_slave_read = fds[0];
		ev->fd_slave_write = fds[1];
		fcntl(ev->fd_slave_write, F_SETFL, O_NONBLOCK);
	}
	ev->obj = obj;

	ev->delete_me = 0;
	ev->opening = 1;
	ev->play_ok = 0;
	ev->video_mute = 1;

	if (opt)
	{
		ev->opt_no_audio = opt->no_audio;
		ev->opt_no_video = opt->no_video;
	}


	/* init libvlc */
	int vlc_argc = 0;
	char const *vlc_argv[] =
	{
		"-q",
		//"-vvvvv",
		"--ignore-config",
		"--vout", "vmem",
		"--vmem-width", ev->width,
		"--vmem-height", ev->height,
		"--vmem-pitch", ev->pitch,
		"--vmem-chroma", "RV32",
		"--vmem-lock", ev->clock,
		"--vmem-unlock", ev->cunlock,
		"--vmem-data", ev->cdata,
	};
	vlc_argc = sizeof(vlc_argv) / sizeof(*vlc_argv);
	sprintf(ev->clock, "%lld", (long long int)(intptr_t)_em_lock);
	sprintf(ev->cunlock, "%lld", (long long int)(intptr_t)_em_unlock);
	sprintf(ev->cdata, "%lld", (long long int)(intptr_t)ev);
	sprintf(ev->width, "%d", WIDTH);
	sprintf(ev->height, "%d", HEIGHT);
	sprintf(ev->pitch, "%d", 4*WIDTH);

	ev->vlc_ex = (libvlc_exception_t*)malloc(sizeof(libvlc_exception_t));
	libvlc_exception_init(ev->vlc_ex);
	ev->vlc_player = libvlc_new(vlc_argc, vlc_argv, ev->vlc_ex);
	CATCH(ev->vlc_ex)

	if(ev->vlc_player)	{
		ev->play_ok = 1;
	}

	pthread_mutex_init(&ef->frame_copy_lock, NULL);
	pthread_create(&t_id, NULL, _em_slave_thread, ev);

	ev->opening = 0;
	write(ev->fd_write, &event, sizeof(void *));

	*emotion_video = ev;
	return 1;
}

static int em_shutdown(void *ef)
{
	Emotion_Vlc_Video *ev;
	Emotion_Vlc_Video_Frame *evvf;

	ev = (Emotion_Vlc_Video *)ef;
	evvf = ev->cur_frame;

	ASSERT_EV_VLC(ev) return 0;
	libvlc_release(ev->vlc_player);
	CATCH(ev->vlc_ex)
	ev->vlc_player = NULL;
	free(ev->vlc_ex);

	ev->closing = 0;

	ecore_main_fd_handler_del(ev->fd_handler);

	if(evvf)	{
		if(evvf->frame_A)	{
			free(evvf->frame_A);
		}
		if(evvf->frame_B)	{
			free(evvf->frame_B);
		}
		pthread_mutex_destroy(&(evvf->frame_copy_lock));
		free(evvf);
	}
	close(ev->fd_write);
	close(ev->fd_read);
	close(ev->fd_slave_write);
	close(ev->fd_slave_read);

	return 1;
}

static unsigned char em_file_open(const char *file, Evas_Object *obj, void *ef)
{
	Emotion_Vlc_Video *ev;
	int i;

	ev = (Emotion_Vlc_Video *)ef;
	ASSERT_EV(ev) return 0;
	ASSERT_EV_VLC(ev) return 0;

	ev->vlc_m = libvlc_media_new(ev->vlc_player, file, ev->vlc_ex);
	CATCH(ev->vlc_ex)
	ASSERT_EV_M(ev) return 0;
	ev->vlc_mp = libvlc_media_player_new_from_media(ev->vlc_m, ev->vlc_ex);
	CATCH(ev->vlc_ex)
	libvlc_media_release(ev->vlc_m); 
	CATCH(ev->vlc_ex)
	ev->vlc_m = NULL;
	ASSERT_EV_MP(ev) return 0;

	ev->vlc_evtmngr = libvlc_media_player_event_manager (ev->vlc_mp, ev->vlc_ex);
	CATCH(ev->vlc_ex)

	for(i = EVENT_BEG_ID; i<EVENT_BEG_ID+NB_EVENTS; i++)	{
		libvlc_event_attach(ev->vlc_evtmngr, (libvlc_event_type_t)i, _em_event, ev, ev->vlc_ex);
		CATCH(ev->vlc_ex)
	}

	if(ev->filename)	{
		free(ev->filename);
		ev->filename = NULL;
	}
	ev->filename = strdup(file);

	/* set properties to video object */
	ev->just_loaded = 1;

	return 1;
}

static void em_file_close(void *ef)
{
	Emotion_Vlc_Video *ev;
	int i;

	ev = (Emotion_Vlc_Video *)ef;
	ASSERT_EV(ev) return;
	ASSERT_EV_MP(ev) return;

	for(i = EVENT_BEG_ID; i<EVENT_BEG_ID+NB_EVENTS; i++)	{
		libvlc_event_detach(ev->vlc_evtmngr, (libvlc_event_type_t)i, _em_event, ev, ev->vlc_ex);
		CATCH(ev->vlc_ex)
	}
	libvlc_media_player_stop(ev->vlc_mp, ev->vlc_ex);
	CATCH(ev->vlc_ex)
	libvlc_media_player_release(ev->vlc_mp);
	CATCH(ev->vlc_ex)
	ev->vlc_mp = NULL;
	if(ev->filename)	{
		free(ev->filename);
		ev->filename = NULL;
	}
}

static void em_play(void *ef, double pos)
{
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	ASSERT_EV(ev) return;
	ASSERT_EV_MP(ev) return;

	ev->play = 1;
	ev->play_ok = 0;

	if(pos != ev->pos)	{
		libvlc_media_player_set_time(ev->vlc_mp, (libvlc_time_t)(pos*1000), ev->vlc_ex);
	}
	
	libvlc_media_player_play(ev->vlc_mp, ev->vlc_ex);

	ev->just_loaded = 0;
}

static void em_stop(void *ef)
{
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	ASSERT_EV(ev) return;
	ASSERT_EV_MP(ev) return;

	ev->play = 0;
	ev->play_ok = 0;
	libvlc_media_player_pause(ev->vlc_mp, ev->vlc_ex);
	CATCH(ev->vlc_ex)
}

static void em_size_get(void *ef, int *w, int *h)
{
	Emotion_Vlc_Video *ev;
	Emotion_Vlc_Video_Frame *fr;

	ev = (Emotion_Vlc_Video *)ef;
	ASSERT_EV(ev) return;
	fr = ev->cur_frame;

	pthread_mutex_lock(&fr->frame_copy_lock);
	if(w) *w = ev->w;
	if(h) *h = ev->h;
	pthread_mutex_unlock(&fr->frame_copy_lock);
}

static void em_pos_set(void *ef, double pos)
{
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	ASSERT_EV(ev) return;
	ASSERT_EV_MP(ev) return;
	libvlc_media_player_set_time(ev->vlc_mp, (libvlc_time_t)(pos*1000), ev->vlc_ex);
	CATCH(ev->vlc_ex)
}

static void em_vis_set(void *ef, Emotion_Vis vis)
{
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	ASSERT_EV(ev) return;
	if (ev->vis == vis) return;
	ev->vis = vis;
}

static double em_len_get(void *ef)
{
	Emotion_Vlc_Video *ev;
	ASSERT_EV(ev) return 0;

	ev = (Emotion_Vlc_Video *)ef;
	return ev->len;
}

static int em_fps_num_get(void *ef)
{
	Emotion_Vlc_Video *ev;

	ASSERT_EV(ev) return 0;
	ev = (Emotion_Vlc_Video *)ef;
	return (int)(ev->fps * 1000.0);
}

static int em_fps_den_get(void *ef)
{
	Emotion_Vlc_Video *ev;

	return 1000;
}

static double em_fps_get(void *ef)
{
	Emotion_Vlc_Video *ev;

	ASSERT_EV(ev) return 0;
	ev = (Emotion_Vlc_Video *)ef;
	return ev->fps;
}

static double em_pos_get(void *ef)
{
	Emotion_Vlc_Video *ev;

	ASSERT_EV(ev) return 0;
	ev = (Emotion_Vlc_Video *)ef;
	return ev->pos;
}

static Emotion_Vis em_vis_get(void *ef)
{
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	return ev->vis;
}

static double em_ratio_get(void *ef)
{
	double ratio;
	Emotion_Vlc_Video *ev;
	Emotion_Vlc_Video_Frame *fr;

	ASSERT_EV(ev) return 0;
	ev = (Emotion_Vlc_Video *)ef;
	fr = ev->cur_frame;
	pthread_mutex_lock(&fr->frame_copy_lock);
	ratio = ev->ratio;
	pthread_mutex_unlock(&fr->frame_copy_lock);
	return ratio;
}

static int em_video_handled(void *ef)
{
	int ret = 0;
	Emotion_Vlc_Video *ev;

	ASSERT_EV(ev) return 0;
	ASSERT_EV_MP(ev) return 0;
	ev = (Emotion_Vlc_Video *)ef;
	if (ev->opening || (!ev->play_ok)) return 0;

	ret = libvlc_media_player_has_vout(ev->vlc_mp, ev->vlc_ex);
	CATCH(ev->vlc_ex)
	return ret;
}

static int em_audio_handled(void *ef)
{
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	ASSERT_EV(ev) return 0;
	if (ev->opening || (!ev->play_ok)) return 0;
	return 1;
}

static int em_seekable(void *ef)
{
	int ret = 0;
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	ASSERT_EV(ev) return 0;
	ASSERT_EV_MP(ev) return 0;
	if (ev->opening || (!ev->play_ok)) return 0;
	ret = vlc_media_player_is_seekable(ev->vlc_mp, ev->vlc_ex);
	CATCH(ev->vlc_ex)
	return ret;
}

static void em_frame_done(void *ef)
{
	return;
}

static Emotion_Format em_format_get(void *ef)
{
	Emotion_Vlc_Video *ev;

	return EMOTION_FORMAT_BGRA;
}

static void em_video_data_size_get(void *ef, int *w, int *h)
{
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	ASSERT_EV(ev) return;
	if (w) *w = ev->w;
	if (h) *h = ev->h;
}

static int em_yuv_rows_get(void *ef, int w, int h, unsigned char **yrows, unsigned char **urows, unsigned char **vrows)
{
	Emotion_Vlc_Video *ev;
	return 0;
}

static int em_bgra_data_get(void *ef, unsigned char **bgra_data)
{
	Emotion_Vlc_Video *ev;
	Emotion_Vlc_Video_Frame *fr;

	ev = (Emotion_Vlc_Video *)ef;
	ASSERT_EV(ev) return 0;
	fr = ev->cur_frame;
	if (!fr) return 0;
	if (fr->frame)
	{
		/* wait for frame to be ready */
		pthread_mutex_lock(&fr->frame_copy_lock);
		/* send cur frame to emotion */
		*bgra_data = fr->frame;

		/* switch frames */
		if(fr->frame == fr->frame_A)	{
			fr->frame = fr->frame_B;
		} else {
			fr->frame = fr->frame_A;
		}

		/* unlock both frames */
		pthread_mutex_unlock(&fr->frame_copy_lock);
		return 1;
	}
	return 0;
}

static void _em_resize(Emotion_Vlc_Video *ev, int x,  int y)
{
	int i;
	int event;
	int vlc_argc = 0;
	char const *vlc_argv[] =
	{
		"-q",
		//"-vvvvv",
		"--ignore-config",
		"--vout", "vmem",
		"--vmem-width", ev->width,
		"--vmem-height", ev->height,
		"--vmem-pitch", ev->pitch,
		"--vmem-chroma", "RV32",
		"--vmem-lock", ev->clock,
		"--vmem-unlock", ev->cunlock,
		"--vmem-data", ev->cdata,
	};
	Emotion_Vlc_Video_Frame *fr;

	ASSERT_EV(ev)	return;
	ASSERT_EV_VLC(ev) return;
	fr = ev->cur_frame;

	/* We look for new size if not specified */
	if(x == -1)	{
		x = libvlc_video_get_width(ev->vlc_mp, ev->vlc_ex);
		CATCH(ev->vlc_ex)
	}
	if(y == -1)	{
		y = libvlc_video_get_height(ev->vlc_mp, ev->vlc_ex);
		CATCH(ev->vlc_ex)
	}

	if((x == 0)||(y == 0))	{
		return;
	}

	/* stop vlc if necessary */
	if(ev->filename)	{	
		libvlc_media_player_stop(ev->vlc_mp, ev->vlc_ex);
		CATCH(ev->vlc_ex)
		libvlc_media_player_release(ev->vlc_mp);
		CATCH(ev->vlc_ex)
	}

	/* update size in object */
	ev->w = x;
	ev->h = y;
	ev->ratio = (double)x/(double)y;

	/* we need to restart vlc */
	libvlc_release(ev->vlc_player);
	CATCH(ev->vlc_ex)
	vlc_argc = sizeof(vlc_argv) / sizeof(*vlc_argv);
	sprintf(ev->clock, "%lld", (long long int)(intptr_t)_em_lock);
	sprintf(ev->cunlock, "%lld", (long long int)(intptr_t)_em_unlock);
	sprintf(ev->cdata, "%lld", (long long int)(intptr_t)ev);
	sprintf(ev->width, "%d", ev->w);
	sprintf(ev->height, "%d", ev->h);
	sprintf(ev->pitch, "%d", 4*ev->w);
	ev->vlc_player = libvlc_new(vlc_argc, vlc_argv, ev->vlc_ex);
	CATCH(ev->vlc_ex)

	pthread_mutex_lock(&fr->frame_copy_lock);

	if(ev->filename)	{	
		/* relaunch media and mediaplayer */
		ev->vlc_m = libvlc_media_new(ev->vlc_player, ev->filename, ev->vlc_ex);
		CATCH(ev->vlc_ex)
			ASSERT_EV_M(ev) return;
		ev->vlc_mp = libvlc_media_player_new_from_media(ev->vlc_m, ev->vlc_ex);
		CATCH(ev->vlc_ex)
			libvlc_media_release(ev->vlc_m); 
		CATCH(ev->vlc_ex)
			ev->vlc_m = NULL;
		ASSERT_EV_MP(ev) return;

		ev->vlc_evtmngr = libvlc_media_player_event_manager (ev->vlc_mp, ev->vlc_ex);
		CATCH(ev->vlc_ex)

		for(i = EVENT_BEG_ID; i<EVENT_BEG_ID+NB_EVENTS; i++)	{
			libvlc_event_attach(ev->vlc_evtmngr, (libvlc_event_type_t)i, _em_event, ev, ev->vlc_ex);
			CATCH(ev->vlc_ex)
		}
	}

	/* resize frames */
	if(fr->frame_A)	{
		free(fr->frame_A);
	}
	if(fr->frame_B)	{
		free(fr->frame_B);
	}
	fr->frame_A = malloc(ev->w*ev->h*sizeof(char)*4);
	fr->frame_B = malloc(ev->w*ev->h*sizeof(char)*4);
	memset(fr->frame_A, 0xFF, x*y*sizeof(char)*4);
	memset(fr->frame_B, 0xFF, x*y*sizeof(char)*4);
	fr->frame = fr->frame_A;

	/* notify a new frame is available */
	event = VLC_RESIZE;
	write(ev->fd_write, &event, sizeof(void *));

	pthread_mutex_unlock(&fr->frame_copy_lock);

	/* unlock both frames */
	if(ev->filename)	{	
		libvlc_media_player_play(ev->vlc_mp, ev->vlc_ex);
	}

	/* activate display */
	ev->video_mute = 0;
}

static int _em_fd_active(void *data, Ecore_Fd_Handler *fdh)
{
	/* this only used for ecore notification */
	/* every other internal event should be done in _em_slave_thread */
	int event;
	int fd, len;
	Emotion_Vlc_Video_Frame *fr;
	Emotion_Vlc_Video *ev;

	ev = data;
	fd = ecore_main_fd_handler_fd_get(fdh);
	while((len = read(fd, &event, sizeof(int))) > 0) {
		if(len == sizeof(int)) {
			switch(event)	{
				case VLC_NEW_FRAME:
					ASSERT_EV(ev)	return 1;
					_emotion_frame_new(ev->obj);
				break;
				case VLC_RESIZE:
					ASSERT_EV(ev)	return 1;
					_emotion_frame_resize(ev->obj, ev->w, ev->h, ev->ratio);
				break;
				default:
				break;
			}
		}
	}
	return 1;
}


/* STUB */
static void em_event_feed(void *ef, int event)
{
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	if ((ev->opening) || (!ev->play_ok)) return;

	switch (event)
	{
		case EMOTION_EVENT_MENU1:
			break;
		case EMOTION_EVENT_MENU2:
			break;
		case EMOTION_EVENT_MENU3:
			break;
		case EMOTION_EVENT_MENU4:
			break;
		case EMOTION_EVENT_MENU5:
			break;
		case EMOTION_EVENT_MENU6:
			break;
		case EMOTION_EVENT_MENU7:
			break;
		case EMOTION_EVENT_UP:
			break;
		case EMOTION_EVENT_DOWN:
			break;
		case EMOTION_EVENT_LEFT:
			break;
		case EMOTION_EVENT_RIGHT:
			break;
		case EMOTION_EVENT_SELECT:
			break;
		case EMOTION_EVENT_NEXT:
			break;
		case EMOTION_EVENT_PREV:
			break;
		case EMOTION_EVENT_ANGLE_NEXT:
			break;
		case EMOTION_EVENT_ANGLE_PREV:
			break;
		case EMOTION_EVENT_FORCE:
			break;
		case EMOTION_EVENT_0:
			break;
		case EMOTION_EVENT_1:
			break;
		case EMOTION_EVENT_2:
			break;
		case EMOTION_EVENT_3:
			break;
		case EMOTION_EVENT_4:
			break;
		case EMOTION_EVENT_5:
			break;
		case EMOTION_EVENT_6:
			break;
		case EMOTION_EVENT_7:
			break;
		case EMOTION_EVENT_8:
			break;
		case EMOTION_EVENT_9:
			break;
		case EMOTION_EVENT_10:
			break;
		default:
			return;
			break;
	}
}

/* STUB */
static void em_event_mouse_button_feed(void *ef, int button, int x, int y)
{
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	if ((ev->opening) || (!ev->play_ok)) return;
}

/* STUB */
static void em_event_mouse_move_feed(void *ef, int x, int y)
{
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	if ((ev->opening) || (!ev->play_ok)) return;
}

static int em_video_channel_count(void *ef)
{
	int ret  = 0;
	Emotion_Vlc_Video *ev;
	ev = (Emotion_Vlc_Video *)ef; 
	ASSERT_EV(ev) return 0;
	ASSERT_EV_MP(ev) return 0;
	if (ev->opening || (!ev->play_ok)) return 0;
	ret = libvlc_media_player_has_vout(ev->vlc_mp, ev->vlc_ex);
	CATCH(ev->vlc_ex)
	return ret;
}

/* STUB */
static void em_video_channel_set(void *ef, int channel)
{
	return;
}

/* STUB */
static int em_video_channel_get(void *ef)
{
	return 1;
}

/* STUB */
static const char * em_video_channel_name_get(void *ef, int channel)
{
	Emotion_Vlc_Video *ev;
	return NULL;
}

static void em_video_channel_mute_set(void *ef, int mute)
{
	Emotion_Vlc_Video *ev;
	ev = (Emotion_Vlc_Video*)ef;
	ASSERT_EV(ev)	return;
	ev->video_mute = mute;
}

static int em_video_channel_mute_get(void *ef)
{
	Emotion_Vlc_Video *ev;
	ev = (Emotion_Vlc_Video*)ef;
	ASSERT_EV(ev)	return;
	return ev->video_mute;
}

/* STUB */
static int em_audio_channel_count(void *ef)
{
	return -1;
}

static void em_audio_channel_set(void *ef, int channel)
{
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	ASSERT_EV(ev) return;
	ASSERT_EV_VLC(ev) return;
	if (channel < -1) channel = -1;
	ev->audio_channel = channel;
	libvlc_audio_set_channel(ev->vlc_player, channel, ev->vlc_ex);
	CATCH(ev->vlc_ex)
}

static int em_audio_channel_get(void *ef)
{
	int ret = 0;
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	ASSERT_EV(ev) return 0;
	ASSERT_EV_VLC(ev) return 0;
	if (ev->opening || (!ev->play_ok)) return 0;
	ret = libvlc_audio_get_channel(ev->vlc_player, ev->vlc_ex);
	CATCH(ev->vlc_ex)
	return ret;
}

/* STUB */
static const char * em_audio_channel_name_get(void *ef, int channel)
{
	return NULL;
}

static void em_audio_channel_mute_set(void *ef, int mute)
{
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	ASSERT_EV(ev) return;
	ASSERT_EV_VLC(ev) return;
	ev->audio_mute = mute;
	libvlc_audio_set_mute(ev->vlc_player, (int)(ev->audio_mute), ev->vlc_ex);
	CATCH(ev->vlc_ex)
}

static int em_audio_channel_mute_get(void *ef)
{
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	ASSERT_EV(ev) return 0;
	return ev->audio_mute;
}

static void em_audio_channel_volume_set(void *ef, double vol)
{
	int event = VLC_CHANGE_VOL;
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	ASSERT_EV(ev) return;
	ASSERT_EV_VLC(ev) return;

	if (vol < 0.0) vol = 0.0;
	if (vol > 1.0) vol = 1.0;
	libvlc_audio_set_volume(ev->vlc_player, (int)(vol * 200), ev->vlc_ex);
	write(ev->fd_slave_write, &event, sizeof(void *));
	CATCH(ev->vlc_ex)
}

static double em_audio_channel_volume_get(void *ef)
{
	int vol;
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	ASSERT_EV(ev) return;
	ASSERT_EV_VLC(ev) return;
	if (ev->opening || (!ev->play_ok)) return 0;
	vol = libvlc_audio_get_volume(ev->vlc_player, ev->vlc_ex);
	CATCH(ev->vlc_ex)
	return (double)(vol/200.0);
}

/* STUB */
static int em_spu_channel_count(void *ef)
{
	return 0;
}

static void em_spu_channel_set(void *ef, int channel)
{
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	ASSERT_EV(ev) return;
	ASSERT_EV_MP(ev) return;
	if (channel < 0) channel = 0;
	ev->spu_channel = channel;
	libvlc_video_set_spu(ev->vlc_mp, ev->spu_channel, ev->vlc_ex);
	CATCH(ev->vlc_ex)
}

static int em_spu_channel_get(void *ef)
{
	int num = 0;
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	ASSERT_EV(ev) return 0;
	ASSERT_EV_MP(ev) return 0;
	if (ev->opening || (!ev->play_ok)) return 0;
	num = libvlc_video_get_spu(ev->vlc_mp, ev->vlc_ex);
	CATCH(ev->vlc_ex)
	return num;
}

/* STUB */
static const char * em_spu_channel_name_get(void *ef, int channel)
{
	return NULL;
}

/* STUB */
static void em_spu_channel_mute_set(void *ef, int mute)
{
	return;
}

/* STUB */
static int em_spu_channel_mute_get(void *ef)
{
	return 0;
}

static int em_chapter_count(void *ef)
{
	int num = 0;
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	ASSERT_EV(ev) return 0;
	ASSERT_EV_MP(ev) return 0;
	if (ev->opening || (!ev->play_ok)) return 0;
	num = libvlc_media_player_get_chapter_count(ev->vlc_mp, ev->vlc_ex);
	CATCH(ev->vlc_ex)
	return num;
}

static void em_chapter_set(void *ef, int chapter)
{
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	ASSERT_EV(ev) return;
	ASSERT_EV_MP(ev) return ;
	libvlc_media_player_set_chapter(ev->vlc_mp, chapter, ev->vlc_ex);
	CATCH(ev->vlc_ex)
}

static int em_chapter_get(void *ef)
{
	int num = 0;
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	ASSERT_EV(ev) return 0;
	ASSERT_EV_MP(ev) return 0;
	num = libvlc_media_player_get_chapter(ev->vlc_mp, ev->vlc_ex);
	CATCH(ev->vlc_ex)
	return num;
}

/* STUB */
static const char * em_chapter_name_get(void *ef, int chapter)
{
	return NULL;
}

static void em_speed_set(void *ef, double speed)
{
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	ASSERT_EV(ev) return;
	ASSERT_EV_MP(ev) return;
	libvlc_media_player_set_rate(ev->vlc_mp, speed, ev->vlc_ex);
	CATCH(ev->vlc_ex)
}

static double em_speed_get(void *ef)
{
	float speed = 0;
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	ASSERT_EV(ev) return 0;
	ASSERT_EV_MP(ev) return 0;
	speed = libvlc_media_player_get_rate(ev->vlc_mp, ev->vlc_ex);
	CATCH(ev->vlc_ex)
	return (double)speed;
}

static int em_eject(void *ef)
{
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	system("eject");
	return 1;
}

static const char * em_meta_get(void *ef, int meta)
{
	char * meta_data = NULL;
	Emotion_Vlc_Video *ev;

	ev = (Emotion_Vlc_Video *)ef;
	ASSERT_EV(ev)	return NULL;
	ASSERT_EV_M(ev)	return NULL;
	if ((ev->opening) || (!ev->play_ok)) return NULL;
	switch (meta)
	{
		case META_TRACK_TITLE:
			meta_data = libvlc_media_get_meta(ev->vlc_m, libvlc_meta_Title, ev->vlc_ex);
			break;
		case META_TRACK_ARTIST:
			meta_data = libvlc_media_get_meta(ev->vlc_m, libvlc_meta_Artist, ev->vlc_ex);
			break;
		case META_TRACK_GENRE:
			meta_data = libvlc_media_get_meta(ev->vlc_m, libvlc_meta_Genre, ev->vlc_ex);
			break;
		case META_TRACK_COMMENT:
			meta_data = libvlc_media_get_meta(ev->vlc_m, libvlc_meta_Description, ev->vlc_ex);
			break;
		case META_TRACK_ALBUM:
			meta_data = libvlc_media_get_meta(ev->vlc_m, libvlc_meta_Album, ev->vlc_ex);
			break;
		case META_TRACK_YEAR:
			meta_data = libvlc_media_get_meta(ev->vlc_m, libvlc_meta_Date, ev->vlc_ex);
			break;
		case META_TRACK_DISCID:
			meta_data = libvlc_media_get_meta(ev->vlc_m, libvlc_meta_TrackID, ev->vlc_ex);
			break;
		default:
			break;
	}
	return meta_data;
}

static void _em_event(const libvlc_event_t *event, void *data)
{
	Emotion_Vlc_Event *new_ev;
	Emotion_Vlc_Video *ev;

	ev = data;
	ASSERT_EV(ev)	return;
	switch (event->type) {
		case libvlc_MediaPlayerTimeChanged: {
				libvlc_time_t pos = event->u.media_player_time_changed.new_time;
				double time = (pos / 1000000.f);
				_emotion_video_pos_update(ev->obj, time, 0);
				ev->pos = time;
				if(ev->len == 0)	{
					pos = libvlc_media_player_get_length(ev->vlc_mp, ev->vlc_ex);
					ev->len = (double)(pos / 1000.0);
				}
			}
		case libvlc_MediaPlayerPlaying: {
				int x, y;
				float fps = 0;
				float pos = 0;
				float total_pos = 0;
				libvlc_time_t time = 0;
				libvlc_time_t total_time = 0;

				/* get video properties */
				total_time = libvlc_media_player_get_length(ev->vlc_mp, ev->vlc_ex);
				CATCH(ev->vlc_ex)
				fps = libvlc_media_player_get_fps(ev->vlc_mp, ev->vlc_ex);
				CATCH(ev->vlc_ex)
				x = libvlc_video_get_width(ev->vlc_mp, ev->vlc_ex);
				CATCH(ev->vlc_ex)
				y = libvlc_video_get_height(ev->vlc_mp, ev->vlc_ex);
				CATCH(ev->vlc_ex)
				/* set them to object */
				if(ev->fps == 0)	{
					ev->fps = fps;
				}
				if(ev->len == 0)	{
					ev->len = (double)(total_time / 1000.0);
				}
				if((x != 0) && (y != 0))	{
					if((ev->w != x)||(ev->h != y))	{
						int event;
						event = VLC_RESIZE;
						write(ev->fd_slave_write, &event, sizeof(void *));
					}
				}
			} break;
		case libvlc_MediaPlayerEndReached: {
				ev->play = 0;
				_emotion_decode_stop(ev->obj);
				_emotion_playback_finished(ev->obj);
			} break;
		default:
			break;
	}
}

static void *_em_lock(void *par)
{
	Emotion_Vlc_Video *ev;
	Emotion_Vlc_Video_Frame *evvf; 

	/* get current frame */
	ev = (Emotion_Vlc_Video*)par;
	evvf = ev->cur_frame;

	/* lock current frame */
	pthread_mutex_lock(&(evvf->frame_copy_lock));
	
	/* send current frame address to vlc */
	return evvf->frame;
}

static void  _em_unlock(void *par)
{
	float p = 0;
	int ret;
	int event;
	Emotion_Vlc_Video *ev;
	Emotion_Vlc_Video_Frame *evvf; 

	/* get current frame */
	ev = (Emotion_Vlc_Video*)par;
	evvf = ev->cur_frame;

	/* notify a new frame is available */
	event = VLC_NEW_FRAME;
	
	if(ev->video_mute == 0)	{
		ret = write(ev->fd_write, &event, sizeof(void *));
	}
	pthread_mutex_unlock(&(evvf->frame_copy_lock));

	return;
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
	NULL, /* yuv_rows_get */
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

unsigned char module_open(Evas_Object *obj, const Emotion_Video_Module **module, void **video, Emotion_Module_Options *opt)
{
	if (!module)	{
		return 0;
	}

	if (!em_module.init(obj, video, opt))	{
		return 0;
	}

	*module = &em_module;
	
	return 1;
}

void module_close(Emotion_Video_Module *module, void *video)
{
	em_module.shutdown(video);
}

#endif /* EMOTION_VLC_C */

