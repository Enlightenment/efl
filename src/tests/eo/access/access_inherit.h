#ifndef INHERIT_H
#define INHERIT_H

extern EAPI Eo_Op INHERIT_BASE_ID;

enum {
     INHERIT_SUB_ID_PROT_PRINT,
     INHERIT_SUB_ID_LAST
};

#define INHERIT_ID(sub_id) (INHERIT_BASE_ID + sub_id)

#define inherit_prot_print() INHERIT_ID(INHERIT_SUB_ID_PROT_PRINT)

#define INHERIT_CLASS inherit_class_get()
const Eo_Class *inherit_class_get(void);

#endif
