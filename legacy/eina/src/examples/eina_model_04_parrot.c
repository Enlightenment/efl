/*
 * parrot.c
 */

#include "eina_model_04_parrot.h"
#include "eina_model_04_whistler.h"

static Eina_Bool initialized = EINA_FALSE;

static void
_parrot_fly(Eina_Model *m)
{
   printf("%s\t%s", eina_model_type_name_get(eina_model_type_get(m)),
         __func__);
   printf("\t\t Fly Parrot\n");
}

static void
_parrot_eat(Eina_Model *m)
{
   printf("%s\t%s", eina_model_type_name_get(eina_model_type_get(m)),
         __func__);
   printf("\t\t Grain \n");
}

static void
_parrot_whistle(Eina_Model *m)
{
   printf("%s\t%s", eina_model_type_name_get(eina_model_type_get(m)),
         __func__);
   printf("\t\t Whistle Parrot\n");
}

/*
 * defining Parrot Model Instance
 * defining Whistler Interface instance
 */
const char *PARROT_MODEL_TYPE_NAME = NULL;

static Parrot_Type _PARROT_TYPE;
const Eina_Model_Type * const PARROT_TYPE = (Eina_Model_Type *) &_PARROT_TYPE;

static const Whistler_Interface _WHISTLER_INTERFACE;
static const Eina_Model_Interface * const WHISTLER_INTERFACE =
   (Eina_Model_Interface *) &_WHISTLER_INTERFACE;

static const Eina_Model_Interface * MODEL_INTERFACES_ARRAY[] =
   { &_WHISTLER_INTERFACE.base_interface, NULL }; //this array is for model

void
parrot_init()
{
   Eina_Model_Type *type;
   if (initialized) return;
   initialized = EINA_TRUE;

   animal_init();
   /*
    *overriding Whistler Interface (creating instance of Whistler Interface)
    */
   Eina_Model_Interface *iface = (Eina_Model_Interface *) &_WHISTLER_INTERFACE;
   iface->version = EINA_MODEL_INTERFACE_VERSION;
   iface->interface_size = sizeof(Whistler_Interface);
   iface->name = WHISTLER_INTERFACE_NAME;
   WHISTLER_INTERFACE(iface)->whistle = _parrot_whistle;

   PARROT_MODEL_TYPE_NAME = "Parrot_Model_Type";
   
   type = (Eina_Model_Type *)&_PARROT_TYPE;
   type->version = EINA_MODEL_TYPE_VERSION;
   type->name = PARROT_MODEL_TYPE_NAME;
   type->private_size = 0;
   
   eina_model_type_subclass_setup(type, ANIMAL_TYPE);

   type->type_size = sizeof(Parrot_Type);
   type->interfaces = MODEL_INTERFACES_ARRAY;

   ANIMAL_TYPE(type)->eat = _parrot_eat;
   PARROT_TYPE(type)->fly = _parrot_fly;
}


void
parrot_fly(Eina_Model *m)
{
   EINA_SAFETY_ON_FALSE_RETURN(eina_model_instance_check(m, PARROT_TYPE));

   void (*pf)(Eina_Model *m);
   pf = eina_model_method_resolve(m, Parrot_Type, fly);
   EINA_SAFETY_ON_NULL_RETURN(pf);
   printf("%s()    \t", __func__);
   pf(m);
}

