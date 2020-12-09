#ifndef INTERFACE_H
#define INTERFACE_H

int interface_ab_sum_get(Eo *obj);

#define INTERFACE_CLASS interface_class_get()
const Efl_Class *interface_class_get(void);

#endif
