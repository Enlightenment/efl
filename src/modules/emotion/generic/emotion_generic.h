#ifndef EMOTION_GENERIC_H
#define EMOTION_GENERIC_H

#include <sys/types.h>

#include "Emotion_Generic_Plugin.h"

/* default values */

typedef struct _Emotion_Generic_Video Emotion_Generic_Video;
typedef struct _Emotion_Generic_Player Emotion_Generic_Player;
typedef struct _Emotion_Generic_Cmd_Buffer Emotion_Generic_Cmd_Buffer;
typedef struct _Emotion_Generic_Channel Emotion_Generic_Channel;
typedef struct _Emotion_Generic_Meta Emotion_Generic_Meta;

struct _Emotion_Generic_Player
{
   Ecore_Exe *exe;
};

struct _Emotion_Generic_Channel
{
   int id;
   const char *name;
};

struct _Emotion_Generic_Meta
{
   const char *title;
   const char *artist;
   const char *album;
   const char *year;
   const char *genre;
   const char *comment;
   const char *disc_id;
   const char *count;
};

struct _Emotion_Generic_Cmd_Buffer
{
   char *tmp;
   int type;
   ssize_t i, total;
   int s_len;
   int num_params, cur_param;
   int padding;
   union {
      struct {
         int width;
         int height;
      } size;
      int i_num;
      float f_num;
      struct {
         int total;
         int current;
         Emotion_Generic_Channel *channels;
      } track;
      Emotion_Generic_Meta meta;
   } param;
};

typedef struct _Emotion_Engine_Generic
{
   Emotion_Engine engine;
   char *path;
} Emotion_Engine_Generic;

/* emotion/generic main structure */
struct _Emotion_Generic_Video
{
   const Emotion_Engine_Generic *engine;
   const char                   *shmname;

   Emotion_Generic_Player        player;
   Emotion_Generic_Cmd_Buffer    cmd;
   Ecore_Event_Handler          *player_add, *player_del, *player_data;
   int                           drop;
   Ecore_Pipe                   *fd_read;
   Ecore_Pipe                   *fd_write;
   const unsigned char          *buffer;
   ssize_t                       length;
   ssize_t                       offset;

   const char                   *filename;
   volatile double               len;
   volatile double               pos;
   double                        fps;
   double                        ratio;
   int                           w, h;
   Evas_Object                  *obj;
   Emotion_Generic_Video_Shared *shared;
   Emotion_Generic_Video_Frame   frame;
   volatile int                  fq;
   float                         volume;
   float                         speed;
   Emotion_Vis                   vis;
   Eina_Bool                     initializing : 1;
   Eina_Bool                     ready : 1;
   Eina_Bool                     play : 1;
   Eina_Bool                     video_mute : 1;
   Eina_Bool                     audio_mute : 1;
   Eina_Bool                     spu_mute : 1;
   Eina_Bool                     seekable : 1;
   volatile Eina_Bool            opening : 1;
   volatile Eina_Bool            closing : 1;
   Eina_Bool                     file_changed : 1;
   Eina_Bool                     file_ready : 1;
   int                           audio_channels_count;
   int                           audio_channel_current;
   Emotion_Generic_Channel      *audio_channels;
   int                           video_channels_count;
   int                           video_channel_current;
   Emotion_Generic_Channel      *video_channels;
   int                           spu_channels_count;
   int                           spu_channel_current;
   Emotion_Generic_Channel      *spu_channels;
   Emotion_Generic_Meta	         meta;
   const char                   *subtitle_path;
};

#endif

