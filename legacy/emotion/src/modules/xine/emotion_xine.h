#ifndef EMOTION_XINE_H
#define EMOTION_XINE_H

#include <xine.h>
#include <xine/xine_plugin.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

typedef struct _Emotion_Xine_Video       Emotion_Xine_Video;
typedef struct _Emotion_Xine_Video_Frame Emotion_Xine_Video_Frame;
typedef struct _Emotion_Xine_Event       Emotion_Xine_Event;

struct _Emotion_Xine_Video
{
   xine_t                   *decoder;
   xine_video_port_t        *video;
   xine_audio_port_t        *audio;
   xine_stream_t            *stream;
   xine_event_queue_t       *queue;
   volatile double           len;
   volatile double           pos;
   double                    fps;
   double                    ratio;
   int                       w, h;
   Evas_Object              *obj;
   volatile Emotion_Xine_Video_Frame *cur_frame;
   volatile int              get_poslen;
   volatile int              spu_channel;
   volatile int              audio_channel;
   volatile int              video_channel;
   volatile double           seek_to_pos;
   volatile double           seeked_pos;
   volatile int              fq;
   Emotion_Vis               vis;
    int                       fd_read;
   int                       fd_write;
   Ecore_Fd_Handler         *fd_handler;
   int                       fd_ev_read;
   int                       fd_ev_write;
   Ecore_Fd_Handler         *fd_ev_handler;
   unsigned char             play : 1;
   unsigned char             just_loaded : 1;
   unsigned char             video_mute : 1;
   unsigned char             audio_mute : 1;
   unsigned char             spu_mute : 1;
   unsigned char             opt_no_video : 1;
   unsigned char             opt_no_audio : 1;
   volatile unsigned char    delete_me : 1;
   volatile unsigned char    no_time : 1;
   volatile unsigned char    opening : 1;
   volatile unsigned char    closing : 1;
   volatile unsigned char    have_vo : 1;
   volatile unsigned char    play_ok : 1;
   
   pthread_t                 get_pos_len_th;
   pthread_cond_t            get_pos_len_cond;
   pthread_mutex_t           get_pos_len_mutex;
   
   pthread_t                 slave_th;
   int                       fd_slave_read;
   int                       fd_slave_write;
   
   unsigned char             get_pos_thread_deleted : 1;
};

struct _Emotion_Xine_Video_Frame
{
   int             w, h;
   double          ratio;
   Emotion_Format  format;
   unsigned char  *y, *u, *v;
   unsigned char  *bgra_data;
   int             y_stride, u_stride, v_stride;
   Evas_Object    *obj;
   double          timestamp;
   void          (*done_func)(void *data);
   void           *done_data;
   void           *frame;
};

struct _Emotion_Xine_Event
{
   int   type;
   void *xine_event;
   int   mtype;
};

unsigned char         module_open(Evas_Object *obj, const Emotion_Video_Module **module, void **video, Emotion_Module_Options *opt);
void                  module_close(Emotion_Video_Module *module, void *video);

#endif
