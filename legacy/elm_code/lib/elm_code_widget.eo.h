#ifndef _ELM_CODE_WIDGET_EO_H_
#define _ELM_CODE_WIDGET_EO_H_

#ifndef _ELM_CODE_WIDGET_EO_CLASS_TYPE
#define _ELM_CODE_WIDGET_EO_CLASS_TYPE

typedef Eo Elm_Code_Widget;

#endif

#ifndef _ELM_CODE_WIDGET_EO_TYPES
#define _ELM_CODE_WIDGET_EO_TYPES


#endif
#define ELM_CODE_WIDGET_CLASS elm_code_widget_class_get()

const Eo_Class *elm_code_widget_class_get(void) EINA_CONST;

/**
 *
 * Set the underlying code object that this widget renders
 *
 * @ingroup Data
 *
 * @param[in] code Our underlying Elm_Code object
 *
 */
EOAPI void  elm_code_widget_code_set(Elm_Code *code);

/**
 *
 * Get the underlying code object we are rendering
 *
 * @ingroup Data
 *
 *
 */
EOAPI Elm_Code * elm_code_widget_code_get(void);

/**
 *
 * Set the font size that this widget uses, the font will always be a system monospaced font
 *
 * @ingroup Style
 *
 * @param[in] font_size The font size of the widgget
 *
 */
EOAPI void  elm_code_widget_font_size_set(Evas_Font_Size font_size);

/**
 *
 * Get the font size currently in use
 *
 * @ingroup Style
 *
 *
 */
EOAPI Evas_Font_Size  elm_code_widget_font_size_get(void);

/**
 *
 * Set whether this widget allows editing
 *
 * If @a editable then the widget will allow user input to manipulate
 * the underlying Elm_Code_File of this Elm_Code instance.
 * Any other Elm_Code_Widget's connected to this Elm_Code will
 * update to reflect the changes.
 *
 * @ingroup Features
 *
 * @param[in] editable The editable state of the widget
 *
 */
EOAPI void  elm_code_widget_editable_set(Eina_Bool editable);

/**
 *
 * Get the current editable state of this widget
 *
 * @return EINA_TRUE if the widget is editable, EINA_FALSE otherwise.
 * If this widget is not editable the underlying Elm_Code_File could
 * still be manipulated by a different widget or the filesystem.
 *
 * @ingroup Features
 *
 *
 */
EOAPI Eina_Bool  elm_code_widget_editable_get(void);

EOAPI extern const Eo_Event_Description _ELM_CODE_WIDGET_EVENT_LINE_CLICKED;

/**
 * No description
 */
#define ELM_CODE_WIDGET_EVENT_LINE_CLICKED (&(_ELM_CODE_WIDGET_EVENT_LINE_CLICKED))

#endif
