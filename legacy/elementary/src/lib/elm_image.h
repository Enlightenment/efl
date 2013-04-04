/**
 * @defgroup Image Image
 * @ingroup Elementary
 *
 * @image html image_inheritance_tree.png
 * @image latex image_inheritance_tree.eps
 *
 * @image html img/widget/image/preview-00.png
 * @image latex img/widget/image/preview-00.eps
 *
 * An Elementary image object is a direct realization of
 * @ref elm-image-class, and it allows one to load and display an @b image
 * file on it, be it from a disk file or from a memory
 * region. Exceptionally, one may also load an Edje group as the
 * contents of the image. In this case, though, most of the functions
 * of the image API will act as a no-op.
 *
 * One can tune various properties of the image, like:
 * - pre-scaling,
 * - smooth scaling,
 * - orientation,
 * - aspect ratio during resizes, etc.
 *
 * An image object may also be made valid source and destination for
 * drag and drop actions, through the elm_image_editable_set() call.
 *
 * Signals that you can add callbacks for are:
 *
 * @li @c "drop" - This is called when a user has dropped an image
 *                 typed object onto the object in question -- the
 *                 event info argument is the path to that image file
 * @li @c "clicked" - This is called when a user has clicked the image
 *
 * An example of usage for this API follows:
 * @li @ref tutorial_image
 */


#define ELM_OBJ_IMAGE_CLASS elm_obj_image_class_get()

const Eo_Class *elm_obj_image_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_IMAGE_BASE_ID;

enum
{
   ELM_OBJ_IMAGE_SUB_ID_ASPECT_FIXED_SET,
   ELM_OBJ_IMAGE_SUB_ID_ASPECT_FIXED_GET,

   ELM_OBJ_IMAGE_SUB_ID_EDITABLE_SET,
   ELM_OBJ_IMAGE_SUB_ID_EDITABLE_GET,

   ELM_OBJ_IMAGE_SUB_ID_FILE_SET,
   ELM_OBJ_IMAGE_SUB_ID_FILE_GET,

   ELM_OBJ_IMAGE_SUB_ID_SIZING_EVAL,

   ELM_OBJ_IMAGE_SUB_ID_SMOOTH_SCALE_SET,
   ELM_OBJ_IMAGE_SUB_ID_SMOOTH_SCALE_GET,

   ELM_OBJ_IMAGE_SUB_ID_FILL_INSIDE_SET,
   ELM_OBJ_IMAGE_SUB_ID_FILL_INSIDE_GET,

   ELM_OBJ_IMAGE_SUB_ID_FILL_OUTSIDE_SET,
   ELM_OBJ_IMAGE_SUB_ID_FILL_OUTSIDE_GET,

   ELM_OBJ_IMAGE_SUB_ID_OBJECT_GET,

   ELM_OBJ_IMAGE_SUB_ID_LOAD_SIZE_SET,
   ELM_OBJ_IMAGE_SUB_ID_LOAD_SIZE_GET,

   ELM_OBJ_IMAGE_SUB_ID_MEMFILE_SET,

   ELM_OBJ_IMAGE_SUB_ID_ORIENT_SET,
   ELM_OBJ_IMAGE_SUB_ID_ORIENT_GET,

   ELM_OBJ_IMAGE_SUB_ID_PRELOAD_DISABLED_SET,

   ELM_OBJ_IMAGE_SUB_ID_RESIZE_DOWN_SET,
   ELM_OBJ_IMAGE_SUB_ID_RESIZE_DOWN_GET,

   ELM_OBJ_IMAGE_SUB_ID_RESIZE_UP_SET,
   ELM_OBJ_IMAGE_SUB_ID_RESIZE_UP_GET,

   ELM_OBJ_IMAGE_SUB_ID_SCALE_SET,
   ELM_OBJ_IMAGE_SUB_ID_SCALE_GET,

   ELM_OBJ_IMAGE_SUB_ID_OBJECT_SIZE_GET,

   ELM_OBJ_IMAGE_SUB_ID_SMOOTH_SET,
   ELM_OBJ_IMAGE_SUB_ID_SMOOTH_GET,

