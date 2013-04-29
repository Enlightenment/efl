/**
 * @ingroup Photo
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
 * @}
 */
