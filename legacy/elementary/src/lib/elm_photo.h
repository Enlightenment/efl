/**
 * @defgroup Photo Photo
 * @ingroup Elementary
 *
 * @image html photo_inheritance_tree.png
 * @image latex photo_inheritance_tree.eps
 *
 * The Elementary photo widget is intended for displaying a photo, for
 * ex., a person's image (contact). Simple, yet with a very specific
 * purpose. It has a decorative frame around the inner image itself,
 * on the default theme. If and while no photo is set on it, it
 * displays a person icon, indicating it's a photo placeholder.
 *
 * This widget relies on an internal @ref Icon, so that the APIs of
 * these two widgets are similar (drag and drop is also possible here,
 * for example).
 *
 * Signals that you can add callbacks for are:
 * - @c "clicked" - This is called when a user has clicked the photo
 * - @c "drag,start" - One has started dragging the inner image out of the
 *                     photo's frame
 * - @c "drag,end" - One has dropped the dragged image somewhere
 *
 * @{
 */

#define ELM_OBJ_PHOTO_CLASS elm_obj_photo_class_get()

const Eo_Class *elm_obj_photo_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_PHOTO_BASE_ID;

enum
{
   ELM_OBJ_PHOTO_SUB_ID_FILE_SET,
   ELM_OBJ_PHOTO_SUB_ID_SIZE_SET,
   ELM_OBJ_PHOTO_SUB_ID_SIZE_GET,
   ELM_OBJ_PHOTO_SUB_ID_FILL_INSIDE_SET,
   ELM_OBJ_PHOTO_SUB_ID_FILL_INSIDE_GET,
   ELM_OBJ_PHOTO_SUB_ID_EDITABLE_SET,
   ELM_OBJ_PHOTO_SUB_ID_EDITABLE_GET,
   ELM_OBJ_PHOTO_SUB_ID_THUMB_SET,
   ELM_OBJ_PHOTO_SUB_ID_ASPECT_FIXED_SET,
   ELM_OBJ_PHOTO_SUB_ID_ASPECT_FIXED_GET,
   ELM_OBJ_PHOTO_SUB_ID_LAST
};

#define ELM_OBJ_PHOTO_ID(sub_id) (ELM_OBJ_PHOTO_BASE_ID + sub_id)


/**
 * @def elm_obj_photo_add
 * @since 1.8
 *
 * Add a new photo to the parent
 *
 * @param[out] ret
 *
 * @see elm_photo_add
 */
#define elm_obj_photo_add(ret) ELM_OBJ_PHOTO_ID(ELM_OBJ_PHOTO_SUB_ID_ADD), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def elm_obj_photo_file_set
 * @since 1.8
 *
 * Set the file that will be used as the photo widget's image.
 *
 * @param[in] file
 * @param[out] ret
 *
 * @see elm_photo_file_set
 */
#define elm_obj_photo_file_set(file, ret) ELM_OBJ_PHOTO_ID(ELM_OBJ_PHOTO_SUB_ID_FILE_SET), EO_TYPECHECK(const char *, file), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_photo_size_set
 * @since 1.8
 *
 * Set the size that will be used on the photo
 *
 * @param[in] size
 *
 * @see elm_photo_size_set
 */
#define elm_obj_photo_size_set(size) ELM_OBJ_PHOTO_ID(ELM_OBJ_PHOTO_SUB_ID_SIZE_SET), EO_TYPECHECK(int, size)

/**
 * @def elm_obj_photo_size_get
 * @since 1.8
 *
 * Get the size that will be used on the photo
 *
 * @param[out] ret
 *
 * @note There is no elm_photo_size_get
 *
 * @see elm_photo_size_set
 * @see elm_obj_photo_size_set
 */
#define elm_obj_photo_size_get(ret) ELM_OBJ_PHOTO_ID(ELM_OBJ_PHOTO_SUB_ID_SIZE_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_photo_fill_inside_set
 * @since 1.8
 *
 * Set if the photo should be completely visible or not.
 *
 * @param[in] fill
 *
 * @see elm_photo_fill_inside_set
 */
#define elm_obj_photo_fill_inside_set(fill) ELM_OBJ_PHOTO_ID(ELM_OBJ_PHOTO_SUB_ID_FILL_INSIDE_SET), EO_TYPECHECK(Eina_Bool, fill)

/**
 * @def elm_obj_photo_fill_inside_get
 * @since 1.8
 *
 * Get if the photo should be completely visible or not.
 *
 * @param[out] ret
 *
 * @note There is no elm_photo_fill_inside_get
 *
 * @see elm_photo_fill_inside_set
 * @see elm_obj_photo_fill_inside_set
 */
#define elm_obj_photo_fill_inside_get(ret) ELM_OBJ_PHOTO_ID(ELM_OBJ_PHOTO_SUB_ID_FILL_INSIDE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_photo_editable_set
 * @since 1.8
 *
 * Set editability of the photo.
 *
 * @param[in] set
 *
 * @see elm_photo_editable_set
 */
