#ifndef _EVAS_IMAGE_EO_H_
#define _EVAS_IMAGE_EO_H_

#ifndef _EVAS_IMAGE_EO_CLASS_TYPE
#define _EVAS_IMAGE_EO_CLASS_TYPE

typedef Eo Evas_Image;

#endif

#ifndef _EVAS_IMAGE_EO_TYPES
#define _EVAS_IMAGE_EO_TYPES


#endif
/** Internal class for legacy support of Evas Image.
 *
 * @ingroup Evas_Image
 */
#define EVAS_IMAGE_CLASS evas_image_class_get()

EWAPI const Efl_Class *evas_image_class_get(void);

#endif
