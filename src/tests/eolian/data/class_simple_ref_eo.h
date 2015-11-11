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
 *
 * @ingroup Class_Simple
 */
EOAPI Eina_Bool evas_obj_simple_a_set(int value);
#endif

#ifdef CLASS_SIMPLE_BETA
/**
 * @brief Common desc for a
 *
 * @return Value description
 *
 * @ingroup Class_Simple
 */
EOAPI int evas_obj_simple_a_get(void);
#endif

EOAPI void evas_obj_simple_b_set(void);

#ifdef CLASS_SIMPLE_BETA
/**
 * @brief comment foo
 *
 * @param[in,out] b
 * @param[out] c
 *
 * @return comment for method return
 *
 * @ingroup Class_Simple
 */
EOAPI char *evas_obj_simple_foo(int a, char *b, double *c);
#endif

EOAPI int evas_obj_simple_bar(int x);


#endif
