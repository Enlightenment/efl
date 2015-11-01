#ifndef SIMPLE_H
#define SIMPLE_H

typedef struct
{
   int a;
} Simple_Public_Data;

EAPI void simple_a_set(Eo const* ___object, int a);
EAPI Eina_Bool simple_a_print(Eo const* ___object);
EAPI Eina_Bool simple_class_print(Eo const* ___object);
EAPI Eina_Bool simple_class_print2(Eo const* ___object);
EAPI void eo_super_simple_a_set(Eo_Class const* __klass, Eo const* ___object, int a);
EAPI Eina_Bool eo_super_simple_a_print(Eo_Class const* __klass, Eo const* ___object);
EAPI Eina_Bool eo_super_simple_class_print(Eo_Class const* __klass, Eo const* ___object);
EAPI Eina_Bool eo_super_simple_class_print2(Eo_Class const* __klass, Eo const* ___object);

extern const Eo_Event_Description _SIG_A_CHANGED;
#define SIG_A_CHANGED (&(_SIG_A_CHANGED))

#define SIMPLE_CLASS simple_class_get()
const Eo_Class *simple_class_get(void);

#endif
