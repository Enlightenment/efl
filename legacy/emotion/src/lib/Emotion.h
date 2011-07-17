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

/**
 * @enum _Emotion_Meta_Info
 *
 * Used for retrieving information about the media file being played.
 *
 * @see emotion_object_meta_info_get()
 *
 * @ingroup Emotion_Info
 */
enum _Emotion_Meta_Info
{
   EMOTION_META_INFO_TRACK_TITLE, /**< track title */
   EMOTION_META_INFO_TRACK_ARTIST, /**< artist name */
   EMOTION_META_INFO_TRACK_ALBUM, /**< album name */
   EMOTION_META_INFO_TRACK_YEAR, /**< track year */
   EMOTION_META_INFO_TRACK_GENRE, /**< track genre */
   EMOTION_META_INFO_TRACK_COMMENT, /**< track comments */
   EMOTION_META_INFO_TRACK_DISC_ID, /**< track disc ID */
   EMOTION_META_INFO_TRACK_COUNT /**< track count - number of the track in the album */
};

/**
 * @enum _Emotion_Vis
 *
 * Used for displaying a visualization on the emotion object.
 *
 * @see emotion_object_vis_set()
 *
 * @ingroup Emotion_Visualization
 */
enum _Emotion_Vis
{
  EMOTION_VIS_NONE, /**< no visualization set */
  EMOTION_VIS_GOOM, /**< goom */
  EMOTION_VIS_LIBVISUAL_BUMPSCOPE, /**< bumpscope */
  EMOTION_VIS_LIBVISUAL_CORONA, /**< corona */
  EMOTION_VIS_LIBVISUAL_DANCING_PARTICLES, /**< dancing particles */
  EMOTION_VIS_LIBVISUAL_GDKPIXBUF, /**< gdkpixbuf */
  EMOTION_VIS_LIBVISUAL_G_FORCE, /**< G force */
  EMOTION_VIS_LIBVISUAL_GOOM, /**< goom */
  EMOTION_VIS_LIBVISUAL_INFINITE, /**< infinite */
  EMOTION_VIS_LIBVISUAL_JAKDAW, /**< jakdaw */
  EMOTION_VIS_LIBVISUAL_JESS, /**< jess */
  EMOTION_VIS_LIBVISUAL_LV_ANALYSER, /**< lv analyser */
  EMOTION_VIS_LIBVISUAL_LV_FLOWER, /**< lv flower */
  EMOTION_VIS_LIBVISUAL_LV_GLTEST, /**< lv gltest */
  EMOTION_VIS_LIBVISUAL_LV_SCOPE, /**< lv scope */
  EMOTION_VIS_LIBVISUAL_MADSPIN, /**< madspin */
  EMOTION_VIS_LIBVISUAL_NEBULUS, /**< nebulus */
  EMOTION_VIS_LIBVISUAL_OINKSIE, /**< oinksie */
  EMOTION_VIS_LIBVISUAL_PLASMA, /**< plasma */
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
typedef enum _Emotion_Meta_Info Emotion_Meta_Info; /**< Meta info type to be retrieved. */
typedef enum _Emotion_Vis       Emotion_Vis; /**< Type of visualization. */

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
 * Emotion provides an Evas smart object that allows to play, control and
 * display a video or audio file. The API is synchronous but not everything
 * happens immediately. There are also some signals to report changed states.
 *
 * Basically, once the object is created and initialized, a file will be set to
 * it, and then it can be resized, moved, and controlled by other Evas object
 * functions.
 *
 * However, the decoding of the music and video occurs not in the Ecore main
 * loop, but usually in another thread (this depends on the module being used).
 * The synchronization between this other thread and the main loop not visible
 * to the end user of the library. The user can just register callbacks to the
 * available signals to receive information about the changed states, and can
 * call other functions from the API to request more changes on the current
 * loaded file.
 *
 * There will be a delay between an API being called and it being really
 * executed, since this request will be done in the main thread, and it needs to
 * be sent to the decoding thread. For this reason, always call functions like
 * emotion_object_size_get() or emotion_object_length_get() after some signal
 * being sent, like "playback_started" or "open_done". @ref
 * emotion_signals_example.c "This example demonstrates this behavior".
 *
 * @section signals Available signals
 * The Evas_Object returned by emotion_object_add() has a number of signals that
 * can be listened to using evas' smart callbacks mechanism. All signals have
 * NULL as event info. The following is a list of interesting signals:
 * @li "playback_started" - Emitted when the playback starts
 * @li "playback_finished" - Emitted when the playback finishes
 * @li "frame_decode" - Emitted every time a frame is decoded
 * @li "open_done" - Emitted when the media file is opened
 * @li "position_update" - Emitted when emotion_object_position_set is called
 * @li "decode_stop" - Emitted after the last frame is decoded
 *
 * @section Examples
 *
 * The following examples exemplify the emotion usage. There's also the
 * emotion_test binary that is distributed with this library and cover the
 * entire API, but since it is too long and repetitive to be explained, its code
 * is just displayed as another example.
 *
 * @li @ref emotion_basic_example_c
 * @li @ref emotion_signals_example.c "Emotion signals"
 * @li @ref emotion_test_main.c "emotion_test - full API usage"
 *
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
 * @defgroup Emotion_Visualization Visualization control functions
 */

/**
 * @defgroup Emotion_Info Miscellaneous information retrieval functions
 */

EAPI Eina_Bool emotion_init(void);
EAPI Eina_Bool emotion_shutdown(void);

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
 */
/**
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
 * The position is returned as the number of seconds since the beginning of the
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

/**
 * @brief Retrieve the video aspect ratio of the media file loaded.
 *
 * @param obj The emotion object which the video aspect ratio will be retrieved
 * from.
 * @return The video aspect ratio of the file loaded.
 *
 * This function returns the video aspect ratio (width / height) of the file
 * loaded. It can be used to adapt the size of the emotion object in the canvas,
 * so the aspect won't be changed (by wrongly resizing the object). Or to crop
 * the video correctly, if necessary.
 *
 * @note This function returns the aspect ratio that the video @b should be, but
 * sometimes the reported size from emotion_object_size_get() represents a
 * different aspect ratio. You can safely resize the video to respect the aspect
 * ratio returned by @b this function.
 *
 * @see emotion_object_size_get()
 *
 * @ingroup Emotion_Video
 */
EAPI double       emotion_object_ratio_get             (const Evas_Object *obj);

/**
 * @brief Retrieve the video size of the loaded file.
 *
 * @param obj The object from which we are retrieving the video size.
 * @param iw A pointer to a variable where the width will be stored.
 * @param iw A pointer to a variable where the height will be stored.
 *
 * This function returns the reported size of the loaded video file. If a file
 * that doesn't contain a video channel is loaded, then this size can be
 * ignored.
 *
 * The value reported by this function should be consistent with the aspect
 * ratio returned by emotion_object_ratio_get(), but sometimes the information
 * stored in the file is wrong. So use the ratio size reported by
 * emotion_object_ratio_get(), since it is more likely going to be accurate.
 *
 * @note Use @c NULL for @p iw or @p ih if you don't need one of these values.
 *
 * @see emotion_object_ratio_get()
 *
 * @ingroup Emotion_Video
 */
EAPI void         emotion_object_size_get              (const Evas_Object *obj, int *iw, int *ih);

/**
 * @brief Sets whether to use of high-quality image scaling algorithm
 * of the given video object.
 *
 * When enabled, a higher quality video scaling algorithm is used when
 * scaling videos to sizes other than the source video. This gives
 * better results but is more computationally expensive.
 *
 * @param obj The given video object.
 * @param smooth Whether to use smooth scale or not.
 *
 * @see emotion_object_smooth_scale_get()
 *
 * @ingroup Emotion_Video
 */
EAPI void         emotion_object_smooth_scale_set      (Evas_Object *obj, Eina_Bool smooth);

/**
 * @brief Gets whether the high-quality image scaling algorithm
 * of the given video object is used.
 *
 * @param obj The given video object.
 * @return Whether the smooth scale is used or not.
 *
 * @see emotion_object_smooth_scale_set()
 *
 * @ingroup Emotion_Video
 */
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

/**
 * @brief Set the mute video option for this object.
 *
 * @param obj The object which we are setting the mute video option.
 * @param mute Whether the video should be muted (@c EINA_TRUE) or not (@c
 * EINA_FALSE).
 *
 * This function sets the mute video option for this emotion object. The
 * current module used for this object can use this information to avoid
 * decoding the video portion of the loaded media file.
 *
 * @see emotion_object_video_mute_get()
 * @see emotion_object_audio_mute_set()
 *
 * @ingroup Emotion_Video
 */
EAPI void         emotion_object_video_mute_set        (Evas_Object *obj, Eina_Bool mute);

/**
 * @brief Get the mute video option of this object.
 *
 * @param obj The object which we are retrieving the mute video option from.
 * @return Whether the video is muted (@c EINA_TRUE) or not (@c EINA_FALSE).
 *
 * This function returns the mute video option from this emotion object. It can
 * be set with emotion_object_video_mute_set().
 *
 * @see emotion_object_video_mute_set()
 *
 * @ingroup Emotion_Video
 */
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

/**
 * @brief Get the dvd title from this emotion object.
 *
 * @param obj The object which the title will be retrieved from.
 * @return A string containing the title.
 *
 * This function is only useful when playing a DVD.
 *
 * @note Don't change or free the string returned by this function.
 *
 * @ingroup Emotion_Info
 */
EAPI const char  *emotion_object_title_get             (const Evas_Object *obj);
EAPI const char  *emotion_object_ref_file_get          (const Evas_Object *obj);
EAPI int          emotion_object_ref_num_get           (const Evas_Object *obj);
EAPI int          emotion_object_spu_button_count_get  (const Evas_Object *obj);
EAPI int          emotion_object_spu_button_get        (const Evas_Object *obj);

/**
 * @brief Retrieve meta information from this file being played.
 *
 * @param obj The object which the meta info will be extracted from.
 * @param meta The type of meta information that will be extracted.
 *
 * This function retrieves information about the file loaded. It can retrieve
 * the track title, artist name, album name, etc. See @ref Emotion_Meta_Info
 * for all the possibilities.
 *
 * The meta info may be not available on all types of files. It will return @c
 * NULL if the the file doesn't have meta info, or if this specific field is
 * empty.
 *
 * @note Don't change or free the string returned by this function.
 *
 * @see Emotion_Meta_Info
 *
 * @ingroup Emotion_Info
 */
EAPI const char  *emotion_object_meta_info_get         (const Evas_Object *obj, Emotion_Meta_Info meta);

/**
 * @brief Set the visualization to be used with this object.
 *
 * @param obj The object where the visualization will be set on.
 * @param visualization The type of visualization to be used.
 *
 * The @p visualization specified will be played instead of a video. This is
 * commonly used to display a visualization for audio only files (musics).
 *
 * The available visualizations are @ref Emotion_Vis.
 *
 * @see Emotion_Vis
 * @see emotion_object_vis_get()
 * @see emotion_object_vis_supported()
 *
 * @ingroup Emotion_Visualization
 */
EAPI void         emotion_object_vis_set               (Evas_Object *obj, Emotion_Vis visualization);

/**
 * @brief Get the type of visualization in use by this emotion object.
 *
 * @param obj The emotion object which the visualization is being retrieved
 * from.
 * @return The type of visualization in use by this object.
 *
 * The type of visualization can be set by emotion_object_vis_set().
 *
 * @see Emotion_Vis
 * @see emotion_object_vis_set()
 * @see emotion_object_vis_supported()
 *
 * @ingroup Emotion_Visualization
 */
EAPI Emotion_Vis  emotion_object_vis_get               (const Evas_Object *obj);

/**
 * @brief Query whether a type of visualization is supported by this object.
 *
 * @param obj The object which the query is being ran on.
 * @param visualization The type of visualization that is being queried.
 * @return EINA_TRUE if the visualization is supported, EINA_FALSE otherwise.
 *
 * This can be used to check if a visualization is supported. e.g.: one wants to
 * display a list of available visualizations for a specific object.
 *
 * @see Emotion_Vis
 * @see emotion_object_vis_set()
 * @see emotion_object_vis_get()
 *
 * @ingroup Emotion_Visualization
 */
EAPI Eina_Bool    emotion_object_vis_supported         (const Evas_Object *obj, Emotion_Vis visualization);

EAPI void         emotion_object_last_position_load    (Evas_Object *obj);
EAPI void         emotion_object_last_position_save    (Evas_Object *obj);

EAPI void         emotion_object_suspend_set           (Evas_Object *obj, Emotion_Suspend state);
EAPI Emotion_Suspend emotion_object_suspend_get        (Evas_Object *obj);

EAPI Eina_Bool    emotion_object_extension_may_play_fast_get(const char *file);
EAPI Eina_Bool    emotion_object_extension_may_play_get(const char *file);

typedef struct _Emotion_Webcam Emotion_Webcam;

EAPI const Eina_List *emotion_webcams_get(void);
EAPI const char      *emotion_webcam_name_get(Emotion_Webcam *ew);
EAPI const char      *emotion_webcam_device_get(Emotion_Webcam *ew);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
