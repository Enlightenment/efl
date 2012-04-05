#ifndef ELW_BUTTON_H
#define ELW_BUTTON_H

#include "eobj.h"

extern EAPI Eobj_Op ELW_BUTTON_BASE_ID;

enum {
     ELW_BUTTON_SUB_ID_TEXT_SET,
     ELW_BUTTON_SUB_ID_LAST
};

#define ELW_BUTTON_ID(sub_id) (ELW_BUTTON_BASE_ID + sub_id)

/* FIXME Doesn't belong here, but just for the example... */
#define ELW_BUTTON_TEXT_SET(obj) ELW_BUTTON_ID(ELW_BUTTON_SUB_ID_TEXT_SET), EOBJ_TYPECHECK(const char *, obj)

extern const Eobj_Event_Description _SIG_CLICKED;
#define SIG_CLICKED (&(_SIG_CLICKED))

#define ELW_BUTTON_CLASS elw_button_class_get()
const Eobj_Class *elw_button_class_get(void) EINA_CONST;

#endif
