#ifndef _EOLIAN_OUTPUT_H_
#define _EOLIAN_OUTPUT_H_

#ifndef _CLASS_SIMPLE_EO_CLASS_TYPE
#define _CLASS_SIMPLE_EO_CLASS_TYPE

typedef Eo Class_Simple;

#endif

#ifndef _CLASS_SIMPLE_EO_TYPES
#define _CLASS_SIMPLE_EO_TYPES


#endif
/** Class Desc Simple */
#define CLASS_SIMPLE_CLASS class_simple_class_get()

EAPI const Eo_Class *class_simple_class_get(void) EINA_CONST;

#ifdef CLASS_SIMPLE_BETA
/**
 * @brief Common desc for a
 *
 * comment a.set
 *
 * @param[in] value Value description
 *
 * @return comment for property set return
 */
EOAPI Eina_Bool  evas_obj_simple_a_set(int value);
#endif

#ifdef CLASS_SIMPLE_BETA
/**
 * @brief Common desc for a
 *
 * @return Value description
 */
EOAPI int  evas_obj_simple_a_get(void);
#endif

/**
 * No description supplied.
 */
EOAPI void  evas_obj_simple_b_set(void);

#ifdef CLASS_SIMPLE_BETA
/**
 * @brief comment foo
 *
 * @param[inout] b
 * @param[out] c
 *
 * @return comment for method return
 */
EOAPI char * evas_obj_simple_foo(int a, char *b, double *c);
#endif

/**
 * No description supplied.
 *
 * @param[in] x No description supplied.
 */
EOAPI int  evas_obj_simple_bar(int x);


#endif
