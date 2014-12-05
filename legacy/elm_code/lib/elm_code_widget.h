#ifndef ELM_CODE_WIDGET_H_
# define ELM_CODE_WIDGET_H_

#include <Evas.h>
#include "elm_code_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file
 * @brief These routines are used for rendering instances of Elm Code.
 */

typedef struct _Elm_Code_Widget
{
   Elm_Code *code;

   Evas_Font_Size font_size;
   unsigned int cursor_line, cursor_col;
   Eina_Bool editable;

} Elm_Code_Widget;

#define ELM_CODE_WIDGET_DATA_KEY "Elm_Code_Widget"
#define ELM_CODE_WIDGET_DATA_GET(o, ptr, ...)             \
  Elm_Code_Widget *ptr;                                   \
  ptr = evas_object_data_get(o, ELM_CODE_WIDGET_DATA_KEY);

#define ELM_CODE_WIDGET_DATA_GET_OR_RETURN(o, ptr, ...)   \
  Elm_Code_Widget *ptr;                                   \
  ptr = evas_object_data_get(o, ELM_CODE_WIDGET_DATA_KEY);\
  if (EINA_UNLIKELY(!ptr))                                \
    {                                                     \
       CRI("no widget data for object %p (%s)",           \
           o, evas_object_type_get(o));                   \
       return __VA_ARGS__;                                \
    }

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

EAPI void elm_code_widget_font_size_set(Evas_Object *widget, Evas_Font_Size size);

EAPI void elm_code_widget_editable_set(Evas_Object *widget, Eina_Bool editable);

EAPI void elm_code_widget_fill(Evas_Object *o, Elm_Code *code);

EAPI void elm_code_widget_fill_line_tokens(Evas_Textgrid_Cell *cells, unsigned int count, Elm_Code_Line *line);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ELM_CODE_WIDGET_H_ */
