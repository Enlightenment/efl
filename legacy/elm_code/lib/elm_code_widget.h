#ifndef ELM_CODE_WIDGET_H_
# define ELM_CODE_WIDGET_H_

EAPI extern const Eo_Event_Description ELM_CODE_WIDGET_EVENT_LINE_CLICKED;

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

/**
 * @}
 *
 * @brief UI Manipulation functions.
 * @defgroup UI  Manage aspects of an Elm_Code widget
 *
 * @{
 *
 * Functions for UI manipulation.
 *
 */

/**
 * Set the font size of a widget
 *
 * Change the size of the monospaced font used by this widget instance.
 * 10 pt is the default font size for an elm_code widget.
 *
 * @param widget The widget to change
 * @param size The font size to set on this widget
 *
 * @ingroup UI
 */
EAPI void elm_code_widget_font_size_set(Evas_Object *widget, Evas_Font_Size size);

/**
 * Set this widget to be editable
 *
 * An editable widget displays a cursor and accepts user input.
 * It will also accept focus.
 * If EINA_FALSE is passed this widget will return to being read-only.
 * EINA_FALSE is the default value for editable.
 *
 * @param widget The widget to change
 * @param editable Whether or not this widget should be editable
 *
 * @ingroup UI
 */
EAPI void elm_code_widget_editable_set(Evas_Object *widget, Eina_Bool editable);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ELM_CODE_WIDGET_H_ */
