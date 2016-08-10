#ifndef INHERIT2_H
#define INHERIT2_H

EAPI Eina_Bool inherit2_print(Eo *obj);
EAPI Eina_Bool inherit2_print2(Eo *obj);

#define INHERIT2_CLASS inherit2_class_get()
const Efl_Class *inherit2_class_get(void);

#endif
