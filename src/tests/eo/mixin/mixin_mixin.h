#ifndef MIXIN_H
#define MIXIN_H

EAPI int mixin_ab_sum_get(void);

#define MIXIN_CLASS mixin_class_get()
const Eo_Class *mixin_class_get(void);

#endif
