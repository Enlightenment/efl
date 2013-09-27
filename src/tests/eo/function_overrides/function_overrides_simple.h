#ifndef SIMPLE_H
#define SIMPLE_H

extern EAPI Eo_Op SIMPLE_BASE_ID;

enum {
     SIMPLE_SUB_ID_A_SET,
     SIMPLE_SUB_ID_A_PRINT,
     SIMPLE_SUB_ID_CLASS_PRINT,
     SIMPLE_SUB_ID_CLASS_PRINT2,
     SIMPLE_SUB_ID_LAST
};

typedef struct
{
   int a;
} Simple_Public_Data;

#define SIMPLE_ID(sub_id) (SIMPLE_BASE_ID + sub_id)

#define simple_a_set(a) SIMPLE_ID(SIMPLE_SUB_ID_A_SET), EO_TYPECHECK(int, a)
#define simple_a_print() SIMPLE_ID(SIMPLE_SUB_ID_A_PRINT)
#define simple_class_print() SIMPLE_ID(SIMPLE_SUB_ID_CLASS_PRINT)
#define simple_class_print2() SIMPLE_ID(SIMPLE_SUB_ID_CLASS_PRINT2)

extern const Eo_Event_Description _SIG_A_CHANGED;
#define SIG_A_CHANGED (&(_SIG_A_CHANGED))

#define SIMPLE_CLASS simple_class_get()
const Eo_Class *simple_class_get(void);

#endif
