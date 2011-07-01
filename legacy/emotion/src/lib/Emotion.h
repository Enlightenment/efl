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

/**
 * @file Emotion.h
 * @brief The file that provides Emotion the API, with functions available for
 *        play, seek, change volume, etc.
 */

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

typedef enum
{
  EMOTION_WAKEUP,
  EMOTION_SLEEP,
  EMOTION_DEEP_SLEEP,
  EMOTION_HIBERNATE
} Emotion_Suspend;

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

/**
 * @brief How to create, initialize, manipulate and connect to signals of an
 * Emotion object.
 * @defgroup Emotion_API API available for manipulating Emotion object.
 *
 * @{
 *
 * @li Add the description of modules here.
 * @li Basic emotion example
 * @li Signals available
 */

/**
 * @defgroup Emotion_Init Creation and initialization functions
 */

/**
 * @defgroup Emotion_Audio Audio control functions
 */

/**
 * @defgroup Emotion_Video Video control functions
 */

/**
 * @brief Add an emotion object to the canvas.
 *
 * @param evas The canvas where the object will be added to.
 * @return The emotion object just created.
 *
 * This function creates an emotion object and adds it to the specified @p evas.
 * The returned object can be manipulated as any other Evas object, using the
 * default object manipulation functions - evas_object_*.
 *
 * After creating the object with this function, it's still necessary to
 * initialize it with emotion_object_init(), and if an audio file is going to be
 * played with this object instead of a video, use
 * emotion_object_video_mute_set().
 *
 * The next step is to open the desired file with emotion_object_file_set(), and
 * start playing it with emotion_object_play_set().
 *
 * @see emotion_object_init()
 * @see emotion_object_video_mute_set()
 * @see emotion_object_file_set()
 * @see emotion_object_play_set()
 *
 * @ingroup Emotion_Init
 */
EAPI Evas_Object *emotion_object_add                   (Evas *evas);

/**
 * @brief Set the specified option for the current module.
 *
 * @param obj The emotion object which the option is being set to.
 * @param opt The option that is being set. Currently supported optiosn: "video"
 * and "audio".
 * @param val The value of the option. Currently only supports "off" (?!?!?!)
 *
 * This function allows one to mute the video or audio of the emotion object.
 *
 * @note Please don't use this function, consider using
 * emotion_object_audio_mute_set() and emotion_object_video_mute_set() instead.
 *
 * @see emotion_object_audio_mute_set()
 * @see emotion_object_video_mute_set()
 *
 * @ingroup Emotion_Audio
 * @ingroup Emotion_Video
 */
EAPI void         emotion_object_module_option_set     (Evas_Object *obj, const char *opt, const char *val);

/**
 * @brief Initializes an emotion object with the specified module.
 *
 * @param obj The emotion object to be initialized.
 * @param module_filename The name of the module to be used (gstreamer or xine).
 * @return @c EINA_TRUE if the specified module was successfully initialized for
 * this object, @c EINA_FALSE otherwise.
 *
 * This function is required after creating the emotion object, in order to
 * specify which module will be used with this object. Different objects can
 * use different modules to play a media file. The current supported modules are
 * @b gstreamer and @b xine.
 *
 * To use any of them, you need to make sure that support for them was compiled
 * correctly.
 *
 * @note It's possible to disable the build of a module with
 * --disable-module_name.
 *
 * @see emotion_object_add()
 * @see emotion_object_file_set()
 *
 * @ingroup Emotion_Init
 */
EAPI Eina_Bool    emotion_object_init                  (Evas_Object *obj, const char *module_filename);

/**
 * @brief Set the file to be played in the Emotion object.
 *
 * @param obj The emotion object where the file is being loaded.
 * @param filename Path to the file to be loaded. It can be absolute or relative
 * path.
 * @return EINA_TRUE if the new file could be loaded successfully, and
 * EINA_FALSE if the file could not be loaded. This happens when the filename is
 * could not be found, when the module couldn't open the file, when no module is
 * initialized in this object, or when the @p filename is the same as the
 * one previously set.
 *
 * This function sets the file to be used with this emotion object. If the
 * object already has another file set, this file will be unset and unloaded,
 * and the new file will be loaded to this emotion object. The seek position
 * will be set to 0, and the emotion object will be paused, instead of playing.
 *
 * If there was already a filename set, and it's the same as the one being set
 * now, this function does nothing and returns EINA_FALSE.
 *
 * Use @c NULL as argument to @p filename if you want to unload the current file
 * but don't want to load anything else.
 *
 * @see emotion_object_init()
 * @see emotion_object_play_set()
 * @see emotion_object_file_get()
 *
 * @ingroup Emotion_Init
 */