   ELM_OBJ_IMAGE_SUB_ID_NO_SCALE_SET,
   ELM_OBJ_IMAGE_SUB_ID_NO_SCALE_GET,

   ELM_OBJ_IMAGE_SUB_ID_RESIZABLE_SET,
   ELM_OBJ_IMAGE_SUB_ID_RESIZABLE_GET,

   ELM_OBJ_IMAGE_SUB_ID_ANIMATED_AVAILABLE_GET,

   ELM_OBJ_IMAGE_SUB_ID_ANIMATED_SET,
   ELM_OBJ_IMAGE_SUB_ID_ANIMATED_GET,

   ELM_OBJ_IMAGE_SUB_ID_ANIMATED_PLAY_SET,
   ELM_OBJ_IMAGE_SUB_ID_ANIMATED_PLAY_GET,

   ELM_OBJ_IMAGE_SUB_ID_LAST
};

#define ELM_OBJ_IMAGE_ID(sub_id) (ELM_OBJ_IMAGE_BASE_ID + sub_id)


/**
 * @def elm_obj_image_memfile_set
 * @since 1.8
 *
 * Set a location in memory to be used as an image object's source
 *
 * @param[in] img
 * @param[in] size
 * @param[in] format
 * @param[in] key
 * @param[out] ret
 *
 * @see elm_image_memfile_set
 */
#define elm_obj_image_memfile_set(img, size, format, key, ret) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_MEMFILE_SET), EO_TYPECHECK(const void *, img), EO_TYPECHECK(size_t, size), EO_TYPECHECK(const char *, format), EO_TYPECHECK(const char *, key), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_image_file_set
 * @since 1.8
 *
 * Set the file that will be used as the image's source.
 *
 * @param[in] file
 * @param[in] group
 * @param[out] ret
 *
 * @see elm_image_file_set
 */
#define elm_obj_image_file_set(file, group, ret) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_FILE_SET), EO_TYPECHECK(const char *, file), EO_TYPECHECK(const char *, group), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_image_file_get
 * @since 1.8
 *
 * Get the file that will be used as image.
 *
 * @param[out] file
 * @param[out] group
 *
 * @see elm_image_file_get
 */
#define elm_obj_image_file_get(file, group) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_FILE_GET), EO_TYPECHECK(const char **, file), EO_TYPECHECK(const char **, group)

/**
 * @def elm_obj_image_smooth_set
 * @since 1.8
 *
 * Set the smooth effect for an image.
 *
 * @param[in] smooth
 *
 * @see elm_image_smooth_set
 */
#define elm_obj_image_smooth_set(smooth) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_SMOOTH_SET), EO_TYPECHECK(Eina_Bool, smooth)

/**
 * @def elm_obj_image_smooth_get
 * @since 1.8
 *
 * Get the smooth effect for an image.
 *
 * @param[out] ret
 *
 * @see elm_image_smooth_get
 */
#define elm_obj_image_smooth_get(ret) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_SMOOTH_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_image_size_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] w
 * @param[out] h
 *
 * @see elm_image_size_get
 */
#define elm_obj_image_size_get(w, h) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_OBJECT_SIZE_GET), EO_TYPECHECK(int *, w), EO_TYPECHECK(int *, h)

/**
 * @def elm_obj_image_no_scale_set
 * @since 1.8
 *
 * Disable scaling of this object.
 *
 * @param[in] no_scale
 *
 * @see elm_image_no_scale_set
 */
#define elm_obj_image_no_scale_set(no_scale) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_NO_SCALE_SET), EO_TYPECHECK(Eina_Bool, no_scale)

/**
 * @def elm_obj_image_no_scale_get
 * @since 1.8
 *
 * Get whether scaling is disabled on the object.
 *
 * @param[out] ret
 *
 * @see elm_image_no_scale_get
 */
#define elm_obj_image_no_scale_get(ret) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_NO_SCALE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_image_resizable_set
 * @since 1.8
 *
 * Set if the object is (up/down) resizable.
 *
 * @param[in] up
 * @param[in] down
 *
 * @see elm_image_resizable_set
 */
#define elm_obj_image_resizable_set(up, down) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_RESIZABLE_SET), EO_TYPECHECK(Eina_Bool, up), EO_TYPECHECK(Eina_Bool, down)

