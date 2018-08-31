#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include <Eina.h>
#include <Ecore.h>
#include <Eldbus.h>
#include <Elocation.h>
#include <elocation_private.h>

/* FIXME: These globals really need to get reduced before leaving the PROTO
 * area.
 */
static char *unique_name = NULL;
static Eldbus_Connection *conn = NULL;
static Elocation_Provider *address_provider = NULL;
static Elocation_Provider *position_provider = NULL;
static Eldbus_Object *obj_meta = NULL;
static Eldbus_Proxy *manager_master = NULL;
static Eldbus_Proxy *meta_geoclue = NULL;
static Eldbus_Proxy *meta_address = NULL;
static Eldbus_Proxy *meta_position = NULL;
static Eldbus_Proxy *meta_masterclient = NULL;
static Eldbus_Proxy *meta_velocity = NULL;
static Eldbus_Proxy *meta_nmea = NULL;
static Eldbus_Proxy *meta_satellite = NULL;
static Eldbus_Proxy *geonames_geocode = NULL;
static Eldbus_Proxy *geonames_rgeocode = NULL;
static Eldbus_Proxy *master_poi = NULL;
static Elocation_Address *address = NULL;
static Elocation_Position *position = NULL;
static Elocation_Address *addr_geocode = NULL;
static Elocation_Position *pos_geocode = NULL;
static Elocation_Velocity *velocity = NULL;
static int status = -1; /* 0 is a valid status code */
static char nmea_sentence[256];

int _elocation_log_dom = -1;

/* Elocation ecore event types we provide to the application. */
EAPI int ELOCATION_EVENT_STATUS = 0;
EAPI int ELOCATION_EVENT_POSITION = 0;
EAPI int ELOCATION_EVENT_ADDRESS = 0;
EAPI int ELOCATION_EVENT_VELOCITY = 0;
EAPI int ELOCATION_EVENT_GEOCODE = 0;
EAPI int ELOCATION_EVENT_REVERSEGEOCODE = 0;
EAPI int ELOCATION_EVENT_NMEA = 0;
EAPI int ELOCATION_EVENT_SATELLITE = 0;
EAPI int ELOCATION_EVENT_POI = 0;
EAPI int ELOCATION_EVENT_META_READY = 0;

/* Internal events */
int ELOCATION_EVENT_IN = 0;
int ELOCATION_EVENT_OUT = 0;

static void
_dummy_free(void *user_data EINA_UNUSED, void *func_data EINA_UNUSED)
{
   /* Don't free the event data after dispatching the event. We keep track of
    * it on our own
    */
}

/* Generic provider message unmarshaller. Used from all different provider
 * calbacks that receive such a message
 */
static Eina_Bool
unmarshall_provider(const Eldbus_Message *reply, Elocation_Provider *provider)
{
   char *name = NULL, *desc = NULL, *service = NULL, *path = NULL;

   if (!eldbus_message_arguments_get(reply, "ssss", &name, &desc, &service, &path))
     return EINA_FALSE;

   provider->name = strdup(name);
   provider->description = strdup(desc);
   provider->service = strdup(service);
   provider->path = strdup(path);
   return EINA_TRUE;
}

static void
meta_address_provider_info_cb(void *data, const Eldbus_Message *reply, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *err, *errmsg;
   Elocation_Provider *addr_provider;

   addr_provider = data;

   if (eldbus_message_error_get(reply, &err, &errmsg))
     {
        ERR("Error: %s %s", err, errmsg);
        return;
     }

   if (!unmarshall_provider(reply, addr_provider))
     {
        ERR("Error: Unable to unmarshall address provider");
        return;
     }

   DBG("Meta address provider name: %s, %s, %s, %s", addr_provider->name,
                                                     addr_provider->description,
                                                     addr_provider->service,
                                                     addr_provider->path);
}

static void
meta_position_provider_info_cb(void *data, const Eldbus_Message *reply, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *err, *errmsg;
   Elocation_Provider *pos_provider;

   pos_provider = data;

   if (eldbus_message_error_get(reply, &err, &errmsg))
     {
        ERR("Error: %s %s", err, errmsg);
        return;
     }

   if (!unmarshall_provider(reply, pos_provider))
     {
        ERR("Error: Unable to unmarshall position provider");
        return;
     }

   DBG("Meta position provider name: %s, %s, %s, %s", pos_provider->name,
                                                      pos_provider->description,
                                                      pos_provider->service,
                                                      pos_provider->path);
}

static void
meta_address_provider_info_signal_cb(void *data, const Eldbus_Message *reply)
{
   Elocation_Provider *addr_provider;
   addr_provider = data;

   if (!unmarshall_provider(reply, addr_provider))
     {
        ERR("Error: Unable to unmarshall address provider");
        return;
     }

   DBG("Meta address provider name changed: %s, %s, %s, %s", addr_provider->name,
                                                             addr_provider->description,
                                                             addr_provider->service,
                                                             addr_provider->path);
}

static void
meta_position_provider_info_signal_cb(void *data, const Eldbus_Message *reply)
{
   Elocation_Provider *pos_provider;
   pos_provider = data;

   if (!unmarshall_provider(reply, pos_provider))
     {
        ERR("Error: Unable to unmarshall position provider");
        return;
     }

   DBG("Meta position provider name changed: %s, %s, %s, %s", pos_provider->name,
                                                              pos_provider->description,
                                                              pos_provider->service,
                                                              pos_provider->path);
}

/* A address is quite flexible what kind of key value pairs it contains in the
 * dict. Similar to a reverse GeoCode message as both return an address object.
 */