EAPI Eina_Bool    emotion_object_file_set              (Evas_Object *obj, const char *filename);

/**
 * @brief Get the filename of the file associated with the emotion object.
 *
 * @param obj The emotion object from which the filename will be retrieved.
 * @return The path to the file loaded into this emotion object.
 *
 * This function returns the path of the file loaded in this emotion object. If
 * no object is loaded, it will return @c NULL.
 *
 * @note Don't free or change the string returned by this function in any way.
 * If you want to unset it, use @c emotion_object_file_set(obj, NULL).
 *
 * @see emotion_object_file_set()
 *
 * @ingroup Emotion_Init
 */
EAPI const char  *emotion_object_file_get              (const Evas_Object *obj);
/**
 * @defgroup Emotion_Play Play control functions
 *
 * @{
 *
 * @brief Set play/pause state of the media file.
 *
 * @param obj The emotion object whose state will be changed.
 * @param play EINA_TRUE to play, EINA_FALSE to pause.
 *
 * This functions sets the currently playing status of the video. Using this
 * function to play or pause the video doesn't alter it's current position.
 */
EAPI void         emotion_object_play_set              (Evas_Object *obj, Eina_Bool play);
/**
 * @brief Get play/pause state of the media file.
 *
 * @param obj The emotion object from which the state will be retrieved.
 * @return EINA_TRUE if playing. EINA_FALSE if not playing.
 */
EAPI Eina_Bool    emotion_object_play_get              (const Evas_Object *obj);
/**
 * @brief Set the position in the media file.
 *
 * @param obj The emotion object whose position will be changed.
 * @param sec The position(in seconds) to which the media file will be set.
 *
 * This functions sets the current position of the media file to @p sec, this
 * only works on seekable streams. Setting the position doesn't change the
 * playing state of the media file.
 *
 * @see emotion_object_seekable_get
 */
EAPI void         emotion_object_position_set          (Evas_Object *obj, double sec);
/**
 * @brief Get the position in the media file.
 *
 * @param obj The emotion object from which the position will be retrieved.
 * @return The position of the media file.
 *
 * The position is returned as the number of seconds since the beggining of the
 * media file.
 */
EAPI double       emotion_object_position_get          (const Evas_Object *obj);
/**
 * @brief Get whether the media file is seekable.
 *
 * @param obj The emotion object from which the seekable status will be
 * retrieved.
 * @return EINA_TRUE if the media file is seekable, EINA_FALSE otherwise.
 */
EAPI Eina_Bool    emotion_object_seekable_get          (const Evas_Object *obj);
/**
 * @brief Get the length of play for the media file.
 *
 * @param obj The emotion object from which the length will be retrieved.
 * @return The length of the media file in seconds.
 *
 * This function returns the length of the media file in seconds.
 *
 * @warning This will return 0 if called before the "length_change" signal has,
 * been emitted.
 */
EAPI double       emotion_object_play_length_get       (const Evas_Object *obj);
/**
 * @brief Gets the native aspect ratio of the media file.
 *
 * @param obj The emotion object from which the ratio will be retrieved.
 * @return The aspect ration of the video.
 *
 * This function returns the aspect ratio of the video file being played. The
 * aspect is width divided by height.
 *
 * @warning For audio files this function returns 0.
 */
EAPI double       emotion_object_ratio_get             (const Evas_Object *obj);
/**
 * @brief Set the play speed of the media file.
 *
 * @param obj The emotion object whose speed will be set.
 * @param speed The speed to be set in the range [0,infinity)
 *
 * This function sets the speed with which the media file will be played. 1.0
 * represents the normal speed, 2 double speed, 0.5 half speed and so on.
 *
 * @warning The only backend that implements this is the experimental VLC
 * backend.
 */
EAPI void         emotion_object_play_speed_set        (Evas_Object *obj, double speed);
/**
 * @brief Get  the play speed of the media file.
 *
 * @param obj The emotion object from which the filename will be retrieved.
 * @return The current speed of the media file.
 *
 * @see emotion_object_play_speed_set
 */
EAPI double       emotion_object_play_speed_get        (const Evas_Object *obj);
/**
 * @brief Get how much of the file has been played.
 *
 * @param obj The emotion object from which the filename will be retrieved.
 * @return The progress of the media file.
 *
 * @warning Don't change of free the returned string.
 * @warning gstreamer xine backends don't implement this(will return NULL).
 */
EAPI const char  *emotion_object_progress_info_get     (const Evas_Object *obj);
/**
 * @brief Get how much of the file has been played.
 *
 * @param obj The emotion object from which the filename will be retrieved
 * @return The progress of the media file.
 *
 * This function gets the progress in playing the file, the return value is in
 * the [0, 1] range.
 *
 * @warning gstreamer xine backends don't implement this(will return 0).
 */
