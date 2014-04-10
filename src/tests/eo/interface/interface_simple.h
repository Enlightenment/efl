#ifndef SIMPLE_H
#define SIMPLE_H

EAPI void simple_a_set(int a);
EAPI int simple_a_get(void);
EAPI void simple_b_set(int b);
EAPI int simple_b_get(void);

#define SIMPLE_CLASS simple_class_get()
const Eo_Class *simple_class_get(void);

#endif
