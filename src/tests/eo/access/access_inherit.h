#ifndef INHERIT_H
#define INHERIT_H

EAPI void inherit_prot_print(void);

#define INHERIT_CLASS inherit_class_get()
const Eo_Class *inherit_class_get(void);

#endif
