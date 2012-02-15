/*
 * animal.c
 */

#include "eina_model_04_animal.h"

static Eina_Bool initialized = EINA_FALSE;

static void
_animal_eat(Eina_Model *mdl)
{
   printf("%s\t%s", eina_model_type_name_get(eina_model_type_get(mdl)),
         __func__);
   printf("\t\t Eat Animal\n");
}

static void
_animal_breathe(Eina_Model *mdl)
{
   printf("%s\t%s", eina_model_type_name_get(eina_model_type_get(mdl)),
         __func__);
   printf("\t\t Breathe Animal\n");
}

static Animal_Type _ANIMAL_TYPE;
const Eina_Model_Type * const ANIMAL_TYPE = (Eina_Model_Type *) &_ANIMAL_TYPE;

void animal_init()
{
   if (initialized) return;
   initialized = EINA_TRUE;

   Eina_Model_Type *type = (Eina_Model_Type *) &_ANIMAL_TYPE;
   type->version = EINA_MODEL_TYPE_VERSION;
   type->parent = EINA_MODEL_TYPE_BASE;
   type->type_size = sizeof(Animal_Type);
   type->name = ANIMAL_MODEL_TYPE_NAME;
   type->parent = EINA_MODEL_TYPE_GENERIC;

   ANIMAL_TYPE(type)->breathe = _animal_breathe;
   ANIMAL_TYPE(type)->eat = _animal_eat;
}

void
animal_breathe(Eina_Model *mdl)
{
   EINA_SAFETY_ON_FALSE_RETURN(eina_model_instance_check(mdl, ANIMAL_TYPE));

   void (*pf)(Eina_Model *mdl);
   pf = eina_model_method_resolve(mdl, Animal_Type, breathe);
   EINA_SAFETY_ON_NULL_RETURN(pf);
   printf("%s()   \t", __func__);
   pf(mdl);
}

void
animal_eat(Eina_Model *mdl)
{
   EINA_SAFETY_ON_FALSE_RETURN(eina_model_instance_check(mdl, ANIMAL_TYPE));

   void (*pf)(Eina_Model *mdl);
   pf = eina_model_method_resolve(mdl, Animal_Type, eat);
   EINA_SAFETY_ON_NULL_RETURN(pf);
   printf("%s()    \t", __func__);
   pf(mdl);
}

