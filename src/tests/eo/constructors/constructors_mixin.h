#ifndef MIXIN_H
#define MIXIN_H

EAPI void mixin_add_and_print(Eo *obj, int x);

#define MIXIN_CLASS mixin_class_get()
const Efl_Class *mixin_class_get(void);

#endif