static Eina_Bool
unmarshall_address(const Eldbus_Message *reply, Elocation_Address *addr)
{
   int32_t level, timestamp;
   Eldbus_Message_Iter *sub, *dict, *entry;
   double horizontal;
   double vertical;
   const char *key, *signature;
   char *value;

   signature = eldbus_message_signature_get(reply);

   if (!strcmp(signature, "ia{ss}(idd)"))
     {
        if (!eldbus_message_arguments_get(reply, "ia{ss}(idd)", &timestamp, &dict, &sub))
          return EINA_FALSE;
        addr->timestamp = timestamp;
     }
   else if (!strcmp(signature, "a{ss}(idd)"))
     {
        if (!eldbus_message_arguments_get(reply, "a{ss}(idd)", &dict, &sub))
          return EINA_FALSE;
        addr->timestamp = 0;
      }
   else
     return EINA_FALSE;


   /* Cleanup potential old entries before re-using */
   addr->country = NULL;
   addr->countrycode = NULL;
   addr->locality = NULL;
   addr->postalcode = NULL;
   addr->region = NULL;
   addr->timezone = NULL;

   while (eldbus_message_iter_get_and_next(dict, 'e', &entry))
    {
       if (!eldbus_message_iter_arguments_get(entry, "ss", &key, &value))
         continue;

       if (!strcmp(key, "country"))
         {
            free(addr->country);
            addr->country = strdup(value);
         }
       else if (!strcmp(key, "countrycode"))
         {
            free(addr->countrycode);
            addr->countrycode = strdup(value);
         }
       else if (!strcmp(key, "locality"))
         {
            free(addr->locality);
            addr->locality = strdup(value);
         }
       else if (!strcmp(key, "postalcode"))
         {
            free(addr->postalcode);
            addr->postalcode = strdup(value);
         }
       else if (!strcmp(key, "region"))
         {
            free(addr->region);
            addr->region = strdup(value);
         }
       else if (!strcmp(key, "timezone"))
         {
            free(addr->timezone);
            addr->timezone = strdup(value);
         }
    }

   if (!eldbus_message_iter_arguments_get(sub, "idd", &level, &horizontal, &vertical))
     return EINA_FALSE;

   addr->accur->level = level;
   addr->accur->horizontal = horizontal;
   addr->accur->vertical = vertical;
   return EINA_TRUE;
}

/* Receive and unmarshall a reverse GeoCode message. The dict can contain a
 * variable set of key value pairs so we need to handle this with care
 */
static void
rgeocode_cb(void *data EINA_UNUSED, const Eldbus_Message *reply, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *err, *errmsg;

   if (eldbus_message_error_get(reply, &err, &errmsg))
     {
        ERR("Error: %s %s", err, errmsg);
        return;
     }

   if (!unmarshall_address(reply, addr_geocode))
     {
        ERR("Error: Unable to unmarshall address");
        return;
     }

   /* Send out an event to all interested parties that we have an update */
   ecore_event_add(ELOCATION_EVENT_REVERSEGEOCODE, addr_geocode, _dummy_free, NULL);
}

/* Point of Interest (POI) aka landmark message unmarshalling. Thsi interface is
 * not in standard GeoClue but currently a Tizen extension.
 */
static void
poi_cb(void *data EINA_UNUSED, const Eldbus_Message *reply, Eldbus_Pending *pending EINA_UNUSED)
{
   int32_t count, id, rank;
   double lat, lon, bound_left, bound_top, bound_right, bound_bottom;
   const char *name, *icon, *house, *road, *village, *suburb, *postcode;
   const char *city, *county, *country, *country_code;
   Eldbus_Message_Iter *array, *struct_landmark;
   const char *err, *errmsg;

   if (eldbus_message_error_get(reply, &err, &errmsg))
     {
        ERR("Error: %s %s", err, errmsg);
        return;
     }

   /* Yeah, its quite a horrible message. The POI interface could use a better design */
   if (!eldbus_message_arguments_get(reply, "ia(iiddddddsssssssssss", &count ,&array))
     return;

   /* TODO re-check that the parameter ordering is what we expect */
   while (eldbus_message_iter_get_and_next(array, 'r', &struct_landmark))
     {
        if (!eldbus_message_iter_arguments_get(struct_landmark, "iiddddddsssssssssss", &id, &rank,
                                         &lat, &lon, &bound_left, &bound_top, &bound_right,
                                         &bound_bottom, &name, &icon, &house, &road,
                                         &village, &suburb, &postcode, &city, &county,
                                         &country, &country_code))
          return;

        DBG("Landmark received: %i, %i, %f, %f, %f, %f, %f, %f, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s,",
                                         id, rank, lat, lon, bound_left, bound_top, bound_right,
                                         bound_bottom, name, icon, house, road, village,
                                         suburb, postcode, city, county, country, country_code);
     }

   /* Send out an event to all interested parties that we have an update */
   ecore_event_add(ELOCATION_EVENT_POI, NULL, _dummy_free, NULL);
}

/* Unmarshall a GeoCode message */
static void
geocode_cb(void *data EINA_UNUSED, const Eldbus_Message *reply, Eldbus_Pending *pending EINA_UNUSED)
{
   GeocluePositionFields fields;
   int32_t level;
   double horizontal = 0.0;
   double vertical = 0.0;
   double latitude = 0.0;
   double longitude = 0.0;
   double altitude = 0.0;
   Eldbus_Message_Iter *sub;
   const char *err, *errmsg;

   if (eldbus_message_error_get(reply, &err, &errmsg))
     {
        ERR("Error: %s %s", err, errmsg);
        return;
     }

   if (!eldbus_message_arguments_get(reply, "iddd(idd)", &fields,&latitude,
                                    &longitude, &altitude, &sub))
     return;

   /* GeoClue uses some flags to mark position fields as valid. We set invalid
    * fields to 0.0 */
   if (fields & GEOCLUE_POSITION_FIELDS_LATITUDE)
      pos_geocode->latitude = latitude;
   else
      pos_geocode->latitude = 0.0;

   if (fields & GEOCLUE_POSITION_FIELDS_LONGITUDE)
      pos_geocode->longitude = longitude;
   else
      pos_geocode->longitude = 0.0;

   if (fields & GEOCLUE_POSITION_FIELDS_ALTITUDE)
      pos_geocode->altitude = altitude;
   else
      pos_geocode->altitude = 0.0;

   if (!eldbus_message_iter_arguments_get(sub, "idd", &level, &horizontal, &vertical))
     return;

   pos_geocode->accur->level = level;
   pos_geocode->accur->horizontal = horizontal;
   pos_geocode->accur->vertical = vertical;
   /* Send out an event to all interested parties that we have an update */
   ecore_event_add(ELOCATION_EVENT_GEOCODE, pos_geocode, _dummy_free, NULL);
}

