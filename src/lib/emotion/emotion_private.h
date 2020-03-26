#ifndef EMOTION_PRIVATE_H
#define EMOTION_PRIVATE_H

#include <Evas.h>
#include <Eet.h>
#include "Emotion.h"

Eina_Bool emotion_webcam_init(void);
void emotion_webcam_shutdown(void);

Eina_Bool emotion_modules_init(void);
void emotion_modules_shutdown(void);

extern int _emotion_log_domain;
extern Eina_Prefix *_emotion_pfx;

#define DBG(...) EINA_LOG_DOM_DBG(_emotion_log_domain, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_emotion_log_domain, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_emotion_log_domain, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_emotion_log_domain, __VA_ARGS__)
#define CRI(...) EINA_LOG_DOM_CRIT(_emotion_log_domain, __VA_ARGS__)

#include "emotion_modules.h"

typedef struct _Emotion_Engine_Instance Emotion_Engine_Instance;

Emotion_Engine_Instance *emotion_engine_instance_new(const char *name, Evas_Object *obj, Emotion_Module_Options *opts);
void                     emotion_engine_instance_del(Emotion_Engine_Instance *inst);
Eina_Bool                emotion_engine_instance_name_equal(const Emotion_Engine_Instance *inst, const char *name);
void                    *emotion_engine_instance_data_get(const Emotion_Engine_Instance *inst);

