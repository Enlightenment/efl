/** Used to set if a video thumbnail is animating or not
 *
 * @ingroup Elm_Thumb
 */
typedef enum
{
  ELM_THUMB_ANIMATION_START = 0, /** Play animation once */
  ELM_THUMB_ANIMATION_LOOP, /** Keep playing animation until stop is requested
                             */
  ELM_THUMB_ANIMATION_STOP, /** Stop playing the animation */
  ELM_THUMB_ANIMATION_LAST
} Elm_Thumb_Animation_Setting;

/**
 * Add a new thumb object to the parent.
 *
 * @param parent The parent object.
 * @return The new object or NULL if it cannot be created.
 *
 * @see elm_thumb_file_set()
 * @see elm_thumb_ethumb_client_get()
 *
 * @ingroup Elm_Thumb
 */
EAPI Evas_Object                *elm_thumb_add(Evas_Object *parent);

/**
 *
 * Set the file that will be used as thumbnail @b source.
 *
 * The file can be an image or a video (in that case, acceptable
 * extensions are: avi, mp4, ogv, mov, mpg and wmv). To start the
 * video animation, use the function elm_thumb_animate().
 *
 * @see elm_thumb_file_get()
 * @see elm_thumb_reload()
 * @see elm_thumb_animate()
 *
 * @ingroup Elm_Thumb
 *
 * @param[in] file The path to file that will be used as thumbnail source.
 * @param[in] key The key used in case of an EET file.
 */
EAPI void elm_thumb_file_set(Eo *obj, const char *file, const char *key);

/**
 *
 * Get the image or video path and key used to generate the thumbnail.
 *
 * @see elm_thumb_file_set()
 * @see elm_thumb_path_get()
 *
 * @ingroup Elm_Thumb
 *
 * @param[out] file The path to file that will be used as thumbnail source.
 * @param[out] key The key used in case of an EET file.
 */
EAPI void elm_thumb_file_get(const Eo *obj, const char **file, const char **key);

/**
 * @brief Make the thumbnail 'editable'.
 *
 * This means the thumbnail is a valid drag target for drag and drop, and can
 * be cut or pasted too.
 *
 * @param[in] edit The editable state, default is @c false.
 *
 * @ingroup Elm_Thumb
 */
EAPI Eina_Bool elm_thumb_editable_set(Evas_Object *obj, Eina_Bool edit);

/**
 * @brief Get whether the thumbnail is editable.
 *
 * This means the thumbnail is a valid drag target for drag and drop, and can
 * be cut or pasted too.
 *
 * @return The editable state, default is @c false.
 *
 * @ingroup Elm_Thumb
 */
EAPI Eina_Bool elm_thumb_editable_get(const Evas_Object *obj);

/**
 * @brief Set the compression for the thumb object.
 *
 * @param[in] compress The compression of the thumb.
 *
 * @since 1.8
 *
 * @ingroup Elm_Thumb
 */
EAPI void elm_thumb_compress_set(Evas_Object *obj, int compress);

/**
 * @brief Get the compression of the thumb object.
 *
 * @param[out] compress The compression of the thumb.
 *
 * @return Force the return type to be sure the argument doesn't become the
 * return
 *
 * @since 1.8
 *
 * @ingroup Elm_Thumb
 */
EAPI void elm_thumb_compress_get(const Evas_Object *obj, int *compress);

/**
 * @brief Set the format for the thumb object.
 *
 * @param[in] format The format setting.
 *
 * @since 1.8
 *
 * @ingroup Elm_Thumb
 */
EAPI void elm_thumb_format_set(Evas_Object *obj, Ethumb_Thumb_Format format);

/**
 * @brief Get the format of the thumb object.
 *
 * @return The format setting.
 *
 * @since 1.8
 *
 * @ingroup Elm_Thumb
 */
EAPI Ethumb_Thumb_Format elm_thumb_format_get(const Evas_Object *obj);

/**
 * @brief Set the animation state for the thumb object.
 *
 * If its content is an animated video, you may start/stop the animation or
 * tell it o play continuously and looping.
 *
 * @param[in] setting The animation setting or #ELM_THUMB_ANIMATION_LAST
 *
 * @ingroup Elm_Thumb
 */
EAPI void elm_thumb_animate_set(Evas_Object *obj, Elm_Thumb_Animation_Setting setting);

/**
 * @brief Get the animation state for the thumb object.
 *
 * @return The animation setting or #ELM_THUMB_ANIMATION_LAST
 *
 * @ingroup Elm_Thumb
 */
EAPI Elm_Thumb_Animation_Setting elm_thumb_animate_get(const Evas_Object *obj);

