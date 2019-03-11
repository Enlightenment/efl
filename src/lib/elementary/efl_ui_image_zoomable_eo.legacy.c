
EAPI void
elm_photocam_gesture_enabled_set(Efl_Ui_Image_Zoomable *obj, Eina_Bool gesture)
{
   efl_ui_image_zoomable_gesture_enabled_set(obj, gesture);
}

EAPI Eina_Bool
elm_photocam_gesture_enabled_get(const Efl_Ui_Image_Zoomable *obj)
{
   return efl_ui_image_zoomable_gesture_enabled_get(obj);
}
