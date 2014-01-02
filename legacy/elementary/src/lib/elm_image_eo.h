/**
 * @ingroup Image
 *
 * @{
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
   ELM_OBJ_IMAGE_SUB_ID_MMAP_SET,

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
 * @def elm_obj_image_mmap_set
 * @since 1.8
 *
 * Set the file that will be used as the image's source.
 *
 * @param[in] file
 * @param[in] group
 * @param[out] ret
 *
 * @see elm_image_mmap_set
 */
#define elm_obj_image_mmap_set(file, group, ret) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_MMAP_SET), EO_TYPECHECK(const Eina_File *, file), EO_TYPECHECK(const char *, group), EO_TYPECHECK(Eina_Bool *, ret)

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
 *
 * @see elm_image_editable_set
 */
#define elm_obj_image_editable_set(set) ELM_OBJ_IMAGE_ID(ELM_OBJ_IMAGE_SUB_ID_EDITABLE_SET), EO_TYPECHECK(Eina_Bool, set)

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
 * @}
 */
