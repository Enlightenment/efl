/**
 * @file Elocation.h
 * @brief Elocation Library
 *
 * @defgroup Location Location
 */

/**
 * @page elocation_main Elocation (BETA)
 *
 * @section intro Elocation Use Cases
 *
 * Elocation is meant as a convenience library to ease application developers
 * the usage of geo information in their apps. Adding a geo tag to a picture or
 * translating an address to a GPS position and show it on a map widget are just
 * some of the use cases.
 *
 * In the beginning elocation will rely on the GeoClue DBus service. Its has
 * providers for various techniques to get hold off the current position.
 * Ranging from GeoIP over wifi and GSM cell location to GPS. As well as
 * provider to translates between location in a textual form to coordinates
 * (GeoCode).
 *
 * Elocation covers all of these interfaces but in the end it depends on your
 * system and the installed GeoClue providers what can be used.
 *
 * Currently it offer the following functionality:
 * @li Request current address in textual form
 * @li Request current position in GPS format
 * @li Translate a position into and address or an address in a position
 *
 * You can find the API documentation at @ref Location
*/
#ifndef _ELOCATION_H
#define _ELOCATION_H

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

#ifdef EFL_BETA_API_SUPPORT

#include <stdio.h>

#include <Ecore.h>
#include <Eldbus.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup Location
 * @brief Available location events that are emitted from the library
 * @since 1.13
 *
 * Ecore events emitted by the library. Applications can register ecore event
 * handlers to react on such events. After the initial query this can be used
 * to keep track of changes and update your UI or data accordingly.
 * @{
 */
EAPI extern int ELOCATION_EVENT_STATUS; /**< Status changed */
EAPI extern int ELOCATION_EVENT_POSITION; /**< Position changed */
EAPI extern int ELOCATION_EVENT_ADDRESS; /**< Address changed */
EAPI extern int ELOCATION_EVENT_VELOCITY; /**< Velocity changed */
EAPI extern int ELOCATION_EVENT_GEOCODE; /**< Reply for geocode translation arrived */
EAPI extern int ELOCATION_EVENT_REVERSEGEOCODE; /**< Reply for geocode translation arrived */
EAPI extern int ELOCATION_EVENT_NMEA; /**< NMEA update */
EAPI extern int ELOCATION_EVENT_SATELLITE; /**< Satellite info changed */
EAPI extern int ELOCATION_EVENT_POI; /**< POI reply */
EAPI extern int ELOCATION_EVENT_META_READY; /**< Meta provider is ready to be used */
/**@}*/

/**
 * @ingroup Location
 * @typedef Elocation_Accuracy_Level
 * @since 1.13
 *
 * Different location accuracy levels from country level up to detailed,
 * e.g. GPS, information.
 * @{
 */
typedef enum {
   ELOCATION_ACCURACY_LEVEL_NONE = 0,
   ELOCATION_ACCURACY_LEVEL_COUNTRY = 1,
   ELOCATION_ACCURACY_LEVEL_REGION, /* GeoClue1 only */
   ELOCATION_ACCURACY_LEVEL_LOCALITY, /* GeoClue1 only */
   ELOCATION_ACCURACY_LEVEL_POSTALCODE, /* GeoClue1 only */
   ELOCATION_ACCURACY_LEVEL_CITY = 4, /* GeoClue2 only */
   ELOCATION_ACCURACY_LEVEL_NEIGHBORHOOD = 5, /* GeoClue2 only */
   ELOCATION_ACCURACY_LEVEL_STREET = 6,
   ELOCATION_ACCURACY_LEVEL_DETAILED, /* GeoClue1 only */
   ELOCATION_ACCURACY_LEVEL_EXACT = 8, /* GeoClue2 only */
} Elocation_Accuracy_Level;
/**@}*/

/**
 * @ingroup Location
 * @typedef Elocation_Resource_Flags
 * @since 1.13
 *
 * Flags for available system resources to be used for locating. So far they
 * cover physical resources like network connection, cellular network
 * connection and GPS.
 * @{
 */
typedef enum {
   ELOCATION_RESOURCE_NONE = 0,
   ELOCATION_RESOURCE_NETWORK = 1 << 0, /**< Internet connection is available */
   ELOCATION_RESOURCE_CELL = 1 << 1, /**< Cell network information, e.g. GSM, is available */
   ELOCATION_RESOURCE_GPS = 1 << 2, /**< GPS information is available */

   ELOCATION_RESOURCE_ALL = (1 << 10) - 1 /**< All resources are available */
} Elocation_Resource_Flags;
/**@}*/

/**
 * @ingroup Location
 * @typedef Elocation_Accuracy
 * @since 1.13
 *
 * Information about the accuracy of the reported location. For details about
 * the level of accuracy see #Elocation_Accuracy_Level. It also covers
 * horizontal and vertical accuracy. The values depend on the used provider
 * and may very in quality.
 */
typedef struct _Elocation_Accuracy
{
   Elocation_Accuracy_Level level;
   double horizontal;
   double vertical;
} Elocation_Accuracy;

/**
 * @ingroup Location
 * @typedef Elocation_Address
 * @since 1.13
 *
 * Location information in textual form. Depending on the used provider this
 * can cover only the country or a detailed address with postcode and street.
 * The level of detail varies depending on the used provider.
 * A timestamp is available to calculate the age of the address data.
 */
typedef struct _Elocation_Address
{
   unsigned int timestamp; /**< Timestamp of data read out in seconds since epoch */
   char *country;
   char *countrycode;
   char *locality;
   char *postalcode;
   char *region;
   char *timezone;
   Elocation_Accuracy *accur;
} Elocation_Address;

/**
 * @ingroup Location
 * @typedef Elocation_Position
 * @since 1.13
 *
 * Location information based on the GPS grid. Latitude, longitude and altitude.
 * A timestamp is available to calculate the age of the address data.
 */
typedef struct _Elocation_Position
{
   unsigned int timestamp; /**< Timestamp of data read out in seconds since epoch */
   double latitude;
   double longitude;
   double altitude;
   Elocation_Accuracy *accur;
} Elocation_Position;

/**
 * @ingroup Location
 * @typedef Elocation_Velocity
 * @since 1.13
 *
 * Velocity information. So far this interface is only offered with GPS based
 * providers. It offers information about speed, direction and climb.
 * A timestamp is available to calculate the age of the address data.
 *
 * FIXME: check units and formats of this values coming in from GeoClue
 */
typedef struct _Elocation_Velocity
{
   unsigned int timestamp; /**< Timestamp of data read out in seconds since epoch */
   double speed;
   double direction;
   double climb;
} Elocation_Velocity;

/**
 * @ingroup Location
 * @typedef Elocation_Requirements
 * @since 1.13
 *
 * Requirement settings for the location provider. Requirements can be a level
 * of accuracy or allowed resources like network access or GPS. See
 * #Elocation_Resource_Flags for all available resource flags.
 *
 * Based on this setting the best provider is chosen between the available
 * providers of GeoClue.
 */
typedef struct _Elocation_Requirements
{
   Elocation_Accuracy_Level accurancy_level;
   int min_time; /**< Minimal time between updates. Not implemented upstream */
   Eina_Bool require_update;
   Elocation_Resource_Flags allowed_resources;
} Elocation_Requirements;

/**
 * @brief Create a new address object to operate on.
 * @return Address object.
 *
 * The returned address object is safe to be operated on. It can be used for
 * all other elocation functions. Once finished with it it need to be destroyed
 * with a call to #elocation_address_free.
 *
 * @ingroup Location
 * @since 1.13
 */
EAPI Elocation_Address *elocation_address_new(void);

/**
 * @brief Free an address object
 * @param address Address object to be freed.
 *
 * Destroys an address object created with #elocation_address_new. Should be
 * used during the cleanup of the application or whenever the address object is
 * no longer needed.
 *
 * @ingroup Location
 * @since 1.13
 */
EAPI void elocation_address_free(Elocation_Address *address);

/**
 * @brief Create a new position object to operate on.
 * @return Position object.
 *
 * The returned address object is safe to be operated on. It can be used for
 * all other elocation functions. Once finished with it it need to be destroyed
 * with a call to #elocation_address_free.
 *
 * @ingroup Location
 * @since 1.13
 */
EAPI Elocation_Position *elocation_position_new(void);

/**
 * @brief Free an position object
 * @param position Position object to be freed.
 *
 * Destroys a position object created with #elocation_address_new. Should be
 * used during the cleanup of the application or whenever the location object is
 * no longer needed.
 *
 * @ingroup Location
 * @since 1.13
 */
EAPI void elocation_position_free(Elocation_Position *position);

/**
 * @brief Get the current address information.
 * @param address Address struct to be filled with information.
 * @return EINA_TRUE for success and EINA_FALSE for failure.
 *
 * Request the latest address. The requested to the underling components might
 * be asynchronous so better check the timestamp if the data has changed. To get
 * events when the address changes one can also subscribe to the
 * #ELOCATION_EVENT_ADDRESS ecore event which will have the address object as
 * event.
 *
 * @ingroup Location
 * @since 1.13
 */
EAPI Eina_Bool elocation_address_get(Elocation_Address *address);

/**
 * @brief Get the current position information.
 * @param position Position struct to be filled with information.
 * @return EINA_TRUE for success and EINA_FALSE for failure.
 *
 * Request the latest position. The requested to the underling components might
 * be asynchronous so better check the timestamp if the data has changed. To get
 * events when the position changes one can also subscribe to the
 * #ELOCATION_EVENT_POSITION ecore event which will have the position object as
 * event.
 *
 * @ingroup Location
 * @since 1.13
 */
EAPI Eina_Bool elocation_position_get(Elocation_Position *position);

/**
 * @brief Get the current status.
 * @param status Status
 * @return EINA_TRUE for success and EINA_FALSE for failure.
 *
 * @ingroup Location
 * @since 1.13
 */
EAPI Eina_Bool elocation_status_get(int *status);

/**
 * @brief Set the requirements.
 * @param requirements Requirements
 * @return EINA_TRUE for success and EINA_FALSE for failure.
 *
 * Set the requirements for selecting a provider.
 *
 * @ingroup Location
 * @since 1.13
 */
EAPI Eina_Bool elocation_requirements_set(Elocation_Requirements *requirements);

/**
 * @brief Convert position to address
 * @param position_shadow Position input
 * @param address_shadow Address output
 * @return EINA_TRUE for success and EINA_FALSE for failure.
 *
 * Use a GeoCode provider to translate from a given GPS coordinate
 * representation of a location to a representation in textual form.
 *
 * @ingroup Location
 * @since 1.13
 */
EAPI Eina_Bool elocation_position_to_address(Elocation_Position *position_shadow, Elocation_Address *address_shadow);

/**
 * @brief Convert address to position
 * @param address_shadow Address input
 * @param position_shadow Position output
 * @return EINA_TRUE for success and EINA_FALSE for failure.
 *
 * Use a GeoCode provider to translate from a given textual form
 * representation of a location to a representation as GPS coordinates.
 *
 * @ingroup Location
 * @since 1.13
 */
EAPI Eina_Bool elocation_address_to_position(Elocation_Address *address_shadow, Elocation_Position *position_shadow);

/**
 * @brief Convert free form address tring to position
 * @param freeform_address Address string in free form
 * @param position_shadow Position output
 * @return EINA_TRUE for success and EINA_FALSE for failure.
 *
 * Similar GeoCode translation from textual form to GPS coordinates as
 * #elocation_address_to_position but in this case the address is a simple
 * string which hopefully contains enough information for the provider to
 * understand and translate.
 *
 * Useful for an easy search interface in an application but also more error
 * prone regarding correct results.
 *
 * @ingroup Location
 * @since 1.13
 */
EAPI Eina_Bool elocation_freeform_address_to_position(const char *freeform_address, Elocation_Position *position_shadow);

/**
 * @brief Request a landmark position
 * @param position_shadow Position ouput
 * @param address_shadow Address input
 * @return EINA_TRUE for success and EINA_FALSE for failure.
 *
 * Request a landmark position also known as Point Of Interest (POI) from
 * GeoClue.
 *
 * @ingroup Location
 * @since 1.13
 */
EAPI Eina_Bool elocation_landmarks_get(Elocation_Position *position_shadow, Elocation_Address *address_shadow);

/**
 * @brief Initialize the elocation  subsystem.
 * @return EINA_TRUE for success and EINA_FALSE for failure.
 *
 * This function must be called before using any of the Elocation functionality
 * in your application to make sure it it setup correctly for usage.
 *
 * @ingroup Location
 * @since 1.13
 */
EAPI Eina_Bool elocation_init(void);

/**
 * @brief Cleanup and shutdown the elocation  subsystem.
 *
 * This function must be called when the application is no longer using any of
 * the Elocation functionality to allow the subsystem to shutdown cleanly.
 *
 * @ingroup Location
 * @since 1.13
 */
EAPI void elocation_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* BETA API */

#undef EAPI
#define EAPI

#endif
