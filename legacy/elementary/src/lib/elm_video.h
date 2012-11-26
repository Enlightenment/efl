/**
 * @defgroup Video Video
 * @ingroup Elementary
 *
 * @addtogroup Video
 * @{
 *
 * @image html video_inheritance_tree.png
 * @image latex video_inheritance_tree.eps
 *
 * @image html player_inheritance_tree.png
 * @image latex player_inheritance_tree.eps
 *
 * Elementary comes with two object that help design application that need
 * to display video.
 *
 * The first one, Elm_Video, display a video by using Emotion.
 * It embeds the video inside an Edje object, so you can do some
 * animation depending on the video state change. It also implements a
 * resource management policy to remove this burden from the application.
 *
 * The second one,
 * Elm_Player is a video player that need to be linked with an Elm_Video.
 * It take care of updating its content according to Emotion event and provide a
 * way to theme itself. It also automatically raises the priority of the
 * linked Elm_Video so it will use the video decoder, if available. It also
 * activates the "remember" function on the linked Elm_Video object.
 *
 * Both widgets inherit from the @ref Layout one, so that all the
 * functions acting on it also work for video objects.
 *
 * The player widget emits the following signals, besides the ones
 * sent from @ref Layout:
 *  - @c "forward,clicked" - the user clicked the forward button.
 *  - @c "info,clicked" - the user clicked the info button.
 *  - @c "next,clicked" - the user clicked the next button.
 *  - @c "pause,clicked" - the user clicked the pause button.
 *  - @c "play,clicked" - the user clicked the play button.
 *  - @c "prev,clicked" - the user clicked the prev button.
 *  - @c "rewind,clicked" - the user clicked the rewind button.
 *  - @c "stop,clicked" - the user clicked the stop button.
 *
 * Default content parts of the player widget that you can use for are:
 * @li "video" - A video of the player
 *
 */

/**
 * ELM_OBJ_PLAYER_CLASS
 */
#define ELM_OBJ_PLAYER_CLASS elm_obj_player_class_get()

const Eo_Class *elm_obj_player_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_PLAYER_BASE_ID;

enum
{
   ELM_OBJ_PLAYER_SUB_ID_LAST
};

#define ELM_OBJ_PLAYER_ID(sub_id) (ELM_OBJ_PLAYER_BASE_ID + sub_id)

/**
 * ELM_OBJ_VIDEO_CLASS
 */
#define ELM_OBJ_VIDEO_CLASS elm_obj_video_class_get()

const Eo_Class *elm_obj_video_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_VIDEO_BASE_ID;

enum
{
   ELM_OBJ_VIDEO_SUB_ID_FILE_SET,
   ELM_OBJ_VIDEO_SUB_ID_EMOTION_GET,
   ELM_OBJ_VIDEO_SUB_ID_PLAY,
   ELM_OBJ_VIDEO_SUB_ID_PAUSE,
   ELM_OBJ_VIDEO_SUB_ID_STOP,
   ELM_OBJ_VIDEO_SUB_ID_IS_PLAYING_GET,
   ELM_OBJ_VIDEO_SUB_ID_IS_SEEKABLE_GET,
   ELM_OBJ_VIDEO_SUB_ID_AUDIO_MUTE_GET,
   ELM_OBJ_VIDEO_SUB_ID_AUDIO_MUTE_SET,
   ELM_OBJ_VIDEO_SUB_ID_AUDIO_LEVEL_GET,
   ELM_OBJ_VIDEO_SUB_ID_AUDIO_LEVEL_SET,
   ELM_OBJ_VIDEO_SUB_ID_PLAY_POSITION_GET,
   ELM_OBJ_VIDEO_SUB_ID_PLAY_POSITION_SET,
   ELM_OBJ_VIDEO_SUB_ID_PLAY_LENGTH_GET,
   ELM_OBJ_VIDEO_SUB_ID_TITLE_GET,
   ELM_OBJ_VIDEO_SUB_ID_REMEMBER_POSITION_SET,
   ELM_OBJ_VIDEO_SUB_ID_REMEMBER_POSITION_GET,
   ELM_OBJ_VIDEO_SUB_ID_LAST
};

