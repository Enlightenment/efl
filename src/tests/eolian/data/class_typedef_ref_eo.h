#ifndef _EOLIAN_CLASS_TYPEDEF_EO_H_
#define _EOLIAN_CLASS_TYPEDEF_EO_H_

#ifndef _CLASS_TYPEDEF_EO_CLASS_TYPE
#define _CLASS_TYPEDEF_EO_CLASS_TYPE

#ifndef _CLASS_TYPEDEF_EO_CLASS_TYPE_FWD
#define _CLASS_TYPEDEF_EO_CLASS_TYPE_FWD
typedef Eo Class_Typedef;
#endif
#ifndef _CLASS_SIMPLE_EO_CLASS_TYPE_FWD
#define _CLASS_SIMPLE_EO_CLASS_TYPE_FWD
typedef Eo Class_Simple;
#endif
#ifndef _NMSP2_CLASS1_EO_CLASS_TYPE_FWD
#define _NMSP2_CLASS1_EO_CLASS_TYPE_FWD
typedef Eo nmsp2_class1;
#endif
#ifndef _NMSP1_NMSP11_CLASS2_EO_CLASS_TYPE_FWD
#define _NMSP1_NMSP11_CLASS2_EO_CLASS_TYPE_FWD
typedef Eo nmsp1_nmsp11_class2;
#endif
#ifndef _NO_NMSP_EO_CLASS_TYPE_FWD
#define _NO_NMSP_EO_CLASS_TYPE_FWD
typedef Eo no_nmsp;
#endif
#ifndef _EVENTS_EO_CLASS_TYPE_FWD
#define _EVENTS_EO_CLASS_TYPE_FWD
typedef Eo Events;
#endif

#endif

#ifndef _CLASS_TYPEDEF_EO_TYPES
#define _CLASS_TYPEDEF_EO_TYPES


#endif
/** Docs for class Class_Typedef.
 *
 * @since 1.66
 *
 * @ingroup Class_Typedef
 */
#define CLASS_TYPEDEF_CLASS class_typedef_class_get()

EAPI EAPI_WEAK const Efl_Class *class_typedef_class_get(void) EINA_CONST;

/**
 * @brief Names a class in a key and in a value.
 *
 * @param[in] obj The object.
 * @param[in] key
 * @param[in] value
 *
 * @since 1.66
 *
 * @ingroup Class_Typedef
 */
EAPI EAPI_WEAK void class_typedef_linked_set(Eo *obj, nmsp2_class1 *key, nmsp1_nmsp11_class2 *value);

/**
 * @brief Names a class in a key and in a value.
 *
 * @param[in] obj The object.
 * @param[in] key
 *
 * @since 1.66
 *
 * @ingroup Class_Typedef
 */
EAPI EAPI_WEAK nmsp1_nmsp11_class2 *class_typedef_linked_get(const Eo *obj, nmsp2_class1 *key);

/**
 * @brief Names a class in a setter only.
 *
 * @param[in] obj The object.
 * @param[in] value
 *
 * @since 1.66
 *
 * @ingroup Class_Typedef
 */
EAPI EAPI_WEAK void class_typedef_only_set_set(Eo *obj, no_nmsp *value);

/** Names a class as a return type.
 *
 * @since 1.66
 *
 * @ingroup Class_Typedef
 */
EAPI EAPI_WEAK Class_Simple *class_typedef_make(Eo *obj);

EAPI EAPI_WEAK extern const Efl_Event_Description _CLASS_TYPEDEF_EVENT_LINKED;

/** Names a class as an event type.
 * @return Events *
 *
 * @since 1.66
 *
 * @ingroup Class_Typedef
 */
#define CLASS_TYPEDEF_EVENT_LINKED (&(_CLASS_TYPEDEF_EVENT_LINKED))

#endif
