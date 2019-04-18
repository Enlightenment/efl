#ifndef _EFL_UI_IMAGE_ZOOMABLE_EO_LEGACY_H_
#define _EFL_UI_IMAGE_ZOOMABLE_EO_LEGACY_H_

#ifndef _EFL_UI_IMAGE_ZOOMABLE_EO_CLASS_TYPE
#define _EFL_UI_IMAGE_ZOOMABLE_EO_CLASS_TYPE

typedef Eo Efl_Ui_Image_Zoomable;

#endif

#ifndef _EFL_UI_IMAGE_ZOOMABLE_EO_TYPES
#define _EFL_UI_IMAGE_ZOOMABLE_EO_TYPES


#endif

/**
 * @brief Set the gesture state for photocam.
 *
 * This sets the gesture state to on or off for photocam. The default is off.
 * This will start multi touch zooming.
 *
 * @param[in] obj The object.
 * @param[in] gesture The gesture state.
 *
 * @ingroup Elm_Photocam_Group
 */
EAPI void elm_photocam_gesture_enabled_set(Efl_Ui_Image_Zoomable *obj, Eina_Bool gesture);

/**
 * @brief Get the gesture state for photocam.
 *
 * This gets the current gesture state for the photocam object.
 *
 * @param[in] obj The object.
 *
 * @return The gesture state.
 *
 * @ingroup Elm_Photocam_Group
 */
EAPI Eina_Bool elm_photocam_gesture_enabled_get(const Efl_Ui_Image_Zoomable *obj);



#endif
