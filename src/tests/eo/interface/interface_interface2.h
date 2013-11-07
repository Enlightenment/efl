#ifndef INTERFACE2_H
#define INTERFACE2_H

EAPI int interface2_ab_sum_get2(void);

#define INTERFACE2_CLASS interface2_class_get()
const Eo_Class *interface2_class_get(void);

#endif