/**
 * @def elm_obj_image_resizable_get
 * @since 1.8
 *
 * Get if the object is (up/down) resizable.
 *
 * @param[out] size_up
 * @param[out] size_down
 *
 * @see elm_image_resizable_get
 */
#define elm_obj_image_resizable_get(size_up, size_down) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_RESIZABLE_GET), EO_TYPECHECK(Eina_Bool *, size_up), EO_TYPECHECK(Eina_Bool *, size_down)

/**
 * @def elm_obj_image_fill_outside_set
 * @since 1.8
 *
 * Set if the image fills the entire object area, when keeping the aspect ratio.
 *
 * @param[in] fill_outside
 *
 * @see elm_image_fill_outside_set
 */
#define elm_obj_image_fill_outside_set(fill_outside) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_FILL_OUTSIDE_SET), EO_TYPECHECK(Eina_Bool, fill_outside)

/**
 * @def elm_obj_image_fill_outside_get
 * @since 1.8
 *
 * Get if the object is filled outside
 *
 * @param[out] ret
 *
 * @see elm_image_fill_outside_get
 */
#define elm_obj_image_fill_outside_get(ret) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_FILL_OUTSIDE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_image_preload_disabled_set
 * @since 1.8
 *
 * Enable or disable preloading of the image
 *
 * @param[in] disabled
 *
 * @see elm_image_preload_disabled_set
 */
#define elm_obj_image_preload_disabled_set(disabled) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_PRELOAD_DISABLED_SET), EO_TYPECHECK(Eina_Bool, disabled)

/**
 * @def elm_obj_image_orient_set
 * @since 1.8
 *
 * Set the image orientation.
 *
 * @param[in] orient
 *
 * @see elm_image_orient_set
 */
#define elm_obj_image_orient_set(orient) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_ORIENT_SET), EO_TYPECHECK(Elm_Image_Orient, orient)

/**
 * @def elm_obj_image_orient_get
 * @since 1.8
 *
 * Get the image orientation.
 *
 * @param[out] ret
 *
 * @see elm_image_orient_get
 */
#define elm_obj_image_orient_get(ret) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_ORIENT_GET), EO_TYPECHECK(Elm_Image_Orient *, ret)

/**
 * @def elm_obj_image_editable_set
 * @since 1.8
 *
 * Make the image 'editable'.
 *
 * @param[in] set
 * @param[in] parent
 *
 * @see elm_image_editable_set
 */
#define elm_obj_image_editable_set(set, parent) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_EDITABLE_SET), EO_TYPECHECK(Eina_Bool, set), EO_TYPECHECK(Evas_Object *, parent)

/**
 * @def elm_obj_image_editable_get
 * @since 1.8
 *
 * Check if the image is 'editable'.
 *
 * @param[out] ret
 *
 * @see elm_image_editable_get
 */
#define elm_obj_image_editable_get(ret) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_EDITABLE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_image_object_get
 * @since 1.8
 *
 * Get the inlined image object of the image widget.
 *
 * @param[out] ret
 *
 * @see elm_image_object_get
 */
#define elm_obj_image_object_get(ret) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_OBJECT_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def elm_obj_image_aspect_fixed_set
 * @since 1.8
 *
 * Set whether the original aspect ratio of the image should be kept on resize.
 *
 * @param[in] fixed
 *
 * @see elm_image_aspect_fixed_set
 */
#define elm_obj_image_aspect_fixed_set(fixed) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_ASPECT_FIXED_SET), EO_TYPECHECK(Eina_Bool, fixed)

/**
 * @def elm_obj_image_aspect_fixed_get
 * @since 1.8
 *
 * Get if the object retains the original aspect ratio.
 *
 * @param[out] ret
 *
 * @see elm_image_aspect_fixed_get
 */
#define elm_obj_image_aspect_fixed_get(ret) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_ASPECT_FIXED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_image_animated_available_get
 * @since 1.8
 *
 * Get whether an image object supports animation or not.
 *
 * @param[out] ret
 *
 * @see elm_image_animated_available_get
 */
