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
EAPI Eina_Bool evas_object_simple_a_set(Class_Simple *obj, int value);

/**
 * @brief Common desc for a
 *
 * @return Value description
 *
 * @ingroup Class_Simple
 */
EAPI int evas_object_simple_a_get(const Class_Simple *obj);
EAPI void evas_object_simple_b_set(Class_Simple *obj);

/**
 * @brief comment foo
 *
 * @param[in,out] b
 * @param[out] c
 * @param[in] d
 *
 * @return comment for method return
 *
 * @ingroup Class_Simple
 */
EAPI char *evas_object_simple_foo(Class_Simple *obj, int a, char *b, double *c, int *d);
EAPI int *evas_object_simple_bar(Class_Simple *obj, int x);

#endif
