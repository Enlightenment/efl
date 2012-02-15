/*
 * animal.h
 */

#ifndef ANIMAL_H_
#define ANIMAL_H_

#include <Eina.h>
#include <eina_safety_checks.h>

#define ANIMAL_MODEL_TYPE_NAME "Animal_Model_Type"

extern const Eina_Model_Type * const ANIMAL_TYPE;
#define ANIMAL_TYPE(x) ((Animal_Type *) x)

typedef struct _Animal_Type
{
   Eina_Model_Type parent_class;
   void (*eat)(Eina_Model *mdl);
   void (*breathe)(Eina_Model *mdl);
} Animal_Type;

void animal_init();
void animal_breathe(Eina_Model *mdl);
void animal_eat(Eina_Model *mdl);

#endif /* ANIMAL_H_ */