#define elm_obj_image_animated_available_get(ret) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_ANIMATED_AVAILABLE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_image_animated_set
 * @since 1.8
 *
 * Set whether an image object (which supports animation) is to
 *
 * @param[in] anim
 *
 * @see elm_image_animated_set
 */
#define elm_obj_image_animated_set(anim) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_ANIMATED_SET), EO_TYPECHECK(Eina_Bool, anim)

/**
 * @def elm_obj_image_animated_get
 * @since 1.8
 *
 * Get whether an image object has animation enabled or not.
 *
 * @param[out] ret
 *
 * @see elm_image_animated_get
 */
#define elm_obj_image_animated_get(ret) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_ANIMATED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_image_animated_play_set
 * @since 1.8
 *
 * Start or stop an image object's animation.
 *
 * @param[in] play
 *
 * @see elm_image_animated_play_set
 */
#define elm_obj_image_animated_play_set(play) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_ANIMATED_PLAY_SET), EO_TYPECHECK(Eina_Bool, play)

/**
 * @def elm_obj_image_animated_play_get
 * @since 1.8
 *
 * Get whether an image object is under animation or not.
 *
 * @param[out] ret
 *
 * @see elm_image_animated_play_get
 */
#define elm_obj_image_animated_play_get(ret) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_ANIMATED_PLAY_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_image_sizing_eval
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 */
#define elm_obj_image_sizing_eval() ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_SIZING_EVAL)

/**
 * @def elm_obj_image_smooth_scale_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] smooth
 *
 * @see elm_image_smooth_scale_set
 */
#define elm_obj_image_smooth_scale_set(smooth) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_SMOOTH_SCALE_SET), EO_TYPECHECK(Eina_Bool, smooth)

/**
 * @def elm_obj_image_smooth_scale_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] ret
 *
 * @see elm_image_smooth_scale_get
 */
#define elm_obj_image_smooth_scale_get(ret) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_SMOOTH_SCALE_GET), EO_TYPECHECK(Eina_Bool *, ret)


/**
 * @def elm_obj_image_fill_inside_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] fill_inside
 *
 * @see elm_image_fill_inside_set
 */
#define elm_obj_image_fill_inside_set(fill_inside) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_FILL_INSIDE_SET), EO_TYPECHECK(Eina_Bool, fill_inside)

/**
 * @def elm_obj_image_fill_inside_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] ret
 *
 * @see elm_image_fill_inside_get
 */
#define elm_obj_image_fill_inside_get(ret) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_FILL_INSIDE_GET), EO_TYPECHECK(Eina_Bool *, ret)


/**
 * @def elm_obj_image_load_size_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] size
 *
 * @see elm_image_load_size_set
 */
#define elm_obj_image_load_size_set(size) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_LOAD_SIZE_SET), EO_TYPECHECK(int, size)

/**
 * @def elm_obj_image_load_size_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] ret
 *
 * @see elm_image_load_size_get
 */
#define elm_obj_image_load_size_get(ret) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_LOAD_SIZE_GET), EO_TYPECHECK(int *, ret)


/**
 * @def elm_obj_image_resize_down_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] resize_down
 *
 * @see elm_image_resize_down_set
 */
#define elm_obj_image_resize_down_set(resize_down) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_RESIZE_DOWN_SET), EO_TYPECHECK(Eina_Bool, resize_down)

/**
 * @def elm_obj_image_resize_down_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] ret
 *
 * @see elm_image_resize_down_get
 */
#define elm_obj_image_resize_down_get(ret) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_RESIZE_DOWN_GET), EO_TYPECHECK(Eina_Bool *, ret)


/**
 * @def elm_obj_image_resize_up_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] resize_up
 *
 * @see elm_image_resize_up_set
 */
#define elm_obj_image_resize_up_set(resize_up) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_RESIZE_UP_SET), EO_TYPECHECK(Eina_Bool, resize_up)

/**
 * @def elm_obj_image_resize_up_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] ret
 *
 * @see elm_image_resize_up_get
 */
#define elm_obj_image_resize_up_get(ret) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_RESIZE_UP_GET), EO_TYPECHECK(Eina_Bool *, ret)


/**
 * @def elm_obj_image_scale_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] scale
 *
 * @see elm_image_scale_set
 */
