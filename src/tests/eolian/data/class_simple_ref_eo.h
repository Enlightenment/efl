#ifndef _EOLIAN_CLASS_SIMPLE_EO_H_
#define _EOLIAN_CLASS_SIMPLE_EO_H_

#ifndef _CLASS_SIMPLE_EO_CLASS_TYPE
#define _CLASS_SIMPLE_EO_CLASS_TYPE

typedef Eo Class_Simple;

#endif

#ifndef _CLASS_SIMPLE_EO_TYPES
#define _CLASS_SIMPLE_EO_TYPES

#ifndef FOO
/** doc for constant
 *
 * @ingroup Foo
 */
#define FOO 5
#endif

/** doc for global
 *
 * @ingroup Bar
 */
EWAPI extern float BAR;

/** in header but not in source
 *
 * @ingroup Baz
 */
EWAPI extern long BAZ;


#endif
/** Class Desc Simple
 *
 * @ingroup Class_Simple
 */
#define CLASS_SIMPLE_CLASS class_simple_class_get()

EWAPI const Efl_Class *class_simple_class_get(void);

#ifdef EFL_BETA_API_SUPPORT
/**
 * @brief Common desc for a
 *
 * comment a.set
 *
 * @param[in] obj The object.
 * @param[in] value Value description
 *
 * @return comment for property set return
 *
 * @ingroup Class_Simple
 */
EOAPI Eina_Bool efl_canvas_object_simple_a_set(Eo *obj, int value);
#endif /* EFL_BETA_API_SUPPORT */

#ifdef EFL_BETA_API_SUPPORT
/**
 * @brief Common desc for a
 *
 * @param[in] obj The object.
 *
 * @return Value description
 *
 * @ingroup Class_Simple
 */
EOAPI int efl_canvas_object_simple_a_get(const Eo *obj);
#endif /* EFL_BETA_API_SUPPORT */

EOAPI void efl_canvas_object_simple_b_set(Eo *obj);

#ifdef EFL_BETA_API_SUPPORT
/**
 * @brief comment foo
 *
 * @param[in] obj The object.
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
#endif /* EFL_BETA_API_SUPPORT */

EOAPI int *efl_canvas_object_simple_bar(Eo *obj, int x);

#endif
