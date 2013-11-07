#ifndef MIXIN_H
#define MIXIN_H

EAPI void mixin_add_and_print(int x);

#define MIXIN_CLASS mixin_class_get()
const Eo_Class *mixin_class_get(void);

#endif