#define ELM_OBJ_VIDEO_ID(sub_id) (ELM_OBJ_VIDEO_BASE_ID + sub_id)


/**
 * @def elm_obj_video_file_set
 * @since 1.8
 *
 * @brief Define the file or URI that will be the video source.
 *
 * @param[in] filename
 * @param[out] ret
 *
 * @see elm_video_file_set
 */
#define elm_obj_video_file_set(filename, ret) ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_FILE_SET), EO_TYPECHECK(const char *, filename), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_video_emotion_get
 * @since 1.8
 *
 * @brief Get the underlying Emotion object.
 *
 * @param[out] ret
 *
 * @see elm_video_emotion_get
 */
#define elm_obj_video_emotion_get(ret) ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_EMOTION_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def elm_obj_video_play
 * @since 1.8
 *
 * @brief Start to play the video
 *
 *
 * @see elm_video_play
 */
#define elm_obj_video_play() ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_PLAY)

/**
 * @def elm_obj_video_pause
 * @since 1.8
 *
 * @brief Pause the video
 *
 *
 * @see elm_video_pause
 */
#define elm_obj_video_pause() ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_PAUSE)

/**
 * @def elm_obj_video_stop
 * @since 1.8
 *
 * @brief Stop the video
 *
 *
 * @see elm_video_stop
 */
#define elm_obj_video_stop() ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_STOP)

/**
 * @def elm_obj_video_is_playing_get
 * @since 1.8
 *
 * @brief Is the video actually playing.
 *
 * @param[out] ret
 *
 * @see elm_video_is_playing_get
 */
#define elm_obj_video_is_playing_get(ret) ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_IS_PLAYING_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_video_is_seekable_get
 * @since 1.8
 *
 * @brief Is it possible to seek inside the video.
 *
 * @param[out] ret
 *
 * @see elm_video_is_seekable_get
 */
#define elm_obj_video_is_seekable_get(ret) ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_IS_SEEKABLE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_video_audio_mute_get
 * @since 1.8
 *
 * @brief Is the audio muted.
 *
 * @param[out] ret
 *
 * @see elm_video_audio_mute_get
 */
#define elm_obj_video_audio_mute_get(ret) ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_AUDIO_MUTE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_video_audio_mute_set
 * @since 1.8
 *
 * @brief Change the mute state of the Elm_Video object.
 *
 * @param[in] mute
 *
 * @see elm_video_audio_mute_set
 */
#define elm_obj_video_audio_mute_set(mute) ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_AUDIO_MUTE_SET), EO_TYPECHECK(Eina_Bool, mute)

/**
 * @def elm_obj_video_audio_level_get
 * @since 1.8
 *
 * @brief Get the audio level of the current video.
 *
 * @param[out] ret
 *
 * @see elm_video_audio_level_get
 */
#define elm_obj_video_audio_level_get(ret) ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_AUDIO_LEVEL_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_video_audio_level_set
 * @since 1.8
 *
 * @brief Set the audio level of an Elm_Video object.
 *
 * @param[in] volume
 *
 * @see elm_video_audio_level_set
 */
#define elm_obj_video_audio_level_set(volume) ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_AUDIO_LEVEL_SET), EO_TYPECHECK(double, volume)

/**
 * @def elm_obj_video_play_position_get
 * @since 1.8
 *
 * @brief Get the current position (in seconds) being played in the
 * Elm_Video object.
 *
 * @param[out] ret
 *
 * @see elm_video_play_position_get
 */
