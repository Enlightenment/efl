#ifndef SIMPLE_H
#define SIMPLE_H

typedef struct
{
   int a;
} Simple_Public_Data;

void simple_a_set(Eo *obj, int a);
Eina_Bool simple_a_print(Eo *obj);
Eina_Bool simple_class_print(const Eo *obj);
Eina_Bool simple_class_print2(const Eo *obj);

extern const Efl_Event_Description _SIG_A_CHANGED;
#define SIG_A_CHANGED (&(_SIG_A_CHANGED))

#define SIMPLE_CLASS simple_class_get()
const Efl_Class *simple_class_get(void);

#endif
