#ifndef SIMPLE_H
#define SIMPLE_H

extern EAPI Eo_Op SIMPLE_BASE_ID;

enum {
     SIMPLE_SUB_ID_A_SET,
     SIMPLE_SUB_ID_LAST
};

typedef struct
{
   int a;
} Simple_Public_Data;

#define SIMPLE_ID(sub_id) (SIMPLE_BASE_ID + sub_id)

#define simple_a_set(a) SIMPLE_ID(SIMPLE_SUB_ID_A_SET), EO_TYPECHECK(int, a)

#define SIMPLE_CLASS simple_class_get()
const Eo_Class *simple_class_get(void);

#endif
