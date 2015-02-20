/**
 * @addtogroup Image
 *
 * @{
 */

/**
 * Using Evas_Image_Orient enums
 * @since 1.14
 */
typedef Evas_Image_Orient Elm_Image_Orient;

#define ELM_IMAGE_ORIENT_NONE     EVAS_IMAGE_ORIENT_NONE
#define ELM_IMAGE_ORIENT_0        EVAS_IMAGE_ORIENT_0
#define ELM_IMAGE_ROTATE_90       EVAS_IMAGE_ORIENT_90
#define ELM_IMAGE_ORIENT_90       EVAS_IMAGE_ORIENT_90
#define ELM_IMAGE_ROTATE_180      EVAS_IMAGE_ORIENT_180
#define ELM_IMAGE_ORIENT_180      EVAS_IMAGE_ORIENT_180
#define ELM_IMAGE_ROTATE_270      EVAS_IMAGE_ORIENT_270
#define ELM_IMAGE_ORIENT_270      EVAS_IMAGE_ORIENT_270
#define ELM_IMAGE_FLIP_HORIZONTAL EVAS_IMAGE_FLIP_HORIZONTAL
#define ELM_IMAGE_FLIP_VERTICAL   EVAS_IMAGE_FLIP_VERTICAL
#define ELM_IMAGE_FLIP_TRANSPOSE  EVAS_IMAGE_FLIP_TRANSPOSE
#define ELM_IMAGE_FLIP_TRANSVERSE EVAS_IMAGE_FLIP_TRANSVERSE

/**
 * Structure associated with smart callback 'download,progress'.
 * @since 1.8
 */
typedef struct _Elm_Image_Progress Elm_Image_Progress;
/**
 * Structure associated with smart callback 'download,progress'.
 */
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
/**
 * Structure associated with smart callback 'download,progress'.
 */
struct _Elm_Image_Error
{
   int status;

   Eina_Bool open_error;
};

/**
 * @}
 */
