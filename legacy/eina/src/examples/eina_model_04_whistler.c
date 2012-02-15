/*
 * whistler.c
 *
 */

#include "eina_model_04_whistler.h"

void
whistler_whistle(Eina_Model *mdl)
{
   const Eina_Model_Interface *iface = NULL;
   iface = eina_model_interface_get(mdl, WHISTLER_INTERFACE_NAME);

   EINA_SAFETY_ON_NULL_RETURN(iface);

   void (*pf)(Eina_Model *);

   pf = eina_model_interface_method_resolve(iface, mdl, Whistler_Interface, whistle);
   EINA_SAFETY_ON_NULL_RETURN(pf);
   printf("%s()    \t", __func__);
   pf(mdl);
}
/*
 * call for overridden Swimmer Interface function
 */
void
swimmer_swim(Eina_Model *mdl)
{
   const Eina_Model_Interface *iface = NULL;
   iface = eina_model_interface_get(mdl, SWIMMER_INTERFACE_NAME);

   EINA_SAFETY_ON_NULL_RETURN(iface);

   void (*pf)(Eina_Model *);

   pf = eina_model_interface_method_resolve(iface, mdl, Swimmer_Interface, swim);
   EINA_SAFETY_ON_NULL_RETURN(pf);
   printf("%s()   \t", __func__);
   pf(mdl);
}

/*
 * call for overridden Diver Interface function
 */
void
diver_dive(Eina_Model *mdl)
{
   const Eina_Model_Interface *iface = NULL;
   iface = eina_model_interface_get(mdl, DIVER_INTERFACE_NAME);

   EINA_SAFETY_ON_NULL_RETURN(iface);

   void (*pf)(Eina_Model *);

   pf = eina_model_interface_method_resolve(iface, mdl, Diver_Interface, dive);
   EINA_SAFETY_ON_NULL_RETURN(pf);
   printf("%s()    \t", __func__);
   pf(mdl);
}
