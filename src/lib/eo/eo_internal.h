#ifndef _EO_INTERNAL_H_
#define _EO_INTERNAL_H_

#include <Eo.h>

typedef unsigned char Eina_Bool;
typedef struct _Eo_Opaque Eo;
typedef struct _Efl_Event_Description Efl_Event_Description;

/**
 * Don't use this.
 * The values of the returned event structure are also internal, don't assume
 * anything about them.
 * @internal
 */
EO_API const Efl_Event_Description *efl_object_legacy_only_event_description_get(const char *_event_name);

EO_API void ___efl_auto_unref_set(Eo *obj_id, Eina_Bool enable);

EO_API int ___efl_ref2_count(const Eo *obj_id);
EO_API void ___efl_ref2_reset(const Eo *obj_id);

#define EFL_CLASS_SIMPLE_CLASS(FUNC, NAME, ABSTRACT_CLASS) \
static const Efl_Class_Description FUNC ##_realized_class_desc = { \
     EO_VERSION, \
     NAME "_Realized", \
     EFL_CLASS_TYPE_REGULAR, \
     0, NULL, NULL, NULL \
}; \
EFL_DEFINE_CLASS(FUNC ##_realized_class_get, &FUNC ##_realized_class_desc, ABSTRACT_CLASS, NULL)

#endif
