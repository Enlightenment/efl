#ifndef _EFL_UI_VIDEO_EO_LEGACY_H_
#define _EFL_UI_VIDEO_EO_LEGACY_H_

#ifndef _EFL_UI_VIDEO_EO_CLASS_TYPE
#define _EFL_UI_VIDEO_EO_CLASS_TYPE

typedef Eo Efl_Ui_Video;

#endif

#ifndef _EFL_UI_VIDEO_EO_TYPES
#define _EFL_UI_VIDEO_EO_TYPES


#endif

/**
 * @brief Set whether the object can remember the last played position.
 *
 * @note This API only serves as indication. System support is required.
 *
 * @param[in] obj The object.
 * @param[in] remember @c true when the object can remember the last position,
 * @c false otherwise
 *
 * @ingroup Elm_Video_Group
 */
EAPI void elm_video_remember_position_set(Efl_Ui_Video *obj, Eina_Bool remember);

/**
 * @brief Set whether the object can remember the last played position.
 *
 * @note This API only serves as indication. System support is required.
 *
 * @param[in] obj The object.
 *
 * @return @c true when the object can remember the last position, @c false
 * otherwise
 *
 * @ingroup Elm_Video_Group
 */
EAPI Eina_Bool elm_video_remember_position_get(const Efl_Ui_Video *obj);

/**
 * @brief Get the underlying Emotion object.
 *
 * @param[in] obj The object.
 *
 * @return The underlying Emotion object.
 *
 * @ingroup Elm_Video_Group
 */
EAPI Efl_Canvas_Object *elm_video_emotion_get(const Efl_Ui_Video *obj);

/**
 * @brief Get the title (for instance DVD title) from this emotion object.
 *
 * This function is only useful when playing a DVD.
 *
 * @note Don't change or free the string returned by this function.
 *
 * @param[in] obj The object.
 *
 * @return A string containing the title.
 *
 * @ingroup Elm_Video_Group
 */
EAPI const char *elm_video_title_get(const Efl_Ui_Video *obj);

#endif
