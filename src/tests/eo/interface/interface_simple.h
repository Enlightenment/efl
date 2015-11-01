#ifndef SIMPLE_H
#define SIMPLE_H

EAPI void simple_a_set(Eo const* ___object, int a);
EAPI int simple_a_get(Eo const* ___object);
EAPI void simple_b_set(Eo const* ___object, int b);
EAPI int simple_b_get(Eo const* ___object);

EAPI void eo_super_simple_a_set(Eo_Class const* ___klass, Eo const* ___object, int a);
EAPI int eo_super_simple_a_get(Eo_Class const* ___klass, Eo const* ___object);
EAPI void eo_super_simple_b_set(Eo_Class const* ___klass, Eo const* ___object, int b);
EAPI int eo_super_simple_b_get(Eo_Class const* ___klass, Eo const* ___object);

#define SIMPLE_CLASS simple_class_get()
const Eo_Class *simple_class_get(void);

#endif
