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
 *
 * @ingroup Video
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
 *
 * @ingroup Video
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
 *
 * @ingroup Video
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
 *
 * @ingroup Video
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
 *
 * @ingroup Video
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
 *
 * @ingroup Video
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
 *
 * @ingroup Video
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
 *
 * @ingroup Video
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
 *
 * @ingroup Video
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
 *
 * @ingroup Video
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
 *
 * @ingroup Video
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
 *
 * @ingroup Video
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
 *
 * @ingroup Video
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
 *
 * @ingroup Video
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
 *
 * @ingroup Video
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
 *
 * @ingroup Video
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
 *
 * @ingroup Video
 */
#define elm_obj_video_remember_position_get(ret) ELM_OBJ_VIDEO_ID(ELM_OBJ_VIDEO_SUB_ID_REMEMBER_POSITION_GET), EO_TYPECHECK(Eina_Bool *, ret)
