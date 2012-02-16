/*
 * child.c
 */

#include "eina_model_04_child.h"
#include "eina_model_04_whistler.h"

static Eina_Bool initialized = EINA_FALSE;

static void
_child_cry(Eina_Model *m)
{
   printf("%s\t%s", eina_model_type_name_get(eina_model_type_get(m)),
         __func__);
   printf("\t\t Cry Child\n");
}

static void
_child_dive(Eina_Model *m)
{
   printf("%s\t%s", eina_model_type_name_get(eina_model_type_get(m)),
         __func__);
   printf("\t\t Dive Child\n");
}

const char *CHILD_MODEL_TYPE_NAME = NULL;

static Child_Type _CHILD_TYPE;
const Eina_Model_Type * const CHILD_TYPE = (Eina_Model_Type *) &_CHILD_TYPE;

static const Diver_Interface _DIVER_INTERFACE;
static const Eina_Model_Interface * const DIVER_INTERFACE =
   (Eina_Model_Interface *) &_DIVER_INTERFACE;

static const Eina_Model_Interface * CLASS_INTERFACE_ARRAY[] =
   { &_DIVER_INTERFACE.base_interface, NULL }; //this array is for model

void
child_init()
{
   Eina_Model_Type *type;

   if (initialized) return;
   initialized = EINA_TRUE;

   human_init();

   //overriding Diver Interface
   Eina_Model_Interface * iface = (Eina_Model_Interface *) &_DIVER_INTERFACE;
   iface->version = EINA_MODEL_INTERFACE_VERSION;
   iface->interface_size = sizeof(Diver_Interface);
   iface->name = DIVER_INTERFACE_NAME;
   DIVER_INTERFACE(iface)->dive = _child_dive;

   //creating instance of Child type
   CHILD_MODEL_TYPE_NAME = "Child_Model_Type";

   type = (Eina_Model_Type *) &_CHILD_TYPE;
   type->version = EINA_MODEL_TYPE_VERSION;
   type->name = CHILD_MODEL_TYPE_NAME;

   eina_model_type_subclass_setup(type, HUMAN_TYPE);

   type->type_size = sizeof(Child_Type);
   type->interfaces = CLASS_INTERFACE_ARRAY;

   CHILD_TYPE(type)->cry = _child_cry;
}

//call for implemented Child Class function
void
child_cry(Eina_Model *m)
{
   EINA_SAFETY_ON_FALSE_RETURN(eina_model_instance_check(m, CHILD_TYPE));

   void (*pf)(Eina_Model *m);
   pf = eina_model_method_resolve(m, Child_Type, cry);
   EINA_SAFETY_ON_NULL_RETURN(pf);
   printf("%s() \t\t", __func__);
   pf(m);
}
