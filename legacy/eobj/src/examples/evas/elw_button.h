#ifndef ELW_BUTTON_H
#define ELW_BUTTON_H

#include "Eo.h"

extern EAPI Eo_Op ELW_BUTTON_BASE_ID;

enum {
     ELW_BUTTON_SUB_ID_TEXT_SET,
     ELW_BUTTON_SUB_ID_LAST
};

#define ELW_BUTTON_ID(sub_id) (ELW_BUTTON_BASE_ID + sub_id)

/**
 * @def elw_button_text_set(text)
 * @brief Set button text
 * @param[in] text text to assing to button
 * FIXME Doesn't belong here, but just for the example...
 */
#define elw_button_text_set(text) ELW_BUTTON_ID(ELW_BUTTON_SUB_ID_TEXT_SET), EO_TYPECHECK(const char *, text)

extern const Eo_Event_Description _EV_CLICKED;
#define EV_CLICKED (&(_EV_CLICKED))

#define ELW_BUTTON_CLASS elw_button_class_get()
const Eo_Class *elw_button_class_get(void);

#endif
