#ifndef SIMPLE_H
#define SIMPLE_H

typedef struct
{
   int a;
   Eina_Bool a_print_called;
} Simple_Public_Data;

EAPI void simple_a_set(int a);
EAPI void simple_a_print(void);
EAPI void simple_class_print(void);
EAPI void simple_class_print2(void);

extern const Eo_Event_Description _SIG_A_CHANGED;
#define SIG_A_CHANGED (&(_SIG_A_CHANGED))

#define SIMPLE_CLASS simple_class_get()
const Eo_Class *simple_class_get(void);

extern Eina_Bool class_print_called;
extern Eina_Bool class_print2_called;

#endif
