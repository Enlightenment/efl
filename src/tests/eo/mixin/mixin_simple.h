#ifndef SIMPLE_H
#define SIMPLE_H

EAPI void simple_a_set(Eo *obj, int a);
EAPI int simple_a_get(Eo *obj);
EAPI void simple_b_set(Eo *obj, int b);
EAPI int simple_b_get(Eo *obj);

#define SIMPLE_CLASS simple_class_get()
const Efl_Class *simple_class_get(void);

#endif