#define elm_obj_image_scale_set(scale) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_SCALE_SET), EO_TYPECHECK(double, scale)

/**
 * @def elm_obj_image_scale_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] ret
 *
 * @see elm_image_scale_get
 */
#define elm_obj_image_scale_get(ret) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_SCALE_GET), EO_TYPECHECK(double *, ret)

/**
 * @addtogroup Image
 * @{
 */

/**
 * Possible orientation options for elm_image_orient_set().
 *
 * @image html elm_image_orient_set.png
 * @image latex elm_image_orient_set.eps width=\textwidth
 *
 * @ingroup Image
 */
typedef enum
{
   ELM_IMAGE_ORIENT_NONE = 0, /**< no orientation change */
   ELM_IMAGE_ORIENT_0 = 0, /**< no orientation change */
   ELM_IMAGE_ROTATE_90 = 1, /**< rotate 90 degrees clockwise */
   ELM_IMAGE_ROTATE_180 = 2, /**< rotate 180 degrees clockwise */
   ELM_IMAGE_ROTATE_270 = 3, /**< rotate 90 degrees counter-clockwise (i.e. 270 degrees clockwise) */
   ELM_IMAGE_FLIP_HORIZONTAL = 4, /**< flip image horizontally */
   ELM_IMAGE_FLIP_VERTICAL = 5, /**< flip image vertically */
   ELM_IMAGE_FLIP_TRANSPOSE = 6, /**< flip the image along the y = (width - x) line (bottom-left to top-right) */
   ELM_IMAGE_FLIP_TRANSVERSE = 7 /**< flip the image along the y = x line (top-left to bottom-right) */
} Elm_Image_Orient;

/**
 * Add a new image to the parent.
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @see elm_image_file_set()
 *
 * @ingroup Image
 */
EAPI Evas_Object     *elm_image_add(Evas_Object *parent);

/**
 * Set a location in memory to be used as an image object's source
 * bitmap.
 *
 * @param obj The image object
 * @param img The binary data that will be used as image source
 * @param size The size of binary data blob @p img
 * @param format (Optional) expected format of @p img bytes
 * @param key Optional indexing key of @p img to be passed to the
 *            image loader (eg. if @p img is a memory-mapped EET file)
 *
 * This function is handy when the contents of an image file are
 * mapped in memory, for example.
 *
 * The @p format string should be something like @c "png", @c "jpg",
 * @c "tga", @c "tiff", @c "bmp" etc, when provided (@c NULL, on the
 * contrary). This improves the loader performance as it tries the
 * "correct" loader first, before trying a range of other possible
 * loaders until one succeeds.
 *
 * @return (@c EINA_TRUE = success, @c EINA_FALSE = error)
 *
 * @since 1.7
 *
 * @ingroup Image
 */
EAPI Eina_Bool        elm_image_memfile_set(Evas_Object *obj, const void *img, size_t size, const char *format, const char *key);

/**
 * Set the file that will be used as the image's source.
 *
 * @param obj The image object
 * @param file The path to file that will be used as image source
 * @param group The group that the image belongs to, in case it's an
 *              EET (including Edje case) file
 *
 * @return (@c EINA_TRUE = success, @c EINA_FALSE = error)
 *
 * @see elm_image_file_get()
 *
 * @note This function will trigger the Edje file case based on the
 * extension of the @a file string (expects @c ".edj", for this
 * case). If one wants to force this type of file independently of the
 * extension, elm_image_file_edje_set() must be used, instead.
 *
 * @ingroup Image
 */
EAPI Eina_Bool        elm_image_file_set(Evas_Object *obj, const char *file, const char *group);

/**
 * Get the file that will be used as image.
 *
 * @param obj The image object
 * @param file The path to file
 * @param group The group that the image belongs in edje file
 *
 * @see elm_image_file_set()
 *
 * @ingroup Image
 */
EAPI void             elm_image_file_get(const Evas_Object *obj, const char **file, const char **group);