static void
address_cb(void *data EINA_UNUSED, const Eldbus_Message *reply, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *err, *errmsg;

   if (eldbus_message_error_get(reply, &err, &errmsg))
     {
        ERR("Error: %s %s", err, errmsg);
        return;
     }

   if (!unmarshall_address(reply, address))
     {
        ERR("Error: Unable to unmarshall address");
        return;
     }

   /* Send out an event to all interested parties that we have an update */
   ecore_event_add(ELOCATION_EVENT_ADDRESS, address, _dummy_free, NULL);
}

static void
address_signal_cb(void *data EINA_UNUSED, const Eldbus_Message *reply)
{
   const char *err, *errmsg;

   if (eldbus_message_error_get(reply, &err, &errmsg))
     {
        ERR("Error: %s %s", err, errmsg);
        return;
     }

   if (!unmarshall_address(reply, address))
     {
        ERR("Error: Unable to unmarshall address");
        return;
     }

   /* Send out an event to all interested parties that we have an update */
   ecore_event_add(ELOCATION_EVENT_ADDRESS, address, _dummy_free, NULL);
}

/* Unmarshall a velocity message. This is only available if we use a GPS
 * provider from GeoClue. None of the other providers offer this currently.
 */
static Eina_Bool
unmarshall_velocity(const Eldbus_Message *reply)
{
   GeoclueVelocityFields fields;
   int32_t timestamp = 0;
   double speed = 0.0;
   double direction = 0.0;
   double climb = 0.0;

   if (!eldbus_message_arguments_get(reply, "iiddd", &fields, &timestamp,
                                    &speed, &direction, &climb))
     return EINA_FALSE;

   velocity->timestamp = timestamp;

   /* GeoClue uses some flags to mark velocity fields as valid. We set invalid
    * fields to 0.0 */
   if (fields & GEOCLUE_VELOCITY_FIELDS_SPEED)
      velocity->speed = speed;
   else
      velocity->speed = 0.0;

   if (fields & GEOCLUE_VELOCITY_FIELDS_DIRECTION)
      velocity->direction = direction;
   else
      velocity->direction = 0.0;

   if (fields & GEOCLUE_VELOCITY_FIELDS_CLIMB)
      velocity->climb = climb;
   else
      velocity->climb = 0.0;

   return EINA_TRUE;
}

static void
velocity_cb(void *data EINA_UNUSED, const Eldbus_Message *reply, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *err, *errmsg;

   if (eldbus_message_error_get(reply, &err, &errmsg))
     {
        WARN("Warning: %s %s", err, errmsg);
        return;
     }

   if (!unmarshall_velocity(reply))
     {
        ERR("Error: Unable to unmarshall velocity");
        return;
     }

   /* Send out an event to all interested parties that we have an update */
   ecore_event_add(ELOCATION_EVENT_VELOCITY, velocity, _dummy_free, NULL);
}

static void
velocity_signal_cb(void *data EINA_UNUSED, const Eldbus_Message *reply)
{
   if (!unmarshall_velocity(reply))
     {
        ERR("Error: Unable to unmarshall velocity");
        return;
     }

   /* Send out an event to all interested parties that we have an update */
   ecore_event_add(ELOCATION_EVENT_VELOCITY, velocity, _dummy_free, NULL);
}

/* Unmarshall an raw NMEA message. It conatins a raw NMEA sentence which we can
 * pass on to applications that want to use their own NMEA parser. This is not
 * reommended.  Better use the other interfaces to access the needed data.
 *
 * This is currently a Tizen only interface and not in GeoClue upstream.
 */
static Eina_Bool
unmarshall_nmea(const Eldbus_Message *reply)
{
   int32_t timestamp = 0;

   if (!eldbus_message_arguments_get(reply, "is", &timestamp, &nmea_sentence))
     return EINA_FALSE;

   return EINA_TRUE;
}

static void
nmea_cb(void *data EINA_UNUSED, const Eldbus_Message *reply, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *err, *errmsg;

   if (eldbus_message_error_get(reply, &err, &errmsg))
     {
        WARN("Warning: %s %s", err, errmsg);
        return;
     }

   if (!unmarshall_nmea(reply))
     {
        ERR("Error: Unable to unmarshall nmea");
        return;
     }

   /* Send out an event to all interested parties that we have an update */
   ecore_event_add(ELOCATION_EVENT_NMEA, nmea_sentence, _dummy_free, NULL);
}

static void
nmea_signal_cb(void *data EINA_UNUSED, const Eldbus_Message *reply)
{
   if (!unmarshall_nmea(reply))
     {
        ERR("Error: Unable to unmarshall nmea");
        return;
     }

   ecore_event_add(ELOCATION_EVENT_NMEA, nmea_sentence, _dummy_free, NULL);
}

/* Unmarshall a satellite information message. This offers GPS specific
 * information about the used satellites and its properties. It can be used for
 * applications that rely on GPS and want to show more information like a 3D fix
 * or used satellites.
 *
 * This is currently a Tizen only interface and not available in GeoClue upstream.
 */
