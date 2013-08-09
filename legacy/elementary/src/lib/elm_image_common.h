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
 * Structure associated with smart callback 'download,progress'.
 * @since 1.8
 */
typedef struct _Elm_Image_Progress Elm_Image_Progress;

struct _Elm_Image_Progress
{
   double now;
   double total;
};


/**
 * Structre associated with smart callback 'download,error'
 * @since 1.8
 */
typedef struct _Elm_Image_Error Elm_Image_Error;
struct _Elm_Image_Error
{
   int status;

   Eina_Bool open_error;
};
