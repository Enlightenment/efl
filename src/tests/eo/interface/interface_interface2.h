#ifndef INTERFACE2_H
#define INTERFACE2_H

EAPI int interface2_ab_sum_get2(Eo *obj);

#define INTERFACE2_CLASS interface2_class_get()
const Efl_Class *interface2_class_get(void);

#endif