EAPI double       emotion_object_progress_status_get   (const Evas_Object *obj);
/**
 * @}
 */
EAPI Eina_Bool    emotion_object_video_handled_get     (const Evas_Object *obj);
EAPI Eina_Bool    emotion_object_audio_handled_get     (const Evas_Object *obj);
EAPI void         emotion_object_size_get              (const Evas_Object *obj, int *iw, int *ih);
EAPI void         emotion_object_smooth_scale_set      (Evas_Object *obj, Eina_Bool smooth);
EAPI Eina_Bool    emotion_object_smooth_scale_get      (const Evas_Object *obj);
EAPI void         emotion_object_event_simple_send     (Evas_Object *obj, Emotion_Event ev);

/**
 * @brief Set the audio volume.
 *
 * @param obj The object where the volume is being set.
 * @param vol The new volume parameter. Range is from 0.0 to 1.0.
 *
 * Sets the audio volume of the stream being played. This has nothing to do with
 * the system volume. This volume will be multiplied by the system volume. e.g.:
 * if the current volume level is 0.5, and the system volume is 50%, it will be
 * 0.5 * 0.5 = 0.25.
 *
 * The default value depends on the module used. This value doesn't get changed
 * when another file is loaded.
 *
 * @see emotion_object_audio_volume_get()
 *
 * @ingroup Emotion_Audio
 */
EAPI void         emotion_object_audio_volume_set      (Evas_Object *obj, double vol);

/**
 * @brief Get the audio volume.
 *
 * @param obj The object from which we are retrieving the volume.
 * @return The current audio volume level for this object.
 *
 * Get the current value for the audio volume level. Range is from 0.0 to 1.0.
 * This volume is set with emotion_object_audio_volume_set().
 *
 * @see emotion_object_audio_volume_set()
 *
 * @ingroup Emotion_Audio
 */
EAPI double       emotion_object_audio_volume_get      (const Evas_Object *obj);

/**
 * @brief Set the mute audio option for this object.
 *
 * @param obj The object which we are setting the mute audio option.
 * @param mute Whether the audio should be muted (@c EINA_TRUE) or not (@c
 * EINA_FALSE).
 *
 * This function sets the mute audio option for this emotion object. The current
 * module used for this object can use this to avoid decoding the audio portion
 * of the loaded media file.
 *
 * @see emotion_object_audio_mute_get()
 * @see emotion_object_video_mute_set()
 *
 * @ingroup Emotion_Audio
 */
EAPI void         emotion_object_audio_mute_set        (Evas_Object *obj, Eina_Bool mute);

/**
 * @brief Get the mute audio option of this object.
 *
 * @param obj The object which we are retrieving the mute audio option from.
 * @return Whether the audio is muted (@c EINA_TRUE) or not (@c EINA_FALSE).
 *
 * This function return the mute audio option from this emotion object. It can
 * be set with emotion_object_audio_mute_set().
 *
 * @see emotion_object_audio_mute_set()
 *
 * @ingroup Emotion_Audio
 */
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
EAPI void         emotion_object_eject                 (Evas_Object *obj);
EAPI const char  *emotion_object_title_get             (const Evas_Object *obj);
EAPI const char  *emotion_object_ref_file_get          (const Evas_Object *obj);
EAPI int          emotion_object_ref_num_get           (const Evas_Object *obj);
EAPI int          emotion_object_spu_button_count_get  (const Evas_Object *obj);
EAPI int          emotion_object_spu_button_get        (const Evas_Object *obj);
EAPI const char  *emotion_object_meta_info_get         (const Evas_Object *obj, Emotion_Meta_Info meta);

EAPI void         emotion_object_vis_set               (Evas_Object *obj, Emotion_Vis visualization);
EAPI Emotion_Vis  emotion_object_vis_get               (const Evas_Object *obj);
EAPI Eina_Bool    emotion_object_vis_supported         (const Evas_Object *obj, Emotion_Vis visualization);

EAPI void         emotion_object_last_position_load    (Evas_Object *obj);
EAPI void         emotion_object_last_position_save    (Evas_Object *obj);

EAPI void         emotion_object_suspend_set           (Evas_Object *obj, Emotion_Suspend state);
EAPI Emotion_Suspend emotion_object_suspend_get        (Evas_Object *obj);

EAPI Eina_Bool    emotion_object_extension_can_play_get(const Evas_Object *obj, const char *file);
EAPI Eina_Bool    emotion_object_extension_can_play_fast_get(const Evas_Object *obj, const char *file);

EAPI Eina_Bool    emotion_object_extension_may_play_fast_get(const char *file);
EAPI Eina_Bool    emotion_object_extension_may_play_get(const char *file);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