/**
 * @brief Set the FDO size for the thumb object.
 *
 * @param[in] size The FDO size setting.
 *
 * @since 1.8
 *
 * @ingroup Elm_Thumb
 */
EAPI void elm_thumb_fdo_size_set(Evas_Object *obj, Ethumb_Thumb_FDO_Size size);

/**
 * @brief Get the fdo size of the thumb object.
 *
 * @return The FDO size setting.
 *
 * @since 1.8
 *
 * @ingroup Elm_Thumb
 */
EAPI Ethumb_Thumb_FDO_Size elm_thumb_fdo_size_get(const Evas_Object *obj);

/**
 * @brief Set the orientation for the thumb object.
 *
 * @param[in] orient The orientation setting.
 *
 * @since 1.8
 *
 * @ingroup Elm_Thumb
 */
EAPI void elm_thumb_orientation_set(Evas_Object *obj, Ethumb_Thumb_Orientation orient);

/**
 * @brief Get the orientation of the thumb object.
 *
 * @return The orientation setting.
 *
 * @since 1.8
 *
 * @ingroup Elm_Thumb
 */
EAPI Ethumb_Thumb_Orientation elm_thumb_orientation_get(const Evas_Object *obj);

/**
 * @brief Set the aspect for the thumb object.
 *
 * @param[in] aspect The aspect setting.
 *
 * @since 1.8
 *
 * @ingroup Elm_Thumb
 */
EAPI void elm_thumb_aspect_set(Evas_Object *obj, Ethumb_Thumb_Aspect aspect);

/**
 * @brief Get the aspect of the thumb object.
 *
 * @return The aspect setting.
 *
 * @since 1.8
 *
 * @ingroup Elm_Thumb
 */
EAPI Ethumb_Thumb_Aspect elm_thumb_aspect_get(const Evas_Object *obj);

/**
 * @brief Set the quality for the thumb object.
 *
 * @param[in] quality The quality of the thumb.
 *
 * @since 1.8
 *
 * @ingroup Elm_Thumb
 */
EAPI void elm_thumb_quality_set(Evas_Object *obj, int quality);

/**
 * @brief Get the quality of the thumb object.
 *
 * @param[out] quality The quality of the thumb.
 *
 * @since 1.8
 *
 * @ingroup Elm_Thumb
 */
EAPI void elm_thumb_quality_get(const Evas_Object *obj, int *quality);

/**
 * @brief Set the size for the thumb object.
 *
 * @param[in] tw The width of the thumb.
 * @param[in] th The height of the thumb.
 *
 * @since 1.8
 *
 * @ingroup Elm_Thumb
 */
EAPI void elm_thumb_size_set(Evas_Object *obj, int tw, int th);

/**
 * @brief Get the size of the thumb object.
 *
 * @param[out] tw The width of the thumb.
 * @param[out] th The height of the thumb.
 *
 * @since 1.8
 *
 * @ingroup Elm_Thumb
 */
EAPI void elm_thumb_size_get(const Evas_Object *obj, int *tw, int *th);

/**
 * @brief Set the crop alignment for the thumb object.
 *
 * @param[in] cropx The x coordinate of the crop.
 * @param[in] cropy The y coordinate of the crop.
 *
 * @since 1.8
 *
 * @ingroup Elm_Thumb
 */
EAPI void elm_thumb_crop_align_set(Evas_Object *obj, double cropx, double cropy);

/**
 * @brief Get the crop alignment of the thumb object.
 *
 * @param[out] cropx The x coordinate of the crop.
 * @param[out] cropy The y coordinate of the crop.
 *
 * @since 1.8
 *
 * @ingroup Elm_Thumb
 */
EAPI void elm_thumb_crop_align_get(const Evas_Object *obj, double *cropx, double *cropy);

/**
 * @brief Get the path and key to the image or video thumbnail generated by
 * ethumb.
 *
 * One just needs to make sure that the thumbnail was generated before getting
 * its path; otherwise, the path will be @c null. One way to do that is by
 * asking for the path when/after the "generate,stop" smart callback is called.
 *
 * @param[out] file Pointer to thumb path.
 * @param[out] key Pointer to thumb key.
 *
 * @ingroup Elm_Thumb
 */
EAPI void elm_thumb_path_get(const Evas_Object *obj, const char **file, const char **key);

/**
 * @brief Reload thumbnail if it was generated before.
 *
 * This is useful if the ethumb client configuration changed, like its size,
 * aspect or any other property one set in the handle returned by
 * elm_thumb_ethumb_client_get().
 *
 * If the options didn't change, the thumbnail won't be generated again, but
 * the old one will still be used.
 *
 * @ingroup Elm_Thumb
 */
EAPI void elm_thumb_reload(Evas_Object *obj);

#include "elm_thumb_eo.legacy.h"
