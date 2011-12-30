   /**
    * @defgroup Colorselector Colorselector
    *
    * @{
    *
    * @image html img/widget/colorselector/preview-00.png
    * @image latex img/widget/colorselector/preview-00.eps
    *
    * @brief Widget for user to select a color.
    *
    * Signals that you can add callbacks for are:
    * "changed" - When the color value changes(event_info is NULL).
    *
    * See @ref tutorial_colorselector.
    */
   /**
    * @brief Add a new colorselector to the parent
    *
    * @param parent The parent object
    * @return The new object or NULL if it cannot be created
    *
    * @ingroup Colorselector
    */
   EAPI Evas_Object *elm_colorselector_add(Evas_Object *parent) EINA_ARG_NONNULL(1);
   /**
    * Set a color for the colorselector
    *
    * @param obj   Colorselector object
    * @param r     r-value of color
    * @param g     g-value of color
    * @param b     b-value of color
    * @param a     a-value of color
    *
    * @ingroup Colorselector
    */
   EAPI void         elm_colorselector_color_set(Evas_Object *obj, int r, int g , int b, int a) EINA_ARG_NONNULL(1);
   /**
    * Get a color from the colorselector
    *
    * @param obj   Colorselector object
    * @param r     integer pointer for r-value of color
    * @param g     integer pointer for g-value of color
    * @param b     integer pointer for b-value of color
    * @param a     integer pointer for a-value of color
    *
    * @ingroup Colorselector
    */
   EAPI void         elm_colorselector_color_get(const Evas_Object *obj, int *r, int *g , int *b, int *a) EINA_ARG_NONNULL(1);
   /**
    * @}
    */

