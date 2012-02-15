/*
 * parrot.h
 */

#ifndef PARROT_H_
#define PARROT_H_

#include "eina_model_04_animal.h"

#define PARROT_MODEL_TYPE_NAME "Parrot_Model_Type"

extern const Eina_Model_Type * const PARROT_TYPE;
#define PARROT_TYPE(x) ((Parrot_Type *) x)

typedef struct _Parrot_Type
{
   Animal_Type parent_class;
   void (*fly)(Eina_Model *mdl);
} Parrot_Type;

void parrot_init();
void parrot_fly(Eina_Model *mdl);

#endif /* PARROT_H_ */
