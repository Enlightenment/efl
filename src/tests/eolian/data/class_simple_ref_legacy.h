#ifndef _EOLIAN_CLASS_SIMPLE_LEGACY_H_
#define _EOLIAN_CLASS_SIMPLE_LEGACY_H_

#ifndef _CLASS_SIMPLE_EO_CLASS_TYPE
#define _CLASS_SIMPLE_EO_CLASS_TYPE

typedef Eo Class_Simple;

#endif

#ifndef _CLASS_SIMPLE_EO_TYPES
#define _CLASS_SIMPLE_EO_TYPES

#ifndef Foo
/** doc for constant
 *
 * @ingroup Foo
 */
#define Foo 5
#endif

/** doc for global
 *
 * @ingroup Bar
 */
extern float Bar;

/** in header but not in source
 *
 * @ingroup Baz
 */
extern long Baz;


#endif

EAPI void evas_object_simple_b_set(Class_Simple *obj);

EAPI int *evas_object_simple_bar(Class_Simple *obj, int x);

#endif
