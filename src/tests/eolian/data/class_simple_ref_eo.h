#ifndef _EOLIAN_OUTPUT_H_
#define _EOLIAN_OUTPUT_H_

#ifndef _CLASS_SIMPLE_EO_CLASS_TYPE
#define _CLASS_SIMPLE_EO_CLASS_TYPE

typedef Eo Class_Simple;

#endif

#ifndef _CLASS_SIMPLE_EO_TYPES
#define _CLASS_SIMPLE_EO_TYPES


#endif
/** Class Desc Simple
 *
 * @ingroup Class_Simple
 */
#define CLASS_SIMPLE_CLASS class_simple_class_get()

EWAPI const Eo_Class *class_simple_class_get(void);

#ifdef CLASS_SIMPLE_BETA
/**
 * @brief Common desc for a
 *
 * comment a.set
 *
 * @param[in] value Value description
 *
 * @return comment for property set return
 *
 * @ingroup Class_Simple
 */
EOAPI Eina_Bool efl_canvas_object_simple_a_set(Eo *obj, int value);
#endif

#ifdef CLASS_SIMPLE_BETA
/**
 * @brief Common desc for a
 *
 * @return Value description
 *
 * @ingroup Class_Simple
 */
EOAPI int efl_canvas_object_simple_a_get(const Eo *obj);
#endif

EOAPI void efl_canvas_object_simple_b_set(Eo *obj);

#ifdef CLASS_SIMPLE_BETA
/**
 * @brief comment foo
 *
 * @param[in] a a
 * @param[in,out] b
 * @param[out] c
 * @param[in] d
 *
 * @return comment for method return
 *
 * @ingroup Class_Simple
 */
EOAPI char *efl_canvas_object_simple_foo(Eo *obj, int a, char *b, double *c, int *d);
#endif

EOAPI int *efl_canvas_object_simple_bar(Eo *obj, int x);


#endif
