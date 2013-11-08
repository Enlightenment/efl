#ifndef INHERIT2_H
#define INHERIT2_H

EAPI void inherit2_print(void);
EAPI void inherit2_print2(void);

#define INHERIT2_CLASS inherit2_class_get()
const Eo_Class *inherit2_class_get(void);

extern Eina_Bool inherit2_print_called;
extern Eina_Bool inherit2_print2_called;

#endif