static Eina_Bool
unmarshall_satellite(const Eldbus_Message *reply)
{
   int32_t timestamp = 0, satellite_used = 0, satellite_visible = 0;
   int32_t snr = 0, elevation = 0, azimuth = 0, prn = 0, used_prn = 0;
   Eldbus_Message_Iter *sub_prn, *sub_info, *struct_info;

   if (!eldbus_message_arguments_get(reply, "iiiaia(iiii)", &timestamp, &satellite_used,
                                    &satellite_visible, &sub_prn, &sub_info))
     return EINA_FALSE;

   while (eldbus_message_iter_get_and_next(sub_prn, 'i', &used_prn))
     {
       DBG("Satellite used PRN %i", used_prn);
     }

   /* TODO re-check that the parameter ordering is what we expect */
   while (eldbus_message_iter_get_and_next(sub_info, 'r', &struct_info))
     {
        if (!eldbus_message_iter_arguments_get(struct_info, "iiii", &prn, &elevation, &azimuth, &snr))
          return EINA_FALSE;

        DBG("Satellite info %i, %i, %i, %i", prn, elevation, azimuth, snr);
     }

   return EINA_TRUE;
}

static void
satellite_cb(void *data EINA_UNUSED, const Eldbus_Message *reply, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *err, *errmsg;

   if (eldbus_message_error_get(reply, &err, &errmsg))
     {
        WARN("Warning: %s %s", err, errmsg);
        return;
     }

   if (!unmarshall_satellite(reply))
     {
        ERR("Error: Unable to unmarshall satellite");
        return;
     }

   /* Send out an event to all interested parties that we have an update */
   ecore_event_add(ELOCATION_EVENT_SATELLITE, NULL, _dummy_free, NULL);
}

static void
last_satellite_cb(void *data EINA_UNUSED, const Eldbus_Message *reply, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *err, *errmsg;

   if (eldbus_message_error_get(reply, &err, &errmsg))
     {
        WARN("Warning: %s %s", err, errmsg);
        return;
     }

   if (!unmarshall_satellite(reply))
     {
        ERR("Error: Unable to unmarshall last satellite");
        return;
     }

   /* Send out an event to all interested parties that we have an update */
   ecore_event_add(ELOCATION_EVENT_SATELLITE, NULL, _dummy_free, NULL);
}

static void
satellite_signal_cb(void *data EINA_UNUSED, const Eldbus_Message *reply)
{
   if (!unmarshall_satellite(reply))
     {
        ERR("Error: Unable to unmarshall satellite");
        return;
     }

   /* Send out an event to all interested parties that we have an update */
   ecore_event_add(ELOCATION_EVENT_SATELLITE, NULL, _dummy_free, NULL);
}

/* Unmarshall position coordination message */
static Eina_Bool
unmarshall_position(const Eldbus_Message *reply)
{
   GeocluePositionFields fields;
   int32_t level, timestamp;
   double horizontal = 0.0;
   double vertical = 0.0;
   double latitude = 0.0;
   double longitude = 0.0;
   double altitude = 0.0;
   Eldbus_Message_Iter *sub;

   if (!eldbus_message_arguments_get(reply, "iiddd(idd)", &fields, &timestamp,
                                    &latitude, &longitude, &altitude, &sub))
     return EINA_FALSE;

   if (!eldbus_message_iter_arguments_get(sub, "idd", &level, &horizontal, &vertical))
     return EINA_FALSE;

   position->timestamp = timestamp;

   /* GeoClue uses some flags to mark position fields as valid. We set invalid
    * fields to 0.0 */
   if (fields & GEOCLUE_POSITION_FIELDS_LATITUDE)
      position->latitude = latitude;
   else
      position->latitude = 0.0;

   if (fields & GEOCLUE_POSITION_FIELDS_LONGITUDE)
      position->longitude = longitude;
   else
      position->longitude = 0.0;

   if (fields & GEOCLUE_POSITION_FIELDS_ALTITUDE)
      position->altitude = altitude;
   else
      position->altitude = 0.0;

   position->accur->level = level;
   position->accur->horizontal = horizontal;
   position->accur->vertical = vertical;

   return EINA_TRUE;
}

static void
position_cb(void *data EINA_UNUSED, const Eldbus_Message *reply, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *err, *errmsg;

   if (eldbus_message_error_get(reply, &err, &errmsg))
     {
        ERR("Error: %s %s", err, errmsg);
        return;
     }

   if (!unmarshall_position(reply))
     {
        ERR("Error: Unable to unmarshall position");
        return;
     }

   /* Send out an event to all interested parties that we have an update */
   ecore_event_add(ELOCATION_EVENT_POSITION, position, _dummy_free, NULL);
}

static void
position_signal_cb(void *data EINA_UNUSED, const Eldbus_Message *reply)
{
   if (!unmarshall_position(reply))
     {
        ERR("Error: Unable to unmarshall position");
        return;
     }

   /* Send out an event to all interested parties that we have an update */
   ecore_event_add(ELOCATION_EVENT_POSITION, position, _dummy_free, NULL);
}

