#ifndef _EDJE_EDIT_EO_H_
#define _EDJE_EDIT_EO_H_

#ifndef _EDJE_EDIT_EO_CLASS_TYPE
#define _EDJE_EDIT_EO_CLASS_TYPE

typedef Eo Edje_Edit;

#endif

#ifndef _EDJE_EDIT_EO_TYPES
#define _EDJE_EDIT_EO_TYPES


#endif
/**
 * @brief Edje editing class to access edje object internals.
 *
 * This was intended ONLY for use in an actual edje editor program. Unless you
 * are writing one of these, do NOT use this API here.
 *
 * @ingroup Edje_Edit
 */
#define EDJE_EDIT_CLASS edje_edit_class_get()

EWAPI const Efl_Class *edje_edit_class_get(void);

#endif
