#ifndef INTERFACE2_H
#define INTERFACE2_H

EAPI int interface2_ab_sum_get2(Eo const* ___object);
EAPI int eo_super_interface2_ab_sum_get2(Eo_Class const* ___klass, Eo const* ___object);

#define INTERFACE2_CLASS interface2_class_get()
const Eo_Class *interface2_class_get(void);

#endif
