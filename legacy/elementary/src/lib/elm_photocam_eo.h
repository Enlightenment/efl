#define ELM_OBJ_PHOTOCAM_PAN_CLASS elm_obj_photocam_pan_class_get()

const Eo_Class *elm_obj_photocam_pan_class_get(void) EINA_CONST;

#define ELM_OBJ_PHOTOCAM_CLASS elm_obj_photocam_class_get()

const Eo_Class *elm_obj_photocam_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_PHOTOCAM_BASE_ID;

enum
{
   ELM_OBJ_PHOTOCAM_SUB_ID_FILE_SET,
   ELM_OBJ_PHOTOCAM_SUB_ID_FILE_GET,
   ELM_OBJ_PHOTOCAM_SUB_ID_ZOOM_SET,
   ELM_OBJ_PHOTOCAM_SUB_ID_ZOOM_GET,
   ELM_OBJ_PHOTOCAM_SUB_ID_ZOOM_MODE_SET,
   ELM_OBJ_PHOTOCAM_SUB_ID_ZOOM_MODE_GET,
   ELM_OBJ_PHOTOCAM_SUB_ID_IMAGE_SIZE_GET,
   ELM_OBJ_PHOTOCAM_SUB_ID_IMAGE_REGION_GET,
   ELM_OBJ_PHOTOCAM_SUB_ID_IMAGE_REGION_SHOW,
   ELM_OBJ_PHOTOCAM_SUB_ID_PAUSED_SET,
   ELM_OBJ_PHOTOCAM_SUB_ID_PAUSED_GET,
   ELM_OBJ_PHOTOCAM_SUB_ID_INTERNAL_IMAGE_GET,
   ELM_OBJ_PHOTOCAM_SUB_ID_GESTURE_ENABLED_SET,
   ELM_OBJ_PHOTOCAM_SUB_ID_GESTURE_ENABLED_GET,
   ELM_OBJ_PHOTOCAM_SUB_ID_LAST
};

#define ELM_OBJ_PHOTOCAM_ID(sub_id) (ELM_OBJ_PHOTOCAM_BASE_ID + sub_id)


/**
 * @def elm_obj_photocam_file_set
 * @since 1.8
 *
 * @brief Set the photo file to be shown
 *
 * @param[in] file
 * @param[out] ret
 *
 * @see elm_photocam_file_set
 *
 * @ingroup Photocam
 */
#define elm_obj_photocam_file_set(file, ret) ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_FILE_SET), EO_TYPECHECK(const char *, file), EO_TYPECHECK(Evas_Load_Error *, ret)

/**
 * @def elm_obj_photocam_file_get
 * @since 1.8
 *
 * @brief Returns the path of the current image file
 *
 * @param[out] ret
 *
 * @see elm_photocam_file_get
 *
 * @ingroup Photocam
 */
#define elm_obj_photocam_file_get(ret) ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_FILE_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_photocam_zoom_set
 * @since 1.8
 *
 * @brief Set the zoom level of the photo
 *
 * @param[in] zoom
 *
 * @see elm_photocam_zoom_set
 *
 * @ingroup Photocam
 */
#define elm_obj_photocam_zoom_set(zoom) ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_ZOOM_SET), EO_TYPECHECK(double, zoom)

/**
 * @def elm_obj_photocam_zoom_get
 * @since 1.8
 *
 * @brief Get the zoom level of the photo
 *
 * @param[out] ret
 *
 * @see elm_photocam_zoom_get
 *
 * @ingroup Photocam
 */
#define elm_obj_photocam_zoom_get(ret) ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_ZOOM_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_photocam_zoom_mode_set
 * @since 1.8
 *
 * @brief Set the zoom mode
 *
 * @param[in] mode
 *
 * @see elm_photocam_zoom_mode_set
 *
 * @ingroup Photocam
 */
#define elm_obj_photocam_zoom_mode_set(mode) ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_ZOOM_MODE_SET), EO_TYPECHECK(Elm_Photocam_Zoom_Mode, mode)

