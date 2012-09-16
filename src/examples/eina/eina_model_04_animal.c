/*
 * animal.c
 */

#include "eina_model_04_animal.h"

static Eina_Bool initialized = EINA_FALSE;

static void
_animal_eat(Eina_Model *m)
{
   printf("%s\t%s", eina_model_type_name_get(eina_model_type_get(m)),
         __func__);
   printf("\t\t Eat Animal\n");
}

static void
_animal_breathe(Eina_Model *m)
{
   printf("%s\t%s", eina_model_type_name_get(eina_model_type_get(m)),
         __func__);
   printf("\t\t Breathe Animal\n");
}

const char *ANIMAL_MODEL_TYPE_NAME = NULL;
static Animal_Type _ANIMAL_TYPE;

const Eina_Model_Type * const ANIMAL_TYPE = (Eina_Model_Type *) &_ANIMAL_TYPE;

void
animal_init(void)
{
   Eina_Model_Type *type;

   if (initialized) return;
   initialized = EINA_TRUE;

   ANIMAL_MODEL_TYPE_NAME = "Animal_Model_Type";

   type = (Eina_Model_Type *)&_ANIMAL_TYPE;
   type->version = EINA_MODEL_TYPE_VERSION;
   type->name = ANIMAL_MODEL_TYPE_NAME;
   type->private_size = 0;

   eina_model_type_subclass_setup(type, EINA_MODEL_TYPE_GENERIC);

   /* define extra methods */

   type->type_size = sizeof(Animal_Type);
   ANIMAL_TYPE(type)->breathe = _animal_breathe;
   ANIMAL_TYPE(type)->eat = _animal_eat;
}

void
animal_breathe(Eina_Model *m)
{
   EINA_SAFETY_ON_FALSE_RETURN(eina_model_instance_check(m, ANIMAL_TYPE));

   void (*pf)(Eina_Model *m);
   pf = eina_model_method_resolve(m, Animal_Type, breathe);
   EINA_SAFETY_ON_NULL_RETURN(pf);
   printf("%s()   \t", __func__);
   pf(m);
}

void
animal_eat(Eina_Model *m)
{
   EINA_SAFETY_ON_FALSE_RETURN(eina_model_instance_check(m, ANIMAL_TYPE));

   void (*pf)(Eina_Model *m);
   pf = eina_model_method_resolve(m, Animal_Type, eat);
   EINA_SAFETY_ON_NULL_RETURN(pf);
   printf("%s()    \t", __func__);
   pf(m);
}