#define elm_obj_photo_editable_set(set) ELM_OBJ_PHOTO_ID(ELM_OBJ_PHOTO_SUB_ID_EDITABLE_SET), EO_TYPECHECK(Eina_Bool, set)

/**
 * @def elm_obj_photo_editable_get
 * @since 1.8
 *
 * Get editability of the photo.
 *
 * @param[out] ret
 *
 * @note There is no elm_photo_editable_get
 *
 * @see elm_photo_editable_set
 * @see elm_obj_photo_editable_set
 */
#define elm_obj_photo_editable_get(ret) ELM_OBJ_PHOTO_ID(ELM_OBJ_PHOTO_SUB_ID_EDITABLE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_photo_thumb_set
 * @since 1.8
 *
 * Set the file that will be used as thumbnail in the photo.
 *
 * @param[in] file
 * @param[in] group
 *
 * @see elm_photo_thumb_set
 */
#define elm_obj_photo_thumb_set(file, group) ELM_OBJ_PHOTO_ID(ELM_OBJ_PHOTO_SUB_ID_THUMB_SET), EO_TYPECHECK(const char *, file), EO_TYPECHECK(const char *, group)

/**
 * @def elm_obj_photo_aspect_fixed_set
 * @since 1.8
 *
 * Set whether the original aspect ratio of the photo should be kept on resize.
 *
 * @param[in] fixed
 *
 * @see elm_photo_aspect_fixed_set
 */
#define elm_obj_photo_aspect_fixed_set(fixed) ELM_OBJ_PHOTO_ID(ELM_OBJ_PHOTO_SUB_ID_ASPECT_FIXED_SET), EO_TYPECHECK(Eina_Bool, fixed)

/**
 * @def elm_obj_photo_aspect_fixed_get
 * @since 1.8
 *
 * Get if the object keeps the original aspect ratio.
 *
 * @param[out] ret
 *
 * @see elm_photo_aspect_fixed_get
 */
#define elm_obj_photo_aspect_fixed_get(ret) ELM_OBJ_PHOTO_ID(ELM_OBJ_PHOTO_SUB_ID_ASPECT_FIXED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * Add a new photo to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Photo
 */
EAPI Evas_Object *elm_photo_add(Evas_Object *parent);

/**
 * Set the file that will be used as the photo widget's image.
 *
 * @param obj The photo object
 * @param file The path to file that will be used as @a obj's image.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * @note Use @c NULL on @a file to set the photo widget back to it's
 * initial state, which indicates "no photo".
 *
 * @ingroup Photo
 */
EAPI Eina_Bool elm_photo_file_set(Evas_Object *obj, const char *file);

/**
 * Set the file that will be used as thumbnail in the photo.
 *
 * @param obj The photo object.
 * @param file The path to file that will be used as thumbnail.
 * @param group The key used in case of an EET file.
 *
 * @ingroup Photo
 */
EAPI void      elm_photo_thumb_set(const Evas_Object *obj, const char *file, const char *group);

/**
 * Set the size that will be used on the photo
 *
 * @param obj The photo object
 * @param size The size of the photo
 *
 * @ingroup Photo
 */
EAPI void      elm_photo_size_set(Evas_Object *obj, int size);

/**
 * Set if the photo should be completely visible or not.
 *
 * @param obj The photo object
 * @param fill if true the photo will be completely visible
 *
 * @ingroup Photo
 */
EAPI void      elm_photo_fill_inside_set(Evas_Object *obj, Eina_Bool fill);

/**
 * Set editability of the photo.
 *
 * An editable photo can be dragged to or from, and can be cut or
 * pasted too.  Note that pasting an image or dropping an item on
 * the image will delete the existing content.
 *
 * @param obj The photo object.
 * @param set To set of clear editability.
 */
EAPI void      elm_photo_editable_set(Evas_Object *obj, Eina_Bool set);

/**
 * Set whether the original aspect ratio of the photo should be kept on resize.
 *
 * @param obj The photo object.
 * @param fixed @c EINA_TRUE if the photo should fix the aspect,
 * @c EINA_FALSE otherwise.
 *
 * The original aspect ratio (width / height) of the photo is usually
 * distorted to match the object's size. Enabling this option will fix
 * this original aspect, and the way that the photo is fit into
 * the object's area
 *
 * @see elm_photo_aspect_fixed_get()
 *
 * @ingroup Photo
 */
EAPI void             elm_photo_aspect_fixed_set(Evas_Object *obj, Eina_Bool fixed);

/**
 * Get if the object keeps the original aspect ratio.
 *
 * @param obj The photo object.
 * @return @c EINA_TRUE if the object keeps the original aspect, @c EINA_FALSE
 * otherwise.
 *
 * @ingroup Photo
 */
EAPI Eina_Bool        elm_photo_aspect_fixed_get(const Evas_Object *obj);

/**
 * @}
 */