/**
 * @def elm_obj_photocam_zoom_mode_get
 * @since 1.8
 *
 * @brief Get the zoom mode
 *
 * @param[out] ret
 *
 * @see elm_photocam_zoom_mode_get
 *
 * @ingroup Photocam
 */
#define elm_obj_photocam_zoom_mode_get(ret) ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_ZOOM_MODE_GET), EO_TYPECHECK(Elm_Photocam_Zoom_Mode *, ret)

/**
 * @def elm_obj_photocam_image_size_get
 * @since 1.8
 *
 * @brief Get the current image pixel width and height
 *
 * @param[out] w
 * @param[out] h
 *
 * @see elm_photocam_image_size_get
 *
 * @ingroup Photocam
 */
#define elm_obj_photocam_image_size_get(w, h) ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_IMAGE_SIZE_GET), EO_TYPECHECK(int *, w), EO_TYPECHECK(int *, h)

/**
 * @def elm_obj_photocam_image_region_get
 * @since 1.8
 *
 * @brief Get the region of the image that is currently shown
 *
 * @param[out] x
 * @param[out] y
 * @param[out] w
 * @param[out] h
 *
 * @see elm_photocam_image_region_get
 *
 * @ingroup Photocam
 */
#define elm_obj_photocam_image_region_get(x, y, w, h) ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_IMAGE_REGION_GET), EO_TYPECHECK(int *, x), EO_TYPECHECK(int *, y), EO_TYPECHECK(int *, w), EO_TYPECHECK(int *, h)

/**
 * @def elm_obj_photocam_image_region_show
 * @since 1.8
 *
 * @brief Set the viewed region of the image
 *
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 *
 * @see elm_photocam_image_region_show
 *
 * @ingroup Photocam
 */
#define elm_obj_photocam_image_region_show(x, y, w, h) ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_IMAGE_REGION_SHOW), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def elm_obj_photocam_paused_set
 * @since 1.8
 *
 * @brief Set the paused state for photocam
 *
 * @param[in] paused
 *
 * @see elm_photocam_paused_set
 *
 * @ingroup Photocam
 */
#define elm_obj_photocam_paused_set(paused) ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_PAUSED_SET), EO_TYPECHECK(Eina_Bool, paused)

/**
 * @def elm_obj_photocam_paused_get
 * @since 1.8
 *
 * @brief Get the paused state for photocam
 *
 * @param[out] ret
 *
 * @see elm_photocam_paused_get
 *
 * @ingroup Photocam
 */
#define elm_obj_photocam_paused_get(ret) ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_PAUSED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_photocam_internal_image_get
 * @since 1.8
 *
 * @brief Get the internal low-res image used for photocam
 *
 * @param[out] ret
 *
 * @see elm_photocam_internal_image_get
 *
 * @ingroup Photocam
 */
#define elm_obj_photocam_internal_image_get(ret) ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_INTERNAL_IMAGE_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def elm_obj_photocam_gesture_enabled_set
 * @since 1.8
 *
 * @brief Set the gesture state for photocam.
 *
 * @param[in] gesture
 *
 * @see elm_photocam_gesture_enabled_set
 *
 * @ingroup Photocam
 */
#define elm_obj_photocam_gesture_enabled_set(gesture) ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_GESTURE_ENABLED_SET), EO_TYPECHECK(Eina_Bool, gesture)

/**
 * @def elm_obj_photocam_gesture_enabled_get
 * @since 1.8
 *
 * @brief Get the gesture state for photocam.
 *
 * @param[out] ret
 *
 * @see elm_photocam_gesture_enabled_get
 *
 * @ingroup Photocam
 */
#define elm_obj_photocam_gesture_enabled_get(ret) ELM_OBJ_PHOTOCAM_ID(ELM_OBJ_PHOTOCAM_SUB_ID_GESTURE_ENABLED_GET), EO_TYPECHECK(Eina_Bool *, ret)
