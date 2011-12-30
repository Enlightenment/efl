   /**
    * @defgroup Fingers Fingers
    *
    * Elementary is designed to be finger-friendly for touchscreens,
    * and so in addition to scaling for display resolution, it can
    * also scale based on finger "resolution" (or size). You can then
    * customize the granularity of the areas meant to receive clicks
    * on touchscreens.
    *
    * Different profiles may have pre-set values for finger sizes.
    *
    * @ref general_functions_example_page "This" example contemplates
    * some of these functions.
    *
    * @{
    */

   /**
    * Get the configured "finger size"
    *
    * @return The finger size
    *
    * This gets the globally configured finger size, <b>in pixels</b>
    *
    * @ingroup Fingers
    */
   EAPI Evas_Coord       elm_finger_size_get(void);

   /**
    * Set the configured finger size
    *
    * This sets the globally configured finger size in pixels
    *
    * @param size The finger size
    * @ingroup Fingers
    */
   EAPI void             elm_finger_size_set(Evas_Coord size);

   /**
    * Set the configured finger size for all applications on the display
    *
    * This sets the globally configured finger size in pixels for all
    * applications on the display
    *
    * @param size The finger size
    * @ingroup Fingers
    */
   // XXX: deprecate and replace with elm_config_all_flush()
   EAPI void             elm_finger_size_all_set(Evas_Coord size);

   /**
    * Adjust size of an element for finger usage.
    *
    * @param times_w How many fingers should fit horizontally
    * @param w Pointer to the width size to adjust
    * @param times_h How many fingers should fit vertically
    * @param h Pointer to the height size to adjust
    *
    * This takes width and height sizes (in pixels) as input and a
    * size multiple (which is how many fingers you want to place
    * within the area, being "finger" the size set by
    * elm_finger_size_set()), and adjusts the size to be large enough
    * to accommodate the resulting size -- if it doesn't already
    * accommodate it. On return the @p w and @p h sizes pointed to by
    * these parameters will be modified, on those conditions.
    *
    * @note This is kind of a low level Elementary call, most useful
    * on size evaluation times for widgets. An external user wouldn't
    * be calling, most of the time.
    *
    * @ingroup Fingers
    */
   EAPI void             elm_coords_finger_size_adjust(int times_w, Evas_Coord *w, int times_h, Evas_Coord *h);

   /**
    * @}
    */

