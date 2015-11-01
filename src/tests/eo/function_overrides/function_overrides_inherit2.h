#ifndef INHERIT2_H
#define INHERIT2_H

EAPI Eina_Bool inherit2_print(Eo const* ___object);
EAPI Eina_Bool inherit2_print2(Eo const* ___object);
EAPI Eina_Bool eo_super_inherit2_print(Eo_Class const* ___klass, Eo const* ___object);
EAPI Eina_Bool eo_super_inherit2_print2(Eo_Class const* ___klass, Eo const* ___object);

#define INHERIT2_CLASS inherit2_class_get()
const Eo_Class *inherit2_class_get(void);

#endif
