/*
 * child.h
 */

#ifndef CHILD_H_
#define CHILD_H_

#include "eina_model_04_human.h"

#define CHILD_MODEL_TYPE_NAME "Child_Model_Type"

extern const Eina_Model_Type * const CHILD_TYPE;
#define CHILD_TYPE(x) ((Child_Type *) x)

typedef struct _Child_Type
{
   Human_Type parent_class;
   void (*cry)(Eina_Model *mdl);
} Child_Type;

void child_init();
void child_cry(Eina_Model *mdl);

#endif /* CHILD_H_ */
