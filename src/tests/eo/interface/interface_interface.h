#ifndef INTERFACE_H
#define INTERFACE_H

EAPI int interface_ab_sum_get(void);

#define INTERFACE_CLASS interface_class_get()
const Eo_Class *interface_class_get(void);

#endif
