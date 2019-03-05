#ifndef _EFL_CANVAS_VIDEO_EO_LEGACY_H_
#define _EFL_CANVAS_VIDEO_EO_LEGACY_H_

#ifndef _EFL_CANVAS_VIDEO_EO_CLASS_TYPE
#define _EFL_CANVAS_VIDEO_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Video;

#endif

#ifndef _EFL_CANVAS_VIDEO_EO_TYPES
#define _EFL_CANVAS_VIDEO_EO_TYPES


#endif

/**
 * @brief Set the specified option for the current module.
 *
 * This function allows one to mute the video or audio of the emotion object.
 *
 * Please don't use this function, consider using @ref Efl.Player.mute instead.
 *
 * @param[in] obj The object.
 * @param[in] opt The option that is being set. Currently supported options:
 * "video" and "audio".
 * @param[in] val The value of the option. Currently only supports "off"
 * (?!?!?!)
 *
 * @ingroup (null)_Group
 */
EAPI void emotion_object_module_option_set(Efl_Canvas_Video *obj, const char *opt, const char *val);

/**
 * @brief Initializes an emotion object with the specified module.
 *
 * This function is required after creating the emotion object, in order to
 * specify which module will be used with this object. Different objects can
 * use different modules to play a media file. The current supported modules
 * are gstreamer and xine.
 *
 * To use any of them, you need to make sure that support for them was compiled
 * correctly.
 *
 * It's possible to disable the build of a module with --disable-module_name.
 *
 * See also @ref Efl.File.file.
 *
 * @param[in] obj The object.
 * @param[in] module_filename The name of the module to be used (gstreamer or
 * xine).
 *
 * @return @c true if the specified module was successfully initialized for
 * this object, @c false otherwise.
 *
 * @ingroup (null)_Group
 */
EAPI Eina_Bool emotion_object_init(Efl_Canvas_Video *obj, const char *module_filename);

#endif
