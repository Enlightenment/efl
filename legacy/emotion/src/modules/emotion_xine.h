#ifndef EMOTION_XINE_H
#define EMOTION_XINE_H

#include <xine.h>

typedef struct _Emotion_Xine_Video       Emotion_Xine_Video;
typedef struct _Emotion_Xine_Video_Frame Emotion_Xine_Video_Frame;
typedef struct _Emotion_Xine_Event Emotion_Xine_Event;

struct _Emotion_Xine_Video
{
   xine_video_port_t        *video;
   xine_audio_port_t        *audio;
   xine_stream_t            *stream;
   xine_event_queue_t       *queue;
   int                       fd;
   double                    len;
   double                    pos;
   double                    fps;
   double                    ratio;
   int                       w, h;
   Evas_Object              *obj;
   Emotion_Xine_Video_Frame *cur_frame;
   int                       seek_to;
   double                    seek_to_pos;
   Ecore_Timer              *timer;
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
};

struct _Emotion_Xine_Video_Frame
{
   int            w, h;
   double         ratio;
   unsigned char *y, *u, *v;
   int            y_stride, u_stride, v_stride;
   Evas_Object   *obj;
   double         timestamp;
   void         (*done_func)(void *data);
   void          *done_data;
   void          *frame;
};

struct _Emotion_Xine_Event
{
   int type;
   char *xine_event;
};

Emotion_Video_Module *module_open (void);
void                  module_close(Emotion_Video_Module *module);

#endif
