#ifndef EMOTION_H
#define EMOTION_H

#include <Evas.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EMOTION_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EMOTION_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

enum _Emotion_Module
{
  EMOTION_MODULE_XINE,
  EMOTION_MODULE_GSTREAMER
};

enum _Emotion_Event
{
   EMOTION_EVENT_MENU1, // Escape Menu
   EMOTION_EVENT_MENU2, // Title Menu
   EMOTION_EVENT_MENU3, // Root Menu
   EMOTION_EVENT_MENU4, // Subpicture Menu
   EMOTION_EVENT_MENU5, // Audio Menu
   EMOTION_EVENT_MENU6, // Angle Menu
   EMOTION_EVENT_MENU7, // Part Menu
   EMOTION_EVENT_UP,
   EMOTION_EVENT_DOWN,
   EMOTION_EVENT_LEFT,
   EMOTION_EVENT_RIGHT,
   EMOTION_EVENT_SELECT,
   EMOTION_EVENT_NEXT,
   EMOTION_EVENT_PREV,
   EMOTION_EVENT_ANGLE_NEXT,
   EMOTION_EVENT_ANGLE_PREV,
   EMOTION_EVENT_FORCE,
   EMOTION_EVENT_0,
   EMOTION_EVENT_1,
   EMOTION_EVENT_2,
   EMOTION_EVENT_3,
   EMOTION_EVENT_4,
   EMOTION_EVENT_5,
   EMOTION_EVENT_6,
   EMOTION_EVENT_7,
   EMOTION_EVENT_8,
   EMOTION_EVENT_9,
   EMOTION_EVENT_10
};

enum _Emotion_Meta_Info
{
   EMOTION_META_INFO_TRACK_TITLE,
   EMOTION_META_INFO_TRACK_ARTIST,
   EMOTION_META_INFO_TRACK_ALBUM,
   EMOTION_META_INFO_TRACK_YEAR,
   EMOTION_META_INFO_TRACK_GENRE,
   EMOTION_META_INFO_TRACK_COMMENT,
   EMOTION_META_INFO_TRACK_DISC_ID,
   EMOTION_META_INFO_TRACK_COUNT
};

enum _Emotion_Vis
{
  EMOTION_VIS_NONE,
  EMOTION_VIS_GOOM,
  EMOTION_VIS_LIBVISUAL_BUMPSCOPE,
  EMOTION_VIS_LIBVISUAL_CORONA,
  EMOTION_VIS_LIBVISUAL_DANCING_PARTICLES,
  EMOTION_VIS_LIBVISUAL_GDKPIXBUF,
  EMOTION_VIS_LIBVISUAL_G_FORCE,
  EMOTION_VIS_LIBVISUAL_GOOM,
  EMOTION_VIS_LIBVISUAL_INFINITE,
  EMOTION_VIS_LIBVISUAL_JAKDAW,
  EMOTION_VIS_LIBVISUAL_JESS,
  EMOTION_VIS_LIBVISUAL_LV_ANALYSER,
  EMOTION_VIS_LIBVISUAL_LV_FLOWER,
  EMOTION_VIS_LIBVISUAL_LV_GLTEST,
  EMOTION_VIS_LIBVISUAL_LV_SCOPE,
  EMOTION_VIS_LIBVISUAL_MADSPIN,
  EMOTION_VIS_LIBVISUAL_NEBULUS,
  EMOTION_VIS_LIBVISUAL_OINKSIE,
  EMOTION_VIS_LIBVISUAL_PLASMA,
  EMOTION_VIS_LAST /* sentinel */
};

typedef enum _Emotion_Module    Emotion_Module;
typedef enum _Emotion_Event     Emotion_Event;
typedef enum _Emotion_Meta_Info Emotion_Meta_Info;
typedef enum _Emotion_Vis       Emotion_Vis;

#define EMOTION_CHANNEL_AUTO -1
#define EMOTION_CHANNEL_DEFAULT 0

