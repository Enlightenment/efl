#ifndef INHERIT_H
#define INHERIT_H

void inherit_prot_print(Eo *obj);

#define INHERIT_CLASS inherit_class_get()
const Efl_Class *inherit_class_get(void);

#endif
