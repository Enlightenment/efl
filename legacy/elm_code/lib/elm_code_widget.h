#ifndef ELM_CODE_WIDGET_H_
# define ELM_CODE_WIDGET_H_

#include <Eina.h>
#include <Evas.h>
#include "elm_code_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file
 * @brief These routines are used for rendering instances of Elm Code.
 */

/**
 * @brief UI Loading functions.
 * @defgroup Init  Creating a widget to render an Elm Code backend
 *
 * @{
 *
 * Functions for UI loading.
 *
 */

EAPI Evas_Object *elm_code_widget_add(Evas_Object *parent, Elm_Code *code);
EAPI void elm_code_widget_fill(Evas_Object *o, Elm_Code *code);                  


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ELM_CODE_WIDGET_H_ */
