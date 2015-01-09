#include <stdio.h>

#define EFL_BETA_API_SUPPORT

#include <Ecore.h>
#include <Elocation.h>

/* A set of callbacks to react on incoming elocation events. They are standard
 * ecore events and we register callbacks based on these events in the main
 * function.
 */
static Eina_Bool
status_changed(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *event)
{
   int *status = event;

   printf("Status changed to: %i\n", *status);
   printf("\n");

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
rgeocode_arrived(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *event)
{
   Elocation_Address *address;

   address = event;
   printf("Geocode reply:\n");
   printf("Country: %s\n", address->country);
   printf("Countrycode: %s\n", address->countrycode);
   printf("Locality: %s\n", address->locality);
   printf("Postalcode: %s\n", address->postalcode);
   printf("Region: %s\n", address->region);
   printf("Timezone: %s\n", address->timezone);
   printf("Accuracy level: %i\n", address->accur->level);
   printf("Accuracy horizontal: %f\n", address->accur->horizontal);
   printf("Accuracy vertical: %f\n", address->accur->vertical);
   printf("\n");

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
geocode_arrived(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *event)
{
   Elocation_Position *position;

   position = event;
   printf("Reverse geocode reply:\n");
   printf("Latitude:\t %f\n", position->latitude);
   printf("Longitude:\t %f\n", position->longitude);
   printf("Altitude:\t %f\n", position->altitude);
   printf("Accuracy level: %i\n", position->accur->level);
   printf("Accuracy horizontal: %f\n", position->accur->horizontal);
   printf("Accuracy vertical: %f\n", position->accur->vertical);
   printf("\n");

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
address_changed(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *event)
{
   Elocation_Address *address;

   address = event;
   printf("Address update with data from timestamp: %i\n", address->timestamp);
   printf("Country: %s\n", address->country);
   printf("Countrycode: %s\n", address->countrycode);
   printf("Locality: %s\n", address->locality);
   printf("Postalcode: %s\n", address->postalcode);
   printf("Region: %s\n", address->region);
   printf("Timezone: %s\n", address->timezone);
   printf("Accuracy level: %i\n", address->accur->level);
   printf("Accuracy horizontal: %f\n", address->accur->horizontal);
   printf("Accuracy vertical: %f\n", address->accur->vertical);
   printf("\n");

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
position_changed(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *event)
{
   Elocation_Position *position;

   position = event;
   printf("GeoClue position reply with data from timestamp %i\n", position->timestamp);
   printf("Latitude:\t %f\n", position->latitude);
   printf("Longitude:\t %f\n", position->longitude);
   printf("Altitude:\t %f\n", position->altitude);
   printf("Accuracy level: %i\n", position->accur->level);
   printf("Accuracy horizontal: %f\n", position->accur->horizontal);
   printf("Accuracy vertical: %f\n", position->accur->vertical);
   printf("\n");

   return ECORE_CALLBACK_DONE;
}

int
main(void)
{
   Elocation_Address *address, *addr_geocode;
   Elocation_Position *position, *pos_geocode;
   int status;

   /* Init the needed efl subsystems so we can safely use them */
   ecore_init();
   eldbus_init();
   elocation_init();

   /* Create an address and positon object that we use for all our operations.
    * Needs to be freed manually with elocation_*_free when we now longer use
    * them */
   address = elocation_address_new();
   position = elocation_position_new();

   /* Register callback so we get updates later on */
   ecore_event_handler_add(ELOCATION_EVENT_STATUS, status_changed, NULL);
   ecore_event_handler_add(ELOCATION_EVENT_POSITION, position_changed, NULL);
   ecore_event_handler_add(ELOCATION_EVENT_ADDRESS, address_changed, NULL);
   ecore_event_handler_add(ELOCATION_EVENT_GEOCODE, geocode_arrived, NULL);
   ecore_event_handler_add(ELOCATION_EVENT_REVERSEGEOCODE, rgeocode_arrived, NULL);

   /* To the initial request for status address and position. This fills in the
    * objects with the data from GeoClue */
   elocation_status_get(&status);
   elocation_position_get(position);
   elocation_address_get(address);

   /* Another set of address and position object. This time for demonstrating
    * the GeoCode functionalities */
   addr_geocode = elocation_address_new();
   pos_geocode = elocation_position_new();

   /* The Freeform API could use any string to transform it into position
    * coordinates. How good that works depends on the used GeoClue provider */
   elocation_freeform_address_to_position("London", pos_geocode);

   /* Some demo values to show the position to address conversion */
   pos_geocode->latitude = 51.7522;
   pos_geocode->longitude = -1.25596;
   pos_geocode->accur->level = 3;
   elocation_position_to_address(pos_geocode, addr_geocode);

   /* And now from address to position */
   addr_geocode->locality = "Cambridge";
   addr_geocode->countrycode = "UK";
   elocation_address_to_position(addr_geocode, pos_geocode);

   /* Enter the mainloop now that we are setup with initial data and waiting for
    * events. */
   ecore_main_loop_begin();

   /* Cleanup allocated memory now that we shut down */
   elocation_address_free(addr_geocode);
   elocation_position_free(pos_geocode);
   elocation_address_free(address);
   elocation_position_free(position);

   /* Make sure we also shut down the initialized subsystems */
   elocation_shutdown();
   eldbus_shutdown();
   ecore_shutdown();
   return 0;
}
