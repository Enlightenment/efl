/*
 * child.h
 */

#ifndef CHILD_H_
#define CHILD_H_

#include "eina_model_04_human.h"

extern const char *CHILD_MODEL_TYPE_NAME;
extern const Eina_Model_Type * const CHILD_TYPE;
#define CHILD_TYPE(x) ((Child_Type *) (eina_model_type_subclass_check((x), CHILD_TYPE) ? (x) : NULL))

typedef struct _Child_Type
{
   Human_Type parent_class;
   void (*cry)(Eina_Model *m);
} Child_Type;

void child_init();
void child_cry(Eina_Model *m);

#endif /* CHILD_H_ */