Eina_Bool      emotion_engine_instance_file_open(Emotion_Engine_Instance *inst, const char *file);
void           emotion_engine_instance_file_close(Emotion_Engine_Instance *inst);
void           emotion_engine_instance_play(Emotion_Engine_Instance *inst, double pos);
void           emotion_engine_instance_stop(Emotion_Engine_Instance *inst);
void           emotion_engine_instance_size_get(const Emotion_Engine_Instance *inst, int *w, int *h);
void           emotion_engine_instance_pos_set(Emotion_Engine_Instance *inst, double pos);
double         emotion_engine_instance_len_get(const Emotion_Engine_Instance *inst);
double         emotion_engine_instance_buffer_size_get(const Emotion_Engine_Instance *inst);
int            emotion_engine_instance_fps_num_get(const Emotion_Engine_Instance *inst);
int            emotion_engine_instance_fps_den_get(const Emotion_Engine_Instance *inst);
double         emotion_engine_instance_fps_get(const Emotion_Engine_Instance *inst);
double         emotion_engine_instance_pos_get(const Emotion_Engine_Instance *inst);
void           emotion_engine_instance_vis_set(Emotion_Engine_Instance *inst, Emotion_Vis vis);
Emotion_Vis    emotion_engine_instance_vis_get(const Emotion_Engine_Instance *inst);
Eina_Bool      emotion_engine_instance_vis_supported(Emotion_Engine_Instance *inst, Emotion_Vis vis);
double         emotion_engine_instance_ratio_get(const Emotion_Engine_Instance *inst);
Eina_Bool      emotion_engine_instance_video_handled(Emotion_Engine_Instance *inst);
Eina_Bool      emotion_engine_instance_audio_handled(Emotion_Engine_Instance *inst);
Eina_Bool      emotion_engine_instance_seekable(Emotion_Engine_Instance *inst);
void           emotion_engine_instance_frame_done(Emotion_Engine_Instance *inst);
Emotion_Format emotion_engine_instance_format_get(const Emotion_Engine_Instance *inst);
void           emotion_engine_instance_video_data_size_get(const Emotion_Engine_Instance *inst, int *w, int *h);
Eina_Bool      emotion_engine_instance_yuv_rows_get(const Emotion_Engine_Instance *inst, int w, int h, unsigned char **yrows, unsigned char **urows, unsigned char **vrows);
Eina_Bool      emotion_engine_instance_bgra_data_get(const Emotion_Engine_Instance *inst, unsigned char **bgra_data);
void           emotion_engine_instance_event_feed(Emotion_Engine_Instance *inst, int event);
void           emotion_engine_instance_event_mouse_button_feed(Emotion_Engine_Instance *inst, int button, int x, int y);
void           emotion_engine_instance_event_mouse_move_feed(Emotion_Engine_Instance *inst, int x, int y);
int            emotion_engine_instance_video_channel_count(Emotion_Engine_Instance *inst);
void           emotion_engine_instance_video_channel_set(Emotion_Engine_Instance *inst, int channel);
int            emotion_engine_instance_video_channel_get(const Emotion_Engine_Instance *inst);
void           emotion_engine_instance_video_subtitle_file_set(Emotion_Engine_Instance *inst, const char *filepath);
const char *   emotion_engine_instance_video_subtitle_file_get(const Emotion_Engine_Instance *inst);
const char *   emotion_engine_instance_video_channel_name_get(const Emotion_Engine_Instance *inst, int channel);
void           emotion_engine_instance_video_channel_mute_set(Emotion_Engine_Instance *inst, Eina_Bool mute);
Eina_Bool      emotion_engine_instance_video_channel_mute_get(const Emotion_Engine_Instance *inst);
int            emotion_engine_instance_audio_channel_count(Emotion_Engine_Instance *inst);
void           emotion_engine_instance_audio_channel_set(Emotion_Engine_Instance *inst, int channel);
int            emotion_engine_instance_audio_channel_get(const Emotion_Engine_Instance *inst);
const char *   emotion_engine_instance_audio_channel_name_get(const Emotion_Engine_Instance *inst, int channel);
void           emotion_engine_instance_audio_channel_mute_set(Emotion_Engine_Instance *inst, Eina_Bool mute);
Eina_Bool      emotion_engine_instance_audio_channel_mute_get(const Emotion_Engine_Instance *inst);
void           emotion_engine_instance_audio_channel_volume_set(Emotion_Engine_Instance *inst, double vol);
double         emotion_engine_instance_audio_channel_volume_get(const Emotion_Engine_Instance *inst);
int            emotion_engine_instance_spu_channel_count(Emotion_Engine_Instance *inst);
void           emotion_engine_instance_spu_channel_set(Emotion_Engine_Instance *inst, int channel);
int            emotion_engine_instance_spu_channel_get(const Emotion_Engine_Instance *inst);
const char *   emotion_engine_instance_spu_channel_name_get(const Emotion_Engine_Instance *inst, int channel);
void           emotion_engine_instance_spu_channel_mute_set(Emotion_Engine_Instance *inst, Eina_Bool mute);
Eina_Bool      emotion_engine_instance_spu_channel_mute_get(const Emotion_Engine_Instance *inst);
int            emotion_engine_instance_chapter_count(Emotion_Engine_Instance *inst);
void           emotion_engine_instance_chapter_set(Emotion_Engine_Instance *inst, int chapter);
int            emotion_engine_instance_chapter_get(const Emotion_Engine_Instance *inst);
const char *   emotion_engine_instance_chapter_name_get(const Emotion_Engine_Instance *inst, int chapter);
void           emotion_engine_instance_speed_set(Emotion_Engine_Instance *inst, double speed);
double         emotion_engine_instance_speed_get(const Emotion_Engine_Instance *inst);
Eina_Bool      emotion_engine_instance_eject(Emotion_Engine_Instance *inst);
const char *   emotion_engine_instance_meta_get(const Emotion_Engine_Instance *inst, int meta);
void           emotion_engine_instance_priority_set(Emotion_Engine_Instance *inst, Eina_Bool priority);
Eina_Bool      emotion_engine_instance_priority_get(const Emotion_Engine_Instance *inst);
void       *   emotion_engine_instance_meta_artwork_get(const Emotion_Engine_Instance *inst, Evas_Object *img, const char *path, Emotion_Artwork_Info type);
#endif