#define elm_obj_video_play_position_get(ret) ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_PLAY_POSITION_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_video_play_position_set
 * @since 1.8
 *
 * @brief Set the current position (in seconds) to be played in the
 * Elm_Video object.
 *
 * @param[in] position
 *
 * @see elm_video_play_position_set
 */
#define elm_obj_video_play_position_set(position) ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_PLAY_POSITION_SET), EO_TYPECHECK(double, position)

/**
 * @def elm_obj_video_play_length_get
 * @since 1.8
 *
 * @brief Get the total playing time (in seconds) of the Elm_Video object.
 *
 * @param[out] ret
 *
 * @see elm_video_play_length_get
 */
#define elm_obj_video_play_length_get(ret) ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_PLAY_LENGTH_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_video_title_get
 * @since 1.8
 *
 * @brief Get the title (for instance DVD title) from this emotion object.
 *
 * @param[out] ret
 *
 * @see elm_video_title_get
 */
#define elm_obj_video_title_get(ret) ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_TITLE_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_video_remember_position_set
 * @since 1.8
 *
 * @brief Set whether the object can remember the last played position.
 *
 * @param[in] remember
 *
 * @see elm_video_remember_position_set
 */
#define elm_obj_video_remember_position_set(remember) ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_REMEMBER_POSITION_SET), EO_TYPECHECK(Eina_Bool, remember)

/**
 * @def elm_obj_video_remember_position_get
 * @since 1.8
 *
 * @brief Set whether the object can remember the last played position.
 *
 * @param[out] ret
 *
 * @see elm_video_remember_position_get
 */
#define elm_obj_video_remember_position_get(ret) ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_REMEMBER_POSITION_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @brief Add a new Elm_Player object to the given parent Elementary (container) object.
 *
 * @param parent The parent object
 * @return a new player widget handle or @c NULL, on errors.
 *
 * This function inserts a new player widget on the canvas.
 *
 * @see elm_object_part_content_set()
 *
 * @ingroup Video
 */
EAPI Evas_Object         *elm_player_add(Evas_Object *parent);

/**
 * @brief Add a new Elm_Video object to the given parent Elementary (container) object.
 *
 * @param parent The parent object
 * @return a new video widget handle or @c NULL, on errors.
 *
 * This function inserts a new video widget on the canvas.
 *
 * @see elm_video_file_set()
 *
 * @ingroup Video
 */
EAPI Evas_Object         *elm_video_add(Evas_Object *parent);

/**
 * @brief Define the file or URI that will be the video source.
 *
 * @param video The video object to define the file or URI for the video
 * of the Elm_Video object.
 *
 * @param filename The file or URI to target.
 * Local files can be specified using file:// or by using full file paths.
 * URI could be remote source of video, like http:// or local source like
 * WebCam (v4l2://). (You can use Emotion API to request and list
 * the available Webcam on your system).
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * This function will explicitly define a file or URI as a source
 * for the video of the Elm_Video object.
 *
 * @see elm_video_add()
 * @see elm_player_add()
 *
 * @ingroup Video
 */
EAPI Eina_Bool            elm_video_file_set(Evas_Object *video, const char *filename);

/**
 * @brief Get the underlying Emotion object.
 *
 * @param video The video object to proceed the request on.
 * @return the underlying Emotion object.
 *
 * @ingroup Video
 */
EAPI Evas_Object         *elm_video_emotion_get(const Evas_Object *video);

/**
 * @brief Start to play the video
 *
 * @param video The video object to proceed the request on.
 *
 * Start to play the video and cancel all suspend state.
 *
 * @ingroup Video
 */
EAPI void                 elm_video_play(Evas_Object *video);

/**
 * @brief Pause the video
 *
 * @param video The video object to proceed the request on.
 *
 * Pause the video and start a timer to trigger suspend mode.
 *
 * @ingroup Video
 */
EAPI void                 elm_video_pause(Evas_Object *video);

