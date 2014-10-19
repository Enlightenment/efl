#ifndef _EOLIAN_OUTPUT_H_
#define _EOLIAN_OUTPUT_H_

#ifndef _CLASS_SIMPLE_EO_CLASS_TYPE
#define _CLASS_SIMPLE_EO_CLASS_TYPE

typedef Eo Class_Simple;

#endif

#ifndef _CLASS_SIMPLE_EO_TYPES
#define _CLASS_SIMPLE_EO_TYPES


#endif
/**
 * Class Desc Simple
 */

/**
 *
 * Common desc for a
 * comment a.set
 *
 * @param[in] value Value description
 */
EAPI Eina_Bool evas_object_simple_a_set(Class_Simple *obj, int value);

/**
 *
 * Common desc for a
 * 
 *
 */
EAPI int evas_object_simple_a_get(const Class_Simple *obj);

/**
 *
 * No description supplied.
 *
 */
EAPI void evas_object_simple_b_set(Class_Simple *obj);

/**
 *
 * comment foo
 * 
 *
 * @param[in] a a
 * @param[inout] b No description supplied.
 * @param[out] c No description supplied.
 */
EAPI char *evas_object_simple_foo(Class_Simple *obj, int a, char *b, double *c);

/**
 *
 * No description supplied.
 *
 * @param[in] x No description supplied.
 */
EAPI int evas_object_simple_bar(Class_Simple *obj, int x);

#endif
