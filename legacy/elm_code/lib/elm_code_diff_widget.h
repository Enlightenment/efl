#ifndef ELM_CODE_DIFF_WIDGET_H_
# define ELM_CODE_DIFF_WIDGET_H_

#include <Evas.h>
#include "elm_code_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file
 * @brief These routines are used for rendering diff instances of Elm Code.
 */

/**
 * @brief UI Loading functions.
 * @defgroup Init  Creating a diff widget to render an Elm Code backend
 *  when it's referencing a diff file
 *
 * @{
 *
 * Functions for Diff UI loading.
 *
 */

EAPI Evas_Object *elm_code_diff_widget_add(Evas_Object *parent, Elm_Code *code);

EAPI void elm_code_diff_widget_font_size_set(Evas_Object *widget, int size);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ELM_CODE_DIFF_WIDGET_H_ */