#ifdef __cplusplus
extern "C" {
#endif
   
/* api calls available */
EAPI Evas_Object *emotion_object_add                   (Evas *evas);
EAPI void         emotion_object_module_option_set     (Evas_Object *obj, const char *opt, const char *val);
EAPI Eina_Bool    emotion_object_init                  (Evas_Object *obj, const char *module_filename);
EAPI Eina_Bool    emotion_object_file_set              (Evas_Object *obj, const char *filename);
EAPI const char  *emotion_object_file_get              (const Evas_Object *obj);
EAPI void         emotion_object_play_set              (Evas_Object *obj, Eina_Bool play);
EAPI Eina_Bool    emotion_object_play_get              (const Evas_Object *obj);
EAPI void         emotion_object_position_set          (Evas_Object *obj, double sec);
EAPI double       emotion_object_position_get          (const Evas_Object *obj);
EAPI Eina_Bool    emotion_object_video_handled_get     (const Evas_Object *obj);
EAPI Eina_Bool    emotion_object_audio_handled_get     (const Evas_Object *obj);
EAPI Eina_Bool    emotion_object_seekable_get          (const Evas_Object *obj);
EAPI double       emotion_object_play_length_get       (const Evas_Object *obj);
EAPI void         emotion_object_size_get              (const Evas_Object *obj, int *iw, int *ih);
EAPI void         emotion_object_smooth_scale_set      (Evas_Object *obj, Eina_Bool smooth);
EAPI Eina_Bool    emotion_object_smooth_scale_get      (const Evas_Object *obj);
EAPI double       emotion_object_ratio_get             (const Evas_Object *obj);
EAPI void         emotion_object_event_simple_send     (Evas_Object *obj, Emotion_Event ev);
EAPI void         emotion_object_audio_volume_set      (Evas_Object *obj, double vol);
EAPI double       emotion_object_audio_volume_get      (const Evas_Object *obj);
EAPI void         emotion_object_audio_mute_set        (Evas_Object *obj, Eina_Bool mute);
EAPI Eina_Bool    emotion_object_audio_mute_get        (const Evas_Object *obj);
EAPI int          emotion_object_audio_channel_count   (const Evas_Object *obj);
EAPI const char  *emotion_object_audio_channel_name_get(const Evas_Object *obj, int channel);
EAPI void         emotion_object_audio_channel_set     (Evas_Object *obj, int channel);
EAPI int          emotion_object_audio_channel_get     (const Evas_Object *obj);
EAPI void         emotion_object_video_mute_set        (Evas_Object *obj, Eina_Bool mute);
EAPI Eina_Bool    emotion_object_video_mute_get        (const Evas_Object *obj);
EAPI int          emotion_object_video_channel_count   (const Evas_Object *obj);
EAPI const char  *emotion_object_video_channel_name_get(const Evas_Object *obj, int channel);
EAPI void         emotion_object_video_channel_set     (Evas_Object *obj, int channel);
EAPI int          emotion_object_video_channel_get     (const Evas_Object *obj);
EAPI void         emotion_object_spu_mute_set          (Evas_Object *obj, Eina_Bool mute);
EAPI Eina_Bool    emotion_object_spu_mute_get          (const Evas_Object *obj);
EAPI int          emotion_object_spu_channel_count     (const Evas_Object *obj);
EAPI const char  *emotion_object_spu_channel_name_get  (const Evas_Object *obj, int channel);
EAPI void         emotion_object_spu_channel_set       (Evas_Object *obj, int channel);
EAPI int          emotion_object_spu_channel_get       (const Evas_Object *obj);
EAPI int          emotion_object_chapter_count         (const Evas_Object *obj);
EAPI void         emotion_object_chapter_set           (Evas_Object *obj, int chapter);
EAPI int          emotion_object_chapter_get           (const Evas_Object *obj);
EAPI const char  *emotion_object_chapter_name_get      (const Evas_Object *obj, int chapter);
EAPI void         emotion_object_play_speed_set        (Evas_Object *obj, double speed);
EAPI double       emotion_object_play_speed_get        (const Evas_Object *obj);
EAPI void         emotion_object_eject                 (Evas_Object *obj);
EAPI const char  *emotion_object_title_get             (const Evas_Object *obj);
EAPI const char  *emotion_object_progress_info_get     (const Evas_Object *obj);
EAPI double       emotion_object_progress_status_get   (const Evas_Object *obj);
EAPI const char  *emotion_object_ref_file_get          (const Evas_Object *obj);
EAPI int          emotion_object_ref_num_get           (const Evas_Object *obj);
EAPI int          emotion_object_spu_button_count_get  (const Evas_Object *obj);
EAPI int          emotion_object_spu_button_get        (const Evas_Object *obj);
EAPI const char  *emotion_object_meta_info_get         (const Evas_Object *obj, Emotion_Meta_Info meta);

EAPI void         emotion_object_vis_set               (Evas_Object *obj, Emotion_Vis visualization);
EAPI Emotion_Vis  emotion_object_vis_get               (const Evas_Object *obj);
EAPI Eina_Bool    emotion_object_vis_supported         (const Evas_Object *obj, Emotion_Vis visualization);

EAPI Eina_Bool    emotion_object_extension_can_play_get(const Evas_Object *obj, const char *file);
EAPI Eina_Bool    emotion_object_extension_can_play_fast_get(const Evas_Object *obj, const char *file);

EAPI Eina_Bool    emotion_object_extension_may_play_fast_get(const char *file);
EAPI Eina_Bool    emotion_object_extension_may_play_get(const char *file);

#ifdef __cplusplus
}
#endif

#endif
