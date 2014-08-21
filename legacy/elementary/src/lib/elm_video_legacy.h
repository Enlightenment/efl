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
 * @ingroup Video
 *
 * @param[in] filename The file or URI to target.
Local files can be specified using file:// or by using full file paths.
URI could be remote source of video, like http:// or local source like
WebCam (v4l2://). (You can use Emotion API to request and list
the available Webcam on your system).
 */
EAPI Eina_Bool elm_video_file_set(Eo *obj, const char *filename);

#include "elm_video.eo.legacy.h"
