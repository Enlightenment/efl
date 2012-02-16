/*
 * animal.h
 */

#ifndef ANIMAL_H_
#define ANIMAL_H_

#include <Eina.h>

extern const char *ANIMAL_MODEL_TYPE_NAME;
extern const Eina_Model_Type * const ANIMAL_TYPE;

#define ANIMAL_TYPE(x) ((Animal_Type *) (eina_model_type_subclass_check((x), ANIMAL_TYPE) ? (x) : NULL))

typedef struct _Animal_Type
{
   Eina_Model_Type parent_class;
   void (*eat)(Eina_Model *m);
   void (*breathe)(Eina_Model *m);
} Animal_Type;

void animal_init(void);
void animal_breathe(Eina_Model *m);
void animal_eat(Eina_Model *m);

#endif /* ANIMAL_H_ */
