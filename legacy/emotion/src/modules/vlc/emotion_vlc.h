/* 
 * SICMA AERO SEAT
 * code@ife-sit.info
 */

#ifndef EMOTION_VLC_H
#define EMOTION_VLC_H

#include <vlc/vlc.h>
#include <fcntl.h>
#include <pthread.h>


/* default values */
#define WIDTH		10
#define HEIGHT		10

/* internal values */
#define EVENT_BEG_ID	6
#define NB_EVENTS	14

/* checking macros */
#define ASSERT_EV(ev)		if(!ev)
#define ASSERT_EV_M(ev)		if(!ev->vlc_m)
#define ASSERT_EV_MP(ev)	if(!ev->vlc_mp)
#define ASSERT_EV_VLC(ev)	if(!ev->vlc_player)

#ifdef DEBUG
#define CATCH(ex)	if(libvlc_exception_raised(ex)){ \
				fprintf(stderr,"VLC EX [%s]::%s:%d %s\n", __FILE__,__func__,__LINE__, \
				libvlc_exception_get_message(ex)); \
			}libvlc_exception_clear(ex);
#else
#define CATCH(ex)	libvlc_exception_clear(ex);
#endif

/* internal events */
#define VLC_NEW_FRAME	1
#define VLC_RESIZE	2
#define VLC_CHANGE_VOL	3

typedef struct _Emotion_Vlc_Video       Emotion_Vlc_Video;
typedef struct _Emotion_Vlc_Video_Frame Emotion_Vlc_Video_Frame;
typedef struct _Emotion_Vlc_Event       Emotion_Vlc_Event;

/* module api EMOTION --> VLC */
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
static Evas_Bool      em_vis_supported(void *ef, Emotion_Vis vis);
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

/* entry points for module */
unsigned char         module_open(Evas_Object *obj, const Emotion_Video_Module **module, void **video, Emotion_Module_Options *opt);
void                  module_close(Emotion_Video_Module *module, void *video);

typedef struct _vlc_event_t {
	libvlc_event_type_t type;
	int data_length;
	void * data;
} vlc_event_t;

/* emotion/vlc main structure */
struct _Emotion_Vlc_Video
{
   /* vlc objects */
   libvlc_instance_t *	     vlc_player;
   libvlc_media_player_t *   vlc_mp;
   libvlc_media_t * 	     vlc_m;
   libvlc_exception_t *      vlc_ex;
   libvlc_event_manager_t *  vlc_evtmngr;
   
   /* vlc datas */
   char clock[64], cunlock[64], cdata[64];
   char width[32], height[32], pitch[32];
  
   int	                     fd_read, fd_write;
   int 			     fd_slave_read, fd_slave_write;
   Ecore_Fd_Handler	     *fd_handler;

   char 		     *filename;
   volatile double	     len;
   volatile double           pos;
   double                    fps;
   double                    ratio;
   int                       w, h;
   Evas_Object               *obj;
   Emotion_Vlc_Video_Frame   *cur_frame;
   volatile int              spu_channel;
   volatile int              audio_channel;
   volatile int              video_channel;
   volatile int              fq;
   Emotion_Vis               vis;
   unsigned char             play : 1;
   unsigned char             just_loaded : 1;
   unsigned char             video_mute : 1;
   unsigned char             audio_mute : 1;
   unsigned char             spu_mute : 1;
   unsigned char             opt_no_video : 1;
   unsigned char             opt_no_audio : 1;
   volatile unsigned char    delete_me : 1;
   volatile unsigned char    opening : 1;
   volatile unsigned char    closing : 1;
   volatile unsigned char    have_vo : 1;
   volatile unsigned char    play_ok : 1;
};

/* structure for frames 2 buffers to keep integrity */
struct _Emotion_Vlc_Video_Frame
{
   unsigned char  *frame;
   unsigned char  *frame_A;
   unsigned char  *frame_B;
   pthread_mutex_t frame_copy_lock;
};


/* internal util calls */
static void *_em_lock          (void *par);
static void  _em_unlock        (void *par);
static void  _em_event         (const libvlc_event_t *event, void* data);
static void  _em_resize        (Emotion_Vlc_Video *ev, int x,  int y);
static int   _em_fd_active     (void *data, Ecore_Fd_Handler *fdh);
static int   _em_reload_vlc    (Emotion_Vlc_Video *ev);
void*	     _em_slave_thread  (void * t);
/* internal event struct */

struct _Emotion_Vlc_Event
{
   int   type;
   int   mtype;
};

#endif