/**
 * Set the smooth effect for an image.
 *
 * @param obj The image object
 * @param smooth @c EINA_TRUE if smooth scaling should be used, @c EINA_FALSE
 * otherwise. Default is @c EINA_TRUE.
 *
 * Set the scaling algorithm to be used when scaling the image. Smooth
 * scaling provides a better resulting image, but is slower.
 *
 * The smooth scaling should be disabled when making animations that change
 * the image size, since it will be faster. Animations that don't require
 * resizing of the image can keep the smooth scaling enabled (even if the
 * image is already scaled, since the scaled image will be cached).
 *
 * @see elm_image_smooth_get()
 *
 * @ingroup Image
 */
EAPI void             elm_image_smooth_set(Evas_Object *obj, Eina_Bool smooth);

/**
 * Get the smooth effect for an image.
 *
 * @param obj The image object
 * @return @c EINA_TRUE if smooth scaling is enabled, @c EINA_FALSE otherwise.
 *
 * @see elm_image_smooth_set()
 *
 * @ingroup Image
 */
EAPI Eina_Bool        elm_image_smooth_get(const Evas_Object *obj);

/**
 * Gets the current size of the image.
 *
 * @param obj The image object.
 * @param w Pointer to store width, or NULL.
 * @param h Pointer to store height, or NULL.
 *
 * This is the real size of the image, not the size of the object.
 *
 * @ingroup Image
 */
EAPI void             elm_image_object_size_get(const Evas_Object *obj, int *w, int *h);

/**
 * Disable scaling of this object.
 *
 * @param obj The image object.
 * @param no_scale @c EINA_TRUE if the object is not scalable, @c EINA_FALSE
 * otherwise. Default is @c EINA_FALSE.
 *
 * This function disables scaling of the elm_image widget through the
 * function elm_object_scale_set(). However, this does not affect the widget
 * size/resize in any way. For that effect, take a look at
 * elm_image_resizable_set().
 *
 * @see elm_image_no_scale_get()
 * @see elm_image_resizable_set()
 * @see elm_object_scale_set()
 *
 * @ingroup Image
 */
EAPI void             elm_image_no_scale_set(Evas_Object *obj, Eina_Bool no_scale);

/**
 * Get whether scaling is disabled on the object.
 *
 * @param obj The image object
 * @return @c EINA_TRUE if scaling is disabled, @c EINA_FALSE otherwise
 *
 * @see elm_image_no_scale_set()
 *
 * @ingroup Image
 */
EAPI Eina_Bool        elm_image_no_scale_get(const Evas_Object *obj);

/**
 * Set if the object is (up/down) resizable.
 *
 * @param obj The image object
 * @param size_up A bool to set if the object is resizable up. Default is
 * @c EINA_TRUE.
 * @param size_down A bool to set if the object is resizable down. Default
 * is @c EINA_TRUE.
 *
 * This function limits the image resize ability. If @p size_up is set to
 * @c EINA_FALSE, the object can't have its height or width resized to a value
 * higher than the original image size. Same is valid for @p size_down.
 *
 * @see elm_image_resizable_get()
 *
 * @ingroup Image
 */
EAPI void             elm_image_resizable_set(Evas_Object *obj, Eina_Bool size_up, Eina_Bool size_down);

/**
 * Get if the object is (up/down) resizable.
 *
 * @param obj The image object
 * @param size_up A bool to set if the object is resizable up
 * @param size_down A bool to set if the object is resizable down
 *
 * @see elm_image_resizable_set()
 *
 * @ingroup Image
 */
EAPI void             elm_image_resizable_get(const Evas_Object *obj, Eina_Bool *size_up, Eina_Bool *size_down);

/**
 * Set if the image fills the entire object area, when keeping the aspect ratio.
 *
 * @param obj The image object
 * @param fill_outside @c EINA_TRUE if the object is filled outside,
 * @c EINA_FALSE otherwise. Default is @c EINA_FALSE.
 *
 * When the image should keep its aspect ratio even if resized to another
 * aspect ratio, there are two possibilities to resize it: keep the entire
 * image inside the limits of height and width of the object (@p fill_outside
 * is @c EINA_FALSE) or let the extra width or height go outside of the object,
 * and the image will fill the entire object (@p fill_outside is @c EINA_TRUE).
 *
 * @note This option will have no effect if
 * elm_image_aspect_fixed_set() is set to @c EINA_FALSE.
 *
 * @see elm_image_fill_outside_get()
 * @see elm_image_aspect_fixed_set()
 *
 * @ingroup Image
 */
