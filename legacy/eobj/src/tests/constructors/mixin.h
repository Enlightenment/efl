#ifndef MIXIN_H
#define MIXIN_H

extern EAPI Eo_Op MIXIN_BASE_ID;

enum {
     MIXIN_SUB_ID_ADD_AND_SET,
     MIXIN_SUB_ID_LAST
};

#define MIXIN_ID(sub_id) (MIXIN_BASE_ID + sub_id)

#define mixin_add_and_print(x) MIXIN_ID(MIXIN_SUB_ID_ADD_AND_SET), EO_TYPECHECK(int, x)

#define MIXIN_CLASS mixin_class_get()
const Eo_Class *mixin_class_get(void);

#endif
