#ifndef MIXIN_H
#define MIXIN_H

int mixin_ab_sum_get(Eo *obj);

#define MIXIN_CLASS mixin_class_get()
const Efl_Class *mixin_class_get(void);

#endif
