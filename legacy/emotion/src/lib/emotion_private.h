#ifndef EMOTION_PRIVATE_H
#define EMOTION_PRIVATE_H

#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Job.h>

#include "config.h"

typedef struct _Emotion_Video_Module Emotion_Video_Module;

struct _Emotion_Video_Module
{
   int          (*init) (void);
   int          (*shutdown) (void);
   void *       (*file_open) (const char *file, Evas_Object *obj);
   void         (*file_close) (void *ef);
   void         (*play) (void *ef, double pos);
   void         (*stop) (void *ef);
   void         (*size_get) (void *ef, int *w, int *h);
   void         (*pos_set) (void *ef, double pos);
   double       (*len_get) (void *ef);
   double       (*fps_get) (void *ef);
   double       (*pos_get) (void *ef);
   double       (*ratio_get) (void *ef);
   int          (*seekable) (void *ef);
   void         (*frame_done) (void *ef);
   void         (*yuv_size_get) (void *ef, int *w, int *h);
   int          (*yuv_rows_get) (void *ef, int w, int h, unsigned char **yrows, unsigned char **urows, unsigned char **vrows);
   void         (*event_feed) (void *ef, int event);
   void         (*event_mouse_button_feed) (void *ef, int button, int x, int y);
   void         (*event_mouse_move_feed) (void *ef, int x, int y);
   int          (*video_channel_count) (void *ef);
   void         (*video_channel_set) (void *ef, int channel);
   int          (*video_channel_get) (void *ef);
   const char * (*video_channel_name_get) (void *ef, int channel);
   void         (*video_channel_mute_set) (void *ef, int mute);
   int          (*video_channel_mute_get) (void *ef);
   int          (*audio_channel_count) (void *ef);
   void         (*audio_channel_set) (void *ef, int channel);
   int          (*audio_channel_get) (void *ef);
   const char * (*audio_channel_name_get) (void *ef, int channel);
   void         (*audio_channel_mute_set) (void *ef, int mute);
   int          (*audio_channel_mute_get) (void *ef);
   void         (*audio_channel_volume_set) (void *ef, double vol);
   double       (*audio_channel_volume_get) (void *ef);
   int          (*spu_channel_count) (void *ef);
   void         (*spu_channel_set) (void *ef, int channel);
   int          (*spu_channel_get) (void *ef);
   const char * (*spu_channel_name_get) (void *ef, int channel);
   void         (*spu_channel_mute_set) (void *ef, int mute);
   int          (*spu_channel_mute_get) (void *ef);
   int          (*chapter_count) (void *ef);
   void         (*chapter_set) (void *ef, int chapter);
   int          (*chapter_get) (void *ef);
   const char * (*chapter_name_get) (void *ef, int chapter);
   void         (*speed_set) (void *ef, double speed);
   double       (*speed_get) (void *ef);
   int          (*eject) (void *ef);
   
   void          *handle;
};

void *_emotion_video_get(Evas_Object *obj);
void  _emotion_frame_new(Evas_Object *obj);
void  _emotion_video_pos_update(Evas_Object *obj, double pos, double len);
void  _emotion_frame_resize(Evas_Object *obj, int w, int h, double ratio);

#endif
