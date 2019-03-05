typedef Eo Elm_Video;

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
 * @ingroup Elm_Video
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
 * @ingroup Elm_Video
 */
EAPI Evas_Object         *elm_video_add(Evas_Object *parent);

/**
 *
 * @brief Define the file or URI that will be the video source.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * This function will explicitly define a file or URI as a source
 * for the video of the Elm_Video object.
 *
 * @see elm_video_add()
 * @see elm_player_add()
 *
 * @ingroup Elm_Video
 *
 * @param[in] filename The file or URI to target.
Local files can be specified using file:// or by using full file paths.
URI could be remote source of video, like http:// or local source like
WebCam (v4l2://). (You can use Emotion API to request and list
the available Webcam on your system).
 */
EAPI Eina_Bool elm_video_file_set(Eo *obj, const char *filename);

/**
 *
 * @brief Get the file or URI that is used as the video source.
 *
 * @ingroup Elm_Video
 *
 * @param filename The file or URI.
 *
 * @since 1.14
 */
EAPI void elm_video_file_get(Eo *obj, const char **filename);

/**
 * @brief Set the audio level of an Elm_Video object.
 *
 * @param[in] volume The audio level.
 *
 * @ingroup Elm_Video
 */
EAPI void elm_video_audio_level_set(Evas_Object *obj, double volume);

/**
 * @brief Get the audio level of the current video.
 *
 * @return The audio level.
 *
 * @ingroup Elm_Video
 */
EAPI double elm_video_audio_level_get(const Evas_Object *obj);

/**
 * @brief Change the mute state of the Elm_Video object.
 *
 * @param[in] mute The mute state.
 *
 * @ingroup Elm_Video
 */
EAPI void elm_video_audio_mute_set(Evas_Object *obj, Eina_Bool mute);

/**
 * @brief Get whether audio is muted.
 *
 * @return The mute state.
 *
 * @ingroup Elm_Video
 */
EAPI Eina_Bool elm_video_audio_mute_get(const Evas_Object *obj);

/**
 * @brief Get the total playing time (in seconds) of the Elm_Video object.
 *
 * @return The total duration (in seconds) of the media file.
 *
 * @ingroup Elm_Video
 */
EAPI double elm_video_play_length_get(const Evas_Object *obj);

/**
 * @brief Is it possible to seek inside the video.
 *
 * @return true if is possible to seek inside the video.
 *
 * @ingroup Elm_Video
 */
EAPI Eina_Bool elm_video_is_seekable_get(const Evas_Object *obj);

/**
 * @brief Set the current position (in seconds) to be played in the Elm_Video
 * object.
 *
 * @param[in] position The time (in seconds) since the beginning of the media
 * file.
 *
 * @ingroup Elm_Video
 */
EAPI void elm_video_play_position_set(Evas_Object *obj, double position);

/**
 * @brief Get the current position (in seconds) being played in the Elm_Video
 * object.
 *
 * @return The time (in seconds) since the beginning of the media file.
 *
 * @ingroup Elm_Video
 */
EAPI double elm_video_play_position_get(const Evas_Object *obj);

/**
 * @brief Get whether the video actually playing.
 * You should consider watching event on the object instead of
 * polling the object state.
 *
 * @return @c true if the video is playing, @c false otherwise.
 *
 * @ingroup Elm_Video
 */
EAPI Eina_Bool elm_video_is_playing_get(Evas_Object *obj);

/**
 * @brief Start playing a video.
 *
 * @ingroup Elm_Video
 */
EAPI void elm_video_play(Evas_Object *obj);

/**
 * @brief Stop a video.
 *
 * @ingroup Elm_Video
 */
EAPI void elm_video_stop(Evas_Object *obj);

/**
 * @brief Pause a video.
 *
 * @ingroup Elm_Video
 */
EAPI void elm_video_pause(Evas_Object *obj);

#include "efl_ui_video_eo.legacy.h"
