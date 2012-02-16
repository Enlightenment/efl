/*
 * parrot.h
 */

#ifndef PARROT_H_
#define PARROT_H_

#include "eina_model_04_animal.h"

extern const char *PARROT_MODEL_TYPE_NAME;
extern const Eina_Model_Type * const PARROT_TYPE;

#define PARROT_TYPE(x) ((Parrot_Type *) (eina_model_type_subclass_check((x), PARROT_TYPE) ? (x) : NULL))

typedef struct _Parrot_Type
{
   Animal_Type parent_class;
   void (*fly)(Eina_Model *m);
} Parrot_Type;

void parrot_init();
void parrot_fly(Eina_Model *m);

#endif /* PARROT_H_ */
