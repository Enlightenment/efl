typedef Eo Elm_Bg;

/**
 * @brief Identifiers on how a background widget is to display its image
 *
 * -- if it was set to use an image file.
 *
 * @ingroup Elm_Bg
 */
 typedef enum
 {
   ELM_BG_OPTION_CENTER = 0, /**< Center the background image. */
   ELM_BG_OPTION_SCALE, /**< Scale the background image, retaining aspect ratio
                         * (default). */
   ELM_BG_OPTION_STRETCH, /**< Stretch the background image to fill the widget's
                           * area. */
   ELM_BG_OPTION_TILE, /**< Tile background image at its original size. */
   ELM_BG_OPTION_LAST /**< Sentinel value to indicate last enum field during
                       * iteration. Also used to indicate errors */
 } Elm_Bg_Option;

/**
 * @brief Adds a new background to the parent
 *
 * @param parent The parent object
 * @return The new object or @c NULL if it cannot be created
 *
 * @ingroup Elm_Bg
 */
EAPI Evas_Object                 *elm_bg_add(Evas_Object *parent);

/**
 * @brief Sets the color on a given background widget
 *
 * @param obj The background object handle
 * @param r The red color component's value
 * @param g The green color component's value
 * @param b The blue color component's value
 *
 * This sets the color used for the background rectangle, in RGB
 * format. Each color component's range is from 0 to 255.
 *
 * @note You probably only want to use this function if you haven't
 * previously called elm_bg_file_set(), so that you just want a solid
 * color background.
 *
 * @note You can reset the color by setting @p r, @p g, @p b as -1, -1, -1.
 *
 * @see elm_bg_color_get()
 *
 * @ingroup Elm_Bg
 */
EAPI void                         elm_bg_color_set(Evas_Object *obj, int r, int g, int b);

/**
 * @brief Gets the color set on a given background widget
 *
 * @param obj The background object handle
 * @param r Where to store the red color component's value
 * @param g Where to store the green color component's value
 * @param b Where to store the blue color component's value
 *
 * @note Use @c NULL pointers on the file components you're not
 * interested in: they'll be ignored by the function.
 *
 * @see elm_bg_color_get() for more details
 *
 * @ingroup Elm_Bg
 */
EAPI void                         elm_bg_color_get(const Evas_Object *obj, int *r, int *g, int *b);

/**
 * @brief Sets the file (image or edje collection) to give life for the
 * background
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * This sets the image file used in the background object. If the
 * image comes from an Edje group, it will be stretched to completely
 * fill the background object. If it comes from a traditional image file, it
 * will by default be centered in this widget's area (thus retaining
 * its aspect), what could lead to some parts being not visible. You
 * may change the mode of exhibition for a real image file with
 * elm_bg_option_set().
 *
 * @note Once the image of @p obj is set, a previously set one will be
 * deleted, even if @p file is @c NULL.
 *
 * @note This will only affect the contents of one of the background's
 * swallow spots, namely @c "elm.swallow.background". If you want to
 * achieve the @c Layout's file setting behavior, you'll have to call
 * that method on this object.
 *
 * @ingroup Elm_Bg
 *
 * @param[in] obj The background object handle
 * @param[in] file The file path
 * @param[in] group Optional key (group in Edje) within the file
 */
EAPI Eina_Bool elm_bg_file_set(Eo *obj, const char *file, const char *group);

/**
 * @brief Gets the file (image or edje collection) set on a given background
 * widget
 *
 * @note Use @c NULL pointers on the file components you're not
 * interested in: they'll be ignored by the function.
 *
 * @ingroup Elm_Bg
 *
 * @param[in] obj The background object handle
 * @param[out] file The file path
 * @param[out] group Optional key (group in Edje) within the file
 */
EAPI void elm_bg_file_get(const Eo *obj, const char **file, const char **group);

/**
  * @brief Sets the mode of display for a given background widget's image
  *
  * This sets how the background widget will display its image. This
  * will only work if the elm_bg_file_set() was previously called with
  * an image file on $obj. The image can be display tiled, scaled,
  * centered or stretched. #ELM_BG_OPTION_SCALE by default.
  *
  * @param[in] option The desired background option(See Elm.Bg.Option).
  *
  * @see elm_bg_option_get()
  *
  * @ingroup Elm_Bg
  */
EAPI void elm_bg_option_set(Evas_Object *obj, Elm_Bg_Option option);

/**
  * @brief Gets the mode of display of a given background widget's image
  *
  * @return  The background option Elm.Bg.Option Default is #Elm_Bg_SCALE.
  *
  * @see elm_bg_option_set()
  *
  * @ingroup Elm_Bg
  */
EAPI Elm_Bg_Option elm_bg_option_get(const Evas_Object *obj);

/**
  * @brief Sets the size of the pixmap representation of the image set on a
  * given background widget.
  *
  * @warning: This function just makes sense if an image file was set on
  * @p obj, with elm_bg_file_set().
  *
  * This function sets a new size for pixmap representation of the
  * given bg image. It allows for the image to be loaded already in the
  * specified size, reducing the memory usage and load time (for
  * example, when loading a big image file with its load size set to a
  * smaller size)
  *
  * @note: This is just a hint for the underlying system. The real size
  * of the pixmap may differ depending on the type of image being
  * loaded, being bigger than requested.
  *
  * @ingroup Elm_Bg
  *
  * @param[in] w The new width of the image pixmap representation
  * @param[in] h The new height of the image pixmap representation
  */
EAPI void elm_bg_load_size_set(Evas_Object *obj, int w, int h);

#include "efl_ui_bg.eo.legacy.h"
