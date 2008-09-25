#ifndef EMOTION_PRIVATE_H
#define EMOTION_PRIVATE_H

#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Job.h>
#include <Ecore_Data.h>
#include <Ecore_Str.h>
#include <Emotion.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

#define META_TRACK_TITLE 1
#define META_TRACK_ARTIST 2
#define META_TRACK_GENRE 3
#define META_TRACK_COMMENT 4
#define META_TRACK_ALBUM 5
#define META_TRACK_YEAR 6
#define META_TRACK_DISCID 7
#define META_TRACK_COUNT 8

typedef enum _Emotion_Format Emotion_Format;
typedef struct _Emotion_Video_Module Emotion_Video_Module;
typedef struct _Emotion_Module_Options Emotion_Module_Options;

enum _Emotion_Format
{
   EMOTION_FORMAT_NONE,
   EMOTION_FORMAT_I420,
   EMOTION_FORMAT_YV12,
   EMOTION_FORMAT_YUY2,     /* unused for now since evas does not support yuy2 format */
   EMOTION_FORMAT_BGRA
};

struct _Emotion_Module_Options
{
   unsigned char no_video : 1;
   unsigned char no_audio : 1;
};

struct _Emotion_Video_Module
{
   unsigned char  (*init) (Evas_Object *obj, void **video, Emotion_Module_Options *opt);
   int            (*shutdown) (void *video);
   unsigned char  (*file_open) (const char *file, Evas_Object *obj, void *video);
   void           (*file_close) (void *ef);
   void           (*play) (void *ef, double pos);
   void           (*stop) (void *ef);
   void           (*size_get) (void *ef, int *w, int *h);
   void           (*pos_set) (void *ef, double pos);
   double         (*len_get) (void *ef);
   int            (*fps_num_get) (void *ef);
   int            (*fps_den_get) (void *ef);
   double         (*fps_get) (void *ef);
   double         (*pos_get) (void *ef);
   void           (*vis_set) (void *ef, Emotion_Vis vis);
   Emotion_Vis    (*vis_get) (void *ef);
   Evas_Bool      (*vis_supported) (void *ef, Emotion_Vis vis);
   double         (*ratio_get) (void *ef);
   int            (*video_handled) (void *ef);
   int            (*audio_handled) (void *ef);
   int            (*seekable) (void *ef);
   void           (*frame_done) (void *ef);
   Emotion_Format (*format_get) (void *ef);
   void           (*video_data_size_get) (void *ef, int *w, int *h);
   int            (*yuv_rows_get) (void *ef, int w, int h, unsigned char **yrows, unsigned char **urows, unsigned char **vrows);
   int            (*bgra_data_get) (void *ef, unsigned char **bgra_data);
   void           (*event_feed) (void *ef, int event);
   void           (*event_mouse_button_feed) (void *ef, int button, int x, int y);
   void           (*event_mouse_move_feed) (void *ef, int x, int y);
   int            (*video_channel_count) (void *ef);
   void           (*video_channel_set) (void *ef, int channel);
   int            (*video_channel_get) (void *ef);
   const char *   (*video_channel_name_get) (void *ef, int channel);
   void           (*video_channel_mute_set) (void *ef, int mute);
   int            (*video_channel_mute_get) (void *ef);
   int            (*audio_channel_count) (void *ef);
   void           (*audio_channel_set) (void *ef, int channel);
   int            (*audio_channel_get) (void *ef);
   const char *   (*audio_channel_name_get) (void *ef, int channel);
   void           (*audio_channel_mute_set) (void *ef, int mute);
   int            (*audio_channel_mute_get) (void *ef);
   void           (*audio_channel_volume_set) (void *ef, double vol);
   double         (*audio_channel_volume_get) (void *ef);
   int            (*spu_channel_count) (void *ef);
   void           (*spu_channel_set) (void *ef, int channel);
   int            (*spu_channel_get) (void *ef);
   const char *   (*spu_channel_name_get) (void *ef, int channel);
   void           (*spu_channel_mute_set) (void *ef, int mute);
   int            (*spu_channel_mute_get) (void *ef);
   int            (*chapter_count) (void *ef);
   void           (*chapter_set) (void *ef, int chapter);
   int            (*chapter_get) (void *ef);
   const char *   (*chapter_name_get) (void *ef, int chapter);
   void           (*speed_set) (void *ef, double speed);
   double         (*speed_get) (void *ef);
   int            (*eject) (void *ef);
   const char *   (*meta_get) (void *ef, int meta);

   Ecore_Plugin    *plugin;
   Ecore_Path_Group *path_group;
};

EAPI void *_emotion_video_get(Evas_Object *obj);
EAPI void  _emotion_frame_new(Evas_Object *obj);
EAPI void  _emotion_video_pos_update(Evas_Object *obj, double pos, double len);
EAPI void  _emotion_frame_resize(Evas_Object *obj, int w, int h, double ratio);
EAPI void  _emotion_decode_stop(Evas_Object *obj);
EAPI void  _emotion_playback_finished(Evas_Object *obj);
EAPI void  _emotion_audio_level_change(Evas_Object *obj);
EAPI void  _emotion_channels_change(Evas_Object *obj);
EAPI void  _emotion_title_set(Evas_Object *obj, char *title);
EAPI void  _emotion_progress_set(Evas_Object *obj, char *info, double stat);
EAPI void  _emotion_file_ref_set(Evas_Object *obj, char *file, int num);
EAPI void  _emotion_spu_button_num_set(Evas_Object *obj, int num);
EAPI void  _emotion_spu_button_set(Evas_Object *obj, int button);

#endif
