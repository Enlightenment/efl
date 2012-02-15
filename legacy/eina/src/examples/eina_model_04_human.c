/*
 * human.c
 *
 */

#include "eina_model_04_human.h"
#include "eina_model_04_whistler.h"

static Eina_Bool initialized = EINA_FALSE;

static void
_human_eat(Eina_Model *mdl)
{
   printf("%s\t%s", eina_model_type_name_get(eina_model_type_get(mdl)),
         __func__);
   printf("\t\t Salad\n");
}

static void
_human_walk(Eina_Model *mdl)
{
   printf("%s\t%s", eina_model_type_name_get(eina_model_type_get(mdl)),
         __func__);
   printf("\t\t Walk\n");
}

static void
_human_whistle(Eina_Model *mdl)
{
   printf("%s\t%s", eina_model_type_name_get(eina_model_type_get(mdl)),
         __func__);
   printf("\t\t Whistle Human\n");
}

static void
_human_swim(Eina_Model *mdl)
{
   printf("%s\t%s", eina_model_type_name_get(eina_model_type_get(mdl)),
         __func__);
   printf("\t\t Swim Human\n");
}

static void
_human_dive(Eina_Model *mdl)
{
   printf("%s\t%s", eina_model_type_name_get(eina_model_type_get(mdl)),
         __func__);
   printf("\t\t Dive Human\n");
}
/*
 * defining Human Model Instance
 * defining Whistler Interface instance
 * defining Swimmer Interface instance
 * defining Diver Interface instance
 */
static Human_Type _HUMAN_TYPE;
const Eina_Model_Type * const HUMAN_TYPE = (Eina_Model_Type *) &_HUMAN_TYPE;

static const Whistler_Interface _WHISTLER_INTERFACE;
static const Eina_Model_Interface * const WHISTLER_INTERFACE =
   (Eina_Model_Interface *) &_WHISTLER_INTERFACE;

static const Swimmer_Interface _SWIMMER_INTERFACE;
static const Eina_Model_Interface * const SWIMMER_INTERFACE =
   (Eina_Model_Interface *) &_SWIMMER_INTERFACE;

static const Diver_Interface _DIVER_INTERFACE;
static const Eina_Model_Interface * const DIVER_INTERFACE =
   (Eina_Model_Interface *) &_DIVER_INTERFACE;

/*
 * defining parent interfaces for Diver Interface instance
 * defining Interfaces for Human Model instance
 */
static const Eina_Model_Interface * PARENT_INTERFACES_ARRAY[] =
   { &_SWIMMER_INTERFACE.base_interface, NULL }; //this array is for model
static const Eina_Model_Interface * MODEL_INTERFACES_ARRAY[] =
   { &_WHISTLER_INTERFACE.base_interface, &_DIVER_INTERFACE.base_interface,
      NULL }; //this array is for model

void
human_init()
{
   if (initialized) return;
   initialized = EINA_TRUE;

   animal_init();

   /*
    * Initializing Whistler Interface Instance
    */
   Eina_Model_Interface *iface = (Eina_Model_Interface *) &_WHISTLER_INTERFACE;
   iface->version = EINA_MODEL_INTERFACE_VERSION;
   iface->interface_size = sizeof(Whistler_Interface);
   iface->name = WHISTLER_INTERFACE_NAME;
   WHISTLER_INTERFACE(iface)->whistle = _human_whistle;

   /*
    * Initializing Swimmer Interface Instance
    */
   iface = (Eina_Model_Interface *) &_SWIMMER_INTERFACE;
   iface->version = EINA_MODEL_INTERFACE_VERSION;
   iface->interface_size = sizeof(Swimmer_Interface);
   iface->name = SWIMMER_INTERFACE_NAME;
   SWIMMER_INTERFACE(iface)->swim = _human_swim;

   /*
    * Initializing Diver Interface Instance
    * Diver_Interface is inherited from Swimmer
    */
   iface = (Eina_Model_Interface *) &_DIVER_INTERFACE;
   iface->version = EINA_MODEL_INTERFACE_VERSION;
   iface->interface_size = sizeof(Diver_Interface);
   iface->name = DIVER_INTERFACE_NAME;
   iface->interfaces = PARENT_INTERFACES_ARRAY;
   DIVER_INTERFACE(iface)->dive = _human_dive;

   /*
    * Initializing instance of Human Model
    */
   Eina_Model_Type *type = (Eina_Model_Type *) &_HUMAN_TYPE;
   type->version = EINA_MODEL_TYPE_VERSION;
   type->parent = ANIMAL_TYPE;
   type->type_size = sizeof(Human_Type);
   type->name = HUMAN_MODEL_TYPE_NAME;
   type->interfaces = MODEL_INTERFACES_ARRAY;

   ANIMAL_TYPE(type)->eat = _human_eat;
   HUMAN_TYPE(type)->walk =_human_walk;
}


/*
 * call for implemented Human Class function
 */
void
human_walk(Eina_Model *mdl)
{
   EINA_SAFETY_ON_FALSE_RETURN(eina_model_instance_check(mdl, HUMAN_TYPE));

   void (*pf)(Eina_Model *mdl);
   pf = eina_model_method_resolve(mdl, Human_Type, walk);
   EINA_SAFETY_ON_NULL_RETURN(pf);
   printf("%s()    \t", __func__);
   pf(mdl);
}
