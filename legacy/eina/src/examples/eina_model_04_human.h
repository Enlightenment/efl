/*
 * human.h
 */

#ifndef HUMAN_H_
#define HUMAN_H_

#include "eina_model_04_animal.h"

#define HUMAN_MODEL_TYPE_NAME "Human_Model_Type"

extern const Eina_Model_Type * const HUMAN_TYPE;
#define HUMAN_TYPE(x) ((Human_Type *) x)

typedef struct _Human_Type
{
   Animal_Type parent_class;
   void (*walk)(Eina_Model *mdl);
} Human_Type;

void human_init();
void human_walk(Eina_Model *mdl);

#endif /* HUMAN_H_ */
