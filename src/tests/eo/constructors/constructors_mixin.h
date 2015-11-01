#ifndef MIXIN_H
#define MIXIN_H

EAPI void mixin_add_and_print(Eo const* ___object, int x);
EAPI void eo_super_mixin_add_and_print(Eo_Class const* ___klass, Eo const* ___object, int x);

#define MIXIN_CLASS mixin_class_get()
const Eo_Class *mixin_class_get(void);

#endif