/**
 * @brief Stop the video
 *
 * @param video The video object to proceed the request on.
 *
 * Stop the video and put the emotion in deep sleep mode.
 *
 * @ingroup Video
 */
EAPI void                 elm_video_stop(Evas_Object *video);

/**
 * @brief Is the video actually playing.
 *
 * @param video The video object to proceed the request on.
 * @return @c EINA_TRUE if the video is actually playing.
 *
 * You should consider watching event on the object instead of polling
 * the object state.
 *
 * @ingroup Video
 */
EAPI Eina_Bool            elm_video_is_playing_get(const Evas_Object *video);

/**
 * @brief Is it possible to seek inside the video.
 *
 * @param video The video object to proceed the request on.
 * @return @c EINA_TRUE if is possible to seek inside the video.
 *
 * @ingroup Video
 */
EAPI Eina_Bool            elm_video_is_seekable_get(const Evas_Object *video);

/**
 * @brief Is the audio muted.
 *
 * @param video The video object to proceed the request on.
 * @return @c EINA_TRUE if the audio is muted.
 *
 * @ingroup Video
 */
EAPI Eina_Bool            elm_video_audio_mute_get(const Evas_Object *video);

/**
 * @brief Change the mute state of the Elm_Video object.
 *
 * @param video The video object to proceed the request on.
 * @param mute The new mute state.
 *
 * @ingroup Video
 */
EAPI void                 elm_video_audio_mute_set(Evas_Object *video, Eina_Bool mute);

/**
 * @brief Get the audio level of the current video.
 *
 * @param video The video object to proceed the request on.
 * @return the current audio level.
 *
 * @ingroup Video
 */
EAPI double               elm_video_audio_level_get(const Evas_Object *video);

/**
 * @brief Set the audio level of an Elm_Video object.
 *
 * @param video The video object to proceed the request on.
 * @param volume The new audio volume.
 *
 * @ingroup Video
 */
EAPI void                 elm_video_audio_level_set(Evas_Object *video, double volume);

/**
 * @brief Get the current position (in seconds) being played in the
 * Elm_Video object.
 *
 * @param video The video object.
 * @return The time (in seconds) since the beginning of the media file.
 *
 * @ingroup Video
 */
EAPI double               elm_video_play_position_get(const Evas_Object *video);

/**
 * @brief Set the current position (in seconds) to be played in the
 * Elm_Video object.
 *
 * @param video The video object.
 * @param position The time (in seconds) since the beginning of the media file.
 *
 * @ingroup Video
 */
EAPI void                 elm_video_play_position_set(Evas_Object *video, double position);
/**
 * @brief Get the total playing time (in seconds) of the Elm_Video object.
 *
 * @param video The video object.
 * @return The total duration (in seconds) of the media file.
 *
 * @ingroup Video
 */
EAPI double               elm_video_play_length_get(const Evas_Object *video);

/**
 * @brief Set whether the object can remember the last played position.
 *
 * @param video The video object.
 * @param remember the last played position of the Elm_Video object.
 *
 * @note This API only serves as indication. System support is required.
 *
 * @ingroup Video
 */
EAPI void                 elm_video_remember_position_set(Evas_Object *video, Eina_Bool remember);

/**
 * @brief Set whether the object can remember the last played position.
 *
 * @param video The video object.
 * @return whether the object remembers the last played position (@c EINA_TRUE)
 * or not.
 *
 * @note This API only serves as indication. System support is required.
 *
 * @ingroup Video
 */
EAPI Eina_Bool            elm_video_remember_position_get(const Evas_Object *video);

/**
 * @brief Get the title (for instance DVD title) from this emotion object.
 *
 * @param video The Elm_Video object.
 * @return A string containing the title.
 *
 * This function is only useful when playing a DVD.
 *
 * @note Don't change or free the string returned by this function.
 *
 * @ingroup Video
 */
EAPI const char          *elm_video_title_get(const Evas_Object *video);
/**
 * @}
 */
