#ifndef INHERIT2_H
#define INHERIT2_H

extern EAPI Eo_Op INHERIT2_BASE_ID;

enum {
     INHERIT2_SUB_ID_PRINT,
     INHERIT2_SUB_ID_PRINT2,
     INHERIT2_SUB_ID_LAST
};

#define INHERIT2_ID(sub_id) (INHERIT2_BASE_ID + sub_id)

#define inherit2_print() INHERIT2_ID(INHERIT2_SUB_ID_PRINT)
#define inherit2_print2() INHERIT2_ID(INHERIT2_SUB_ID_PRINT2)

#define INHERIT2_CLASS inherit2_class_get()
const Eo_Class *inherit2_class_get(void);

#endif
