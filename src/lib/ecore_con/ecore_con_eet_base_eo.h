#ifndef _ECORE_CON_EET_BASE_EO_H_
#define _ECORE_CON_EET_BASE_EO_H_

#ifndef _ECORE_CON_EET_BASE_EO_CLASS_TYPE
#define _ECORE_CON_EET_BASE_EO_CLASS_TYPE

typedef Eo Ecore_Con_Eet_Base;

#endif

#ifndef _ECORE_CON_EET_BASE_EO_TYPES
#define _ECORE_CON_EET_BASE_EO_TYPES

/** Ecore connection reply data structure
 *
 * @ingroup Ecore_Con
 */
typedef struct _Ecore_Con_Reply Ecore_Con_Reply;


#endif
/**
 * @brief Ecore Connection Eet Base class.
 *
 * This class provides Eet data serialization features to Ecore Connection
 * objects.
 *
 * @ingroup Ecore_Con_Eet_Base
 */
#define ECORE_CON_EET_BASE_CLASS ecore_con_eet_base_class_get()

EWAPI const Efl_Class *ecore_con_eet_base_class_get(void);

/**
 * @brief The server object to which we send and receive.
 *
 * @param[in] obj The object.
 * @param[in] data Server object
 *
 * @ingroup Ecore_Con_Eet_Base
 */
EOAPI void ecore_con_eet_base_server_set(Eo *obj, Ecore_Con_Server *data);

/**
 * @brief The server object to which we send and receive.
 *
 * @param[in] obj The object.
 *
 * @return Server object
 *
 * @ingroup Ecore_Con_Eet_Base
 */
EOAPI Ecore_Con_Server *ecore_con_eet_base_server_get(const Eo *obj);

/**
 * @brief A callback function which should be called when data is received by
 * ecore_con_eet_object.
 *
 * @param[in] obj The object.
 * @param[in] name The name of the eet stream.
 * @param[in] func The callback function.
 * @param[in] data The data (if any) that should be passed to callback
 * function.
 *
 * @ingroup Ecore_Con_Eet_Base
 */
EOAPI void ecore_con_eet_base_data_callback_set(Eo *obj, const char *name, Ecore_Con_Eet_Data_Cb func, const void *data);

/**
 * @brief A callback function which should be calledn when raw data is received
 * by ecore_con_eet_object.
 *
 * @param[in] obj The object.
 * @param[in] name The name of the eet stream.
 * @param[in] func The callback function.
 * @param[in] data The data (if any) that should be passed to callback
 * function.
 *
 * @ingroup Ecore_Con_Eet_Base
 */
EOAPI void ecore_con_eet_base_raw_data_callback_set(Eo *obj, const char *name, Ecore_Con_Eet_Raw_Data_Cb func, const void *data);

/**
 * @brief Function to delete the @ref ecore_con_eet_base_data_callback_set.
 *
 * @param[in] obj The object.
 * @param[in] name The name of the eet stream.
 *
 * @ingroup Ecore_Con_Eet_Base
 */
EOAPI void ecore_con_eet_base_data_callback_del(Eo *obj, const char *name);

/**
 * @brief Function to delete the @ref ecore_con_eet_base_raw_data_callback_set.
 *
 * @param[in] obj The object.
 * @param[in] name The name of the eet stream.
 *
 * @ingroup Ecore_Con_Eet_Base
 */
EOAPI void ecore_con_eet_base_raw_data_callback_del(Eo *obj, const char *name);

/**
 * @brief Function to register a @ref Eet.Data.Descriptor to the ecore_con_eet
 * object.
 *
 * @param[in] obj The object.
 * @param[in] name The name of the eet stream.
 * @param[in] edd The Eet.Data.Descriptor that is to be registered.
 *
 * @ingroup Ecore_Con_Eet_Base
 */
EOAPI void ecore_con_eet_base_register(Eo *obj, const char *name, Eet_Data_Descriptor *edd);

/**
 * @brief Function to send data.
 *
 * @param[in] obj The object.
 * @param[in] reply Contains the ecore_con_eet object to which the data has to
 * be sent.
 * @param[in] name The name of the eet stream.
 * @param[in] value Actual data
 *
 * @ingroup Ecore_Con_Eet_Base
 */
EOAPI void ecore_con_eet_base_send(Eo *obj, Ecore_Con_Reply *reply, const char *name, void *value);

/**
 * @brief Function to send raw data.
 *
 * @param[in] obj The object.
 * @param[in] reply Contains the ecore_con_eet object to which the data has to
 * be sent.
 * @param[in] protocol_name The name of the eet stream.
 * @param[in] section Name of section in the eet descriptor.
 * @param[in] section_data
 *
 * @ingroup Ecore_Con_Eet_Base
 */
EOAPI void ecore_con_eet_base_raw_send(Eo *obj, Ecore_Con_Reply *reply, const char *protocol_name, const char *section, Eina_Binbuf *section_data);

#endif
