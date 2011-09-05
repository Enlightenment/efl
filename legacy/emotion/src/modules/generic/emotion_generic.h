#ifndef EMOTION_GENERIC_H
#define EMOTION_GENERIC_H

#include "Emotion_Generic_Plugin.h"

/* default values */

typedef struct _Emotion_Generic_Video       Emotion_Generic_Video;
typedef struct _Emotion_Generic_Player       Emotion_Generic_Player;
typedef struct _Emotion_Generic_Audio_Channel Emotion_Generic_Audio_Channel;

struct _Emotion_Generic_Player
{
   Ecore_Exe *exe;
};

struct _Emotion_Generic_Audio_Channel
{
   int id;
   const char *name;
};

/* emotion/generic main structure */
struct _Emotion_Generic_Video
{
   const char		     *cmdline;
   const char		     *shmname;

   Emotion_Generic_Player    player;
   Ecore_Event_Handler	     *player_add, *player_del, *player_data;
   int			     drop;
   int			     fd_read, fd_write;
   Ecore_Fd_Handler	     *fd_handler;

   const char		     *filename;
   volatile double	     len;
   volatile double           pos;
   double                    fps;
   double                    ratio;
   int                       w, h;
   Evas_Object               *obj;
   Emotion_Generic_Video_Shared *shared;
   Emotion_Generic_Video_Frame frame;
   volatile int              spu_channel;
   volatile int              video_channel;
   volatile int              fq;
   int			     volume;
   float		     speed;
   Emotion_Vis               vis;
   Eina_Bool		     initializing : 1;
   Eina_Bool		     ready : 1;
   Eina_Bool                 play : 1;
   Eina_Bool                 video_mute : 1;
   Eina_Bool                 audio_mute : 1;
   Eina_Bool                 spu_mute : 1;
   Eina_Bool		     seekable : 1;
   volatile Eina_Bool        opening : 1;
   volatile Eina_Bool        closing : 1;
   Eina_Bool		     file_changed : 1;
   int			     audio_channels_count;
   int			     audio_channel_current;
   struct _Emotion_Generic_Audio_Channel *audio_channels;
};

#endif

