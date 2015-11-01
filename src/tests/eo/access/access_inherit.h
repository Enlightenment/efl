#ifndef INHERIT_H
#define INHERIT_H

EAPI void inherit_prot_print(Eo const* ___object);
EAPI void eo_super_inherit_prot_print(Eo_Class const* ___klass, Eo const* ___object);

#define INHERIT_CLASS inherit_class_get()
const Eo_Class *inherit_class_get(void);

#endif
