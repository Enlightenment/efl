/*
 * human.h
 */

#ifndef HUMAN_H_
#define HUMAN_H_

#include "eina_model_04_animal.h"

extern const char *HUMAN_MODEL_TYPE_NAME;
extern const Eina_Model_Type * const HUMAN_TYPE;

#define HUMAN_TYPE(x) ((Human_Type *) (eina_model_type_subclass_check((x), ANIMAL_TYPE) ? (x) : NULL))

typedef struct _Human_Type
{
   Animal_Type parent_class;
   void (*walk)(Eina_Model *m);
} Human_Type;

void human_init();
void human_walk(Eina_Model *m);

#endif /* HUMAN_H_ */