EAPI void             elm_image_fill_outside_set(Evas_Object *obj, Eina_Bool fill_outside);

/**
 * Get if the object is filled outside
 *
 * @param obj The image object
 * @return @c EINA_TRUE if the object is filled outside, @c EINA_FALSE otherwise.
 *
 * @see elm_image_fill_outside_set()
 *
 * @ingroup Image
 */
EAPI Eina_Bool        elm_image_fill_outside_get(const Evas_Object *obj);

/**
 * Enable or disable preloading of the image
 *
 * @param obj The image object
 * @param disabled If EINA_TRUE, preloading will be disabled
 * @ingroup Image
 */
EAPI void             elm_image_preload_disabled_set(Evas_Object *obj, Eina_Bool disabled);

/**
 * Set the prescale size for the image
 *
 * @param obj The image object
 * @param size The prescale size. This value is used for both width and
 * height.
 *
 * This function sets a new size for pixmap representation of the given
 * image. It allows the image to be loaded already in the specified size,
 * reducing the memory usage and load time when loading a big image with load
 * size set to a smaller size.
 *
 * It's equivalent to the elm_bg_load_size_set() function for bg.
 *
 * @note this is just a hint, the real size of the pixmap may differ
 * depending on the type of image being loaded, being bigger than requested.
 *
 * @see elm_image_prescale_get()
 * @see elm_bg_load_size_set()
 *
 * @ingroup Image
 */
EAPI void             elm_image_prescale_set(Evas_Object *obj, int size);

/**
 * Get the prescale size for the image
 *
 * @param obj The image object
 * @return The prescale size
 *
 * @see elm_image_prescale_set()
 *
 * @ingroup Image
 */
EAPI int              elm_image_prescale_get(const Evas_Object *obj);

/**
 * Set the image orientation.
 *
 * @param obj The image object
 * @param orient The image orientation @ref Elm_Image_Orient
 *  Default is #ELM_IMAGE_ORIENT_NONE.
 *
 * This function allows to rotate or flip the given image.
 *
 * @see elm_image_orient_get()
 * @see @ref Elm_Image_Orient
 *
 * @ingroup Image
 */
EAPI void             elm_image_orient_set(Evas_Object *obj, Elm_Image_Orient orient);

/**
 * Get the image orientation.
 *
 * @param obj The image object
 * @return The image orientation @ref Elm_Image_Orient
 *
 * @see elm_image_orient_set()
 * @see @ref Elm_Image_Orient
 *
 * @ingroup Image
 */
EAPI Elm_Image_Orient elm_image_orient_get(const Evas_Object *obj);

/**
 * Make the image 'editable'.
 *
 * @param obj Image object.
 * @param set Turn on or off editability. Default is @c EINA_FALSE.
 *
 * This means the image is a valid drag target for drag and drop, and can be
 * cut or pasted too.
 *
 * @ingroup Image
 */
EAPI void             elm_image_editable_set(Evas_Object *obj, Eina_Bool set);

/**
 * Check if the image is 'editable'.
 *
 * @param obj Image object.
 * @return Editability.
 *
 * A return value of EINA_TRUE means the image is a valid drag target
 * for drag and drop, and can be cut or pasted too.
 *
 * @ingroup Image
 */
EAPI Eina_Bool        elm_image_editable_get(const Evas_Object *obj);

/**
 * Get the inlined image object of the image widget.
 *
 * @param obj The image object to get the inlined image from
 * @return The inlined image object, or NULL if none exists
 *
 * This function allows one to get the underlying @c Evas_Object of type
 * Image from this elementary widget. It can be useful to do things like get
 * the pixel data, save the image to a file, etc.
 *
 * @note Be careful to not manipulate it, as it is under control of
 * elementary.
 *
 * @ingroup Image
 */
EAPI Evas_Object     *elm_image_object_get(const Evas_Object *obj);