static Eina_Bool
geoclue_start(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *event EINA_UNUSED)
{
   DBG("GeoClue start event at %s", unique_name);
   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
geoclue_stop(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *event EINA_UNUSED)
{
   DBG("GeoClue stop event");
   return ECORE_CALLBACK_DONE;
}

static void
_reference_add_cb(void *data EINA_UNUSED, const Eldbus_Message *reply, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *err, *errmsg;

   if (eldbus_message_error_get(reply, &err, &errmsg))
     {
        ERR("Error: %s %s", err, errmsg);
        return;
     }

   DBG("Reference added");
}

static void
_reference_del_cb(void *data EINA_UNUSED, const Eldbus_Message *reply EINA_UNUSED, Eldbus_Pending *pending EINA_UNUSED)
{
   /* Dummy callback. We are not waiting for any reply here on shutdown */
}

static void
status_cb(void *data EINA_UNUSED, const Eldbus_Message *reply, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *err, *errmsg;

   if (eldbus_message_error_get(reply, &err, &errmsg))
     {
        ERR("Error: %s %s", err, errmsg);
        return;
     }

   if (!eldbus_message_arguments_get(reply,"i",  &status))
     {
        ERR("Error: Unable to unmarshall status");
        return;
     }

   address_provider->status = position_provider->status = status;
   /* Send out an event to all interested parties that we have an update */
   ecore_event_add(ELOCATION_EVENT_STATUS, &status, _dummy_free, NULL);
}

static void
status_signal_cb(void *data EINA_UNUSED, const Eldbus_Message *reply)
{
   if (!eldbus_message_arguments_get(reply,"i",  &status))
     {
        ERR("Error: Unable to unmarshall status");
        return;
     }

   address_provider->status = position_provider->status = status;
   /* Send out an event to all interested parties that we have an update */
   ecore_event_add(ELOCATION_EVENT_STATUS, &status, _dummy_free, NULL);
}

static void
_dummy_cb(void *data EINA_UNUSED, const Eldbus_Message *msg EINA_UNUSED, Eldbus_Pending *pending EINA_UNUSED)
{
}

/* We got notified from GeoClue that the meta-provider we asked for is now
 * ready. That means we can finish up our initialization and set up all
 * callbacks and handling for the interfaces we use on the meta-provider.
 *
 * We also call the interfaces to get an initial set of data that we can provide
 * to eager aplications.
 */
static void
create_cb(void *data EINA_UNUSED, const Eldbus_Message *reply, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *object_path;
   Eina_Bool updates;
   int accur_level, min_time, resources;
   const char *err, *errmsg;

   if (eldbus_message_error_get(reply, &err, &errmsg))
     {
        ERR("Error: %s %s", err, errmsg);
        return;
     }

   if (!eldbus_message_arguments_get(reply, "o", &object_path)) return;

   DBG("Object path for client: %s", object_path);

   /* With the created object path we now have a meta provider we can operate on.
    * Geoclue handles the selection of the best provider internally for the meta
    * provider */
   obj_meta = eldbus_object_get(conn, GEOCLUE_DBUS_NAME, object_path);
   if (!obj_meta)
     {
        ERR("Error: could not get object for client");
        return;
     }

   meta_geoclue = eldbus_proxy_get(obj_meta, GEOCLUE_GEOCLUE_IFACE);
   if (!meta_geoclue)
     {
        ERR("Error: could not get proxy for geoclue");
        return;
     }

   meta_address = eldbus_proxy_get(obj_meta, GEOCLUE_ADDRESS_IFACE);
   if (!meta_address)
     {
        ERR("Error: could not get proxy address");
        return;
     }

   meta_position = eldbus_proxy_get(obj_meta, GEOCLUE_POSITION_IFACE);
   if (!meta_position)
     {
        ERR("Error: could not get proxy for position");
        return;
     }

   meta_masterclient = eldbus_proxy_get(obj_meta, GEOCLUE_MASTERCLIENT_IFACE);
   if (!meta_masterclient)
     {
        ERR("Error: could not get proxy for master client");
        return;
     }

   meta_velocity = eldbus_proxy_get(obj_meta, GEOCLUE_VELOCITY_IFACE);
   if (!meta_velocity)
     {
        ERR("Error: could not get proxy for velocity");
        return;
     }

   meta_nmea = eldbus_proxy_get(obj_meta, GEOCLUE_NMEA_IFACE);
   if (!meta_nmea)
     {
        ERR("Error: could not get proxy for nmea");
        return;
     }

   meta_satellite = eldbus_proxy_get(obj_meta, GEOCLUE_SATELLITE_IFACE);
   if (!meta_satellite)
     {
        ERR("Error: could not get proxy for satellite");
        return;
     }

   /* Send Geoclue a set of requirements we have for the provider and start the address and position
    * meta provider afterwards. After this we should be ready for operation. */
   updates = EINA_FALSE; /* Especially the web providers do not offer updates */
   accur_level = ELOCATION_ACCURACY_LEVEL_COUNTRY;
   min_time = 0; /* Minimal times between updates (no implemented yet) */
   resources = ELOCATION_RESOURCE_ALL;

   eldbus_proxy_signal_handler_add(meta_masterclient, "AddressProviderChanged",
                                  meta_address_provider_info_signal_cb, address_provider);
   eldbus_proxy_signal_handler_add(meta_masterclient, "PositionProviderChanged",
                                  meta_position_provider_info_signal_cb, position_provider);

   if (!eldbus_proxy_call(meta_masterclient, "SetRequirements", _dummy_cb, NULL, -1, "iibi",
                         accur_level, min_time, updates, resources))
     {
        ERR("Error: could not call SetRequirements");
        return;
     }

   if (!eldbus_proxy_call(meta_masterclient, "AddressStart", _dummy_cb, NULL, -1, ""))
     {
        ERR("Error: could not call AddressStart");
        return;
     }

   if (!eldbus_proxy_call(meta_masterclient, "PositionStart", _dummy_cb, NULL, -1, ""))
     {
        ERR("Error: could not call PositionStart");
        return;
     }

   if (!eldbus_proxy_call(meta_geoclue, "AddReference", _reference_add_cb, NULL, -1, ""))
     {
        ERR("Error: could not call AddReference");
        return;
     }

   if (!eldbus_proxy_call(meta_address, "GetAddress", address_cb, NULL, -1, ""))
     {
        ERR("Error: could not call GetAddress");
        return;
     }

   if (!eldbus_proxy_call(meta_position, "GetPosition", position_cb, NULL, -1, ""))
     {
        ERR("Error: could not call GetPosition");
        return;
     }

   if (!eldbus_proxy_call(meta_geoclue, "GetStatus", status_cb, NULL, -1, ""))
     {
        ERR("Error: could not call GetStatus");
        return;
     }

   if (!eldbus_proxy_call(meta_velocity, "GetVelocity", velocity_cb, NULL, -1, ""))
     {
        ERR("Error: could not call GetVelocity");
        return;
     }

   if (!eldbus_proxy_call(meta_nmea, "GetNmea", nmea_cb, NULL, -1, ""))
     {
        ERR("Error: could not call GetNmea");
        return;
     }

   if (!eldbus_proxy_call(meta_satellite, "GetSatellite", satellite_cb, NULL, -1, ""))
     {
        ERR("Error: could not call GetSatellite");
        return;
     }

   if (!eldbus_proxy_call(meta_satellite, "GetLastSatellite", last_satellite_cb, NULL, -1, ""))
     {
        ERR("Error: could not call GetLastSatellite");
        return;
     }

   if (!eldbus_proxy_call(meta_masterclient, "GetAddressProvider", meta_address_provider_info_cb,
                         address_provider, -1, ""))
     {
        ERR("Error: could not call GetAddressProvider");
        return;
     }

   if (!eldbus_proxy_call(meta_masterclient, "GetPositionProvider", meta_position_provider_info_cb,
                         position_provider, -1, ""))
     {
        ERR("Error: could not call GetPositionProvider");
        return;
     }

   eldbus_proxy_signal_handler_add(meta_address, "AddressChanged", address_signal_cb, NULL);
   eldbus_proxy_signal_handler_add(meta_position, "PositionChanged", position_signal_cb, NULL);
   eldbus_proxy_signal_handler_add(meta_geoclue, "StatusChanged", status_signal_cb, NULL);
   eldbus_proxy_signal_handler_add(meta_velocity, "VelocityChanged", velocity_signal_cb, NULL);
   eldbus_proxy_signal_handler_add(meta_nmea, "NmeaChanged", nmea_signal_cb, NULL);
   eldbus_proxy_signal_handler_add(meta_satellite, "SatelliteChanged", satellite_signal_cb, NULL);

   ecore_event_add(ELOCATION_EVENT_META_READY, NULL, NULL, NULL);
}

static void
_name_owner_changed(void *data EINA_UNUSED, const char *bus EINA_UNUSED, const char *old, const char *new)
{
   if (old[0] == '\0' && new[0] != '\0')
     {
        ecore_event_add(ELOCATION_EVENT_IN, NULL, NULL, NULL);
        unique_name = strdup(new);
     }
   else if (old[0] != '\0' && new[0] == '\0')
     {
        if (strcmp(unique_name, old) != 0)
           WARN("%s was not the known name %s, ignored.", old, unique_name);
        else
           ecore_event_add(ELOCATION_EVENT_OUT, NULL, NULL, NULL);
     }
   else
     {
        DBG("unknown change from %s to %s", old, new);
     }
}

/* Public API function to request a landmarks position based on an address object */
EAPI Eina_Bool
elocation_landmarks_get(Elocation_Position *position_shadow EINA_UNUSED, Elocation_Address *address_shadow EINA_UNUSED)
{
   Eldbus_Message *msg;
   Eldbus_Message_Iter *iter;
   const char *keyword = NULL, *lang = NULL, *country_code = NULL;
   int limit = 0;
   double left= 0.0, top = 0.0, right = 0.0, bottom = 0.0;

   msg = eldbus_proxy_method_call_new(master_poi, "SearchByPosition");
   iter = eldbus_message_iter_get(msg);
   eldbus_message_iter_basic_append(iter, 's', keyword);
   eldbus_message_iter_basic_append(iter, 's', lang);
   eldbus_message_iter_basic_append(iter, 's', country_code);
   eldbus_message_iter_basic_append(iter, 'i', limit);
   eldbus_message_iter_basic_append(iter, 'd', left);
   eldbus_message_iter_basic_append(iter, 'd', top);
   eldbus_message_iter_basic_append(iter, 'd', right);
   eldbus_message_iter_basic_append(iter, 'd', bottom);
   if (!eldbus_proxy_send(master_poi, msg, poi_cb, NULL, -1))
     {
        ERR("Error: could not call SearchByPosition");
        eldbus_message_unref(msg);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

/* Public API function to get an address from a position */
EAPI Eina_Bool
elocation_position_to_address(Elocation_Position *position_shadow, Elocation_Address *address_shadow EINA_UNUSED)
{
   Eldbus_Message *msg;
   Eldbus_Message_Iter *iter, *structure;

   msg = eldbus_proxy_method_call_new(geonames_rgeocode, "PositionToAddress");
   iter = eldbus_message_iter_get(msg);
   eldbus_message_iter_basic_append(iter, 'd', position_shadow->latitude);
   eldbus_message_iter_basic_append(iter, 'd', position_shadow->longitude);
   structure = eldbus_message_iter_container_new(iter, 'r', NULL);
   eldbus_message_iter_basic_append(structure, 'i', position_shadow->accur->level);
   eldbus_message_iter_basic_append(structure, 'd', position_shadow->accur->horizontal);
   eldbus_message_iter_basic_append(structure, 'd', position_shadow->accur->vertical);
   eldbus_message_iter_container_close(iter, structure);
   if (!eldbus_proxy_send(geonames_rgeocode, msg, rgeocode_cb, NULL, -1))
     {
        ERR("Error: could not call PositionToAddress");
        eldbus_message_unref(msg);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

/* Public API function to get a position from and address */
EAPI Eina_Bool
elocation_address_to_position(Elocation_Address *address_shadow, Elocation_Position *position_shadow EINA_UNUSED)
{
   Eldbus_Message *msg;
   Eldbus_Message_Iter *iter, *array;

   /* In function macro to generate a key value pair structure for the dict */
   #define ENTRY(key) { #key, address_shadow->key }
   struct keyval {
      const char *key;
      const char *val;
   } keyval[] = {
      ENTRY(country),
      ENTRY(countrycode),
      ENTRY(locality),
      ENTRY(postalcode),
      ENTRY(region),
      ENTRY(timezone),
      { NULL, NULL }
   };
   #undef ENTRY

   struct keyval *k;

   msg = eldbus_proxy_method_call_new(geonames_geocode, "AddressToPosition");
   iter = eldbus_message_iter_get(msg);

   array = eldbus_message_iter_container_new(iter, 'a', "{ss}");

   for (k = keyval; k && k->key; k++)
     {
        Eldbus_Message_Iter *entry;

        if (!k->val) continue;

        entry = eldbus_message_iter_container_new(array, 'e', NULL);
        eldbus_message_iter_arguments_append(entry, "ss", k->key, k->val);
        eldbus_message_iter_container_close(array, entry);
     }

   eldbus_message_iter_container_close(iter, array);

   if (!eldbus_proxy_send(geonames_geocode, msg, geocode_cb, NULL, -1))
     {
        ERR("Error: could not call AddressToPosition");
        eldbus_message_unref(msg);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

/* Public API function to get the position from a freeform text input style
 * address
 */
EAPI Eina_Bool
elocation_freeform_address_to_position(const char *freeform_address, Elocation_Position *position_shadow EINA_UNUSED)
{
   if (!eldbus_proxy_call(geonames_geocode, "FreeformAddressToPosition", geocode_cb, NULL, -1, "s", freeform_address))
     {
        ERR("Error: could not call FreeformAddressToPosition");
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

/* Public API function to request the current address */
EAPI Eina_Bool
elocation_address_get(Elocation_Address *address_shadow)
{
   if (!address) return EINA_FALSE;
   if (address == address_shadow) return EINA_TRUE;

   *address_shadow = *address;
   return EINA_TRUE;
}

/* Public API function to request the current position */
EAPI Eina_Bool
elocation_position_get(Elocation_Position *position_shadow)
{
   if (!position) return EINA_FALSE;
   if (position == position_shadow) return EINA_TRUE;

   *position_shadow = *position;
   return EINA_TRUE;
}

/* Public API function to request the status */
EAPI Eina_Bool
elocation_status_get(int *status_shadow)
{
   if (status < 0) return EINA_FALSE;
   if (&status == status_shadow) return EINA_TRUE;

   *status_shadow = status;
   return EINA_TRUE;
}

/* Public API function to create a new position object */
EAPI Elocation_Position *
elocation_position_new(void)
{
   /* Malloc the global struct we operate on here in this lib. This shadows the
    * updated data we are giving to the application */
   position = calloc(1, sizeof(Elocation_Position));
   if (!position) return NULL;

   position->accur = calloc(1, sizeof(Elocation_Accuracy));
   if (!position->accur) return NULL;

   return position;
}

/* Public API function to create an new address object */
EAPI Elocation_Address *
elocation_address_new(void)
{
   /* Malloc the global struct we operate on here in this lib. This shadows the
    * updated data we are giving to the application */
   address = calloc(1, sizeof(Elocation_Address));
   if (!address) return NULL;

   address->accur = calloc(1, sizeof(Elocation_Accuracy));
   if (!address->accur) return NULL;

   return address;
}

/* Public API function to free an position object */
EAPI void
elocation_position_free(Elocation_Position *position_shadow)
{
   if (position != position_shadow)
     {
        ERR("Corrupted position object");
        return;
     }

   free(position->accur);
   free(position);
}

/* Public API function to free an address object */
EAPI void
elocation_address_free(Elocation_Address *address_shadow)
{
   if (address != address_shadow)
     {
        ERR("Corrupted address object");
        return;
     }

   if (address)
     {
        free(address->country);
        free(address->countrycode);
        free(address->locality);
        free(address->postalcode);
        free(address->region);
        free(address->timezone);
        free(address->accur);
        free(address);
     }
}

/* Public API function to initialize the elocation library */
EAPI Eina_Bool
elocation_init(void)
{
   Eldbus_Object *obj_master = NULL;
   Eldbus_Object *obj_geonames = NULL;

   if (!eina_init()) return EINA_FALSE;
   if (!ecore_init()) return EINA_FALSE;
   if (!eldbus_init()) return EINA_FALSE;

   _elocation_log_dom = eina_log_domain_register("elocation", EINA_COLOR_BLUE);
   if (_elocation_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register 'elocation' log domain.");
     }

   /* Create objects, one for each kind, we operate on internally */
   address_provider = calloc(1, sizeof(Elocation_Provider));
   position_provider = calloc(1, sizeof(Elocation_Provider));

   addr_geocode = calloc(1, sizeof(Elocation_Address));
   if (!addr_geocode) return EINA_FALSE;

   addr_geocode->accur = calloc(1, sizeof(Elocation_Accuracy));
   if (!addr_geocode->accur) return EINA_FALSE;

   pos_geocode = calloc(1, sizeof(Elocation_Position));
   if (!pos_geocode) return EINA_FALSE;

   pos_geocode->accur = calloc(1, sizeof(Elocation_Accuracy));
   if (!pos_geocode->accur) return EINA_FALSE;

   conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   if (!conn)
     {
      ERR("Error: could not connect to session bus.");
      return EXIT_FAILURE;
     }

   /* Create all ecore event types we send out to interested applications */
   ELOCATION_EVENT_IN = ecore_event_type_new();
   ELOCATION_EVENT_OUT = ecore_event_type_new();
   ELOCATION_EVENT_STATUS = ecore_event_type_new();
   ELOCATION_EVENT_POSITION = ecore_event_type_new();
   ELOCATION_EVENT_ADDRESS = ecore_event_type_new();
   ELOCATION_EVENT_VELOCITY = ecore_event_type_new();
   ELOCATION_EVENT_GEOCODE = ecore_event_type_new();
   ELOCATION_EVENT_REVERSEGEOCODE = ecore_event_type_new();
   ELOCATION_EVENT_NMEA = ecore_event_type_new();
   ELOCATION_EVENT_SATELLITE = ecore_event_type_new();
   ELOCATION_EVENT_POI = ecore_event_type_new();
   ELOCATION_EVENT_META_READY = ecore_event_type_new();

   obj_master= eldbus_object_get(conn, GEOCLUE_DBUS_NAME, GEOCLUE_OBJECT_PATH);
   if (!obj_master)
     {
        ERR("Error: could not get object");
        return EXIT_FAILURE;
     }

   manager_master = eldbus_proxy_get(obj_master, GEOCLUE_MASTER_IFACE);
   if (!manager_master)
     {
        ERR("Error: could not get proxy");
        return EXIT_FAILURE;
     }

   /* Create a meta provider for all normal use cases. This will allow GeoClue
    * to decide which provider is the best for us internally.
    * Right now we don't have the functionality in place to specifically request
    * a provider but we maybe need this in the future. We will try without it
    * for now.
    */
   if (!eldbus_proxy_call(manager_master, "Create", create_cb, NULL, -1, ""))
     {
        ERR("Error: could not call Create");
        return EXIT_FAILURE;
     }

   master_poi = eldbus_proxy_get(obj_master, GEOCLUE_POI_IFACE);
   if (!master_poi)
     {
        ERR("Error: could not get proxy");
        return EXIT_FAILURE;
     }

   /* Geocode and reverse geocode never show up as meta provider. Still we want
    * to be able to convert so we keep them around directly here. */
   obj_geonames= eldbus_object_get(conn, GEONAMES_DBUS_NAME, GEONAMES_OBJECT_PATH);
   if (!obj_geonames)
     {
        ERR("Error: could not get object for geonames");
        return EXIT_FAILURE;
     }

   geonames_geocode = eldbus_proxy_get(obj_geonames, GEOCLUE_GEOCODE_IFACE);
   if (!geonames_geocode)
     {
        ERR("Error: could not get proxy");
        return EXIT_FAILURE;
     }

   geonames_rgeocode = eldbus_proxy_get(obj_geonames, GEOCLUE_REVERSEGEOCODE_IFACE);
   if (!geonames_rgeocode)
     {
        ERR("Error: could not get proxy");
        return EXIT_FAILURE;
     }

   eldbus_name_owner_changed_callback_add(conn, GEOCLUE_DBUS_NAME, _name_owner_changed,
                                         NULL, EINA_TRUE);

   ecore_event_handler_add(ELOCATION_EVENT_IN, geoclue_start, NULL);
   ecore_event_handler_add(ELOCATION_EVENT_OUT, geoclue_stop, NULL);

   return EINA_TRUE;
}

/* Public API function to shutdown the elocation library form the application */
EAPI void
elocation_shutdown(void)
{
   /* Depending on if the create_cb was successfully received meta_geoclue is
    * setup or not. So we  * need to check here if this is not the case
    */
   if (meta_geoclue)
     {
        /* To allow geoclue freeing unused providers we free our reference on it here */
        if (!eldbus_proxy_call(meta_geoclue, "RemoveReference", _reference_del_cb, NULL, -1, ""))
          {
            ERR("Error: could not call RemoveReference");
          }
     }

   ecore_event_type_flush(ELOCATION_EVENT_IN,
                          ELOCATION_EVENT_OUT,
                          ELOCATION_EVENT_STATUS,
                          ELOCATION_EVENT_POSITION,
                          ELOCATION_EVENT_ADDRESS,
                          ELOCATION_EVENT_VELOCITY,
                          ELOCATION_EVENT_GEOCODE,
                          ELOCATION_EVENT_REVERSEGEOCODE,
                          ELOCATION_EVENT_NMEA,
                          ELOCATION_EVENT_SATELLITE,
                          ELOCATION_EVENT_POI,
                          ELOCATION_EVENT_META_READY);

   /* Quite a bit of allocated string and generic memory cleanup. This should be
    *less when we went away from all this global var business.
    */
   if (address_provider)
     {
        free(address_provider->name);
        free(address_provider->description);
        free(address_provider->service);
        free(address_provider->path);
        free(address_provider);
     }

   if (position_provider)
     {
        free(position_provider->name);
        free(position_provider->description);
        free(position_provider->service);
        free(position_provider->path);
        free(position_provider);
     }

   if (pos_geocode)
     {
        free(pos_geocode->accur);
        free(pos_geocode);
     }

   if (addr_geocode)
     {
        free(addr_geocode->country);
        free(addr_geocode->countrycode);
        free(addr_geocode->locality);
        free(addr_geocode->postalcode);
        free(addr_geocode->region);
        free(addr_geocode->timezone);
        free(addr_geocode->accur);
        free(addr_geocode);
     }

   /* Unreference some eldbus strcutures we now longer use. To allow eldbus to
    * free them internally.
    */
   if (manager_master)
      eldbus_proxy_unref(manager_master);

   eldbus_name_owner_changed_callback_del(conn, GEOCLUE_DBUS_NAME, _name_owner_changed, NULL);
   eldbus_connection_unref(conn);
   eldbus_shutdown();
   ecore_shutdown();
   eina_log_domain_unregister(_elocation_log_dom);
   eina_shutdown();
}
