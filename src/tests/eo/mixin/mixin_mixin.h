#ifndef MIXIN_H
#define MIXIN_H

EAPI int mixin_ab_sum_get(Eo const* ___object);
EAPI int eo_super_mixin_ab_sum_get(Eo_Class const* ___klass, Eo const* ___object);

#define MIXIN_CLASS mixin_class_get()
const Eo_Class *mixin_class_get(void);

#endif
