#ifndef _ELM_ATSPI_APP_OBJECT_EO_H_
#define _ELM_ATSPI_APP_OBJECT_EO_H_

#ifndef _ELM_ATSPI_APP_OBJECT_EO_CLASS_TYPE
#define _ELM_ATSPI_APP_OBJECT_EO_CLASS_TYPE

typedef Eo Elm_Atspi_App_Object;

#endif

#ifndef _ELM_ATSPI_APP_OBJECT_EO_TYPES
#define _ELM_ATSPI_APP_OBJECT_EO_TYPES


#endif
/** AT-SPI application object class
 *
 * @ingroup Elm_Atspi_App_Object
 */
#define ELM_ATSPI_APP_OBJECT_CLASS elm_atspi_app_object_class_get()

EWAPI const Efl_Class *elm_atspi_app_object_class_get(void);

#endif
