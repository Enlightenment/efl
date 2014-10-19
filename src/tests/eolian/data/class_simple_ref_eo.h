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
#define CLASS_SIMPLE_CLASS class_simple_class_get()

const Eo_Class *class_simple_class_get(void) EINA_CONST;

/**
 *
 * Common desc for a
 * comment a.set
 *
 * @param[in] value Value description
 *
 */
EOAPI Eina_Bool  evas_obj_simple_a_set(int value);

/**
 *
 * Common desc for a
 * 
 *
 *
 */
EOAPI int  evas_obj_simple_a_get(void);

/**
 *
 * No description supplied.
 *
 *
 */
EOAPI void  evas_obj_simple_b_set(void);

/**
 *
 * comment foo
 * 
 *
 * @param[in] a a
 * @param[inout] b No description supplied.
 * @param[out] c No description supplied.
 *
 */
EOAPI char * evas_obj_simple_foo(int a, char *b, double *c);

/**
 *
 * No description supplied.
 *
 * @param[in] x No description supplied.
 *
 */
EOAPI int  evas_obj_simple_bar(int x);


#endif
