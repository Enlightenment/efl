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

#include "elm_video.eo.legacy.h"