/**
 * Set whether the original aspect ratio of the image should be kept on resize.
 *
 * @param obj The image object.
 * @param fixed @c EINA_TRUE if the image should retain the aspect,
 * @c EINA_FALSE otherwise.
 *
 * The original aspect ratio (width / height) of the image is usually
 * distorted to match the object's size. Enabling this option will retain
 * this original aspect, and the way that the image is fit into the object's
 * area depends on the option set by elm_image_fill_outside_set().
 *
 * @see elm_image_aspect_fixed_get()
 * @see elm_image_fill_outside_set()
 *
 * @ingroup Image
 */
EAPI void             elm_image_aspect_fixed_set(Evas_Object *obj, Eina_Bool fixed);

/**
 * Get if the object retains the original aspect ratio.
 *
 * @param obj The image object.
 * @return @c EINA_TRUE if the object keeps the original aspect, @c EINA_FALSE
 * otherwise.
 *
 * @ingroup Image
 */
EAPI Eina_Bool        elm_image_aspect_fixed_get(const Evas_Object *obj);

/**
 * Get whether an image object supports animation or not.
 *
 * @param obj The image object
 * @return @c EINA_TRUE if the image supports animation,
 *         @c EINA_FALSE otherwise.
 *
 * This function returns if this Elementary image object's internal
 * image can be animated. Currently Evas only supports GIF
 * animation. If the return value is @b EINA_FALSE, other
 * @c elm_image_animated_xxx API calls won't work.
 *
 * @see elm_image_animated_set()
 *
 * @ingroup Image
 * @since 1.7
 */
EAPI Eina_Bool        elm_image_animated_available_get(const Evas_Object *obj);

/**
 * Set whether an image object (which supports animation) is to
 * animate itself or not.
 *
 * @param obj The image object

 * @param animated @c EINA_TRUE if the object is to animate itself,
 *                 @c EINA_FALSE otherwise. Default is @c EINA_FALSE.
 *
 * An image object, even if it supports animation, will be displayed
 * by default without animation. Call this function with @a animated
 * set to @c EINA_TRUE to enable its animation. To start or stop the
 * animation, actually, use elm_image_animated_play_set().
 *
 * @see elm_image_animated_get()
 * @see elm_image_animated_available_get()
 * @see elm_image_animated_play_set()
 *
 * @ingroup Image
 * @since 1.7
 */
EAPI void             elm_image_animated_set(Evas_Object *obj, Eina_Bool animated);

/**
 * Get whether an image object has animation enabled or not.
 *
 * @param obj The image object
 *
 * @return @c EINA_TRUE if the image has animation enabled,
 *         @c EINA_FALSE otherwise.
 *
 * @see elm_image_animated_set()
 *
 * @ingroup Image
 * @since 1.7
 */
EAPI Eina_Bool        elm_image_animated_get(const Evas_Object *obj);

/**
 * Start or stop an image object's animation.
 *
 * @param obj The image object
 * @param play @c EINA_TRUE to start the animation, @c EINA_FALSE
 *             otherwise. Default is @c EINA_FALSE.
 *
 * To actually start playing any image object's animation, if it
 * supports it, one must do something like:
 *
 * @code
 * if (elm_image_animated_available_get(img))
 *   {
 *      elm_image_animated_set(img, EINA_TRUE);
 *      elm_image_animated_play_set(img, EINA_TRUE);
 *   }
 * @endcode
 *
 * elm_image_animated_set() will enable animation on the image, <b>but
 * not start it yet</b>. This is the function one uses to start and
 * stop animations on image objects.
 *
 * @see elm_image_animated_available_get()
 * @see elm_image_animated_set()
 * @see elm_image_animated_play_get()
 *
 * @ingroup Image
 * @since 1.7
 */
EAPI void             elm_image_animated_play_set(Evas_Object *obj, Eina_Bool play);

/**
 * Get whether an image object is under animation or not.
 *
 * @param obj The image object
 * @return @c EINA_TRUE, if the image is being animated, @c EINA_FALSE
 *            otherwise.
 *
 * @see elm_image_animated_play_get()
 *
 * @ingroup Image
 * @since 1.7
 */
EAPI Eina_Bool        elm_image_animated_play_get(const Evas_Object *obj);

/**
 * @}
 */
