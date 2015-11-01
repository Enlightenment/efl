#ifndef INTERFACE_H
#define INTERFACE_H

EAPI int interface_ab_sum_get(Eo const* ___object);
EAPI int eo_super_interface_ab_sum_get(Eo_Class const* ___klass, Eo const* ___object);

#define INTERFACE_CLASS interface_class_get()
const Eo_Class *interface_class_get(void);

#endif
